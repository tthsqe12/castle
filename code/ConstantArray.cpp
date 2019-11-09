#include "globalstate.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"

ex dcode_sConstantArray(er e)
{
//std::cout << "dcode_sConstantArray: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sConstantArray.get()));

    if (elength(e) == 2)
    {
        er e1 = echild(e,1);
        er e2 = echild(e,2);
        if (eis_int(e2) && fmpz_fits_si(eint_data(e2)) && fmpz_sgn(eint_data(e2)) >= 0)
        {
            size_t n = fmpz_get_si(eint_data(e2));
            uex r(gs.sym_sList.get(), n);
            for (size_t i = 0; i < n; i++)
            {
                r.push_back(ecopy(e1));
            }
            return r.release();
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
}
