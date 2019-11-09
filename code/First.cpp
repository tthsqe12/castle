#include "globalstate.h"
#include "code.h"

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
