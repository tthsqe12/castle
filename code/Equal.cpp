#include "globalstate.h"
#include "code.h"
#include "arithmetic.h"

ex dcode_sEqual(er e)
{
//std::cout << "dcode_sLessEqual: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sEqual.get()));

    if (elength(e) == 2 && ex_same(echild(e,1), echild(e,2)))
    {
        return gs.sym_sTrue.copy();
    }
    else
    {
        return ecopy(e);
    }
}
