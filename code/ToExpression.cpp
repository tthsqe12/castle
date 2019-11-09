#include "globalstate.h"
#include "code.h"
#include "ex_parse.h"
#include "eval.h"

ex dcode_sToExpression(er e)
{
//std::cout << "dcode_sToExpression: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sToExpression.get()));

    if (elength(e) < 1 || elength(e) > 3)
    {
        return _handle_message_argb(e, 1 + (3 << 8));
    }

    int serror = 0;
    std::vector<size_t> istack;
    std::vector<uex> vin;
    bool toplevel = elength(e) < 2 || echild(e,2) == gs.sym_sInputForm.get();
    ex_parse_exbox(vin, echild(e,1), toplevel, serror, istack);

    if (serror)
    {
        _gen_message(gs.sym_sGeneral.get(), "sntx", "Syntax error near `1`.", emake_str("???"));
        return gs.sym_s$Failed.copy();
    }
    else
    {
//std::cout << "ToExpression " << toplevel <<" parsed as " << exvec_tostring_full(vin) << std::endl;
        if (elength(e) >= 3)
        {
            return emake_node(ecopychild(e,3), vin);
        }
        else
        {
            if (vin.size() == 1)
            {
                return vin[0].copy();
            }
            else
            {
                return emake_node(gs.sym_sSequence.copy(), vin);
            }
        }
    }
}
