#include "timing.h"
#include "font.h"
#include "graphics.h"
#include "notebook.h"
#include "box_lex.h"

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


void rootbox::measure(boxmeasurearg ma)
{
    int32_t accheight = 0;
    int32_t maxwidth = 0;
    for (auto& i : childcells)
    {
        i.cbox->measure(ma);
        i.offx = 0;
        i.offy = accheight;
        maxwidth = std::max(maxwidth, i.cbox->sizex);
        accheight += i.cbox->sizey;
    }

    sizex = maxwidth;
    sizey = accheight;
    centery = accheight/2;
}

void cellbox::measure(boxmeasurearg ma)
{
    uint32_t fs = fontint_to_fontsize(ma.fi);

    int32_t pad_above = 0;
    int32_t pad_left = 50;
    uint32_t dflag = 0;
    uint32_t fint = ma.fi;
    switch (celltype)
    {
        case cellt_INPUT:
            pad_above = 15; pad_left = 150;
            dflag = MFLAG_EXSPACE;
            fint = fontint_to_courier_bold(fontint_smaller(ma.fi,0));
            break;
        case cellt_OUTPUT:
            pad_above = 15; pad_left = 150;
            dflag = MFLAG_EXSPACE;
            fint = fontint_to_courier_reg(fontint_smaller(ma.fi,0));
            break;
        case cellt_PRINT:
            pad_above = 6;  pad_left = 20;
            dflag = MFLAG_EXSPACE;
            fint = fontint_to_courier_reg(fontint_smaller(ma.fi,2));
            break;
        case cellt_MESSAGE:
            pad_above = 8;  pad_left = 70;
            fint = fontint_to_tahoma_reg(fontint_smaller(ma.fi,2));
            break;
        case cellt_TEXT:
            pad_above = 10; pad_left = 35;
            fint = fontint_to_tahoma_reg(fontint_smaller(ma.fi,2));
            break;
        case cellt_BOLDTEXT:
            pad_above = 10; pad_left = 35;
            fint = fontint_to_tahoma_bold(fontint_smaller(ma.fi,2));
            break;
        case cellt_SUBSUBSECTION:
            pad_above = 16; pad_left = 90;
            fint = fontint_to_tahoma_reg(fontint_larger(ma.fi,2));
            break;
        case cellt_SUBSECTION:
            pad_above = 18; pad_left = 60;
            fint = fontint_to_tahoma_reg(fontint_larger(ma.fi,4));
            break;
        case cellt_SECTION:
            pad_above = 20; pad_left = 30;
            fint = fontint_to_times_reg(fontint_larger(ma.fi,6));
            break;
        case cellt_TITLE:
            pad_above = 24; pad_left = 15;
            fint = fontint_to_times_bold(fontint_larger(ma.fi,8));
            break;
    }
    pad_above += pad_above*fontsize_size(fs)/32;
    int32_t pad_below = pad_above;
    pad_left = pad_left*fontsize_size(fs)/32;
    int32_t pad_right = fontsize_size(fs)/32;

//printf("fi: %x  smaller: %x\n",fi,fontint_to_sansreg(fontint_smaller(fi,2)));

    int32_t cellbracket_w = glb_dingbat.get_char_sizex(DINGBAT_CELLGEN, fontsize_size(fs));
    int32_t cellbracket_h = glb_dingbat.get_char_sizey(DINGBAT_CELLGEN, fontsize_size(fs));
/*
std::cout << "NEW cell measure" << std::endl;
std::cout << "pad_above: " << pad_above << std::endl;
std::cout << "pad_below: " << pad_below << std::endl;
std::cout << "pad_left: " << pad_left << std::endl;
std::cout << "pad_right: " << pad_right << std::endl;
std::cout << "cellbracket_w: " << cellbracket_w << std::endl;
std::cout << "cellbracket_h: " << cellbracket_h << std::endl;
*/
    body.cbox->measure(boxmeasurearg(fint, ma.deswidth - pad_left - pad_right - cellbracket_w*(2*0+1)/2, dflag));

    int32_t label_sizey = 0;
    bool label_on_left = false;
    if (label.cbox != nullptr)
    {
        label.cbox->measure(boxmeasurearg(fontint_to_tahoma_reg(fontint_smaller(ma.fi,4)), ma.deswidth - pad_left - cellbracket_w*(2*0+1)/2, 0));

        if (label.cbox->sizex + fontsize_size(fs)/2 >= pad_left)
        {
            label.offx = fontsize_size(fs)/2;
            label.offy = (pad_above+1)/2;
            pad_above = std::max(pad_above, label.offy + label.cbox->sizey);
        }
        else
        {
            label.offx = pad_left - (label.cbox->sizex + fontsize_size(fs)/2);
            label.offy = pad_above + 3;
            label_on_left = true;
        }
        label_sizey = label.offy + label.cbox->sizey;
    }

    do {
        body.offx = pad_left;
        body.offy = pad_above;
        sizex = body.offx + body.cbox->sizex;
        sizey = std::max(label_sizey, body.offy + body.cbox->sizey + pad_below);
        centery = sizey/2;
        pad_above += 1;
        pad_below += 1;
    } while (sizey < cellbracket_h);

    if (label_on_left)
    {
        label.offy = body.offy + body.cbox->child[0].offy + ibox_centery(body.cbox->child[0].cibox)
                              - label.cbox->child[0].offy - ibox_centery(label.cbox->child[0].cibox);
    }
}


void charbox::measure(boxmeasurearg ma)
{
    assert(false);
}

void nullbox::measure(boxmeasurearg ma)
{
    return;
}

void graphics3dbox::measure(boxmeasurearg ma)
{
    fs = fontint_to_fontsize(ma.fi);
    sizex = image.sizex*UNITS_PER_PIXEL + 2*5;
    sizey = image.sizey*UNITS_PER_PIXEL + 2*5;
    centery = sizey/2;
}

void subscriptbox::measure(boxmeasurearg ma)
{
    uint32_t fs = fontint_to_fontsize(ma.fi);
    int32_t default_sizey = fontsize_default_sizey(fs);
    int32_t default_centery = fontsize_default_centery(fs);
    int32_t default_sizex = fontsize_default_sizex(fs);

    sub.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,2), ma.deswidth, ma.mflags));
    sub.offx = 0;
    sizex = sub.cbox->sizex;
    centery = sub.cbox->sizey;
    sizey = centery + default_centery;
    sub.offy = 0;
}

void superscriptbox::measure(boxmeasurearg ma)
{
    uint32_t fs = fontint_to_fontsize(ma.fi);
    int32_t default_sizey = fontsize_default_sizey(fs);
    int32_t default_centery = fontsize_default_centery(fs);
    int32_t default_sizex = fontsize_default_sizex(fs);

    super.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,2), ma.deswidth, ma.mflags));
    super.offx = 0;
    sizex = super.cbox->sizex;
    centery = default_centery;
    sizey = super.cbox->sizey + centery;
    super.offy = centery;
}

void subsuperscriptbox::measure(boxmeasurearg ma)
{
    uint32_t fs = fontint_to_fontsize(ma.fi);
    int32_t default_sizey = fontsize_default_sizey(fs);
    int32_t default_centery = fontsize_default_centery(fs);
    int32_t default_sizex = fontsize_default_sizex(fs);

    sub.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,2), ma.deswidth, ma.mflags));
    super.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,2), ma.deswidth, ma.mflags));

    sub.offx = 0;
    super.offx = 0;
    sizex = std::max(sub.cbox->sizex, super.cbox->sizex);

    sub.offy = super.cbox->sizey;
    super.offy = 0;
    sizey = sub.cbox->sizey + super.cbox->sizey;
    centery = super.cbox->sizey;
}

void fractionbox::measure(boxmeasurearg ma)
{
    fs = fontint_to_fontsize(ma.fi);
    num.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,1), ma.deswidth, ma.mflags));
    den.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,1), ma.deswidth, ma.mflags));

    int32_t frac_over = fontsize_frac_over(fs);
    int32_t frac_under = fontsize_frac_under(fs);
    int32_t frac_center = fontsize_frac_center(fs);

    int32_t extrax = frac_center/3;

    int32_t childmax = std::max(num.cbox->sizex, den.cbox->sizex);
    sizex = extrax + childmax + extrax;

    sizey = num.cbox->sizey + frac_over + frac_under + den.cbox->sizey;
    centery = num.cbox->sizey + frac_center;
    num.offx = extrax + (childmax - num.cbox->sizex)/2;
    den.offx = extrax + (childmax - den.cbox->sizex)/2;
    num.offy = 0;
    den.offy = num.cbox->sizey + frac_over + frac_under;
}

void sqrtbox::measure(boxmeasurearg ma)
{
    fs = fontint_to_fontsize(ma.fi);
    inside.cbox->measure(boxmeasurearg(ma.fi, ma.deswidth - ROW_DECREMENT, ma.mflags));

    int32_t padup = fontsize_size(fs)*SIZE_TO_SQRT_PADUP;
    int32_t paddown = fontsize_size(fs)*SIZE_TO_SQRT_PADDOWN;
    int32_t padleft = fontsize_size(fs)*SIZE_TO_SQRT_PADLEFTEX;
    int32_t padright = fontsize_size(fs)*SIZE_TO_SQRT_PADRIGHT;

    inside.offx = padleft;
    inside.offy = padup;
    sizex = padleft + inside.cbox->sizex + padright;
    sizey = padup + inside.cbox->sizey + paddown;
    centery = padup + inside.cbox->centery;
}

void rotationbox::measure(boxmeasurearg ma)
{
    inside.cbox->measure(boxmeasurearg(ma.fi, ma.deswidth, ma.mflags));
    rotbox_sizexy_offxy(sizex, sizey, inside.offx, inside.offy,
                        angle, inside.cbox->sizex, inside.cbox->sizey);
    centery = sizey/2;
}


void underscriptbox::measure(boxmeasurearg ma)
{
    body.cbox->measure(boxmeasurearg(ma.fi, ma.deswidth, ma.mflags));
    under.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,2), ma.deswidth, ma.mflags));

    sizex = std::max(body.cbox->sizex, under.cbox->sizex);
    sizey = body.cbox->sizey + under.cbox->sizey;
    centery = body.cbox->centery;
    body.offx = (sizex - body.cbox->sizex)/2;
    under.offx = (sizex - under.cbox->sizex)/2;
    body.offy = 0;
    under.offy = body.cbox->sizey;
}

void overscriptbox::measure(boxmeasurearg ma)
{
    body.cbox->measure(boxmeasurearg(ma.fi, ma.deswidth, ma.mflags));
    over.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,2), ma.deswidth, ma.mflags));

    sizex = std::max(body.cbox->sizex, over.cbox->sizex);
    sizey = over.cbox->sizey + body.cbox->sizey;
    centery = over.cbox->sizey + body.cbox->centery;
    over.offx = (sizex - over.cbox->sizex)/2;
    body.offx = (sizex - body.cbox->sizex)/2;
    over.offy = 0;
    body.offy = over.cbox->sizey;
}

void underoverscriptbox::measure(boxmeasurearg ma)
{
    body.cbox->measure(boxmeasurearg(ma.fi, ma.deswidth, ma.mflags));
    under.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,2), ma.deswidth, ma.mflags));
    over.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,2), ma.deswidth, ma.mflags));

    sizex = std::max(std::max(body.cbox->sizex, under.cbox->sizex), over.cbox->sizex);
    sizey = over.cbox->sizey + body.cbox->sizey + under.cbox->sizey;
    centery = over.cbox->sizey + body.cbox->centery;
    over.offx = (sizex - over.cbox->sizex)/2;
    body.offx = (sizex - body.cbox->sizex)/2;
    under.offx = (sizex - under.cbox->sizex)/2;
    over.offy = 0;
    body.offy = over.cbox->sizey;
    under.offy = over.cbox->sizey + body.cbox->sizey;
}

void gridbox::measure(boxmeasurearg ma)
{
    uint32_t fs = fontint_to_fontsize(ma.fi);

    std::vector<int32_t> max_width, acc_width;
    std::vector<int32_t> max_above(array.size(), 0);
    std::vector<int32_t> max_below(array.size(), 0);

    for (size_t j = 0; j < array.size(); j++)
    {
        for (size_t i = 0; i < array[j].size(); i++)
        {
            array[j][i].cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,1), ma.deswidth, ma.mflags));
            while (max_width.size() <= i)
                max_width.push_back(0);
            max_width[i] = std::max(max_width[i], array[j][i].cbox->sizex);
            max_above[j] = std::max(max_above[j], array[j][i].cbox->centery);
            max_below[j] = std::max(max_below[j], array[j][i].cbox->sizey - array[j][i].cbox->centery);
        }
    }

    int32_t accx = (fs&65535)*GRID_EXTRAX1;
    for (int32_t i = 0; i < max_width.size(); i++)
    {
        acc_width.push_back(accx);
        accx += max_width[i] + int32_t((fs&65535)*GRID_EXTRAX2);
    }
    accx += int32_t((fs&65535)*GRID_EXTRAX3) - int32_t((fs&65535)*GRID_EXTRAX2);

    int32_t accy = (fs&65535)*GRID_EXTRAY1;
    for (size_t j = 0; j < array.size(); j++)
    {
        for (size_t i = 0; i < array[j].size(); i++)
        {
            array[j][i].offx = acc_width[i] + (max_width[i] - array[j][i].cbox->sizex)/2;
            array[j][i].offy = accy + (max_above[j] - array[j][i].cbox->centery);
        }
        accy += max_above[j] + max_below[j] + int32_t((fs&65535)*GRID_EXTRAY2);
    }
    accy += int32_t((fs&65535)*GRID_EXTRAY3) - int32_t((fs&65535)*GRID_EXTRAY2);

    sizex = accx;
    sizey = accy;
    centery = sizey/2;
}


void rowbox::measure(boxmeasurearg ma)
{
    int32_t maxabove, maxbelow, accwidth, accheight, maxwidth, maxheight;
//    rasterfont * font = fontint_to_fontp(fi);
//    int default_sizey = font->default_sizey();
//    int default_centery = font->default_centery();
//    int default_sizex = font->default_sizex();

//printf("fi: %d\n",fi);
    fs = fontint_to_fontsize(ma.fi);
//printf("fs: %d\n", fs);
    int32_t default_sizey = fontsize_default_sizey(fs);
    int32_t default_centery = fontsize_default_centery(fs);
    int32_t default_sizex = fontsize_default_sizex(fs);

//printf("default_sizex %d\n",default_sizex);
//printf("default_sizey %d\n",default_sizey);
//printf("default_centery %d\n",default_centery);

    ma.deswidth = std::max(ma.deswidth, 10 + 2*default_sizex);



    if (flags & BNFLAG_MEASURED)
        return;

    flags |= BNFLAG_MEASURED;
    flags &= ~BNFLAG_COLORED;

//printf("boxnode_measure row\n"); boxnode_print(nullptr, box(us), 0);



    std::vector<int32_t> ostack;
    std::vector<int32_t> maxabove_between;
    std::vector<int32_t> maxbelow_between;
    blexer L(child.size() + 1);

//printf("before 1st scan:\n"); boxnode_print(NULL, Us, 0);


    /* first scan - find lexical types, position brackets, and position supers */
    ostack.push_back(-1);
    maxabove_between.push_back(default_centery);
    maxbelow_between.push_back(default_sizey - default_centery);

    for (int32_t i = 0; i < child.size(); i++)
    {
        assert(ostack.size() > 0);
        assert(ostack.size() == maxabove_between.size());
        assert(ostack.size() == maxbelow_between.size());

        if (ibox_is_char(child[i].cibox))
        {
            uint32_t a = ibox_type(child[i].cibox)&65535;
            uint32_t b;
            int32_t j = ostack.back();

            int32_t a_fsizex = fontsize_char_sizex(fs, a);
            int32_t a_fsizey = fontsize_char_sizey(fs, a);
            int32_t a_fcentery = fontsize_char_centery(fs, a);

            maxabove_between.back() = std::max(maxabove_between.back(), a_fcentery);
            maxbelow_between.back() = std::max(maxbelow_between.back(), a_fsizey - a_fcentery);

            /* the guess returned by add_char is the correct lextype for brackets */
            a += 65536*L.add_char(a);

            if (    a == '(' + 65536*lextype_parenth_open
                 || a == '{'+ 65536*lextype_curly_open
                 || a == '['+ 65536*lextype_bracket_open
                 || a == CHAR_LeftDoubleBracket + 65536*lextype_bracket_open)
            {
                ostack.push_back(i);
                maxabove_between.push_back(a_fcentery);
                maxbelow_between.push_back(a_fsizey - a_fcentery);
            }
            else if (j >= 0 && ((    b = '(',
                                     a == ')' + 65536*lextype_parenth_close)
                                 || (b = '{',
                                     a == '}' + 65536*lextype_curly_close)
                                 || (b = '[',
                                     a == ']' + 65536*lextype_bracket_close)
                                 || (b = CHAR_LeftDoubleBracket,
                                     a == CHAR_RightDoubleBracket + 65536*lextype_bracket_close))
                            && (ibox_is_char(child[j].cibox, b)))
            {
                // we have found a matching bracket b at index j
                if (     maxabove_between.back() > default_centery
                      || maxbelow_between.back() > default_sizey - default_centery)
                {
                    int half = std::max(maxabove_between.back()-1, maxbelow_between.back()-1);
                    if (   a == '}' + 65536*lextype_curly_close
                        || a == ']' + 65536*lextype_bracket_close
                        || a == CHAR_RightDoubleBracket + 65536*lextype_bracket_close)
                    {
                        half = (1*half+3*default_centery)/4;
                    }
                    if (2*half > default_sizey)
                    {
                        a_fcentery = half;
                        a_fsizey = 2*half;
                        child[j].cibox = iboxchar_set_sizes(child[j].cibox, a_fsizey, a_fcentery);
                    }
                }
                assert(maxabove_between.size() == maxbelow_between.size());
                assert(maxabove_between.size() > 1);
                ostack.pop_back();
                int32_t above = maxabove_between.back(); maxabove_between.pop_back();
                int32_t below = maxbelow_between.back(); maxbelow_between.pop_back();
                maxabove_between.back() = std::max(maxabove_between.back(), above);
                maxbelow_between.back() = std::max(maxbelow_between.back(), below);
            }
            // set size of a:
            child[i].cibox = iboxchar_set_sizes(child[i].cibox, a_fsizex, a_fsizey, a_fcentery);
        }
        else if (ibox_to_ptr(child[i].cibox)->get_type() == BNTYPE_NULLER)
        {
            L.add_newline();
        }
        else
        {
            boxbase* c = ibox_to_ptr(child[i].cibox);
            c->measure(boxmeasurearg(ma.fi, ma.deswidth - ROW_DECREMENT, ma.mflags));
            maxabove_between.back() = std::max(maxabove_between.back(), c->centery);
            maxbelow_between.back() = std::max(maxbelow_between.back(), c->sizey - c->centery);

            L.add_box(c->get_type());

            if (c->get_type() == BNTYPE_SUPER)
            {
                superscriptbox* s = dynamic_cast<superscriptbox*>(c);
                int32_t prev_centery = i > 0 ? ibox_centery(child[i - 1].cibox) : default_centery;
                int32_t prev_sizey   = i > 0 ? ibox_sizey(child[i - 1].cibox) : default_sizey;
                s->super.offx = 0;
                s->super.offy = std::min(-prev_centery, -s->super.cbox->sizey);
                s->sizex = s->super.cbox->sizex;
                s->sizey = prev_sizey - prev_centery - s->super.offy;
                s->centery = -s->super.offy;
                s->super.offy = 0;
            }
            else if (c->get_type() == BNTYPE_SUB)
            {
                subscriptbox* s = dynamic_cast<subscriptbox*>(c);
                int32_t prev_centery = i > 0 ? ibox_centery(child[i - 1].cibox) : default_centery;
                int32_t prev_sizey   = i > 0 ? ibox_sizey(child[i - 1].cibox) : default_sizey;
                s->sub.offx = 0;
                s->sub.offy = std::max(prev_sizey - s->sub.cbox->sizey, prev_centery);
                s->sizex = s->sub.cbox->sizex;
                s->sizey = s->sub.offy + s->sub.cbox->sizey;
                s->centery = prev_centery;
            }
            else if (c->get_type() == BNTYPE_SUBSUPER)
            {
                subsuperscriptbox* s = dynamic_cast<subsuperscriptbox*>(c);
                int32_t prev_centery = default_centery;
                int32_t prev_sizey = default_sizey;
                int32_t slant = 0;
                if (i > 0)
                {
                    prev_sizey   = ibox_sizey(child[i - 1].cibox);
                    prev_centery = ibox_centery(child[i - 1].cibox);
                    int32_t prev_type = ibox_type(child[i - 1].cibox);
                    /* adjust limits of integral */
                    if (prev_type >= 0 && (prev_type&65535) == CHAR_Integral)
                    {
                        slant = fontsize_size(fs)/3;
                    }
                }
                s->super.offx = slant;
                s->sub.offx = -slant;
                s->super.offy = std::min(-prev_centery, -s->super.cbox->sizey);
                s->sub.offy = std::max(prev_sizey - prev_centery - s->sub.cbox->sizey, 0);
                s->sizex = std::max(s->sub.cbox->sizex, s->super.cbox->sizex);
                s->sizey = s->sub.offy + s->sub.cbox->sizey - s->super.offy;
                s->centery = -s->super.offy;
                s->sub.offy -= s->super.offy;
                s->super.offy = 0;
            }
        }
    }

//printf("before 2nd scan:\n"); boxnode_print(NULL, Us, 0);

    /* second scan - place extra horiz space between appropriate lexical types */
    if (ma.mflags & MFLAG_EXSPACE)
    {
//printf("NEW using MFLAG_EXSPACE\n");

        int32_t us_lextype, prev_lextype = lextype_unknown;
        int32_t us_type, prev_type = 0;
        bool prev_is_expr = false;
        for (int32_t i = 0; i < child.size(); i++)
        {
            us_lextype = L.type[i];
            if (ibox_is_char(child[i].cibox))
            {
                if (ibox_is_imm(child[i].cibox))
                {
                    us_type = iboximm_type(child[i].cibox)&65535;
                    child[i].cibox = iboximm_addlextype(child[i].cibox, us_lextype);
                }
                else
                {
                    us_type = ibox_to_ptr(child[i].cibox)->get_type()&65535;
                    ibox_to_ptr(child[i].cibox)->set_type(us_type + 65536*us_lextype);
                }
            }
            else
            {
                us_type = ibox_to_ptr(child[i].cibox)->get_type();            
            }

            us_lextype = us_lextype & 255;

            int32_t e =   (prev_lextype == lextype_comma)
                       && (us_lextype != lextype_whitespace);

            switch (us_lextype)
            {
                case lextype_unknown:
                    break;

                case lextype_number:
                case lextype_symbol:
                    break;


                case lextype_curly_open:
                case lextype_parenth_open:
                    e |= prev_is_expr;
                    prev_is_expr = false;
                    break;

                case lextype_bracket_open:
                    prev_is_expr = false;
                    break;

                case lextype_expr:
                    if ( prev_is_expr && us_type != BNTYPE_SUB
                                      && us_type != BNTYPE_SUBSUPER
                                      && us_type != BNTYPE_SUPER) {
                        e = 1;
                    }
                    prev_is_expr = true;
                    break;

                case lextype_slot_1st:
                case lextype_blank_1st:
                case lextype_string_1st:
                case lextype_number_1st:
                case lextype_symbol_1st:
                case lextype_pattern_1st:
                    e |= prev_is_expr;
                    prev_is_expr = true;
                    break;

                case lextype_oppost_1st:
                    prev_is_expr = true;
                    break;
                case lextype_oppre_1st:
                    e |= prev_is_expr;
                    prev_is_expr = false;
                    break;
                case lextype_opinf_1st:
                    prev_is_expr = (us_type != '/') && (us_type != '^');
                    e |= (us_type != ';') && prev_is_expr;
                    break;

                case lextype_opinf:
                    if (!prev_is_expr) {
                        L.extrax[i - 1] = 1;
                        prev_is_expr = true;
                    }
                case lextype_oppost:
                case lextype_oppre:
                    break;

                case lextype_curly_close:
                case lextype_parenth_close:
                case lextype_bracket_close:
                    prev_is_expr = true;
                    break;

                case lextype_comma:
                    prev_is_expr = false;
                    break;

                case lextype_message_name:
                case lextype_string:
                case lextype_blank:
                case lextype_slot:
                case lextype_comment:
                case lextype_comment_1st:
                case lextype_whitespace:
                    break;
                default:
                    assert(false);
            }
            if (prev_lextype == lextype_unknown || prev_lextype == lextype_whitespace) {
                e = 0;
            }

            L.extrax[i] = e;

            prev_lextype = us_lextype;
            prev_type = us_type;
        }
    }
    else
    {
        int32_t us_lextype;
        for (int32_t i = 0; i < child.size(); i++)
        {
            us_lextype = L.type[i];
            if (ibox_is_char(child[i].cibox))
            {
                if (ibox_is_imm(child[i].cibox))
                {
                    child[i].cibox = iboximm_addlextype(child[i].cibox, us_lextype);
                }
                else
                {
                    ibox_to_ptr(child[i].cibox)->set_type((ibox_to_ptr(child[i].cibox)->get_type() & 65535) + 65536*us_lextype);
                }
            } else {
                //us_type = bptr_type(child);
            }

            L.extrax[i] = 0;
        }
    }

//printf("NEW before 3rd scan:\n");

    /* third scan - find line breaks and place children into row */
    accheight = 0;
    int32_t accheightsave = accheight;
//std::cout << "accheight: " << accheight << std::endl;
    maxwidth = 1;
    maxabove = default_centery;
    maxbelow = default_sizey - default_centery;
    int32_t a = 0, b = 0;
    int32_t checklinecount=0;
    int32_t accwidth_restart = (ma.mflags & MFLAG_EXSPACE) ? default_sizex : 0;
//printf("processing new row\n");
    do {
//printf("a = %d\n",a);
        checklinecount++;
        // find the delimiters [a,b) of this line
        while (b + 1 < child.size())
        {
            if (ibox_type(child[b].cibox) == BNTYPE_NULLER)
            {
                break;
            }
            b++;
        }

//printf("a = %d  b = %d\n",a,b);

        // process chars in [a,b)
        int32_t child_type, child_sizex, child_sizey, child_centery;
        int32_t c = a;
        int32_t d = a;
        accwidth = 0;
        bool processed_end = false;
        while (d < b)
        {

//printf("looking at [%d, %d)\n",c,d); printf("accwidth %d\n",accwidth); SleepUS(50000);

            child_type = ibox_type(child[d].cibox);
            child_sizex = ibox_sizex(child[d].cibox);
            child_sizey = ibox_sizey(child[d].cibox);
            child_centery = ibox_centery(child[d].cibox);

            child[d].offx = accwidth;

//std::cout << "setting child["<<d<<"].offx = " << accwidth << std::endl;

            //us->array[d].offy = accheight + maxabove - child_centery;

            accwidth += child_sizex + L.extrax[d + 1]*default_sizex;

            if (accwidth > ma.deswidth && c < d)
            {
                int32_t j = d;
                int32_t smallest = L.penalty[j] + (d-j)*(d-j);
                for (int32_t i = d - 1; i > c; i--)
                {
                    if (smallest > L.penalty[i] + (d-i)*(d-i))
                    {
                        smallest = L.penalty[i] + (d-i)*(d-i);
                        j = i;
                    }
                }
                d = j;
                assert(c < d);
                maxabove = default_centery;
                maxbelow = default_sizey - default_centery;
//std::cout << "init maxabove: " << maxabove << std::endl;
//std::cout << "init maxbelow: " << maxbelow << std::endl;

                for (int32_t i = c; i < d; i++)
                {
                    child_sizey = ibox_sizey(child[i].cibox);
                    child_centery = ibox_centery(child[i].cibox);
                    maxabove = std::max(maxabove, child_centery);
                    maxbelow = std::max(maxbelow, child_sizey - child_centery);
//std::cout << "     maxabove: " << maxabove << std::endl;
//std::cout << "     maxbelow: " << maxbelow << std::endl;
                }
                for (int32_t i = c; i < d; i++)
                {
                    child[i].offy = accheight + maxabove - ibox_centery(child[i].cibox);
                }
                accheight += maxabove + maxbelow;
                accheightsave = accheight;
                accheight += default_sizey/6;


                maxwidth = std::max(maxwidth, child[d].offx);

//std::cout << "     child[" << d << "].offx: " << child[d].offx << std::endl;


                processed_end = true;
                if (d >= b)
                {
                    // process ending
                    assert(d == b);
                    child[d].offx = accwidth;
//std::cout << "setting child["<<d<<"].offx = " << accwidth << std::endl;


                    child[d].offy = accheight + maxabove - ibox_centery(child[d].cibox);
                }

                accwidth = accwidth_restart;
                c = d;
            }
            else
            {
                processed_end = false;
                d++;
            }
        }
        assert(d == b);
        if (!processed_end)
        {
                maxabove = default_centery;
                maxbelow = default_sizey - default_centery;
//std::cout << "init maxabove: " << maxabove << std::endl;
//std::cout << "init maxbelow: " << maxbelow << std::endl;

                for (int32_t i = c; i < d; i++)
                {
                    child_sizey = ibox_sizey(child[i].cibox);
                    child_centery = ibox_centery(child[i].cibox);
                    maxabove = std::max(maxabove, child_centery);
                    maxbelow = std::max(maxbelow, child_sizey - child_centery);
//std::cout << "     maxabove: " << maxabove << std::endl;
//std::cout << "     maxbelow: " << maxbelow << std::endl;
                }
                for (int32_t i = c; i < d; i++)
                {
                    child[i].offy = accheight + maxabove - ibox_centery(child[i].cibox);
                }

                // process ending
                child[d].offx = accwidth;
                child[d].offy = accheight + maxabove - ibox_centery(child[d].cibox);

                accheight += maxabove + maxbelow;
//std::cout << "     accheight: " << accheight << std::endl;
//                    accheight += default_sizey/4;
                accheightsave = accheight;
                accheight += default_sizey/6;



                maxwidth = std::max(maxwidth, child[d].offx);

//std::cout << "     child[" << d << "].offx: " << child[d].offx << std::endl;


        }

        // goto next line
        a = ++b;
        if (a < child.size())
        {
            accheightsave = accheight;
            accheight += default_sizey/5;
        }
    } while (a < child.size());
    // process ending
//        child = us->array[us->len-1].child;
//        us->array[us->len-1].offx = accwidth;
//        us->array[us->len-1].offy = accheight + maxabove - child->centery;

//printf("maxwidth: %d  accwidth: %d  maxabove: %d\n",maxwidth,accwidth,maxabove);
//printf("checklinecount: %d\n",checklinecount);

    sizex = maxwidth;
    sizey = accheightsave;
    centery = checklinecount == 1 ? maxabove : accheight/2;

//printf("measure row 3 done\n");
}





void notebook::measure()
{
//    resize(glb_image.sizex, glb_image.sizey);
//    _boxnode_measure(root, 9, 0, glb_image.sizex, 0);
//    if (cursor_needs_fitting)
//        fitcursor();

    myroot->measure(boxmeasurearg(9, glb_image.sizex, 0));
}
