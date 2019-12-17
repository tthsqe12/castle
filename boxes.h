#pragma once

#include "types.h"
#include "ex.h"
#include "graphics.h"
#include "ex_parse.h"

#define BNTYPE_TEXT      -1
#define BNTYPE_STRING    -2
#define BNTYPE_PLOT2D    -3
#define BNTYPE_PLOT3D    -4
#define BNTYPE_NULLER    -6
#define BNTYPE_ERROR     -7
#define BNTYPE_ROOT      -11
#define BNTYPE_COL       -12
#define BNTYPE_ROW       -13
#define BNTYPE_CELLGROUP -20
#define BNTYPE_CELL      -21
#define BNTYPE_FRAC      -22
#define BNTYPE_SQRT      -23
#define BNTYPE_SUPER     -24
#define BNTYPE_SUB       -25
#define BNTYPE_SUBSUPER  -26
#define BNTYPE_UNDER     -27
#define BNTYPE_OVER      -28
#define BNTYPE_UNDEROVER -29
#define BNTYPE_GRID      -30
#define BNTYPE_ROT       -31
#define BNTYPE_BUTTON    -32
#define BNTYPE_TABVIEW   -33
#define BNTYPE_MONO      -34

#define BNFLAG_MEASURED    1
#define BNFLAG_COLORED     2
#define BNFLAG_OPEN        8
#define BNFLAG_PRINTMARK   16
#define BNFLAG_CURSORAMARK 32
#define BNFLAG_CURSORBMARK 64

#define CURSOR_WIDTH 3
#define COL_EXTRA 1

#define GRID_EXTRAX1 (0.4)
#define GRID_EXTRAX2 (0.9)
#define GRID_EXTRAX3 (0.4)
#define GRID_EXTRAY1 (0.2)
#define GRID_EXTRAY2 (0.5)
#define GRID_EXTRAY3 (0.2)

#define GP_SIZEX 7
#define GP_EXTRAX 3

#define MAX_ROW_WIDTH 5000
#define MIN_ROW_WIDTH 50
#define ROW_DECREMENT 10

#define MFLAG_EXSPACE 1

#define DFLAG_SCOLOR 1
#define DFLAG_NLINE  2
#define DFLAG_SELECTION 4
#define DFLAG_IGNORESEL 8


enum cellType {
    cellt_INPUT,
    cellt_OUTPUT,
    cellt_MESSAGE,
    cellt_PRINT,
    cellt_TEXT,
    cellt_BOLDTEXT,
    cellt_SUBSUBSECTION,
    cellt_SUBSECTION,
    cellt_SECTION,
    cellt_TITLE,
    cellt_max
};

extern const char* cell_type_names[cellt_max];

enum moveRet {
    moveret_OK,
    moveret_Replace,
    moveret_End
};
enum moveArg {
    movearg_Left,
    movearg_Right,
    movearg_ShiftLeft,
    movearg_ShiftRight,
    movearg_Last,
    movearg_First,
    movearg_Up,
    movearg_Down,
    movearg_ShiftUp,
    movearg_ShiftDown,
    movearg_Switch,
    movearg_Home,
    movearg_End,
    movearg_Tab
};

enum insertRet {
    insertret_Done,
    insertret_Bad
};
enum insertArg {
    insertarg_Newline,
    insertarg_Subscript,
    insertarg_Superscript,
    insertarg_Underscript,
    insertarg_Overscript,
    insertarg_Fraction,
    insertarg_Sqrt,
    insertarg_Rotation,
    insertarg_GridRow,
    insertarg_GridCol,
    insertarg_Text,
    insertarg_Graphics2D,
    insertarg_Graphics3D,
    insertarg_,
};

enum removeRet {
    removeret_OK,
    removeret_End,
    removeret_Replace,
    removeret_Bad
};
enum removeArg {
    removearg_Left,
    removearg_Right,
};

enum visitRet {
    visitret_OK
};
enum visitArg {
    visitarg_InvalidateAll
};


std::string stdvector_tostring(std::vector<int32_t> v);


class notebook;
class bsymer;

class boxmeasurearg {
public:
    uint32_t fi;
    int32_t deswidth;
    uint32_t mflags;
    int32_t level;
    boxmeasurearg(uint32_t fi_, int32_t deswidth_, uint32_t mflags_, int32_t level_) : fi(fi_), deswidth(deswidth_), mflags(mflags_), level(level_) {};
    boxmeasurearg(const boxmeasurearg& other) : fi(other.fi), deswidth(other.deswidth), mflags(other.mflags), level(other.level + 1) {};
};

class boxdrawarg {
public:
    notebook * nb;
    int32_t globx, globy;
    uint32_t dflags;
    int32_t level;
    aftransform* T;
    boxdrawarg(notebook * nb_, int32_t globx_, int32_t globy_, uint32_t dflags_, aftransform* T_)
         : nb(nb_), globx(globx_), globy(globy_), dflags(dflags_), level(0), T(T_) {};
    boxdrawarg(const boxdrawarg& other, int32_t x, int32_t y)
         : nb(other.nb), globx(other.globx + x), globy(other.globy + y), dflags(other.dflags), level(other.level+1), T(other.T) {};
    boxdrawarg(const boxdrawarg& other, int32_t x, int32_t y, uint32_t f)
         : nb(other.nb), globx(other.globx + x), globy(other.globy + y), dflags(other.dflags | f), level(other.level+1), T(other.T) {};
    inline std::string tostring() {
        std::string s;
        s.push_back('[');
        s.append(stdstring_to_string(T->orig_x));
        s.push_back(',');
        s.append(stdstring_to_string(T->orig_y));
        s.push_back(']');
        s.push_back('[');
        s.append(stdstring_to_string(T->theta));
        s.push_back(']');
        s.push_back('+');
        s.push_back('[');
        s.append(stdstring_to_string(globx));
        s.push_back(',');
        s.append(stdstring_to_string(globy));
        s.push_back(']');
        s.push_back(' ');
        s.append(stdstring_to_string(dflags));
        return s;
    };
};


class boxbase {
    int32_t type;
public:
    int32_t sizex, sizey, centery;

    boxbase(int32_t type_) : type(type_) {};
    boxbase(int32_t type_, int32_t sizex_, int32_t sizey_, int32_t centery_) : type(type_), sizex(sizex_), sizey(sizey_), centery(centery_) {};
    virtual ~boxbase() {};
    int32_t get_type() {return type;};
    void set_type(int32_t type_) {type = type_;};
    virtual void print(size_t indent, int32_t offx, int32_t offy) {assert(false);};
    virtual void insert_char(int32_t c) {assert(false);};
    virtual void measure(boxmeasurearg ma) {assert(false);};
    virtual void get_cursor(aftransform * T) {assert(false);};
    virtual void draw_pre(boxdrawarg da) {assert(false);};
    virtual void draw_main(boxdrawarg da) {assert(false);};
    virtual void draw_post(boxdrawarg da) {assert(false);};
    virtual boxbase * copy() {assert(false); return nullptr;};
    virtual void key_copy(boxbase*&b) {assert(false);};
    virtual void key_paste(boxbase*&b) {assert(false);};
    virtual visitRet visit(visitArg m) {assert(false); return visitret_OK;};
    virtual moveRet move(boxbase*&b, moveArg m) {assert(false); return moveret_OK;};
    virtual insertRet insert(boxbase*&b, insertArg m) {assert(false); return insertret_Done;};
    virtual removeRet remove(boxbase*&b, removeArg m) {assert(false); return removeret_OK;};
    virtual ex get_ex() {assert(false); return nullptr;};
    virtual void set_scolor(bsymer* B) {assert(false);};
};

class nullbox : public boxbase {
public:

    nullbox() : boxbase(BNTYPE_NULLER, 2, 2, 1) {};
    void print(size_t indent, int32_t offx, int32_t offy);
    void insert_char(int32_t c);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};

class charbox : public boxbase {
public:

    charbox(int32_t type_, int32_t sizex_, int32_t sizey_, int32_t centery_) : boxbase(type_, sizex_, sizey_, centery_) {assert(type_ > 0);};
    void print(size_t indent, int32_t offx, int32_t offy);
    void insert_char(int32_t c);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);    
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};

class errorbox : public boxbase {
public:

    errorbox(er e) : boxbase(BNTYPE_ERROR, 40, 40, 20) {};
    void print(size_t indent, int32_t offx, int32_t offy);
    void insert_char(int32_t c);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);    
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};


typedef union {
    boxbase* ptr;
    uint64_t imm;   // we need all 64 bits
} ibox;

inline bool ibox_is_imm(ibox b) {return 0 != (b.imm&1);}
inline bool ibox_is_ptr(ibox b) {return 0 == (b.imm&1);}
inline uint64_t ibox_to_imm(ibox b) {assert(ibox_is_imm(b)); return b.imm;}
inline boxbase* ibox_to_ptr(ibox b) {assert(ibox_is_ptr(b)); return b.ptr;}
inline int32_t iboximm_sizex(ibox b)   {return (ibox_to_imm(b)>>22)&0x03FF;} // 10 bits
inline int32_t iboximm_sizey(ibox b)   {return (ibox_to_imm(b)>> 1)&0x07FF;} // 11 bits
inline int32_t iboximm_centery(ibox b) {return (ibox_to_imm(b)>>12)&0x03FF;} // 10 bits
inline int32_t iboximm_type(ibox b)    {return (ibox_to_imm(b)>>32);}
inline int32_t iboximm_lextype(ibox b) {return (ibox_to_imm(b)>>48)&255;}
inline int32_t iboximm_symtype(ibox b) {return (ibox_to_imm(b)>>56)&255;}
inline int32_t ibox_sizex(ibox b)   {return ibox_is_imm(b) ? iboximm_sizex(b) : ibox_to_ptr(b)->sizex;}
inline int32_t ibox_sizey(ibox b)   {return ibox_is_imm(b) ? iboximm_sizey(b) : ibox_to_ptr(b)->sizey;}
inline int32_t ibox_centery(ibox b) {return ibox_is_imm(b) ? iboximm_centery(b) : ibox_to_ptr(b)->centery;}
inline int32_t ibox_type(ibox b)    {return ibox_is_imm(b) ? iboximm_type(b) : ibox_to_ptr(b)->get_type();}
inline int32_t ibox_is_char(ibox b) {return ibox_is_imm(b) || ibox_to_ptr(b)->get_type() >= 0;}
inline int32_t ibox_is_node(ibox b) {return ibox_is_ptr(b) && ibox_to_ptr(b)->get_type() < BNTYPE_NULLER;}
inline int32_t iboxchar_type(ibox b)    {assert(ibox_is_char(b)); return ibox_type(b);}
inline int32_t iboxchar_lextype(ibox b) {assert(ibox_is_char(b)); return (ibox_type(b) >> 16)&255;}
inline int32_t iboxchar_symtype(ibox b) {assert(ibox_is_char(b)); return (ibox_type(b) >> 24)&255;}

inline ibox ibox_copy(ibox a)
{
    if (ibox_is_imm(a))
    {
        return a;
    }
    else
    {
        ibox b;
        b.ptr = ibox_to_ptr(a)->copy();
        return b;
    }
}


inline ibox iboxptr_make(boxbase* p)
{
    ibox b;
    b.ptr = p;
    return b;
}

inline ibox iboximm_make(int32_t type)
{
    ibox b;
    b.imm = (((uint64_t)(type)) << 32) + 1;
    return b;
}

inline int32_t ibox_is_char(ibox b, int32_t c)
{
    if (ibox_is_imm(b))
        return ((iboximm_type(b) ^ c) & 65535) == 0;
    else
        return ibox_to_ptr(b)->get_type() >= 0 && ((ibox_to_ptr(b)->get_type() ^ c) & 65535) == 0;
}

inline int32_t ibox_is_char(ibox b, int32_t c1, int32_t c2)
{
    if (ibox_is_imm(b))
        return ((iboximm_type(b) ^ c1) & 65535) == 0
               || ((iboximm_type(b) ^ c2) & 65535) == 0;
    else
        return ibox_to_ptr(b)->get_type() >= 0 && (((ibox_to_ptr(b)->get_type() ^ c1) & 65535) == 0
                                                || ((ibox_to_ptr(b)->get_type() ^ c2) & 65535) == 0);
}

inline ibox iboximm_addlextype(ibox b, uint64_t t)
{
    b.imm = (ibox_to_imm(b) & 0x0FFFFFFFFFFFFULL) + (t << 48);
    return b;
}

inline ibox iboximm_addsymtype(ibox b, uint64_t t)
{
    b.imm = (ibox_to_imm(b) & 0x0FFFFFFFFFFFFFFULL) + (t << 56);
    return b;
}

inline ibox iboxchar_addsymtype(ibox b, uint64_t t)
{
    assert(ibox_is_char(b));
    if (ibox_is_imm(b))
    {
        return iboximm_addsymtype(b, t);
    }
    else
    {
        ibox_to_ptr(b)->set_type((ibox_to_ptr(b)->get_type() & 0x0FFFFFFULL) + (t << 24));
        return b;
    }
}




ibox iboxchar_set_sizes(ibox b, int32_t sizex, int32_t sizey, int32_t centery);
ibox iboxchar_set_sizes(ibox b, int32_t sizey, int32_t centery);


class iboxarrayelem {
public:
    ibox cibox;
    int32_t offx, offy;

    iboxarrayelem() : offx(0), offy(0) {cibox.ptr = nullptr;};
    iboxarrayelem(ibox cibox_) : offx(0), offy(0) {cibox = cibox_;};
    iboxarrayelem(boxbase* cibox_) : offx(0), offy(0) {cibox.ptr = cibox_;};
    iboxarrayelem(uint64_t cibox_) : offx(0), offy(0) {cibox.imm = cibox_;};
};


class rowbox : public boxbase {
public:
    int32_t cursor_a, cursor_b; // through: a < n, b >= n. selection: a < n, b < n
    std::vector<iboxarrayelem> child;
    uint32_t fs;
    uint32_t flags = 0;

    rowbox(int32_t size, int32_t cursor_a_, int32_t cursor_b_)
         : boxbase(BNTYPE_ROW),
           cursor_a(cursor_a_),
           cursor_b(cursor_b_)
    {
        child.resize(size);
    };

    rowbox(const char * a, size_t n) : boxbase(BNTYPE_ROW)
    {
        cursor_b = cursor_a = 0;
        size_t i = 0;
        while (i < n)
        {
            char16_t c;
            i += readonechar16(c, a + i);
            child.push_back(iboxarrayelem(iboximm_make(c)));
            cursor_b++;
        }
        child.push_back(iboxarrayelem(new nullbox()));
    };

    rowbox(const char * a) : boxbase(BNTYPE_ROW)
    {
        cursor_b = cursor_a = 0;
        while (*a)
        {
            child.push_back(iboxarrayelem(iboximm_make(*a)));
            a++;
            cursor_b++;
        }
        child.push_back(iboxarrayelem(new nullbox()));
    };

    ~rowbox()
    {
        for (auto& i : child)
            if (ibox_is_ptr(i.cibox))
                delete ibox_to_ptr(i.cibox);
    }

    void erasechild(int32_t i)
    {
        if (ibox_is_ptr(child[i].cibox))
            delete ibox_to_ptr(child[i].cibox);
        child.erase(child.begin() + i);
    }

    void replacechild(int32_t i, boxbase * b)
    {
        if (ibox_is_ptr(child[i].cibox))
            delete ibox_to_ptr(child[i].cibox);
        child[i].cibox.ptr = b;
    }

    void delete_selection();
    void select_prev_if_possible();
    void select_placeholder_if_possible();
    bool is_selected_placeholder()
    {
        return child.size() == 2 && cursor_a < 2 && cursor_b < 2 && cursor_a != cursor_b
                && ibox_is_char(child[0].cibox, CHAR_Placeholder);
    }

    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    void insert_char(int32_t c);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};


class rowboxarrayelem {
public:
    rowbox* cbox;
    int32_t offx, offy;

    rowboxarrayelem(rowbox* cbox_) : cbox(cbox_), offx(0), offy(0) {};
};


class monobox : public boxbase {
public:
    size_t row_cursor, col_cursor;
    size_t row_cursor2, col_cursor2;
    std::vector<std::vector<int32_t>> array;
    uint32_t fs;
    uint32_t flags = 0;
    int32_t dx, dy;
    int32_t mx, my;

    monobox(size_t nrows, size_t ncols, int32_t c, size_t row_cursor_, size_t col_cursor_)
     : boxbase(BNTYPE_MONO),
       row_cursor(row_cursor_), col_cursor(col_cursor_),
       row_cursor2(row_cursor_), col_cursor2(col_cursor_),
       array(nrows, std::vector<int32_t>(ncols, c))
    {
    }

    monobox(size_t row_cursor_, size_t col_cursor_,
            size_t row_cursor2_, size_t col_cursor2_,
            std::vector<std::vector<int32_t>> array_)
     : boxbase(BNTYPE_GRID),
       row_cursor(row_cursor_), col_cursor(col_cursor_),
       row_cursor2(row_cursor2_), col_cursor2(col_cursor2_),
       array(array_)
    {
    }

    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    void insert_char(int32_t c);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();

    void delete_selection();
};


class cellbox : public boxbase {
public:
    rowboxarrayelem body, label;
    monobox* mexpr;
    int32_t bracket_offy;
    int32_t bracket_sizey;
    cellType celltype;
    int32_t cursor; // 0 -> body, 1->label
    uint32_t flags;

    cellbox(rowbox * body_, cellType celltype_)
      : boxbase(BNTYPE_CELL),
        body(body_),
        label(nullptr),
        mexpr(nullptr),
        celltype(celltype_),
        cursor(0),
        flags(0)
    {};

    cellbox(rowbox * body_, rowbox * label_, monobox* mexpr_, cellType celltype_)
      : boxbase(BNTYPE_CELL),
        body(body_),
        label(label_),
        mexpr(mexpr_),
        celltype(celltype_),
        cursor(0),
        flags(0)
    {};

    ~cellbox()
    {
        delete body.cbox;
        if (label.cbox)
            delete label.cbox;
        if (mexpr)
            delete mexpr;
    }

    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    void insert_char(int32_t c);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();

    void toggle_cell_expr();
};


class cellarrayelem {
public:
    boxbase* cbox; // cellbox or cellgroupbox
    int32_t offx, offy;

    cellarrayelem() : cbox(nullptr), offx(0), offy(0) {};
    cellarrayelem(boxbase* cbox_) : cbox(cbox_), offx(0), offy(0) {};
};

class cellgroupbox : public boxbase {
public:
    std::vector<cellarrayelem> child;

    cellgroupbox(int32_t size)
         : boxbase(BNTYPE_CELLGROUP)
    {
        child.resize(size);
    };

    ~cellgroupbox()
    {
        for (auto& i : child)
            delete i.cbox;
    }

    void delete_selection() {assert(false);};
    void key_shiftenter(notebook* nb) {assert(false);};

    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T) {assert(false);};
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy() {assert(false); return nullptr;};
    void key_copy(boxbase*&b) {assert(false);};
    void key_paste(boxbase*&b) {assert(false);};
    void set_scolor(bsymer* B) {assert(false);};
    void insert_char(int32_t c) {assert(false);};
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m) {assert(false); return moveret_OK;};
    insertRet insert(boxbase*&b, insertArg m) {assert(false); return insertret_Done;};
    removeRet remove(boxbase*&b, removeArg m) {assert(false); return removeret_OK;};
    ex get_ex();
};

class rootbox : public boxbase {
public:
    std::vector<int32_t> cursor_a, cursor_b, cursor_t, cursor_s; // through to cell: b empty. cell section: b not empty
    std::vector<cellarrayelem> child;


    boxbase* _us();
    int32_t _pi();
    boxbase* _parent();
    int32_t _gpi();
    boxbase* _gparent();
    void _up1();
    void _deleteup1();
    void _down1(int32_t a);
    boxbase* _remove(int32_t idx);
    boxbase* _removerange(int32_t start, int32_t end);
    void _insert(boxbase* newchild);
    void _insertup1(boxbase* newparent, int32_t idx);
    void cell_insert(boxbase* c);
    void _fix_cell_insert();
    void _invalidate_downto(size_t n) {return;};
    void cell_delete();
    boxbase* cell_remove();
    bool goto_next_cellbreak();
    bool goto_prev_cellbreak();
    bool goto_first_cellbreak();
    void _cell_remove_sels(boxbase * r, std::vector<int32_t>&s, size_t i);
    void _cell_remove_selt(boxbase * before, boxbase * after, std::vector<int32_t>&t, size_t i);
    boxbase* _cell_remove_selection(std::vector<int32_t>&s, std::vector<int32_t>&t, size_t i);
    void makecell(cellType t);

    void erase_cell_mark(uint32_t mask);

    void print_cell(cellbox* c);


    rootbox()
      : boxbase(BNTYPE_ROOT)
    {};

    rootbox(int32_t size, int32_t cursor_a_, int32_t cursor_b_)
         : boxbase(BNTYPE_ROOT)
    {
        child.resize(size);
        cursor_a.push_back(cursor_a_);
        if (cursor_b_ <= size)
            cursor_b.push_back(cursor_b_);
    };

    ~rootbox()
    {
        for (auto& i : child)
            delete i.cbox;
    }

    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    void insert_char(int32_t c);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();

    void delete_selection();
    void key_shiftenter(notebook* nb);
    void toggle_cell_expr();
};


int32_t childlen(boxbase* b);
boxbase* childat(boxbase* b, size_t i);
int32_t offyat(boxbase* b, size_t i);
int32_t offxat(boxbase* b, size_t i);



class subscriptbox : public boxbase {
public:
    rowboxarrayelem sub;

    subscriptbox(rowbox * sub_) : boxbase(BNTYPE_SUB), sub(sub_) {};
    ~subscriptbox() {
        delete sub.cbox;
    }
    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    void insert_char(int32_t c);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};

class superscriptbox : public boxbase {
public:
    rowboxarrayelem super;

    superscriptbox(rowbox * super_) : boxbase(BNTYPE_SUPER), super(super_) {};
    ~superscriptbox() {
        delete super.cbox;
    }
    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    void insert_char(int32_t c);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};

class subsuperscriptbox : public boxbase {
public:
    rowboxarrayelem sub, super;
    int32_t cursor;

    subsuperscriptbox(rowbox * sub_, rowbox * super_, int32_t cursor_) : boxbase(BNTYPE_SUBSUPER), sub(sub_), super(super_), cursor(cursor_) {};
    ~subsuperscriptbox() {
        delete sub.cbox;
        delete super.cbox;
    }
    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    void insert_char(int32_t c);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};

class fractionbox : public boxbase {
public:
    rowboxarrayelem num, den;
    int32_t cursor;
    uint32_t fs;

    fractionbox(rowbox * num_, rowbox * den_, int32_t cursor_)
      : boxbase(BNTYPE_FRAC),
        num(num_),
        den(den_),
        cursor(cursor_)
    {};

    ~fractionbox()
    {
        delete num.cbox;
        delete den.cbox;
    }

    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    void insert_char(int32_t c);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};

class sqrtbox : public boxbase {
public:
    rowboxarrayelem inside;
    uint32_t fs;

    sqrtbox(rowbox * inside_) : boxbase(BNTYPE_SQRT), inside(inside_) {};
    ~sqrtbox() {
        delete inside.cbox;
    }
    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void insert_char(int32_t c);
    void set_scolor(bsymer* B);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};

class rotationbox : public boxbase {
public:
    rowboxarrayelem inside;
    uint32_t angle, original_angle;
//    uint32_t fs;

    rotationbox(rowbox * inside_, uint32_t angle_, uint32_t original_angle_) : boxbase(BNTYPE_ROT), inside(inside_), angle(angle_), original_angle(original_angle_) {};
    ~rotationbox() {
        delete inside.cbox;
    }
    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void insert_char(int32_t c);
    void set_scolor(bsymer* B);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};

class underscriptbox : public boxbase {
public:
    rowboxarrayelem body, under;
    int32_t cursor;

    underscriptbox(rowbox * body_, rowbox * under_, int32_t cursor_)
      : boxbase(BNTYPE_UNDER),
        body(body_),
        under(under_),
        cursor(cursor_)
    {};

    ~underscriptbox()
    {
        delete body.cbox;
        delete under.cbox;
    }

    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    void insert_char(int32_t c);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};

class overscriptbox : public boxbase {
public:
    rowboxarrayelem body, over;
    int32_t cursor;

    overscriptbox(rowbox * body_, rowbox * over_, int32_t cursor_)
      : boxbase(BNTYPE_OVER),
        body(body_),
        over(over_),
        cursor(cursor_)
    {};

    ~overscriptbox()
    {
        delete body.cbox;
        delete over.cbox;
    }

    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    void insert_char(int32_t c);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};

class underoverscriptbox : public boxbase {
public:
    rowboxarrayelem body, under, over;
    int32_t cursor;

    underoverscriptbox(rowbox * body_, rowbox * under_, rowbox * over_, int32_t cursor_)
      : boxbase(BNTYPE_OVER),
        body(body_),
        under(under_),
        over(over_),
        cursor(cursor_)
    {};

    ~underoverscriptbox()
    {
        delete body.cbox;
        delete under.cbox;
        delete over.cbox;
    }

    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    void insert_char(int32_t c);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};

class gridbox : public boxbase {
public:
    size_t row_cursor, col_cursor;
    size_t row_cursor2, col_cursor2;
    std::vector<std::vector<rowboxarrayelem>> array;

    gridbox(size_t nrows, size_t ncols, size_t row_cursor_, size_t col_cursor_)
     : boxbase(BNTYPE_GRID),
       row_cursor(row_cursor_), col_cursor(col_cursor_),
       row_cursor2(row_cursor_), col_cursor2(col_cursor_),
       array(nrows, std::vector<rowboxarrayelem>(ncols, rowboxarrayelem(nullptr)))
    {
    }

    gridbox(std::vector<std::vector<rowboxarrayelem>> array_, size_t row_cursor_, size_t col_cursor_)
     : boxbase(BNTYPE_GRID),
       row_cursor(row_cursor_), col_cursor(col_cursor_),
       row_cursor2(row_cursor_), col_cursor2(col_cursor_),
       array(array_)
    {
    }

    ~gridbox()
    {
        for (auto i = array.begin(); i != array.end(); ++i)
            for (auto j = i->begin(); j != i->end(); ++j)
                delete j->cbox;
    }

    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    void insert_char(int32_t c);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};

class graphics3dbox : public boxbase {
public:
    uint32_t fs;
//    std::vector<point3d> verts;
//    std::vector<uint32_t> tris;
    bitmap3d image;
    double center_x, center_y, center_z;
    double box_x, box_y, box_z;
    double fov;
    quaterion_t qangle;
    orthomatrix mangle;
int64_t fxn_evals, grad_evals;

    uex fxn, fxn_x, fxn_y, fxn_z;

    graphics3dbox();

    graphics3dbox(er f, er f_x, er f_y, er f_z, double x_min, double x_max, double y_min, double y_max, double z_min, double z_max)
      : boxbase(BNTYPE_PLOT3D),
        image(800, 800),
        fxn(ecopy(f)),
        fxn_x(ecopy(f_x)),
        fxn_y(ecopy(f_y)),
        fxn_z(ecopy(f_z))
    {
        qangle->w = 0.8;
        qangle->x = -0.6;
        qangle->y = 0.0;
        qangle->z = 0.1;
        fov = 1.8;

        box_x = std::abs(x_max - x_min)*0.5;
        box_y = std::abs(y_max - y_min)*0.5;
        box_z = std::abs(z_max - z_min)*0.5;

        center_x = (x_max + x_min)*0.5;
        center_y = (y_max + y_min)*0.5;
        center_z = (z_max + z_min)*0.5;


/*
        verts.resize(3);
        verts[0].x = +1.0; verts[0].y = -1.0; verts[0].z = -1.0;
        verts[1].x = -1.0; verts[1].y = +1.0; verts[1].z = -1.0;
        verts[2].x = -1.0; verts[2].y = -1.0; verts[2].z = +1.0;

        tris.resize(3);
        tris[0] = 0; tris[1] = 1; tris[2] = 2;

        boxX = 1.0;
        boxY = 1.0;
        boxZ = 1.0;
*/
        render();
    };

    double evalfxn(double t, point3d_t pos, point3d_t dir);
    void evalgrad(point3d_t g, double t, point3d_t pos, point3d_t dir);
    double find_sign_change(double min, double max, point3d_t pos, point3d_t dir);
    double find_sign_change2(double a, double a_eval, point3d_t a_grad, double b, double b_eval, point3d_t pos, point3d_t dir);
    void render();

    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void get_cursor(aftransform * T);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    boxbase * copy();
    void key_copy(boxbase*&b);
    void key_paste(boxbase*&b);
    void set_scolor(bsymer* B);
    void insert_char(int32_t c);
    visitRet visit(visitArg m);
    moveRet move(boxbase*&b, moveArg m);
    insertRet insert(boxbase*&b, insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};

int escape_seq_to_action(const char * s);
rowbox * steal_rowbox(rowbox * row, int32_t a, int32_t b);
bool made_into_placeholder(rowbox * r);

void drawtrect(double minx, double maxx, double miny, double maxy, uint32_t color, aftransform*T);
void drawbtrect(double blend, double minx, double maxx, double miny, double maxy, uint32_t color, aftransform*T);
void drawtline(double X1, double Y1, double X2, double Y2, double e, uint32_t color, aftransform*T);
void drawtlines(double * coords, size_t nlines, double e, uint32_t color, aftransform*T);
void drawtchar(uint32_t fs, char16_t c, int32_t sizex, int32_t sizey, int32_t offx, int32_t offy, uint32_t color, aftransform * T);
void drawbtchar(double blend, uint32_t fs, char16_t c, int32_t sizex, int32_t sizey, int32_t offx, int32_t offy, uint32_t color, aftransform * T);
void _draw_cellgroup_bracket(boxbase * us, boxdrawarg da);