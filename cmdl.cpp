#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<string>
#include<memory>
#include<vector>
#include<stack>
#include<list>
#include<map>
#include<set>
#include<assert.h>
#include<string.h>
#include<cmath>

#include <new>

#include "flintarb_wrappers.h"

#include "types.h"
#include "ex.h"
#include "ex_cont.h"
#include "globalstate.h"
#include "ex_parse.h"
#include "ex_parse_string.h"
#include "ex_print.h"

#include "ex_print.h"
#include "eval.h"
#include "sudcode.h"
#include "hash.h"
#include "arithmetic.h"

globalstate gs;
FILE * fp_in, * fp_out;
int fp_out_type;


int main(int argc, char *argv[])
{
    int ret_code = 0;
    fp_out = stdout;
    fp_out_type = FP_OUT_TEXT;

    try
    {
        std::vector<uex> results;
        std::string inputline;
        int empty_input_count = 0;

        gs.inc_$Line();

        std::cout << "Hello. This program comes with a guaranteed warning." << std::endl;

        int argi = 1;
        while (true)
        {
            if (argi < argc)
            {
                inputline = argv[argi];
                argi++;
            }
            else
            {
                std::getline(std::cin, inputline);
            }

            if (inputline.empty())
            {
                if (++empty_input_count > 1)
                    break;
                else
                    continue;
            }
            else
            {
                empty_input_count = 0;
            }

            if (inputline == "Quit" || inputline == "Exit")
            {
                break;
            }
            std::vector<uex> v;
            syntax_report sr;
            ex_parse_string(v, inputline.c_str(), inputline.size(), true, sr);
            if (sr.have_error())
            {
                _gen_message(gs.sym_sGeneral.get(), "sntx", NULL, sr.translate_error(), sr.near_error());
                continue;
            }
            for (auto e = v.begin(); e != v.end(); ++e)
            {
                std::cout << "In[" << ex_tostring(gs.get_$Line()) << "] := " << ex_tostring(e->get()) << std::endl;
                esym_assign_dvalue(gs.sym_sIn.get(), emake_node(gs.sym_sIn.copy(), ecopy(gs.get_$Line())), e->copy());
                e->set(topeval(e->release()));
                esym_assign_dvalue(gs.sym_sOut.get(), emake_node(gs.sym_sOut.copy(), ecopy(gs.get_$Line())), e->copy());
                if (!eis_sym(e->get(), gs.sym_sNull.get()))
                {
                    std::cout << "Out[" << ex_tostring(gs.get_$Line()) << "] = " << ex_tostring(e->get()) << std::endl;
                }
                gs.inc_$Line();
                eclear(e->release());
            }
            std::cout << std::endl;
        }
    }
    catch (const exception_exit & X)
    {
        ret_code = X.retcode;
    }

    gs.clear_values();
    flint_cleanup();
    return ret_code;
}
