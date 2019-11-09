#include "globalstate.h"
#include "code.h"
#include "ex_cont.h"

ex dcode_sLength(er e)
{
//std::cout << "dcode_sLength: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sLength.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    size_t r;
    er x = echild(e,1);
    if (eis_node(x))
    {
        return emake_int_ui(elength(x));
    }
    else if (eis_cont(x))
    {
        assert(false);
        return emake_cint(0);
    }
    else
    {
        return emake_cint(0);
    }
}
