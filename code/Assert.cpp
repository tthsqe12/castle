#include "globalstate.h"
#include "code.h"
#include "eval.h"

ex dcode_sAssert(er e)
{
//std::cout << "dcode_sAssert: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sAssert.get()));

    bool asserts_are_on = true;

    if (!asserts_are_on)
    {
        return ecopy(e);
    }

    if (elength(e) == 0)
    {
        _gen_message(echild(e,0), "asrte", "Assertion `1` had no test.", ecopy(e));
        return ecopy(e);
    }

    try {
        wex x(eval(ecopychild(e,1)));
        if (!eis_sym(x.get(), gs.sym_sTrue.get()))
        {
            if (!eis_sym(x.get(), gs.sym_sFalse.get()))
            {
                _gen_message(echild(e,0), "asrttf", "Assertion test `1` evaluated to `2` that is neither True nor False.", ecopychild(e,1), x.copy());
            }
            _gen_message(echild(e,0), "asrtf", "Assertion `1` failed.", ecopychild(e,1));
            ex t = emake_node(gs.sym_sHoldComplete.copy(), ecopy(e));
            eclear(eval(emake_node(gs.sym_s$AssertFunction.copy(), t)));
        }
        return gs.sym_sNull.copy();
    }
    catch (const exception_stack_overflow & X)
    {
        uex x(reinterpret_cast<ex>(X.data));
        _gen_message(echild(e,0), "asrtso", "Assertion test `1` generated a stack overflow.", ecopychild(e,1));
        ex t = emake_node(gs.sym_sHoldComplete.copy(), ecopy(e));
        eclear(eval(emake_node(gs.sym_s$AssertFunction.copy(), t)));
        throw exception_stack_overflow(x.release());
        return nullptr;
    }
}
