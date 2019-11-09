#pragma once

#include "digits.h"
#include "ex.h"
#include "globalstate.h"
#include <fstream>

enum PError
{
    erNone,
    erUnexpectedEnd,
    erPost,
    erBad
};

enum Association {assoDefault, assoLeft, assoRight, assoNo};

enum Operator
{
    op_inf_GenPost,         //  //
    op_inf_Set,             //  =
    op_inf_GenPre,          //  @
    op_inf_Apply,           //  @@
    op_inf_Span,            //  ;;
    op_inf_Greater,         //  >
    op_inf_Put,             //  >>
    op_inf_GreaterEqual,    //  >=
    op_inf_Less,            //  <
    op_inf_StringJoin,      //  <>

    op_inf_LessEqual,       //  <=
    op_bang,                //  !
    op_pre_Not,             //  !
    op_post_Factorial,      //  !
    op_inf_Unequal,         //  !=
    op_post_Factorial2,     //  !!
    op_post_Unset,          //  =.
    op_inf_Equal,           //  ==
    op_inf_SameQ,           //  ===
    op_inf_UnsameQ,         //  =!=

    op_inf_Divide,          //  /
    op_inf_DivideBy,        //  /=
    op_inf_Map,             //  /@
    op_inf_ReplaceAll,      //  /.
    op_inf_MapAll,          //  //@
    op_inf_ReplaceRepeated, //  //.
    op_inf_Power,           //  ^
    op_inf_UpSetDelayed,    //  ^:=
    op_inf_UpSet,           //  ^=
    op_inf_Times,           //  *

    op_inf_TimesBy,         //  *=
    op_inf_NonCommutativeMultiply,  // **
    op_inf_Rule,            //  ->
    op_inf_SubtractFrom,    //  -=
    op_inf_AddTo,           //  +=
    op_inf_MessageName,     //  ::
    op_inf_SetDelayed,      //  :=
    op_inf_RuleDelayed,     //  :>
    op_post_Derivative,     //  ' n times
    op_inf_Alternatives,    //  |

    op_inf_Or,              //  ||
    op_inf_Dot,             //  .
    op_inf_Condition,       //  /;
    op_post_Repeated,       //  ..
    op_post_RepeatedNull,   //  ...
    op_inf_tag,             //  /:
    op_post_Function,       //  &
    op_inf_And,             //  &&
    op_CompoundExpression, op_inf_CompoundExpression, op_postd_CompoundExpression, //  ;
	op_inf_StringExpression,
    op_inf_PatternTest,     //  ?

    op_Plus, op_pre_Plus, op_inf_Plus,                      //  +
    op_Minus, op_pre_Minus, op_inf_Minus,                   //  -
    op_Decrement, op_pre_PreDecrement, op_post_Decrement,   //  --
    op_pre_Sum,

    op_pre_Product,
    op_Increment, op_pre_PreIncrement, op_post_Increment,   //  ++
    op_Pattern, op_inf_Pattern, op_inf_Optional,            //  :
    op_post_SuperscriptBox,
    op_post_SubscriptBox,
    op_post_SubsuperscriptBox,

    opComma,
    opEnd,
    bracLRound, bracRRound,
    bracLSquare, bracRSquare,
    bracLCurly, bracRCurly,
    bracLDouble, bracRDouble,

    bracLDoubleSpecial, bracRDoubleSpecial,
    bracLAssociation, bracRAssociation,
    bracLAssociationSpecial, bracRAssociationSpecial,
    op_pre_Integrate,
    op_pre_DifferentialD,
    op_none
};

enum Precedence
{
    prec_none,
    prec_lowest,
    prec_inf_CompoundExpression,    //  a;b;c
    prec_postd_CompoundExpression   /*  a;b;c; */ = prec_inf_CompoundExpression,
    prec_inf_tag_Set_SetDelayed,    //  /: := =
    prec_inf_Set = prec_inf_tag_Set_SetDelayed,
    prec_inf_SetDelayed = prec_inf_tag_Set_SetDelayed,
    prec_inf_UpSet,
    prec_inf_UpSetDelayed,
    prec_post_Unset,                //  =.
    prec_inf_to_postfix,            //  //
    prec_post_Function,             //  &
    prec_inf_DivideBy,              //  \=
    prec_inf_TimesBy,               //  *=
    prec_inf_SubtractFrom,          //  -=
    prec_inf_AddTo,                 //  +=
    prec_inf_ReplaceRepeated,       //  //.
    prec_inf_ReplaceAll,            //  /.
    prec_inf_RuleDelayed,           //  :>
    prec_inf_Rule,                  //  ->
    prec_inf_Condition,             //  /;
	prec_inf_StringExpression,		//  a~~b
    prec_inf_Pattern,               //  a:b
    prec_inf_Alternatives,          //  |
    prec_post_RepeatedNull,         //  ...
    prec_post_Repeated,             //  ..
    prec_inf_Or,                    //  ||
    prec_inf_And,                   //  &&
    prec_pre_Not,                   //  !
    prec_inf_Inequality,            //  ===, =!=, ==, !=, >, <=, <, <=
    prec_inf_Span,                  //  ;;
    prec_inf_Plus,                  //  +
    prec_inf_Minus,                 //  -
    prec_pre_Plus,                  //  +
    prec_pre_Minus,                 //  -
    prec_pre_Sum,                   //
    prec_pre_DifferentialD,         //
    prec_pre_Product,               //
    prec_inf_Times,                 //  *
    prec_inf_Divide,                //  /
    prec_inf_Dot,                   //  .
    prec_post_SubsuperscriptBox,    //  
    prec_post_SuperscriptBox,       //  
    prec_inf_Power,                 //  ^
    prec_inf_StringJoin,            //  <>
    prec_post_Derivative,           //  '
    prec_post_Factorial2,           //  !
    prec_post_Factorial,            //  !
    prec_inf_Apply1,                //  @@@
    prec_inf_Apply,                 //  @@
    prec_inf_MapAll,                //  //@
    prec_inf_Map,                   //  /@
    prec_inf_to_prefix,             //  @
    prec_pre_PreDecrement,          //  --
    prec_pre_PreIncrement,          //  ++
    prec_post_Decrement,            //  --
    prec_post_Increment,            //  ++
    prec_postd_doublesquarebracket, //  a[[b,c]]
    prec_postd_squarebracket,       //  a[b,c]
    prec_inf_PatternTest,           //  ?
    prec_inf_Subscript,             //
    prec_post_SubscriptBox,         //  
    prec_inf_MessageName,           // a::b::c
    prec_highest
};

constexpr Precedence operator +(Precedence a, int b) {return Precedence(int(a) + b);}
constexpr Precedence operator -(Precedence a, int b) {return Precedence(int(a) - b);}


class syntax_report
{
public:
    std::string descr;
    uex around;
    slong line;
    PError error;

    syntax_report() : error(erNone) {};

    bool have_error() const {return error != erNone;};
    ex translate_error();
    ex near_error();

    void handle_row_error(PError e, er b, size_t i, size_t si);
    void handle_row_error(PError e, er b, size_t i);
    void handle_row_end_error(er b);
    void handle_string_error(PError e, er b, size_t si);
    void handle_cstr_error(PError e, const char * b, size_t si);
};


class eparser
{
public:
    std::vector<uex> out;
    std::vector<wex> estack;
    std::vector<Operator> bstack; // last bracket operator on estack
    std::vector<char16_t> store;
    std::string esc_name;
    std::string num_base;
    std::string num_mantissa;
    std::string num_prec;
    std::string num_exp;
    std::string num_mantissa2;
    std::string num_exp2;
    std::string sym_context;
    std::string sym_name;
    xfmpz number;
    slong digits_after_dot, digits_after_dot2;
    uex pattern_name, blank_type;
    uex prevTk, currTk, nextTk;
    void (*stage)(eparser&p, int32_t c); // stage of lexer
    PError error = erNone;
    slong comment_level;
    int num_tick_count;
    bool num_hex;
    bool toplevel;

    eparser();
    eparser(bool toplevel_);

    void handle_rawchar(int32_t c);
    void handle_char(int32_t c);
    void handle_ex(ex e);
    void handle_post_oper(uint64_t d0);
    void handle_pre_oper(uint64_t d0);
    void handle_inf_oper(uint64_t d0);
    void handle_end();
    void handle_newline();

    void handle_token_end();
    void handle_token_ex(ex e);
    void handle_token_raw(uint64_t d0);
    
    ex make_num();
    ex make_hexnum1();
    ex make_hexnum2();

    void try_newexpr();
    void popop(Precedence prec);
    void pushop_prefix(Operator pre);
    void pushop_infix(Operator in);
    void pushop_postfix(Operator post);
    void pushop_postfix(Operator post, ex H);
    void pushop_prefix_infix(Operator pre, Operator in);
    void pushop_prefix_postfix(Operator pre, Operator post);
    void pushex(ex E);
//    ex scan_oper_store(size_t&i, int32_t nextchar);

    bool have_top_ex() {return error == erNone &&  !estack.empty()   && !eis_raw(estack.back().get()); }
    bool have_one_ex() {return error == erNone && estack.size() == 1 && !eis_raw(estack.back().get()); }
    bool have_no_ex() {return error == erNone && estack.size() == 0;}
    bool have_comma_sequence();
    bool have_inequality(size_t&q);
};

Precedence prefix_prec(Operator op);
Precedence infix_prec(Operator op);
Precedence postfix_prec(Operator op);
Association op_asso(Operator op);
er op_to_sym(Operator op);

int32_t esccode_to_char(const char * s);
char16_t escapedname_to_char(const char * s);
//const char * char_to_escapedname(char16_t c);

size_t readonechar16(char16_t&c, const unsigned char * a);
void stdstring_pushback_char16(std::string&s, char16_t c);
void stdstring_append_char16v(std::string&s, std::vector<char16_t>&v);
void stdstring_pushback_cvtascii(std::string&s, char16_t c, bool esc);
void stdstring_append_cvtascii(std::string&s, std::string&a, bool esc);


class globalcharinfo {
public:

    unsigned char lookup[65536];
    globalcharinfo();
};

extern globalcharinfo gci;

inline bool isletterchar(char16_t c)  {return (gci.lookup[c] & 1) != 0;}
inline bool ispureopchar(char16_t c)  {return (gci.lookup[c] & 2) != 0;}
inline bool isbracketchar(char16_t c) {return (gci.lookup[c] & 4) != 0;}
inline bool isopchar(char16_t c)      {return (gci.lookup[c] & 6) != 0;}


ex exbox_to_exs(er b, syntax_report & sr);
void ex_parse_exboxs(std::vector<uex> &ans, er b, bool toplevel, syntax_report & sr);
void ex_parse_string(std::vector<uex> &ans, const char * ss, size_t sn, bool toplevel, syntax_report & sr);
void ex_parse_file(std::vector<uex> &ans, std::ifstream & is, bool toplevel, syntax_report & sr);

ex exbox_to_ex(er b, int &error, std::vector<size_t> &istack);
void ex_parse_exbox(std::vector<uex> &ans, er b, bool toplevel, int &error, std::vector<size_t> &istack);

