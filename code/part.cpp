#include "globalstate.h"
#include "code.h"
#include "ex_cont.h"

ex dcode_sPart(er e)
{
//std::cout << "dcode_sPart: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sPart.get()));

    if (elength(e) == 0)
    {
        return _handle_message_argm(e, 1);
    }

    uex r(ecopychild(e,1));
    for (size_t n = 2; n <= elength(e); n++)
    {
        er idx = echild(e, n);
        if (eis_node(r.get()))
        {
            if (!eis_machine_int(idx))
            {
                _gen_message(gs.sym_sPart.get(), "pkspec1", NULL, ecopy(idx));
                return ecopy(e);
            }
            size_t i = eget_machine_int(idx);
            if (i > elength(r.get()))
            {
                _gen_message(gs.sym_sPart.get(), "partw", NULL, ecopy(idx), r.copy());
                return ecopy(e);
            }
            r.setnz(r.copychild(i));
        }
        else if (eis_leaf(r.get()))
        {
            _gen_message(gs.sym_sPart.get(), "partd", NULL, ecopy(e));
            return ecopy(e);
        }
        else if (eis_hmap(r.get()))
        {
            if (ehas_head_sym_length(idx, gs.sym_sKey.get(), 1))
            {
                wex key(ecopychild(idx,1));
                auto it = ehmap_data(r.get()).find(key);
                if (it == ehmap_data(r.get()).end())
                {
                    ex t = emake_str("KeyAbsent");
                    return emake_node(gs.sym_sMissing.copy(), t, key.copy());
                }
                else
                {
                    r.setnz(it->second.second.copy());
                }
            }
            else
            {
                if (!eis_machine_int(idx))
                {
                    _gen_message(gs.sym_sPart.get(), "pkspec1", NULL, ecopy(idx));
                    return ecopy(e);
                }
                size_t i = eget_machine_int(idx) - 1;
                if (i >= ehmap_data(r.get()).size())
                {
                    _gen_message(gs.sym_sPart.get(), "partw", NULL, ecopy(idx), r.copy());
                    return ecopy(e);
                }
                for (std::pair<wex, std::pair<size_t, wex>> it : ehmap_data(r.get()))
                {
                    if (i == it.second.first/2)
                    {
                        r.setnz(it.second.second.copy());
                        break;
                    }
                }
            }
        }
        else
        {
            _gen_message(gs.sym_sPart.get(), "partd", NULL, ecopy(e));
            return ecopy(e);                
        }
    } 

    return r.release();
}

ex dcode_sFirst(er e)
{
//std::cout << "dcode_sFirst: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sFirst.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    er X = echild(e,1);
    if (eis_node(X) && elength(X) > 0)
    {
        return ecopychild(X,1);
    }
    else
    {
        return ecopy(e);
    }
}

ex dcode_sLast(er e)
{
//std::cout << "dcode_sLast: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sLast.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    er X = echild(e,1);
    if (eis_node(X) && elength(X) > 0)
    {
        return ecopychild(X,elength(X));
    }
    else
    {
        return ecopy(e);
    }
}
