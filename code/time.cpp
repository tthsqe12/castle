#include "timing.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"

ex dcode_sTiming(er e)
{
//std::cout << "dcode_sTiming: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sTiming.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    ulong start = GetUS();
    uex r(eval(ecopychild(e,1)));
    ex t = emake_double(static_cast<double>(GetUS()-start)/1000000);
    return emake_node(gs.sym_sList.copy(), t, r.release());
}

ex dcode_sPause(er e)
{
//std::cout << "dcode_sPause: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sPause.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    double t = econvert_todouble(echild(e,1));
    if (std::isfinite(t) && t >= 0)
    {
        SleepUS(t*1000000);
        return gs.sym_sNull.copy();
    }
    else
    {
        return ecopy(e);
    }
}
