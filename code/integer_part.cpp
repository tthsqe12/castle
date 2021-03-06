#include "globalstate.h"
#include "code.h"

ex dcode_sFloor(er e)
{
//std::cout << "dcode_sFloor: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sFloor.get()));

    return ecopy(e);
}

ex dcode_sMod(er e)
{
//std::cout << "dcode_sMod: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sMod.get()));

    size_t n = elength(e);
    if (n == 2)
    {
        er x = echild(e,1);
        er y = echild(e,2);
        if (eis_int(x) && eis_int(y))
        {
            if (fmpz_is_zero(eint_data(y)))
                return gs.const_indeterminate.copy();
            ex z = emake_int();
            fmpz_fdiv_r(eint_data(z), eint_data(x), eint_data(y));
            return efix_int(z);
        }
        else if (eis_int(x) && eis_rat(y))
        {
            return ecopy(e);
        }
        else if (eis_rat(x) && eis_int(y))
        {
            return ecopy(e);
        }
        else if (eis_rat(x) && eis_rat(y))
        {
            return ecopy(e);
        }
        else
        {
            return ecopy(e);
        }
    }
    else
    {
            return ecopy(e);
    }
    return ecopy(e);
}
