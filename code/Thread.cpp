#include "globalstate.h"
#include "code.h"

ex dcode_sThread(er e)
{
//std::cout << "dcode_sThread: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sThread.get()));

    if (elength(e) == 1)
    {
        return ecopy(e);
    }
    else
    {
        return ecopy(e);
    }
}
