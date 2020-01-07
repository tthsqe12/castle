#include "globalstate.h"
#include "code.h"

ex dcode_sExit_Quit(er e)
{
//std::cout << "dcode_sExit_Quit: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sExit.get()) || ehas_head_sym(e, gs.sym_sQuit.get()));

    if (elength(e) > 1)
    {
        return _handle_message_argt(e, 0 + (1 << 8));
    }

    if (elength(e) == 0)
    {
        throw exception_exit(0);
    }
    else
    {
        if (eis_intm(echild(e,1)))
        {
            throw exception_exit(eintm_get(echild(e,1)));
        }
        else
        {
            _gen_message(echild(e,0), "intm", NULL, emake_cint(1), ecopy(e));
        }
    }

    return ecopy(e);
}
