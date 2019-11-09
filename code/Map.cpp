#include "uex.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"

static ex _dcode_map_pp(er f, er e, slong a, slong b)
{
    assert(0 <= a);
    assert(a <= b);
    if (a == 0)
    {
        if (eis_node(e) && b > 0)
        {
            std::vector<uex> v;
            for (size_t i = 1; i <= elength(e); i++)
            {
                v.push_back(uex(_dcode_map_pp(f, echild(e,i), a, b - 1)));
            }
            return emake_node(ecopy(f), v);
        }
        else
        {
            return emake_node(ecopy(f), ecopy(e));
        }
    }
    else
    {
        if (eis_node(e))
        {
            std::vector<uex> v;
            for (size_t i = 1; i <= elength(e); i++)
            {
                v.push_back(uex(_dcode_map_pp(f, echild(e,i), a - 1, b - 1)));
            }
            ex ne = emake_node(ecopychild(e,0), v);
            return emake_node(ecopy(f), ne);
        }
        else
        {
            return ecopy(e);
        }
    }
}

static ex _dcode_map_gen(er f, er e, slong & edepth, slong elevel, slong a, slong b)
{
    ex ne;
    if (eis_node(e))
    {
        std::vector<uex> v;
        slong child_depth;
        edepth = 2;
        for (size_t i = 1; i <= elength(e); i++)
        {
            child_depth = 0;
            v.push_back(uex(_dcode_map_gen(f, echild(e,i), child_depth, elevel + 1, a, b)));
            assert(child_depth > 0);
            edepth = std::max(edepth, 1 + child_depth);
        }
        ne = emake_node(ecopychild(e,0), v);
    }
    else
    {
        edepth = 1;
        ne = ecopy(e);
    }
    if (   (a >= 0 ? elevel >= a : edepth <= -a)
        && (b >= 0 ? elevel <= b : edepth >= -b))
    {
        return emake_node(ecopy(f), ne);
    }
    else
    {
        return ne;
    }
}

ex dcode_sMap(er e)
{
//std::cout << "dcode_sMap: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sMap.get()));

    if (elength(e) == 2)
    {
        slong edepth;
        return eval(_dcode_map_gen(echild(e,1), echild(e,2), edepth, 0, 1, 1));
    }
    else if (elength(e) == 3)
    {
        slong edepth;
        er l = echild(e, 3);
        if (ehas_head_sym(l, gs.sym_sList.get()))
        {
            if (elength(l) == 1 && fmpz_fits_si(eint_data(echild(l,1))))
            {
                slong n = fmpz_get_si(eint_data(echild(l,1)));
                return eval(_dcode_map_gen(echild(e,1), echild(e,2), edepth, 0, n, n));
            }
            else if (elength(l) == 2 && fmpz_fits_si(eint_data(echild(l,1)))
                                     && fmpz_fits_si(eint_data(echild(l,2))))
            {
                slong n1 = fmpz_get_si(eint_data(echild(l,1)));
                slong n2 = fmpz_get_si(eint_data(echild(l,2)));
                return eval(_dcode_map_gen(echild(e,1), echild(e,2), edepth, 0, n1, n2));
            }
            else
            {
                return ecopy(e);
            }
        }
        else if (eis_int(l) && fmpz_fits_si(eint_data(l)))
        {
            slong n = fmpz_get_si(eint_data(l));
            return eval(_dcode_map_gen(echild(e,1), echild(e,2), edepth, 0, 1, n));
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
