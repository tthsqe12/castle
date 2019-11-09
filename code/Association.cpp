#include "types.h"
#include "ex.h"
#include "uex.h"
#include "ex_cont.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"

ex scode_sAssociation(er e)
{
//std::cout << "scode_sAssociation: " << ex_tostring_full(e) << std::endl;
    assert(eis_node(e));

    er h = echild(e,0);
    if (elength(e) != 1 || !eis_hmap(h))
    {
        return ecopy(e);
    }

    wex key(ecopychild(e,1));
    auto it = ehmap_data(h).find(key);
    if (it == ehmap_data(h).end())
    {
        ex t = emake_str("KeyAbsent");
        return emake_node(gs.sym_sMissing.copy(), t, key.copy());
    }
    else
    {
        return it->second.second.copy();
    }
}

ex dcode_sAssociation(er e)
{
//std::cout << "dcode_sAssociation: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sAssociation.get()));

    size_t n = elength(e);
    for (size_t i = 1; i <= n; i++)
    {
        er ei = echild(e,i);
        if (   !ehas_head_sym_length(ei, gs.sym_sRule.get(), 2)
            && !ehas_head_sym_length(ei, gs.sym_sRuleDelayed.get(), 2))
        {
            return ecopy(e);
        }
    }
    uex r(emake_hmap());
//std::cout << "r.get(): " << r.get() << std::endl;
//std::cout << "association returning " << ex_tostring(r.get()) << std::endl;
    for (size_t i = 1; i <= n; i++)
    {
        er ei = echild(e,i);

//std::cout << "r.get(): " << r.get() << std::endl;

        ehmap_assign(r.get(), ecopychild(ei,1), ecopychild(ei,2), eis_sym(echild(ei,0), gs.sym_sRuleDelayed.get()));
//std::cout << "r.get(): " << r.get() << std::endl;
//std::cout << "association returning " << ex_tostring(r.get()) << std::endl;
    }

//std::cout << "association returning " << ex_tostring(r.get()) << std::endl;

    return r.release();
}
