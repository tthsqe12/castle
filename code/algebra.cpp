#include <cmath>
#include <cfloat>

#include "uex.h"
#include "timing.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"
#include "polynomial.h"

#ifndef NOFACTOR

#include <flint/fmpq_mpoly_factor.h>
#include <flint/profiler.h>
#include <flint/fmpz.h>

#include "djrat.h"

ex eval_fakeabs(bool&switched, ex E)
{
    uex e(E);

    switched = false;
    if (eis_number(E))
    {
        if (   (eis_int(E) && fmpz_sgn(eint_data(E)) < 0)
            || (eis_rat(E) && fmpq_sgn(erat_data(E)) < 0))
        {
            switched = true;
            e.setnz(num_Minus1(e.get()));
        }
        return e.release();
    }

    if (ehas_head_sym(E, gs.sym_sTimes.get()) 
             && elength(E) > 1
             && eis_number(echild(E,1)))
    {
        er F = echild(E,1);
        if (   (eis_int(F) && fmpz_sgn(eint_data(F)) < 0)
            || (eis_rat(F) && fmpq_sgn(erat_data(F)) < 0))
        {
            switched = true;
            if (eis_int(F, -1))
            {
                if (elength(E) > 2)
                    e.removechild(1);
                else
                    e.setnz(e.copychild(2));
            }
            else
            {
                e.replacechild(1, num_Minus1(F));
            }
        }
    }

    return e.release();
}

static void split_numerator_denominator(std::vector<wex> & a, std::vector<wex> & b, er e)
{
    if (ehas_head_sym(e, gs.sym_sTimes.get()))
    {
        for (size_t i = 1; i <= elength(e); i++)
            split_numerator_denominator(a, b, echild(e,i));
    }
    else if (ehas_head_sym_length(e, gs.sym_sPower.get(), 2))
    {
        bool negative = false;
        if (ehas_head_sym(echild(e,2), gs.sym_sPlus.get()))
        {
            for (size_t i = 1; i <= elength(echild(e,2)); i++)
            {
                ex newpow = eval_fakeabs(negative, ecopychild(e,2,i));
                (negative ? b : a).push_back(emake_node(gs.sym_sPower.copy(), ecopychild(e,1), newpow));
            }
        }
        else
        {
            ex newpow = eval_fakeabs(negative, ecopychild(e,2));
            if (negative)
            {
                b.push_back(emake_node(gs.sym_sPower.copy(), ecopychild(e,1), newpow));
            }
            else
            {
                eclear(newpow);
                a.push_back(ecopy(e));
            }
        }
    }
    else if (eis_int(e))
    {
        a.push_back(ecopy(e));
    }
    else if (eis_rat(e))
    {
        b.push_back(emake_int_copy(fmpq_denref(erat_data(e))));
        if (!fmpz_is_one(fmpq_numref(erat_data(e))))
            a.push_back(emake_int_copy(fmpq_numref(erat_data(e))));
    }
    else
    {
        a.push_back(wex(ecopy(e)));
    }
}


ex dcode_sNumerator(er e)
{
//std::cout << "dcode_sNumerator: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sNumerator.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    if (eis_int(echild(e,1)))
    {
        return ecopy(e);
    }
    else if (eis_rat(echild(e,1)))
    {
        return emake_int_copy(fmpq_numref(erat_data(echild(e,1))));
    }
    else
    {
        std::vector<wex> num, den;
        split_numerator_denominator(num, den, echild(e,1));
        return emake_node_times(num);
    }
}

ex dcode_sDenominator(er e)
{
//std::cout << "dcode_sDenominator: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sDenominator.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    if (eis_int(echild(e,1)))
    {
        return emake_cint(1);
    }
    else if (eis_rat(echild(e,1)))
    {
        return emake_int_copy(fmpq_denref(erat_data(echild(e,1))));
    }
    else
    {
        std::vector<wex> num, den;
        split_numerator_denominator(num, den, echild(e,1));
        return emake_node_times(den);
    }
}



ex fmpq_mpoly_to_ex(const fmpq_mpoly_t a, const fmpq_mpoly_ctx_t ctx, const std::vector<wex> vars)
{
    slong n = ctx->zctx->minfo->nvars;
    assert(n == vars.size());

    uex e(gs.sym_sPlus.get(), a->zpoly->length);

    xfmpq c;

	if (a->zpoly->bits <= FLINT_BITS)
	{
		std::vector<ulong> exps(n);
	    for (slong i = fmpq_mpoly_length(a, ctx) - 1; i >= 0; i--)
	    {
	        uex t(gs.sym_sTimes.get(), n + 1);
	        fmpq_mpoly_get_term_coeff_fmpq(c.data, a, i, ctx);
	        fmpq_mpoly_get_term_exp_ui(exps.data(), a, i, ctx);
	        t.push_back(emake_rat_move(c));
	        for (slong j = 0; j < n; j++)
	        {
	            t.push_back(emake_node(gs.sym_sPower.copy(), vars[j].copy(), emake_int_ui(exps[j])));
	        }
	        e.push_back(t.release());
	    }
	}
	else
	{
		std::vector<xfmpz> exps;
		std::vector<fmpz *> expps;
	    for (slong i = 0; i < n; i++)
			exps.push_back(xfmpz());
	    for (slong i = 0; i < n; i++)
			expps.push_back(exps[i].data);

	    for (slong i = fmpq_mpoly_length(a, ctx) - 1; i >= 0; i--)
	    {
	        uex t(gs.sym_sTimes.get(), n + 1);
	        fmpq_mpoly_get_term_coeff_fmpq(c.data, a, i, ctx);
	        fmpq_mpoly_get_term_exp_fmpz(expps.data(), a, i, ctx);
	        t.push_back(emake_rat_move(c));
	        for (slong j = 0; j < n; j++)
	        {
	            t.push_back(emake_node(gs.sym_sPower.copy(), vars[j].copy(), emake_int_move(exps[j])));
	        }
	        e.push_back(t.release());
	    }
	}

    return e.release();
}

ex nmod_mpoly_to_ex(const nmod_mpoly_t a, const nmod_mpoly_ctx_t ctx, const std::vector<wex> vars)
{
    slong n = ctx->minfo->nvars;
    assert(n == vars.size());

    uex e(gs.sym_sPlus.get(), a->length);

	if (a->bits <= FLINT_BITS)
	{
		std::vector<ulong> exps(n);
	    for (slong i = nmod_mpoly_length(a, ctx) - 1; i >= 0; i--)
	    {
	        uex t(gs.sym_sTimes.get(), n + 1);
	        ulong c = nmod_mpoly_get_term_coeff_ui(a, i, ctx);
	        nmod_mpoly_get_term_exp_ui(exps.data(), a, i, ctx);
	        t.push_back(emake_int_ui(c));
	        for (slong j = 0; j < n; j++)
	        {
	            t.push_back(emake_node(gs.sym_sPower.copy(), vars[j].copy(), emake_int_ui(exps[j])));
	        }
	        e.push_back(t.release());
	    }
	}
	else
	{
		std::vector<xfmpz> exps;
		std::vector<fmpz *> expps;
	    for (slong i = 0; i < n; i++)
			exps.push_back(xfmpz());
	    for (slong i = 0; i < n; i++)
			expps.push_back(exps[i].data);

	    for (slong i = nmod_mpoly_length(a, ctx) - 1; i >= 0; i--)
	    {
	        uex t(gs.sym_sTimes.get(), n + 1);
	        ulong c = nmod_mpoly_get_term_coeff_ui(a, i, ctx);
	        nmod_mpoly_get_term_exp_fmpz(expps.data(), a, i, ctx);
	        t.push_back(emake_int_ui(c));
	        for (slong j = 0; j < n; j++)
	        {
	            t.push_back(emake_node(gs.sym_sPower.copy(), vars[j].copy(), emake_int_move(exps[j])));
	        }
	        e.push_back(t.release());
	    }
	}

    return e.release();
}



void poly_perm_vars(poly & a, poly & b, std::vector<size_t> p)
{
    a.coeffs = b.coeffs;
    a.exps.clear();
    std::vector<xfmpz> t(a.vars.size());
    for (size_t i = 0; i < b.coeffs.size(); i++)
    {
        for (size_t j = 0; j < a.vars.size(); j++)
        {
            fmpz_zero(t[j].data);
        }

        for (size_t j = 0; j < b.vars.size(); j++)
        {
            fmpz_set(t[p[j]].data, b.exps[i*b.vars.size() + j].data);
        }

        for (size_t j = 0; j < a.vars.size(); j++)
        {
            a.exps.push_back(t[j]);
        }
    }
}


void poly_add(poly & a, poly & b, poly & c)
{
    a.vars.clear();

    std::vector<size_t> bperm;
    for (size_t i = 0; i < b.vars.size(); i++)
    {
        a.vars.push_back(wex(b.vars[i].copy()));
        bperm.push_back(i);
    }

    std::vector<size_t> cperm;
    for (size_t i = 0; i < c.vars.size(); i++)
    {
        for (size_t j = 0; j < a.vars.size(); j++)
        {
            if (ex_same(a.vars[j].get(), c.vars[i].get()))
            {
                cperm.push_back(j);
                break;
            }
        }
        if (cperm.size() <= i)
        {
            cperm.push_back(a.vars.size());
            a.vars.push_back(wex(c.vars[i].copy()));
        }
    }

    poly B(0), C(0);
    B.vars = a.vars;
    poly_perm_vars(B, b, bperm);
    poly_sort_terms(B);
    C.vars = a.vars;
    poly_perm_vars(C, c, cperm);
    poly_sort_terms(C);

    poly_add_vars_match(a, B, C);
}




void poly_mul(poly & a, poly & b, poly & c)
{
    a.vars.clear();

    std::vector<size_t> bperm;
    for (size_t i = 0; i < b.vars.size(); i++)
    {
        a.vars.push_back(wex(b.vars[i].copy()));
        bperm.push_back(i);
    }

    std::vector<size_t> cperm;
    for (size_t i = 0; i < c.vars.size(); i++)
    {
        for (size_t j = 0; j < a.vars.size(); j++)
        {
            if (ex_same(a.vars[j].get(), c.vars[i].get()))
            {
                cperm.push_back(j);
                break;
            }
        }
        if (cperm.size() <= i)
        {
            cperm.push_back(a.vars.size());
            a.vars.push_back(wex(c.vars[i].copy()));
        }
    }

    poly B(0), C(0);
    B.vars = a.vars;
    poly_perm_vars(B, b, bperm);
    poly_sort_terms(B);
    C.vars = a.vars;
    poly_perm_vars(C, c, cperm);
    poly_sort_terms(C);

    poly_mul_vars_match(a, B, C);
}

void poly_append(poly & b, poly & c)
{
    std::vector<wex> vars;

    std::vector<size_t> bperm;
    for (size_t i = 0; i < b.vars.size(); i++)
    {
        vars.push_back(wex(b.vars[i].copy()));
        bperm.push_back(i);
    }

    bool must_change = false;

    std::vector<size_t> cperm;
    for (size_t i = 0; i < c.vars.size(); i++)
    {
        for (size_t j = 0; j < vars.size(); j++)
        {
            if (ex_same(vars[j].get(), c.vars[i].get()))
            {
                cperm.push_back(j);
                break;
            }
        }
        if (cperm.size() <= i)
        {
            cperm.push_back(vars.size());
            vars.push_back(wex(c.vars[i].copy()));
            must_change = true;
        }
    }

    poly C(0);
    C.vars = vars;
    poly_perm_vars(C, c, cperm);

    if (must_change)
    {
//std::cout << "must_change" << std::endl;
        poly B(0);
        B.vars = vars;
        poly_perm_vars(B, b, bperm);
        b.swap(B);
    }

    assert(b.vars.size() == vars.size());
    assert(C.vars.size() == vars.size());

    for (size_t i = 0; i < C.coeffs.size(); i++)
    {
        b.coeffs.push_back(C.coeffs[i]);
        for (size_t j = 0; j < vars.size(); j++)
        {
            b.exps.push_back(C.exps[i*vars.size() + j]);
        }
    }
}


ex enumber_reduce_mod(er e, const xfmpz & m)
{
    assert(fmpz_sgn(m.data) >= 0);

    if (fmpz_is_zero(m.data))
        return ecopy(e);

    if (eis_number(e))
    {
        switch (etype(e))
        {
            case ETYPE_INT:
            {
                ex x = emake_int();
                fmpz_mod(eint_data(x), eint_data(e), m.data);
                return ereturn_int(x);
            }
            case ETYPE_RAT:
            {
                ex x = emake_int();
                if (fmpq_mod_fmpz(eint_data(x), erat_data(e), m.data))
                    return ereturn_int(x);
                eclear(x);
                return nullptr;
            }
            default:
            {
                return nullptr;
            }
        }
    }
    else
    {
        return ecopy(e);
    }
}

void monomial_swap(xfmpz * A, xfmpz * B, size_t nvars);

int poly_reduce_mod(poly & A, const xfmpz & m)
{
    size_t nvars = A.vars.size();
    xfmpz * Aexps = A.exps.data();
    size_t Alen = 0;
    for (size_t i = 0; i < A.coeffs.size(); i++)
    {
        ex x = enumber_reduce_mod(A.coeffs[i].get(), m);
        if (x == nullptr)
            return 1;
        if (eis_zero(x))
        {
            eclear(x);
        }
        else
        {
            A.coeffs[Alen].reset(x);
            if (Alen != i)
                monomial_swap(Aexps + Alen*nvars, Aexps + i*nvars, nvars);
            Alen++;
        }
    }

    A.coeffs.resize(Alen);
    A.exps.resize(Alen*nvars);
    return 0;
}

/* return 0 for success, nonzero for failure */
int poly_set_any_ex(poly & p, er e, const xfmpz & m)
{
    int r;

    if (!eis_node(e))
    {
        if (eis_number(e))
        {
            p.vars.clear();
            p.exps.clear();
            p.coeffs.clear();
            ex c = enumber_reduce_mod(e, m);
            if (c == nullptr)
                return 1;
            if (eis_zero(c))
                eclear(c);
            else
                p.coeffs.push_back(wex(c));
            return 0;
        }
        else
        {
            p.vars.clear();
            p.exps.clear();
            p.coeffs.clear();
            p.vars.push_back(wex(ecopy(e)));
            p.coeffs.push_back(wex(emake_cint(1)));
            p.exps.push_back(xfmpz(ulong(1)));
        }
//std::cout << " from " << ex_tostring(e) << " got " << p.tostring() << std::endl;
        return 0;
    }
    else if (ehas_head_sym(e, gs.sym_sPlus.get()))
    {
        p.vars.clear();
        p.exps.clear();
        p.coeffs.clear();
        poly q(0);
        for (size_t i = 1; i <= elength(e); i++)
        {
            r = poly_set_any_ex(q, echild(e,i), m);
            if (r != 0)
            {
//std::cout << " from " << ex_tostring(e) << " failed 5" << std::endl;
                return r;
            }
            poly_append(p, q);
        }

timeit_t timer;
timeit_start(timer);
        poly_sort_terms(p);
timeit_stop(timer);
//std::cout << "sort time: " << timer->wall << std::endl;


timeit_start(timer);
        poly_combine_like_terms(p);
timeit_stop(timer);
//std::cout << "combine time: " << timer->wall << std::endl;

        r = poly_reduce_mod(p, m);
        if (r != 0)
        {
//std::cout << " from " << ex_tostring(e) << " failed 6" << std::endl;
            return r;
        }

//std::cout << " from " << ex_tostring(e) << " got " << p.tostring() << std::endl;
        return 0;
    }
    else if (ehas_head_sym(e, gs.sym_sTimes.get()))
    {
        p.vars.clear();
        p.exps.clear();
        p.coeffs.clear();
        p.coeffs.push_back(wex(emake_cint(1)));
        poly t(0), q(0);
        for (size_t i = 1; i <= elength(e); i++)
        {
            r = poly_set_any_ex(q, echild(e,i), m);
            if (r != 0)
            {
//std::cout << " from " << ex_tostring(e) << " failed 4" << std::endl;
                return r;
            }
            poly_mul(t, p, q);
            p.swap(t);
        }

        r = poly_reduce_mod(p, m);
        if (r != 0)
        {
//std::cout << " from " << ex_tostring(e) << " failed 7" << std::endl;
            return r;
        }


//std::cout << " from " << ex_tostring(e) << " got " << p.tostring() << std::endl;
        return 0;
    }
    else if (ehas_head_sym_length(e, gs.sym_sPower.get(), 2)
                && eis_int(echild(e,2))
                && fmpz_sgn(eint_data(echild(e,2))) >= 0)
    {
        poly q(0);
        r = poly_set_any_ex(q, echild(e,1), m);
        if (r != 0)
        {
//std::cout << " from " << ex_tostring(e) << " failed 3" << std::endl;
            return r;
        }
        r = poly_pow(p, q, fmpz_get_ui(eint_data(echild(e,2))));
        if (r != 0)
        {
//std::cout << " from " << ex_tostring(e) << " failed 2" << std::endl;
            return r;
        }


        r = poly_reduce_mod(p, m);
        if (r != 0)
        {
//std::cout << " from " << ex_tostring(e) << " failed 8" << std::endl;
            return r;
        }


//std::cout << " from " << ex_tostring(e) << " got " << p.tostring() << std::endl;
        return 0;
    }
    else
    {
//std::cout << " from " << ex_tostring(e) << " failed 1" << std::endl;
        return 1;
    }
}


/* return 0 for successs, nonzero for failure */
int poly_to_fmpq_mpoly(fmpq_mpoly_t a, const fmpq_mpoly_ctx_t ctx, const poly & b)
{
    size_t n = b.vars.size();
    assert(n == ctx->zctx->minfo->nvars);

    fmpz ** exps = new fmpz*[n];
    for (slong i = 0; i < n; i++)
    {
        exps[i] = new fmpz;
        fmpz_init(exps[i]);
    }

    fmpq_mpoly_resize(a, 0, ctx);
    for (size_t i = 0; i < b.coeffs.size(); i++)
    {
        for (size_t j = 0; j < n; j++)
            fmpz_set(exps[j], b.exps[n*i + j].data);

        er bi = b.coeffs[i].get();
        if (eis_int(bi))
        {
            fmpq_mpoly_push_term_fmpz_fmpz(a, eint_data(bi), exps, ctx);
        }
        else if (eis_rat(bi))
        {
            fmpq_mpoly_push_term_fmpq_fmpz(a, erat_data(bi), exps, ctx);
        }
        else
        {
            assert(false);
            return 1;
        }
    }

    for (slong i = 0; i < n; i++)
    {
        fmpz_clear(exps[i]);
        delete exps[i];
    }
    delete[] exps;

    fmpq_mpoly_reduce(a, ctx);
    assert(fmpq_mpoly_is_canonical(a, ctx));

    return 0;
}

/* return 0 for successs, nonzero for failure */
int poly_to_nmod_mpoly(nmod_mpoly_t a, const nmod_mpoly_ctx_t ctx, const poly & b)
{
    size_t n = b.vars.size();
    assert(n == ctx->minfo->nvars);

    fmpz ** exps = new fmpz*[n];
    for (slong i = 0; i < n; i++)
    {
        exps[i] = new fmpz;
        fmpz_init(exps[i]);
    }

    nmod_mpoly_resize(a, 0, ctx);
    for (size_t i = 0; i < b.coeffs.size(); i++)
    {
        for (size_t j = 0; j < n; j++)
            fmpz_set(exps[j], b.exps[n*i + j].data);

        er bi = b.coeffs[i].get();
        ulong c;
        if (eis_int(bi))
        {
            c = fmpz_fdiv_ui(eint_data(bi), ctx->ffinfo->mod.n);
        }
        else if (eis_rat(bi))
        {
            c = nmod_div(fmpz_fdiv_ui(fmpq_numref(erat_data(bi)), ctx->ffinfo->mod.n),
                         fmpz_fdiv_ui(fmpq_denref(erat_data(bi)), ctx->ffinfo->mod.n), ctx->ffinfo->mod);
        }
        else
        {
//std::cout << "poly_to_nmod_mpoly failing" << std::endl;
            return 1;
        }

        nmod_mpoly_push_term_ui_fmpz(a, c, exps, ctx);
    }

    for (slong i = 0; i < n; i++)
    {
        fmpz_clear(exps[i]);
        delete exps[i];
    }
    delete[] exps;

    assert(nmod_mpoly_is_canonical(a, ctx));

    return 0;
}









/*****************************************************************************/

static void split_base_power(wex & base, xfmpq & power)
{
    er b = base.get();
    if (!ehas_head_sym_length(b, gs.sym_sPower.get(), 2))
        return;

    er e = echild(b, 2);

    if (eis_int(e) && fmpz_sgn(eint_data(e)) > 0)
    {
        base.reset(ecopychild(b,1));
        fmpq_mul_fmpz(power.data, power.data, eint_data(e));
        return;
    }
    else if (eis_rat(e) && fmpq_sgn(erat_data(e)) > 0)
    {
        base.reset(ecopychild(b,1));
        fmpq_mul(power.data, power.data, erat_data(e));
        return;
    }
}


/* power can be negative */
static ex split_base_intpower(er b, fmpz_t power)
{
    if (ehas_head_sym_length(b, gs.sym_sPower.get(), 2))
    {
        er e = echild(b,2);

        if (eis_int(e))
        {
            fmpz_set(power, eint_data(e));
            return ecopychild(b,1);
        }
        else if (eis_rat(e))
        {
            xfmpq q(1,1);
            fmpz_set(power, fmpq_numref(erat_data(e)));
            fmpz_set(fmpq_denref(q.data), fmpq_denref(erat_data(e)));
            return emake_node(gs.sym_sPower.copy(), ecopychild(b,1), emake_rat_move(q));
        }
        else if (ehas_head_sym(e, gs.sym_sTimes.get())
                     && elength(e) >= 2
                     && (eis_rat(echild(e,1)) || eis_int(echild(e,1))))
        {
            uex f(ecopy(e));
            if (eis_int(echild(e,1)))
            {
                fmpz_set(power, eint_data(echild(e,1)));
                f.removechild(1);
                if (elength(f.get()) == 1)
                    f.reset(f.copychild(1));
            }
            else
            {
                xfmpq q(1,1);
                fmpz_set(power, fmpq_numref(erat_data(echild(e,1))));
                fmpz_set(fmpq_denref(q.data), fmpq_denref(erat_data(echild(e,1))));
                f.replacechild(1, emake_rat_move(q));
            }
            return emake_node(gs.sym_sPower.copy(), ecopychild(b,1), f.release());
        }
    }
    fmpz_one(power);
    return ecopy(b);
}

/* power must be >= 0 */
static ex split_base_posintpower(er b, fmpz_t power)
{
    if (ehas_head_sym_length(b, gs.sym_sPower.get(), 2))
    {
        er e = echild(b,2);

        if (eis_int(e))
        {
            if (fmpz_sgn(eint_data(e)) >= 0)
            {
                fmpz_set(power, eint_data(e));
                return ecopychild(b,1);
            }
            else
            {
                fmpz_neg(power, eint_data(e));
                return emake_node(gs.sym_sPower.copy(), ecopychild(b,1), emake_cint(-1));
            }
        }
        else if (eis_rat(e))
        {
            xfmpq q(1,1);
            fmpz_set(power, fmpq_numref(erat_data(e)));
            fmpz_set(fmpq_denref(q.data), fmpq_denref(erat_data(e)));
            if (fmpz_sgn(power) < 0)
            {
                fmpz_neg(power, power);
                fmpz_set_si(fmpq_numref(q.data), -1);
            }
            return emake_node(gs.sym_sPower.copy(), ecopychild(b,1), emake_rat_move(q));
        }
        else if (ehas_head_sym(e, gs.sym_sTimes.get())
                     && elength(e) >= 2
                     && (eis_rat(echild(e,1)) || eis_int(echild(e,1))))
        {
            uex f(ecopy(e));
            if (eis_int(echild(e,1)))
            {
                fmpz_set(power, eint_data(echild(e,1)));
                if (fmpz_sgn(power) >= 0)
                {
                    f.removechild(1);
                    if (elength(f.get()) == 1)
                        f.reset(f.copychild(1));
                }
                else
                {
                    fmpz_neg(power, power);
                    f.replacechild(1, emake_cint(-1));
                }
            }
            else
            {
                xfmpq q(1,1);
                fmpz_set(power, fmpq_numref(erat_data(echild(e,1))));
                fmpz_set(fmpq_denref(q.data), fmpq_denref(erat_data(echild(e,1))));
                if (fmpz_sgn(power) < 0)
                {
                    fmpz_neg(power, power);
                    fmpz_set_si(fmpq_numref(q.data), -1);
                }
                f.replacechild(1, emake_rat_move(q));
            }
            return emake_node(gs.sym_sPower.copy(), ecopychild(b,1), f.release());
        }
    }
    fmpz_one(power);
    return ecopy(b);
}

template <class R>
static void set_map(
    std::vector<typename R::xmpoly> & map,
    std::vector<wex> & org,
    std::vector<wex> & target,
    const typename R::mpoly_ctx ctx)
{
    map.clear();
    for (size_t i = 0; i < org.size(); i++)
    {
        wex Obase(org[i].copy());
        xfmpq Opow(1, 1);
        split_base_power(Obase, Opow);

        bool found = false;
        for (size_t j = 0; j < target.size(); j++)
        {
            wex Tbase(target[j].copy());
            xfmpq Tpow(1, 1);
            split_base_power(Tbase, Tpow);

            if (ex_same(Obase.get(), Tbase.get()))
            {
                xfmpq q;
                fmpq_div(q.data, Opow.data, Tpow.data);
                if (fmpz_is_one(fmpq_denref(q.data)))
                {
					map.push_back(typename R::xmpoly(ctx));
                    R::mpoly_gen(map.back().data, j, ctx);
                    R::mpoly_pow_fmpz(map.back().data, map.back().data, fmpq_numref(q.data), ctx);
                    found = true;
                    break;
                }
            }
        }
        assert(found);
    }
    assert(map.size() == org.size());
}

static void merge_vars(
    std::vector<wex> & Avars,
    const std::vector<wex> & Bvars,
    const std::vector<wex> & Cvars)
{
    Avars = Bvars;

    for (size_t i = 0; i < Cvars.size(); i++)
    {
        wex Cbase(Cvars[i].copy());
        xfmpq Cpow(1, 1);
        split_base_power(Cbase, Cpow);

        bool found = false;
        for (size_t j = 0; j < Avars.size(); j++)
        {
            wex Abase(Avars[j].copy());
            xfmpq Apow(1,1);
            split_base_power(Abase, Apow);

            if (ex_same(Abase.get(), Cbase.get()))
            {
                xfmpq g;
                fmpq_gcd(g.data, Cpow.data, Apow.data);
                if (fmpq_is_one(g.data))
                    Avars[j].reset(Abase.copy());
                else
                    Avars[j].reset(emake_node(gs.sym_sPower.copy(), Abase.copy(), emake_rat_move(g)));
                found = true;
                break;
            }
        }

        if (!found)
        {
            Avars.push_back(wex(Cvars[i].copy()));
        }
    }
}

static bool vars_match(
    const std::vector<wex> & Bvars,
    const std::vector<wex> & Cvars)
{
	size_t n = Cvars.size();
	if (Bvars.size() != n)
		return false;

	for (size_t i = 0; i < n; i++)
	{
		if (!ex_same(Bvars[i].get(), Cvars[i].get()))
			return false;
	}

	return true;
}

enum ratpoly_error {
    rperror_ok,
    rperror_large_exp,
    rperror_inexact_coeff,
    rperror_division_by_zero,
    rperror_invalid,
    rperror_factor,
    rperror_gcd,
};

#define RATPOLY_FLAG_EXPAND 1
#define RATPOLY_FLAG_TRIG   2

class fmpq_ratpoly {
public:
    std::vector<wex> vars;
    fmpq_polyfactor data;
    fmpq_mpoly_ctx_t ctx;

    constexpr static ex (*mpoly_to_ex)(const fmpq_mpoly_t, const fmpq_mpoly_ctx_t, const std::vector<wex>) = &fmpq_mpoly_to_ex;

	constexpr static bool (*polyfactor_add_const)(fmpq_polyfactor & a, const fmpq_polyfactor & b, const fmpq_t c, const fmpq_mpoly_ctx_t ctx) = &fmpq_polyfactor_add_fmpq;
    constexpr static bool (*polyfactor_add)(fmpq_polyfactor & a, const fmpq_polyfactor & b, const fmpq_polyfactor & c, const fmpq_mpoly_ctx_t ctx) = &fmpq_polyfactor_add;
	constexpr static bool (*polyfactor_mul_const)(fmpq_polyfactor & a, const fmpq_polyfactor & b, const fmpq_t c, const fmpq_mpoly_ctx_t ctx) = &fmpq_polyfactor_mul_fmpq;
    constexpr static bool (*polyfactor_mul)(fmpq_polyfactor & a, const fmpq_polyfactor & b, const fmpq_polyfactor & c, const fmpq_mpoly_ctx_t ctx) = &fmpq_polyfactor_mul;
	constexpr static bool (*polyfactor_gcd_const)(fmpq_polyfactor & a, const fmpq_polyfactor & b, const fmpq_t c, const fmpq_mpoly_ctx_t ctx) = &fmpq_polyfactor_gcd_fmpq;
    constexpr static bool (*polyfactor_gcd)(fmpq_polyfactor & a, const fmpq_polyfactor & b, const fmpq_polyfactor & c, const fmpq_mpoly_ctx_t ctx) = &fmpq_polyfactor_gcd;
    constexpr static bool (*polyfactor_pow)(fmpq_polyfactor & a, const fmpz_t, const fmpq_mpoly_ctx_t ctx) = &fmpq_polyfactor_pow;
    constexpr static bool (*polyfactor_factorize)(fmpq_polyfactor & a, const fmpq_polyfactor & b, const fmpq_mpoly_ctx_t ctx) = &fmpq_polyfactor_factorize;
    constexpr static bool (*polyfactor_expand_numerator)(fmpq_polyfactor & a, const fmpq_polyfactor & b, const fmpq_mpoly_ctx_t ctx) = &fmpq_polyfactor_expand_numerator;

    constexpr static void (*mpoly_gen)(fmpq_mpoly_t, slong, const fmpq_mpoly_ctx_t) = &fmpq_mpoly_gen;
    constexpr static void (*mpoly_pow_fmpz)(fmpq_mpoly_t, const fmpq_mpoly_t, const fmpz_t, const fmpq_mpoly_ctx_t) = &fmpq_mpoly_pow_fmpz;

	typedef fmpq_polyfactor polyfactor;
	typedef xfmpq_mpoly xmpoly;
	typedef fmpq_mpoly_ctx_t mpoly_ctx;

    fmpq_ratpoly(int)
    {
        fmpq_mpoly_ctx_init(ctx, 1, ORD_LEX);
    }

    fmpq_ratpoly(const fmpq_ratpoly & other)
    {
        fmpq_mpoly_ctx_init(ctx, 1, ORD_LEX);
    }

    ~fmpq_ratpoly()
    {
        fmpq_mpoly_ctx_clear(ctx);
    }

    void set_ctx()
    {
        if (vars.empty())
            return;
        data.reset(ctx);
        fmpq_mpoly_ctx_clear(ctx);
        fmpq_mpoly_ctx_init(ctx, vars.size(), ORD_LEX);
    }

	void swap(fmpq_ratpoly & b)
	{
	    std::swap(vars, b.vars);
	    fmpq_polyfactor_swap(data, b.data);
	    fmpq_mpoly_ctx_struct t = *ctx;
	    *ctx = *b.ctx;
	    *b.ctx = t;
	}

    void set_fmpz(const fmpz_t a)
    {
        vars.clear();
        fmpz_set(fmpq_numref(data.sign), a);
        fmpz_one(fmpq_denref(data.sign));
        data.length = 0;
    }

    void set_fmpq(const fmpq_t a)
    {
        vars.clear();
        fmpq_set(data.sign, a);
        data.length = 0;
    }

    ex sign_to_ex() const
    {
        return emake_rat_copy(data.sign);
    }

    ex base_to_ex(slong i)
    {
        assert(i < data.length);
        return fmpq_mpoly_to_ex(data.base + i, ctx, vars);
    }

	void const_zero()
	{
        fmpq_zero(data.sign);
	}
	void const_one()
	{
        fmpq_one(data.sign);
	}
	void const_set(const fmpq_ratpoly & b)
	{
        fmpq_set(data.sign, b.data.sign);
	}
	void const_add(const fmpq_ratpoly & b, const fmpq_ratpoly & c)
	{
        fmpq_add(data.sign, b.data.sign, c.data.sign);
	}
	void const_gcd(const fmpq_ratpoly & b, const fmpq_ratpoly & c)
	{
        fmpq_gcd(data.sign, b.data.sign, c.data.sign);
	}
	void const_mul(const fmpq_ratpoly & b, const fmpq_ratpoly & c)
	{
        fmpq_mul(data.sign, b.data.sign, c.data.sign);
	}

	void map(fmpq_polyfactor & bm, fmpq_ratpoly & b)
	{
		std::vector<xfmpq_mpoly> Bmap;
		set_map<fmpq_ratpoly>(Bmap, b.vars, vars, ctx);
		fmpq_polyfactor_map(bm, ctx, b.data, b.ctx, Bmap);
	}
};

class nmod_ratpoly {
public:
    std::vector<wex> vars;
    nmod_polyfactor data;
    nmod_mpoly_ctx_t ctx;

    constexpr static ex (*mpoly_to_ex)(const nmod_mpoly_t, const nmod_mpoly_ctx_t, const std::vector<wex>) = &nmod_mpoly_to_ex;

	constexpr static bool (*polyfactor_add_const)(nmod_polyfactor & a, const nmod_polyfactor & b, mp_limb_t c, const nmod_mpoly_ctx_t ctx) = &nmod_polyfactor_add_nmod;
    constexpr static bool (*polyfactor_add)(nmod_polyfactor & a, const nmod_polyfactor & b, const nmod_polyfactor & c, const nmod_mpoly_ctx_t ctx) = &nmod_polyfactor_add;
	constexpr static bool (*polyfactor_mul_const)(nmod_polyfactor & a, const nmod_polyfactor & b, mp_limb_t c, const nmod_mpoly_ctx_t ctx) = &nmod_polyfactor_mul_nmod;
    constexpr static bool (*polyfactor_mul)(nmod_polyfactor & a, const nmod_polyfactor & b, const nmod_polyfactor & c, const nmod_mpoly_ctx_t ctx) = &nmod_polyfactor_mul;
	constexpr static bool (*polyfactor_gcd_const)(nmod_polyfactor & a, const nmod_polyfactor & b, mp_limb_t c, const nmod_mpoly_ctx_t ctx) = &nmod_polyfactor_gcd_nmod;
    constexpr static bool (*polyfactor_gcd)(nmod_polyfactor & a, const nmod_polyfactor & b, const nmod_polyfactor & c, const nmod_mpoly_ctx_t ctx) = &nmod_polyfactor_gcd;
    constexpr static bool (*polyfactor_pow)(nmod_polyfactor & a, const fmpz_t, const nmod_mpoly_ctx_t ctx) = &nmod_polyfactor_pow;
    constexpr static bool (*polyfactor_factorize)(nmod_polyfactor & a, const nmod_polyfactor & b, const nmod_mpoly_ctx_t ctx) = &nmod_polyfactor_factorize;
    constexpr static bool (*polyfactor_expand_numerator)(nmod_polyfactor & a, const nmod_polyfactor & b, const nmod_mpoly_ctx_t ctx) = &nmod_polyfactor_expand_numerator;

    constexpr static void (*mpoly_gen)(nmod_mpoly_t, slong, const nmod_mpoly_ctx_t) = &nmod_mpoly_gen;
    constexpr static void (*mpoly_pow_fmpz)(nmod_mpoly_t, const nmod_mpoly_t, const fmpz_t, const nmod_mpoly_ctx_t) = &nmod_mpoly_pow_fmpz;

	typedef nmod_polyfactor polyfactor;
	typedef xnmod_mpoly xmpoly;
	typedef nmod_mpoly_ctx_t mpoly_ctx;

    nmod_ratpoly(mp_limb_t modulus)
    {
        nmod_mpoly_ctx_init(ctx, 1, ORD_LEX, modulus);
    }

    nmod_ratpoly(const nmod_ratpoly & other)
    {
        nmod_mpoly_ctx_init(ctx, 1, ORD_LEX, other.ctx->ffinfo->mod.n);
    }

    ~nmod_ratpoly()
    {
        nmod_mpoly_ctx_clear(ctx);
    }

    void set_ctx()
    {
		mp_limb_t modulus = nmod_mpoly_ctx_modulus(ctx);
        if (vars.empty())
            return;
        data.reset(ctx);
        nmod_mpoly_ctx_clear(ctx);
        nmod_mpoly_ctx_init(ctx, vars.size(), ORD_LEX, modulus);
    }

	void swap(nmod_ratpoly & b)
	{
	    std::swap(vars, b.vars);
	    nmod_polyfactor_swap(data, b.data);
	    nmod_mpoly_ctx_struct t = *ctx;
	    *ctx = *b.ctx;
	    *b.ctx = t;
	}

    void set_fmpz(const fmpz_t a)
    {
        vars.clear();
		data.sign = fmpz_fdiv_ui(a, ctx->ffinfo->mod.n);
        data.length = 0;
    }

    void set_fmpq(const fmpq_t a)
    {
        vars.clear();
		data.sign = nmod_div(fmpz_fdiv_ui(fmpq_numref(a), ctx->ffinfo->mod.n),
							 fmpz_fdiv_ui(fmpq_denref(a), ctx->ffinfo->mod.n), ctx->ffinfo->mod);
        data.length = 0;
    }

    ex sign_to_ex() const
    {
        return emake_int_ui(data.sign);
    }

    ex base_to_ex(slong i)
    {
        assert(i < data.length);
        return nmod_mpoly_to_ex(data.base + i, ctx, vars);
    }

	void const_zero()
	{
		data.sign = 0;
	}
	void const_one()
	{
		data.sign = 1;
	}
	void const_set(const nmod_ratpoly & b)
	{
        data.sign = b.data.sign;
	}
	void const_add(const nmod_ratpoly & b, const nmod_ratpoly & c)
	{
        data.sign = nmod_add(b.data.sign, c.data.sign, ctx->ffinfo->mod);
	}
	void const_gcd(const nmod_ratpoly & b, const nmod_ratpoly & c)
	{
		data.sign = (b.data.sign != 0) || (c.data.sign != 0);
	}
	void const_mul(const nmod_ratpoly & b, const nmod_ratpoly & c)
	{
        data.sign = nmod_mul(b.data.sign, c.data.sign, ctx->ffinfo->mod);
	}

	void map(nmod_polyfactor & bm, nmod_ratpoly & b)
	{
		std::vector<xnmod_mpoly> Bmap;
		set_map<nmod_ratpoly>(Bmap, b.vars, vars, ctx);
		nmod_polyfactor_map(bm, ctx, b.data, b.ctx, Bmap);
	}
};


template <class R>
ex ratpoly_get_ex(R & p)
{
    uex r(gs.sym_sTimes.get(), p.data.length + 1);
    r.push_back(p.sign_to_ex());
    if (!p.vars.empty() && p.data.length > 0)
    {
        for (slong i = 0; i < p.data.length; i++)
        {
            uex t1(p.base_to_ex(i));
            ex t2 = emake_int_copy(p.data.exp + i);
            r.push_back(emake_node(gs.sym_sPower.copy(), t1.release(), t2));
        }
    }
    return r.release();
}

template <class R>
std::string ratpoly_tostring(R & p)
{
    wex e(ratpoly_get_ex<R>(p));
    return ex_tostring(e.get());
}

template<class R>
ratpoly_error ratpoly_add(R & a, R & b, R & c)
{
//std::cout << "ratpoly_add b: " << ratpoly_tostring<R>(b) << std::endl;
//std::cout << "            c: " << ratpoly_tostring<R>(c) << std::endl;

    if (c.vars.empty())
    {
        if (b.vars.empty())
        {
            a.vars.clear();
            a.const_add(b, c);
        }
        else
        {
            a.vars = b.vars;
            a.set_ctx();
            R::polyfactor_add_const(a.data, b.data, c.data.sign, a.ctx);
        }
    }
    else if (b.vars.empty())
    {
        a.vars = c.vars;
        a.set_ctx();
        R::polyfactor_add_const(a.data, c.data, b.data.sign, a.ctx);
    }
	else if (vars_match(b.vars, c.vars))
	{
		a.vars = b.vars;
	    a.set_ctx();
	    R::polyfactor_add(a.data, b.data, c.data, a.ctx);		
	}
	else
	{
	    merge_vars(a.vars, b.vars, c.vars);
	    a.set_ctx();
	    typename R::polyfactor bm, cm;
		a.map(bm, b);
		a.map(cm, c);
	    R::polyfactor_add(a.data, bm, cm, a.ctx);
	}

//std::cout << "ratpoly_add a: " << ratpoly_tostring<R>(a) << std::endl;
    return rperror_ok;
}

template <class R>
ratpoly_error ratpoly_gcd(R & a, R & b, R & c)
{
//std::cout << "ratpoly_gcd b: " << ratpoly_tostring<R>(b) << std::endl;
//std::cout << "            c: " << ratpoly_tostring<R>(c) << std::endl;

    if (c.vars.empty())
    {
        if (b.vars.empty())
        {
            a.vars.clear();
            a.const_gcd(b, c);
        }
        else
        {
            a.vars = b.vars;
            a.set_ctx();
            R::polyfactor_gcd_const(a.data, b.data, c.data.sign, a.ctx);
        }
    }
    else if (b.vars.empty())
    {
        a.vars = c.vars;
        a.set_ctx();
        R::polyfactor_gcd_const(a.data, c.data, b.data.sign, a.ctx);
    }
	else if (vars_match(b.vars, c.vars))
	{
		a.vars = b.vars;
	    a.set_ctx();
	    R::polyfactor_gcd(a.data, b.data, c.data, a.ctx);		
	}
	else
	{
	    merge_vars(a.vars, b.vars, c.vars);
	    a.set_ctx();
	    typename R::polyfactor bm, cm;
		a.map(bm, b);
		a.map(cm, c);
	    R::polyfactor_gcd(a.data, bm, cm, a.ctx);
	}

//std::cout << "ratpoly_gcd a: " << ratpoly_tostring<R>(a) << std::endl;
    return rperror_ok;
}

template <class R>
ratpoly_error ratpoly_mul(R & a, R & b, R & c)
{
//std::cout << "ratpoly_mul b: " << ratpoly_tostring<R>(b) << std::endl;
//std::cout << "            c: " << ratpoly_tostring<R>(c) << std::endl;

    if (c.vars.empty())
    {
        if (b.vars.empty())
        {
            a.vars.clear();
			a.const_mul(b, c);
        }
        else
        {
            a.vars = b.vars;
            a.set_ctx();
            R::polyfactor_mul_const(a.data, b.data, c.data.sign, a.ctx);
        }
    }
    else if (b.vars.empty())
    {
        a.vars = c.vars;
        a.set_ctx();
        R::polyfactor_mul_const(a.data, c.data, b.data.sign, a.ctx);
    }
	else if (vars_match(b.vars, c.vars))
	{
		a.vars = b.vars;
	    a.set_ctx();
	    R::polyfactor_mul(a.data, b.data, c.data, a.ctx);		
	}
	else
	{
	    merge_vars(a.vars, b.vars, c.vars);
	    a.set_ctx();
	    typename R::polyfactor bm, cm;
		a.map(bm, b);
		a.map(cm, c);
	    R::polyfactor_mul(a.data, bm, cm, a.ctx);
	}

//std::cout << "ratpoly_mul a: " << ratpoly_tostring<R>(a) << std::endl;
    return rperror_ok;
}

template <class R>
ratpoly_error ratpoly_pow(R & a, const fmpz_t power)
{
//std::cout << "ratpoly_pow a: " << ratpoly_tostring<R>(a) << std::endl;
    R::polyfactor_pow(a.data, power, a.ctx);
//std::cout << "ratpoly_pow a: " << ratpoly_tostring<R>(a) << std::endl;
    return rperror_ok;
}

template <class R>
ratpoly_error ratpoly_factor(R & a, R & b)
{
//std::cout << "ratpoly_factor b: " << ratpoly_tostring<R>(b) << std::endl;

    if (b.vars.empty())
	{
		a.vars.clear();
		a.const_set(b);
	}
	else
	{
		a.vars = b.vars;
	    a.set_ctx();
	    R::polyfactor_factorize(a.data, b.data, a.ctx);
	}

//std::cout << "ratpoly_factor a: " << ratpoly_tostring<R>(b) << std::endl;
    return rperror_ok;
}

template <class R>
ratpoly_error ratpoly_expand_numerator(R & a, R & b)
{
//std::cout << "ratpoly_expand_numerator b: " << ratpoly_tostring<R>(b) << std::endl;

    if (b.vars.empty())
	{
		a.vars.clear();
		a.const_set(b);
	}
	else
	{
		a.vars = b.vars;
	    a.set_ctx();
	    R::polyfactor_expand_numerator(a.data, b.data, a.ctx);
	}

//std::cout << "ratpoly_expand_numerator a: " << ratpoly_tostring<R>(a) << std::endl;
    return rperror_ok;
}


/* return 0 for success, nonzero for failure */
template <class R>
ratpoly_error ratpoly_set_ex(R & p, er e, uint32_t flags)
{
//std::cout << "ratpoly_set_ex e: " << ex_tostring_full(e) << std::endl;

    ratpoly_error r;
    if (!eis_node(e))
    {
        if (eis_number(e))
        {
            if (eis_int(e))
            {
                p.set_fmpz(eint_data(e));
                return rperror_ok;
            }
            if (eis_rat(e))
            {
                p.set_fmpq(erat_data(e));
                return rperror_ok;
            }
            return rperror_inexact_coeff;
        }
        else
        {
            p.vars.clear();
            p.vars.push_back(wex(ecopy(e)));
            p.set_ctx();
			p.const_one();
	        p.data.set_length(1, p.ctx);
	        R::mpoly_gen(p.data.base + 0, 0, p.ctx);
	        fmpz_one(p.data.exp + 0);
            return rperror_ok;
        }
    }
    else if (ehas_head_sym(e, gs.sym_sPlus.get()))
    {
        R q(p), t(p);
        p.vars.clear();
		p.const_zero();
        p.data.length = 0;
        for (size_t i = 1; i <= elength(e); i++)
        {
            r = ratpoly_set_ex<R>(q, echild(e,i), flags);
            if (r != rperror_ok)
                return r;
            r = ratpoly_add<R>(t, p, q);
            if (r != rperror_ok)
                return r;
            if ((flags & RATPOLY_FLAG_EXPAND) == 0)
                p.swap(t);
            else
            {
                r = ratpoly_expand_numerator<R>(p, t);
                if (r != rperror_ok)
                    return r;
            }
        }
        return rperror_ok;
    }
    else if (ehas_head_sym(e, gs.sym_sTimes.get()))
    {
        R q(p), t(p);
        p.vars.clear();
		p.const_one();
        p.data.length = 0;
        for (size_t i = 1; i <= elength(e); i++)
        {
            r = ratpoly_set_ex<R>(q, echild(e,i), flags);
            if (r != rperror_ok)
                return r;
            r = ratpoly_mul<R>(t, p, q);
            if (r != rperror_ok)
                return r;
            if ((flags & RATPOLY_FLAG_EXPAND) == 0)
                p.swap(t);
            else
            {
                r = ratpoly_expand_numerator<R>(p, t);
                if (r != rperror_ok)
                    return r;
            }
        }
        return rperror_ok;
    }
    else if (ehas_head_sym_length(e, gs.sym_sPower.get(), 2)
                && eis_int(echild(e,2))
                && (   (flags & RATPOLY_FLAG_EXPAND) == 0
                    || fmpz_sgn(eint_data(echild(e,2))) > 0) )
    {
        r = ratpoly_set_ex<R>(p, echild(e,1), flags);
        if (r != rperror_ok)
            return r;
        r = ratpoly_pow<R>(p, eint_data(echild(e,2)));
        if (r != rperror_ok)
            return r;
        if ((flags & RATPOLY_FLAG_EXPAND) == 0)
            return rperror_ok;
        else
        {
            R t(p);
            ratpoly_expand_numerator<R>(t, p);
            if (r != rperror_ok)
                return r;
            p.swap(t);
            return rperror_ok;
        }
    }
    else
    {
        p.vars.clear();
        p.vars.push_back(wex(gs.sym_sNull.copy()));
        p.set_ctx();
        p.const_one();
        p.data.set_length(1, p.ctx);
        R::mpoly_gen(p.data.base + 0, 0, p.ctx);
        p.vars.back() = wex(((flags & RATPOLY_FLAG_EXPAND) == 0)
                                 ? split_base_intpower(e, p.data.exp + 0)
                                 : split_base_posintpower(e, p.data.exp + 0));
//std::cout << "--------general-------" << std::endl;
//std::cout << "p.vars: " << exvec_tostring_full(p.vars) << std::endl;
//std::cout << "fmpq_ratpoly_set_any_ex returning p: " << ex_tostring_full(etor(fmpq_ratpoly_get_ex(p))) << std::endl;
        return rperror_ok;
    }
}


bool getOption(wex & optvalue, er e, size_t idx, er optname)
{
    for (size_t i = idx + 1; i <= elength(e); i++)
    {
        if (ehas_head_sym_length(echild(e,i), gs.sym_sRule.get(), 2))
        {
            if (eis_sym(echild(e,i,1), optname))
            {
                optvalue.reset(ecopychild(e,i,2));
                return true;
            }
        }
        else
        {
            _gen_message(echild(e,0), "nonopt", NULL, ecopychild(e,i), emake_int_ui(idx), ecopy(e));
            return false;
        }
    }
    return true; 
}

static ex _handle_ratpoly_error(er e, ratpoly_error r)
{
    switch (r)
    {
        case rperror_large_exp:
            _gen_message(echild(e,0), "lgexp", "Exponent is out of bounds for function `1`.", ecopychild(e,0));
            break;
        case rperror_division_by_zero:
            _gen_message(echild(e,0), "zdiv", "`1` generated a division by zero.", ecopychild(e,1));
            break;
        case rperror_invalid:
            _gen_message(echild(e,0), "invalid", "`1` is invalid.", ecopychild(e,1));
            break;
        case rperror_inexact_coeff:
            _gen_message(echild(e,0), "inexcoeff", "A coefficient of `1` is inexact.", ecopychild(e,0));
            break;
        case rperror_factor:
            _gen_message(echild(e,0), "ufac", "Unable to perform the factorization.", ecopychild(e,0));
            break;
        case rperror_gcd:
            _gen_message(echild(e,0), "ugcd", "Unable to perform the GCD.", ecopychild(e,0));
            break;
        case rperror_ok:
            break;
    }
    return ecopy(e);
}

ex dcode_sTogether(er e)
{
//std::cout << "dcode_sTogether: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sTogether.get()));
    wex optModulus(emake_cint(0));
    wex optExtension(gs.sym_sNone.copy());
    wex optTrig(gs.sym_sFalse.copy());

    if (elength(e) == 0)
    {
        return _handle_message_argx1(e);
    }

    if (   !getOption(optExtension, e, 1, gs.sym_sExtension.get())
        || !getOption(optModulus, e, 1, gs.sym_sModulus.get())
        || !getOption(optTrig, e, 1, gs.sym_sTrig.get()))
    {
        return ecopy(e);
    }

    if (eis_int(optModulus.get()))
    {
        ratpoly_error r;
        xfmpz m(eint_number(optModulus.get()));
        fmpz_abs(m.data, m.data);
        if (!fmpz_is_zero(m.data) && (!fmpz_is_probabprime(m.data) || !fmpz_is_prime(m.data)))
        {
            _gen_message(echild(e,0), "modp", NULL, gs.sym_sModulus.copy(), optModulus.copy());
            return ecopy(e);
        }

        if (fmpz_is_zero(m.data))
        {
            fmpq_ratpoly p(0);
            r = ratpoly_set_ex<fmpq_ratpoly>(p, echild(e,1), 0);
            if (r != rperror_ok)
                return _handle_ratpoly_error(e, r);
            return ratpoly_get_ex<fmpq_ratpoly>(p);
        }
        else
        {
            _gen_message(echild(e,0), "priml", "Prime `1` is too large for this implementation.");
            return ecopy(e);
        }
    }
    else
    {
        _gen_message(echild(e,0), "bmod", NULL, ecopychild(e,0));
        return ecopy(e);
    }
}

ex dcode_sCancel(er e)
{
//std::cout << "dcode_sCancel: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sCancel.get()));
    wex optModulus(emake_cint(0));
    wex optExtension(gs.sym_sNone.copy());
    wex optTrig(gs.sym_sFalse.copy());

    if (elength(e) == 0)
    {
        return _handle_message_argx1(e);
    }

    if (   !getOption(optExtension, e, 1, gs.sym_sExtension.get())
        || !getOption(optModulus, e, 1, gs.sym_sModulus.get())
        || !getOption(optTrig, e, 1, gs.sym_sTrig.get()))
    {
        return ecopy(e);
    }

    if (eis_int(optModulus.get()))
    {
        ratpoly_error r;
        xfmpz m(eint_number(optModulus.get()));
        fmpz_abs(m.data, m.data);
        if (!fmpz_is_zero(m.data) && (!fmpz_is_probabprime(m.data) || !fmpz_is_prime(m.data)))
        {
            _gen_message(echild(e,0), "modp", NULL, gs.sym_sModulus.copy(), optModulus.copy());
            return ecopy(e);
        }

        if (fmpz_is_zero(m.data))
        {
            fmpq_ratpoly p(0);
            er f = echild(e,1);
            if (ehas_head_sym(f, gs.sym_sPlus.get()))
            {
                uex s(gs.sym_sPlus.get(), elength(f));
                for (size_t i = 1; i <= elength(f); i++)
                {
                    r = ratpoly_set_ex<fmpq_ratpoly>(p, echild(f,i), 0);
                    if (r != rperror_ok)
                        return _handle_ratpoly_error(e, r);
                    s.push_back(ratpoly_get_ex<fmpq_ratpoly>(p));
                }
                return s.release();
            }
            else
            {
                r = ratpoly_set_ex<fmpq_ratpoly>(p, f, 0);
                if (r != rperror_ok)
                    return _handle_ratpoly_error(e, r);
                return ratpoly_get_ex<fmpq_ratpoly>(p);
            }
        }
        else
        {
            _gen_message(echild(e,0), "priml", "Prime `1` is too large for this implementation.");
            return ecopy(e);
        }
    }
    else
    {
        _gen_message(echild(e,0), "bmod", NULL, ecopychild(e,0));
        return ecopy(e);
    }
}


ex dcode_sExpand(er e)
{
//std::cout << "dcode_sExpand: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sExpand.get()));
    wex optModulus(emake_cint(0));
    wex optExtension(gs.sym_sNone.copy());
    wex optTrig(gs.sym_sFalse.copy());

    if (elength(e) == 0)
    {
        return _handle_message_argx1(e);
    }

    if (   !getOption(optExtension, e, 1, gs.sym_sExtension.get())
        || !getOption(optModulus, e, 1, gs.sym_sModulus.get())
        || !getOption(optTrig, e, 1, gs.sym_sTrig.get()))
    {
        return ecopy(e);
    }

    if (eis_int(optModulus.get()))
    {
        ratpoly_error r;
        xfmpz m(eint_number(optModulus.get()));
        fmpz_abs(m.data, m.data);
        if (!fmpz_is_zero(m.data) && (!fmpz_is_probabprime(m.data) || !fmpz_is_prime(m.data)))
        {
            _gen_message(echild(e,0), "modp", NULL, gs.sym_sModulus.copy(), optModulus.copy());
            return ecopy(e);
        }

        if (fmpz_is_zero(m.data))
        {
//timeit_t timer;
            fmpq_ratpoly p(0);

//timeit_start(timer);
            r = ratpoly_set_ex<fmpq_ratpoly>(p, echild(e,1), RATPOLY_FLAG_EXPAND);
//timeit_stop(timer);
//std::cout << "ratpoly_set_ex time: " << timer->wall << std::endl;

            if (r != rperror_ok)
                return _handle_ratpoly_error(e, r);

//timeit_start(timer);
            ex t = ratpoly_get_ex<fmpq_ratpoly>(p);
//timeit_stop(timer);
//std::cout << "ratpoly_get_ex time: " << timer->wall << std::endl;

            return t;
        }
        else if (fmpz_abs_fits_ui(m.data))
        {
            nmod_ratpoly p(fmpz_get_ui(m.data));
            r = ratpoly_set_ex<nmod_ratpoly>(p, echild(e,1), RATPOLY_FLAG_EXPAND);
            if (r != rperror_ok)
                return _handle_ratpoly_error(e, r);
            return ratpoly_get_ex<nmod_ratpoly>(p);
			return ecopy(e);
        }
        else
        {
            _gen_message(echild(e,0), "priml", "Prime `1` is too large for this implementation.", optModulus.copy());
            return ecopy(e);
        }
    }
    else
    {
        _gen_message(echild(e,0), "bmod", NULL, ecopychild(e,0));
        return ecopy(e);
    }
}


ex dcode_sFactorTerms(er e)
{
//std::cout << "dcode_sFactorTerms: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sFactorTerms.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    return ecopy(e);
}


ex dcode_sPolynomialGCD(er e)
{
//std::cout << "dcode_sPolynomialGCD: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sPolynomialGCD.get()));
    wex optModulus(emake_cint(0));
    wex optExtension(gs.sym_sNone.copy());
    wex optTrig(gs.sym_sFalse.copy());

    if (elength(e) == 0)
    {
        return ecopy(e);
    }
/*
    if (   !getOption(optExtension, e, 1, gs.sym_sExtension.get())
        || !getOption(optModulus, e, 1, gs.sym_sModulus.get())
        || !getOption(optTrig, e, 1, gs.sym_sTrig.get()))
    {
        return ecopy(e);
    }
*/

    size_t len = elength(e);

    if (eis_int(optModulus.get()))
    {
        ratpoly_error r;
        xfmpz m(eint_number(optModulus.get()));
        fmpz_abs(m.data, m.data);
        if (!fmpz_is_zero(m.data) && (!fmpz_is_probabprime(m.data) || !fmpz_is_prime(m.data)))
        {
            _gen_message(echild(e,0), "modp", NULL, gs.sym_sModulus.copy(), optModulus.copy());
            return ecopy(e);
        }

        if (fmpz_is_zero(m.data))
        {
            fmpq_ratpoly q(0), p(0), t(0);
            r = ratpoly_set_ex<fmpq_ratpoly>(p, echild(e,1), 0);
            if (r != 0)
                return _handle_ratpoly_error(e, r);
            for (size_t i = 2; i <= len; i++)
            {
                r = ratpoly_set_ex<fmpq_ratpoly>(q, echild(e,i), 0);
                if (r != rperror_ok)
                    return _handle_ratpoly_error(e, r);
                r = ratpoly_gcd<fmpq_ratpoly>(t, p, q);
                if (r != rperror_ok)
                    return _handle_ratpoly_error(e, r);
                p.swap(t);
            }
            return ratpoly_get_ex<fmpq_ratpoly>(p);
        }
        else
        {
            _gen_message(echild(e,0), "priml", "Prime `1` is too large for this implementation.");
            return ecopy(e);
        }
    }
    else
    {
        _gen_message(echild(e,0), "bmod", NULL, ecopychild(e,0));
        return ecopy(e);
    }
}

ex dcode_sFactor(er e)
{
//std::cout << "dcode_sFactor: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sFactor.get()));
    wex optModulus(emake_cint(0));
    wex optExtension(gs.sym_sNone.copy());
    wex optTrig(gs.sym_sFalse.copy());

    if (elength(e) == 0)
    {
        return _handle_message_argx1(e);
    }

    if (   !getOption(optExtension, e, 1, gs.sym_sExtension.get())
        || !getOption(optModulus, e, 1, gs.sym_sModulus.get())
        || !getOption(optTrig, e, 1, gs.sym_sTrig.get()))
    {
        return ecopy(e);
    }

    if (eis_int(optModulus.get()))
    {
        ratpoly_error r;
        xfmpz m(eint_number(optModulus.get()));
        fmpz_abs(m.data, m.data);
        if (!fmpz_is_zero(m.data) && (!fmpz_is_probabprime(m.data) || !fmpz_is_prime(m.data)))
        {
            _gen_message(echild(e,0), "modp", NULL, gs.sym_sModulus.copy(), optModulus.copy());
            return ecopy(e);
        }

        if (fmpz_is_zero(m.data))
        {
            fmpq_ratpoly q(0), p(0);
            r = ratpoly_set_ex<fmpq_ratpoly>(p, echild(e,1), 0);
            if (r != rperror_ok)
                return _handle_ratpoly_error(e, r);
            r = ratpoly_factor<fmpq_ratpoly>(q, p);
            if (r != rperror_ok)
                return _handle_ratpoly_error(e, r);
            return ratpoly_get_ex<fmpq_ratpoly>(q);
        }
        else if (fmpz_abs_fits_ui(m.data))
        {
            nmod_ratpoly q(fmpz_get_ui(m.data)), p(fmpz_get_ui(m.data));
            r = ratpoly_set_ex<nmod_ratpoly>(p, echild(e,1), 0);
            if (r != rperror_ok)
                return _handle_ratpoly_error(e, r);
            r = ratpoly_factor<nmod_ratpoly>(q, p);
            if (r != rperror_ok)
                return _handle_ratpoly_error(e, r);
            return ratpoly_get_ex<nmod_ratpoly>(q);
        }
        else
        {
            _gen_message(echild(e,0), "priml", "Prime `1` is too large for this implementation.");
            return ecopy(e);
        }
    }
    else
    {
        _gen_message(echild(e,0), "bmod", NULL, ecopychild(e,0));
        return ecopy(e);
    }
}

#else

ex dcode_sCancel(er e)
{
    return ecopy(e);
}

ex dcode_sDenominator(er e)
{
    return ecopy(e);
}

ex dcode_sExpand(er e)
{
    return ecopy(e);
}

ex dcode_sFactorTerms(er e)
{
    return ecopy(e);
}

ex dcode_sFactor(er e)
{
    return ecopy(e);
}

ex dcode_sNumerator(er e)
{
    return ecopy(e);
}

ex dcode_sPolynomialGCD(er e)
{
    return ecopy(e);
}

ex dcode_sTogether(er e)
{
    return ecopy(e);
}

#endif

ex dcode_sCoefficientRules(er e)
{
//std::cout << "dcode_sTogether: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sCoefficientRules.get()));

    if (elength(e) == 2 && ehas_head_sym(echild(e,2), gs.sym_sList.get()))
    {
        poly p(elength(echild(e,2)));
        if (!ex_to_polynomial(p, echild(e,1), echild(e,2)))
        {
            return ecopy(e);
        }
//        compiledpoly prog;
//        compile_poly(prog, p);
//        std::cout << "compiled: " << prog.tostring() << std::endl;
/*
        std::vector<wex> stack;
        for (size_t i = 0; i < p.nvars; i++)
        {
            stack.push_back(emake_node(gs.sym_sSlot.copy(), emake_int_ui(i+1)));
        }
        stack.push_back(wex(gs.sym_sNull.copy()));
        stack.push_back(wex(gs.sym_sNull.copy()));
        stack.push_back(wex(gs.sym_sNull.copy()));
        stack.push_back(wex(gs.sym_sNull.copy()));
        eval_poly_ex(stack, prog.prog.data(), prog.prog.size());
std::cout << "output from program: " << ex_tostring(stack[p.nvars].get()) << std::endl;
*/
        uex r(gs.sym_sList.get(), p.size());
        for (size_t i = 0; i < p.size(); i++)
        {
            uex ve(gs.sym_sList.get(), p.vars.size());
            for (size_t j = 0; j < p.vars.size(); j++)
            {
                ve.push_back(emake_int_copy(p.exps.data()[i*p.vars.size() + j].data));
            }
            r.push_back(emake_node(gs.sym_sRule.copy(), ve.release(), p.coeffs[i].copy()));
        }
        return r.release();
    }
    else
    {
        return ecopy(e);
    }
}
