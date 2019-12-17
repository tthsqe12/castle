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

ex dcode_sLess(er e)
{
//std::cout << "dcode_sLess: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sLess.get()));

    if (elength(e) == 2 && eis_finite_number(echild(e,1))
                        && eis_finite_number(echild(e,2)))
    {
        return emake_boole(num_Less2(echild(e,1), echild(e,2)));
    }
    else
    {
        return ecopy(e);
    }
}

ex dcode_sLessEqual(er e)
{
//std::cout << "dcode_sLessEqual: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sLessEqual.get()));

    if (elength(e) == 2 && eis_finite_number(echild(e,1))
                        && eis_finite_number(echild(e,2)))
    {
        return emake_boole(num_LessEqual2(echild(e,1), echild(e,2)));
    }
    else
    {
        return ecopy(e);
    }
}

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
