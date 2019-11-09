#include "globalstate.h"
#include "code.h"
#include "arithmetic.h"

ex dcode_sGreater(er e)
{
//std::cout << "dcode_sGreater: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sGreater.get()));

    if (elength(e) == 2 && eis_finite_number(echild(e,1))
                        && eis_finite_number(echild(e,2)))
    {
        return emake_boole(num_Less2(echild(e,2), echild(e,1)));    
    }
    else
    {
        return ecopy(e);
    }
}
