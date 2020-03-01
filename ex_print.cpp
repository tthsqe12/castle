#include "types.h"
#include "ex.h"
#include "uex.h"
#include "ex_cont.h"
#include "ex_parse.h"
#include "ex_print.h"
#include "globalstate.h"
#include "eval.h"

ex eval_with_minus(ex E)
{
    if (eis_leaf(E))
    {
        if (   (eis_int(E) && fmpz_sgn(eint_data(E)) < 0)
            || (eis_rat(E) && fmpq_sgn(erat_data(E)) < 0)
            || (eis_double(E) && edouble_number(E) < 0)
			|| (eis_real(E) && arf_sgn(arb_midref(ereal_data(E))) < 0)  )
        {
            uex e(E);
            return emake_node(gs.sym_sMinus.copy(), num_Minus1(e.get()));
        }
        else if (eis_cmplx(E))
        {
            if (!eis_zero(eto_cmplx(E)->real))
            {
                return E;
            }

            er Ei = ecmplx_imag(E);
            if (   (eis_int(Ei) && fmpz_sgn(eint_data(Ei)) < 0)
                || (eis_rat(Ei) && fmpq_sgn(erat_data(Ei)) < 0)
                || (eis_double(Ei) && edouble_number(Ei) < 0)
			    || (eis_real(Ei) && arf_sgn(arb_midref(ereal_data(Ei))) < 0)  )
            {
                uex e(E);
                ex t = num_Minus1(e.get());
                return emake_node(gs.sym_sMinus.copy(), t);
            }

            return E;
        }
        return E;
    }

    if (ehas_head_sym(E, gs.sym_sTimes.get()) && elength(E) > 1)
    {
        uex e(E);
        size_t len = elength(E);
        ex E1 = eval_with_minus(ecopychild(E,1));

        if (ehas_head_sym_length(E1, gs.sym_sMinus.get(), 1))
        {
            er F = echild(E1,1);
            if (eis_one(F))
            {
                eclear(E1);
                if (len == 2)
                {
                    e.reset(ecopychild(E,2));
                }
                else
                {
                    e.removechild(1);
                }
            }
            else
            {
                ex FF = ecopy(F);
                eclear(E1);
                e.replacechild(1, FF);
            }
            return emake_node(gs.sym_sMinus.copy(), e.release());
        }
        else
        {
            eclear(E1);
            return e.release();
        }
    }

    return E;
}


ex eval_with_sqrt(ex E) {
    uex e(E);
//std::cout << "eval_with_sqrt: " << ex_tostring_full(e.get()) << std::endl;

    if (eis_leaf(e.get()))
    {
        return e.release();
    }
    else if (!eis_node(E))
    {
        return e.release();
    }
    for (size_t i = 0; i<=elength(e.get()); i++)
    {
        e.replacechild(i, eval_with_sqrt(e.copychild(i)));
    }
    if (ehas_head_sym_length(e.get(), gs.sym_sPower.get(), 2))
    {
        er f = e.child(2);
        if (ehas_head_sym(f, gs.sym_sDivide.get())
              && elength(f) == 2
              && eis_int(echild(f,1), 1)
              && eis_int(echild(f,2), 2))
        {
            return emake_node(gs.sym_sSqrt.copy(), e.copychild(1));
        }
    }
    return e.release();
}


ex eval_with_minus_divide(ex E)
{
    uex e(E);
//std::cout << "eval_with_minus_divide: " << ex_tostring_full(e.get()) << std::endl;

    if (eis_leaf(e.get()))
    {
        if (   (eis_int(e.get()) && fmpz_sgn(eint_data(e.get())) < 0)
            || (eis_double(e.get()) && edouble_number(e.get()) < 0)
			|| (eis_real(e.get()) && arf_sgn(arb_midref(ereal_data(e.get()))) < 0)  )
        {
            return emake_node(gs.sym_sMinus.copy(), num_Minus1(e.get()));
        }
        else if (eis_rat(e.get()))
        {
            bool has_minus = false;
            xfmpz_t a, b;
            fmpz_set(a.data, fmpq_numref(erat_data(e.get())));
            fmpz_set(b.data, fmpq_denref(erat_data(e.get())));
            if (fmpz_sgn(a.data) < 0)
            {
                fmpz_neg(a.data, a.data);
                has_minus = true;
            }
            uex f(emake_node(gs.sym_sDivide.copy(), emake_int_move(a), emake_int_move(b)));
            if (has_minus)
            {
                return emake_node(gs.sym_sMinus.copy(), f.release());
            }
            else
            {
                return f.release();
            }
        }
        else if (eis_cmplx(e.get()))
        {
            uex Real(ecopy(eto_cmplx(e.get())->real));
            uex Imag(ecopy(eto_cmplx(e.get())->imag));
            eclear(e.release());
            if (eis_zero(Imag.get()))
            {
                return eval_with_minus_divide(Real.release());
            }
            else
            {
                if (eis_zero(Real.get()))
                {
                    return eval_with_minus_divide(
                                emake_node(ecopy(gs.sym_sTimes.get()),
                                           Imag.release(),
                                           ecopy(gs.sym_sI.get())));
                }
                else
                {
                    return eval_with_minus_divide(
                                emake_node(ecopy(gs.sym_sPlus.get()),
                                           Real.release(),
                                           emake_node(ecopy(gs.sym_sTimes.get()),
                                                      Imag.release(),
                                                      ecopy(gs.sym_sI.get()))));
                }
            }
        }
        else
        {
            return e.release();
        }
    }
    else if (!eis_node(E))
    {
        return e.release();
    }

    size_t n = elength(e.get());
    for (size_t i = 0; i <= n; i++)
    {
        e.replacechild(i, eval_with_minus_divide(e.copychild(i)));
    }

    if (ehas_head_sym(e.get(), gs.sym_sPlus.get()) && n>=2)
    {
        size_t j=2;
        while(j<=n)
        {
            er F = echild(e.get(),j);
            er G = echild(e.get(),j-1);
            if (ehas_head_sym(F, gs.sym_sMinus.get()) && elength(F)==1)
            {
                if (ehas_head_sym(G, gs.sym_sMinus.get()) && elength(G)>=2)
                {
                    e.replacechild(j-1, eappend(ecopy(G), ecopychild(F,1)));
                } else {
                    e.replacechild(j-1, emake_node(gs.sym_sMinus.copy(), ecopy(G), ecopychild(F,1)));
                }
                e.removechild(j); n--;
            } else {
                j++;
            }
        }
        if (n==1) {
            return e.copychild(1);
        } else {
            return e.release();
        }
    }
    else if (ehas_head_sym(e.get(), gs.sym_sTimes.get()) && elength(e.get())>=2)
    {
        std::vector<uex> a;
        std::vector<uex> b;
        bool has_minus = false;
        for (size_t i=1; i<=n; i++)
        {            
            er f = echild(e.get(),i);
            if (ehas_head_sym(f, gs.sym_sMinus.get()) && elength(f) == 1)
            {
                has_minus = !has_minus;
                f = echild(f,1);
            }
            if (ehas_head_sym(f, gs.sym_sDivide.get()) && elength(f) == 2)
            {
                if (!eis_int(echild(f,1), 1)) {
                    a.push_back(uex(ecopychild(f,1)));
                }
                b.push_back(uex(ecopychild(f,2)));
            }
            else
            {
                if (!eis_int(f, 1))
                {
                    a.push_back(uex(ecopy(f)));
                }
            }
        }
        eclear(e.release());
        if (b.empty()) {
            e = uex(emake_node_times(a));
        } else {
            e = uex(emake_node(ecopy(gs.sym_sDivide.get()), emake_node_times(a), emake_node_times(b)));
        }
        if (has_minus) {
            e = uex(emake_node(ecopy(gs.sym_sMinus.get()), e.release()));
        }
        return e.release();
    }
    else if (ehas_head_sym(e.get(), gs.sym_sPower.get())
             && elength(e.get())==2
             && ehas_head_sym(echild(e.get(),2), gs.sym_sMinus.get())
             && elength(echild(e.get(),2))==1)
    {

        er g = echild(e.get(),2);
        if (eis_int(echild(g,1), 1)) {
            return emake_node(gs.sym_sDivide.copy(), emake_cint(1), ecopychild(e.get(),1));
        } else {
            e.replacechild(2, ecopychild(g,1));
            return emake_node(gs.sym_sDivide.copy(), emake_cint(1), e.release());
        }
    }
    else
    {
        return e.release();
    }
}

Operator ex_to_prefix(er e)
{
    if (eis_leaf(e) || elength(e)!=1)
        return op_none;

    er h = echild(e,0);
    if (!eis_sym(h))
        return op_none;
    if (eis_sym(h, gs.sym_sPreIncrement.get())) {return op_pre_PreIncrement;}
    if (eis_sym(h, gs.sym_sPreDecrement.get())) {return op_pre_PreDecrement;}
    if (eis_sym(h, gs.sym_sPlus.get())) {return op_pre_Plus;}
    if (eis_sym(h, gs.sym_sMinus.get())) {return op_pre_Minus;}
    if (eis_sym(h, gs.sym_sNot.get())) {return op_pre_Not;}

    return op_none;
}

Operator ex_to_postfix(er e)
{
    if (eis_leaf(e) || elength(e)!=1)
        return op_none;

    er h = echild(e,0);
    if (!eis_sym(h))
        return op_none;

    if (eis_sym(h, gs.sym_sFunction.get())) {return op_post_Function;}
    if (eis_sym(h, gs.sym_sIncrement.get())) {return op_post_Increment;}
    if (eis_sym(h, gs.sym_sDecrement.get())) {return op_post_Decrement;}
    if (eis_sym(h, gs.sym_sFactorial.get())) {return op_post_Factorial;}
    if (eis_sym(h, gs.sym_sFactorial2.get())) {return op_post_Factorial2;}
    if (eis_sym(h, gs.sym_sUnset.get())) {return op_post_Unset;}
    if (eis_sym(h, gs.sym_sRepeated.get())) {return op_post_Repeated;}
    if (eis_sym(h, gs.sym_sRepeatedNull.get())) {return op_post_RepeatedNull;}

    return op_none;
}

Operator ex_to_infix(er e)
{
    if (eis_leaf(e) || elength(e)<2)
        return op_none;

    er h = echild(e,0);
    if (!eis_sym(h))
        return op_none;

    if (eis_sym(h, gs.sym_sSameQ.get())) {return op_inf_SameQ;}
    if (eis_sym(h, gs.sym_sUnsameQ.get())) {return op_inf_UnsameQ;}
    if (eis_sym(h, gs.sym_sEqual.get())) {return op_inf_Equal;}
    if (eis_sym(h, gs.sym_sUnequal.get())) {return op_inf_Unequal;}
    if (eis_sym(h, gs.sym_sGreater.get())) {return op_inf_Greater;}
    if (eis_sym(h, gs.sym_sGreaterEqual.get())) {return op_inf_GreaterEqual;}
    if (eis_sym(h, gs.sym_sLess.get())) {return op_inf_Less;}
    if (eis_sym(h, gs.sym_sLessEqual.get())) {return op_inf_LessEqual;}
    if (eis_sym(h, gs.sym_sPlus.get())) {return op_inf_Plus;}
    if (eis_sym(h, gs.sym_sMinus.get())) {return op_inf_Minus;}
    if (eis_sym(h, gs.sym_sTimes.get())) {return op_inf_Times;}
    if (eis_sym(h, gs.sym_sDivide.get())) {return op_inf_Divide;}
    if (eis_sym(h, gs.sym_sCompoundExpression.get())) {return op_inf_CompoundExpression;}
    if (eis_sym(h, gs.sym_sOr.get())) {return op_inf_Or;}
    if (eis_sym(h, gs.sym_sAlternatives.get())) {return op_inf_Alternatives;}
    if (eis_sym(h, gs.sym_sDot.get())) {return op_inf_Dot;}

    if (elength(e) == 2)
    {
        if (eis_sym(h, gs.sym_sCondition.get())) {return op_inf_Condition;}
        if (eis_sym(h, gs.sym_sSet.get())) {return op_inf_Set;}
        if (eis_sym(h, gs.sym_sSetDelayed.get())) {return op_inf_SetDelayed;}
        if (eis_sym(h, gs.sym_sPower.get())) {return op_inf_Power;}
        if (eis_sym(h, gs.sym_sAddTo.get())) {return op_inf_AddTo;}
        if (eis_sym(h, gs.sym_sSubtractFrom.get())) {return op_inf_SubtractFrom;}
        if (eis_sym(h, gs.sym_sTimesBy.get())) {return op_inf_TimesBy;}
        if (eis_sym(h, gs.sym_sDivideBy.get())) {return op_inf_DivideBy;}
        if (eis_sym(h, gs.sym_sRule.get())) {return op_inf_Rule;}
        if (eis_sym(h, gs.sym_sRuleDelayed.get())) {return op_inf_RuleDelayed;}
        if (eis_sym(h, gs.sym_sReplaceAll.get())) {return op_inf_ReplaceAll;}
        if (eis_sym(h, gs.sym_sReplaceRepeated.get())) {return op_inf_ReplaceRepeated;}
    }

    return op_none;
}

/*
const char* op_to_cstr(Operator op, bool fancy)
{
    switch (op)
    {
        case op_pre_Not:
            return "!";
        case op_inf_Rule:
            return fancy ? u8"\uF522" : " -> ";
        case op_inf_RuleDelayed:
            return fancy ? u8"\uF51F" : " :> ";
        case op_inf_ReplaceAll:
            return " /. ";
        case op_inf_ReplaceRepeated:
            return " //. ";
        case op_inf_SameQ:
            return " === ";
        case op_inf_UnsameQ:
            return " =!= ";
        case op_inf_Equal:
            return fancy ? u8"\uF431" : " == ";
        case op_inf_Unequal:
            return fancy ? u8"\u2260" : " != ";
        case op_inf_Greater:
            return " > ";
        case op_inf_GreaterEqual:
            return fancy ? u8"\u2265" : " >= ";
        case op_inf_Less:
            return " < ";
        case op_inf_LessEqual:
            return fancy ? u8"\u2264" : " <= ";
        case op_pre_PreIncrement:
        case op_post_Increment:
            return "++";
        case op_pre_PreDecrement:
        case op_post_Decrement:
            return "--";
        case op_pre_Plus:
            return "+";
        case op_inf_Plus:
            return " + ";
        case op_pre_Minus:
            return "-";
        case op_inf_Minus:
            return " - ";
        case op_inf_Times:
            return fancy ? " " : "*";
        case op_inf_Divide:
            return "/";
        case op_inf_Power:
            return "^";
        case op_post_Factorial:
            return "!";
        case op_post_Factorial2:
            return "!!";
        case op_inf_tag:
            return " /: ";
        case op_inf_Set:
            return " = ";
        case op_inf_SetDelayed:
            return " := ";
        case op_post_Unset:
            return " =.";
        case op_inf_AddTo:
            return " += ";
        case op_inf_SubtractFrom:
            return " -= ";
        case op_inf_TimesBy:
            return " *= ";
        case op_inf_DivideBy:
            return " /= ";
        case op_inf_CompoundExpression:
            return "; ";
        case op_inf_Condition:
            return " /; ";
        case op_inf_Alternatives:
            return " | ";
        case op_inf_Or:
            return " || ";
        case op_inf_Dot:
            return " . ";
        case op_post_Function:
            return " &";
        case op_post_Repeated:
            return "..";
        case op_post_RepeatedNull:
            return "...";
        default:
            assert(false);
            return "???";
    }
}
*/

bool istagchar(char16_t c) {
    return isletterchar(c) || ('0' <= c && c <= '9')/* || c == '`'*/;
}

bool eis_str_messagetag(er e)
{
    if (etype(e) != ETYPE_STR)
    {
        return false;
    }
    for (size_t i = 0; i < eto_str(e)->string.length(); i++)
    {
        if (!istagchar(eto_str(e)->string[i]))
        {
            return false;
        }
    }
    return true;
}
