#include "globalstate.h"
#include "code.h"

ex dcode_sSort(er e)
{
//std::cout << "dcode_sSort: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sSort.get()));

    if (elength(e) == 1 && eis_node(echild(e,1)))
    {
        return ex_sort(ecopychild(e,1));
    }
    else
    {
        return ecopy(e);
    }
}
