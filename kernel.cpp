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
    printf("kernel started with fd_out = %s, fd_in = %s\n", argv[1], argv[2]);

    int fdout = atoi(argv[1]);
    fp_out = fdopen(fdout, "wb");
    int fdin  = atoi(argv[2]);
    fp_in = fdopen(fdin, "rb");
    fp_out_type = FP_OUT_BINARY;

    gs.inc_$Line();
    uex m(emake_node(gs.symsInputNamePacket.copy(), gs.in_prompt_standardform()));
    swrite_byte(fp_out, CMD_EXPR);
    swrite_ex(fp_out, m.get());
    fflush(fp_out);
    gs.send_alive_symbols();

    while (1)
    {
        unsigned char buffer[8];
        if (1 != fread(buffer, 1, 1, fp_in))
        {
            std::cout << "ker: could not read" << std::endl;
            break;
        }

        if (buffer[0] == CMD_EXPR)
        {
            uex e;
            int err = sread_ex(fp_in, e);
std::cout << "<gui reader>: CMD_EXPR err: " << err << "  e: " << ex_tostring_full(e.get()) << std::endl;
            if (err != 0)
            {
                std::cerr << "<!gui reader>: could not read CMD_EXPR - error " << err << std::endl;
            }
            else if (ehas_head_sym_length(e.get(), gs.symsEnterExpressionPacket.get(), 1))
            {
                e.reset(e.copychild(1));
                e.reset(topeval(e.release()));
                e.reset(emake_node(gs.symsReturnExpressionPacket.copy(), e.release()));
                swrite_byte(fp_out, CMD_EXPR);
                swrite_ex(fp_out, e.get());
                fflush(fp_out);
            }
            else if (ehas_head_sym_length(e.get(), gs.symsEnterTextPacket.get(), 1))
            {
                e.reset(e.copychild(1));
                std::vector<uex> v;
                syntax_report sr;
                ex_parse_exboxs(v, e.get(), true, sr);
                if (sr.have_error())
                {
		            _gen_message(gs.symsGeneral.get(), "sntx", "`1` near `2`.", sr.translate_error(), sr.near_error());
                    continue;
                }
std::cout << "<kernel>: parsed as " << exvec_tostring_full(v) << std::endl;
                for (auto f = v.begin(); f != v.end(); ++f)
                {
                    esym_assign_dvalue(gs.symsIn.get(), emake_node(gs.symsIn.copy(), ecopy(gs.get_$Line())), f->copy());
                    f->set(topeval(f->release()));
                    esym_assign_dvalue(gs.symsOut.get(), emake_node(gs.symsOut.copy(), ecopy(gs.get_$Line())), f->copy());
                    if (!eis_sym(f->get(), gs.symsNull.get()))
                    {
                        m.reset(emake_node(gs.symsOutputNamePacket.copy(), gs.out_prompt_standardform()));
                        swrite_byte(fp_out, CMD_EXPR);
                        swrite_ex(fp_out, m.get());
                        fflush(fp_out);

                        f->setnz(ex_to_exbox_standard(f->get()));
                        f->set(emake_node(gs.symsReturnTextPacket.copy(), f->release()));
                        swrite_byte(fp_out, CMD_EXPR);
                        swrite_ex(fp_out, f->get());
                        fflush(fp_out);
                    }
                    gs.inc_$Line();
                }
                m.reset(emake_node(gs.symsInputNamePacket.copy(), gs.in_prompt_standardform()));
                swrite_byte(fp_out, CMD_EXPR);
                swrite_ex(fp_out, m.get());
                fflush(fp_out);
                gs.send_alive_symbols();
            }
        }
        else
        {
            std::cerr << "ker: received unknown cmd " << (int)(buffer[0]) << std::endl;
        }
    }

    fclose(fp_out);
    fclose(fp_in);
    std::cout << "ker: closing" << std::endl;
}
