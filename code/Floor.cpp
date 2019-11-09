#include "globalstate.h"
#include "code.h"

ex dcode_sFloor(er e)
{
//std::cout << "dcode_sFloor: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sFloor.get()));

    return ecopy(e);
}
