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


std::vector<void*> exs_in_use;
globalstate gs;
FILE * fp_in, * fp_out;
int fp_out_type;


int main(int argc, char *argv[])
{
    fp_out = stdout;
    fp_out_type = FP_OUT_TEXT;
    print_memory_usage();

    std::vector<uex> results;
    std::string inputline;

    gs.inc_$Line();

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

        if (inputline.empty() || inputline == "Quit" || inputline == "Exit")
        {
            break;
        }
        std::vector<uex> v;
        syntax_report sr;
        ex_parse_string(v, inputline.c_str(), inputline.size(), true, sr);
        if (sr.have_error())
        {
            _gen_message(gs.symsGeneral.get(), "sntx", "`1` near `2`.", sr.translate_error(), sr.near_error());
            continue;
        }
        for (auto e = v.begin(); e != v.end(); ++e)
        {
            std::cout << "In[" << ex_tostring(gs.get_$Line()) << "] := " << ex_tostring(e->get()) << std::endl;
            esym_assign_dvalue(gs.symsIn.get(), emake_node(gs.symsIn.copy(), ecopy(gs.get_$Line())), e->copy());
            e->set(topeval(e->release()));
            esym_assign_dvalue(gs.symsOut.get(), emake_node(gs.symsOut.copy(), ecopy(gs.get_$Line())), emake_cint(10));
            if (!eis_sym(e->get(), gs.symsNull.get()))
            {
                std::cout << "Out[" << ex_tostring(gs.get_$Line()) << "] = " << ex_tostring(e->get()) << std::endl;
            }
            gs.inc_$Line();
            eclear(e->release());
        }
        std::cout << std::endl;
    }

    gs.clear_values();
    print_memory_usage();
    flint_cleanup();
    return 0;
}
