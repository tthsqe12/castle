#include "globalstate.h"
#include "code.h"
#include "ex_cont.h"

ex dcode_devPackedArrayQ(er e)
{
//std::cout << "dcode_sHashCode: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_devPackedArrayQ.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    return emake_boole(etype(echild(e,1)) == ETYPE_PARRAY);
}
