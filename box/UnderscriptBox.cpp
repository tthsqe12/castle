#include <stack>
#include "boxes.h"
#include "ex_parse.h"
#include "notebook.h"
#include "box_convert.h"

void underscriptbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) underscript: size (%d,%d:%d)\n", offx, offy, sizex, sizey, centery);

    body.cbox->print(indent + 1, body.offx, body.offy);
    under.cbox->print(indent + 1, under.offx, under.offy);
}


boxbase * underscriptbox::copy()
{
    rowbox * newbody = dynamic_cast<rowbox*>(body.cbox->copy());
    rowbox * newunder = dynamic_cast<rowbox*>(under.cbox->copy());
    underscriptbox * r = new underscriptbox(newbody, newunder, cursor);
    r->body.offx = body.offx;
    r->body.offy = body.offy;
    r->under.offx = under.offx;
    r->under.offy = under.offy;
    return r;    
}

void underscriptbox::key_copy(boxbase*&b)
{
    if (cursor == 0)
    {
        body.cbox->key_copy(b);
    }
    else
    {
        assert(cursor == 1);
        under.cbox->key_copy(b);
    }
}

void underscriptbox::key_paste(boxbase*&b)
{
    if (cursor == 0)
    {
        body.cbox->key_paste(b);
    }
    else
    {
        assert(cursor == 1);
        under.cbox->key_paste(b);
    }
}

void underscriptbox::insert_char(int32_t c)
{
    if (cursor == 0)
    {
        body.cbox->insert_char(c);
    }
    else
    {
        assert(cursor == 1);
        under.cbox->insert_char(c);
    }
}

visitRet underscriptbox::visit(visitArg m)
{
    body.cbox->visit(m);
    under.cbox->visit(m);
    return visitret_OK;
}

moveRet underscriptbox::move(boxbase*&b, moveArg m)
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
            else
            {
                assert(cursor == 1);
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
        }
        case movearg_ShiftLeft:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(b, movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = under.cbox->move(b, movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Right:
        {
            if (cursor == 1)
            {
                r = under.cbox->move(b, movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                return r;
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
            else
            {
                assert(cursor == 1);
                r = under.cbox->move(b, movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Last:
        {
            r = under.cbox->move(b, movearg_Last);
            assert(r == moveret_OK);
            cursor = 1;
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
            else
            {
                assert(cursor == 1);
                r = under.cbox->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Switch:
        {
            rowbox * newunder = steal_rowbox(under.cbox, 0,0);
            rowbox * newbody = steal_rowbox(body.cbox, 0,0);
            rowbox* newover = new rowbox(2, 0,1);
            newover->child[0].cibox = iboximm_make(CHAR_Placeholder);
            newover->child[1].cibox.ptr = new nullbox();
            b = new underoverscriptbox(newbody, newunder, newover, 2);
            return moveret_Replace;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

insertRet underscriptbox::insert(boxbase*&b, insertArg m)
{
    if (cursor == 0)
    {
        return body.cbox->insert(b, m);
    }
    else
    {
        assert(cursor == 1);
        return under.cbox->insert(b, m);
    }
}

removeRet underscriptbox::remove(boxbase*&b, removeArg m)
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
    else
    {
        assert(cursor == 1);
        if (under.cbox->is_selected_placeholder())
        {
            b = steal_rowbox(body.cbox, 0,0);
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
}

ex underscriptbox::get_ex()
{
    uex t1(body.cbox->get_ex());
    ex t2 = under.cbox->get_ex();
    return emake_node(gs.sym_sUnderscriptBox.copy(), t1.release(), t2);
}


void underscriptbox::get_cursor(aftransform * T)
{
    if (cursor == 0)
    {
        body.cbox->get_cursor(T);
        T->orig_x += body.offx;
        T->orig_y += body.offy;
    }
    else
    {
        assert(cursor == 1);
        under.cbox->get_cursor(T);
        T->orig_x += under.offx;
        T->orig_y += under.offy;        
    }
}

void underscriptbox::measure(boxmeasurearg ma)
{
    body.cbox->measure(boxmeasurearg(ma));
    under.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,2), ma.deswidth, ma.mflags, ma.level + 1));

    sizex = std::max(body.cbox->sizex, under.cbox->sizex);
    sizey = body.cbox->sizey + under.cbox->sizey;
    centery = body.cbox->centery;
    body.offx = (sizex - body.cbox->sizex)/2;
    under.offx = (sizex - under.cbox->sizex)/2;
    body.offy = 0;
    under.offy = body.cbox->sizey;
}

void underscriptbox::draw_pre(boxdrawarg da)
{
    if (cursor == 0)
    {
        body.cbox->draw_pre(boxdrawarg(da, body.offx, body.offy));
    }
    else
    {
        assert(cursor == 1);
        under.cbox->draw_pre(boxdrawarg(da, under.offx, under.offy));
    }
}

void underscriptbox::draw_main(boxdrawarg da)
{
    body.cbox->draw_main(boxdrawarg(da, body.offx, body.offy, cursor == 0 ? 0 : DFLAG_IGNORESEL));
    under.cbox->draw_main(boxdrawarg(da, under.offx, under.offy, cursor == 1 ? 0 : DFLAG_IGNORESEL));
}

void underscriptbox::draw_post(boxdrawarg da)
{
    if (cursor == 0)
    {
        body.cbox->draw_post(boxdrawarg(da, body.offx, body.offy));
    }
    else
    {
        assert(cursor == 1);
        under.cbox->draw_post(boxdrawarg(da, under.offx, under.offy));
    }
}
