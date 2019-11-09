#include "globalstate.h"
#include "code.h"
#include "arithmetic.h"

ex dcode_sGreaterEqual(er e)
{
//std::cout << "dcode_sGreaterEqual: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sGreaterEqual.get()));

    if (elength(e) == 2 && eis_finite_number(echild(e,1))
                        && eis_finite_number(echild(e,2)))
    {
        return emake_boole(num_LessEqual2(echild(e,2), echild(e,1)));
    }
    else
    {
        return ecopy(e);
    }
}
