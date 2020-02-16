#include "types.h"
#include "ex.h"
#include "ex_parse.h"
#include "globalstate.h"

#include "named_chars.cpp"

// handlers for each stage of the lexer
typedef void lexfxn(eparser&p, int32_t c);
lexfxn  tokenize_start,
        tokenize_oper_lbracket,
        tokenize_oper_rbracket,
        tokenize_oper_lcurly,
        tokenize_oper_rcurly,
        tokenize_oper_lround,
        tokenize_oper_rround,
        tokenize_oper_plus,
        tokenize_oper_minus,
        tokenize_oper_colon, tokenize_oper_colon_colon,
        tokenize_oper_vbar,
        tokenize_oper_dot, tokenize_oper_dot_dot,
        tokenize_oper_star,
        tokenize_oper_carat, tokenize_oper_carat_colon,
        tokenize_oper_divide, tokenize_oper_divide_divide, tokenize_oper_divide_dot,
        tokenize_oper_equal, tokenize_oper_equal_bang, tokenize_oper_equal_equal,
        tokenize_oper_amp,
        tokenize_oper_bang,
        tokenize_oper_less,
        tokenize_oper_greater,
        tokenize_oper_tick, tokenize_oper_tick_after,
        tokenize_oper_semicolon,
        tokenize_oper_at,
        tokenize_oper_tilde,
        tokenize_num_start,
        tokenize_num_after_carat,
        tokenize_num_after_star,
        tokenize_num_after_tick,
        tokenize_num_after_mantissa,
        tokenize_num_after_prec,
        tokenize_num_after_exp,
        tokenize_num_hexmid_mantissa,
        tokenize_num_hexmid_after_star,
        tokenize_num_hexmid_exp,
        tokenize_num_hexspace_after_plus,
        tokenize_num_hexspace_after_slash,
        tokenize_num_hexrad_mantissa,
        tokenize_num_hexrad_after_star,
        tokenize_num_hexrad_exp,
        tokenize_percent_start,
        tokenize_percent_num,
        tokenize_percent_neg,
        tokenize_comment_start,
        tokenize_comment_after_paren,
        tokenize_comment_after_star,
        tokenize_str_start,
        tokenize_str_after_escape,
        tokenize_str_after_escape_hex,
        tokenize_sym_start,
        tokenize_sym_after_context,
        tokenize_slot_start,
        tokenize_slot_1_digit,
        tokenize_slot_1_alpha,
        tokenize_slot_2,
        tokenize_slot_2_digit,
        tokenize_blank_1,
        tokenize_blank_1_dot,
        tokenize_blank_2,
        tokenize_blank_3,
        tokenize_message_start;


// parser starts in the "start" stage
eparser::eparser()
{
    stage = &tokenize_start;
    error = erNone;
    comment_level = 0;
    toplevel = false;
}

eparser::eparser(bool toplevel_)
{
    stage = &tokenize_start;
    error = erNone;
    comment_level = 0;
    toplevel = toplevel_;
}


// see if the stack represents one complete expression and push it on out if so
void eparser::try_newexpr()
{
    if (!toplevel || !bstack.empty())
    {
        return;
    }

    std::vector<wex> stack_save(estack);
    uex prevTk_save, currTk_save, nextTk_save;
    prevTk_save.set(prevTk.copynull());
    currTk_save.set(currTk.copynull());
    nextTk_save.set(nextTk.copynull());

    handle_token_raw(opEnd);
    popop(prec_lowest);

    if (have_one_ex())
    {
        out.push_back(uex(estack.back().copy()));
        estack.pop_back();
        prevTk.reset(nullptr);
        currTk.reset(nullptr);
        nextTk.reset(nullptr);
    }
    else
    {
        estack = stack_save;
        prevTk.reset(prevTk_save.release());
        currTk.reset(currTk_save.release());
        nextTk.reset(nextTk_save.release());
    }
    error = erNone;
}



void tokenize_start(eparser&p, int32_t c)
{
//std::cout << "tokenize_start("<<c<<" = "<<char(c)<<"), error = " << error << std::endl;
//std::cout << "stack <" <<std::endl; print_exstack(estack);

    p.store.clear();

    p.stage = &tokenize_start;

    if (p.error) {return;}

    if (c < 0)
    {
        if (c == CHAR_NEWLINE)
        {
            p.try_newexpr();
        }
        return;
    }
    else if (isletterchar(c))
    {
        p.stage = &tokenize_sym_start;
        p.sym_context.clear();
        p.sym_name.clear();
        p.store.push_back(c);
        return;
    }
    else if (isopchar(c))
    {
        // TODO disbatch without so much if-then
        if (c < 128)
        {
            if (c == ',')
            {
                p.handle_token_ex(emake_raw(opComma));
                return;
            }

            if (c == '?')
            {
                p.handle_token_ex(emake_raw(op_inf_PatternTest));
                return;
            }

            p.stage = c == '[' ? &tokenize_oper_lbracket : 
                      c == ']' ? &tokenize_oper_rbracket : 
                      c == '{' ? &tokenize_oper_lcurly : 
                      c == '}' ? &tokenize_oper_rcurly : 
                      c == '(' ? &tokenize_oper_lround : 
                      c == ')' ? &tokenize_oper_rround : 
                      c == '+' ? &tokenize_oper_plus : 
                      c == '-' ? &tokenize_oper_minus : 
                      c == ':' ? &tokenize_oper_colon : 
                      c == '|' ? &tokenize_oper_vbar : 
                      c == '.' ? &tokenize_oper_dot : 
                      c == '*' ? &tokenize_oper_star : 
                      c == '^' ? &tokenize_oper_carat : 
                      c == '/' ? &tokenize_oper_divide : 
                      c == '=' ? &tokenize_oper_equal : 
                      c == '&' ? &tokenize_oper_amp : 
                      c == '!' ? &tokenize_oper_bang : 
                      c == '<' ? &tokenize_oper_less : 
                      c == '>' ? &tokenize_oper_greater : 
                      c == '\'' ? &tokenize_oper_tick : 
                      c == ';' ? &tokenize_oper_semicolon : 
                      c == '@' ? &tokenize_oper_at : 
                      c == '~' ? &tokenize_oper_tilde : 
                                 static_cast<lexfxn*>(nullptr);
            return;
        }
        else
        {
            if (c == CHAR_LeftDoubleBracket)
            {
                p.bstack.push_back(bracLDoubleSpecial);
                p.handle_token_ex(emake_raw(bracLDouble));
                return;
            }
            else if (c == CHAR_RightDoubleBracket)
            {
                if (p.bstack.empty() || p.bstack.back() != bracLDoubleSpecial)
                {
                    p.error = erBad;
                    return;
                }
                p.bstack.pop_back();
                p.handle_token_ex(emake_raw(bracRDouble));
                return;
            }
            else if (c == CHAR_LeftAssociation)
            {
                p.bstack.push_back(bracLAssociationSpecial);
                p.handle_token_ex(emake_raw(bracLAssociation));
                return;
            }
            else if (c == CHAR_RightAssociation)
            {
                if (p.bstack.empty() || p.bstack.back() != bracLAssociationSpecial)
                {
                    p.error = erBad;
                    return;
                }
                p.bstack.pop_back();        
                p.handle_token_ex(emake_raw(bracRAssociation));
                return;
            }
            else if (c == CHAR_Rule)
            {
                p.handle_token_ex(emake_raw(op_inf_Rule));
                return;
            }
            else if (c == CHAR_RuleDelayed)
            {
                p.handle_token_ex(emake_raw(op_inf_RuleDelayed));
                return;
            }
            else if (c == CHAR_Equal)
            {
                p.handle_token_ex(emake_raw(op_inf_Equal));
                return;
            }
            else if (c == CHAR_NotEqual)
            {
                p.handle_token_ex(emake_raw(op_inf_Unequal));
                return;
            }
            else if (c == CHAR_LessEqual)
            {
                p.handle_token_ex(emake_raw(op_inf_LessEqual));
                return;
            }
            else if (c == CHAR_GreaterEqual)
            {
                p.handle_token_ex(emake_raw(op_inf_GreaterEqual));
                return;
            }
            else if (c == CHAR_Integral)
            {
                ex t = emake_node(gs.sym_sIntegrate.copy(), gs.sym_sNull.copy(), gs.sym_sNull.copy());
                p.handle_token_ex(emake_raw(op_pre_Integrate, 0, t));
                return;
            }
            else if (c == CHAR_DifferentialD)
            {
                p.handle_token_ex(emake_raw(op_pre_DifferentialD));
                return;
            }
            else
            {
                assert(false);
                return;
            }
        }
    }
    else if ('0' <= c && c <= '9')
    {
        p.stage = &tokenize_num_start;
        p.num_tick_count = 0;
        p.digits_after_dot = -1;
        p.num_hex = false;
        p.num_base.clear();
        p.num_mantissa.clear();
        p.num_prec.clear();
        p.num_exp.clear();
        p.store.push_back(c);
        return;
    }
    else if (c == '_')
    {
        assert(p.pattern_name.get() == nullptr);
        assert(p.blank_type.get() == nullptr);
        p.blank_type.reset(gs.sym_sBlank.copy());
        p.stage = &tokenize_blank_1;
        return;
    }
    else if (c == '"')
    {
        p.stage = &tokenize_str_start;
        return;
    }
    else if (c == '#')
    {
        p.stage = &tokenize_slot_start;
        return;
    }
    else if (c == '%')
    {
        p.digits_after_dot = 1; // number of % read
        p.stage = &tokenize_percent_start;
        return;
    }
    else if (c == ' ' || c == '\n' || c == '\t' || c == '\r')
    {
        if (c == '\n')
        {
            p.try_newexpr();
        }
        return;
    }
    else
    {
        p.error = erBad;
        return;
    }
}

// have read [
void tokenize_oper_lbracket(eparser&p, int32_t c)
{
    if (c == '[')
    {
        p.bstack.push_back(bracLDouble);
        p.handle_token_ex(emake_raw(bracLDouble));
        p.stage = &tokenize_start;
        return;
    }
    else
    {
        p.bstack.push_back(bracLSquare);
        p.handle_token_ex(emake_raw(bracLSquare));
        tokenize_start(p, c);
        return;
    }
}

// have read ]
void tokenize_oper_rbracket(eparser&p, int32_t c)
{
    if (p.bstack.empty())
    {
        p.error = erBad;
        return;
    }
    else if (p.bstack.back() == bracLSquare)
    {
        p.bstack.pop_back();
        p.handle_token_ex(emake_raw(bracRSquare));
        tokenize_start(p, c);
        return;
    }
    else if (c == ']' && p.bstack.back() == bracLDouble)
    {
        p.bstack.pop_back();
        p.handle_token_ex(emake_raw(bracRDouble));
        p.stage = &tokenize_start;
        return;
    }
    else
    {
        p.error = erBad;
        return;
    }
}

// have read {
void tokenize_oper_lcurly(eparser&p, int32_t c)
{
    p.bstack.push_back(bracLCurly);
    p.handle_token_ex(emake_raw(bracLCurly));
    tokenize_start(p, c);
    return;
}

// have read }
void tokenize_oper_rcurly(eparser&p, int32_t c)
{
    if (p.bstack.empty() || p.bstack.back() != bracLCurly)
    {
        p.error = erBad;
        return;
    }
    p.bstack.pop_back();
    p.handle_token_ex(emake_raw(bracRCurly));
    tokenize_start(p, c);
    return;
}

// have read (
void tokenize_oper_lround(eparser&p, int32_t c)
{
    if (c == '*')
    {
        p.stage = &tokenize_comment_start;
        p.comment_level = 1;
        return;
    }
    else
    {
        p.bstack.push_back(bracLRound);
        p.handle_token_ex(emake_raw(bracLRound));
        tokenize_start(p, c);
        return;
    }
}

// have read )
void tokenize_oper_rround(eparser&p, int32_t c)
{
    if (p.bstack.empty() || p.bstack.back() != bracLRound)
    {
        p.error = erBad;
        return;
    }
    p.bstack.pop_back();
    p.handle_token_ex(emake_raw(bracRRound));
    tokenize_start(p, c);
    return;
}

// have read +
void tokenize_oper_plus(eparser&p, int32_t c)
{
    if (c == '=')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_AddTo));
        return;
    }
    else if (c == '+')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_Increment));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_Plus));
        tokenize_start(p, c);
        return;
    }
}

// have read -
void tokenize_oper_minus(eparser&p, int32_t c)
{
    if (c == '-')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_Decrement));
        return;
    }
    else if (c == '=')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_SubtractFrom));
        return;
    }
    else if (c == '>')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_Rule));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_Minus));
        tokenize_start(p, c);
        return;
    }
}

// have read :
void tokenize_oper_colon(eparser&p, int32_t c)
{
    if (c == '=')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_SetDelayed));
        return;
    }
    else if (c == '>')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_RuleDelayed));
        return;
    }
    else if (c == ':')
    {
        p.stage = &tokenize_oper_colon_colon;
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_Pattern));
        tokenize_start(p, c);
        return;
    }
}

// have read ::
void tokenize_oper_colon_colon(eparser&p, int32_t c)
{
    if (isletterchar(c))
    {
        p.store.clear();
        p.store.push_back(c);
        p.stage = &tokenize_message_start;
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_inf_MessageName));
        tokenize_start(p, c);
        return;
    }
}

// have read |
void tokenize_oper_vbar(eparser&p, int32_t c)
{
    if (c == '|')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_Or));
        return;
    }
    else if (c == '>')
    {
        if (p.bstack.empty() || p.bstack.back() != bracLAssociation)
        {
            p.error = erBad;
            return;
        }
        p.stage = &tokenize_start;
        p.bstack.pop_back();
        p.handle_token_ex(emake_raw(bracRAssociation));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_inf_Alternatives));
        tokenize_start(p, c);
        return;
    }
}

// have read .
void tokenize_oper_dot(eparser&p, int32_t c)
{
    if ('0' <= c && c <= '9')
    {
        p.stage = &tokenize_num_start;
        p.num_tick_count = 0;
        p.digits_after_dot = 1;
        p.num_hex = false;
        p.num_base.clear();
        p.num_mantissa.clear();
        p.num_mantissa.push_back(c);
        p.num_prec.clear();
        p.num_exp.clear();
        p.store.clear();
        p.stage = &tokenize_num_after_mantissa;
        return;
    }
    else if (c == '.')
    {
        p.stage = &tokenize_oper_dot_dot;
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_inf_Dot));
        tokenize_start(p, c);
        return;
    }
}

// have read ..
void tokenize_oper_dot_dot(eparser&p, int32_t c)
{
    if (c == '.')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_post_RepeatedNull));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_post_Repeated));
        tokenize_start(p, c);
        return;
    }
}

// have read *
void tokenize_oper_star(eparser&p, int32_t c)
{
    if (c == '*')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_NonCommutativeMultiply));
        return;
    }
    else if (c == '=')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_TimesBy));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_inf_Times));
        tokenize_start(p, c);
        return;
    }
}

// have read ^
void tokenize_oper_carat(eparser&p, int32_t c)
{
    if (c == '=')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_UpSet));
        return;
    }
    else if (c == ':')
    {
        p.stage = &tokenize_oper_carat_colon;
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_inf_Power));
        tokenize_start(p, c);
        return;
    }
}

// have read ^:
void tokenize_oper_carat_colon(eparser&p, int32_t c)
{
    if (c == '=')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_UpSetDelayed));
        return;
    }
    else
    {
        p.error = erBad;
        return;
    }
}

// have read /
void tokenize_oper_divide(eparser&p, int32_t c)
{
    if (c == '/')
    {
        p.stage = &tokenize_oper_divide_divide;
        return;
    }
    else if (c == ':')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_tag));
        return;
    }
    else if (c == ';')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_Condition));
        return;
    }
    else if (c == '.')
    {
        p.stage = &tokenize_oper_divide_dot;
        return;
    }
    else if (c == '@')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_Map));
        return;
    }
    else if (c == '=')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_DivideBy));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_inf_Divide));
        tokenize_start(p, c);
        return;
    }
}

// have read //
void tokenize_oper_divide_divide(eparser&p, int32_t c)
{
    if (c == '.')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_ReplaceRepeated));
        return;
    }
    else if (c == '@')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_MapAll));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_inf_GenPost));
        tokenize_start(p, c);
        return;
    }
}

// have read /.
void tokenize_oper_divide_dot(eparser&p, int32_t c)
{
    if ('0' <= c && c <= '9')
    {
        p.num_tick_count = 0;
        p.digits_after_dot = 1;
        p.num_hex = false;
        p.num_base.clear();
        p.num_mantissa.clear();
        p.num_mantissa.push_back(c);
        p.num_prec.clear();
        p.num_exp.clear();
        p.store.clear();
        p.stage = &tokenize_num_after_mantissa;
        p.handle_token_ex(emake_raw(op_inf_Divide));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_inf_ReplaceAll));
        tokenize_start(p, c);
        return;
    }
}

// have read =
void tokenize_oper_equal(eparser&p, int32_t c)
{
    if (c == '!')
    {
        p.stage = &tokenize_oper_equal_bang;
        return;
    }
    else if (c == '=')
    {
        p.stage = &tokenize_oper_equal_equal;
        return;
    }
    else if (c == '.')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_post_Unset));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_inf_Set));
        tokenize_start(p, c);
        return;
    }
}

// have read =!
void tokenize_oper_equal_bang(eparser&p, int32_t c)
{
    if (c == '=')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_UnsameQ));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_inf_Set));
        p.stage = &tokenize_oper_bang;
        tokenize_oper_bang(p, c);
        return;
    }
}

// have read ==
void tokenize_oper_equal_equal(eparser&p, int32_t c)
{
    if (c == '=')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_SameQ));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_inf_Equal));
        tokenize_start(p, c);
        return;
    }
}

// have read &
void tokenize_oper_amp(eparser&p, int32_t c)
{
    if (c == '&')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_And));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_post_Function));
        tokenize_start(p, c);
        return;
    }
}

// have read !
void tokenize_oper_bang(eparser&p, int32_t c)
{
    if (c == '!')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_post_Factorial2));
        return;
    }
    else if (c == '=')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_Unequal));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_bang));
        tokenize_start(p, c);
        return;
    }
}

// have read <
void tokenize_oper_less(eparser&p, int32_t c)
{
    if (c == '=')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_LessEqual));
        return;
    }
    else if (c == '>')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_StringJoin));
        return;
    }
    else if (c == '|')
    {
        p.stage = &tokenize_start;
        p.bstack.push_back(bracLAssociation);
        p.handle_token_ex(emake_raw(bracLAssociation));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_inf_Less));
        tokenize_start(p, c);
        return;
    }
}

// have read >
void tokenize_oper_greater(eparser&p, int32_t c)
{
    if (c == '=')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_GreaterEqual));
        return;
    }
    else if (c == '>')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_Put));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_inf_Greater));
        tokenize_start(p, c);
        return;
    }
}

// have read '
void tokenize_oper_tick(eparser&p, int32_t c)
{
    if (c != '\'')
    {
        p.handle_token_ex(emake_raw(op_post_Derivative, 1, 0));
        tokenize_start(p, c);
        return;
    }
    else
    {
        p.stage = &tokenize_oper_tick_after;
        fmpz_one(p.number.data);
        return;
    }
}

// have read ''
void tokenize_oper_tick_after(eparser&p, int32_t c)
{
    fmpz_add_ui(p.number.data, p.number.data, 1);
    if (c != '\'')
    {
        slong m = fmpz_get_si(p.number.data);
        p.handle_token_ex(emake_raw(op_post_Derivative, m, 0));
        tokenize_start(p, c);
    }
    return;
}

// have read ;
void tokenize_oper_semicolon(eparser&p, int32_t c)
{
    if (c == ';')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_Span));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_CompoundExpression));
        tokenize_start(p, c);
        return;
    }
}

// have read @
void tokenize_oper_at(eparser&p, int32_t c)
{
    if (c == '@')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_Apply));
        return;
    }
    else
    {
        p.handle_token_ex(emake_raw(op_inf_GenPre));
        tokenize_start(p, c);
        return;
    }
}

// have read ~
void tokenize_oper_tilde(eparser&p, int32_t c)
{
    if (c == '~')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_raw(op_inf_StringExpression));
        return;
    }
    else
    {
        p.error = erBad;
        return;
    }
}

// have read %
void tokenize_percent_start(eparser&p, int32_t c)
{    
    if (c == '%')
    {
        p.digits_after_dot = 2;
        p.stage = &tokenize_percent_neg;
    }
    else if ('0' <= c && c <= '9')
    {
        fmpz_set_ui(p.number.data, c - '0');
        p.stage = &tokenize_percent_num;
    }
    else
    {
        p.handle_token_ex(emake_node(gs.sym_sOut.copy()));
        tokenize_start(p, c);
        return;        
    }
}

// have read % and a digit
void tokenize_percent_num(eparser&p, int32_t c)
{    
    if ('0' <= c && c <= '9')
    {
        fmpz_mul_ui(p.number.data, p.number.data, 10);
        fmpz_add_ui(p.number.data, p.number.data, c - '0');
        p.stage = &tokenize_percent_num;
    }
    else
    {
        p.handle_token_ex(emake_node(gs.sym_sOut.copy(), emake_int_move(p.number)));
        tokenize_start(p, c);
        return;        
    }
}

// have read %%
void tokenize_percent_neg(eparser&p, int32_t c)
{    
    if (c == '%')
    {
        p.digits_after_dot++;
        p.stage = &tokenize_percent_neg;
    }
    else
    {
        p.handle_token_ex(emake_node(gs.sym_sOut.copy(), emake_int_si(-p.digits_after_dot)));
        tokenize_start(p, c);
        return;        
    }
}

// have read (*
void tokenize_comment_start(eparser&p, int32_t c)
{
    assert(p.comment_level > 0);
    if (c < 0)
    {
        if (c == CHAR_NONE)
        {
            p.error = erBad;
        }
    }
    else
    {
        if (c == '(')
        {
            p.stage = &tokenize_comment_after_paren;
        }
        else if (c == '*')
        {
            p.stage = &tokenize_comment_after_star;
        }
    }
}

// have read ( inside a comment
void tokenize_comment_after_paren(eparser&p, int32_t c)
{
    assert(p.comment_level > 0);
    p.stage = &tokenize_comment_start;
    if (c < 0)
    {
        if (c == CHAR_NONE)
        {
            p.error = erBad;
        }
    }
    else
    {
        if (c == '*')
        {
            p.comment_level++;
        }
    }
}

// have read * inside a comment
void tokenize_comment_after_star(eparser&p, int32_t c)
{
    assert(p.comment_level > 0);
    p.stage = &tokenize_comment_start;
    if (c < 0)
    {
        if (c == CHAR_NONE)
        {
            p.error = erBad;
        }
    }
    else if (c == ')')
    {
        p.comment_level--;
        if (p.comment_level <= 0)
        {
            p.stage = &tokenize_start;
        }
    }
}

// have read a digit
void tokenize_num_start(eparser&p, int32_t c)
{
//std::cout << "push_back_num_start " << c << std::endl;
    if ('0' <= c && c <= '9')
    {
        p.store.push_back(c);
        p.stage = &tokenize_num_start;
        return;
    }
    else if (c == '.')
    {
        p.num_mantissa.clear();
        stdstring_append_char16v(p.num_mantissa, p.store);
        p.store.clear();
        p.digits_after_dot = 0;
        p.stage = &tokenize_num_after_mantissa;
        return;
    }
    else if (c == '^')
    {
        p.num_base.clear();
        stdstring_append_char16v(p.num_base, p.store);
        p.store.clear();
        p.stage = &tokenize_num_after_carat;
        return;
    }
    else if (c == '*')
    {
        p.num_mantissa.clear();
        stdstring_append_char16v(p.num_mantissa, p.store);
        p.store.clear();
        p.stage = &tokenize_num_after_star;
        return;
    }
    else if (c == 'x' && p.store.size() == 1 && p.store[0] == '0')
    {
        p.num_mantissa.clear();
        p.num_mantissa2.clear();
        p.num_exp.clear();
        p.num_exp2.clear();
        p.digits_after_dot = -1;
        p.digits_after_dot2 = -1;
        p.stage = &tokenize_num_hexmid_mantissa;
        return;
    }
    else
    {
        p.num_mantissa.clear();
        stdstring_append_char16v(p.num_mantissa, p.store);
        p.store.clear();
        p.handle_token_ex(p.make_num());
        tokenize_start(p, c);
    }
    return;
}

// have read ^ in a number
void tokenize_num_after_carat(eparser&p, int32_t c)
{
//std::cout << "push_back_num_after_carat " << c << std::endl;
    if (c == '^')
    {
        p.digits_after_dot = -1;
        p.stage = &tokenize_num_after_mantissa;
        return;
    }
    else
    {
        p.num_mantissa.swap(p.num_base);
        p.num_base.clear();
        p.num_exp.clear();
        p.handle_token_ex(p.make_num());
        p.stage = &tokenize_oper_carat;
        tokenize_oper_carat(p, c);
        return;
    }
}

// have read * in a number
void tokenize_num_after_star(eparser&p, int32_t c)
{
//std::cout << "push_back_num_after_star " << c << std::endl;
    if (c=='^')
    {
        p.stage = &tokenize_num_after_exp;
        return;
    }
    else
    {
        p.handle_token_ex(p.make_num());
        p.stage = &tokenize_oper_star;
        tokenize_oper_star(p, c);
        return;
    }
}

// in the mantissa of a number
void tokenize_num_after_mantissa(eparser&p, int32_t c)
{
//std::cout << "push_back_num_after_mantissa " << c << std::endl;
//std::cout << "digits_after_dot " << digits_after_dot << std::endl;
    if (c == '.')
    {
        if (p.digits_after_dot >= 0)
        {
            if (p.digits_after_dot > 0)
            {
                p.num_exp.clear();
                p.handle_token_ex(p.make_num());
                tokenize_start(p, c);
                return;
            }
            else
            {
                p.digits_after_dot--;
                p.handle_token_ex(p.make_num());
                p.stage = &tokenize_oper_dot_dot;
                return;
            }
        }
        else
        {
            p.digits_after_dot = 0;
            return;
        }
    }
    else if (c == '`')
    {
        if (p.num_mantissa.empty()) {p.error = erBad; return;}
        p.num_tick_count = 1;
        p.stage = &tokenize_num_after_tick;
        return;
    }
    else if (c == '*')
    {
        if (p.num_mantissa.empty()) {p.error = erBad; return;}
        p.stage = &tokenize_num_after_star;
        return;
    }
    else if (   ('0' <= c && c <= '9')
			 || (p.num_mantissa.empty() && c == '-')
			 || (!p.num_base.empty() && (('a' <= c && c <= 'z')
                 || ('A' <= c && c <= 'Z')))
			)
    {
        if (p.digits_after_dot >= 0)
        {
            p.digits_after_dot++;
        }
        p.num_mantissa.push_back(c);
        return;
    }
    else
    {
        if (p.num_mantissa.empty() || p.num_mantissa == "-") {p.error = erBad; return;}
        p.handle_token_ex(p.make_num());
        tokenize_start(p, c);
        return;
    }
}

// have read ` inside a number
void tokenize_num_after_tick(eparser&p, int32_t c)
{
//std::cout << "push_back_num_after_tick " << c << std::endl;
    if (c == '`')
    {
        p.num_tick_count++;
        if (p.num_tick_count > 3)
        {
            p.error = erBad;
        }
        return;
    }
    else if (c == '*')
    {
        p.stage = &tokenize_num_after_star;
        return;
    }
    else if (('0' <= c && c <= '9') || c == '`' || c == '_')
    {
        p.stage = &tokenize_num_after_prec;
        p.num_prec.push_back(c);
        return;
    }
    else
    {
        p.handle_token_ex(p.make_num());
        tokenize_start(p, c);
        return;
    }
}

void tokenize_num_after_prec(eparser&p, int32_t c)
{
    if (c == '*')
    {
        p.stage = &tokenize_num_after_star;
        return;
    }
    else if ('0' <= c && c <= '9')
    {
        p.num_prec.push_back(c);
        return;
    }
    else
    {
        p.handle_token_ex(p.make_num());
        tokenize_start(p, c);
        return;
    }
}

void tokenize_num_after_exp(eparser&p, int32_t c)
{
//std::cout << "push_back_num_after_exp " << c << std::endl;
    if (('0' <= c && c <= '9') || (p.num_exp.empty() && c == '-'))
    {
        p.num_exp.push_back(c);
        return;
    }
    else
    {
        if (p.num_exp.empty() || p.num_exp == "-") {p.error = erBad; return;}
        p.handle_token_ex(p.make_num());
        tokenize_start(p, c);
        return;
    }
    return;
}

void tokenize_num_hexmid_mantissa(eparser&p, int32_t c)
{
//std::cout << "tokenize_num_hexmid_mantissa " << char(c) << std::endl;
    if (   ('0' <= c && c <= '9')
        || ('a' <= c && c <= 'f')
        || ('A' <= c && c <= 'F')
        || (p.num_mantissa.empty() && c == '-'))
    {
        p.num_mantissa.push_back(c);
        if (p.digits_after_dot >= 0)
        {
            p.digits_after_dot++;
        }
        return;
    }
    else if (c == '.')
    {
        if (p.digits_after_dot >= 0)
        {
            p.num_exp.clear();
            p.handle_token_ex(p.make_hexnum1());
            tokenize_start(p, c);
            return;
        }
        else
        {
            p.digits_after_dot = 0;
            return;
        }
    }
    else if (c == '*')
    {
        p.stage = &tokenize_num_hexmid_after_star;
        return;
    }
    else if (c == '+')
    {
        p.stage = &tokenize_num_hexspace_after_plus;
        return;
    }
    else
    {
        p.handle_token_ex(p.make_hexnum1());
        tokenize_start(p, c);
        return;
    }
}

void tokenize_num_hexmid_after_star(eparser&p, int32_t c)
{
//std::cout << "tokenize_num_hexmid_after_star " << char(c) << std::endl;
    if (c == '^')
    {
        p.stage = &tokenize_num_hexmid_exp;
        return;
    }
    else
    {
        p.handle_token_ex(p.make_hexnum1());
        p.stage = &tokenize_oper_star;
        tokenize_oper_star(p, c);
        return;
    }
}

void tokenize_num_hexmid_exp(eparser&p, int32_t c)
{
//std::cout << "tokenize_num_hexmid_exp " << char(c) << std::endl;
    if (('0' <= c && c <= '9') || (p.num_exp.empty() && c == '-'))
    {
        p.num_exp.push_back(c);
        return;
    }
    else if (c == '+')
    {
        p.stage = &tokenize_num_hexspace_after_plus;
        return;
    }
    else
    {
        p.handle_token_ex(p.make_hexnum1());
        tokenize_start(p, c);
        return;
    }
}


void tokenize_num_hexspace_after_plus(eparser&p, int32_t c)
{
//std::cout << "tokenize_num_hexspace_after_plus " << char(c) << std::endl;
    if (c == '/')
    {
        p.stage = &tokenize_num_hexspace_after_slash;
        return;
    }
    else
    {
        p.handle_token_ex(p.make_hexnum1());
        p.stage = &tokenize_oper_plus;
        tokenize_oper_plus(p, c);
        return;
    }
}

void tokenize_num_hexspace_after_slash(eparser&p, int32_t c)
{
//std::cout << "tokenize_num_hexspace_after_slash " << char(c) << std::endl;
    if (c == '-')
    {
        p.stage = &tokenize_num_hexrad_mantissa;
        return;
    }
    else
    {
        p.error = erBad;
        return;
    }
}

void tokenize_num_hexrad_mantissa(eparser&p, int32_t c)
{
//std::cout << "tokenize_num_hexrad_mantissa " << char(c) << std::endl;
    if (   ('0' <= c && c <= '9')
        || ('a' <= c && c <= 'f')
        || ('A' <= c && c <= 'F'))
    {
        p.num_mantissa2.push_back(c);
        if (p.digits_after_dot2 >= 0)
        {
            p.digits_after_dot2++;
        }
        return;
    }
    else if (c == '.')
    {
        if (p.digits_after_dot2 >= 0)
        {
            p.handle_token_ex(p.make_hexnum2());
            tokenize_start(p, c);
            return;
        }
        else
        {
            p.digits_after_dot2 = 0;
            return;
        }
    }
    else if (c == '*')
    {
        p.stage = &tokenize_num_hexrad_after_star;
        return;
    }
    else
    {
        p.handle_token_ex(p.make_hexnum2());
        tokenize_start(p, c);
        return;
    }
}

void tokenize_num_hexrad_after_star(eparser&p, int32_t c)
{
    if (c == '^')
    {
        p.stage = &tokenize_num_hexrad_exp;
        return;
    }
    else
    {
        p.handle_token_ex(p.make_hexnum2());
        p.stage = &tokenize_oper_star;
        tokenize_oper_star(p, c);
        return;
    }
}

void tokenize_num_hexrad_exp(eparser&p, int32_t c)
{
    if (('0' <= c && c <= '9') || (p.num_exp2.empty() && c == '-'))
    {
        p.num_exp2.push_back(c);
        return;
    }
    else
    {
        p.handle_token_ex(p.make_hexnum2());
        tokenize_start(p, c);
        return;
    }
}

// have read "
void tokenize_str_start(eparser&p, int32_t c)
{
//std::cout << "str start " << c << std::endl;
    if (c < 0)
    {
        if (c == CHAR_NEWLINE)
        {
            p.store.push_back(10); // hmm
        }
        else
        {
            p.error = erBad;
        }
        return;
    }
    else if (c == '\\')
    {
        p.stage = &tokenize_str_after_escape;
        return;
    }
    else if (c == '"')
    {
        p.stage = &tokenize_start;
        p.handle_token_ex(emake_str_char16v(p.store));
        return;
    }
    else
    {
        p.store.push_back(c);
        return;
    }
}

// have read \ inside a string
void tokenize_str_after_escape(eparser&p, int32_t c)
{
//std::cout << "str exacpe " << c << std::endl;
    p.stage = &tokenize_str_start;
    if (c == '\\' || c == '"')
    {
        p.store.push_back(c);
        return;
    }
    else if (c == 'n' || c == 't' || c == 'r')
    {
        p.store.push_back('\\');
        p.store.push_back(c);
        return;
    }
    else if (c=='x')
    {
        fmpz_zero(p.number.data);
        p.num_tick_count = 0;
        p.stage = &tokenize_str_after_escape_hex;
        return;
    }
    else
    {
        p.error = erBad;
        return;
    }
}

// have read \x inside a string
void tokenize_str_after_escape_hex(eparser&p, int32_t c)
{
    p.num_tick_count++;
//std::cout << "str exacpe hex " << char(c) << " tickcount: " << num_tick_count << std::endl;
    if (p.num_tick_count <= 4)
    {
        if ('0'<= c && c<='9')
        {
            fmpz_mul_ui(p.number.data, p.number.data, 16);
            fmpz_add_ui(p.number.data, p.number.data, c - '0');
        }
        else if ('A'<= c && c<='F')
        {
            fmpz_mul_ui(p.number.data, p.number.data, 16);
            fmpz_add_ui(p.number.data, p.number.data, c - 'A' + 10);
        }
        else if ('a'<= c && c<='f')
        {
            fmpz_mul_ui(p.number.data, p.number.data, 16);
            fmpz_add_ui(p.number.data, p.number.data, c - 'a' + 10);
        }
        else
        {
            p.error = erBad;
            return;
        }
    }
    else
    {
        ulong u = fmpz_get_ui(p.number.data);
        assert(u < 65536);
        p.store.push_back(char16_t(fmpz_get_ui(p.number.data)));
        p.stage = &tokenize_str_start;
        tokenize_str_start(p, c);
        return;
    }
}

// have read a letter
void tokenize_sym_start(eparser&p, int32_t c)
{
//std::cout << "tokenize_sym_start " << c << std::endl;
    assert(p.store.size() != 0);
    if ((c >= 0 && isletterchar(c)) || ('0' <= c && c <= '9'))
    {
        p.store.push_back(c);
        return;
    }
    else if (c == '`')
    {
        stdstring_append_char16v(p.sym_context, p.store);
        p.sym_context.push_back(c);
        p.store.clear();
        p.stage = &tokenize_sym_after_context;
        return;
    }
    else
    {
        p.sym_name.clear();
        stdstring_append_char16v(p.sym_name, p.store);
        uex u;
        if (p.sym_context.length() == 0)
        {
            u.reset(gs.emake_psym_from_name(p.sym_name.c_str()));
        }
        else
        {
            u.reset(gs.emake_psym_from_context_name(p.sym_context.c_str(), p.sym_name.c_str()));
        }
        p.sym_context.clear();
        p.sym_name.clear();
        p.store.clear();

        if (p.pattern_name.get() == nullptr)
        {
            if (p.blank_type.get() == nullptr)
            {
                if (c == '_')
                {
                    p.pattern_name.reset(u.release());
                    p.blank_type.reset(ecopy(gs.sym_sBlank.get()));
                    p.stage = &tokenize_blank_1;
                    return;
                }
            }
            else
            {
                u.reset(emake_node(p.blank_type.release(), u.release()));
            }
        }
        else
        {
            assert(p.blank_type.get() != nullptr);
            u.reset(emake_node(p.blank_type.release(), u.release()));
            u.reset(emake_node(gs.sym_sPattern.copy(), p.pattern_name.release(), u.release()));
        }
        p.handle_token_ex(u.release());

        tokenize_start(p, c);
        return;
    }
}

// have read `
void tokenize_sym_after_context(eparser&p, int32_t c)
{
//std::cout << "tokenize_sym_after_context " << c << std::endl;
    if (c >= 0 && isletterchar(c))
    {
        p.store.push_back(c);
        p.stage = &tokenize_sym_start;
        return;
    }
    else
    {
        p.error = erBad;
        return;
    }
}


// have read _
void tokenize_blank_1(eparser&p, int32_t c)
{
//std::cout << "tokenize_blank_1 " << c << std::endl;
    if (c == '.')
    {
        p.stage = &tokenize_blank_1_dot;
        return;
    }
    else if (c == '_')
    {
        p.stage = &tokenize_blank_2;
        p.blank_type.reset(gs.sym_sBlankSequence.copy());
        return;
    }
    else if (c >= 0 && isletterchar(c))
    {
        p.stage = &tokenize_sym_start;
        p.store.push_back(c);
        return;
    }
    else
    {
        p.blank_type.reset(emake_node(p.blank_type.release()));
        if (p.pattern_name.get() != nullptr)
        {
            p.blank_type.reset(emake_node(
                        gs.sym_sPattern.copy(),
                        p.pattern_name.release(),
                        p.blank_type.release()));
        }
        p.handle_token_ex(p.blank_type.release());
        tokenize_start(p, c);
        return;
    }
}

// have read _.
void tokenize_blank_1_dot(eparser&p, int32_t c)
{
    if (c == '.')
    {
        if (p.pattern_name.get() == nullptr)
        {
            p.handle_token_ex(
                emake_node(
                    p.blank_type.release())
            );
        }
        else
        {
            p.handle_token_ex(
                emake_node(
                    gs.sym_sPattern.copy(),
                    p.pattern_name.release(),
                    emake_node(
                        p.blank_type.release()))
            );
        }
        p.stage = &tokenize_oper_dot_dot;
        return;
    }
    else
    {
        if (p.pattern_name.get() == nullptr)
        {
            p.handle_token_ex(
                emake_node(
                    gs.sym_sOptional.copy(),
                    emake_node(
                        p.blank_type.release()))
            );
        }
        else
        {
            p.handle_token_ex(
                emake_node(
                    gs.sym_sOptional.copy(),
                    emake_node(
                        gs.sym_sPattern.copy(),
                        p.pattern_name.release(),
                        emake_node(
                            p.blank_type.release())))
            );
        }
        tokenize_start(p, c);
        return;
    }
}

// have read __
void tokenize_blank_2(eparser&p, int32_t c)
{
    if (c == '_')
    {
        p.blank_type.reset(gs.sym_sBlankNullSequence.copy());
        p.stage = &tokenize_blank_3;
        return;
    }
    else if (c >= 0 && isletterchar(c))
    {
        p.stage = &tokenize_sym_start;
        p.store.push_back(c);
        return;
    }
    else
    {
        if (p.pattern_name.get() == nullptr)
        {
            p.handle_token_ex(
                emake_node(
                    p.blank_type.release())
            );
        }
        else
        {
            p.handle_token_ex(
                emake_node(
                    gs.sym_sPattern.copy(),
                    p.pattern_name.release(),
                    emake_node(
                        p.blank_type.release()))
            );
        }
        tokenize_start(p, c);
        return;
    }
}

// have read ___
void tokenize_blank_3(eparser&p, int32_t c)
{
    if (isletterchar(c))
    {
        p.stage = &tokenize_sym_start;
        p.store.push_back(c);
        return;
    }
    else
    {
        if (p.pattern_name.get() == nullptr)
        {
            p.handle_token_ex(
                emake_node(
                    p.blank_type.release())
            );
        }
        else
        {
            p.handle_token_ex(
                emake_node(
                    gs.sym_sPattern.copy(),
                    p.pattern_name.release(),
                    emake_node(
                        p.blank_type.release()))
            );
        }
        tokenize_start(p, c);
        return;
    }
}

// have read #
void tokenize_slot_start(eparser&p, int32_t c)
{
    if ('0' <= c && c <= '9')
    {
        p.stage = &tokenize_slot_1_digit;
        fmpz_set_ui(p.number.data, c - '0');
        return;
    }
    else if (isletterchar(c))
    {
        p.stage = &tokenize_slot_1_alpha;
        p.store.push_back(c);
        return;
    }
    else if (c == '#')
    {
        p.stage = &tokenize_slot_2;
        return;
    }
    else
    {
        p.handle_token_ex(emake_node(gs.sym_sSlot.copy(), emake_cint(1)));
        tokenize_start(p, c);
        return;
    }
}


// have read # followed by one digit
void tokenize_slot_1_digit(eparser&p, int32_t c)
{
    if ('0' <= c && c <= '9')
    {
        fmpz_mul_ui(p.number.data, p.number.data, 10);
        fmpz_add_ui(p.number.data, p.number.data, c - '0');
        return;
    }
    else
    {
        ex a1 = emake_int_move(p.number);
        p.handle_token_ex(emake_node(gs.sym_sSlot.copy(), a1));
        tokenize_start(p, c);
        return;
    }
}

// have read # followed by one letter
void tokenize_slot_1_alpha(eparser&p, int32_t c)
{
    if (('0' <= c && c <= '9') || isletterchar(c))
	{
        p.store.push_back(c);
        return;
    }
	else
	{
        ex a1 = emake_str_char16v(p.store);
        p.handle_token_ex(emake_node(ecopy(gs.sym_sSlot.get()), a1));
        tokenize_start(p, c);
        return;
    }
}

// have read ##
void tokenize_slot_2(eparser&p, int32_t c)
{
    if ('0' <= c && c <= '9')
    {
        p.stage = &tokenize_slot_2_digit;
        fmpz_set_ui(p.number.data, c - '0');
        return;
    }
    else
    {
        p.handle_token_ex(emake_node(gs.sym_sSlotSequence.copy(), emake_cint(1)));
        tokenize_start(p, c);
        return;
    }
}

// have read ## followed by a digit
void tokenize_slot_2_digit(eparser&p, int32_t c)
{
    if ('0' <= c && c <= '9')
    {
        fmpz_mul_ui(p.number.data, p.number.data, UWORD(10));
        fmpz_add_ui(p.number.data, p.number.data, c - '0');
        return;
    }
    else
    {
        ex a1 = emake_int_move(p.number);
        p.handle_token_ex(emake_node(ecopy(gs.sym_sSlotSequence.get()), a1));
        tokenize_start(p, c);
        return;
    }
}

// have read :: followed by a digit or letter
void tokenize_message_start(eparser&p, int32_t c)
{
    if (isletterchar(c) || ('0' <= c && c <= '9'))
    {
        p.store.push_back(c);
    }
    else
    {
        p.handle_token_ex(emake_raw(op_inf_MessageName));
        p.handle_token_ex(emake_str_char16v(p.store));
        tokenize_start(p, c);
    }
}




void eparser::handle_char(int32_t c)
{
//std::cout << "******** handle_char " << c <<" *********"<< std::endl;
//std::cout << "<" <<std::endl<< exvector_tostring_full(estack) <<">" <<std::endl;

    if (!esc_name.empty())
    {
        error = erBad;
        return;
    }

    if (error != erNone)
    {
        return;
    }

    (*stage)(*this, c);
}


void eparser::handle_rawchar(int32_t c)
{
    if (esc_name.empty())
    {
        if (c == '\\')
        {
            esc_name.push_back(c);
        }
        else
        {
            handle_char(c);
        }
    }
    else if (esc_name.size() == 1 && c != '[')
    {
        esc_name.clear();
        handle_char('\\');
        handle_char(c);
    }
    else if (c < 128)
    {
        esc_name.push_back(c);
        if (c == ']')
        {
            c = escapedname_to_char(esc_name.c_str());
            if (c == 0)
            {
                error = erBad;
            }
            else
            {
                esc_name.clear();
                handle_char(c);
            }
        }
    }
    else
    {
        error = erBad;
    }
}
