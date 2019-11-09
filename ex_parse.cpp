#include "types.h"
#include "ex.h"
#include "ex_parse.h"
#include "globalstate.h"
#include "arithmetic.h"

globalcharinfo gci;

Precedence prefix_prec(Operator op)
{
    switch (op)
    {
        case op_pre_PreDecrement: case op_Decrement:
            return prec_pre_PreDecrement;
        case op_pre_PreIncrement: case op_Increment:
            return prec_pre_PreIncrement;
        case op_pre_Plus: case op_Plus:
            return prec_pre_Plus;
        case op_pre_Minus: case op_Minus:
            return prec_pre_Minus;
        case op_pre_Not:
            return prec_pre_Not;
        case op_pre_Sum:
            return prec_pre_Sum;
        case op_pre_Product:
            return prec_pre_Product;
        case op_pre_DifferentialD:
            return prec_pre_DifferentialD;
        default:
            return prec_none;
    }
}

Precedence infix_prec(Operator op)
{
    switch (op)
    {
        case op_inf_PatternTest:
            return prec_inf_PatternTest;
        case op_inf_StringExpression:
            return prec_inf_StringExpression;
        case op_inf_SameQ:
        case op_inf_UnsameQ:
        case op_inf_Equal:
        case op_inf_Unequal:
        case op_inf_Greater:
        case op_inf_GreaterEqual:
        case op_inf_Less:
        case op_inf_LessEqual:
            return prec_inf_Inequality;
        case op_inf_UpSet:
            return prec_inf_UpSet;
        case op_inf_UpSetDelayed:
            return prec_inf_UpSetDelayed;
        case op_inf_Set:
        case op_inf_SetDelayed:
        case op_inf_tag:
            return prec_inf_tag_Set_SetDelayed;
        case op_inf_Condition:
            return prec_inf_Condition;
        case op_inf_Rule:
            return prec_inf_Rule;
        case op_inf_RuleDelayed:
            return prec_inf_RuleDelayed;
        case op_inf_Map:
            return prec_inf_Map;
        case op_inf_MapAll:
            return prec_inf_MapAll;
        case op_inf_StringJoin:
            return prec_inf_StringJoin;
        case op_inf_ReplaceRepeated:
            return prec_inf_ReplaceRepeated;
        case op_inf_ReplaceAll:
            return prec_inf_ReplaceAll;
        case op_inf_CompoundExpression: case op_CompoundExpression:
            return prec_inf_CompoundExpression;
        case op_inf_AddTo:
            return prec_inf_AddTo;
        case op_inf_Apply:
            return prec_inf_Apply;
        case op_inf_GenPre:
            return prec_inf_to_prefix;
        case op_inf_GenPost:
            return prec_inf_to_postfix;
        case op_inf_Plus: case op_Plus:
            return prec_inf_Plus;
        case op_inf_Minus: case op_Minus:
            return prec_inf_Minus;
        case op_inf_Times:
            return prec_inf_Times;
        case op_inf_Divide:
            return prec_inf_Divide;
        case op_inf_Power:
            return prec_inf_Power;
        case op_Pattern:
            return prec_inf_Pattern;
        case op_inf_Alternatives:
            return prec_inf_Alternatives;
        case op_inf_Or:
            return prec_inf_Or;
        case op_inf_And:
            return prec_inf_And;
        case op_inf_Dot:
            return prec_inf_Dot;
        case op_inf_MessageName:
            return prec_inf_MessageName;
        default:
            return prec_none;
    }
}

Precedence postfix_prec(Operator op)
{
    switch (op)
    {
        case op_post_Decrement: case op_Decrement:
            return prec_post_Decrement;
        case op_post_Increment: case op_Increment:
            return prec_post_Increment;
        case op_post_Unset:
            return prec_post_Unset;
        case op_post_Derivative:
            return prec_post_Derivative;
        case op_post_Factorial:
            return prec_post_Factorial;
        case op_post_Factorial2:
            return prec_post_Factorial2;
        case op_post_Repeated:
            return prec_post_Repeated;
        case op_post_RepeatedNull:
            return prec_post_RepeatedNull;
        case op_post_Function:
            return prec_post_Function;
        case op_post_SuperscriptBox:
            return prec_post_SuperscriptBox;
        case op_post_SubscriptBox:
            return prec_post_SubscriptBox;
        case op_post_SubsuperscriptBox:
            return prec_post_SubsuperscriptBox;
        default:
            return prec_none;
    }
}

Association op_asso(Operator op)
{
    switch (op)
    {
        case op_inf_Rule:
        case op_inf_RuleDelayed:
        case op_inf_ReplaceAll:
        case op_inf_ReplaceRepeated:
        case op_inf_Condition:
            return assoLeft;
        case op_inf_Power:
        case op_inf_AddTo:
        case op_inf_SubtractFrom:
        case op_inf_TimesBy:
        case op_inf_DivideBy:
        case op_inf_Apply:
        case op_inf_Map:
        case op_inf_MapAll:
        case op_inf_UpSet:
        case op_inf_UpSetDelayed:
        case op_inf_Set:
        case op_inf_SetDelayed:
            return assoRight;
        case op_Pattern: case op_inf_Pattern: case op_inf_Optional:
            return assoNo;
        default:
            return assoDefault;
    }
}

er op_to_sym(Operator op)
{
    switch (op)
    {
        case op_inf_PatternTest:
            return gs.symsPatternTest.get();
        case op_inf_StringExpression:
            return gs.symsStringExpression.get();
        case op_pre_Not:
            return gs.symsNot.get();
        case op_inf_SameQ:
            return gs.symsSameQ.get();
        case op_inf_UnsameQ:
            return gs.symsUnsameQ.get();
        case op_inf_Equal:
            return gs.symsEqual.get();
        case op_inf_Unequal:
            return gs.symsUnequal.get();
        case op_inf_Greater:
            return gs.symsGreater.get();
        case op_inf_GreaterEqual:
            return gs.symsGreaterEqual.get();
        case op_inf_Less:
            return gs.symsLess.get();
        case op_inf_LessEqual:
            return gs.symsLessEqual.get();
        case op_inf_Condition:
            return gs.symsCondition.get();
        case op_inf_StringJoin:
            return gs.symsStringJoin.get();
        case op_pre_PreDecrement:
            return gs.symsPreDecrement.get();
        case op_post_Decrement:
            return gs.symsDecrement.get();
        case op_pre_PreIncrement:
            return gs.symsPreIncrement.get();
        case op_post_Increment:
            return gs.symsIncrement.get();
        case op_inf_ReplaceRepeated:
            return gs.symsReplaceRepeated.get();
        case op_inf_ReplaceAll:
            return gs.symsReplaceAll.get();
        case op_inf_AddTo:
            return gs.symsAddTo.get();
        case op_post_Derivative:
            return gs.symsDerivative.get();
        case op_inf_Map:
            return gs.symsMap.get();
        case op_inf_MapAll:
            return gs.symsMapAll.get();
        case op_inf_CompoundExpression:
            return gs.symsCompoundExpression.get();
        case op_inf_Rule:
            return gs.symsRule.get();
        case op_inf_RuleDelayed:
            return gs.symsRuleDelayed.get();
        case op_inf_Apply:
            return gs.symsApply.get();
        case op_inf_Set:
            return gs.symsSet.get();
        case op_inf_UpSet:
            return gs.symsUpSet.get();
        case op_inf_UpSetDelayed:
            return gs.symsUpSetDelayed.get();
        case op_post_Function:
            return gs.symsFunction.get();
        case op_inf_SetDelayed:
            return gs.symsSetDelayed.get();
        case op_pre_Plus:
        case op_inf_Plus:
            return gs.symsPlus.get();
        case op_pre_Minus:
        case op_inf_Minus:
            return gs.symsMinus.get();
        case op_inf_Times:
            return gs.symsTimes.get();
        case op_inf_Divide:
            return gs.symsDivide.get();
        case op_inf_Power:
            return gs.symsPower.get();
        case op_post_Factorial:
            return gs.symsFactorial.get();
        case op_post_Factorial2:
            return gs.symsFactorial2.get();
        case op_post_Unset:
            return gs.symsUnset.get();


        case op_post_RepeatedNull:
            return gs.symsRepeatedNull.get();
        case op_post_Repeated:
            return gs.symsRepeated.get();
        case op_inf_Dot:
            return gs.symsDot.get();
        case op_inf_Alternatives:
            return gs.symsAlternatives.get();
        case op_inf_Or:
            return gs.symsOr.get();
        case op_inf_And:
            return gs.symsAnd.get();

        case op_inf_MessageName:
            return gs.symsMessageName.get();

        default:
            return nullptr;
    }
}


// is the stack a sequence of expressions? i.e.  "a"  or  "a,b"  or  "a,b,c"  or  ...
bool eparser::have_comma_sequence()
{
    if (error != erNone) {return false;}
    if ((estack.size()&1)==0) {return false;}
    for (size_t k=0; k<estack.size(); k++)
    {
        if ((k&1) == 0)
        {
            if (eis_raw(estack[k].get())) {return false;}
        }
        else
        {
            if (!eis_raw(estack[k].get(), opComma)) {return false;}
        }
    }
    return true;
}

bool eparser::have_inequality(size_t&q)
{
    
    size_t n = estack.size();
    q = n;

    if (n<5 || eis_raw(estack[n-1].get()))
    {
        return false;
    }

    int mask = 0;
    q = n - 1;
    while (q>=2 && eis_raw(estack[q-1].get()) && !eis_raw(estack[q-2].get()))
    {
        Operator op = static_cast<Operator>(eto_raw(estack[q-1].get())->dat[0]);
        if (op == op_inf_SameQ) {
            mask |= 1<<0;
        } else if (op == op_inf_UnsameQ) {
            mask |= 1<<1;
        } else if (op == op_inf_Equal) {
            mask |= 1<<2;
        } else if (op == op_inf_Unequal) {
            mask |= 1<<3;
        } else if (op == op_inf_Greater) {
            mask |= 1<<4;
        } else if (op == op_inf_GreaterEqual) {
            mask |= 1<<5;
        } else if (op == op_inf_Less) {
            mask |= 1<<6;
        } else if (op == op_inf_LessEqual) {
            mask |= 1<<7;
        } else {
            break;
        }
        q -= 2;
    }

    // return popcnt(mask) > 1
    return mask & (mask - 1);
}


void eparser::popop(Precedence prec)
{
//std::cout << "popop called " << prec << std::endl;
//std::cout << "<" <<std::endl<< exvector_tostring_full(estack) <<">" <<std::endl;

    assert(!error);

    size_t n, m, k;
    if (estack.size()==0) {error = erBad; return;}

    while (true)
    {
        n = estack.size();
        if (n<2 || eis_raw(estack[n-1].get())) {return;} //must have expr on top
        if (!eis_raw(estack[n-2].get())) {return;} //must have operator on second top
        Operator op = static_cast<Operator>(eto_raw(estack[n-2].get())->dat[0]);
        Precedence p;

        if ((p = infix_prec(op)) != prec_none) // do we we have an infix operator on top?
        {
            er h = op_to_sym(op);
            if (p <= prec) {return;}
            size_t len=0;
            for (m=n-1; (m+1)!=0; m-=2)
            {
                len++;
                if (m<2) {break;}
                if (eis_raw(estack[m].get())) {error = erBad; return;}
                if (!eis_raw(estack[m-1].get(), static_cast<uint64_t>(op))) {break;}
            }
            assert(len>=2);
            size_t q;
            if (have_inequality(q))
            {
                assert(n-q>=5);
                assert((n-q)&1);
                uex e(gs.symsInequality.get(), n - q);
                for (k=q; k+1<n; k+=2) {
                    assert(!eis_raw(estack[k].get()));
                    e.push_back(estack[k].copy());
                    op = static_cast<Operator>(eto_raw(estack[k+1].get())->dat[0]);
                    e.push_back(ecopy(op_to_sym(op)));
                }
                assert(!eis_raw(estack[n-1].get()));
                e.push_back(estack[n-1].copy());
                estack.resize(q);
                estack.push_back(wex(e.release()));                
            }
            else if (op == op_inf_Set || op == op_inf_SetDelayed)
            {
                assert(!eis_raw(estack[n-3].get()));
                if (n >= 5 && !eis_raw(estack[n-5].get())
                           && eis_raw(estack[n-4].get(), op_inf_tag))
                {
                    uex e(emake_node(ecopy(op == op_inf_Set ? gs.symsTagSet.get()
                                                            : gs.symsTagSetDelayed.get()),
                                       ecopy(estack[n-5].get()),
                                       ecopy(estack[n-3].get()),
                                       ecopy(estack[n-1].get())));
                    estack.resize(n-5);
                    estack.push_back(wex(e.release()));
                }
                else
                {
                    uex e(emake_node(ecopy(h),
                                       ecopy(estack[n-3].get()),
                                       ecopy(estack[n-1].get())));
                    estack.resize(n-3);
                    estack.push_back(wex(e.release()));
                }
            }
            else if (h != nullptr)
            {
                // infix operators corresponding to symbols
                switch(op_asso(op))
                {
                    case assoNo:
                    {
                        if (len!=2) {error = erBad; return;}
                    }
                    case assoLeft:
                    {
                        k=1;
                        uex e(ecopy(estack[m+2*k-2].get()));
                        for (k=2; k<=len; k++) {
                            e.reset(emake_node(ecopy(h), e.release(), ecopy(estack[m+2*k-2].get())));
                        }
                        estack.resize(m);
                        estack.push_back(wex(e.release()));
                        break;
                    }    
                    case assoRight:
                    {
                        k=len;
                        uex e(ecopy(estack[m+2*k-2].get()));
                        for (k=len-1; k>=1; k--)
                        {
                            e.reset(emake_node(ecopy(h), estack[m+2*k-2].copy(), e.release()));
                        }
                        estack.resize(m);
                        estack.push_back(wex(e.release()));
                        break;
                    }
                    default:
                    {
                        uex e(h, len);
                        for (k=1; k<=len; k++) {
                            e.push_back(estack[m+2*k-2].copy());
                        }
                        estack.resize(m);
                        assert(evalid_contentflags(e.get()));
                        estack.push_back(wex(e.release()));
                    }
                }
            }
            else
            {// infix operators needing manual attention
                if (op == op_inf_GenPost)
                {
                    k=1;
                    uex e(estack[m+2*k-2].copy());
                    for (k=2; k<=len; k++)
                    {
                        e.reset(emake_node(estack[m+2*k-2].copy(), e.release()));
                    }
                    estack.resize(m);
                    estack.push_back(wex(e.release()));
                }
                else if (op == op_inf_GenPre)
                {
                    k=len;
                    uex e(estack[m+2*k-2].copy());
                    for (k=len-1; k>=1; k--)
                    {
                        e.reset(emake_node(estack[m+2*k-2].copy(), e.release()));
                    }
                    estack.resize(m);
                    estack.push_back(wex(e.release()));
                }
                else if (op == op_Pattern)
                {
                    k=1;
                    uex e(estack[m+2*k-2].copy());
                    k=2;
                    h = eis_sym(e.get()) ? gs.symsPattern.get() : gs.symsOptional.get();
                    e.reset(emake_node(ecopy(h), e.release(), estack[m+2*k-2].copy()));
                    for (k=3; k<=len; k++)
                    {
                        e.reset(emake_node(gs.symsOptional.copy(), e.release(), estack[m+2*k-2].copy()));
                    }
                    estack.resize(m);
                    estack.push_back(wex(e.release()));
                }
                else
                {
                    error = erBad; return;
                }
            }
        }
        else if ((p = prefix_prec(op)) != prec_none)
        {
            // we have a prefix operator
            if (p <= prec) {return;}
            if (eto_raw(estack[n-2].get())->child != nullptr)
            {
                if (eto_raw(estack[n-2].get())->dat[0] == op_pre_DifferentialD)
                {
                    uex e(ecopy(eto_raw(estack[n-2].get())->child));
                    if (eis_node(echild(e.get(),2)))
                    {
                        uex f(e.copychild(2));
                        f.replacechild(1, estack[n-1].copy());
                        e.replacechild(2, f.release());
                    }
                    else
                    {
                        e.replacechild(2, estack[n-1].copy());
                    }
                    estack.resize(n-2);
                    pushex(e.release());
                }
                else
                {
                    uex f(ecopy(eto_raw(estack[n-2].get())->child));
                    uex e(estack[n-1].copy());
                    estack.resize(n-2);
                    f.insertchild(1, e.release());
                    pushex(f.release());
                }
            }
            else if (eto_raw(estack[n-2].get())->dat[0] == op_pre_Minus
                     && eis_number(estack[n-1].get()))
            {
                uex f(estack[n-1].copy());
                estack.resize(n-2);
                pushex(num_Minus1(f.release()));
            }
            else
            {
                assert(op_to_sym(op) != nullptr);
                uex e(emake_node(ecopy(op_to_sym(op)), estack[n-1].copy()));
                estack.resize(n-2);
                estack.push_back(wex(e.release()));
            }
        }
        else
        {
            return;
        }
    }
}


void eparser::pushop_postfix(Operator post)
{
    if (estack.size()==0 || eis_raw(estack.back().get()))
    {
        error = erBad;
        return;
    }
    popop(postfix_prec(post));
    if (error!=erNone)
    {
        return;
    }
    er h = op_to_sym(post);
    assert(h != nullptr);
    uex e(emake_node(ecopy(h), estack.back().copy()));
    estack.pop_back();
    estack.push_back(wex(e.release()));
}

void eparser::pushop_postfix(Operator post, ex H)
{
    uex h(H);
    if (estack.empty() || eis_raw(estack.back().get()))
    {
        error = erBad;
        return;
    }
    popop(postfix_prec(post));
    if (error != erNone) {return;}

    uex e(emake_node(h.release(), estack.back().copy()));
    estack.pop_back();
    estack.push_back(wex(e.release()));
}


void eparser::pushop_infix(Operator in)
{
    if (estack.empty() || eis_raw(estack.back().get()))
    {
        error = erBad;
        return;
    }
    popop(infix_prec(in));
    if (error!=erNone) {return;}
    estack.push_back(wex(emake_raw(in)));
}

void eparser::pushop_prefix_infix(Operator pre, Operator in)
{
    if (estack.empty() || eis_raw(estack.back().get()))
    {
        estack.push_back(wex(emake_raw(pre)));
    }
    else
    {
        pushop_infix(in);
    }
}

void eparser::pushop_prefix_postfix(Operator pre, Operator post)
{
    if (estack.empty() || eis_raw(estack.back().get()))
    {
        estack.push_back(wex(emake_raw(pre)));
    }
    else
    {
        pushop_postfix(post);
    }
}

void eparser::pushex(ex E)
{
    uex e(E);
    if (!estack.empty() && !eis_raw(estack.back().get()))
    {
        popop(prec_inf_Times);
        estack.push_back(wex(emake_raw(op_inf_Times)));
    }
    estack.push_back(wex(e.release()));
}



void eparser::handle_newline()
{
    handle_char(CHAR_NEWLINE);
}

void eparser::handle_ex(ex E)
{
    uex e(E);
    handle_char(CHAR_NONE);
    if (error) {return;}
    handle_token_ex(e.release());
}

void eparser::handle_post_oper(uint64_t d0)
{
    handle_char(CHAR_NONE);
    if (error) {return;}
    handle_token_raw(d0);
    popop(postfix_prec(static_cast<Operator>(d0)));
}

void eparser::handle_end()
{
    handle_char(CHAR_NONE);
    if (error) {return;}
    handle_token_raw(opEnd);
    if (error == erNone && !estack.empty())
    {
        popop(prec_lowest);
    }
}



void eparser::handle_token_raw(uint64_t d0)
{
    handle_token_ex(emake_raw(d0));
}

void eparser::handle_token_ex(ex NT)
{
//std::cout << "****** error = " << error << " handle_token_ex: " << ex_tostring_full(etor(NT)) << std::endl;

    if (!esc_name.empty())
    {
        error = erBad;
        return;
    }

    nextTk.reset(NT);
/*
std::cout << "********************" << std::endl;
std::cout << "prev: " << ex_tostring_full(prevTk.get()) <<std::endl;
std::cout << "curr: " << ex_tostring_full(currTk.get()) <<std::endl;
std::cout << "next: " << ex_tostring_full(nextTk.get()) <<std::endl;
std::cout << "estack:" <<std::endl; print_exstack(estack);
*/
    Operator next_op = eis_raw(nextTk.get()) ? static_cast<Operator>(eto_raw(nextTk.get())->dat[0])
                                             : op_none;

    if (currTk.get() == nullptr)
    {
    }
    else if (!eis_raw(currTk.get()))
    {
        pushex(ecopy(currTk.get()));
    }
    else
    {
        uint64_t c0 = eto_raw(currTk.get())->dat[0];

        switch (c0)
        {
            /* infix operators */
            case op_inf_PatternTest:
            case op_inf_And:
            case op_inf_UpSet:
            case op_inf_UpSetDelayed:
			case op_inf_StringExpression:
            case op_inf_StringJoin:
            case op_inf_MessageName:
            case op_inf_Map:
            case op_inf_AddTo:
            case op_inf_SubtractFrom:
            case op_inf_Power:
            case op_inf_Times:
            case op_inf_TimesBy:
            case op_inf_Divide:
            case op_inf_DivideBy:
            case op_inf_ReplaceAll:
            case op_inf_ReplaceRepeated:
            case op_inf_Rule:
            case op_inf_RuleDelayed:
            case op_inf_Apply:
            case op_inf_GenPre:
            case op_inf_GenPost:
            case op_Pattern:
            case op_inf_Alternatives:
            case op_inf_Or:
            case op_inf_Dot:
            case op_inf_Set:
            case op_inf_SetDelayed:
            case op_inf_Condition:
            case op_inf_SameQ:
            case op_inf_UnsameQ:
            case op_inf_Equal:
            case op_inf_Unequal:
            case op_inf_Greater:
            case op_inf_GreaterEqual:
            case op_inf_Less:
            case op_inf_LessEqual:
            case op_inf_tag:
            {
                pushop_infix(static_cast<Operator>(c0));
                break;
            }

            /* postfix operators */
            case op_post_Repeated:
            case op_post_RepeatedNull:
            case op_post_Factorial:
            case op_post_Factorial2:
            case op_post_Function:
            {
                pushop_postfix(static_cast<Operator>(c0));
                break;
            }
            case op_post_Derivative:
            {
                pushop_postfix(static_cast<Operator>(c0), emake_node(ecopy(gs.symsDerivative.get()), emake_int_ui(eto_raw(currTk.get())->dat[1])));
                break;
            }
            case op_post_Unset:
            {
                pushop_postfix(static_cast<Operator>(c0));
                size_t n = estack.size();
                if (n >= 3 && !eis_raw(estack[n-3].get())
                           && eis_raw(estack[n-2].get(), op_inf_tag))
                {
                    assert(ehas_head_sym_length(estack[n-1].get(), gs.symsUnset.get(), 1));
                    uex e(emake_node(gs.symsTagUnset.copy(),
                                       estack[n-3].copy(),
                                       estack[n-1].copychild(1)));
                    estack.resize(n-3);
                    estack.push_back(wex(e.release()));
                }
                break;
            }

            /* operators to ignore */
            case op_post_SuperscriptBox:
            case op_post_SubscriptBox:
            case op_post_SubsuperscriptBox:
            {
                break;
            }

            /* special pre operators */
            case op_pre_Sum:
            case op_pre_Product:
            case op_pre_Integrate:
            {
                estack.push_back(wex(currTk.copy()));
                break;
            }
            case op_pre_DifferentialD:
            {
                popop(prec_lowest);
                if (error) {return;}
                size_t n = estack.size();
                if (n < 2) {return;}
                if (eis_raw(estack[n - 1].get())) {return;}
                if (!eis_raw(estack[n - 2].get(), op_pre_Integrate)) {return;}
                uex integral(ecopy(eto_raw(estack[n - 2].get())->child));
                uex integrand(ecopy(estack[n - 1].get()));
                estack.resize(n - 2);
                integral.replacechild(1, integrand.release());
                estack.push_back(wex(emake_raw(op_pre_DifferentialD, 0, integral.release())));
                break;
            }

            /* ambiguous operators */
            case op_Plus:
            {
                pushop_prefix_infix(op_pre_Plus, op_inf_Plus);
                break;
            }
            case op_Minus:
            {
                pushop_prefix_infix(op_pre_Minus, op_inf_Minus);
                break;
            }
            case op_bang:
            {
                pushop_prefix_postfix(op_pre_Not, op_post_Factorial);
                break;
            }
            case op_Increment:
            {
                pushop_prefix_postfix(op_pre_PreIncrement, op_post_Increment);
                break;
            }
            case op_Decrement:
            {
                pushop_prefix_postfix(op_pre_PreDecrement, op_post_Decrement);
                break;
            }

            case op_CompoundExpression:
            {
                assert(nextTk.get() != nullptr);
                if (eis_raw(nextTk.get()) && next_op != bracLCurly
                                          && next_op != bracLRound
                                          && next_op != op_none
                                          && prefix_prec(next_op) == prec_none)
                {
                    pushop_infix(op_inf_CompoundExpression);
                    if (error) {return;}
                    pushex(ecopy(gs.symsNull.get()));
                    if (error) {return;}            
                    popop(prec_inf_CompoundExpression - 1);
                    if (error) {return;}            
                    break;
                }
                else
                {
                    pushop_infix(op_inf_CompoundExpression);
                    break;
                }
            }

            case opComma:
            {
                popop(prec_lowest);
                estack.push_back(wex(emake_raw(opComma)));
                break;
            }


            /* closedish operators */
            case bracLRound:
            {
                estack.push_back(wex(emake_raw(bracLRound)));
                break;
            }
            case bracRRound:
            {
                popop(prec_lowest);
                size_t n = estack.size();
                if (n<2) {error = erBad; return;}
                if (eis_raw(estack[n-1].get())) {error = erBad; return;}
                if (!eis_raw(estack[n-2].get(), bracLRound)) {error = erBad; return;}
                uex e(ecopy(estack[n-1].get()));
                estack.resize(n-2);
                pushex(e.release());
                break;
            }

            case bracLCurly:
            {
                estack.push_back(wex(emake_raw(bracLCurly)));
                break;
            }
            case bracRCurly:
            {
                popop(prec_lowest);
                size_t n = estack.size();
                assert(n>0);
                size_t m=n-1, k, len=0;
                if (!(eis_raw(estack[m].get(), bracLCurly))) {
                    while (true) {
                        if (m==0 || eis_raw(estack[m].get())) {error = erBad; return;}
                        m--; len++;
                        if (eis_raw(estack[m].get(), bracLCurly)) {break;}
                        if (m==0) {error = erBad; return;}
                        if (!eis_raw(estack[m].get(), opComma)) {error = erBad; return;}
                        m--;
                    }
                }
                uex e(gs.symsList.get(), len);
                for (k = 1; k <= len; k++)
                {
                    e.push_back(estack[m+2*k-1].copy());
                }
                estack.resize(m);
                pushex(e.release());
                break;

            }

            case bracLAssociation:
            {
                estack.push_back(wex(emake_raw(bracLAssociation)));
                break;
            }
            case bracRAssociation:
            {
                popop(prec_lowest);
                size_t n = estack.size();
                assert(n>0);
                size_t m=n-1, k, len=0;
                if (!(eis_raw(estack[m].get(), bracLAssociation))) {
                    while (true) {
                        if (m==0 || eis_raw(estack[m].get())) {error = erBad; return;}
                        m--; len++;
                        if (eis_raw(estack[m].get(), bracLAssociation)) {break;}
                        if (m==0) {error = erBad; return;}
                        if (!eis_raw(estack[m].get(), opComma)) {error = erBad; return;}
                        m--;
                    }
                }
                uex e(gs.symsAssociation.get(), len);
                for (k = 1; k <= len; k++)
                {
                    e.push_back(estack[m+2*k-1].copy());
                }
                estack.resize(m);
                pushex(e.release());
                break;
            }


            case bracLSquare:
            {
                if (estack.empty() || eis_raw(estack.back().get())) {error = erBad; return;}
                popop(prec_postd_squarebracket);
                estack.push_back(wex(emake_raw(bracLSquare)));
                break;
            }
            case bracRSquare:
            {
                popop(prec_lowest);
                size_t n = estack.size();
                assert(n>0);
                size_t m=n-1, k, len=0;
                if (!(eis_raw(estack[m].get(), bracLSquare)))
                {
                    while (true)
                    {
                        if (m==0 || eis_raw(estack[m].get())) {error = erBad; return;}
                        m--; len++;
                        if (eis_raw(estack[m].get(), bracLSquare)) {break;}
                        if (m==0) {error = erBad; return;}
                        if (!eis_raw(estack[m].get(), opComma)) {error = erBad; return;}
                        m--;
                    }
                }
                if (m==0 || eis_raw(estack[m-1].get())) {error = erBad; return;}

                uex e(estack[m-1].get(), len);
                for (k=1; k<=len; k++)
                {
                    e.push_back(estack[m+2*k-1].copy());
                }
                estack.resize(m-1);
                pushex(e.release());
                break;
            }

            case bracLDouble:
            {
                if (estack.empty() || eis_raw(estack.back().get())) {error = erBad; return;}
                popop(prec_postd_doublesquarebracket);
                estack.push_back(wex(emake_raw(bracLDouble)));
                break;
            }
            case bracRDouble:
            {
                popop(prec_lowest);
                size_t n = estack.size();
                assert(n>0);
                size_t m=n-1, k, len=0;
                if (!(eis_raw(estack[m].get(), bracLDouble))) {
                    while (true) {
                        if (m==0 || eis_raw(estack[m].get())) {error = erBad; return;}
                        m--; len++;
                        if (eis_raw(estack[m].get(), bracLDouble)) {break;}
                        if (m==0) {error = erBad; return;}
                        if (!eis_raw(estack[m].get(), opComma)) {error = erBad; return;}
                        m--;
                    }
                }
                if (m==0 || eis_raw(estack[m-1].get())) {error = erBad; return;}

                uex e(gs.symsPart.get(), len+1);
                for (k=0; k<=len; k++) {
                    e.push_back(estack[m+2*k-1].copy());
                }
                estack.resize(m-1);
                estack.push_back(wex(e.release()));
                break;
            }

            default:
            {
                assert(false);
            }
        }
    }

    prevTk.reset(currTk.release());
    currTk.reset(nextTk.release());
}



ex syntax_report::translate_error()
{
    switch (error)
    {
        case erBad:
            return emake_str("Syntax error");
        case erUnexpectedEnd:
            return emake_str("Unexpected end of input");

        default:
            return emake_str("Unknown syntax error");

    }
}

ex syntax_report::near_error()
{
    if (around.get() == nullptr)
    {
        return emake_str("???");
    }
    else
    {
/*        ex t = emake_node(gs.symsList.copy(), emake_str("we don't know"));
        return emake_node(gs.symsRowBox.copy(), t);
*/
        return around.copy();
    }
}

void syntax_report::handle_row_error(PError e, er b, size_t i, size_t si)
{
    if (e != erNone)
    {
        error = e;
    }
    else
    {
        assert(error != erNone);
    }
    if (around.get() == nullptr)
    {
        around.reset(gs.symsPi.copy());
    }
}

void syntax_report::handle_row_error(PError e, er b, size_t i)
{
    if (e != erNone)
    {
        error = e;
    }
    else
    {
        assert(error != erNone);
    }
    if (around.get() == nullptr)
    {
        around.reset(gs.symsPi.copy());
    }
}

void syntax_report::handle_row_end_error(er b)
{
    PError e = erBad;
    if (e != erNone)
    {
        error = e;
    }
    else
    {
        assert(error != erNone);
    }
    if (around.get() == nullptr)
    {
        around.reset(gs.symsPi.copy());
    }
}

void syntax_report::handle_string_error(PError e, er b, size_t si)
{
    if (e != erNone)
    {
        error = e;
    }
    else
    {
        assert(error != erNone);
    }
    if (around.get() == nullptr)
    {
        around.reset(gs.symsPi.copy());
    }
}

void syntax_report::handle_cstr_error(PError e, const char * b, size_t si)
{
    if (e != erNone)
    {
        error = e;
    }
    else
    {
        assert(error != erNone);
    }
    if (around.get() == nullptr)
    {
        around.reset(gs.symsPi.copy());
    }
}
