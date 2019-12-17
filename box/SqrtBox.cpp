#include <stack>
#include "boxes.h"
#include "ex_parse.h"
#include "notebook.h"
#include "box_convert.h"

void sqrtbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) sqrt: size (%d,%d:%d)\n", offx, offy, sizex, sizey, centery);

    inside.cbox->print(indent + 1, inside.offx, inside.offy);
}

visitRet sqrtbox::visit(visitArg m)
{
    inside.cbox->visit(m);
    return visitret_OK;
}

boxbase * sqrtbox::copy()
{
    rowbox * newinside = dynamic_cast<rowbox*>(inside.cbox->copy());
    sqrtbox * r = new sqrtbox(newinside);
    r->inside.offx = inside.offx;
    r->inside.offy = inside.offy;
    return r;    
}

void sqrtbox::key_copy(boxbase*&b)
{
    inside.cbox->key_copy(b);
}

void sqrtbox::key_paste(boxbase*&b)
{
    inside.cbox->key_paste(b);
}

void sqrtbox::insert_char(int32_t c)
{
    inside.cbox->insert_char(c);
    return;
}

moveRet sqrtbox::move(boxbase*&b, moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        case movearg_ShiftLeft:
        case movearg_Right:
        case movearg_ShiftRight:
        {
            r = inside.cbox->move(b, m);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_Last:
        case movearg_First:
        {
            r = inside.cbox->move(b, m);
            assert(r == moveret_OK);
            return r;
        }
        case movearg_Home:
        case movearg_End:
        {
            r = inside.cbox->move(b, m);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_Switch:
        {
            return moveret_OK;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

insertRet sqrtbox::insert(boxbase*&b, insertArg m)
{
    return inside.cbox->insert(b, m);
}

removeRet sqrtbox::remove(boxbase*&b, removeArg m)
{
    assert(b == nullptr);
    removeRet r;

    if (inside.cbox->is_selected_placeholder())
    {
        b = nullptr;
        return removeret_Replace;
    }
    else
    {
        r = inside.cbox->remove(b, m);
        assert(r == removeret_End || r == removeret_OK);
        if (r == removeret_End)
        {
            b = steal_rowbox(inside.cbox, 0,0);
            return removeret_Replace;
        }
        else
        {
            made_into_placeholder(inside.cbox);
            return removeret_OK;
        }
    }
}

ex sqrtbox::get_ex()
{
    ex t = inside.cbox->get_ex();
    return emake_node(gs.sym_sSqrtBox.copy(), t);
}


void sqrtbox::get_cursor(aftransform * T)
{
    inside.cbox->get_cursor(T);
    T->orig_x += inside.offx;
    T->orig_y += inside.offy;    
    return;
}

void sqrtbox::measure(boxmeasurearg ma)
{
    fs = fontint_to_fontsize(ma.fi);
    inside.cbox->measure(boxmeasurearg(ma.fi, ma.deswidth - ROW_DECREMENT, ma.mflags, ma.level + 1));

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

void sqrtbox::draw_pre(boxdrawarg da)
{
    inside.cbox->draw_pre(boxdrawarg(da, inside.offx, inside.offy));
}

void sqrtbox::draw_main(boxdrawarg da)
{
    inside.cbox->draw_main(boxdrawarg(da, inside.offx, inside.offy));

    double padup = fontsize_size(fs)*SIZE_TO_SQRT_PADUP;
    double paddown = fontsize_size(fs)*SIZE_TO_SQRT_PADDOWN;
    double padleft = fontsize_size(fs)*SIZE_TO_SQRT_PADLEFT;
    double padright = fontsize_size(fs)*SIZE_TO_SQRT_PADRIGHT;
    double lw = fontsize_line_width(fs);
    uint32_t color = (da.dflags & DFLAG_SELECTION) ? da.nb->cSelectionForeground :
                     (da.dflags & DFLAG_SCOLOR)    ? da.nb->pallet1[lextype_opinf] :
                                                     glb_image.fcolor;

    double _globy = da.globy;
    double _globx = da.globx;
    double coords[] = {
        _globx + sizex - padright*0.5 , _globy + padup*0.4375,
        _globx + padleft*0.85         , _globy + padup*0.4375,
        _globx + padleft*0.6          , _globy + sizey - padleft*0.1,
        _globx + padleft*0.25         , _globy + sizey - padleft*0.7,
        _globx + padleft*0            , _globy + sizey - padleft*0.5,
    };
    drawtlines(coords, 4, lw, color, da.T);
}

void sqrtbox::draw_post(boxdrawarg da)
{
    inside.cbox->draw_post(boxdrawarg(da, inside.offx, inside.offy));
}
