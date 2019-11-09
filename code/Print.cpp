#include "globalstate.h"
#include "ex_print.h"
#include "code.h"
#include "serialize.h"

ex dcode_sPrint(er e)
{
//std::cout << "dcode_sPrint: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sPrint.get()));
    uex p(ecopy(e));
    if (elength(e) == 1)
    {
        p.reset(ecopychild(e,1));
    }
    else
    {
        p.replacechild(0, gs.sym_sSequenceForm.copy());
    }

    if (fp_out_type == FP_OUT_BINARY)
    {
        p.reset(ex_to_exbox_standard(p.get()));
        p.reset(emake_node(gs.sym_sTextPacket.copy(), p.release()));
        swrite_byte(fp_out, CMD_EXPR);
        swrite_ex(fp_out, p.get());
        fflush(fp_out);
    }
    else if (fp_out_type == FP_OUT_TEXT)
    {
        std::string s = ex_tostring(p.get());
        fwrite(s.c_str(), 1, s.length(), fp_out);
        fputc('\n', fp_out);
        fflush(fp_out);
    }

    return gs.sym_sNull.copy();
}
