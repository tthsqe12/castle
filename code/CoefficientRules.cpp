#include "globalstate.h"
#include "code.h"
#include "polynomial.h"

ex dcode_sCoefficientRules(er e)
{
//std::cout << "dcode_sTogether: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sCoefficientRules.get()));

    if (elength(e) == 2 && ehas_head_sym(echild(e,2), gs.sym_sList.get()))
    {
        poly p(elength(echild(e,2)));
        if (!ex_to_polynomial(p, echild(e,1), echild(e,2)))
        {
            return ecopy(e);
        }
//        compiledpoly prog;
//        compile_poly(prog, p);
//        std::cout << "compiled: " << prog.tostring() << std::endl;
/*
        std::vector<wex> stack;
        for (size_t i = 0; i < p.nvars; i++)
        {
            stack.push_back(emake_node(gs.sym_sSlot.copy(), emake_int_ui(i+1)));
        }
        stack.push_back(wex(gs.sym_sNull.copy()));
        stack.push_back(wex(gs.sym_sNull.copy()));
        stack.push_back(wex(gs.sym_sNull.copy()));
        stack.push_back(wex(gs.sym_sNull.copy()));
        eval_poly_ex(stack, prog.prog.data(), prog.prog.size());
std::cout << "output from program: " << ex_tostring(stack[p.nvars].get()) << std::endl;
*/
        uex r(gs.sym_sList.get(), p.size());
        for (size_t i = 0; i < p.size(); i++)
        {
            uex ve(gs.sym_sList.get(), p.vars.size());
            for (size_t j = 0; j < p.vars.size(); j++)
            {
                ve.push_back(emake_int_copy(p.exps.data()[i*p.vars.size() + j].data));
            }
            r.push_back(emake_node(gs.sym_sRule.copy(), ve.release(), p.coeffs[i].copy()));
        }
        return r.release();
    }
    else
    {
        return ecopy(e);
    }
}
