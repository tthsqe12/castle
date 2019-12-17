#include <stack>
#include "boxes.h"
#include "ex_parse.h"
#include "notebook.h"
#include "box_convert.h"

void overscriptbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) overscript: size (%d,%d:%d) cursor(%d)\n", offx, offy, sizex, sizey, centery, cursor);

    body.cbox->print(indent + 1, body.offx, body.offy);
    over.cbox->print(indent + 1, over.offx, over.offy);
}

visitRet overscriptbox::visit(visitArg m)
{
    body.cbox->visit(m);
    over.cbox->visit(m);
    return visitret_OK;
}

boxbase * overscriptbox::copy()
{
    rowbox * newbody = dynamic_cast<rowbox*>(body.cbox->copy());
    rowbox * newover = dynamic_cast<rowbox*>(over.cbox->copy());
    overscriptbox * r = new overscriptbox(newbody, newover, cursor);
    r->body.offx = body.offx;
    r->body.offy = body.offy;
    r->over.offx = over.offx;
    r->over.offy = over.offy;
    return r;    
}

void overscriptbox::key_copy(boxbase*&b)
{
    if (cursor == 0)
    {
        body.cbox->key_copy(b);
    }
    else
    {
        assert(cursor == 1);
        over.cbox->key_copy(b);
    }
}

void overscriptbox::key_paste(boxbase*&b)
{
    if (cursor == 0)
    {
        body.cbox->key_paste(b);
    }
    else
    {
        assert(cursor == 1);
        over.cbox->key_paste(b);
    }
}

void overscriptbox::insert_char(int32_t c)
{
    if (cursor == 0)
    {
        body.cbox->insert_char(c);
    }
    else
    {
        assert(cursor == 1);
        over.cbox->insert_char(c);
    }
}

moveRet overscriptbox::move(boxbase*&b, moveArg m)
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
                r = over.cbox->move(b, movearg_Left);
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
                r = over.cbox->move(b, movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Right:
        {
            if (cursor == 1)
            {
                r = over.cbox->move(b, movearg_Right);
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
                    r = over.cbox->move(b, movearg_First);
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
                r = over.cbox->move(b, movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Last:
        {
            r = over.cbox->move(b, movearg_Last);
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
                r = over.cbox->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Switch:
        {
            rowbox * newover = steal_rowbox(over.cbox, 0,0);
            rowbox * newbody = steal_rowbox(body.cbox, 0,0);
            rowbox* newunder = new rowbox(2, 0,1);
            newunder->child[0].cibox = iboximm_make(CHAR_Placeholder);
            newunder->child[1].cibox.ptr = new nullbox();
            b = new underoverscriptbox(newbody, newunder, newover, 1);
            return moveret_Replace;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

insertRet overscriptbox::insert(boxbase*&b, insertArg m)
{
    if (cursor == 0)
    {
        return body.cbox->insert(b, m);
    }
    else
    {
        assert(cursor == 1);
        return over.cbox->insert(b, m);
    }
}

removeRet overscriptbox::remove(boxbase*&b, removeArg m)
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
        if (over.cbox->is_selected_placeholder())
        {
            b = steal_rowbox(body.cbox, 0,0);
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

ex overscriptbox::get_ex()
{
    uex t1(body.cbox->get_ex());
    ex t2 = over.cbox->get_ex();
    return emake_node(gs.sym_sOverscriptBox.copy(), t1.release(), t2);
}

void overscriptbox::get_cursor(aftransform * T)
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
        over.cbox->get_cursor(T);
        T->orig_x += over.offx;
        T->orig_y += over.offy;        
    }
}

void overscriptbox::measure(boxmeasurearg ma)
{
    body.cbox->measure(boxmeasurearg(ma));
    over.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,2), ma.deswidth, ma.mflags, ma.level + 1));

    sizex = std::max(body.cbox->sizex, over.cbox->sizex);
    sizey = over.cbox->sizey + body.cbox->sizey;
    centery = over.cbox->sizey + body.cbox->centery;
    over.offx = (sizex - over.cbox->sizex)/2;
    body.offx = (sizex - body.cbox->sizex)/2;
    over.offy = 0;
    body.offy = over.cbox->sizey;
}

void overscriptbox::draw_pre(boxdrawarg da)
{
    if (cursor == 0)
    {
        body.cbox->draw_pre(boxdrawarg(da, body.offx, body.offy));
    }
    else
    {
        assert(cursor == 1);
        over.cbox->draw_pre(boxdrawarg(da, over.offx, over.offy));
    }
}

void overscriptbox::draw_main(boxdrawarg da)
{
    body.cbox->draw_main(boxdrawarg(da, body.offx, body.offy, cursor == 0 ? 0 : DFLAG_IGNORESEL));
    over.cbox->draw_main(boxdrawarg(da, over.offx, over.offy, cursor == 1 ? 0 : DFLAG_IGNORESEL));
}

void overscriptbox::draw_post(boxdrawarg da)
{
    if (cursor == 0)
    {
        body.cbox->draw_post(boxdrawarg(da, body.offx, body.offy));
    }
    else
    {
        assert(cursor == 1);
        over.cbox->draw_post(boxdrawarg(da, over.offx, over.offy));
    }
}

