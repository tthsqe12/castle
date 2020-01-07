#include <cmath>
#include <cfloat>

#include "timing.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"
#include "flint/arith.h"

ex dcode_sPrecision(er e)
{
//std::cout << "dcode_sPrecision: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sPrecision.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    er x = echild(e,1);

    if (!eis_number(x))
        return ecopy(e);

    if (!eis_real(x))
        return gs.const_infinity.copy();

    if (arb_contains_zero(ereal_data(x)))
        return emake_cint(0);

    ex z = emake_real();
    arb_get_rad_arb(ereal_data(z), ereal_data(x));
    arb_div(ereal_data(z), ereal_data(z), ereal_data(x), 32);
    arb_mul_2exp_si(ereal_data(z), ereal_data(z), 1);
    arb_abs(ereal_data(z), ereal_data(z));
    arb_log_base_ui(ereal_data(z), ereal_data(z), 10, 32 + fmpz_bits(ARF_EXPREF(arb_midref(ereal_data(z)))));
    arb_add_error_2exp_si(ereal_data(z), -30);
    arb_neg(ereal_data(z), ereal_data(z));
    return efix_real(z);
}

ex dcode_sAccuracy(er e)
{
//std::cout << "dcode_sAccuracy: " << ex_tostring_full(e) << std::endl; 
    assert(ehas_head_sym(e, gs.sym_sAccuracy.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    er x = echild(e,1);

    if (!eis_number(x))
        return ecopy(e);

    if (!eis_real(x))
        return gs.const_infinity.copy();

    ex z = emake_real();
    arb_get_rad_arb(ereal_data(z), ereal_data(x));
    arb_mul_2exp_si(ereal_data(z), ereal_data(z), 1);
    arb_abs(ereal_data(z), ereal_data(z));
    arb_log_base_ui(ereal_data(z), ereal_data(z), 10, 32 + fmpz_bits(ARF_EXPREF(arb_midref(ereal_data(z)))));
    arb_add_error_2exp_si(ereal_data(z), -30);
    arb_neg(ereal_data(z), ereal_data(z));
    return efix_real(z);
}


ex ncode_sPi(er e, slong prec)
{
    if (!eis_sym(e, gs.sym_sPi.get()))
        return ecopy(e);

    ex z = emake_real();
    arb_const_pi(ereal_data(z), prec);
    return z;
}

ex ncode_sE(er e, slong prec)
{
    if (!eis_sym(e, gs.sym_sE.get()))
        return ecopy(e);

    ex z = emake_real();
    arb_const_e(ereal_data(z), prec);
    return z;
}

ex ncode_sEulerGamma(er e, slong prec)
{
    if (!eis_sym(e, gs.sym_sEulerGamma.get()))
        return ecopy(e);

    ex z = emake_real();
    arb_const_euler(ereal_data(z), prec);
    return z;
}

ex ncode_sCatalan(er e, slong prec)
{
    if (!eis_sym(e, gs.sym_sCatalan.get()))
        return ecopy(e);

    ex z = emake_real();
    arb_const_catalan(ereal_data(z), prec);
    return z;
}

ex ncode_sKhinchin(er e, slong prec)
{
    if (!eis_sym(e, gs.sym_sKhinchin.get()))
        return ecopy(e);

    ex z = emake_real();
    arb_const_khinchin(ereal_data(z), prec);
    return z;
}

ex ncode_sGlaisher(er e, slong prec)
{
    if (!eis_sym(e, gs.sym_sGlaisher.get()))
        return ecopy(e);

    ex z = emake_real();
    arb_const_glaisher(ereal_data(z), prec);
    return z;
}

ex ncode_sGoldenRatio(er e, slong prec)
{
    if (!eis_sym(e, gs.sym_sGoldenRatio.get()))
        return ecopy(e);

    ex z = emake_real();
    arb_sqrt_ui(ereal_data(z), 5, prec);
    arb_add_ui(ereal_data(z), ereal_data(z), 1, prec);
    arb_mul_2exp_si(ereal_data(z), ereal_data(z), -1);
    return z;
}

ex ncode_sPlus(er e, slong prec)
{
    if (!ehas_head_sym(e, gs.sym_sPlus.get()))
        return ecopy(e);

    uex f(gs.sym_sPlus.get(), elength(e));

    for (size_t i = 1; i <= elength(e); i++)
    {
        f.push_back(eval_num(echild(e,i), prec + 1));
    }

    f.setnz(dcode_sPlus(f.get()));
    return f.release();
}

ex ncode_sTimes(er e, slong prec)
{
    if (!ehas_head_sym(e, gs.sym_sTimes.get()))
        return ecopy(e);

    uex f(gs.sym_sTimes.get(), elength(e));

    for (size_t i = 1; i <= elength(e); i++)
    {
        f.push_back(eval_num(echild(e,i), prec + 1));
    }

    f.setnz(dcode_sTimes(f.get()));
    return f.release();
}

ex ncode_sPower(er e, slong prec)
{
    if (!ehas_head_sym_length(e, gs.sym_sPower.get(), 2))
        return ecopy(e);

    if (eis_sym(echild(e,1), gs.sym_sE.get()))
    {
        ex t = eval_num(echild(e,2), prec + 1);
        uex g(emake_node(gs.sym_sExp.copy(), t));
        g.setnz(dcode_sExp(g.get()));
        return g.release();
    }

    uex f(gs.sym_sPower.get(), elength(e));

    for (size_t i = 1; i <= elength(e); i++)
    {
        f.push_back(eval_num(echild(e,i), prec + 1));
    }

    f.setnz(dcode_sPower(f.get()));
    return f.release();
}



ex convert_num_to_double(ex E)
{
    uex e(E);
    er X = e.get();
    switch (etype(X))
    {
        case ETYPE_INT:
        {
            return emake_double(fmpz_get_d(eint_data(X)));
        }
        case ETYPE_RAT:
        {
            return emake_double(fmpq_get_d(erat_data(X)));
        }
        case ETYPE_REAL:
        {
            return emake_double(arf_get_d(arb_midref(ereal_data(X)), ARF_RND_NEAR));
        }
        case ETYPE_CMPLX:
        {
            uex re(convert_num_to_double(ecopy(ecmplx_real(X))));
            ex im = convert_num_to_double(ecopy(ecmplx_imag(X)));
            return emake_cmplx(re.release(), im);
        }
        default:
        {
            return e.release();
        }
    }
}


ex eval_num(er e, slong prec)
{
//std::cout << "eval_num: " << ex_tostring(e) << std::endl;

    if (eis_sym(e))
    {
        // apply ncode and nvalues of e
        if (eis_sym(e, gs.sym_sPi.get()))
            return ncode_sPi(e, prec);
        if (eis_sym(e, gs.sym_sE.get()))
            return ncode_sE(e, prec);
        if (eis_sym(e, gs.sym_sEulerGamma.get()))
            return ncode_sEulerGamma(e, prec);
        if (eis_sym(e, gs.sym_sGlaisher.get()))
            return ncode_sGlaisher(e, prec);
        if (eis_sym(e, gs.sym_sCatalan.get()))
            return ncode_sCatalan(e, prec);
        if (eis_sym(e, gs.sym_sKhinchin.get()))
            return ncode_sKhinchin(e, prec);
        if (eis_sym(e, gs.sym_sPi.get()))
            return ncode_sPi(e, prec);
        if (eis_sym(e, gs.sym_sGoldenRatio.get()))
            return ncode_sGoldenRatio(e, prec);
        return ecopy(e);        
    }

    if (!eis_node(e))
    {
        switch (etype(e))
        {
            case ETYPE_INT:
            {
                xarb z;
                arb_set_round_fmpz(z.data, eint_data(e), prec);
                z.limit_prec(prec);
                return emake_real_move(z);
            }
            case ETYPE_RAT:
            {
                xarb z;
                arb_set_fmpq(z.data, erat_data(e), prec);
                z.limit_prec(prec);
                return emake_real_move(z);
            }
            case ETYPE_REAL:
            {
                xarb z;
                arb_set(z.data, ereal_data(e));
                z.limit_prec(prec);
                return emake_real_move(z);
            }
            default:
            {
                return ecopy(e);
            }
        }
    }

    slong cur_prec = prec + 10;

    uex f;

try_again:

std::cout << "using cur_prec = " << cur_prec << " for " << ex_tostring(e) << std::endl;

    ulong len = elength(e);
    ex h = eval_num(echild(e,0), cur_prec);
    uint32_t attr = enormalattr(etor(h));
    f.reset(nullptr);
    f.init_push_back(h, len);

    if (eis_sym(h, gs.sym_sPlus.get()))
    {
        for (size_t i = 1; i <= len; i++)
            f.push_back(ecopychild(e,i));
        f.setnz(ncode_sPlus(e, cur_prec));
    }
    else if (eis_sym(h, gs.sym_sTimes.get()))
    {
        for (size_t i = 1; i <= len; i++)
            f.push_back(ecopychild(e,i));
        f.setnz(ncode_sTimes(f.get(), cur_prec));
    }
    else if (eis_sym(h, gs.sym_sPower.get()))
    {
        for (size_t i = 1; i <= len; i++)
            f.push_back(ecopychild(e,i));
        f.setnz(ncode_sPower(f.get(), cur_prec));
    }
    else
    {
        uint32_t test = ATTR_NHoldFirst;
        for (ulong i = 1; i <= len; i++)
        {
            f.push_back((attr & test) ? ecopychild(e,i) : eval_num(echild(e,i), cur_prec));
            test = ATTR_NHoldRest;
        }

        er sh = esymbolic_head(f.child(0));

        // apply ncode and nvalues of sh
        if (eis_sym(sh, gs.sym_sChampernowneNumber.get()))
            f.setnz(ncode_sChampernowneNumber(f.get(), cur_prec));
    }

    f.set(eval(f.release()));

    if (eis_number(f.get()))
    {
        if (eis_real(f.get()) && ereal_number(f.get()).prec_bits() < prec)
        {
            cur_prec += prec - ereal_number(f.get()).prec_bits();
            if (cur_prec - prec > 2000) // TODO: $MaxExtraPrecision
                return f.release();
            else
                goto try_again;
        }
        return f.release();
    }
    else
    {
        return f.release();
    }
}


ex eval_num_limit_prec(er e, double p)
{
    if (eis_real(e))
    {
        double t = 3.3219280948873623479*p + 1;
        slong n = t;
        t -= n;
        mag_t z1, z2;
        mag_init(z1);
        mag_init(z2);
        arb_get_mag_lower(z1, ereal_data(e));
        mag_set_d_lower(z2, pow(2.0, -t));
        mag_mul_2exp_si(z2, z2, -n);
        mag_mul_lower(z1, z1, z2);
        xarb z;
        arb_set(z.data, ereal_data(e));
        if (mag_cmp(z1, arb_radref(z.data)))
            mag_swap(z1, arb_radref(z.data));
        return emake_real_move(z);
    }
    else
    {
        return ecopy(e);
    }
}

ex eval_num_limit_prec(er e)
{
    return convert_num_to_double(ecopy(e));
}



ex topeval_num(er e, double p) // prec in decimal
{
    uex f(eval_num(e, p * 3.3219280948873623479 + 4));
    f.setnz(eval_num_limit_prec(f.get(), p));
    return f.release();
}

ex topeval_num(er e) // machine precision
{
    uex f(eval_num(e, 53));
    f.setnz(eval_num_limit_prec(f.get()));
    return f.release();
}



ex dcode_sN(er e)
{
    if (elength(e) == 1)
    {
        return convert_num_to_double(eval_num(echild(e,1), 53));
        
    }
    else if (elength(e) == 2)
    {
        uex p(eval_num(echild(e,2), 53));
        p.set(convert_num_to_double(p.release()));
        if (eis_double(p.get()))
        {
            return topeval_num(echild(e,1), edouble_number(p.get()));
        } 
        else
        {
            _gen_message(echild(e,0), "precbd", NULL, ecopychild(e,2));
            return ecopy(e);
        }
    }
    else
    {
        return ecopy(e);
    }
}








ex ex_add(ex a, ex b)
{
    wex t(emake_node(gs.sym_sPlus.copy(), a, b));
    return dcode_sPlus(t.get());
}

ex ex_add(er a, er b)
{
    wex t(emake_node(gs.sym_sPlus.copy(), ecopy(a), ecopy(b)));
    return dcode_sPlus(t.get());
}

ex ex_mul(ex a, ex b)
{
    wex t(emake_node(gs.sym_sTimes.copy(), a, b));
    return dcode_sTimes(t.get());
}

ex ex_mul(ex a, ex b, ex c)
{
    wex t(emake_node(gs.sym_sTimes.copy(), a, b, c));
    return dcode_sTimes(t.get());
}

ex ex_mul(ex a, ex b, ex c, ex d)
{
    wex t(emake_node(gs.sym_sTimes.copy(), a, b, c, d));
    return dcode_sTimes(t.get());
}

ex ex_mul(er a, er b)
{
    wex t(emake_node(gs.sym_sTimes.copy(), ecopy(a), ecopy(b)));
    return dcode_sTimes(t.get());
}

ex ex_mul_si(ex a, slong b)
{
    wex t(emake_node(gs.sym_sTimes.copy(), a, emake_int_si(b)));
    return dcode_sTimes(t.get());
}

ex ex_sub(ex a, ex b)
{
    wex t(emake_node(gs.sym_sMinus.copy(), a, b));
    return dcode_sMinus(t.get());
}

ex ex_sub(er a, er b)
{
    wex t(emake_node(gs.sym_sMinus.copy(), ecopy(a), ecopy(b)));
    return dcode_sMinus(t.get());
}

ex ex_div(ex a, ex b)
{
    wex t(emake_node(gs.sym_sDivide.copy(), a, b));
    return dcode_sDivide(t.get());
}

ex ex_div(er a, er b)
{
    wex t(emake_node(gs.sym_sDivide.copy(), ecopy(a), ecopy(b)));
    return dcode_sDivide(t.get());
}


ex ex_pow(ex a, ex b)
{
    wex t(emake_node(gs.sym_sPower.copy(), a, b));
    return dcode_sPower(t.get());
}

ex ex_pow(er a, er b)
{
    wex t(emake_node(gs.sym_sPower.copy(), ecopy(a), ecopy(b)));
    return dcode_sPower(t.get());
}


ex ex_floor(ex A)
{
    uex a(A);
    if (eis_int(A))
    {
        return a.release();
    }
    else if (eis_rat(A))
    {
        ex z = emake_int();
        fmpz_fdiv_q(eint_data(z), fmpq_numref(erat_data(A)), fmpq_denref(erat_data(A)));
        return z;
    }
    return gs.sym_sNull.copy();
}
