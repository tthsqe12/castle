#include "types.h"
#include "ex.h"
#include "uex.h"
#include "globalstate.h"
#include "ex_cont.h"
#include "ex_parse.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"
#include "arithmetic.h"

ex eval_num(ex E, slong p);

#define bflag_script    0x0001
#define bflag_useboxes  0x0002
#define bflag_useops    0x0004
#define bflag_useascii  0x0008
#define bflag_matrix    0x0100

class exbox_print_options
{
public:
    uint32_t flags;
    uint32_t number_base;

    exbox_print_options(uint32_t f) : flags(f), number_base(10) {};
    exbox_print_options(const exbox_print_options * other) : flags(other->flags & ~bflag_script), number_base(other->number_base) {};
    bool fancyops() const {return flags & bflag_useboxes;}
    bool useboxes() const {return flags & bflag_useboxes;}
};

class exbox_printer
{
public:
    std::vector<uex> v;
    Operator prev_token;
    bool all_strings;

    void add_sym(er e);
    void add_string(std::string & s);
    void add_string(std::string && s);
    void add_string(const char * s);
    void add_char(char16_t c);
    void add_byte(unsigned char c);
    template <bool fancy> void add_op(Operator op);
    inline void add_op(Operator op, const exbox_print_options * OP) {OP->fancyops() ? add_op<true>(op) : add_op<false>(op);}
    void add_ex(ex e) {all_strings = false; v.push_back(uex(e)); prev_token = op_none;};
    void print_ex(er e, Precedence l, Precedence r, const exbox_print_options * OP);
    void print_gen(er e, Precedence l, Precedence r, const exbox_print_options * OP);
    void print_inf_op(Operator op, er e, Precedence l, Precedence r, const exbox_print_options * OP);
    void print_times(std::vector<uex> e, Precedence l, Precedence r, const exbox_print_options * OP);
    void print_power(er base, er pow, Precedence l, Precedence r, const exbox_print_options * OP);
#define PRNT_DECL(name) void print_##name(er e, Precedence l, Precedence r, const exbox_print_options * OP);
    PRNT_DECL(BaseForm);
    PRNT_DECL(Blank);
    PRNT_DECL(BlankNullSequence);
    PRNT_DECL(BlankSequence);
    PRNT_DECL(CompoundExpression);
    PRNT_DECL(Divide);
    PRNT_DECL(FullForm);
    PRNT_DECL(Graphics3D);
    PRNT_DECL(HoldForm);
    PRNT_DECL(Inequality);
    PRNT_DECL(InputForm);
    PRNT_DECL(Integrate);
    PRNT_DECL(List);
    PRNT_DECL(MatrixForm);
    PRNT_DECL(MessageName);
    PRNT_DECL(Out);
    PRNT_DECL(Part);
    PRNT_DECL(Pattern);
    PRNT_DECL(Plus);
    PRNT_DECL(Power);
    PRNT_DECL(Rotate);
    PRNT_DECL(SequenceForm);
    PRNT_DECL(Slot);
    PRNT_DECL(SlotSequence);
    PRNT_DECL(Sqrt);
    PRNT_DECL(StringForm);
    PRNT_DECL(Subscript);
    PRNT_DECL(SumProduct);
    PRNT_DECL(TagSet);
    PRNT_DECL(TagSetDelayed);
    PRNT_DECL(TagUnset);    
    PRNT_DECL(Times);
#undef PRINT_DECL
    ex torowbox();
    exbox_printer() : prev_token(op_none), all_strings(true) {};
};




ex exbox_printer::torowbox()
{
    if (all_strings)
    {
        if (v.size() == 1)
        {
            return ecopy(v[0].get());
        }
        else
        {
            std::string s;
            for (size_t i = 0; i < v.size(); i++)
            {
                s.append(eto_str(v[i].get())->string);
            }
            return emake_str_move(s);
        }
    }
    else
    {
        ex l = emake_node(gs.sym_sList.copy(), v);

std::cout << "torowbox l: " << ex_tostring_full(etor(l)) << std::endl;
        return emake_node(gs.sym_sRowBox.copy(), l);
    }
}

void exbox_printer::add_sym(er e)
{
    assert(eis_sym(e));
    if (!gs.sym_context_assumed(e))
    {
        add_string(esym_context_string(e));
    }
    add_string(esym_name_string(e));
}

void exbox_printer::add_string(std::string & s)
{
    if (v.empty() || !eis_str(v.back().get()) || !erefcnt_is1(v.back().get()))
    {
        v.push_back(uex(emake_str_copy(s)));
    }
    else
    {
        eto_str(v.back().get())->string.append(s);
    }
    prev_token = op_none;
}


void exbox_printer::add_string(std::string && s)
{
    if (v.empty() || !eis_str(v.back().get()) || !erefcnt_is1(v.back().get()))
    {
        v.push_back(uex(emake_str_move(s)));
    }
    else
    {
        eto_str(v.back().get())->string.append(s);
    }
    prev_token = op_none;
}


void exbox_printer::add_string(const char * s)
{
    if (v.empty() || !eis_str(v.back().get()) || !erefcnt_is1(v.back().get()))
    {
        v.push_back(uex(emake_str(s)));
    }
    else
    {
        eto_str(v.back().get())->string.append(s);
    }
    prev_token = op_none;
}


void exbox_printer::add_char(char16_t c)
{
    if (v.empty() || !eis_str(v.back().get()) || !erefcnt_is1(v.back().get()))
    {
        std::string s;
        stdstring_pushback_char16(s, c);
        v.push_back(uex(emake_str_move(s)));
    }
    else
    {
        stdstring_pushback_char16(eto_str(v.back().get())->string, c);
    }
    prev_token = op_none;
}

void exbox_printer::add_byte(unsigned char c)
{
    unsigned char a = (c>>0) & 15;
    unsigned char b = (c>>4) & 15;
    add_char(a + (a > 9 ? 'A' - 10 : '0'));
    add_char(b + (b > 9 ? 'A' - 10 : '0'));
}


template <bool fancy> void exbox_printer::add_op(Operator op)
{
    switch (op)
    {
        case op_inf_GenPost:         //  //
            add_string("//");
            break;
        case op_inf_PatternTest:     //  ?
            add_string("?");
            break;
        case op_inf_Set:             //  =
            if (fancy)
            {
                add_string("=");
            }
            else
            {
                add_string(" = ");
                op = op_none;
            }            
            break;
        case op_inf_GenPre:          //  @
            add_string("@");
            break;
        case op_inf_Apply:           //  @@
            add_string("@@");
            break;
        case op_inf_Span:            //  ;;
            add_string(";;");
            break;
        case op_inf_Greater:         //  >
            if (fancy)
            {
                add_string(">");
            }
            else
            {
                add_string(" > ");
                op = op_none;
            }            
            break;
            break;
        case op_inf_Put:             //  >>
            add_string(">>");
            break;
        case op_inf_GreaterEqual:    //  >=
            add_string(u8"\u2265");
            break;
        case op_inf_Less:            //  <
            add_string("<");
            break;
        case op_inf_StringJoin:      //  <>
            add_string("<>");
            break;
        case op_inf_StringExpression:
            add_string("~~");
            break;

        case op_inf_LessEqual:       //  <=
            add_string(u8"\u2264");
            break;
        case op_bang:                //  !
        case op_pre_Not:             //  !
        case op_post_Factorial:      //  !
            if (prev_token == op_bang || prev_token == op_pre_Not || prev_token == op_post_Factorial)
                add_string(" ");
            add_string("!");
            break;
        case op_inf_Unequal:         //  !=
            if (prev_token == op_bang || prev_token == op_pre_Not || prev_token == op_post_Factorial)
                add_string(" ");
            add_string(u8"\u2260");
            break;
        case op_post_Factorial2:     //  !!
            if (prev_token == op_bang || prev_token == op_pre_Not || prev_token == op_post_Factorial)
                add_string(" ");
            add_string("!!");
            break;
        case op_post_Unset:          //  =.
            add_string("=.");
            break;
        case op_inf_Equal:           //  ==
            if (fancy)
            {
                add_string(u8"\uf431");
            }
            else
            {
                add_string(" == ");
                op = op_none;
            }
            break;
        case op_inf_SameQ:           //  ===
            if (fancy)
            {
                add_string("===");
            }
            else
            {
                add_string(" === ");
                op = op_none;
            }
            break;
        case op_inf_UnsameQ:         //  =!=
            if (fancy)
            {
                add_string("=!=");
            }
            else
            {
                add_string(" =!= ");
                op = op_none;
            }
            break;

        case op_inf_Divide:          //  /
            add_string("/");
            break;
        case op_inf_DivideBy:        //  /=
            add_string("/=");
            break;
        case op_inf_Map:             //  /@
            add_string("/@");
            break;
        case op_inf_ReplaceAll:      //  /.
            if (fancy)
            {
                add_string("/.");
            }
            else
            {
                add_string(" /. ");
                op = op_none;
            }
            break;
        case op_inf_MapAll:          //  //@
            add_string("//@");
            break;
        case op_inf_ReplaceRepeated: //  //.
            add_string("//.");
            break;
        case op_inf_Power:           //  ^
            add_string("^");
            break;
        case op_inf_UpSetDelayed:    //  ^:=
            if (fancy)
            {
                add_string("^:=");
            }
            else
            {
                add_string(" ^:= ");
                op = op_none;
            }            
            break;
        case op_inf_UpSet:           //  ^=
            if (fancy)
            {
                add_string("^=");
            }
            else
            {
                add_string(" ^= ");
                op = op_none;
            }            
            break;
        case op_inf_Times:           //  *
            if (fancy)
            {
                add_string(" ");
            }
            else
            {
                add_string("*");
            }
            break;

        case op_inf_TimesBy:         //  *=
            add_string("*=");
            break;
        case op_inf_NonCommutativeMultiply:  // **
            add_string("**");
            break;
        case op_inf_Rule:            //  ->
            if (fancy)
            {
                add_string(u8"\uf522");
            }
            else
            {
                add_string(" -> ");
                op = op_none;
            }
            break;
        case op_inf_SubtractFrom:    //  -=
            add_string("-=");
            break;
        case op_inf_AddTo:           //  +=
            add_string("+=");
            break;
        case op_inf_MessageName:     //  ::
            add_string("::");
            break;
        case op_inf_SetDelayed:      //  :=
            if (fancy)
            {
                add_string(":=");
            }
            else
            {
                add_string(" := ");
                op = op_none;
            }            
            break;
        case op_inf_RuleDelayed:     //  :>
            if (fancy)
            {
                add_string(u8"\uf51f");
            }
            else
            {
                add_string(" :> ");
                op = op_none;
            }
            break;
        case op_post_Derivative:     //  ' n times
            add_string("'");
            break;
        case op_inf_Alternatives:    //  |
            add_string("|");
            break;

        case op_inf_Or:              //  ||
            if (fancy)
            {
                add_string("||");
            }
            else
            {
                add_string(" || ");
                op = op_none;
            }            
            break;
            break;
        case op_inf_Dot:             //  .
            add_string(".");
            break;
        case op_inf_Condition:       //  /;
            if (fancy)
            {
                add_string("/;");
            }
            else
            {
                add_string(" /; ");
                op = op_none;
            }            
            break;
            break;
        case op_post_Repeated:       //  ..
            if (prev_token == op_post_Repeated || prev_token == op_post_RepeatedNull)
                add_string(" ");
            add_string("..");
            break;
        case op_post_RepeatedNull:   //  ...
            if (prev_token == op_post_Repeated || prev_token == op_post_RepeatedNull)
                add_string(" ");
            add_string("...");
            break;
        case op_inf_tag:             //  /:
            if (fancy)
            {
                add_string("/:");
            }
            else
            {
                add_string(" /: ");
                op = op_none;
            }            
            break;
        case op_post_Function:       //  &
            add_string("&");
            break;
        case op_inf_And:             //  &&
            add_string("&&");
            break;
        case op_CompoundExpression:
        case op_inf_CompoundExpression:
			if (fancy)
			{
            	add_string(";");
			}
			else
			{
            	add_string("; ");
				op =  op_none;
			}
            break;
        case op_postd_CompoundExpression: //  ;
            add_string(";");
            break;

        case op_Plus:
        case op_pre_Plus:
        case op_inf_Plus:                      //  +
            if (prev_token == op_inf_Plus || prev_token == op_post_Increment)
                add_string(" ");
            if (fancy)
            {
                add_string("+");
            }
            else
            {
                add_string(" + ");
                op = op_none;
            }            
            break;
        case op_Minus:
        case op_inf_Minus:                   //  -
            if (fancy)
            {
                add_string("-");
            }
            else
            {
                add_string(" - ");
                op = op_none;
            }            
            break;
        case op_pre_Minus:
            if (fancy && prev_token == op_inf_Times)
			{
                add_string("* ");
			}
            add_string("-");      
            break;
        case op_Decrement:
        case op_pre_PreDecrement:
        case op_post_Decrement:   //  --
            add_string("--");
            break;
        case op_pre_Sum:
            add_string("???");
            break;

        case op_pre_Product:
            add_string("???");
            break;
        case op_Increment:
        case op_pre_PreIncrement:
        case op_post_Increment:   //  ++
            add_string("++");
            break;
        case op_Pattern:
        case op_inf_Pattern:
        case op_inf_Optional:            //  :
            add_string(":");
            break;
        case op_post_SuperscriptBox:
        case op_post_SubscriptBox:
        case op_post_SubsuperscriptBox:
            add_string("???");
            break;

        case opComma:
            if (fancy)
            {
                add_string(",");
            }
            else
            {
                add_string(", ");
                op = op_none;
            }            
            break;
        case bracLRound:
            add_string("(");
            break;
        case bracRRound:
            add_string(")");
            break;
        case bracLSquare:
            add_string("[");
            break;
        case bracRSquare:
            add_string("]");
            break;
        case bracLCurly:
            add_string("{");
            break;
        case bracRCurly:
            add_string("}");
            break;
        case bracLDouble:
            add_string("[[");
            break;
        case bracRDouble:
            add_string("]]");
            break;
        case bracLAssociation:
            add_string("<|");
            break;
        case bracRAssociation:
            add_string("|>");
            break;

        case bracLAssociationSpecial:
            if (fancy)
            {
                add_string(u8"\uf113");
            }
            else
            {
                add_string("<|");
                op = bracLAssociation;
            }            
            break;
        case bracRAssociationSpecial:
            if (fancy)
            {
                add_string(u8"\uf114");
            }
            else
            {
                add_string("|>");
                op = bracLAssociation;
            }            
            break;
        case bracLDoubleSpecial:
            if (fancy)
            {
                add_string(u8"\u301a");
            }
            else
            {
                add_string("[[");
                op = bracLAssociation;
            }            
            break;
        case bracRDoubleSpecial:
            if (fancy)
            {
                add_string(u8"\u301b");
            }
            else
            {
                add_string("]]");
                op = bracLAssociation;
            }            
            break;
        case op_pre_Integrate:
            add_string(u8"\u222b");
            break;
        case op_pre_DifferentialD:
            add_string(u8"\uf74c");
            break;

        case opEnd:
        case op_none:
            add_string("???");
            break;
    }

    prev_token = op;
}


void exbox_printer::print_inf_op(Operator op, er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    assert(len >= 2);
    prec = infix_prec(op);
    assert(prec != prec_none);
    needp = prec <= l || prec <= r;
    if (needp) {l = r = prec_lowest;}
    if (needp) {add_string("(");}
    switch (op_asso(op))
    {
        case assoNo:
        {
            assert(len == 2);
            print_ex(echild(e,1), l, prec - 1, &OQ);
            add_op(op, OP);
            print_ex(echild(e,2), prec - 1, r, &OQ);
            break;
        }
        case assoLeft:
        {
            assert(len == 2);
            print_ex(echild(e,1), l, prec - 1, &OQ);
            add_op(op, &OQ);
            print_ex(echild(e,2), prec, r, &OQ);
            break;
        }
        case assoRight:
        {
            assert(len == 2);
            print_ex(echild(e,1), l, prec, &OQ);
            add_op(op, &OQ);
            print_ex(echild(e,2), prec - 1, r, &OQ);
            break;
        }
        default:
        {
            print_ex(echild(e,1), l, prec, &OQ);
            add_op(op, &OQ);
            for (size_t i = 2; i < len; i++)
            {
                print_ex(echild(e,i), prec, prec, &OQ);
                add_op(op, &OQ);
            }
            print_ex(echild(e,len), prec, r, &OQ);
        }
    }
    if (needp) {add_string(")");}
}


void exbox_printer::print_Plus(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
//std::cout << "print_Plus: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sPlus.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (len < 2)
        return print_gen(e, l, r, OP);

    uex f(gs.sym_sPlus.get(), len);
    std::vector<uex> last;
    last.push_back(eval_with_minus(ecopychild(e,1)));
    for (size_t i = 2; i <= len; i++)
    {
        assert(!last.empty());
        uex cur(eval_with_minus(ecopychild(e,i)));
        if (ehas_head_sym_length(cur.get(), gs.sym_sMinus.get(), 1))
        {
            cur.reset(cur.copychild(1));
            last.push_back(std::move(cur));
        }
        else
        {
            f.push_back(last.size() == 1 ? last[0].copy()
                                         : emake_node(gs.sym_sMinus.copy(), last));
            last.clear();
            last.push_back(std::move(cur));
        }
    }
    f.push_back(last.size() == 1 ? last[0].copy()
                                 : emake_node(gs.sym_sMinus.copy(), last));
    last.clear();

    e = f.get();
    len = elength(e);

    assert(len > 0);
    if (elength(e) == 1)
    {
        print_ex(echild(e,1), l, r, &OQ);
    }
    else
    {
        prec = prec_inf_Plus;
        needp = prec <= l || prec <= r;
        if (needp) {l = r = prec_lowest;}
        if (needp) {add_string("(");}
        print_ex(echild(e,1), l, prec, &OQ);
        add_op(op_inf_Plus, &OQ);
        for (size_t i = 2; i < len; i++)
        {
            print_ex(echild(e,i), prec, prec, &OQ);
            add_op(op_inf_Plus, &OQ);
        }
        print_ex(echild(e,len), prec, r, &OQ);
        if (needp) {add_string(")");}
    }
}


void exbox_printer::print_times(std::vector<uex> e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    size_t len = e.size();
    bool needp;
    Precedence prec;

    if (len == 0)
    {
        print_ex(eget_cint(1), l, r, OP);
    }
    else if (len == 1)
    {
        print_ex(e[0].get(), l, r, OP);
    }
    else
    {
        prec = prec_inf_Times;
        needp = prec <= l || prec <= r;
        if (needp) {l = r = prec_lowest;}
        if (needp) {add_string("(");}
        print_ex(e[0].get(), l, prec, OP);
        add_op(op_inf_Times, OP);
        for (size_t i = 1; i+1 < len; i++)
        {
            print_ex(e[i].get(), prec, prec, OP);
            add_op(op_inf_Times, OP);
        }
        print_ex(e[len - 1].get(), prec, r, OP);
        if (needp) {add_string(")");}
    }
}


void exbox_printer::print_Times(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
//std::cout << "print_Times: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sTimes.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (len < 2)
        return print_gen(e, l, r, OP);

    std::vector<uex> a;
    std::vector<uex> b;
    bool has_minus = false;
    for (size_t i = 1; i <= len; i++)
    {
        er ei = echild(e,i);
        if (ehas_head_sym_length(ei, gs.sym_sPower.get(), 2))
        {
            uex pow(eval_with_minus(ecopychild(ei,2)));
            if (ehas_head_sym_length(pow.get(), gs.sym_sMinus.get(), 1))
            {
                pow.reset(ecopychild(pow.get(),1));
                if (eis_one(pow.get()))
                {
                    b.push_back(uex(ecopychild(ei,1)));
                }
                else
                {
                    b.push_back(uex(emake_node(gs.sym_sPower.copy(), ecopychild(ei,1), pow.release())));
                }
            }
            else if (!eis_one(pow.get()))
            {
                a.push_back(ecopy(ei));
            }
            else
            {
                a.push_back(ecopychild(ei,1));
            }
        }
        else if (i == 1 && eis_number(ei))
        {
            uex num(eval_with_minus_divide(ecopy(ei)));
            uex den(emake_cint(1));
            if (ehas_head_sym_length(num.get(), gs.sym_sMinus.get(), 1))
            {
                has_minus = true;
                num.reset(ecopychild(num.get(),1));
            }
            if (ehas_head_sym_length(num.get(), gs.sym_sDivide.get(), 2))
            {
                den.reset(ecopychild(num.get(),2));
                num.reset(ecopychild(num.get(),1));
            }
            if (!eis_one(num.get()))
            {
                a.push_back(std::move(num));
            }
            if (!eis_one(den.get()))
            {
                b.push_back(std::move(den));
            }
        }
        else
        {
            a.push_back(uex(ecopy(ei)));
        }
    }

    if (a.empty())
    {
        a.push_back(uex(emake_cint(1)));
    }

    bool needp_minus = false;
    if (has_minus)
    {
        prec = prec_pre_Minus;
        needp_minus = (prec <= r);
        if (needp_minus) {l = r = prec_lowest;}
        if (needp_minus) {add_string("(");}
        add_op(op_pre_Minus, &OQ);
    }

    if (b.empty())
    {
        print_times(a, l, r, &OQ);
    }
    else if (!OP->useboxes())
    {
        prec = prec_inf_Divide;
        needp = prec <= l || prec <= r;
        if (needp) {l = r = prec_lowest;}
        if (needp) {add_string("(");}
        print_times(a, l, prec, &OQ);
        add_op(op_inf_Divide, &OQ);
        print_times(b, prec, r, &OQ);
        if (needp) {add_string(")");}
    }
    else if (   (OP->flags & bflag_script)
             && a.size() == 1 && eis_leaf(a[0].get())
             && b.size() == 1 && eis_leaf(b[0].get())   )
    {
        prec = prec_inf_Divide;
        needp = prec <= l || prec <= r;
        if (needp) {l = r = prec_lowest;}
        if (needp) {add_string("(");}
        print_ex(a[0].get(), l, prec, &OQ);
        add_op(op_inf_Divide, &OQ);
        print_ex(b[0].get(), prec, r, &OQ);
        if (needp) {add_string(")");}
    }
    else
    {
        prec = prec_inf_Divide;
        needp = prec <= l || prec <= r;
        if (needp) {l = r = prec_lowest;}
        if (needp) {add_string("(");}
        exbox_printer s1;
        s1.print_times(a, prec_lowest, prec_lowest, &OQ);
        exbox_printer s2;
        s2.print_times(b, prec_lowest, prec_lowest, &OQ);
        uex t1(s1.torowbox());
        ex t2 = s2.torowbox();
        add_ex(emake_node(ecopy(gs.sym_sFractionBox.get()), t1.release(), t2));
        if (needp) {add_string(")");}
    }
    if (needp_minus) {add_string(")");}
}

void exbox_printer::print_Graphics3D(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sGraphics3D.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;

    if (len != 10 || !(OP->flags & bflag_useboxes))
    {
        return print_gen(e, l, r, OP);
    }

    uex ee(ecopy(e));
    ee.replacechild(0, gs.sym_sGraphics3DBox.copy());
    add_ex(ee.release());
    return;
}

void exbox_printer::print_Sqrt(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sSqrt.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (len != 1)
    {
        return print_gen(e, l, r, OP);
    }

    if (!(OP->flags & bflag_useboxes))
    {
        return print_gen(e, l, r, OP);
    }

    exbox_printer s2;
    s2.print_ex(echild(e,1), prec_lowest, prec_lowest, &OQ);
    ex t2 = s2.torowbox();
    add_ex(emake_node(gs.sym_sSqrtBox.copy(), t2));
    return;
}

void exbox_printer::print_Power(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sPower.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (len != 2)
    {
        print_gen(e, l, r, OP);
        return;
    }

    uex pow(eval_with_minus(ecopychild(e,2)));

    if (ehas_head_sym_length(pow.get(), gs.sym_sMinus.get(), 1))
    {
        pow.reset(pow.copychild(1));
        if ((OP->flags & bflag_script)
             && eis_one(pow.get())
             && eis_leaf(echild(e,1)))
        {
            prec = prec_inf_Divide;
            needp = prec <= l || prec <= r;
            if (needp) {l = r = prec_lowest;}
            if (needp) {add_string("(");}
            print_ex(eget_cint(1), l, prec, &OQ);
            add_op(op_inf_Divide, &OQ);
            print_ex(echild(e,1), prec, r, &OQ);
            if (needp) {add_string(")");}
        }
        else
        {
            if (!(OP->flags & bflag_useboxes))
            {
                prec = prec_inf_Divide;
                needp = prec <= l || prec <= r;
                if (needp) {l = r = prec_lowest;}
                if (needp) {add_string("(");}
                print_ex(eget_cint(1), l, prec, &OQ);
                add_op(op_inf_Divide, &OQ);
                print_power(echild(e,1), pow.get(), prec, r, &OQ);
                if (needp) {add_string(")");}
            }
            else
            {
                exbox_printer s1;
                s1.print_ex(eget_cint(1), prec_lowest, prec_lowest, &OQ);
                exbox_printer s2;
                s2.print_power(echild(e,1), pow.get(), prec_lowest, prec_lowest, &OQ);
                uex t1(s1.torowbox());
                ex t2 = s2.torowbox();
                add_ex(emake_node(ecopy(gs.sym_sFractionBox.get()), t1.release(), t2));
            }
        }
    }
    else
    {
        print_power(echild(e,1), echild(e,2), l, r, &OQ);
    }
}

void exbox_printer::print_power(er base, er power, Precedence l, Precedence r, const exbox_print_options * OP)
{
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (eis_one(power))
    {
        print_ex(base, l, r, OP);
        return;    
    }

    if (!(OP->flags & bflag_useboxes))
    {
        if (ex_same(power, eget_crat(1,2)))
        {
            prec = prec_postd_squarebracket;
            needp = prec <= l;
            if (needp) {l = r = prec_lowest;}
            if (needp) {add_string("(");}
            print_ex(gs.sym_sSqrt.get(), l, prec, &OQ);
            add_op(bracLSquare, &OQ);
            print_ex(base, prec_lowest, prec_lowest, &OQ);
            add_op(bracRSquare, &OQ);
        }
        else
        {
            prec = prec_inf_Power;
            needp = prec <= l;
            needp = needp || prec <= r; // extra parenthesis to be nice
            if (needp) {l = r = prec_lowest;}
            if (needp) {add_string("(");}
            print_ex(base, l, prec, &OQ);
            add_op(op_inf_Power, &OQ);
            print_ex(power, prec - 1, r, &OQ);
        }
        if (needp) {add_string(")");}
        return;
    }

    exbox_printer s2;
    if (ex_same(power, eget_crat(1,2)))
    {
        s2.print_ex(base, prec_lowest, prec_lowest, &OQ);
        ex t2 = s2.torowbox();
        add_ex(emake_node(ecopy(gs.sym_sSqrtBox.get()), t2));
        return;
    }

    OQ.flags |= bflag_script;
    s2.print_ex(power, prec_lowest, prec_lowest, &OQ);
    OQ.flags &= ~bflag_script;

    if (ehas_head_sym(base, gs.sym_sSubscript.get()) && elength(base) >= 2)
    {
        prec = prec_post_SubsuperscriptBox;
        needp = prec <= l;
        needp = needp || prec <= r; // extra parenthesis to be nice
        if (needp) {l = r = prec_lowest;}
        if (needp) {add_string("(");}
        print_ex(echild(base,1), l, prec, &OQ);
        OQ.flags |= bflag_script;
        exbox_printer s3;
        for (size_t i = 2; i <= elength(base); i++)
        {
            s3.print_ex(echild(base,i), prec_lowest, prec_lowest, &OQ);
            if (i + 1 <= elength(base))
            {
                s3.add_op(opComma, &OQ);
            }
        }
        uex t2(s2.torowbox());
        ex t3 = s3.torowbox();
        add_ex(emake_node(ecopy(gs.sym_sSubsuperscriptBox.get()), t3, t2.release()));
    }
    else
    {
        prec = prec_post_SuperscriptBox;
        needp = prec <= l;
        needp = needp || prec <= r; // extra parenthesis to be nice
        if (needp) {l = r = prec_lowest;}
        if (needp) {add_string("(");}
        print_ex(base, l, prec, &OQ);
        ex t2 = s2.torowbox();
        add_ex(emake_node(ecopy(gs.sym_sSuperscriptBox.get()), t2));
    }
    if (needp) {add_string(")");}
}


void exbox_printer::print_Subscript(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sSubscript.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (len < 2)
        return print_gen(e, l, r, OP);

    prec = prec_post_SubscriptBox;
    needp = prec <= l;
    if (needp) {l = r = prec_lowest;}
    if (needp) {add_string("(");}
    print_ex(echild(e,1), l, prec, &OQ);
    OQ.flags |= bflag_script;
    exbox_printer s2;
    for (size_t i = 2; i <= len; i++)
    {
        s2.print_ex(echild(e,i), prec_lowest, prec_lowest, &OQ);
        if (i + 1 <= len)
        {
            s2.add_op(opComma, &OQ);
        }
    }
    ex t2 = s2.torowbox();
    add_ex(emake_node(ecopy(gs.sym_sSubscriptBox.get()), t2));
    if (needp) {add_string(")");}
    return;
}


void exbox_printer::print_Divide(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sDivide.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (len != 2)
    {
        print_gen(e, l, r, OP);
        return;
    }

    if (!(OP->flags & bflag_useboxes))
    {
        prec = prec_inf_Divide;
        needp = prec <= l || prec <= r;
        if (needp) {l = r = prec_lowest;}
        if (needp) {add_string("(");}
        print_ex(echild(e,1), l, prec, &OQ);
        add_op(op_inf_Divide, &OQ);
        print_ex(echild(e,2), prec, r, &OQ);
        if (needp) {add_string(")");}
        return;
    }
    else
    {
        prec = prec_inf_Divide;
        needp = prec <= l || prec <= r;
        if (needp) {add_string("(");}
        exbox_printer s1;
        s1.print_ex(echild(e,1), prec_lowest, prec_lowest, &OQ);
        uex t1(s1.torowbox());
        exbox_printer s2;
        s2.print_ex(echild(e,2), prec_lowest, prec_lowest, &OQ);
        ex t2 = s2.torowbox();
        add_ex(emake_node(ecopy(gs.sym_sFractionBox.get()), t1.release(), t2));
        if (needp) {add_string(")");}
        return;
    }
}

void exbox_printer::print_SequenceForm(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sSequenceForm.get()));
    size_t len = elength(e);

    for (size_t i = 1; i <= len; i++)
    {
        print_ex(echild(e,i), prec_lowest, prec_lowest, OP);
    }
    return;
}

void exbox_printer::print_Slot(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sSlot.get()));
    size_t len = elength(e);

    if (len == 1 && eis_int(echild(e,1))
                 && fmpz_sgn(eint_data(echild(e,1))) >= 0)
    {
        add_string("#");
        if (true || !eis_one(echild(e,1)))
        {
            add_string(eint_number(echild(e,1)).tostring());
        }
        return;
    }

    print_gen(e, l, r, OP);
    return;
}

void exbox_printer::print_SlotSequence(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sSlotSequence.get()));
    size_t len = elength(e);

    if (len == 1 && eis_int(echild(e,1))
                 && fmpz_sgn(eint_data(echild(e,1))) >= 0)
    {
        add_string("##");
        if (!eis_one(echild(e,1)))
        {
            add_string(eint_number(echild(e,1)).tostring());
        }
        return;
    }

    print_gen(e, l, r, OP);
    return;
}


void exbox_printer::print_Inequality(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sInequality.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (len >= 5 && eis_valid_inequality(e))
    {
        prec = prec_inf_Inequality;
        needp = prec <= l || prec <= r;
        if (needp) {l = r = prec_lowest;}
        if (needp) {add_string("(");}
        for (size_t i=1; i<len; i+=2)
        {
            print_ex(echild(e,i), i == 1 ? l : prec, prec, &OQ);
            add_op(eis_sym(echild(e,i+1),gs.sym_sSameQ.get())        ? op_inf_SameQ :
                   eis_sym(echild(e,i+1),gs.sym_sUnsameQ.get())      ? op_inf_UnsameQ :
                   eis_sym(echild(e,i+1),gs.sym_sEqual.get())        ? op_inf_Equal :
                   eis_sym(echild(e,i+1),gs.sym_sUnequal.get())      ? op_inf_Unequal :
                   eis_sym(echild(e,i+1),gs.sym_sGreater.get())      ? op_inf_Greater :
                   eis_sym(echild(e,i+1),gs.sym_sGreaterEqual.get()) ? op_inf_GreaterEqual :
                   eis_sym(echild(e,i+1),gs.sym_sLess.get())         ? op_inf_Less :
                                                                     op_inf_LessEqual
            , &OQ);
        }
        print_ex(echild(e,len), prec, r, &OQ);
        if (needp) {add_string(")");}
        return;
    }

    print_gen(e, l, r, OP);
    return;
}

void exbox_printer::print_TagUnset(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sTagUnset.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (len == 2)
    {
        prec = prec_inf_tag_Set_SetDelayed;
        needp = prec <= l || prec <= r;
        if (needp) {l = r = prec_lowest;}
        if (needp) {add_string("(");}
        print_ex(echild(e,1), l, prec, &OQ);
        add_op(op_inf_tag, &OQ);
        print_ex(echild(e,2), prec, prec, &OQ);
        add_op(op_post_Unset, &OQ);
        if (needp) {add_string(")");}
        return;
    }

    print_gen(e, l, r, OP);
    return;
}

void exbox_printer::print_TagSet(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sTagSet.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (len == 3)
    {
        prec = prec_inf_tag_Set_SetDelayed;
        needp = prec <= l || prec <= r;
        if (needp) {l = r = prec_lowest;}
        if (needp) {add_string("(");}
        print_ex(echild(e,1), l, prec, &OQ);
        add_op(op_inf_tag, &OQ);
        print_ex(echild(e,2), prec, prec, &OQ);
        add_op(op_inf_Set, &OQ);
        print_ex(echild(e,3), prec-1, r, &OQ);
        if (needp) {add_string(")");}
        return;
    }

    print_gen(e, l, r, OP);
    return;
}

void exbox_printer::print_TagSetDelayed(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sTagSetDelayed.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (len == 3)
    {
        prec = prec_inf_tag_Set_SetDelayed;
        needp = prec <= l || prec <= r;
        if (needp) {l = r = prec_lowest;}
        if (needp) {add_string("(");}
        print_ex(echild(e,1), l, prec, &OQ);
        add_op(op_inf_tag, &OQ);
        print_ex(echild(e,2), prec, prec, &OQ);
        add_op(op_inf_SetDelayed, &OQ);
        print_ex(echild(e,3), prec-1, r, &OQ);
        if (needp) {add_string(")");}
        return;
    }

    print_gen(e, l, r, OP);
    return;
}


void exbox_printer::print_List(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sList.get()));
    size_t len = elength(e);
    exbox_print_options OQ(OP);

    if ((OP->flags & bflag_matrix) && eis_matrix(e))
    {
        add_string("(");
        std::vector<uex> matrix;
        std::vector<uex> row;
        for (size_t j = 1; j <= elength(e); j++)
        {
            for (size_t i = 1; i <= elength(echild(e,j)); i++)
            {
                exbox_printer entry;
                entry.print_ex(echild(e,j,i), prec_lowest, prec_lowest, &OQ);
                row.push_back(uex(entry.torowbox()));
            }
            matrix.push_back(uex(emake_node(gs.sym_sList.copy(), row)));
            row.clear();
        }
        ex m = emake_node(gs.sym_sList.copy(), matrix);
        add_ex(emake_node(gs.sym_sGridBox.copy(), m));
        add_string(")");
        return;
    }
    else
    {
        add_string("{");
        for (size_t i = 1; i <= len; i++)
        {
            print_ex(echild(e,i), prec_lowest, prec_lowest, &OQ);
            if (i + 1 <= len)
            {
                add_op(opComma, &OQ);
            }
        }
        add_string("}");
        return;
    }
}


void exbox_printer::print_Pattern(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sPattern.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (len == 2 && eis_sym(echild(e,1)))
    {
        if (ehas_head_sym_length(echild(e,2), gs.sym_sBlank.get(), 0))
        {
            add_sym(echild(e,1));
            add_string("_");
        }
        else if (ehas_head_sym_length(echild(e,2), gs.sym_sBlankSequence.get(), 0))
        {
            add_sym(echild(e,1));
            add_string("__");
        }
        else if (ehas_head_sym_length(echild(e,2), gs.sym_sBlankNullSequence.get(), 0))
        {
            add_sym(echild(e,1));
            add_string("___");
        }
        else if (ehas_head_sym_length(echild(e,2), gs.sym_sBlank.get(), 1) && eis_sym(echild(e,2,1)))
        {
            add_sym(echild(e,1));
            add_string("_");
            add_sym(echild(e,2,1));
        }
        else if (ehas_head_sym_length(echild(e,2), gs.sym_sBlankSequence.get(), 1) && eis_sym(echild(e,2,1)))
        {
            add_sym(echild(e,1));
            add_string("__");
            add_sym(echild(e,2,1));
        }
        else if (ehas_head_sym_length(echild(e,2), gs.sym_sBlankNullSequence.get(), 1) && eis_sym(echild(e,2,1)))
        {
            add_sym(echild(e,1));
            add_string("___");
            add_sym(echild(e,2,1));
        }
        else
        {
            prec = prec_inf_Pattern;
            needp = prec <= l || prec <= r;
            if (needp) {l = r = prec_lowest;}
            if (needp) {add_string("(");}
            add_sym(echild(e,1));
            add_string(": ");
            print_ex(echild(e,2), prec, r, &OQ);
            if (needp) {add_string(")");}
        }
        return;
    }

    print_gen(e, l, r, OP);
    return;
}


void exbox_printer::print_Rotate(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sRotate.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (len == 2)
    {
        uex f(emake_node(
                gs.sym_sTimes.copy(),
                ecopychild(e,2),
                emake_int_ui(2147483648),
                emake_node(gs.sym_sPower.copy(), gs.sym_sPi.copy(), emake_cint(-1))
            ));
        f.reset(eval(f.release()));
        f.setnz(eval_num(f.get(), 53));
        uint32_t angle = 0;
        if (eis_number(f.get()))
        {
            double d = num_todouble(f.get());
            if (std::isfinite(d))
            {
                angle = fmod(d, 4294967296.0);
            }
        }
        exbox_printer s1;
        s1.print_ex(echild(e,1), prec_lowest, prec_lowest, &OQ);
        uex t1(s1.torowbox());
        ex t3 = emake_int_ui(-angle);
        add_ex(emake_node(gs.sym_sRotationBox.copy(), t1.release(), ecopychild(e,2), ecopy(etor(t3)), t3));
        return;
    }

    print_gen(e, l, r, OP);
    return;    
}


void exbox_printer::print_StringForm(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sStringForm.get()));
    size_t len = elength(e);
    exbox_print_options OQ(OP);

    if (len >= 1 && eis_str(echild(e,1)))
    {
        size_t n = eto_str(echild(e,1))->string.size();
        const unsigned char * a = (const unsigned char *) eto_str(echild(e,1))->string.c_str();
        size_t i = 0;
        bool in_number = false;
        size_t number = 0;
        while (i < n)
        {
            char16_t c;
            i += readonechar16(c, a + i);
            if (c == '`')
            {
                if (in_number)
                {
                    if (number + 1 <= len)
                    {
                        er b = echild(e, number + 1);
                        if (eis_str(b))
                        {
                            add_string(estr_string(b));
                        }
                        else if (ehas_head_sym_length(b, gs.sym_sRowBox.get(), 1)
                                 && ehas_head_sym(echild(b,1), gs.sym_sList.get()))
                        {
                            b = echild(b,1);
                            for (size_t k = 1; k <= elength(b); k++)
                            {
                                add_ex(ecopychild(b,k));
                            }
                        }
                        else
                        {
                            print_ex(b, prec_lowest, prec_lowest, &OQ);
                        }
                    }
                    else
                    {
                        add_char('?');
                    }
                }
                in_number = !in_number;
                number = 0;
            }
            else
            {
                if (in_number)
                {
                    number = 10 * number + c - '0';
                }
                else
                {
                    add_char(c);
                }
            }
        }
        return;
    }

    print_gen(e, l, r, OP);
    return;    
}


void exbox_printer::print_Integrate(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sIntegrate.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (len == 2)
    {
        prec = prec_pre_DifferentialD;
        needp = (prec <= r);
        if (needp) {l = r = prec_lowest;}
        if (needp) {add_string("(");}
        add_op(op_pre_Integrate, &OQ);
        if (ehas_head_sym_length(echild(e,2), gs.sym_sList.get(), 3))
        {
            exbox_printer s2;
            OQ.flags |= bflag_script;
            s2.print_ex(echild(e,2,2), prec_lowest, prec_lowest, &OQ);
            exbox_printer s3;
            s3.print_ex(echild(e,2,3), prec_lowest, prec_lowest, &OQ);
            OQ.flags &= ~bflag_script;
            uex t2(s2.torowbox());
            ex t3 = s3.torowbox();
            add_ex(emake_node(ecopy(gs.sym_sSubsuperscriptBox.get()), t2.release(), t3));
            print_ex(echild(e,1), prec_lowest, prec_lowest, &OQ);
            add_op(op_pre_DifferentialD, &OQ);
            print_ex(echild(e,2,1), prec, r, &OQ);
        }
        else
        {
            print_ex(echild(e,1), prec_lowest, prec_lowest, &OQ);
            add_op(op_pre_DifferentialD, &OQ);
            print_ex(echild(e,2), prec, r, &OQ);
        }
        if (needp) {add_string(")");}
        return;
    }

    print_gen(e, l, r, OP);
    return;    
}


void exbox_printer::print_SumProduct(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sSum.get()) || ehas_head_sym(e, gs.sym_sProduct.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    er h = echild(e,0);

    if (len == 2 && OQ.useboxes())
    {
        prec = (h == gs.sym_sSum.get()) ? prec_pre_Sum : prec_pre_Product;
        uex symb(emake_str(h == gs.sym_sSum.get() ? u8"\u2211" : u8"\u220f"));
        needp = (prec <= r);
        ex t1;
        if (needp) {l = r = prec_lowest;}
        if (needp) {add_string("(");}
        if (ehas_head_sym_length(echild(e,2), gs.sym_sList.get(), 2))
        {
            exbox_printer s2;
            s2.print_ex(echild(e,2,1), prec_lowest, prec_lowest, &OQ);
            exbox_printer s3;
            s3.print_ex(echild(e,2,2), prec_lowest, prec_lowest, &OQ);
            uex t2(s2.torowbox());
            ex t3 = s3.torowbox();
            t1 = emake_node(ecopy(gs.sym_sUnderoverscriptBox.get()), symb.release(), t2.release(), t3);
        }
        else if (ehas_head_sym_length(echild(e,2), gs.sym_sList.get(), 3))
        {
            exbox_printer s2;
            s2.print_ex(echild(e,2,1), prec_lowest, prec_inf_Set, &OQ);
            s2.add_op(op_inf_Set, &OQ);
            s2.print_ex(echild(e,2,2), prec_inf_Set - 1, prec_lowest, &OQ);
            exbox_printer s3;
            s3.print_ex(echild(e,2,3), prec_lowest, prec_lowest, &OQ);
            uex t2(s2.torowbox());
            ex t3 = s3.torowbox();
            t1 = emake_node(ecopy(gs.sym_sUnderoverscriptBox.get()), symb.release(), t2.release(), t3);
        }
        else
        {
            exbox_printer s2;
            s2.print_ex(echild(e,2), prec_lowest, prec_lowest, &OQ);
            ex t2 = s2.torowbox();
            t1 = emake_node(ecopy(gs.sym_sUnderscriptBox.get()), symb.release(), t2);
        }
        add_ex(t1);
        print_ex(echild(e,1), prec, r, &OQ);
        if (needp) {add_string(")");}
        return;
    }

    print_gen(e, l, r, OP);
    return;    
}


void exbox_printer::print_Blank(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sBlank.get()));
    size_t len = elength(e);

    if (len == 0 || (len == 1 && eis_sym(echild(e,1))))
    {
        add_string("_");
        if (len == 1)
            add_sym(echild(e,1));
        return;
    }

    print_gen(e, l, r, OP);
    return;    
}
void exbox_printer::print_BlankSequence(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sBlankSequence.get()));
    size_t len = elength(e);

    if (len == 0 || (len == 1 && eis_sym(echild(e,1))))
    {
        add_string("__");
        if (len == 1)
            add_sym(echild(e,1));
        return;
    }

    print_gen(e, l, r, OP);
    return;    
}
void exbox_printer::print_BlankNullSequence(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sBlankNullSequence.get()));
    size_t len = elength(e);

    if (len == 0 || (len == 1 && eis_sym(echild(e,1))))
    {
        add_string("___");
        if (len == 1)
            add_sym(echild(e,1));
        return;
    }

    print_gen(e, l, r, OP);
    return;    
}


void exbox_printer::print_Out(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sOut.get()));
    size_t len = elength(e);

    if (len == 1 && eis_int(echild(e,1)))
    {
        er x = echild(e,1);
        if (fmpz_cmp_si(eint_data(x), -10) > 0)
        {
            if (fmpz_sgn(eint_data(x)) < 0)
            {
                for (slong n = fmpz_get_si(eint_data(x)); n < 0; n++)
                {
                    add_char('%');
                }
            }
            else
            {
                add_char('%');
                add_string(eint_number(x).tostring());
            }
            return;
        }
    }
    else if (len == 0)
    {
        add_char('%');
        return;
    }

    print_gen(e, l, r, OP);
    return;
}

void exbox_printer::print_Part(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sPart.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (len > 0)
    {
        prec = prec_postd_doublesquarebracket;
        needp = prec <= l;
        if (needp) {l = r = prec_lowest;}
        if (needp) {add_string("(");}
        print_ex(echild(e,1), l, prec, &OQ);
        add_op(bracLDoubleSpecial, &OQ);
        for (size_t i = 2; i <= len; i++)
        {
            if (i > 2)
            {
                add_op(opComma, &OQ);
            }
            print_ex(echild(e,i), prec_lowest, prec_lowest, &OQ);
        }
        add_op(bracRDoubleSpecial, &OQ);
        if (needp) {add_string(")");}
        return;
    }

    print_gen(e, l, r, OP);
    return;    
}

void exbox_printer::print_MessageName(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sMessageName.get()));
    size_t len = elength(e);

    if (len == 2 && eis_sym(echild(e,1))
                 && eis_str_messagetag(echild(e,2)))
    {
        add_sym(echild(e,1));
        add_string("::");
        add_string(eto_str(echild(e,2))->string);
        return;
    }

    print_gen(e, l, r, OP);
    return;    
}

void exbox_printer::print_CompoundExpression(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sCompoundExpression.get()));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    if (len >= 2)
    {
        if (eis_sym(echild(e,len), gs.sym_sNull.get()))
        {
            prec = prec_postd_CompoundExpression;
            needp = prec <= l /*|| prec <= r*/;
            if (needp) {l = r = prec_lowest;}
            if (needp) {add_string("(");}
            print_ex(echild(e,1), l, prec, &OQ);
            for (size_t i = 2; i < len; i++)
            {
                add_op(op_inf_CompoundExpression, &OQ);
                print_ex(echild(e,i), prec, prec, &OQ);
            }
            add_op(op_postd_CompoundExpression, &OQ);
            if (needp) {add_string( ")");}
            return;
        }
        else
        {
            prec = prec_inf_CompoundExpression;
            needp = prec <= l || prec <= r;
            if (needp) {l = r = prec_lowest;}
            if (needp) {add_string("(");}
            print_ex(echild(e,1), l, prec, &OQ);
            add_op(op_inf_CompoundExpression, &OQ);
            for (size_t i = 2; i < len; i++)
            {
                print_ex(echild(e,i), prec, prec, &OQ);
                add_op(op_inf_CompoundExpression, &OQ);
            }
            print_ex(echild(e,len), prec, r, &OQ);
            if (needp) {add_string(")");}
            return;
        }
    }

    print_gen(e, l, r, OP);
    return;    
}


void exbox_printer::print_MatrixForm(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sMatrixForm.get()));
    size_t len = elength(e);
    exbox_print_options OQ(OP);

    if (len == 1)
    {
        OQ.flags |= bflag_matrix;
        print_ex(echild(e,1), l, r, &OQ);
        return;
    }

    print_gen(e, l, r, OP);
    return;
}

void exbox_printer::print_BaseForm(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sBaseForm.get()));
    size_t len = elength(e);
    exbox_print_options OQ(OP);

    slong t;
    if (len == 2 && eis_int(echild(e,2))
                 && fmpz_fits_si(eint_data(echild(e,2)))
                 && (t = fmpz_get_si(eint_data(echild(e,2))), 2 <= t && t <= 36))
    {
        OQ.number_base = t;
        print_ex(echild(e,1), l, r, &OQ);
        return;
    }

    print_gen(e, l, r, OP);
    return;
}

void exbox_printer::print_FullForm(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sFullForm.get()));
    size_t len = elength(e);
    exbox_print_options OQ(OP);

    OQ.flags &= ~(bflag_useops | bflag_useboxes);

    if (len == 1)
    {
        print_ex(echild(e,1), l, r, &OQ);
        return;
    }

    print_gen(e, l, r, OP);
    return;
}

void exbox_printer::print_InputForm(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sInputForm.get()));
    size_t len = elength(e);
    exbox_print_options OQ(OP);

    OQ.flags &= ~(bflag_useboxes);
    OQ.flags |= bflag_useascii;

    if (len == 1)
    {
        print_ex(echild(e,1), l, r, &OQ);
        return;
    }

    print_gen(e, l, r, OP);
    return;
}


void exbox_printer::print_HoldForm(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
    assert(ehas_head_sym(e, gs.sym_sHoldForm.get()));
    size_t len = elength(e);

    if (len == 1)
    {
        print_ex(echild(e,1), l, r, OP);
        return;
    }

    print_gen(e, l, r, OP);
    return;
}


void exbox_printer::print_gen(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
//std::cout << "print_gen: " << ex_tostring_full(e) << std::endl;
    assert(eis_node(e));
    size_t len = elength(e);
    bool needp;
    Precedence prec;
    exbox_print_options OQ(OP);

    prec = prec_postd_squarebracket;
    needp = prec <= l;
    if (needp) {l = r = prec_lowest;}
    if (needp) {add_string("(");}
    print_ex(echild(e,0), l, prec, &OQ);
    add_op(bracLSquare, &OQ);
    for (size_t i = 1; i <= len; i++)
    {
        print_ex(echild(e,i), prec_lowest, prec_lowest, &OQ);
        if (i + 1 <= len)
        {
            add_op(opComma, &OQ);
        }
    }
    add_op(bracRSquare, &OQ);
    if (needp) {add_string(")");}
}

/*
    print assuming we need to produce s with LW(s)>l and RW(s)>r
    LW(atom) = prec_highest        RW(atom) = prec_highest
    LW(op a) = prec_highest        RW(op a) = min(RW(a),P(op))
    LW(a op) = min(LW(a),P(op))    RW(a op) = prec_highest
    LW(a op b) = min(LW(a),P(op))  RW(a op b) = min(RW(b),P(op))
*/
void exbox_printer::print_ex(er e, Precedence l, Precedence r, const exbox_print_options * OP)
{
//std::cout << "exbox_printer::print_ex("<<l<<", "<<r<<", "<< (OP->flags&bflag_script) <<"): " << ex_tostring_full(e) << std::endl;
    bool needp;
    Precedence prec;

    if (eis_leaf(e))
    {
        switch (etype(e))
        {
            case ETYPE_SYM:
            {
                if (OP->fancyops() && eis_sym(e, gs.sym_sPi.get()))
                {
                    add_string(u8"\u03C0");
                }
                else if (OP->fancyops() && eis_sym(e, gs.sym_sE.get()))
                {
                    add_string(u8"\uf74d");
                }
                else if (OP->fancyops() && eis_sym(e, gs.sym_sInfinity.get()))
                {
                    add_string(u8"\u221e");
                }
                else if (OP->fancyops() && eis_sym(e, gs.sym_sI.get()))
                {
                    add_string(u8"\uf74e");
                }
                else
                {
                    if (!gs.sym_context_assumed(e))
                    {
                        add_string(esym_context_string(e));
                    }
                    add_string(esym_name_string(e));
                }
                break;
            }
            case ETYPE_STR:
            {
                if (OP->flags & bflag_useascii)
                {
                    std::string s("\"");
                    stdstring_append_cvtascii(s, eto_str(e)->string, true);
                    s.push_back('"');
                    add_string(s);
                }
                else
                {
                    add_string("\"");
                    add_string(eto_str(e)->string);
                    add_string("\"");
                }
                break;
            }
            case ETYPE_INT:
            {
                needp = (prec_pre_Minus <= r) && (fmpz_sgn(eint_data(e)) < 0);
                if (needp) {add_string("(");}
                if (OP->number_base != 10)
                {
                    add_string(uint_tostring(uint64_t(OP->number_base)));
                    add_string("^^");
                }
                add_string(eint_number(e).tostring(OP->number_base));
                if (needp) {add_string(")");}
                break;
            }
            case ETYPE_RAT:
            {
                bool neg = fmpq_sgn(erat_data(e)) < 0;
                uex a(emake_int_copy(fmpq_numref(erat_data(e))));
                uex b(emake_int_copy(fmpq_denref(erat_data(e))));
                needp = (prec_inf_Divide <= l) || (prec_inf_Divide <= r);
                needp = needp || ((prec_pre_Minus <= r) && neg);
                if (needp) {add_string("(");}
                if (!(OP->flags & bflag_useboxes))
                {                    
                    print_ex(a.get(), prec_lowest, prec_lowest, OP);
                    add_op(op_inf_Divide, OP);
                    print_ex(b.get(), prec_lowest, prec_lowest, OP);
                }
                else
                {
                    if (neg)
                    {
                        add_op(op_pre_Minus, OP);
                        a.setnz(num_Minus1(a.get()));
                    }
                    if (OP->flags & bflag_script)
                    {
                        print_ex(a.get(), prec_lowest, prec_lowest, OP);
                        add_op(op_inf_Divide, OP);
                        print_ex(b.get(), prec_lowest, prec_lowest, OP);
                    }
                    else
                    {
                        exbox_printer s1;
                        s1.print_ex(a.get(), prec_lowest, prec_lowest, OP);
                        exbox_printer s2;
                        s2.print_ex(b.get(), prec_lowest, prec_lowest, OP);
                        uex t1(s1.torowbox());
                        ex t2 = s2.torowbox();
                        add_ex(emake_node(ecopy(gs.sym_sFractionBox.get()), t1.release(), t2));
                    }
                }
                if (needp) {add_string(")");}
                break;
            }
            case ETYPE_CMPLX:
            {
                add_string("Complex[");// FIXME: may need parentheses
                print_ex(eto_cmplx(e)->real, prec_lowest, prec_lowest, OP);
                add_op(opComma, OP);
                print_ex(eto_cmplx(e)->imag, prec_lowest, prec_lowest, OP);
                add_op(bracRSquare, OP);
                break;
            }
            case ETYPE_NAN:
            {
                print_ex(eto_nan(e)->child, l, r, OP);
                break;
            }
            case ETYPE_REAL:
            {
                add_string(ereal_number(e).tostring());
                break;
            }
            case ETYPE_DOUBLE:
            {
                add_string(double_tostring(eto_double(e)->number));
                break;
            }
            case ETYPE_BIN:
            {
                if (OP->fancyops())
                    add_char(CHAR_LeftSkeleton);
                else
                    add_string("<<");
                add_string(uint_tostring(ebin_size(e)));
                add_string(" bytes ");
                if (ebin_size(e) <= 16)
                {
                    for (size_t i = 0; i < ebin_size(e); i++)
                        add_byte(ebin_data(e)[i]);
                }
                else
                {
                    for (size_t i = 0; i < 4; i++)
                        add_byte(ebin_data(e)[i]);
                    add_string("...");
                    for (size_t i = ebin_size(e)-4; i < ebin_size(e); i++)
                        add_byte(ebin_data(e)[i]);
                }
                if (OP->fancyops())
                    add_char(CHAR_RightSkeleton);
                else
                    add_string(">>");
                break;
            }
            case ETYPE_RAW:
            default:
                add_string("???");
        }
    }
    else if (eis_node(e))
    {
        Operator op;
        size_t len = elength(e);
        er h = echild(e,0);

        if (!(OP->flags & bflag_useops))
        {
            print_gen(e, l, r, OP);
            return;
        }

        // TODO: faster print fxn lookup
        if (eis_sym(h))
        {
#define SYM_CHECK(name) if (eis_sym(h, gs.sym_s##name.get())) return print_##name(e, l, r, OP);
#define SYM_CHECK2(name1, name2) if (eis_sym(h, gs.sym_s##name1.get()) || eis_sym(h, gs.sym_s##name2.get())) return print_##name1##name2(e, l, r, OP);
            SYM_CHECK(BaseForm)
            SYM_CHECK(Blank)
            SYM_CHECK(BlankNullSequence)
            SYM_CHECK(BlankSequence)
            SYM_CHECK(CompoundExpression)
            SYM_CHECK(Divide)
            SYM_CHECK(FullForm)
            SYM_CHECK(Graphics3D)
            SYM_CHECK(HoldForm)
            SYM_CHECK(Inequality)
            SYM_CHECK(InputForm)
            SYM_CHECK(Integrate)
            SYM_CHECK(List)
            SYM_CHECK(MatrixForm)
            SYM_CHECK(MessageName)
            SYM_CHECK(Out)
            SYM_CHECK(Plus)
            SYM_CHECK(Part)
            SYM_CHECK(Pattern)
            SYM_CHECK(Power)
            SYM_CHECK(Rotate)
            SYM_CHECK(SequenceForm)
            SYM_CHECK(Slot)
            SYM_CHECK(SlotSequence)
            SYM_CHECK(Sqrt)
            SYM_CHECK(StringForm)
            SYM_CHECK(Subscript)
            SYM_CHECK2(Sum, Product)
            SYM_CHECK(TagSet)
            SYM_CHECK(TagSetDelayed)
            SYM_CHECK(TagUnset)
            SYM_CHECK(Times)
#undef SYM_CHECK
#undef SYM_CHECK2

            // do we have prefix notation for this head?
            if ((op = ex_to_prefix(e)) != op_none)
            {
                assert(len == 1);
                prec = prefix_prec(op);
                assert(prec != prec_none);
                needp = (prec <= r);
                if (needp) {l = r = prec_lowest;}
                if (needp) {add_string("(");}
                add_op(op, OP);
                print_ex(echild(e,1), prec, r, OP);
                if (needp) {add_string(")");}
                return;
            }
            // do we have postfix notation for this head?
            else if ((op = ex_to_postfix(e)) != op_none)
            {
                assert(len == 1);
                prec = postfix_prec(op);
                assert(prec != prec_none);
                needp = prec <= l;
                if (needp) {l = r = prec_lowest;}
                if (needp) {add_string("(");}
                print_ex(echild(e,1), l, prec, OP);
                add_op(op, OP);
                if (needp) {add_string(")");}
                return;
            }
            // do we have infix notation for this head?
            else if ((op = ex_to_infix(e)) != op_none)
            {
                print_inf_op(op, e, l, r, OP);
                return;
            }
        }

        print_gen(e, l, r, OP);
        return;
    }
    else if (eis_hmap(e))
    {
//std::cout << "exbox_printer::print_ex is_cont: " << ex_tostring_full(e) << std::endl;
        er f = ehmap_get_normal(e);
        add_op(bracLAssociationSpecial, OP);
        for (size_t i = 1; i <= elength(f); i++)
        {
            print_ex(echild(f,i), prec_lowest, prec_lowest, OP);
            if (i + 1 <= elength(f))
                add_op(opComma, OP);
        }
        add_op(bracRAssociationSpecial, OP);
    }
    else if (eis_parray(e))
    {
        slong rank = eparray_rank(e);
        if (rank == 1)
        {
            slong dim0 = eparray_dimension(e, 0);
            if (eparray_type(e) == 0)
            {
                slong * data = eparray_int_data(e);
                add_op(bracLCurly, OP);
                for (slong i = 0; i < dim0; i++)
                {
                    add_string(std::to_string(data[i]));
                    if (i + 1 < dim0)
                        add_op(opComma, OP);
                }
                add_op(bracRCurly, OP);
            }
            else
            {
                assert(false);
            }
        }
        else
        {
            assert(false);
        }
    }
    else
    {
        assert(false);
    }


}


std::string exbox_tostring(ex E)
{
    uex e(E);
    if (eis_str(E))
    {
        return estr_string(E);
    }
    else
    {
        std::string s("(*oops*)");
        s.append(ex_tostring_full(e.get()));
        return s;
    }
}

ex ex_to_exbox_standard(er e)
{
    exbox_printer b;
    exbox_print_options OP(bflag_useboxes | bflag_useops);
    b.print_ex(e, prec_lowest, prec_lowest, &OP);
    return b.torowbox();
}

ex ex_to_exbox_input(er e)
{
    exbox_printer b;
    exbox_print_options OP(bflag_useascii | bflag_useops);
    b.print_ex(e, prec_lowest, prec_lowest, &OP);
    return b.torowbox();
}

ex ex_to_exbox_full(er e)
{
    exbox_printer b;
    exbox_print_options OP(bflag_useascii);
    b.print_ex(e, prec_lowest, prec_lowest, &OP);
    return b.torowbox();
}

std::string ex_tostring(er e)
{
    return exbox_tostring(ex_to_exbox_input(e));
}

ex globalstate::in_prompt_standardform()
{
    exbox_printer b;
    exbox_print_options OP(bflag_useboxes | bflag_useops);
    b.add_string("In[");
    b.print_ex(gs.get_$Line(), prec_lowest, prec_lowest, &OP);
    b.add_string("]:=");
    return b.torowbox();
}

ex globalstate::out_prompt_standardform()
{
    exbox_printer b;
    exbox_print_options OP(bflag_useboxes | bflag_useops);
    b.add_string("Out[");
    b.print_ex(gs.get_$Line(), prec_lowest, prec_lowest, &OP);
    b.add_string("]=");
    return b.torowbox();    
}

std::string globalstate::in_prompt_string()
{
    exbox_printer b;
    exbox_print_options OP(bflag_useascii | bflag_useops);
    b.add_string("In[");
    b.print_ex(gs.get_$Line(), prec_lowest, prec_lowest, &OP);
    b.add_string("]:=");
    return exbox_tostring(b.torowbox());
}

std::string globalstate::out_prompt_string()
{
    exbox_printer b;
    exbox_print_options OP(bflag_useascii | bflag_useops);
    b.add_string("Out[");
    b.print_ex(gs.get_$Line(), prec_lowest, prec_lowest, &OP);
    b.add_string("]=");
    return exbox_tostring(b.torowbox());
}
