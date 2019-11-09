#include "globalstate.h"
#include "code.h"
#include "eval.h"

ex dcode_sIn_Out(er e)
{
//std::cout << "dcode_sIn_Out: " << ex_tostring_full(e.get()) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sIn.get()) || ehas_head_sym(e, gs.sym_sOut.get()));

    if (elength(e) == 0)
    {
        // $Line = 0; % is supposed to loop forever
        ex t = emake_node(gs.sym_sPlus.copy(), emake_cint(-1), gs.sym_s$Line.copy());
        return emake_node(ecopychild(e,0), t);
    }
    else if (elength(e) == 1)
    {
        // $Line = 0; %% is also supposed to loop forever
        er x = echild(e,1);
        if (eis_int(x) && fmpz_sgn(eint_data(x)) < 0)
        {
            ex t = emake_node(gs.sym_sPlus.copy(), ecopy(x), gs.sym_s$Line.copy());
            return emake_node(ecopychild(e,0), t);
        }
        else
        {
            return ecopy(e);
        }
    }
    else
    {
        return _handle_message_argt(e, 0 + (1 << 8));
    }
}
