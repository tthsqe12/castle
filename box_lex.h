#pragma once

#include <memory>

#include "types.h"
#include "ex_parse.h"
#include "boxes.h"

#define lextype_unknown    0
#define lextype_symbol     1
#define lextype_symbol_1st 2
#define lextype_number     3
#define lextype_number_1st 4
#define lextype_opinf       5
#define lextype_opinf_1st   6
#define lextype_string     7
#define lextype_string_1st 8
#define lextype_blank      9
#define lextype_blank_1st  10
#define lextype_bracket_open 11
#define lextype_bracket_close 12
#define lextype_parenth_open  13
#define lextype_parenth_close 14
#define lextype_curly_open  15
#define lextype_curly_close 16
#define lextype_expr          17
#define lextype_whitespace    18
#define lextype_slot      19
#define lextype_slot_1st  20
#define lextype_comma     21
#define lextype_comment     22
#define lextype_comment_1st 23

#define lextype_oppost       24
#define lextype_oppost_1st   25
#define lextype_oppre       26
#define lextype_oppre_1st   27

#define lextype_pattern_1st  28
#define lextype_message_name 29
#define lextype_MAX         30

#define semtype_unknown  0
#define semtype_defined  1
#define semtype_capture  2
#define semtype_table    3
#define semtype_block    4
#define semtype_module   5
#define semtype_conflict 6

#define lexstate_start                  0
#define lexstate_num_start              1
#define lexstate_num_start_zero         2
#define lexstate_num_after_carat        3
#define lexstate_num_after_star         4
#define lexstate_num_after_tick         5
#define lexstate_num_after_mantissa     6
#define lexstate_num_after_prec         7
#define lexstate_num_after_exp          8
#define lexstate_str_start              9
#define lexstate_str_after_escape       10
#define lexstate_str_after_escape_hex   11
#define lexstate_sym_start              12
#define lexstate_sym_after_context      13
#define lexstate_blank_1                14
#define lexstate_blank_1_dot            15
#define lexstate_blank_2                16
#define lexstate_blank_3                17
#define lexstate_blank_sym_start         18
#define lexstate_blank_sym_after_context 19
#define lexstate_slot_start             20
#define lexstate_slot_1_digit           21
#define lexstate_slot_1_alpha           22
#define lexstate_slot_2                 23
#define lexstate_comment_start              24
#define lexstate_comment_after_first_star   25
#define lexstate_comment_after_second_star  26

#define lexstate_message_name_start 27
#define lexstate_message_name       28

//#define lexstate_oper                   30


#define  lexstate_oper_plus                31
#define  lexstate_oper_plus_divide         32
#define  lexstate_oper_minus               33
#define  lexstate_oper_colon               34
#define  lexstate_oper_vbar                35
#define  lexstate_oper_dot                 36
#define  lexstate_oper_dot_dot             37
#define  lexstate_oper_times               38
#define  lexstate_oper_caret               39
#define  lexstate_oper_caret_colon         40
#define  lexstate_oper_divide              41
#define  lexstate_oper_divide_divide       42
#define  lexstate_oper_equal               43
#define  lexstate_oper_equal_equal         44
#define  lexstate_oper_equal_not           45
#define  lexstate_oper_and                 46
#define  lexstate_oper_bang                47
#define  lexstate_oper_greater             48
#define  lexstate_oper_less                49
#define  lexstate_oper_semi                50
#define  lexstate_oper_at                  51
#define  lexstate_oper_prime               52
#define  lexstate_oper_tilde               53


class blexer {
    uint32_t state;
    uint32_t idx;
    bool haveex;
public:
    std::unique_ptr<int32_t[]> penalty;
    std::unique_ptr<int32_t[]> type;
    std::unique_ptr<int32_t[]> extrax;

    blexer(int32_t len): penalty{new int32_t[len]}, type{new int32_t[len]}, extrax{new int32_t[len]} {
        state = lexstate_start;
        idx = 0;
        haveex = false;
        for (int32_t i = 0; i < len; i++)
        {
            extrax[i] = 0;
            penalty[i] = -1;
            type[i] = -1;
        }
    }

    int32_t add_char(char16_t c);
    void add_newline();
    void add_box(int btype);
};

class bsymentry
{
public:
    int32_t idx;
    int32_t lextype;
    int32_t extra;
    int32_t color;
    bsymentry(int32_t i, int32_t t)
    {
        idx = i;
        lextype = t;
        extra = -1;
        color = semtype_unknown;
    };
    bsymentry(int32_t i, int32_t t, int32_t x)
    {
        idx = i;
        lextype = t;
        extra = x;
        color = semtype_unknown;
    };
};


class bsymer
{
public:
    std::vector<bsymentry> tokens;
    iboxarrayelem * array;

    std::vector<std::vector<iboxarrayelem*>> vars_module;
    std::vector<std::vector<iboxarrayelem*>> vars_block;
    std::vector<std::vector<iboxarrayelem*>> vars_table;
    std::vector<std::vector<iboxarrayelem*>> vars_pattern;

    std::vector<int32_t> box_patterns_idx;
    std::vector<std::vector<iboxarrayelem*>> box_patterns_vars;

    void add_scolor(iboxarrayelem * array, int32_t start, int32_t end);
    void color_rowbox(rowbox* r);
    void color_range(size_t startj, size_t endj);
    size_t color_expr(size_t startj);
    size_t color_expr_inf_prec(size_t startj, Precedence p);
    size_t skip_expr(size_t startj);
    size_t color_subbox(size_t startj);
    bool color_symbol_from_vec(size_t startj, const std::vector<std::vector<iboxarrayelem*>>&v, int color);
    void color_symbol(size_t startj);
    size_t color_module(size_t startj);
    size_t color_block(size_t startj);
    size_t color_table(size_t startj);
    size_t color_integral(size_t startj);
    size_t color_rule(size_t startj, Precedence p);
    void collect_patterns_before(size_t endj, Precedence p);

    void print() {
        std::cout << "************ tokens *************" << std::endl;
        for (size_t j = 0; j < tokens.size(); j++) {
            std::cout << "tokens["<<j<<"]: idx "<< tokens[j].idx << ", ";
            std::cout << (tokens[j].lextype == lextype_parenth_open ?    "(   " :
                          tokens[j].lextype == lextype_parenth_close ?   ")   " :
                          tokens[j].lextype == lextype_bracket_open ?    "[   " :
                          tokens[j].lextype == lextype_bracket_close ?   "]   " :
                          tokens[j].lextype == lextype_curly_open ?      "{   " :
                          tokens[j].lextype == lextype_curly_close ?     "}   " :
                          tokens[j].lextype == lextype_symbol_1st ?      "sym " :
                          tokens[j].lextype == lextype_comma ?           ",   " :
                          tokens[j].lextype == lextype_pattern_1st ?     "pat " :
                          tokens[j].lextype == lextype_opinf_1st ?       "inf " :
                          tokens[j].lextype == lextype_oppre_1st ?       "pre " :
                          tokens[j].lextype == lextype_oppost_1st ?      "pst " :
                          tokens[j].lextype == lextype_expr ?            "box " :
                                                                         "??? ");
            std::cout << ", " << tokens[j].extra << ", ";
            std::cout << (tokens[j].color == semtype_unknown ?    "        " :
                          tokens[j].color == semtype_defined ?    "defined " :
                          tokens[j].color == semtype_capture ?    "capture " :
                          tokens[j].color == semtype_block ?      "block   " :
                          tokens[j].color == semtype_module ?     "module  " :
                          tokens[j].color == semtype_conflict ?   "conflict" :
                                                                  "???     ");
            std::cout << "|" << std::endl;
        }
    }
};
