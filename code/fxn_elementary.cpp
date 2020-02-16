#include <cmath>
#include <cfloat>

#include "timing.h"
#include "uex.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"


ex dcode_sPlus(er EE)
{
//std::cout << "dcode_sPlus: " << ex_tostring_full(E) << std::endl;
    assert(ehas_head_sym(EE, gs.sym_sPlus.get()));

    size_t en = elength(EE);

    if (en <= 1)
        return (en == 0) ? emake_cint(0) : ecopychild(EE,1);

    if (eis_number(echild(EE,1)))
    {
        if (en == 2 && eis_number(echild(EE,2)))
            return num_Plus2(echild(EE,1), echild(EE,2));
    }

    uex e(ecopy(EE));
//std::cout << " 1 e: " << e.get() << std::endl;
    e.reset(eflatten_sym(e.release(), gs.sym_sPlus.get()));
    e.reset(ex_sort(e.release()));
//std::cout << "  sorted e: " << ex_tostring(e.get()) << std::endl;

    std::vector<uex> f;
    bool changed = false;
    er E = e.get();
    en = elength(e.get());
    for (size_t ei=1; ei<=en; ei++) {
//std::cout << "looking at: " << ex_tostring_full(echild(E,ei)) << std::endl;
//std::cout << "f: " << exvector_tostring_full(f) << std::endl;

        if (f.empty()) {goto no_match;}

    // both nontimes
        if ( !ehas_head_sym(f.back().get(), gs.sym_sTimes.get())
           &&!ehas_head_sym(echild(E,ei), gs.sym_sTimes.get())) {

            if (eis_number(f.back().get()) && eis_number(echild(E,ei)))
            {
                uex sum(num_Plus2(f.back().get(), echild(E,ei)));
                f.pop_back();
                if (!eis_int(sum.get(), 0))
                    f.push_back(std::move(sum));
                changed = true;
                continue;
            }
            else if (ex_same(f.back().get(), echild(E,ei)))
            {
                f.back().reset(emake_node(gs.sym_sTimes.copy(), emake_cint(2), ecopychild(E,ei)));
                changed = true;
                continue;
            }

            goto no_match;

    // both times
        }
        else if ( ehas_head_sym(f.back().get(), gs.sym_sTimes.get())
               && ehas_head_sym(echild(E,ei), gs.sym_sTimes.get()))
        {

            er x1 = eget_cint(1), x2 = eget_cint(1);
            size_t n1 = elength(f.back().get()), n2 = elength(echild(E,ei));

            size_t k1=1, k2=1;
            if (k1<=n1 && eis_number(echild(f.back().get(),k1))) {x1=echild(f.back().get(),k1); k1++;}
            if (k2<=n2 && eis_number(echild(E,ei,k2))) {x2=echild(E,ei,k2); k2++;}

            if (n1-k1!=n2-k2) {goto no_match;}
            for (; k1<=n1; k1++,k2++) {
                if (!ex_same(echild(f.back().get(),k1), echild(E,ei,k2))) {goto no_match;};
            }

            uex sum(num_Plus2(x1, x2));
            if (eis_int(sum.get(), 0))
            {
                f.pop_back();
            }
            else if (eis_int(sum.get(), 1))
            {
                if (x1 != eget_cint(1))
                {
                    f.back().removechild(1);
                    if (elength(f.back().get())==1)
                    {
                        f.back().reset(ecopychild(f.back().get(),1));
                    }
                }
            } else {
                if (x1 != eget_cint(1)) {
                    f.back().replacechild(1, sum.release());
                } else {
                    f.back().insertchild(1, sum.release());
                }
            }
            changed = true;
            continue;

    // out nontimes, ei times
        }
        else if (   !ehas_head_sym(f.back().get(), gs.sym_sTimes.get())
                    && ehas_head_sym_length(echild(E,ei), gs.sym_sTimes.get(), 2)
                    && eis_number(echild(E,ei,1))
                    && ex_same(f.back().get(), echild(E,ei,2))         )
        {
            uex sum(num_Plus2(echild(E,ei,1), eget_cint(1)));
            f.pop_back();
            if (!eis_int(sum.get(), 0))
            {
                f.push_back(uex(emake_node(ecopy(gs.sym_sTimes.get()), sum.release(), ecopychild(E,ei,2))));
            }
            changed = true;
            continue;

    // out times, ei nontimes
        }
        else if (   !ehas_head_sym(echild(E,ei), gs.sym_sTimes.get())
                    && ehas_head_sym_length(f.back().get(), gs.sym_sTimes.get(), 2)
                    && eis_number(echild(f.back().get(),1))
                    && ex_same(echild(E,ei), echild(f.back().get(),2))    )
        {
            uex sum(num_Plus2(echild(f.back().get(),1), eget_cint(1)));
            f.pop_back();
            if (!eis_int(sum.get(), 0))
            {
                f.push_back(uex(emake_node(gs.sym_sTimes.copy(), sum.release(), ecopychild(E,ei))));
            }
            changed = true;
            continue;
        }

no_match:

        if (!eis_int(echild(E,ei), 0))
        {
            f.push_back(uex(ecopychild(E,ei)));
        }
        else
        {
            changed = true;
        }
    }

//std::cout << "changed: " << changed << std::endl;

    if (changed)
    {
        e.reset(emake_node(ecopy(gs.sym_sPlus.get()), f));
    }

    if (elength(e.get())<=1)
    {
        e.reset(elength(e.get())==1 ? ecopychild(e.get(),1) : emake_cint(0));
    }

//std::cout << "returning dcode_sPlus: " << ex_tostring_full(e.get()) << std::endl;
    return e.release();
}






ex dcode_sTimes(er EE)
{
//std::cout << "dcode_sTimes: " << ex_tostring_full(EE) << std::endl;
    assert(ehas_head_sym(EE, gs.sym_sTimes.get()));

    size_t en = elength(EE);

    if (en <= 1)
        return en == 0 ? emake_cint(1) : ecopychild(EE,1);    // Times[] -> 1, Times[a] -> a

    if (eis_number(echild(EE,1)) && eis_number(echild(EE,2)))
    {
        if (en >= 3)
        {
            if (eis_number(echild(EE,3)))
            {
                if (en >= 4)
                {
                    if (en == 4 && eis_number(echild(EE,4)))
                    {
                        uex t1(num_Times2(echild(EE, 1), echild(EE, 2)));
                        uex t2(num_Times2(echild(EE, 3), echild(EE, 4)));
                        return num_Times2(t1.get(), t2.get());
                    }
                }
                else
                {
                    uex t1(num_Times2(echild(EE,1), echild(EE,2)));
                    return num_Times2(t1.get(), echild(EE, 3));
                }
            }
        }
        else
        {
            return num_Times2(echild(EE,1), echild(EE,2));
        }
    }

    uex e(ecopy(EE));

    e.reset(eflatten_sym(e.release(), gs.sym_sTimes.get()));
    e.reset(ex_sort(e.release()));

    std::vector<uex> f;
    bool changed = false;
    er E = e.get();
    en = elength(e.get());
    for (size_t ei=1; ei<=en; ei++)
    {
//std::cout << "looking at: " << ex_tostring_full(echild(E,ei)) << std::endl;
//std::cout << "f: " << exvector_tostring_full(f) << std::endl;

        if (f.empty()) {goto no_match;}

    // both nonpower
        if ( !ehas_head_sym(f.back().get(), gs.sym_sPower.get())
           &&!ehas_head_sym(echild(E,ei), gs.sym_sPower.get()))
        {
            if (eis_number(f.back().get()) && eis_number(echild(E,ei)))
            {
                uex t(num_Times2(f.back().get(), echild(E,ei)));
                f.pop_back();
                if (!eis_int(t.get(), 1))
                    f.push_back(std::move(t));
                changed = true;
                continue;
            }
            else if (ex_same(f.back().get(), echild(E,ei)))
            {
                f.back().reset(emake_node(gs.sym_sPower.copy(), ecopychild(E,ei), emake_cint(2)));
                changed = true;
                continue;
            }

            goto no_match;

    // both power
        }
        else if ( ehas_head_sym_length(f.back().get(), gs.sym_sPower.get(), 2)
                 && ehas_head_sym_length(echild(E,ei), gs.sym_sPower.get(), 2)
                 && ex_same(echild(f.back().get(),1), echild(E,ei,1))        )
        {
            uex t(ex_add(ecopychild(f.back().get(),2), ecopychild(E,ei,2)));
            f.pop_back();
            if (eis_int(t.get(), 0))
            {
            }
            else if (eis_int(t.get(), 1))
            {
                f.push_back(uex(ecopy(echild(E,ei,1))));
            }
            else
            {
                f.push_back(uex(emake_node(gs.sym_sPower.copy(), ecopychild(E,ei,1), t.release())));
            }
            changed = true;
            continue;

    // out nonpower, ei power
        }
        else if (   !ehas_head_sym(f.back().get(), gs.sym_sPower.get())
                    && ehas_head_sym_length(echild(E,ei), gs.sym_sPower.get(), 2)
                    && ex_same(f.back().get(), echild(E,ei,1))          )
        {
            uex t(ex_add(ecopychild(E,ei,2), emake_cint(1)));
            f.pop_back();
            if (!eis_int(t.get(), 0))
                f.push_back(uex(emake_node(ecopy(gs.sym_sPower.get()), ecopychild(E,ei,1), t.release())));
            changed = true;
            continue;

    // out times, ei nontimes
        } else if (   !ehas_head_sym(echild(E,ei), gs.sym_sPower.get())
                    && ehas_head_sym_length(f.back().get(), gs.sym_sPower.get(), 2)
                    && ex_same(echild(E,ei), echild(f.back().get(),1))      )
        {
            uex t(ex_add(ecopychild(f.back().get(),2), emake_cint(1)));
            f.pop_back();
            if (!eis_int(t.get(), 0))
            {
                f.push_back(uex(emake_node(ecopy(gs.sym_sPower.get()), ecopychild(E,ei), t.release())));
            }
            changed = true;
            continue;
        }

no_match:

        if (!eis_int(echild(E,ei), 1))
        {
            f.push_back(uex(ecopychild(E,ei)));
        }
        else
        {
            changed = true;
        }
    }

    if (changed) {
        e.reset(emake_node(gs.sym_sTimes.copy(), f));
    }
    if (elength(e.get())<=1) {
        e.reset(elength(e.get())==1 ? ecopychild(e.get(),1) : emake_cint(1));
    } else if (eis_int(echild(e.get(),1), 0)) {
        e.reset(emake_cint(0));
    }

//std::cout << "returning dcode_sTimes: " << ex_tostring_full(e.get()) << std::endl;
    return e.release();
}


ex echange_sign(er E)
{
    uex e(ecopy(E));
//printf("entering eval_change_sign: "); expr_printout(e);printf("\n");

    if (eis_number(e.get())) {
        return num_Minus1(e.get());
    }

    if (!eis_leaf(e.get())) {
        if (elength(e.get())>0 && ehas_head_sym(e.get(), gs.sym_sTimes.get())) {
            if (eis_number(echild(e.get(),1))) {
                e.replacechild(1, echange_sign(echild(e.get(),1)));
                if (eis_int(echild(e.get(),1), 1)) {
                    e.removechild(1);
                    if (elength(e.get())==1) {
                        return ecopychild(e.get(), 1);
                    }
                }
            } else {
                e.insertchild(1, emake_cint(-1));
            }
            return e.release();

        } else if (ehas_head_sym(e.get(), gs.sym_sPlus.get())) {
            for (size_t i=1; i<=elength(e.get()); i++) {
                e.replacechild(i, echange_sign(echild(e.get(),i)));
            }
            return e.release();
        }
    }

    return emake_node(ecopy(gs.sym_sTimes.get()), emake_cint(-1), e.release());
}

ex dcode_sMinus(er EE)
{
//std::cout << "dcode_sMinus: " << ex_tostring_full(EE) << std::endl;
    assert(ehas_head_sym(EE, gs.sym_sMinus.get()));

    size_t n = elength(EE);
    if (n <= 1)
        return (n == 0) ? ecopy(EE) : echange_sign(echild(EE,1));

    if (n == 2 && eis_number(echild(EE,1)) && eis_number(echild(EE,2)))
        return num_Minus2(echild(EE,1), echild(EE,2));

    uex e(ecopy(EE));
    e.replacechild(0, ecopy(gs.sym_sPlus.get()));
    for (size_t i=2; i<=n; i++) {
        e.replacechild(i, echange_sign(echild(e.get(),i)));
    }
    return dcode_sPlus(e.get());
}


ex etake_reciprocal(ex E)
{
    uex e(E);
//std::cout << "etake_reciprocal: " << ex_tostring_full(e.get()) << std::endl;

    if (eis_number(e.get())) {
        return num_Divide1(e.get());
    }

    if (!eis_leaf(e.get())) {
        if (elength(e.get())==2 && ehas_head_sym(e.get(), gs.sym_sPower.get())) {
            e.replacechild(2, echange_sign(echild(e.get(),2)));
            if (eis_int(echild(e.get(),2), 1)) {
                return ecopychild(e.get(), 1);
            }
            return e.release();

        } else if (ehas_head_sym(e.get(), gs.sym_sTimes.get())) {
            for (size_t i=1; i<=elength(e.get()); i++) {
                e.replacechild(i, etake_reciprocal(ecopychild(e.get(),i)));
            }
            return e.release();
        }
    }

    return emake_node(ecopy(gs.sym_sPower.get()), e.release(), emake_cint(-1));
}

ex dcode_sDivide(er EE)
{
//std::cout << "dcode_sDivide: " << ex_tostring_full(EE) << std::endl;
    assert(ehas_head_sym(EE, gs.sym_sDivide.get()));

    size_t n = elength(EE);
    if (n <= 1)
        return (n == 0) ? ecopy(EE) : etake_reciprocal(ecopychild(EE,1));

    if (n == 2 && eis_number(echild(EE,1)) && eis_number(echild(EE,2)))
        return num_Divide2(echild(EE,1), echild(EE,2));

    uex e(ecopy(EE));

    e.replacechild(0, gs.sym_sTimes.copy());
    for (size_t i=2; i<=n; i++) {
        e.replacechild(i, etake_reciprocal(ecopychild(e.get(),i)));
    }
    return dcode_sTimes(e.get());
}


ex dcode_sPower_2(er EE)
{
    uex e(ecopy(EE));
//std::cout << "dcode_sPower: " << ex_tostring_full(e.get()) << std::endl;
    assert(ehas_head_sym(e.get(), gs.sym_sPower.get()));
    assert(elength(e.get()) == 2);

    er A = echild(e.get(),1);
    er B = echild(e.get(),2);

    if (!eis_number(B))
    {
        return e.release();
    }

    if (eis_number(A))
    {
        ex t = num_Power2(A, B);
        return ex_same(etor(t), e.get()) ? e.release() : t;
    }

    if (eis_int(B, 1))
    {
        return ecopy(A);
    }
    else if (eis_int(B, 0))
    {
        return emake_cint(1);
    }

    if (eis_int(B))
    {
        if (ehas_head_sym_length(A, gs.sym_sPower.get(), 2))
        {
            ex p = ex_mul(ecopy(B), ecopychild(A,2));
            if (eis_int(p, 1))
            {
                return ecopychild(A,1);
            }
            else
            {
                e.reset(ecopychild(A,1));
                return ex_pow(e.release(), p);
            }
        }
        else if (ehas_head_sym(A, gs.sym_sTimes.get()) && elength(A)>0)
        {
            size_t n = elength(A);
            uex a(gs.sym_sTimes.get(), n);
            for (size_t i = 1; i <= n; i++)
            {
                a.push_back(ex_pow(ecopychild(A,i), ecopy(B)));
            }
            eclear(e.release());
            return dcode_sTimes(a.get());
        }
        else
        {
            return e.release();
        }
    }
    else
    {
        return e.release();
    }
}

ex dcode_sPower(er e)
{
//std::cout << "dcode_sPower: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sPower.get()));

    if (elength(e) == 2)
    {
        return dcode_sPower_2(e);
    }
    else
    {
        ex r = emake_cint(1);
        for (size_t i = elength(e); i != 0; i--)
        {
            r = ex_pow(ecopychild(e,i), r);
        }
        return r;
    }
}


ex dcode_sSqrt(er EE)
{
    uex e(ecopy(EE));
//std::cout << "dcode_sSqrt: " << ex_tostring_full(e.get()) << std::endl;
    assert(ehas_head_sym(e.get(), gs.sym_sSqrt.get()));

    if (elength(EE) != 1)
    {
        return e.release();
    }

    er X = echild(EE,1);
    if (eis_number(X))
    {
        switch (etype(X))
        {
            case ETYPE_INT:
            {
                return emake_node(ecopy(gs.sym_sPower.get()), ecopychild(e.get(),1), emake_crat(1,2));
            }
            case ETYPE_RAT:
            {
                return emake_node(ecopy(gs.sym_sPower.get()), ecopychild(e.get(),1), emake_crat(1,2));
            }
            case ETYPE_DOUBLE:
            {
                double d = edouble_number(X);
                eclear(e.release());
                bool negative = d < 0;
                ex r = emake_double(sqrt(d < 0 ? -d : d));
                if (!negative)
                {
                    return r;
                }
                else
                {
                    return emake_cmplx(emake_cint(0), r);
                }
            }
            case ETYPE_REAL:
            {
                xarb_t z;
                slong p = ereal_number(X).wprec();
                arb_sqrt(z.data, ereal_data(X), p + EXTRA_PRECISION_BASIC);
                return emake_real_move(z);
            }
            default:
            {
                return e.release();
            }
        }
    }
    else
    {
        return emake_node(ecopy(gs.sym_sPower.get()), ecopychild(e.get(),1), emake_crat(1,2));
    }
}
