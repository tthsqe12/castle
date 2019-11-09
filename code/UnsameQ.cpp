#include "globalstate.h"
#include "code.h"

ex dcode_sUnsameQ(er e)
{
//std::cout << "dcode_sUnsameQ: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sUnsameQ.get()));

    for (size_t i = 2; i <= elength(e); i++)
    {
        for (size_t j = 1; j < i; j++)
        {
            if (ex_same(echild(e,i), echild(e,j)))
            {
                return gs.sym_sFalse.copy();
            }
        }
    }

    return gs.sym_sTrue.copy();
}
