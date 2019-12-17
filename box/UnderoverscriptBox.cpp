#include <stack>
#include "boxes.h"
#include "ex_parse.h"
#include "notebook.h"
#include "box_convert.h"

void underoverscriptbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) underoverscript: size (%d,%d:%d)\n", offx, offy, sizex, sizey, centery);

    body.cbox->print(indent + 1, body.offx, body.offy);
    under.cbox->print(indent + 1, under.offx, under.offy);
    over.cbox->print(indent + 1, over.offx, over.offy);
}

visitRet underoverscriptbox::visit(visitArg m)
{
    body.cbox->visit(m);
    under.cbox->visit(m);
    over.cbox->visit(m);
    return visitret_OK;
}

boxbase * underoverscriptbox::copy()
{
    rowbox * newbody = dynamic_cast<rowbox*>(body.cbox->copy());
    rowbox * newunder = dynamic_cast<rowbox*>(under.cbox->copy());
    rowbox * newover = dynamic_cast<rowbox*>(over.cbox->copy());
    underoverscriptbox * r = new underoverscriptbox(newbody, newunder, newover, cursor);
    r->body.offx = body.offx;
    r->body.offy = body.offy;
    r->under.offx = under.offx;
    r->under.offy = under.offy;
    r->over.offx = over.offx;
    r->over.offy = over.offy;
    return r;    
}

void underoverscriptbox::key_copy(boxbase*&b)
{
    if (cursor == 0)
    {
        body.cbox->key_copy(b);
    }
    else if (cursor == 1)
    {
        under.cbox->key_copy(b);
    }
    else
    {
        assert(cursor == 2);
        over.cbox->key_copy(b);
    }
}

void underoverscriptbox::key_paste(boxbase*&b)
{
    if (cursor == 0)
    {
        body.cbox->key_paste(b);
    }
    else if (cursor == 1)
    {
        under.cbox->key_paste(b);
    }
    else
    {
        assert(cursor == 2);
        over.cbox->key_paste(b);
    }
}

void underoverscriptbox::insert_char(int32_t c)
{
    if (cursor == 0)
    {
        body.cbox->insert_char(c);
    }
    else if (cursor == 1)
    {
        under.cbox->insert_char(c);
    }
    else
    {
        assert(cursor == 2);
        over.cbox->insert_char(c);
    }
}

moveRet underoverscriptbox::move(boxbase*&b, moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(b, movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else if (cursor == 1)
            {
                r = under.cbox->move(b, movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = body.cbox->move(b, movearg_Last);
                    assert(r == moveret_OK);
                    cursor = 0;
                }
                return moveret_OK;
            }
            else
            {
                assert(cursor == 2);
                r = over.cbox->move(b, movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = under.cbox->move(b, movearg_Last);
                    assert(r == moveret_OK);
                    cursor = 1;
                }
                return moveret_OK;
            }
        }
        case movearg_ShiftLeft:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(b, movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else if (cursor == 1)
            {
                r = under.cbox->move(b, movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 2);
                r = over.cbox->move(b, movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Right:
        {
            if (cursor == 2)
            {
                r = over.cbox->move(b, movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else if (cursor == 1)
            {
                r = under.cbox->move(b, movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = over.cbox->move(b, movearg_First);
                    assert(r == moveret_OK);
                    cursor = 2;
                }
                return moveret_OK;
            }
            else
            {
                assert(cursor == 0);
                r = body.cbox->move(b, movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = under.cbox->move(b, movearg_First);
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
                r = body.cbox->move(b, movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else if (cursor == 1)
            {
                r = under.cbox->move(b, movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 2);
                r = over.cbox->move(b, movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Last:
        {
            r = over.cbox->move(b, movearg_Last);
            assert(r == moveret_OK);
            cursor = 2;
            return r;
        }
        case movearg_First:
        {
            r = body.cbox->move(b, movearg_First);
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
                r = body.cbox->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else if (cursor == 1)
            {
                r = under.cbox->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 2);
                r = over.cbox->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Switch:
        {
            if (cursor == 1)
            {
                size_t n = over.cbox->child.size();
                over.cbox->cursor_a = 0;
                over.cbox->cursor_b = n - 1;
                cursor = 2;
            }
            else if (cursor == 2)
            {
                size_t n = under.cbox->child.size();
                under.cbox->cursor_a = 0;
                under.cbox->cursor_b = n - 1;
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

insertRet underoverscriptbox::insert(boxbase*&b, insertArg m)
{
    if (cursor == 0)
    {
        return body.cbox->insert(b, m);
    }
    else if (cursor == 1)
    {
        return under.cbox->insert(b, m);
    }
    else
    {
        assert(cursor == 2);
        return over.cbox->insert(b, m);
    }
}

removeRet underoverscriptbox::remove(boxbase*&b, removeArg m)
{
    assert(b == nullptr);
    removeRet r;

    if (cursor == 0)
    {
        if (body.cbox->is_selected_placeholder())
        {
            b = nullptr;
            return removeret_Replace;
        }
        else
        {
            r = body.cbox->remove(b, m);
            assert(r == removeret_End || r == removeret_OK);
            if (made_into_placeholder(body.cbox))
            {
                return removeret_OK;
            }
            return r;
        }
    }
    else if (cursor == 1)
    {
        if (under.cbox->is_selected_placeholder())
        {
            rowbox * newover = steal_rowbox(over.cbox, 0,0);
            int32_t n = body.cbox->child.size() - 1;
            rowbox * newbody = steal_rowbox(body.cbox, n,n);
            b = new overscriptbox(newbody, newover, m == removearg_Left ? 0 : 1);
            return removeret_Replace;
        }
        else
        {
            r = under.cbox->remove(b, m);
            assert(r == removeret_End || r == removeret_OK);
            if (made_into_placeholder(under.cbox))
            {
                return removeret_OK;
            }
            return r;
        }
    }
    else
    {
        assert(cursor == 2);
        if (over.cbox->is_selected_placeholder())
        {
            int32_t n = m == removearg_Right ? 0 : under.cbox->child.size() - 1;
            rowbox * newunder = steal_rowbox(under.cbox, n,n);
            rowbox * newbody = steal_rowbox(body.cbox, 0,0);
            b = new underscriptbox(newbody, newunder, 1);
            return removeret_Replace;
        }
        else
        {
            r = over.cbox->remove(b, m);
            assert(r == removeret_End || r == removeret_OK);
            if (made_into_placeholder(over.cbox))
            {
                return removeret_OK;
            }
            return r;
        }
    }
}

ex underoverscriptbox::get_ex()
{
    uex t1(body.cbox->get_ex());
    uex t2(under.cbox->get_ex());
    ex t3 = over.cbox->get_ex();
    return emake_node(gs.sym_sUnderoverscriptBox.copy(), t1.release(), t2.release(), t3);
}



void underoverscriptbox::get_cursor(aftransform * T)
{
    if (cursor == 0)
    {
        body.cbox->get_cursor(T);
        T->orig_x += body.offx;
        T->orig_y += body.offy;
    }
    else if (cursor == 1)
    {
        under.cbox->get_cursor(T);
        T->orig_x += under.offx;
        T->orig_y += under.offy;        
    }
    else
    {
        assert(cursor == 2);
        over.cbox->get_cursor(T);
        T->orig_x += over.offx;
        T->orig_y += over.offy;        
    }
}



void underoverscriptbox::measure(boxmeasurearg ma)
{
    body.cbox->measure(boxmeasurearg(ma));
    under.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,2), ma.deswidth, ma.mflags, ma.level + 1));
    over.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,2), ma.deswidth, ma.mflags, ma.level + 1));

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



void underoverscriptbox::draw_pre(boxdrawarg da)
{
    if (cursor == 0)
    {
        body.cbox->draw_pre(boxdrawarg(da, body.offx, body.offy));
    }
    else if (cursor == 1)
    {
        under.cbox->draw_pre(boxdrawarg(da, under.offx, under.offy));
    }
    else
    {
        assert(cursor == 2);
        over.cbox->draw_pre(boxdrawarg(da, over.offx, over.offy));
    }
}

void underoverscriptbox::draw_main(boxdrawarg da)
{
    body.cbox->draw_main(boxdrawarg(da, body.offx, body.offy, cursor == 0 ? 0 : DFLAG_IGNORESEL));
    under.cbox->draw_main(boxdrawarg(da, under.offx, under.offy, cursor == 1 ? 0 : DFLAG_IGNORESEL));
    over.cbox->draw_main(boxdrawarg(da, over.offx, over.offy, cursor == 2 ? 0 : DFLAG_IGNORESEL));
}

void underoverscriptbox::draw_post(boxdrawarg da)
{
    if (cursor == 0)
    {
        body.cbox->draw_post(boxdrawarg(da, body.offx, body.offy));
    }
    else if (cursor == 1)
    {
        under.cbox->draw_post(boxdrawarg(da, under.offx, under.offy));
    }
    else
    {
        assert(cursor == 2);
        over.cbox->draw_post(boxdrawarg(da, over.offx, over.offy));
    }
}
