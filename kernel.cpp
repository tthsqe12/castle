#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <string.h>
#include <pthread.h>

#include "flintarb_wrappers.h"

#include "types.h"
#include "ex.h"
#include "globalstate.h"
#include "ex_parse.h"
#include "ex_parse_string.h"
#include "ex_print.h"

#include "eval.h"
#include "hash.h"

#include "timing.h"
#include "digits.h"
#include "serialize.h"
#include "sudcode.h"


std::vector<void*> exs_in_use;
globalstate gs;

#define READ_END 0
#define WRITE_END 1
FILE * fp_out, * fp_in;
int fp_out_type;


int main(int argc, char *argv[])
{
//    printf("kernel started with fd_out = %s, fd_in = %s\n", argv[1], argv[2]);
    int fdout = atoi(argv[1]);
    fp_out = fdopen(fdout, "wb");
    int fdin  = atoi(argv[2]);
    fp_in = fdopen(fdin, "rb");
    fp_out_type = FP_OUT_BINARY;

    gs.inc_$Line();
    uex m(emake_node(gs.sym_sInputNamePacket.copy(), gs.in_prompt_standardform()));
    swrite_byte(fp_out, CMD_EXPR);
    swrite_ex(fp_out, m.get());
    fflush(fp_out);
    gs.send_alive_symbols();

    while (1)
    {
        uint8_t cmd;
        if (0 != sread_byte(fp_in, cmd))
        {
std::cout << "<kernel>: could not read" << std::endl;
            break;
        }

        if (cmd == CMD_EXPR)
        {
            uex e;
            int err = sread_ex(fp_in, e);
//std::cout << "<kernel>: CMD_EXPR err: " << err << "  e: " << ex_tostring_full(e.get()) << std::endl;
            if (err != 0)
            {
                std::cerr << "<!kernel>: could not read CMD_EXPR - error " << err << std::endl;
            }
            else if (ehas_head_sym_length(e.get(), gs.sym_sEnterExpressionPacket.get(), 1))
            {
                e.reset(e.copychild(1));
                e.reset(topeval(e.release()));
                e.reset(emake_node(gs.sym_sReturnExpressionPacket.copy(), e.release()));
                swrite_byte(fp_out, CMD_EXPR);
                swrite_ex(fp_out, e.get());
                fflush(fp_out);
            }
            else if (ehas_head_sym_length(e.get(), gs.sym_sEnterTextPacket.get(), 1))
            {
                e.reset(e.copychild(1));
                std::vector<uex> v;
                syntax_report sr;
                ex_parse_exboxs(v, e.get(), true, sr);
                if (sr.have_error())
                {
		            _gen_message(gs.sym_sGeneral.get(), "sntx", "`1` near `2`.", sr.translate_error(), sr.near_error());
                    continue;
                }
//std::cout << "<kernel>: parsed as " << exvec_tostring_full(v) << std::endl;
                for (auto f = v.begin(); f != v.end(); ++f)
                {
                    esym_assign_dvalue(gs.sym_sIn.get(), emake_node(gs.sym_sIn.copy(), ecopy(gs.get_$Line())), f->copy());
                    f->set(topeval(f->release()));
                    esym_assign_dvalue(gs.sym_sOut.get(), emake_node(gs.sym_sOut.copy(), ecopy(gs.get_$Line())), f->copy());
                    if (!eis_sym(f->get(), gs.sym_sNull.get()))
                    {
                        m.reset(emake_node(gs.sym_sOutputNamePacket.copy(), gs.out_prompt_standardform()));
                        swrite_byte(fp_out, CMD_EXPR);
                        swrite_ex(fp_out, m.get());
                        fflush(fp_out);
                        f->setnz(ex_to_exbox_standard(f->get()));
                        f->set(emake_node(gs.sym_sReturnTextPacket.copy(), f->release()));
                        swrite_byte(fp_out, CMD_EXPR);
                        swrite_ex(fp_out, f->get());
                        fflush(fp_out);
                    }
                    gs.inc_$Line();
                }
                m.reset(emake_node(gs.sym_sInputNamePacket.copy(), gs.in_prompt_standardform()));
                swrite_byte(fp_out, CMD_EXPR);
                swrite_ex(fp_out, m.get());
                fflush(fp_out);
                gs.send_alive_symbols();
            }
        }
        else
        {
            std::cerr << "<!kernel>: received unknown cmd " << +cmd << std::endl;
        }
    }

    fclose(fp_out);
    fclose(fp_in);
std::cout << "<kernel>: exiting" << std::endl;
    return 0;
}
