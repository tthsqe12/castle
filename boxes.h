#pragma once

#include "types.h"
#include "ex.h"
#include "graphics.h"
#include "ex_parse.h"

/************************** nodes **************************/

/*
    internal rules:

    - all not char nodes must have at least one child.
    - ROOT must only last child null, and all other children CELL or CELLGROUP.
    - CELLGROUP must have at least two children, all are CELL or CELLGROUP.
    - CELL, GRID, SQRT, SUPER, SUB must have childern ROW.
    - ROW must have child null for end of lines, and all other children not ROW.

    - Cursor must not point to ROW or CELLGROUP or ROOT nodes.

    The full list of rules is in the function notebok::set_ex.
*/

#define SELTYPE_NONE 0
#define SELTYPE_ROW 1
#define SELTYPE_GRID 3
#define SELTYPE_CELL 4


#define BNTYPE_TEXT    -1
#define BNTYPE_STRING  -2
#define BNTYPE_PLOT2D  -3
#define BNTYPE_PLOT3D  -4

#define BNTYPE_NULLER  -6

#define BNTYPE_ROOT -11
#define BNTYPE_COL  -12
#define BNTYPE_ROW  -13

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

#define BNTYPE_STUB -35



#define CELLTYPE_INPUT   0
#define CELLTYPE_OUTPUT  1
#define CELLTYPE_MESSAGE 2
#define CELLTYPE_PRINT   3
#define CELLTYPE_TEXT    4
#define CELLTYPE_BOLDTEXT    5
#define CELLTYPE_SUBSUBSECTION    6
#define CELLTYPE_SUBSECTION    7
#define CELLTYPE_SECTION    8
#define CELLTYPE_TITLE    9

#define BNFLAG_MEASURED  1
#define BNFLAG_COLORED   2
#define BNFLAG_OPEN      8
#define BNFLAG_MARKED    16


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
    cellt_TITLE
};

enum moveRet {
    moveret_OK,
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
    movearg_ShiftDown
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


class notebook;
class bsymer;

class boxmeasurearg {
public:
    uint32_t fi;
    int32_t deswidth;
    uint32_t mflags;
    boxmeasurearg(uint32_t fi_, int32_t deswidth_, uint32_t mflags_) : fi(fi_), deswidth(deswidth_), mflags(mflags_) {};
};

class boxdrawarg {
public:
    notebook * nb;
    int32_t globx, globy;
    uint32_t dflags;
    aftransform* T;
    boxdrawarg(notebook * nb_, int32_t globx_, int32_t globy_, uint32_t dflags_, aftransform* T_)
         : nb(nb_), globx(globx_), globy(globy_), dflags(dflags_), T(T_) {};
    boxdrawarg(const boxdrawarg& other, int32_t x, int32_t y)
         : nb(other.nb), globx(other.globx + x), globy(other.globy + y), dflags(other.dflags), T(other.T) {};
    boxdrawarg(const boxdrawarg& other, int32_t x, int32_t y, uint32_t f)
         : nb(other.nb), globx(other.globx + x), globy(other.globy + y), dflags(other.dflags | f), T(other.T) {};
    inline std::string tostring() {
        std::string s;
        s.push_back('[');
        s.append(std::to_string(T->orig_x));
        s.push_back(',');
        s.append(std::to_string(T->orig_y));
        s.push_back(']');
        s.push_back('[');
        s.append(std::to_string(T->theta));
        s.push_back(']');
        s.push_back('+');
        s.push_back('[');
        s.append(std::to_string(globx));
        s.push_back(',');
        s.append(std::to_string(globy));
        s.push_back(']');
        s.push_back(' ');
        s.append(std::to_string(dflags));
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
    virtual void insert_char(char16_t c) {assert(false);};
    virtual void measure(boxmeasurearg ma) {assert(false);};
    virtual void draw_pre(boxdrawarg da) {assert(false);};
    virtual void draw_main(boxdrawarg da) {assert(false);};
    virtual void draw_post(boxdrawarg da) {assert(false);};
    virtual moveRet move(moveArg m) {assert(false);};
    virtual insertRet insert(insertArg m) {assert(false);};
    virtual removeRet remove(boxbase*&b, removeArg m) {assert(false);};
    virtual ex get_ex() {assert(false);};
    virtual void set_scolor(bsymer* B) {assert(false);};
};

class nullbox : public boxbase {
public:

    nullbox() : boxbase(BNTYPE_NULLER, 2, 2, 1) {};
    void print(size_t indent, int32_t offx, int32_t offy);
    void insert_char(char16_t c);
    void measure(boxmeasurearg ma);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    void set_scolor(bsymer* B);
    moveRet move(moveArg m);
    insertRet insert(insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};

class charbox : public boxbase {
public:

    charbox(int32_t type_, int32_t sizex_, int32_t sizey_, int32_t centery_) : boxbase(type_, sizex_, sizey_, centery_) {assert(type_ > 0);};
    void print(size_t indent, int32_t offx, int32_t offy);
    void insert_char(char16_t c);
    void measure(boxmeasurearg ma);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    void set_scolor(bsymer* B);
    moveRet move(moveArg m);    
    insertRet insert(insertArg m);
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

    rowbox(const unsigned char * a, size_t n) : boxbase(BNTYPE_ROW)
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
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    void set_scolor(bsymer* B);
    void insert_char(char16_t c);
    moveRet move(moveArg m);
    insertRet insert(insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};


class rowboxarrayelem {
public:
    rowbox* cbox;
    int32_t offx, offy;

    rowboxarrayelem(rowbox* cbox_) : cbox(cbox_), offx(0), offy(0) {};
};

class cellbox : public boxbase {
public:
    rowboxarrayelem body, label;
    cellType celltype;
    int32_t cursor; // 0 -> body, 1->label

    cellbox(rowbox * body_, cellType celltype_)
      : boxbase(BNTYPE_CELL),
        body(body_),
        label(nullptr),
        celltype(celltype_),
        cursor(0)
    {};

    cellbox(rowbox * body_, rowbox * label_, cellType celltype_)
      : boxbase(BNTYPE_CELL),
        body(body_),
        label(label_),
        celltype(celltype_),
        cursor(0)
    {};

    ~cellbox()
    {
        delete body.cbox;
        if (label.cbox)
            delete label.cbox;
    }

    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    void set_scolor(bsymer* B);
    void insert_char(char16_t c);
    moveRet move(moveArg m);
    insertRet insert(insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};


class cellboxarrayelem {
public:
    cellbox* cbox;
    int32_t offx, offy;

    cellboxarrayelem() : cbox(nullptr), offx(0), offy(0) {};
    cellboxarrayelem(cellbox* cbox_) : cbox(cbox_), offx(0), offy(0) {};
};


class rootbox : public boxbase {
public:
    int32_t cursor_a, cursor_b; // through: a < n, b > n. selection: a <= n, b <= n
    std::vector<cellboxarrayelem> childcells;

    rootbox()
      : boxbase(BNTYPE_ROOT)
    {};

    ~rootbox()
    {
        for (auto& i : childcells)
            delete i.cbox;
    }

    void delete_selection();
    void key_shiftenter(notebook* nb);

    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    void set_scolor(bsymer* B);
    void insert_char(char16_t c);
    moveRet move(moveArg m);
    insertRet insert(insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};


class subscriptbox : public boxbase {
public:
    rowboxarrayelem sub;

    subscriptbox(rowbox * sub_) : boxbase(BNTYPE_SUB), sub(sub_) {};
    ~subscriptbox() {
        delete sub.cbox;
    }
    void print(size_t indent, int32_t offx, int32_t offy);
    void measure(boxmeasurearg ma);
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    void set_scolor(bsymer* B);
    void insert_char(char16_t c);
    moveRet move(moveArg m);
    insertRet insert(insertArg m);
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
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    void set_scolor(bsymer* B);
    void insert_char(char16_t c);
    moveRet move(moveArg m);
    insertRet insert(insertArg m);
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
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    void set_scolor(bsymer* B);
    void insert_char(char16_t c);
    moveRet move(moveArg m);
    insertRet insert(insertArg m);
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
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    void set_scolor(bsymer* B);
    void insert_char(char16_t c);
    moveRet move(moveArg m);
    insertRet insert(insertArg m);
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
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    void insert_char(char16_t c);
    void set_scolor(bsymer* B);
    moveRet move(moveArg m);
    insertRet insert(insertArg m);
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
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    void set_scolor(bsymer* B);
    void insert_char(char16_t c);
    moveRet move(moveArg m);
    insertRet insert(insertArg m);
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
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    void set_scolor(bsymer* B);
    void insert_char(char16_t c);
    moveRet move(moveArg m);
    insertRet insert(insertArg m);
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
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    void set_scolor(bsymer* B);
    void insert_char(char16_t c);
    moveRet move(moveArg m);
    insertRet insert(insertArg m);
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
    void draw_pre(boxdrawarg ma);
    void draw_main(boxdrawarg ma);
    void draw_post(boxdrawarg ma);
    void set_scolor(bsymer* B);
    void insert_char(char16_t c);
    moveRet move(moveArg m);
    insertRet insert(insertArg m);
    removeRet remove(boxbase*&b, removeArg m);
    ex get_ex();
};



/*
*********************** old legacy interface ***********************************

    a "box" is either:
        (even) a pointer to a boxnode/boxchar/...
                or
        (odd) an immediate compression of a boxchar

    on a 64 bit system the box type should be passed and returned as a native type (i.e. in registers)
*/

typedef struct _boxheader {
    int32_t type, centery;
    int32_t sizex, sizey;
} boxheader;

typedef union _box {
    boxheader * ptr;
    uint64_t imm;   // we need all 64 bits
} box;


typedef struct _boxarrayelem {
    box child;
    int32_t offx, offy;
} boxarrayelem;

/* general node with any number of children */
typedef struct _boxnode {
    boxheader header;
                // BNTYPE_GRID  BNTYPE_ROW  BNTYPE_CELLGROUP    BNTYPE_CELL     BNTYPE_FRAC     BNTYPE_SQRT     BNTYPE_ROT       BNTYPE_TABVIEW
    uint32_t extra0; // # rows       font                            style      font            font            modified angle   current view idx
    uint32_t extra1; // # cols       flags       flags               flags                                      original angle
    int32_t len, alloc;
    er expr;
    boxarrayelem * array;
} boxnode;

/* a simply lowly character, atomic */
typedef struct {
    boxheader header;
} boxchar;

/* 3d graphics, atomic */
typedef struct {
    boxheader header;
//    graphics3d plot;
} boxplot3d;
typedef boxplot3d boxplot3d_t[1];

/* array of monospaced characters, atomic */
typedef struct {
    boxheader header;
    monotext text;
} boxtext;


extern boxheader box_null;

inline bool bis_ui(box b)     {return 0 != (b.imm&1);}
inline bool bis_ptr(box b)    {return 0 == (b.imm&1);}
inline uint64_t   bto_ui(box b)     {assert(bis_ui(b)); return b.imm;}
inline boxheader* bto_ptr(box b)    {assert(bis_ptr(b)); return reinterpret_cast<boxheader*>(b.ptr);}
inline boxnode*   bto_node(box b)   {assert(bis_ptr(b)); assert(bto_ptr(b)->type < BNTYPE_NULLER); return reinterpret_cast<boxnode*>(b.ptr);}
inline boxchar*   bto_pchar(box b)  {assert(bis_ptr(b)); assert(bto_ptr(b)->type >= 0); return reinterpret_cast<boxchar*>(b.ptr);}
inline boxtext*   bto_text(box b)   {assert(bis_ptr(b)); assert(bto_ptr(b)->type < 0); return reinterpret_cast<boxtext*>(b.ptr);}
//inline boxplot2d* bto_plot2(box b)  {return reinterpret_cast<boxplot2d*>(b);}
//inline boxplot3d* bto_plot3(box b)  {return reinterpret_cast<boxplot3d*>(b);}

inline box bfrom_imm(uint64_t i) {box b; b.imm = i; return b;}
inline box bfrom_ptr(boxheader* p) {box b; b.ptr = reinterpret_cast<boxheader *>(p); return b;}
inline box bfrom_node(boxnode* p) {box b; b.ptr = reinterpret_cast<boxheader *>(p); return b;}

//inline void bset(box &a, box b) {a.imm = b.imm;} // should copy everything :-)

inline bool bare_same(box b, box c) {return bis_ui(b) == bis_ui(c) && (bis_ui(b) ? b.imm == c.imm : b.ptr == c.ptr);}
inline bool bis_special(box b){return bis_ptr(b) && bto_ptr(b)->type > BNTYPE_NULLER && bto_ptr(b)->type < 0;}
inline bool bis_node(box b)   {return bis_ptr(b) && bto_ptr(b)->type < BNTYPE_NULLER;}
inline bool bis_char(box b)                     {return bis_ui(b) || bto_ptr(b)->type >= 0;}
inline bool bis_char(box b, char16_t c1)        {return bis_ui(b) ? (char16_t((bto_ui(b)>>32) & 65535) == c1) 
                                                                  : (char16_t((bto_ptr(b)->type) & 65535) == c1);}
inline bool bis_char(box b, char16_t c1, char16_t c2) {return bis_ui(b) ? ((char16_t((bto_ui(b)>>32) & 65535) == c1) || (char16_t((bto_ui(b)>>32) & 65535) == c2))
                                                                        : ((char16_t((bto_ptr(b)->type) & 65535) == c1) || (char16_t((bto_ptr(b)->type) & 65535) == c2));}
inline bool bptr_isleaf(box b)  {return bto_ptr(b)->type >= BNTYPE_NULLER;}
inline bool bptr_isnode(box b)  {return bto_ptr(b)->type < BNTYPE_NULLER;}
inline int32_t btype(box b)       {return bis_ui(b) ? bto_ui(b)>>32 : bto_ptr(b)->type;}
inline int32_t bui_sizex(box b)   {return (bto_ui(b)>>22)&0x03FF;} // 10 bits
inline int32_t bui_sizey(box b)   {return (bto_ui(b)>> 1)&0x07FF;} // 11 bits
inline int32_t bui_centery(box b) {return (bto_ui(b)>>12)&0x03FF;} // 10 bits
inline int32_t bui_type(box b)    {return (bto_ui(b)>>32);}
inline int32_t bui_lextype(box b) {return (bto_ui(b)>>48)&255;}
inline int32_t bui_symtype(box b) {return (bto_ui(b)>>56)&255;}
inline box bui_addlextype(box b, uint64_t t)
{
    box c;
    c.imm = (bto_ui(b) & 0x0FFFFFFFFFFFFULL) + (t << 48);
    return c;
}
inline box bui_addsymtype(box b, uint64_t t)
{
    box c;
    c.imm = (bto_ui(b) & 0x0FFFFFFFFFFFFFFULL) + (t << 56);
    return c;
}
inline box bchar_addsymtype(box b, uint64_t t)
{
    assert(bis_char(b));
    if (bis_ui(b))
    {
        return bui_addsymtype(b, t);
    }
    else
    {
        bto_ptr(b)->type = (bto_ptr(b)->type & 0x0FFFFFFULL) + (t << 24);
        return b;
    }
}
inline int bchar_type(box b)  {assert(btype(b) >= 0); return btype(b);}
inline int bchar_lextype(box b) {assert(btype(b) >= 0); return (btype(b) >> 16)&255;}
inline int bchar_symtype(box b) {assert(btype(b) >= 0); return (btype(b) >> 24)&255;}
inline int bptr_type(box b)     {return bto_ptr(b)->type;}
inline int bptr_sizex(box b)    {return bto_ptr(b)->sizex;}
inline int bptr_sizey(box b)    {return bto_ptr(b)->sizey;}
inline int bptr_centery(box b)  {return bto_ptr(b)->centery;}

inline int bnode_type(box b)    {return bto_node(b)->header.type;}
inline int bnode_sizex(box b)   {return bto_node(b)->header.sizex;}
inline int bnode_sizey(box b)   {return bto_node(b)->header.sizey;}
inline int bnode_centery(box b) {return bto_node(b)->header.centery;}
inline int bnode_extra0(box b)  {return bto_node(b)->extra0;}
inline int bnode_extra1(box b)  {return bto_node(b)->extra1;}
inline int bnode_len(box b)     {return bto_node(b)->len;}
inline int bnode_alloc(box b)   {return bto_node(b)->alloc;}
inline box bnode_child(box b, int i){return bto_node(b)->array[i].child;}
inline int bnode_offx(box b, int i) {return bto_node(b)->array[i].offx;}
inline int bnode_offy(box b, int i) {return bto_node(b)->array[i].offy;}

inline int boxnode_type(boxnode* b)    {return b->header.type;}
inline int boxnode_sizex(boxnode* b)   {return b->header.sizex;}
inline int boxnode_sizey(boxnode* b)   {return b->header.sizey;}
inline int boxnode_centery(boxnode* b) {return b->header.centery;}
inline er  boxnode_expr(boxnode* b)  {return b->expr;}
inline int boxnode_extra0(boxnode* b)  {return b->extra0;}
inline int boxnode_extra1(boxnode* b)  {return b->extra1;}
inline int boxnode_len(boxnode* b)     {return b->len;}
inline int boxnode_alloc(boxnode* b)   {return b->alloc;}
inline box boxnode_child(boxnode* b, int i){return b->array[i].child;}
inline int boxnode_offx(boxnode* b, int i) {return b->array[i].offx;}
inline int boxnode_offy(boxnode* b, int i) {return b->array[i].offy;}


int boxrow_child_sizey(boxnode * r, int i);
int boxrow_child_centery(boxnode * r, int i);

inline void bget_header(int32_t&type, int32_t&sizex, int32_t&sizey, int32_t&centery, box b)
{
    if (bis_ptr(b))
    {
        type = bptr_type(b);
        sizex = bptr_sizex(b);
        sizey = bptr_sizey(b);
        centery = bptr_centery(b);
    }
    else
    {
        type = bui_type(b);
        sizex = bui_sizex(b);
        sizey = bui_sizey(b);
        centery = bui_centery(b);
    }
}

void box_invalidate_us(box us);
void box_invalidate_all(box us);

void boxplot3d_render(box us);
void boxplot2d_render(box us);

void boxnode_print(box cursorbox, box b, int depth);


box boxchar_set_sizes(box c, int32_t sizex, int32_t sizey, int32_t centery);
box boxchar_create(int32_t type);
box boxchar_create(int32_t type, int32_t sizex);
box boxchar_createbig(int32_t type, int32_t sizex, int32_t sizey, int32_t centery);
boxnode * boxnode_create(int type, int alloc);
box boxnode_make(int type, box b0);
box boxnode_make(int type, box b0, box b1);
box boxnode_make(int type, box b0, box b1, box b2);
box boxnode_copy(box b);
box boxnode_copyrange(boxnode * org, int start, int end);

void box_delete(box b);
void boxptr_delete(boxheader * b);
void box_node_delete(boxnode * b);

void boxnode_append(boxnode * parent, box child);
void boxnode_append_char(boxnode * parent, int c);
void boxnode_append_cstr(boxnode * parent, const char * cs);
void boxnode_append_boxstr(boxnode * Parent, const char16_t * cs);
void boxnode_append_string(boxnode * parent, const std::string& s, bool esc);
box boxnode_make_from_cstr(const char* s);
box boxnode_make_from_string(const std::string& s);

void boxnode_print(box cursorbox, box b, int depth);
box  boxnode_removerange(boxnode * org, int start, int end);
void boxnode_splice(boxnode * a, int idx, boxnode * b);
void boxnode_splice_row(boxnode * a, int idx, boxnode * b);
box  boxnode_split_row(boxnode * org, int idx);
box  boxnode_splitrange_row(boxnode * org, int start, int end);
void boxnode_join_tworows(boxnode * a, boxnode * b);
void boxnode_append_row(boxnode * a, boxnode * b);
void boxnode_deleterange(box org, int start, int end);
box  boxnode_replace(boxnode * parent, int idx, box newchild);
box  boxnode_remove(boxnode * org, int idx);
void boxnode_insert(boxnode * parent, box child, int idx);


inline box boxnode_copyrange(box org, int start, int end) {
    return boxnode_copyrange(bto_node(org), start, end);}


inline void boxnode_join_tworows(box a, box b) {
     return boxnode_join_tworows(bto_node(a), bto_node(b));}

inline void boxnode_append(box parent, box child) {
     return boxnode_append(bto_node(parent), child);}
inline box  boxnode_replace(box parent, int idx, box newchild) {
     return boxnode_replace(bto_node(parent), idx, newchild);}
inline box  boxnode_remove(box org, int idx) {
     return boxnode_remove(bto_node(org), idx);}
inline void box_node_delete(box b) {
     return box_node_delete(bto_node(b));}
inline box  boxnode_removerange(box org, int start, int end) {
     return boxnode_removerange(bto_node(org), start, end);}
inline box  boxnode_split_row(box org, int idx) {
     return boxnode_split_row(bto_node(org), idx);}
inline void boxnode_insert(box parent, box child, int idx) {
     return boxnode_insert(bto_node(parent), child, idx);}
inline box  boxnode_splitrange_row(box org, int start, int end) {
     return boxnode_splitrange_row(bto_node(org), start, end);}
inline void boxnode_splice(box a, int idx, box b) {
     return boxnode_splice(bto_node(a), idx, bto_node(b));}
inline void boxnode_splice_row(box a, int idx, box b) {
     return boxnode_splice_row(bto_node(a), idx, bto_node(b));}


box boxplot3d_create();


box boxtext_create();
void boxtext_insert_tab(boxtext * us);
void boxtext_insert_char(boxtext * us, char16_t c);
void boxtext_insert_newline(boxtext * us);
void boxtext_insert_string(boxtext * us, const unsigned char * s, int sn);
int boxtext_deletekey(boxtext * us);
int boxtext_backspace(boxtext * us);
int boxtext_left(boxtext * us);
int boxtext_right(boxtext * us);
int boxtext_key_up(boxtext * us);
int boxtext_key_down(boxtext * us);
void boxtext_end(boxtext * us);
void boxtext_start(boxtext * us);
void boxtext_key_end(boxtext * us);
void boxtext_key_home(boxtext * us);

inline void boxtext_insert_tab(box us) {return boxtext_insert_tab(bto_text(us));}
inline void boxtext_insert_char(box us, char16_t c) {return boxtext_insert_char(bto_text(us), c);}
inline void boxtext_insert_newline(box us) {return boxtext_insert_newline(bto_text(us));}
inline void boxtext_insert_string(box us, const unsigned char * s, int sn) {return boxtext_insert_string(bto_text(us), s, sn);}
inline int boxtext_deletekey(box us) {return boxtext_deletekey(bto_text(us));}
inline int boxtext_backspace(box us) {return boxtext_backspace(bto_text(us));}
inline int boxtext_left(box us) {return boxtext_left(bto_text(us));}
inline int boxtext_right(box us) {return boxtext_right(bto_text(us));}
inline int boxtext_key_up(box us) {return boxtext_key_up(bto_text(us));}
inline int boxtext_key_down(box us) {return boxtext_key_down(bto_text(us));}
inline void boxtext_end(box us) {return boxtext_end(bto_text(us));}
inline void boxtext_start(box us) {return boxtext_start(bto_text(us));}
inline void boxtext_key_end(box us) {return boxtext_key_end(bto_text(us));}
inline void boxtext_key_home(box us) {return boxtext_key_home(bto_text(us));}


int escape_seq_to_action(const char * s);

