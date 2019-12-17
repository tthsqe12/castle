#include <stack>
#include "boxes.h"
#include "ex_parse.h"
#include "notebook.h"
#include "box_convert.h"

void subsuperscriptbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");

    printf("(%d,%d) subsuperscript: size(%d,%d:%d) cursor(%d)\n",
             offx, offy, sizex, sizey, sizey, cursor);

    sub.cbox->print(indent + 1, sub.offx, sub.offy);
    super.cbox->print(indent + 1, super.offx, super.offy);
}

visitRet subsuperscriptbox::visit(visitArg m)
{
    sub.cbox->visit(m);
    super.cbox->visit(m);
    return visitret_OK;
}

boxbase * subsuperscriptbox::copy()
{
    rowbox * newsub = dynamic_cast<rowbox*>(sub.cbox->copy());
    rowbox * newsuper = dynamic_cast<rowbox*>(super.cbox->copy());
    subsuperscriptbox * r = new subsuperscriptbox(newsub, newsuper, cursor);
    r->sub.offx = sub.offx;
    r->sub.offy = sub.offy;
    r->super.offx = super.offx;
    r->super.offy = super.offy;
    return r;    
}

void subsuperscriptbox::key_copy(boxbase*&b)
{
    if (cursor == 0)
    {
        sub.cbox->key_copy(b);
    }
    else
    {
        assert(cursor == 1);
        super.cbox->key_copy(b);
    }
}

void subsuperscriptbox::key_paste(boxbase*&b)
{
    if (cursor == 0)
    {
        sub.cbox->key_paste(b);
    }
    else
    {
        assert(cursor == 1);
        super.cbox->key_paste(b);
    }
}

void subsuperscriptbox::insert_char(int32_t c)
{
    if (cursor == 0)
    {
        sub.cbox->insert_char(c);
    }
    else
    {
        assert(cursor == 1);
        super.cbox->insert_char(c);
    }
}

moveRet subsuperscriptbox::move(boxbase*&b, moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        {
            if (cursor == 0)
            {
                r = sub.cbox->move(b, movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = super.cbox->move(b, movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = sub.cbox->move(b, movearg_Last);
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
                r = sub.cbox->move(b, movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = super.cbox->move(b, movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Right:
        {
            if (cursor == 1)
            {
                r = super.cbox->move(b, movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 0);
                r = sub.cbox->move(b, movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = super.cbox->move(b, movearg_First);
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
                r = sub.cbox->move(b, movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = super.cbox->move(b, movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Last:
        {
            r = super.cbox->move(b, movearg_Last);
            assert(r == moveret_OK);
            cursor = 1;
            return r;
        }
        case movearg_First:
        {
            r = sub.cbox->move(b, movearg_First);
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
                r = sub.cbox->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = super.cbox->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Switch:
        {
            if (cursor == 0)
            {
                size_t n = super.cbox->child.size();
                super.cbox->cursor_a = 0;
                super.cbox->cursor_b = n - 1;
                cursor = 1;
            }
            else
            {
                assert(cursor == 1);
                size_t n = sub.cbox->child.size();
                sub.cbox->cursor_a = 0;
                sub.cbox->cursor_b = n - 1;
                cursor = 0;
            }
            return moveret_OK;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

insertRet subsuperscriptbox::insert(boxbase*&b, insertArg m)
{
    if (cursor == 0)
    {
        return sub.cbox->insert(b, m);
    }
    else
    {
        assert(cursor == 1);
        return super.cbox->insert(b, m);
    }
}

removeRet subsuperscriptbox::remove(boxbase*&b, removeArg m)
{
    assert(b == nullptr);
    removeRet r;

    if (cursor == 0)
    {
        if (sub.cbox->is_selected_placeholder())
        {
            int32_t n = m == removearg_Right ? 0 : super.cbox->child.size() - 1;
            rowbox * newsuper = steal_rowbox(super.cbox, n,n);
            b = new superscriptbox(newsuper);
            return removeret_Replace;
        }
        else
        {
            r = sub.cbox->remove(b, m);
            assert(r == removeret_End || r == removeret_OK);
            if (made_into_placeholder(sub.cbox))
            {
                return removeret_OK;
            }
            if (r == removeret_End)
            {
                if (m == removearg_Left)
                {
                    return removeret_End;
                }
                else
                {
                    super.cbox->cursor_a = 0;
                    super.cbox->cursor_b = 0;
                    cursor = 1;
                    return removeret_OK;
                }
            }
            else
            {
                return removeret_OK;
            }
        }
    }
    else
    {
        assert(cursor == 1);
        if (super.cbox->is_selected_placeholder())
        {
            int32_t n = m == removearg_Right ? 0 : sub.cbox->child.size() - 1;
            rowbox * newsub = steal_rowbox(sub.cbox, n,n);
            b = new subscriptbox(newsub);
            return removeret_Replace;
        }
        else
        {
            r = super.cbox->remove(b, m);
            assert(r == removeret_End || r == removeret_OK);
            if (made_into_placeholder(super.cbox))
            {
                return removeret_OK;
            }
            if (r == removeret_End)
            {
                if (m == removearg_Right)
                {
                    return removeret_End;
                }
                else
                {
                    sub.cbox->cursor_a = sub.cbox->child.size() - 1;
                    sub.cbox->cursor_b = sub.cbox->child.size() - 1;
                    cursor = 0;
                    return removeret_OK;
                }
            }
            else
            {
                return removeret_OK;
            }
        }
    }
}

ex subsuperscriptbox::get_ex()
{
    uex t1(sub.cbox->get_ex());
    ex t2 = super.cbox->get_ex();
    return emake_node(gs.sym_sSubsuperscriptBox.copy(), t1.release(), t2);
}


void subsuperscriptbox::get_cursor(aftransform * T)
{
    if (cursor == 0)
    {
        sub.cbox->get_cursor(T);
        T->orig_x += sub.offx;
        T->orig_y += sub.offy;
    }
    else
    {
        assert(cursor == 1);
        super.cbox->get_cursor(T);
        T->orig_x += super.offx;
        T->orig_y += super.offy;        
    }
}

void subsuperscriptbox::measure(boxmeasurearg ma)
{
    uint32_t fs = fontint_to_fontsize(ma.fi);
    int32_t default_sizey = fontsize_default_sizey(fs);
    int32_t default_centery = fontsize_default_centery(fs);
    int32_t default_sizex = fontsize_default_sizex(fs);

    sub.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,2), ma.deswidth, ma.mflags, ma.level + 1));
    super.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,2), ma.deswidth, ma.mflags, ma.level + 1));

    sub.offx = 0;
    super.offx = 0;
    sizex = std::max(sub.cbox->sizex, super.cbox->sizex);

    sub.offy = super.cbox->sizey;
    super.offy = 0;
    sizey = sub.cbox->sizey + super.cbox->sizey;
    centery = super.cbox->sizey;
}


void subsuperscriptbox::draw_pre(boxdrawarg da)
{
    if (cursor == 0)
    {
        sub.cbox->draw_pre(boxdrawarg(da, sub.offx, sub.offy));
    }
    else
    {
        assert(cursor == 1);
        super.cbox->draw_pre(boxdrawarg(da, super.offx, super.offy));
    }
}

void subsuperscriptbox::draw_main(boxdrawarg da)
{
    sub.cbox->draw_main(boxdrawarg(da, sub.offx, sub.offy, cursor == 0 ? 0 : DFLAG_IGNORESEL));
    super.cbox->draw_main(boxdrawarg(da, super.offx, super.offy, cursor == 1 ? 0 : DFLAG_IGNORESEL));
}

void subsuperscriptbox::draw_post(boxdrawarg da)
{
    if (cursor == 0)
    {
        sub.cbox->draw_post(boxdrawarg(da, sub.offx, sub.offy));
    }
    else
    {
        assert(cursor == 1);
        super.cbox->draw_post(boxdrawarg(da, super.offx, super.offy));
    }
}
