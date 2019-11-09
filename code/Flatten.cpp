#include "globalstate.h"
#include "code.h"

size_t eflatten_length(er e, er head)
{
    assert(ehas_head_sym(e, head));
    size_t total = 0;
    for (size_t i = elength(e); i > 0; i--)
    {
        er X = echild(e,i);
        if (ehas_head_sym(X, head))
        {
            total += eflatten_length(X, head);
        }
        else
        {
            total++;
        }
    }
    return total;
}

void eflatten_helper(uex&p, er e, er head)
{
//std::cout<<"helper"<<std::endl;
//std::cout<<"e: " << ex_tostring_full(reinterpret_cast<ex>(e)) << "  k: " << k << std::endl;
    size_t en = elength(e);
    for (size_t i = 1; i <= en; i++)
    {
        er X = echild(e,i);
        if (ehas_head_sym(X, head))
        {
            eflatten_helper(p, X, head);
        }
        else
        {
            p.push_back(ecopy(X));
        }
    }
}

ex eflatten_sym(ex E, er head) {
//printf("entering eval_flatten_sym p: "); expr_printout_full(p); printf("\n   E :"); expr_printout_full(E); printf("\n");
    assert(eis_sym(head));
    assert(ehas_head_sym(E, head));
    size_t fn = eflatten_length(etor(E), head);
    size_t en = elength(E);
//std::cout << "fn: " << fn << "  en: " << en << std::endl;
    if (fn == en)
    {
        return E;
    }
    uex e(E);
    uex p(head, fn);
    eflatten_helper(p, e.get(), head);
    return p.release();
}

ex dcode_sFlatten(er e)
{
//std::cout << "dcode_sFlatten: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sFlatten.get()));

    if (elength(e) == 1)
    {
        e = echild(e,1);
        if (eis_node(e) && eis_sym(echild(e,0)))
        {
            er h = echild(e, 0);
            return eflatten_sym(ecopy(e), h);
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


ex dcode_sJoin(er e)
{
//std::cout << "dcode_sJoin: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sJoin.get()));

    size_t n = elength(e);
    size_t tot = 0;
    er h = gs.sym_sList.get();

    for (size_t i = 1; i <= n; i++)
    {
        er ei = echild(e,i);
        if (!eis_node(ei))
        {
            return ecopy(e);
        }

        tot += elength(ei);

        if (i == 1)
        {
            h = echild(ei,0);
        }
        else
        {
            if (!ex_same(h, echild(ei,0)))
            {
                return ecopy(e);
            }
        }
    }

    uex r(h, tot);
    for (size_t i = 1; i <= n; i++)
    {
        er ei = echild(e,i);
        for (size_t j = 1; j <= elength(ei); j++)
        {
            r.push_back(ecopychild(ei,j));
        }
    }

    assert(elength(r.get()) == tot);

    return r.release();
}

ex dcode_sUnion(er e)
{
//std::cout << "dcode_sUnion: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sUnion.get()));

    size_t n = elength(e);
    size_t tot = 0;
    er h = gs.sym_sList.get();

    for (size_t i = 1; i <= n; i++)
    {
        er ei = echild(e,i);
        if (!eis_node(ei))
        {
            return ecopy(e);
        }

        tot += elength(ei);

        if (i == 1)
        {
            h = echild(ei,0);
        }
        else
        {
            if (!ex_same(h, echild(ei,0)))
            {
                return ecopy(e);
            }
        }
    }

    uex r(h, tot);
    for (size_t i = 1; i <= n; i++)
    {
        er ei = echild(e,i);
        for (size_t j = 1; j <= elength(ei); j++)
        {
            r.push_back(ecopychild(ei,j));
        }
    }

    r.set(ex_sort(r.release()));

    std::vector<wex> nr;
    for (size_t i = 1; i <= tot; i++)
    {
        if (i != 1 && ex_same(r.child(i-1), r.child(i)))
            continue;

        nr.push_back(wex(r.copychild(i)));
    }

    return emake_node(ecopy(h), nr);
}
