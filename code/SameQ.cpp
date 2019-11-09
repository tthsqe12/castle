#include "globalstate.h"
#include "code.h"

ex dcode_sSameQ(er e)
{
//std::cout << "dcode_sSameQ: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sSameQ.get()));

    for (size_t i = 1; i + 1 <= elength(e); i++)
    {
        if (!ex_same(echild(e,i), echild(e,i+1)))
        {
            return gs.sym_sFalse.copy();
        }
    }

    return gs.sym_sTrue.copy();
}
