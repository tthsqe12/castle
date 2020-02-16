#include "box_lex.h"
#include "boxes.h"
#include "ex_parse_box.h"
#include "timing.h"


void blexer::add_newline()
{
    switch (state)
    {
    case lexstate_str_start:
    case lexstate_str_after_escape:
    case lexstate_str_after_escape_hex:
    case lexstate_comment_after_first_star:
    case lexstate_comment_after_second_star:
        break;
    default:
        state = lexstate_start;
        break;
    }

    type[idx] = lextype_whitespace;
    penalty[idx] = 1000;
    idx++;
}

void blexer::add_box(int btype)
{
    int t = lextype_expr;
    int p = 1;
    haveex = true;
    if (btype == BNTYPE_SUB || btype == BNTYPE_SUPER || btype == BNTYPE_SUBSUPER)
    {
         p = 500;
    }
    switch (state) {
    case lexstate_str_start:
    case lexstate_str_after_escape:
    case lexstate_str_after_escape_hex:
        state = lexstate_str_start;
        break;
    case lexstate_comment_after_first_star:
    case lexstate_comment_after_second_star:
        state = lexstate_comment_after_first_star;
        break;
    case lexstate_start:
//    case lexstate_oper:
    case lexstate_comment_start:
    case lexstate_num_start:
    case lexstate_num_after_carat:
    case lexstate_num_after_star:
    case lexstate_num_after_tick:
    case lexstate_num_after_mantissa:
    case lexstate_num_after_prec:
    case lexstate_num_after_exp:
    case lexstate_sym_start:
    case lexstate_sym_after_context:
    case lexstate_blank_1:
    case lexstate_blank_1_dot:
    case lexstate_blank_2:
    case lexstate_blank_3:
    case lexstate_blank_sym_start:
    case lexstate_blank_sym_after_context:
    case lexstate_slot_start:
    case lexstate_slot_1_digit:
    case lexstate_slot_1_alpha:
    case lexstate_slot_2:
    default:
        state = lexstate_start;
        break;
/*
        printf("add expr unknown state %d\n",state);
        assert(false);
*/
    }
    type[idx] = t;
    penalty[idx] = p;
    idx++;
}

int blexer::add_char(char16_t c)
{
    uint32_t j;
    int t, p;
    switch (state)
    {

    _lexstate_start:
        state = lexstate_start;
    case lexstate_start:
    {
        if (isletterchar(c)) {
            haveex = true;
            p = 1;
            t = lextype_symbol_1st;
            state = lexstate_sym_start;
        } else if (c == '0') {
            haveex = true;
            p = 1;
            t = lextype_number_1st;
            state = lexstate_num_start_zero;
        } else if ('1' <= c && c <= '9') {
            haveex = true;
            p = 1;
            t = lextype_number_1st;
            state = lexstate_num_start;
        } else if (c == '_') {
            haveex = true;
            p = 1;
            t = lextype_blank_1st;
            state = lexstate_blank_1;
        } else if (c == '"') {
            haveex = true;
            p = 1;
            t = lextype_string_1st;
            state = lexstate_str_start;
        } else if (c == '(') {
            haveex = false;
            p = 20;
            t = lextype_parenth_open;
            state = lexstate_comment_start;
        } else if (c == ')') {
            haveex = true;
            p = 100;
            t = lextype_parenth_close;
            state = lexstate_start;
        } else if (c == '[') {
            haveex = false;
            p = 200;
            t = lextype_bracket_open;
            state = lexstate_start;
        } else if (c == ']') {
            haveex = true;
            p = 200;
            t = lextype_bracket_close;
            state = lexstate_start;
        } else if (c == '{') {
            haveex = false;
            p = 100;
            t = lextype_curly_open;
            state = lexstate_start;
        } else if (c == '}') {
            haveex = true;
            p = 200;
            t = lextype_curly_close;
            state = lexstate_start;
        } else if (c == CHAR_LeftDoubleBracket) {
            haveex = false;
            p = 200;
            t = lextype_bracket_open;
            state = lexstate_start;
        } else if (c == CHAR_RightDoubleBracket) {
            haveex = true;
            p = 200;
            t = lextype_bracket_close;
            state = lexstate_start;
        } else if (c == ',') {
            haveex = false;
            p = 300;
            t = lextype_comma;
            state = lexstate_start;
        } else if (ispureopchar(c)) {
            p = 20;
            t = (c == '\'') ? lextype_oppost_1st + 256*op_post_Derivative :
                (c == '&')  ? lextype_oppost_1st + 256*op_post_Function :
                (c == '|')  ? lextype_opinf_1st + 256*op_inf_Alternatives :
                (c == '.')  ? lextype_opinf_1st + 256*op_inf_Dot :
                (c == '^')  ? lextype_opinf_1st + 256*op_inf_Power :
                (c == '/')  ? lextype_opinf_1st + 256*op_inf_Divide :
                (c == '*')  ? lextype_opinf_1st + 256*op_inf_Times :
                (c == '=')  ? lextype_opinf_1st + 256*op_inf_Set :
                (c == '>')  ? lextype_opinf_1st + 256*op_inf_Greater :
                (c == '<')  ? lextype_opinf_1st + 256*op_inf_Less :
                (c == ';')  ? lextype_opinf_1st + 256*op_inf_CompoundExpression :
                (c == '@')  ? lextype_opinf_1st + 256*op_inf_GenPre :
                (c == '+')  ? (haveex ? lextype_opinf_1st + 256*op_inf_Plus
                                      : lextype_oppre_1st + 256*op_pre_Plus) :
                (c == '-')  ? (haveex ? lextype_opinf_1st + 256*op_inf_Minus
                                      : lextype_oppre_1st + 256*op_pre_Minus) :
                (c == '!')  ? (haveex ? lextype_oppost_1st + 256*op_post_Factorial
                                      : lextype_oppre_1st + 256*op_pre_Not) :
                (c == CHAR_Integral) ? lextype_oppre_1st + 256*op_pre_Integrate :
                (c == CHAR_DifferentialD) ? lextype_oppre_1st + 256*op_pre_DifferentialD:
                              lextype_opinf_1st + 256*op_none;

//if (c == CHAR_Integral) {printf("got CHAR_Integral\n");}
//if (c == CHAR_DifferentialD) {printf("got CHAR_DifferentialD\n");}

            state = c == '+' ? lexstate_oper_plus :
                    c == '-' ? lexstate_oper_minus :
                    c == '*' ? lexstate_oper_times :
                    c == '/' ? lexstate_oper_divide :
                    c == '^' ? lexstate_oper_equal :
                    c == ';' ? lexstate_oper_semi :
                    c == '=' ? lexstate_oper_equal :
                    c == '<' ? lexstate_oper_less :
                    c == '>' ? lexstate_oper_greater :
                    c == '&' ? lexstate_oper_and :
                    c == '!' ? lexstate_oper_bang :
                    c == ':' ? lexstate_oper_colon :
                    c == '\'' ? lexstate_oper_prime :
                    c == '.' ? lexstate_oper_dot :
                    c == '|' ? lexstate_oper_vbar :
                    c == '~' ? lexstate_oper_tilde :
                    c == '@' ? lexstate_oper_at :
                               lexstate_start;
            if (state == lexstate_start) {
                haveex = false;
            }
        } else if (c == '#') {
            haveex = true;
            p = 2;
            t = lextype_slot_1st;
            state = lexstate_slot_start;
        } else if (c==0 || c==' ' || c=='\n' || c=='\t' || c=='\r') {
            p = 2;
            t = lextype_whitespace;
            state = lexstate_start;
        } else {
            p = 2;
            t = lextype_unknown;
            state = lexstate_start;
        }
        break;
    }

    case lexstate_oper_plus:
        p = 500;
        t = lextype_opinf;
        if (c == '=')
        {
            state = lexstate_start;
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_AddTo;
            haveex = false;
        }
        else if (c == '+')
        {
            t = haveex ? lextype_oppost : lextype_oppre;
            type[idx - 1] = haveex ? lextype_oppost_1st + 256*op_post_Increment
                                   : lextype_oppre_1st + 256*op_pre_PreIncrement;
            state = lexstate_start;
        }
        else if (c == '/')
        {
            state = lexstate_oper_plus_divide;
        }
        else
        {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_plus_divide:
        p = 500;
        t = lextype_opinf;
        if (c == '-')
        {
            t = lextype_number;
            type[idx - 2] = t;
            type[idx - 1] = t;
            state = lexstate_num_after_mantissa;
        }
        else
        {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_minus:
        p = 500;
        t = lextype_opinf;
        if (c == '=') {
            state = lexstate_start;
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_SubtractFrom;
            haveex = false;
        } else if (c == '>') {
            state = lexstate_start;
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_Rule;
            haveex = false;
        } else if (c == '-') {
            t = haveex ? lextype_oppost : lextype_oppre;
            type[idx - 1] = haveex ? lextype_oppost_1st + 256*op_post_Decrement
                                   : lextype_oppre_1st + 256*op_pre_PreDecrement;
            state = lexstate_start;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_colon:
        p = 500;
        t = lextype_opinf;
        haveex = false;
        if (c == '=') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_SetDelayed;
            state = lexstate_start;
        } else if (c == '>') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_RuleDelayed;
            state = lexstate_start;
        } else if (c == ':') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_MessageName;
            state = lexstate_message_name_start;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_message_name_start:
        p = 800;
        t = lextype_message_name;
        if (isletterchar(c)) {
            type[idx - 2] = lextype_message_name;
            type[idx - 1] = lextype_message_name;
            state = lexstate_message_name;
            haveex = true;
        } else {
            goto _lexstate_start;
        }

    case lexstate_message_name:
        p = 800;
        t = lextype_message_name;
        if (!(isletterchar(c) || ('0' < c && c <= '9'))) {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_vbar:
        p = 500;
        t = lextype_opinf;
        haveex = false;
        if (c == '|') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_Or;
            state = lexstate_start;
        } else if (c == '>') {
            haveex = false;
            p = 2;
            t = lextype_curly_close;
            type[idx - 1] = lextype_unknown;
            penalty[idx - 1] = 2000;
            state = lexstate_start;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_tilde:
        p = 500;
        t = lextype_opinf;
        haveex = false;
        if (c == '~') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_StringExpression;
            state = lexstate_start;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_dot:
        p = 500;
        t = lextype_oppost;
        haveex = false;
        if (c == '.') {
            type[idx - 1] = lextype_oppost_1st + 256*op_post_Repeated;
            state = lexstate_oper_dot_dot;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_dot_dot:
        p = 500;
        t = lextype_oppost;
        if (c == '.') {
            type[idx - 2] = lextype_oppost_1st + 256*op_post_RepeatedNull;
            state = lexstate_start;
        } else {
            goto _lexstate_start;
        }
        break;

    _lexstate_oper_times:
    case lexstate_oper_times:
        p = 500;
        t = lextype_opinf;
        haveex = false;
        if (c == '*') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_NonCommutativeMultiply;
            state = lexstate_start;
        } else if (c == '=') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_TimesBy;
            state = lexstate_start;
        } else {
            goto _lexstate_start;
        }
        break;

    _lexstate_oper_caret:
    case lexstate_oper_caret:
        p = 500;
        t = lextype_opinf;
        haveex = false;
        if ( c == '=') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_UpSet;
            state = lexstate_start;
        } else if ( c == ':') {
            state = lexstate_oper_caret_colon;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_caret_colon:
        p = 500;
        t = lextype_opinf;
        haveex = false;
        if ( c == '=') {
            type[idx - 2] = lextype_opinf_1st + 256*op_inf_UpSetDelayed;
            state = lexstate_start;
        } else {
            type[idx - 2] = lextype_opinf_1st + 256*op_none;
            type[idx - 1] = lextype_opinf_1st + 256*op_none;
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_divide:
        p = 500;
        t = lextype_opinf;
        haveex = false;
        if (c == ':') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_tag;
            state = lexstate_start;
        } else if (c == ';') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_Condition;
            state = lexstate_start;
        } else if (c == '.') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_ReplaceAll;
            state = lexstate_start;
        } else if (c == '@') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_Map;
            state = lexstate_start;
        } else if (c == '=') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_DivideBy;
            state = lexstate_start;
        } else if ( c == '/') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_GenPost;
            state = lexstate_oper_divide_divide;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_divide_divide:
        p = 500;
        t = lextype_opinf;
        if (c == '.') {
            type[idx - 2] = lextype_opinf_1st + 256*op_inf_ReplaceRepeated;
            state = lexstate_start;
        } else if (c == '@') {
            type[idx - 2] = lextype_opinf_1st + 256*op_inf_MapAll;
            state = lexstate_start;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_equal:
        p = 500;
        t = lextype_opinf;
        haveex = false;
        if (c == '!') {
            state = lexstate_oper_equal_not;
        } else if (c == '=') {
            state = lexstate_oper_equal_equal;
        } else if (c == '.') {
            type[idx - 1] = lextype_oppost_1st + 256*op_post_Unset;
            t = lextype_oppost;
            state = lexstate_start;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_equal_equal:
        p = 500;
        t = lextype_opinf;
        if (c == '=') {
            type[idx - 1] = lextype_oppost_1st + 256*op_inf_SameQ;
            state = lexstate_start;
        } else {
            type[idx - 2] = lextype_opinf_1st + 256*op_inf_Equal;
            type[idx - 1] = lextype_opinf;
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_equal_not:
        p = 500;
        t = lextype_opinf;
        if (c == '=') {
            type[idx - 1] = lextype_oppost_1st + 256*op_inf_UnsameQ;
            state = lexstate_start;
        } else {
            type[idx - 2] = lextype_opinf_1st + 256*op_none;
            type[idx - 1] = lextype_opinf_1st + 256*op_none;
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_and:
        p = 500;
        t = lextype_opinf;
        haveex = false;
        if (c == '&') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_And;
            state = lexstate_start;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_bang:
        p = 500;
        t = lextype_opinf;
        if (c == '=') {
            t = lextype_opinf;
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_Unequal;
            state = lexstate_start;
            haveex = false;
        } else if (c == '!') {
            t = lextype_oppost;
            type[idx - 1] = lextype_oppost_1st + 256*op_post_Factorial2;
            state = lexstate_start;
            haveex = true;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_greater:
        p = 500;
        t = lextype_opinf;
        haveex = false;
        if (c == '=') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_GreaterEqual;
            state = lexstate_start;
        } else if (c == '>') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_Put;
            state = lexstate_start;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_less:
        p = 500;
        t = lextype_opinf;
        haveex = false;
        if (c == '=') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_LessEqual;
            state = lexstate_start;
        } else if (c == '>') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_StringJoin;
            state = lexstate_start;
        } else if (c == '|') {
            haveex = false;
            p = 2;
            t = lextype_unknown;
            type[idx - 1] = lextype_curly_open;
            penalty[idx - 1] = 2000;
            state = lexstate_start;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_semi:
        p = 500;
        t = lextype_opinf;
        haveex = false;
        if (c == ';') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_Span;
            state = lexstate_start;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_at:
        p = 500;
        t = lextype_opinf;
        haveex = false;
        if (c == '@') {
            type[idx - 1] = lextype_opinf_1st + 256*op_inf_Apply;            
            state = lexstate_start;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_oper_prime:
        p = 500;
        t = lextype_oppost;
        haveex = true;
        if (c == '\'') {
            state = lexstate_oper_prime;
        } else {
            goto _lexstate_start;
        }
        break;


    case lexstate_num_start_zero:
        if (c == 'x') {
            p = 1000;
            t = lextype_number;
            state = lexstate_num_after_mantissa;    
            break;        
        }

    case lexstate_num_start:
        if ('0' <= c && c <= '9') {
            p = 1000;
            t = lextype_number;
        } else if (c == '.') {
            p = 1000;
            t = lextype_number;
            state = lexstate_num_after_mantissa;
        } else if (c == '^') {
            p = 1000;
            t = lextype_number;
            state = lexstate_num_after_carat;
        } else if (c == '*') {
            p = 1000;
            t = lextype_number;
            state = lexstate_num_after_star;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_num_after_carat:
        if (c == '^') {
            p = 1000;
            t = lextype_number;
            state = lexstate_num_after_mantissa;
        } else {
            assert(idx > 0);
            type[idx-1] = lextype_opinf_1st;
            penalty[idx-1] = 20;
            goto _lexstate_oper_caret;
        }
        break;

    case lexstate_num_after_star:
        if (c=='^') {
            p = 1000;
            t = lextype_number;
            state = lexstate_num_after_exp;
        } else {
            assert(idx > 0);
            type[idx-1] = lextype_opinf_1st;
            penalty[idx-1] = 20;
            goto _lexstate_oper_times;
        }
        break;

    case lexstate_num_after_mantissa:
        if (c == '.') {
            p = 1000;
            t = lextype_number;
        } else if (c == '`') {
            p = 1000;
            t = lextype_number;
            state = lexstate_num_after_tick;
        } else if (c == '*') {
            p = 1000;
            t = lextype_number;
            state = lexstate_num_after_star;
        } else if (('0' <= c && c <= '9') || ('a' <= c && c <= 'z')
                || ('A' <= c && c <= 'Z') || c == '_') {
            p = 1000;
            t = lextype_number;
        } else {
            state = lexstate_start;
            goto _lexstate_start;
        }
        break;
 
    case lexstate_num_after_tick:
        if (c=='`') {
            p = 1000;
            t = lextype_number;
        } else if (c=='*') {
            p = 1000;
            t = lextype_number;
            state = lexstate_num_after_star;
        } else if (('0' <= c && c <= '9')) {
            p = 1000;
            t = lextype_number;
            state = lexstate_num_after_prec;
        } else {
            state = lexstate_start;
            goto _lexstate_start;
        }
        break;

    case lexstate_num_after_prec:
        if (c=='*') {
            p = 1000;
            t = lextype_number;
            state = lexstate_num_after_star;

        } else if ('0' <= c && c <= '9') {
            p = 1000;
            t = lextype_number;

        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_num_after_exp:
        if (('0' <= c && c <= '9') || (c == '-')) {
            p = 1000;
            t = lextype_number;
        } else {
            goto _lexstate_start;
        }
        break;


    case lexstate_str_start:
        if (c=='\\') {
            p = 300;
            t = lextype_string;
            state = lexstate_str_after_escape;
        } else if (c == '"') {
            p = 300;
            t = lextype_string;
            state = lexstate_start;
        } else {
            p = 300;
            t = lextype_string;
        }
        break;

    case lexstate_str_after_escape:
        p = 300;
        t = lextype_string;
        state = lexstate_str_start;
        break;


    case lexstate_sym_start:
        if (isletterchar(c) || ('0' <= c && c <= '9')) {
            p = 1000;
            t = lextype_symbol;
        } else if (c=='`') {
            p = 1000;
            t = lextype_symbol;
            state = lexstate_sym_after_context;
        } else if (c=='_') {
            assert(idx > 0);

            // change previous symbol to pattern
            j = idx - 1;
            while (1) {
                if (type[j] == lextype_symbol_1st) {
                    type[j] = lextype_pattern_1st;
                    break;
                }
                assert(type[j] == lextype_symbol);
                //type[j] = lextype_blank;
                j--;
            }

            p = 1000;
            t = lextype_blank;
            state = lexstate_blank_1;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_sym_after_context:
        if (isletterchar(c)) {
            p = 1000;
            t = lextype_symbol;
            state = lexstate_sym_start;
        } else {
            goto _lexstate_start;
        }
        break;


    case lexstate_blank_1:
        if (c == '.') {
            p = 1000;
            t = lextype_blank;
            state = lexstate_blank_1_dot;
        } else if (c=='_') {
            p = 1000;
            t = lextype_blank;
            state = lexstate_blank_2;
        } else if (isletterchar(c)) {
            p = 1000;
            t = lextype_blank;
            state = lexstate_blank_sym_start;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_blank_1_dot:
        if (c == '.') {
            t = lextype_oppost;
            haveex = false;
            type[idx - 1] = lextype_oppost_1st + 256*op_post_Repeated;
            state = lexstate_oper_dot_dot;
            p = 500;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_blank_2:
        if (c=='_') {
            p = 1000;
            t = lextype_blank;
            state = lexstate_blank_3;
        } else if (isletterchar(c)) {
            p = 1000;
            t = lextype_blank;
            state = lexstate_blank_sym_start;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_blank_3:
        if (isletterchar(c)) {
            p = 1000;
            t = lextype_blank;
            state = lexstate_blank_sym_start;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_blank_sym_start:
        if (isletterchar(c) || ('0' <= c && c <= '9')) {
            p = 1000;
            t = lextype_blank;
        } else if (c=='`') {
            p = 1000;
            t = lextype_blank;
            state = lexstate_blank_sym_after_context;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_blank_sym_after_context:
        if (isletterchar(c)) {
            p = 1000;
            t = lextype_blank;
            state = lexstate_blank_sym_start;
        } else {
            goto _lexstate_start;
        }
        break;


    case lexstate_slot_start:
        if ('0' <= c && c <= '9') {
            p = 1000;
            t = lextype_slot;
            state = lexstate_slot_1_digit;
        } else if (isletterchar(c)) {
            p = 1000;
            t = lextype_slot;
            state = lexstate_slot_1_alpha;
        } else if (c == '#') {
            p = 1000;
            t = lextype_slot;
            state = lexstate_slot_2;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_slot_1_digit:
        if ('0' <= c && c <= '9') {
            p = 1000;
            t = lextype_slot;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_slot_1_alpha:
        if ('0' <= c && c <= '9' || isletterchar(c)) {
            p = 1000;
            t = lextype_slot;
        } else {
            goto _lexstate_start;
        }
        break;

    case lexstate_slot_2:
        if ('0' <= c && c <= '9') {
            p = 1000;
            t = lextype_slot;
            state = lexstate_slot_1_digit; // overload # and ##
        } else {
            goto _lexstate_start;
        }
        break;


    case lexstate_comment_start:
        if (c == '*') {
            p = 1000;
            t = lextype_comment;
            state = lexstate_comment_after_first_star;
            penalty[idx-1] = 1;
            type[idx-1] = lextype_comment_1st;
        } else {
            goto _lexstate_start;
        }
        break;
    case lexstate_comment_after_first_star:
        if (c == '*') {
            p = 1000;
            t = lextype_comment;
            state = lexstate_comment_after_second_star;
        } else {
            p = 1000;
            t = lextype_comment;
        }
        break;
    case lexstate_comment_after_second_star:
        if (c == ')') {
            p = 1000;
            t = lextype_comment;
            state = lexstate_start;
        } else {
            p = 1000;
            t = lextype_comment;
            state = lexstate_comment_after_first_star;
        }
        break;

    default:
        printf("add char unknown state %d\n",state);
        assert(false);
    }

    type[idx] = t;
    penalty[idx] = p;
    idx++;
    return t;
}


void errorbox::set_scolor(bsymer* T)
{
    return;
}

void nullbox::set_scolor(bsymer* T)
{
    assert(false);
}

void charbox::set_scolor(bsymer* T)
{
    assert(false);
}

void rootbox::set_scolor(bsymer* T)
{
    assert(false);
}

void cellbox::set_scolor(bsymer* T)
{
    assert(false);
}

void rowbox::set_scolor(bsymer* T)
{
    assert(false);
}

void graphics3dbox::set_scolor(bsymer* T)
{
    return;
}

void monobox::set_scolor(bsymer* T)
{
    size_t len_module  = T->vars_module.size();
    size_t len_block   = T->vars_block.size();
    size_t len_table   = T->vars_table.size();
    size_t len_pattern = T->vars_pattern.size();

    bsymer B;
    std::swap(B.vars_module, T->vars_module);
    std::swap(B.vars_block, T->vars_block);
    std::swap(B.vars_table, T->vars_table);
    std::swap(B.vars_pattern, T->vars_pattern);

    std::vector<iboxarrayelem> tchild;
    for (auto y = array.begin(); y != array.end(); ++y)
    {
        for (auto x = y->begin(); x != y->end(); ++x)
        {
            tchild.push_back(iboxarrayelem(iboximm_make(*x)));
        }
        tchild.push_back(iboxarrayelem(new nullbox()));
    }

    B.add_scolor(tchild.data(), 0, tchild.size() - 1);

    size_t toff = 0;
    for (auto y = array.begin(); y != array.end(); ++y)
    {
        for (auto x = y->begin(); x != y->end(); ++x)
        {
            *x = iboximm_type(tchild[toff].cibox);
            toff++;
        }
        delete ibox_to_ptr(tchild[toff].cibox);
        toff++;
    }



/*
    for (size_t k = 0; k < B.tokens.size(); k++)
    {
        if (B.tokens[k].lextype == lextype_pattern_1st)
        {
            box_patterns_vars.back().push_back(B.array + B.tokens[k].idx);
        }
    }
    for (size_t k1 = 0; k1 < B.box_patterns_vars.size(); k1++)
    {
//std::cout << "pushing onto box_patterns_vars.back()" << std::endl;
        for (size_t k2 = 0; k2 < B.box_patterns_vars[k1].size(); k2++)
        {
            box_patterns_vars.back().push_back(B.box_patterns_vars[k1][k2]);
        }
    }
*/
    assert(len_module  == B.vars_module.size());
    assert(len_block   == B.vars_block.size());
    assert(len_table   == B.vars_table.size());
    assert(len_pattern == B.vars_pattern.size());
    std::swap(B.vars_module, T->vars_module);
    std::swap(B.vars_block, T->vars_block);
    std::swap(B.vars_table, T->vars_table);
    std::swap(B.vars_pattern, T->vars_pattern);
    
    return;
}

void bsymer::color_rowbox(rowbox* r)
{
//std::cout << "color_rowbox called " << std::endl;

    size_t len_module  = vars_module.size();
    size_t len_block   = vars_block.size();
    size_t len_table   = vars_table.size();
    size_t len_pattern = vars_pattern.size();

    bsymer B;
    std::swap(B.vars_module, vars_module);
    std::swap(B.vars_block, vars_block);
    std::swap(B.vars_table, vars_table);
    std::swap(B.vars_pattern, vars_pattern);

    r->flags |= BNFLAG_COLORED;
    B.add_scolor(r->child.data(), 0, r->child.size() - 1);

    for (size_t k = 0; k < B.tokens.size(); k++)
    {
        if (B.tokens[k].lextype == lextype_pattern_1st)
        {
            box_patterns_vars.back().push_back(B.array + B.tokens[k].idx);
        }
    }
    for (size_t k1 = 0; k1 < B.box_patterns_vars.size(); k1++)
    {
//std::cout << "pushing onto box_patterns_vars.back()" << std::endl;
        for (size_t k2 = 0; k2 < B.box_patterns_vars[k1].size(); k2++)
        {
            box_patterns_vars.back().push_back(B.box_patterns_vars[k1][k2]);
        }
    }

    assert(len_module  == B.vars_module.size());
    assert(len_block   == B.vars_block.size());
    assert(len_table   == B.vars_table.size());
    assert(len_pattern == B.vars_pattern.size());
    std::swap(B.vars_module, vars_module);
    std::swap(B.vars_block, vars_block);
    std::swap(B.vars_table, vars_table);
    std::swap(B.vars_pattern, vars_pattern);
}



void fractionbox::set_scolor(bsymer* T)
{
    T->color_rowbox(num.cbox);
    T->color_rowbox(den.cbox);
}

void sqrtbox::set_scolor(bsymer* T)
{
    T->color_rowbox(inside.cbox);
}

void rotationbox::set_scolor(bsymer* T)
{
    T->color_rowbox(inside.cbox);
}

void underscriptbox::set_scolor(bsymer* T)
{
    T->color_rowbox(under.cbox);
    T->color_rowbox(body.cbox);
}


void overscriptbox::set_scolor(bsymer* T)
{
    T->color_rowbox(over.cbox);
    T->color_rowbox(body.cbox);
}


void underoverscriptbox::set_scolor(bsymer* T)
{
    T->color_rowbox(under.cbox);
    T->color_rowbox(over.cbox);
    T->color_rowbox(body.cbox);
}

void gridbox::set_scolor(bsymer* T)
{
    for (auto i = array.begin(); i != array.end(); ++i)
        for (auto j = i->begin(); j != i->end(); ++j)
            T->color_rowbox(j->cbox);
}

void subscriptbox::set_scolor(bsymer* T)
{
    T->color_rowbox(sub.cbox);
}

void superscriptbox::set_scolor(bsymer* T)
{
    T->color_rowbox(super.cbox);
}

void subsuperscriptbox::set_scolor(bsymer* T)
{
    T->color_rowbox(sub.cbox);
    T->color_rowbox(super.cbox);
}



void bsymer::add_scolor(iboxarrayelem * Array, int32_t starti, int32_t stopi)
{
    std::vector<int32_t> istack;
    std::vector<int32_t> ostack;

    array = Array;

//printf("add scolor here 1\n");

    // collect info on tokens and matching brackets
    for (int32_t i = starti; i < stopi; i++)
    {
        int32_t type = ibox_type(array[i].cibox);
//printf("type[%d] = %d\n", i, type);
        if (type >= 0)
        {
            if ((type & 0x0FFFFFF) == '(' + 65536*lextype_parenth_open)
            {

                istack.push_back(tokens.size());
                ostack.push_back('(');
                tokens.push_back(bsymentry(i,lextype_parenth_open));
            }
            else if ((type & 0x0FFFFFF) == ')' + 65536*lextype_parenth_close)
            {
                if (!ostack.empty() && ostack.back() == '(')
                {
                    tokens[istack.back()].extra = tokens.size();
                    tokens.push_back(bsymentry(i,lextype_parenth_close, istack.back()));
                    istack.pop_back();
                    ostack.pop_back();
                }
                else
                {
                    tokens.push_back(bsymentry(i,lextype_parenth_close));
                }
            }
            else if ((type & 0x0FFFFFF) == '[' + 65536*lextype_bracket_open)
            {
                istack.push_back(tokens.size());
                ostack.push_back('[');
                tokens.push_back(bsymentry(i,lextype_bracket_open));
            }
            else if ((type & 0x0FFFFFF) == ']' + 65536*lextype_bracket_close)
            {
                if (!ostack.empty() && ostack.back() == '[')
                {
                    tokens[istack.back()].extra = tokens.size();
                    tokens.push_back(bsymentry(i,lextype_bracket_close, istack.back()));
                    istack.pop_back();
                    ostack.pop_back();
                }
                else
                {
                    tokens.push_back(bsymentry(i,lextype_bracket_close));
                }
            }
            else if ((type & 0x0FFFFFF) == '{' + 65536*lextype_curly_open)
            {
                istack.push_back(tokens.size());
                ostack.push_back('{');
                tokens.push_back(bsymentry(i,lextype_curly_open));
            }
            else if ((type & 0x0FFFFFF) == '}' + 65536*lextype_curly_close)
            {
                if (!ostack.empty() && ostack.back() == '{')
                {
                    tokens[istack.back()].extra = tokens.size();
                    tokens.push_back(bsymentry(i,lextype_curly_close, istack.back()));
                    istack.pop_back();
                    ostack.pop_back();
                }
                else
                {
                    tokens.push_back(bsymentry(i,lextype_curly_close));
                }
            }
            else if ((type & 0x0FF0000) ==       65536*lextype_symbol_1st)
            {
                tokens.push_back(bsymentry(i,lextype_symbol_1st));
            }
            else if ((type & 0x0FF0000) ==       65536*lextype_comma)
            {
                tokens.push_back(bsymentry(i,lextype_comma));
            }
            else if ((type & 0x0FF0000) ==       65536*lextype_pattern_1st)
            {
                tokens.push_back(bsymentry(i,lextype_pattern_1st));
            }
            else if ((type & 0x0FF0000) ==       65536*lextype_opinf_1st)
            {
                if (static_cast<Operator>(type >> 24) != op_none)
                {
                    tokens.push_back(bsymentry(i,lextype_opinf_1st, type >> 24));
                }
            }
            else if ((type & 0x0FF0000) ==       65536*lextype_oppost_1st)
            {
                if (static_cast<Operator>(type >> 24) != op_none)
                {
                    tokens.push_back(bsymentry(i,lextype_oppost_1st, type >> 24));
                }
            }
            else if ((type & 0x0FF0000) ==       65536*lextype_oppre_1st)
            {
                if (static_cast<Operator>(type >> 24) != op_none)
                {
                    tokens.push_back(bsymentry(i,lextype_oppre_1st, type >> 24));
                }
            }
        }
        else
        {

//printf("add scolor here 2\n");


            if (type < BNTYPE_NULLER/* && type != BNTYPE_STUB*/)
            {

//printf("add scolor here 2A\n");


                tokens.push_back(bsymentry(i,lextype_expr));
            }
        }
    }

//printf("add scolor here 3\n");


    tokens.push_back(bsymentry(-1,lextype_MAX));

//std::cout << "before coloring" << std::endl; print();

    color_range(0, tokens.size() - 1);

//std::cout << "after coloring" << std::endl; print();

//std::cout << "tokens size: " << tokens.size() << std::endl;


    // put the colors into the original row box
    std::string s;
    for (size_t j = 0; j < tokens.size(); j++)
    {
//std::cout << "token[" << j << "].lextype = " << tokens[j].lextype << std::endl;

        if (   tokens[j].lextype == lextype_symbol_1st
            || tokens[j].lextype == lextype_pattern_1st)
        {
//std::cout << "token " << j << " is a symbol or pattern";
            int32_t i = tokens[j].idx;
            int32_t color = tokens[j].color;

//std::cout << " i " << i << std::endl;
//std::cout << " color " << color << std::endl;

            assert(iboxchar_lextype(array[i].cibox) == tokens[j].lextype);

            if (color == semtype_unknown)
            {
//std::cout << " it is unknown " << std::endl;
                int32_t k = i;
                s.clear();
                do {
                    stdstring_pushback_char16(s, iboxchar_type(array[k].cibox)&65535);
                } while (++k < stopi && ibox_is_char(array[k].cibox)
                                     && iboxchar_lextype(array[k].cibox) == lextype_symbol);
//std::cout << " s: " << s << std::endl;
                if (gs.char_set.count(s))
                {
                    color = semtype_defined;
                }
            }

            do {
                array[i].cibox = iboxchar_addsymtype(array[i].cibox, color);
            } while (++i < stopi && ibox_is_char(array[i].cibox)
                                 && iboxchar_lextype(array[i].cibox) == lextype_symbol);
        }
    }

//print();

//printf("add scolor here 4\n");
}




bool _symbol_matches(iboxarrayelem * a, const char * s, size_t n)
{
    assert(ibox_is_char(a[0].cibox));
    assert(iboxchar_lextype(a[0].cibox) == lextype_symbol_1st);
    assert(n > 0);
    if ((iboxchar_type(a[0].cibox)&65535) != s[0])
    {
        return false;
    }
    for (size_t i = 1; i < n; i++)
    {
        if (!ibox_is_char(a[i].cibox))
        {
            return false;
        }
        if ((iboxchar_type(a[i].cibox)&65535) != s[i])
        {
            return false;
        }
    }
    return true;
}

bool _symbol_matches(iboxarrayelem * a, iboxarrayelem * b)
{
//std::cout << "_symbol_matches called" << std::endl;
    assert(ibox_is_char(a[0].cibox));
    assert(iboxchar_lextype(a[0].cibox) == lextype_symbol_1st);
    assert(   iboxchar_lextype(b[0].cibox) == lextype_symbol_1st
           || iboxchar_lextype(b[0].cibox) == lextype_pattern_1st);
    if ((iboxchar_type(a[0].cibox)&65535) != (iboxchar_type(b[0].cibox)&65535))
    {
        return false;
    }
    for (size_t i = 1; true; i++)
    {
        if (!(ibox_is_char(a[i].cibox) && iboxchar_lextype(a[i].cibox) == lextype_symbol))
        {
            return !(ibox_is_char(b[i].cibox) && iboxchar_lextype(b[i].cibox) == lextype_symbol);
        }
        if ((iboxchar_type(a[i].cibox)&65535) != (iboxchar_type(b[i].cibox)&65535))
        {
            return false;
        }
    }
    return true;
}


bool _symbol_matches_module_type(iboxarrayelem * a)
{
    return _symbol_matches(a, "With", 4)
        || _symbol_matches(a, "Module", 6)
    ;
}

bool _symbol_matches_block_type(iboxarrayelem * a) {
    return _symbol_matches(a, "Block", 5)
    ;
}

bool _symbol_matches_table_type(iboxarrayelem * a) {
    return _symbol_matches(a, "Do", 2)
        || _symbol_matches(a, "Sum", 3)
        || _symbol_matches(a, "Plot", 4)
        || _symbol_matches(a, "Table", 5)
        || _symbol_matches(a, "Plot3D", 6)
        || _symbol_matches(a, "Product", 7)
        || _symbol_matches(a, "ParametricPlot", 14)
        || _symbol_matches(a, "ParametricPlot3D", 16)
    ;
}

void bsymer::color_range(size_t startj, size_t endj)
{
//std::cout << "color_range("<<startj<<","<<endj<<") called" << std::endl;

    size_t j = startj;
    int32_t save = tokens[endj].lextype;
    tokens[endj].lextype = lextype_MAX;

    while (j < endj)
    {
        if (tokens[j].lextype == lextype_symbol_1st)
        {
            color_symbol(j);
            if (_symbol_matches_module_type(array + tokens[j].idx))
            {
                j = color_module(j + 1);
            }
            else if (_symbol_matches_block_type(array + tokens[j].idx))
            {
                j = color_block(j + 1);
            }
            else if (_symbol_matches_table_type(array + tokens[j].idx))
            {
                j = color_table(j + 1);
            } else {
                j++;
            }
        }
        else if (tokens[j].lextype == lextype_opinf_1st)
        {
            if (tokens[j].extra == op_inf_Set)
            {
                j = color_rule(j + 1, prec_inf_Set);
            }
            else if (tokens[j].extra == op_inf_SetDelayed)
            {
                j = color_rule(j + 1, prec_inf_SetDelayed);
            }
            else if (tokens[j].extra == op_inf_Rule)
            {
                j = color_rule(j + 1, prec_inf_Rule);
            }
            else if (tokens[j].extra == op_inf_RuleDelayed)
            {
                j = color_rule(j + 1, prec_inf_RuleDelayed);
            }
            else
            {
                j++;
            }
        }
        else if (tokens[j].lextype == lextype_oppre_1st)
        {
            if (tokens[j].extra == op_pre_Integrate)
            {
                j = color_integral(j + 1);
            }
            else
            {
                j++;
            }
        }
        else if (tokens[j].lextype == lextype_expr)
        {
            j = color_subbox(j);
        }
        else
        {
            j++;
        }
    }
    tokens[endj].lextype = save;

//std::cout << "color_range returning" << std::endl;
}

size_t bsymer::color_expr(size_t startj) {
//std::cout << "color_expr("<<startj<<") called" << std::endl;

    size_t j = startj;
    while (tokens[j].lextype != lextype_MAX)
    {
        if (tokens[j].lextype == lextype_symbol_1st)
        {
            color_symbol(j);
            if (_symbol_matches_module_type(array + tokens[j].idx)) {
                j = color_module(j + 1);
            } else if (_symbol_matches_block_type(array + tokens[j].idx)) {
                j = color_block(j + 1);
            } else if (_symbol_matches_table_type(array + tokens[j].idx)) {
                j = color_table(j + 1);
            } else {
                j++;
            }
        } else if (    tokens[j].lextype == lextype_comma
                    || tokens[j].lextype == lextype_parenth_close
                    || tokens[j].lextype == lextype_bracket_close
                    || tokens[j].lextype == lextype_curly_close) {
//std::cout << "color_expr returning " << j << std::endl;
            return j;

        } else if (   (tokens[j].lextype == lextype_parenth_open
                    || tokens[j].lextype == lextype_bracket_open
                    || tokens[j].lextype == lextype_curly_open) && tokens[j].extra >= 0) {
            color_range(j + 1, tokens[j].extra + 1);
            j = tokens[j].extra + 1;

        } else if (tokens[j].lextype == lextype_opinf_1st) {
            if (tokens[j].extra == op_inf_Set) {
                j = color_rule(j + 1, prec_inf_Set);
            } else if (tokens[j].extra == op_inf_SetDelayed) {
                j = color_rule(j + 1, prec_inf_SetDelayed);
            } else if (tokens[j].extra == op_inf_Rule) {
                j = color_rule(j + 1, prec_inf_Rule);
            } else if (tokens[j].extra == op_inf_RuleDelayed) {
                j = color_rule(j + 1, prec_inf_RuleDelayed);
            } else {
                j++;
            }

        }
        else if (tokens[j].lextype == lextype_oppre_1st)
        {
            if (tokens[j].extra == op_pre_Integrate)
            {
                j = color_integral(j + 1);
            }
            else
            {
                j++;
            }
        }
        else if (tokens[j].lextype == lextype_expr)
        {
            j = color_subbox(j);
        }
        else
        {
            j++;
        }
    }

//std::cout << "color_expr returning " << j << std::endl;
    return j;
}


size_t bsymer::color_expr_inf_prec(size_t startj, Precedence p)
{
//std::cout << "color_expr("<<startj<<") called" << std::endl;

    size_t j = startj;
    while (tokens[j].lextype != lextype_MAX)
    {
        if (tokens[j].lextype == lextype_symbol_1st)
        {
            color_symbol(j);
            if (_symbol_matches_module_type(array + tokens[j].idx)) {
                j = color_module(j + 1);
            } else if (_symbol_matches_block_type(array + tokens[j].idx)) {
                j = color_block(j + 1);
            } else if (_symbol_matches_table_type(array + tokens[j].idx)) {
                j = color_table(j + 1);
            } else {
                j++;
            }
        }
        else if (    tokens[j].lextype == lextype_comma
                  || tokens[j].lextype == lextype_parenth_close
                  || tokens[j].lextype == lextype_bracket_close
                  || tokens[j].lextype == lextype_curly_close)
        {
//std::cout << "color_expr returning " << j << std::endl;
            return j;
        }
        else if (   (tokens[j].lextype == lextype_parenth_open
                  || tokens[j].lextype == lextype_bracket_open
                  || tokens[j].lextype == lextype_curly_open) && tokens[j].extra >= 0)
        {
            color_range(j + 1, tokens[j].extra + 1);
            j = tokens[j].extra + 1;

        }
        else if (tokens[j].lextype == lextype_opinf_1st)
        {
            if (p > infix_prec(static_cast<Operator>(tokens[j].extra))) {
                return j;
            }
            if (tokens[j].extra == op_inf_Set) {
                j = color_rule(j + 1, prec_inf_Set);
            } else if (tokens[j].extra == op_inf_SetDelayed) {
                j = color_rule(j + 1, prec_inf_SetDelayed);
            } else if (tokens[j].extra == op_inf_Rule) {
                j = color_rule(j + 1, prec_inf_Rule);
            } else if (tokens[j].extra == op_inf_RuleDelayed) {
                j = color_rule(j + 1, prec_inf_RuleDelayed);
            } else {
                j++;
            }
        }
        else if (tokens[j].lextype == lextype_oppost_1st)
        {
            if (p > postfix_prec(static_cast<Operator>(tokens[j].extra)))
            {
                return j;
            }
            j++;
        }
        else if (tokens[j].lextype == lextype_oppre_1st)
        {
            // TODO: fix this
            if (tokens[j].extra == op_pre_Integrate)
            {
                j = color_integral(j + 1);
            }
            else
            {
                j++;
            }
        }
        else if (tokens[j].lextype == lextype_expr)
        {
            j = color_subbox(j);
        }
        else
        {
            j++;
        }
    }

//std::cout << "color_expr returning " << j << std::endl;
    return j;
}

size_t bsymer::skip_expr(size_t startj) {
//std::cout << "skip_expr("<<startj<<") called" << std::endl;

    size_t j = startj;
    while (tokens[j].lextype != lextype_MAX)
    {
        if (    tokens[j].lextype == lextype_comma
                    || tokens[j].lextype == lextype_parenth_close
                    || tokens[j].lextype == lextype_bracket_close
                    || tokens[j].lextype == lextype_curly_close) {

//std::cout << "skip_expr returning " << j << std::endl;

            return j;

        } else if (   (tokens[j].lextype == lextype_parenth_open
                    || tokens[j].lextype == lextype_bracket_open
                    || tokens[j].lextype == lextype_curly_open) && tokens[j].extra >= 0) {
            j = tokens[j].extra + 1;
        } else {
            j++;
        }
    }

//std::cout << "skip_expr returning " << j << std::endl;
    return j;
}


int asdf_bnode_sumprod_argc(boxbase* b)
{
    if (b->get_type() == BNTYPE_UNDEROVER)
    {
        underoverscriptbox* r = dynamic_cast<underoverscriptbox*>(b);
        if (r->under.cbox->child.size() == 2
            && ibox_is_char(r->under.cbox->child[0].cibox, CHAR_Sum, CHAR_Product))
        {
            return 2;
        }
    }
    if (b->get_type() == BNTYPE_UNDER)
    {
        underscriptbox* r = dynamic_cast<underscriptbox*>(b);
        if (r->under.cbox->child.size() == 2
            && ibox_is_char(r->under.cbox->child[0].cibox, CHAR_Sum, CHAR_Product))
        {
            return 1;
        }
    }

    return 0;
}



size_t bsymer::color_subbox(size_t startj)
{
//std::cout << "color_subbox called " << startj << std::endl;

    size_t len_module  = vars_module.size();
    size_t len_block   = vars_block.size();
    size_t len_table   = vars_table.size();
    size_t len_pattern = vars_pattern.size();

    for (size_t k = 0; k < box_patterns_idx.size(); k++)
    {
        assert(box_patterns_idx[k] != startj);
    }
    box_patterns_idx.push_back(startj);
    box_patterns_vars.push_back(std::vector<iboxarrayelem*>());

    boxbase * us = ibox_to_ptr(array[tokens[startj].idx].cibox);

/*
    if (asdf_bnode_sumprod_argc(us) > 0)
    {
        Operator op = ibox_is_char(array[tokens[startj].idx].cibox->, CHAR_Sum) ? op_pre_Sum : op_pre_Product;
        Operator op = op_pre_Sum;

        iboxarrayelem* sumvar = nullptr;

        for (int32_t i = 1; i < bnode_len(array[tokens[startj].idx].child); i++)
        {
            boxnode * us = bto_node(bnode_child(array[tokens[startj].idx].child, i));
            assert(us->header.type == BNTYPE_ROW);
            us->extra1 |= BNFLAG_COLORED;

            bsymer B;
            std::swap(B.vars_module, vars_module);
            std::swap(B.vars_block, vars_block);
            std::swap(B.vars_table, vars_table);
            std::swap(B.vars_pattern, vars_pattern);

            B.add_scolor(us->array, 0, us->len-1);

            if (i == 1 && B.tokens[0].lextype == lextype_symbol_1st)
            {
                int k = tokens[0].idx;
                sumvar = B.array + B.tokens[0].idx;
                boxarrayelem* c = sumvar;
                do {
                    c->child = bchar_addsymtype(c->child, semtype_table);
                    c++;
                } while (   bis_char(c->child)
                         && bchar_lextype(c->child) == lextype_symbol);
            }

            for (size_t k = 0; k < B.tokens.size(); k++) {
                if (B.tokens[k].lextype == lextype_pattern_1st) {
                    box_patterns_vars.back().push_back(B.array + B.tokens[k].idx);
                }
            }
            for (size_t k1 = 0; k1 < B.box_patterns_vars.size(); k1++)
            {
                for (size_t k2 = 0; k2 < B.box_patterns_vars[k1].size(); k2++)
                {
                    box_patterns_vars.back().push_back(B.box_patterns_vars[k1][k2]);
                }
            }

            assert(len_module  == B.vars_module.size());
            assert(len_block   == B.vars_block.size());
            assert(len_table   == B.vars_table.size());
            assert(len_pattern == B.vars_pattern.size());
            std::swap(B.vars_module, vars_module);
            std::swap(B.vars_block, vars_block);
            std::swap(B.vars_table, vars_table);
            std::swap(B.vars_pattern, vars_pattern);
        }

        if (box_patterns_vars.back().empty())
        {
            box_patterns_idx.pop_back();
            box_patterns_vars.pop_back();
        }

        if (sumvar != nullptr)
        {
            vars_table.push_back(std::vector<boxarrayelem*>(1,sumvar));
            startj = color_expr_inf_prec(startj + 1, prefix_prec(op));
            vars_table.pop_back();            
        }
        else
        {
            startj++;
        }
    }
    else
*/
    {
        us->set_scolor(this);
        if (box_patterns_vars.back().empty())
        {
            box_patterns_idx.pop_back();
            box_patterns_vars.pop_back();
        }
        startj++;
    }

    return startj;
}

bool bsymer::color_symbol_from_vec(size_t startj, const std::vector<std::vector<iboxarrayelem*>>&v, int32_t color)
{
//std::cout << "color_symbol_from_vec called" << std::endl;
    for (size_t i = 0; i < v.size(); i++)
    {
        for (size_t j = 0; j < v[i].size(); j++)
        {
            if (_symbol_matches(array + tokens[startj].idx, v[i][j]))
            {
                tokens[startj].color = color;
//std::cout << "color_symbol_from_vec returning true" << std::endl;
                return true;
            }
        }
    }
//std::cout << "color_symbol_from_vec returning false" << std::endl;
    return false;
}

void bsymer::color_symbol(size_t startj)
{
//std::cout << "color_symbol called" << std::endl;
    if (color_symbol_from_vec(startj, vars_pattern, semtype_capture))
        return;
    if (color_symbol_from_vec(startj, vars_module, semtype_module))
        return;
    if (color_symbol_from_vec(startj, vars_block, semtype_block))
        return;
    if (color_symbol_from_vec(startj, vars_table, semtype_table))
        return;
}


size_t bsymer::color_module(size_t startj)
{
//std::cout << "color_module(" << startj << ")" << std::endl;

    size_t j = startj;
    std::vector<iboxarrayelem*> temp;

    if (tokens[j].lextype != lextype_bracket_open)
        return j;
    j++;

    if (tokens[j].lextype != lextype_curly_open)
        return j;
    j++;

get_variable:

    if (tokens[j].lextype == lextype_symbol_1st) {
        temp.push_back(array + tokens[j].idx);
        tokens[j].color = semtype_module;
        j++;
    }
    j = color_expr(j);

    if (tokens[j].lextype == lextype_comma) {
        j++;
        goto get_variable;
    }

    if (tokens[j].lextype != lextype_curly_close)
        return j;
    j++;

    if (tokens[j].lextype != lextype_comma)
        return j;
    j++;

    vars_module.push_back(temp);
    j = color_expr(j);
    vars_module.pop_back();

    if (tokens[j].lextype == lextype_bracket_close)
        j++;

//std::cout << "color_module done returning " << j << std::endl;

    return j;
}


size_t bsymer::color_block(size_t startj)
{
//std::cout << "color_block(" << startj << ")" << std::endl;

    size_t j = startj;
    std::vector<iboxarrayelem*> temp;

    if (tokens[j].lextype != lextype_bracket_open)
        return j;
    j++;

    if (tokens[j].lextype != lextype_curly_open)
        return j;
    j++;

get_variable:

    if (tokens[j].lextype == lextype_symbol_1st) {
        temp.push_back(array + tokens[j].idx);
        tokens[j].color = semtype_block;
        j++;
    }
    j = color_expr(j);

    if (tokens[j].lextype == lextype_comma) {
        j++;
        goto get_variable;
    }

    if (tokens[j].lextype != lextype_curly_close)
        return j;
    j++;

    if (tokens[j].lextype != lextype_comma)
        return j;
    j++;

    vars_block.push_back(temp);
    j = color_expr(j);
    vars_block.pop_back();

    if (tokens[j].lextype == lextype_bracket_close)
        j++;

//std::cout << "color_block done returning " << j << std::endl;
    return j;
}



size_t bsymer::color_table(size_t startj)
{
//std::cout << "color_table(" << startj << ")" << std::endl;

    size_t j = startj;
    size_t arg1_end, arg1_start;
    int newvar;

    if (tokens[j].lextype != lextype_bracket_open)
        return j;
    j++;

    vars_table.push_back(std::vector<iboxarrayelem*>());

    arg1_start = j;
    j = skip_expr(j);
    arg1_end = j;

    if (tokens[j].lextype != lextype_comma)
        goto finished;
    j++;

get_list:

    if (tokens[j].lextype != lextype_curly_open)
        goto finished;
    j++;

    if (  tokens[j+0].lextype == lextype_symbol_1st
       && tokens[j+1].lextype == lextype_comma) {
        newvar = tokens[j+0].idx;
        tokens[j+0].color = semtype_table;
        j += 1;
    } else {
        goto finished;
    }

    do {
        j++;
        j = color_expr(j);
    } while (tokens[j].lextype == lextype_comma);

    vars_table.back().push_back(array + newvar);


    if (tokens[j].lextype != lextype_curly_close)
        goto finished;
    j++;

    if (tokens[j].lextype == lextype_comma) {
        j++;
        goto get_list;
    }

    if (tokens[j].lextype == lextype_bracket_close)
        j++;

finished:
    color_range(arg1_start, arg1_end);
    vars_table.pop_back();
    return j;
}

size_t bsymer::color_integral(size_t startj)
{
//std::cout << "color_integral(" << startj << ")" << std::endl;

// TODO: handle in one go all of the integrals on this line

    size_t j = startj;
    size_t arg1_end, arg1_start;
    int newvar;

    arg1_start = j;

    int32_t intcount = 0;

    while (tokens[j].lextype != lextype_MAX)
    {
        if (    tokens[j].lextype == lextype_comma
                    || tokens[j].lextype == lextype_parenth_close
                    || tokens[j].lextype == lextype_bracket_close
                    || tokens[j].lextype == lextype_curly_close)
        {
            return startj;
        }
        else if (   (tokens[j].lextype == lextype_parenth_open
                  || tokens[j].lextype == lextype_bracket_open
                  || tokens[j].lextype == lextype_curly_open) && tokens[j].extra >= 0)
        {
            j = tokens[j].extra + 1;
        }
        else if (tokens[j].lextype == lextype_oppre_1st && tokens[j].extra == op_pre_Integrate)
        {
            intcount++;
            j++;
        }
        else if (tokens[j].lextype == lextype_oppre_1st && tokens[j].extra == op_pre_DifferentialD)
        {
            intcount--;
            if (intcount < 0)
            {
                break;
            }
            j++;
        }
        else
        {
            j++;
        }
    }

    if (!(tokens[j].lextype == lextype_oppre_1st && tokens[j].extra == op_pre_DifferentialD))
    {
        return startj;
    }

    if (tokens[j+1].lextype != lextype_symbol_1st)
    {
        return startj;
    }

    vars_table.push_back(std::vector<iboxarrayelem*>());
    vars_table.back().push_back(array + tokens[j+1].idx);

    color_range(arg1_start, j+2);
    vars_table.pop_back();
    return j+2;
}



void bsymer::collect_patterns_before(size_t endj, Precedence p)
{
//std::cout << "collect_patterns_before(" << endj << ", " << p << ")" << std::endl;
    size_t j = endj;
    while (j > 0)
    {
        j--;
        if (tokens[j].lextype == lextype_pattern_1st)
        {
            vars_pattern.back().push_back(array + tokens[j].idx);
            tokens[j].color = semtype_capture;
        }
        else if (  tokens[j].lextype == lextype_comma
                || tokens[j].lextype == lextype_parenth_open
                || tokens[j].lextype == lextype_bracket_open
                || tokens[j].lextype == lextype_curly_open)
        {
            return;

        }
        else if ( (tokens[j].lextype == lextype_parenth_close
                || tokens[j].lextype == lextype_bracket_close
                || tokens[j].lextype == lextype_curly_close) && tokens[j].extra >= 0)
        {
            size_t startj = tokens[j].extra;
            while (--j > startj)
            {
                if (tokens[j].lextype == lextype_pattern_1st)
                {
                    vars_pattern.back().push_back(array + tokens[j].idx);
                    tokens[j].color = semtype_capture;
                }
            }
        }
        else if (tokens[j].lextype == lextype_opinf_1st)
        {
            if (p > infix_prec(static_cast<Operator>(tokens[j].extra)))
            {
//printf("collect_patterns_before cut at %d\n",j);
                return;
            }
        }
        else if (tokens[j].lextype == lextype_oppre_1st)
        {
            if (p > prefix_prec(static_cast<Operator>(tokens[j].extra)))
            {
                return;
            }
        }
        else if (tokens[j].lextype == lextype_oppost_1st)
        {
            /*
            TODO: fix this
            if (p > postfix_prec(tokens[j].extra)) {
                return;
            }
            */

        }
        else if (tokens[j].lextype == lextype_expr)
        {
//std::cout << "collect_patterns_before got expr  j = " << j << std::endl;

            size_t l = box_patterns_idx.size();
            assert(l == box_patterns_vars.size());

//std::cout << "l: " << l << std::endl;

            size_t k1;
            for (k1 = 0; k1 < l; k1++)
            {
//std::cout << "box_patterns_idx[" << k1 << "] = " << box_patterns_idx[k1] << std::endl;
                if (box_patterns_idx[k1] == j)
                {
                    for (size_t k2 = 0; k2 < box_patterns_vars[k1].size(); k2++)
                    {
                        iboxarrayelem* c = box_patterns_vars[k1][k2];
                        vars_pattern.back().push_back(c);
                        do {
                            c->cibox = iboxchar_addsymtype(c->cibox, semtype_capture);
                            c++;
                        } while (   ibox_is_char(c->cibox)
                                 && iboxchar_lextype(c->cibox) == lextype_symbol);
                    }
                    break;
                }
            }
        }
        else
        {

        }
    }
}

size_t bsymer::color_rule(size_t startj, Precedence p)
{
    size_t j;
    vars_pattern.push_back(std::vector<iboxarrayelem*>());
    collect_patterns_before(startj - 1, p);
    j = color_expr_inf_prec(startj, p);
    vars_pattern.pop_back();
    return j;
}

