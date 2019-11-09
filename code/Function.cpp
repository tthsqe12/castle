#include "uex.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"

static bool _apply_function1(std::vector<uex>&out, er b, er f, er e)
{
//std::cout << "_apply_function1 called: " << ex_tostring(b) << std::endl;
    if (eis_leaf(b))
    {
        out.push_back(uex(ecopy(b)));
        return true;
    }
    else
    {
        if (ehas_head_sym_length(b, gs.sym_sSlot.get(), 1) && eis_int(echild(b,1)))
        {
            if (fmpz_fits_si(eint_data(echild(b,1))))
            {
                slong i = fmpz_get_si(eint_data(echild(b,1)));
                if (0 <= i && i <= elength(e))
                {
                    if (i == 0)
                    {
                        out.push_back(uex(ecopy(f)));
                    }
                    else
                    {
                        out.push_back(uex(ecopychild(e,i)));
                    }
                    return true;
                }
            }
            return false;
        }
        else if (ehas_head_sym_length(b, gs.sym_sSlotSequence.get(), 1) && eis_int(echild(b,1)))
        {
            if (fmpz_fits_si(eint_data(echild(b,1))))
            {
                slong i = fmpz_get_si(eint_data(echild(b,1)));
                if (0 < i && i <= elength(e))
                {
                    while (i <= elength(e))
                    {
                        out.push_back(uex(ecopychild(e,i)));
                        i++;
                    }
                    return true;
                }
            }
            return false;
        }
        else if (ehas_head_sym(b, gs.sym_sFunction.get()))
        {
            out.push_back(uex(ecopy(b)));
            return true;
        }
        else
        {
            std::vector<uex> v;
            bool ret = _apply_function1(v, echild(b,0), f, e);

            if (!ret)
                return false;

            uex h;
            if (v.size() == 1) {
                h.reset(ecopy(v[0].get()));
            } else {
                h.reset(emake_node(gs.sym_sSequence.copy(), v));
            }
            v.clear();
            for (size_t i=1; i<=elength(b); i++)
            {
                ret = _apply_function1(v, echild(b,i), f, e);

                if (!ret)
                    return false;
            }
            out.push_back(uex(emake_node(h.release(), v)));
            return true;
        }
    }
}

ex apply_function1(ex F, ex E)
{
    uex f(F), e(E);
//std::cout << "apply_function1 called: " << ex_tostring(f.get()) << ", " << ex_tostring(e.get()) << std::endl;

    assert(ehas_head_sym_length(f.get(), gs.sym_sFunction.get(), 1));
    assert(eis_node(e.get()));
    std::vector<uex> v;
    bool ret = _apply_function1(v, echild(f.get(),1), f.get(), e.get());
    if (ret)
    {
        if (v.size() == 1)
        {
            return ecopy(v[0].get());
        }
        else
        {
            return emake_node(gs.sym_sSequence.copy(), v);
        }
    }
    else
    {
        e.replacechild(0, f.release());
        return e.release();
    }
}



ex _apply_function2(er b, er l, er e)
{
//std::cout << "_apply_function2 called: " << ex_tostring_full(b) << ", " << ex_tostring_full(l) << ", " << ex_tostring_full(e) << std::endl;
    if (eis_leaf(b))
    {
        if (eis_sym(b))
        {
//std::cout << "_apply_function2 b is sym l: " << ex_tostring(l) << std::endl;
            for (size_t i = 1; i <= elength(l); i++)
            {
                if (b == echild(l,i))
                    return ecopychild(e,i);
            }
        }
        return ecopy(b);
    }
    else
    {
        if (ehas_head_sym(b, gs.sym_sFunction.get()))
        {
            return ecopy(b);
        }
        else
        {
            uex head(_apply_function2(echild(b,0), l, e));
            uex v(head.get(), elength(b));
            eclear(head.release());
            for (size_t i = 1; i <= elength(b); i++)
            {
                v.push_back(_apply_function2(echild(b,i), l, e));
            }
            return v.release();
        }
    }
}


ex apply_function2(ex F, ex E)
{
    uex f(F), e(E);
//std::cout << "apply_function1 called: " << ex_tostring(f.get()) << ", " << ex_tostring(e.get()) << std::endl;

    assert(ehas_head_sym_length(f.get(), gs.sym_sFunction.get(), 2));
    assert(eis_node(e.get()));

    assert(ehas_head_sym(echild(f.get(),1), gs.sym_sList.get()));
    assert(elength(echild(f.get(),1)) == elength(e.get()));

    return _apply_function2(echild(f.get(),2), echild(f.get(),1), e.get());
}


ex scode_sFunction(er e)
{
//std::cout << "scode_sFunction: " << ex_tostring_full(e) << std::endl;
    assert(eis_node(e));

    er h = echild(e,0);

    if (ehas_head_sym_length(h, gs.sym_sFunction.get(), 1))
    {
        return eval(apply_function1(ecopy(h), ecopy(e)));
    }
    else if (ehas_head_sym_length(h, gs.sym_sFunction.get(), 2))
    {
        if (ehas_head_sym_length(echild(h,1), gs.sym_sList.get(), elength(e)))
        {
            return apply_function2(ecopy(h), ecopy(e));
        }
        else
        {
            return ecopy(e);
        }
    }
    else
    {
        return ecopy(e);
    }
}
