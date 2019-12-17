#include <stack>
#include "boxes.h"
#include "ex_parse.h"
#include "notebook.h"
#include "box_convert.h"

void fractionbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) fraction: size(%d,%d:%d) cursor(%d)\n", offx, offy, sizex, sizey, sizey, cursor);

    num.cbox->print(indent + 1, num.offx, num.offy);
    den.cbox->print(indent + 1, den.offx, den.offy);
}

visitRet fractionbox::visit(visitArg m)
{
    num.cbox->visit(m);
    den.cbox->visit(m);
    return visitret_OK;
}

boxbase * fractionbox::copy()
{
    rowbox * newnum = dynamic_cast<rowbox*>(num.cbox->copy());
    rowbox * newden = dynamic_cast<rowbox*>(den.cbox->copy());
    fractionbox * r = new fractionbox(newnum, newden, cursor);
    r->num.offx = num.offx;
    r->num.offy = num.offy;
    r->den.offx = den.offx;
    r->den.offy = den.offy;
    r->fs = fs;
    return r;    
}

void fractionbox::key_copy(boxbase*&b)
{
    if (cursor == 0)
    {
        num.cbox->key_copy(b);
    }
    else
    {
        assert(cursor == 1);
        den.cbox->key_copy(b);
    }
}

void fractionbox::key_paste(boxbase*&b)
{
    if (cursor == 0)
    {
        num.cbox->key_paste(b);
    }
    else
    {
        assert(cursor == 1);
        den.cbox->key_paste(b);
    }
}

void fractionbox::insert_char(int32_t c)
{
    switch (cursor)
    {
        case 0:
            num.cbox->insert_char(c);
            return;
        case 1:
            den.cbox->insert_char(c);
            return;
        default:
            assert(false);
    }
}

moveRet fractionbox::move(boxbase*&b, moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        {
            if (cursor == 0)
            {
                r = num.cbox->move(b, movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = den.cbox->move(b, movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = num.cbox->move(b, movearg_Last);
                    assert(r == moveret_OK);
                    cursor = 0;
                }
                return moveret_OK;
            }
        }
        case movearg_ShiftLeft:
        {
            if (cursor == 0)
            {
                r = num.cbox->move(b, movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = den.cbox->move(b, movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Right:
        {
            if (cursor == 1)
            {
                r = den.cbox->move(b, movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 0);
                r = num.cbox->move(b, movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = den.cbox->move(b, movearg_First);
                    assert(r == moveret_OK);
                    cursor = 1;
                }
                return moveret_OK;
            }
        }
        case movearg_ShiftRight:
        {
            if (cursor == 0)
            {
                r = num.cbox->move(b, movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = den.cbox->move(b, movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Last:
        {
            r = den.cbox->move(b, movearg_Last);
            assert(r == moveret_OK);
            cursor = 1;
            return r;
        }
        case movearg_First:
        {
            r = num.cbox->move(b, movearg_First);
            assert(r == moveret_OK);
            cursor = 0;
            return r;
        }
        case movearg_Home:
        case movearg_End:
        case movearg_Tab:
        {
            if (cursor == 0)
            {
                r = num.cbox->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = den.cbox->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
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

insertRet fractionbox::insert(boxbase*&b, insertArg m)
{
    if (cursor == 0)
    {
        return num.cbox->insert(b, m);
    }
    else
    {
        assert(cursor == 1);
        return den.cbox->insert(b, m);
    }
}

removeRet fractionbox::remove(boxbase*&b, removeArg m)
{
    assert(b == nullptr);
    removeRet r;

    if (cursor == 0)
    {
        if (num.cbox->is_selected_placeholder())
        {
            b = nullptr;
            return removeret_Replace;
        }
        else
        {
            r = num.cbox->remove(b, m);
            assert(r == removeret_End || r == removeret_OK);
            if (r == removeret_End)
            {
                b = steal_rowbox(num.cbox, 0,0);
                return removeret_Replace;
            }
            else
            {
                made_into_placeholder(num.cbox);
                return removeret_OK;
            }
        }
    }
    else
    {
        assert(cursor == 1);
        if (den.cbox->is_selected_placeholder())
        {
            b = steal_rowbox(num.cbox, 0,0);
            return removeret_Replace;
        }
        else
        {
            r = den.cbox->remove(b, m);
            assert(r == removeret_End || r == removeret_OK);
            if (r == removeret_End)
            {
                b = steal_rowbox(num.cbox, 0,0);
                return removeret_Replace;
            }
            else
            {
                made_into_placeholder(den.cbox);
                return removeret_OK;
            }
        }
    }
}

ex fractionbox::get_ex()
{
    uex t1(num.cbox->get_ex());
    ex t2 = den.cbox->get_ex();
    return emake_node(gs.sym_sFractionBox.copy(), t1.release(), t2);
}


void fractionbox::get_cursor(aftransform * T)
{
    if (cursor == 0)
    {
        num.cbox->get_cursor(T);
        T->orig_x += num.offx;
        T->orig_y += num.offy;
    }
    else
    {
        assert(cursor == 1);
        den.cbox->get_cursor(T);
        T->orig_x += den.offx;
        T->orig_y += den.offy;        
    }
}

void fractionbox::measure(boxmeasurearg ma)
{
    fs = fontint_to_fontsize(ma.fi);
    num.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,1), ma.deswidth, ma.mflags, ma.level + 1));
    den.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,1), ma.deswidth, ma.mflags, ma.level + 1));

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


void fractionbox::draw_pre(boxdrawarg da)
{
    switch (cursor)
    {
        case 0:
            num.cbox->draw_pre(boxdrawarg(da, num.offx, num.offy));
            return;
        case 1:
            den.cbox->draw_pre(boxdrawarg(da, den.offx, den.offy));
            return;
        default:
            assert(false);
    }
}

void fractionbox::draw_main(boxdrawarg da)
{
//std::cout << "fractionbox::draw_main " << da.tostring() << std::endl;

    num.cbox->draw_main(boxdrawarg(da, num.offx, num.offy, cursor == 0 ? 0 : DFLAG_IGNORESEL));
    den.cbox->draw_main(boxdrawarg(da, den.offx, den.offy, cursor == 1 ? 0 : DFLAG_IGNORESEL));

    uint32_t color = (da.dflags & DFLAG_SELECTION) ? da.nb->cSelectionForeground :
                     (da.dflags & DFLAG_SCOLOR)    ? da.nb->pallet1[lextype_opinf] :
                                                     glb_image.fcolor;
    int32_t y = da.globy + num.cbox->sizey + fontsize_frac_over(fs);
    drawtline(da.globx + 1, y, da.globx + sizex - 1, y, fontsize_line_width(fs), color, da.T);
}

void fractionbox::draw_post(boxdrawarg da)
{
    switch (cursor)
    {
        case 0:
            num.cbox->draw_post(boxdrawarg(da, num.offx, num.offy));
            return;
        case 1:
            den.cbox->draw_post(boxdrawarg(da, den.offx, den.offy));
            return;
        default:
            assert(false);
    }
}
