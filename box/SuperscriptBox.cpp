#include <stack>
#include "boxes.h"
#include "ex_parse.h"
#include "notebook.h"
#include "box_convert.h"

void superscriptbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");

    printf("(%d,%d) superscript: size(%d,%d:%d)\n",
             offx, offy, sizex, sizey, sizey);

    super.cbox->print(indent + 1, super.offx, super.offy);
}

visitRet superscriptbox::visit(visitArg m)
{
    super.cbox->visit(m);
    return visitret_OK;
}

boxbase * superscriptbox::copy()
{
    rowbox * newsuper = dynamic_cast<rowbox*>(super.cbox->copy());
    superscriptbox * r = new superscriptbox(newsuper);
    r->super.offx = super.offx;
    r->super.offy = super.offy;
    return r;    
}

void superscriptbox::key_copy(boxbase*&b)
{
    super.cbox->key_copy(b);
}

void superscriptbox::key_paste(boxbase*&b)
{
    super.cbox->key_paste(b);
}

void superscriptbox::insert_char(int32_t c)
{
    super.cbox->insert_char(c);
}

moveRet superscriptbox::move(boxbase*&b, moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        case movearg_ShiftLeft:
        case movearg_Right:
        case movearg_ShiftRight:
        {
            r = super.cbox->move(b, m);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_Last:
        case movearg_First:
        {
            r = super.cbox->move(b, m);
            assert(r == moveret_OK);
            return r;
        }
        case movearg_Home:
        case movearg_End:
        case movearg_Tab:
        {
            r = super.cbox->move(b, m);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_Switch:
        {
            rowbox * newsuper = steal_rowbox(super.cbox, 0,0);
            rowbox* newsub = new rowbox(2, 0,1);
            newsub->child[0].cibox = iboximm_make(CHAR_Placeholder);
            newsub->child[1].cibox.ptr = new nullbox();
            b = new subsuperscriptbox(newsub, newsuper, 0);
            return moveret_Replace;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

insertRet superscriptbox::insert(boxbase*&b, insertArg m)
{
    return super.cbox->insert(b, m);
}

removeRet superscriptbox::remove(boxbase*&b, removeArg m)
{
    assert(b == nullptr);
    removeRet r;

    if (super.cbox->is_selected_placeholder())
    {
        b = nullptr;
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
            b = steal_rowbox(super.cbox, 0,0);
            return removeret_Replace;
        }
        else
        {
            return removeret_OK;
        }
    }
}

ex superscriptbox::get_ex()
{
    ex t = super.cbox->get_ex();
    return emake_node(gs.sym_sSuperscriptBox.copy(), t);
}


void superscriptbox::get_cursor(aftransform * T)
{
    super.cbox->get_cursor(T);
    T->orig_x += super.offx;
    T->orig_y += super.offy;        
}

void superscriptbox::measure(boxmeasurearg ma)
{
    uint32_t fs = fontint_to_fontsize(ma.fi);
    int32_t default_sizey = fontsize_default_sizey(fs);
    int32_t default_centery = fontsize_default_centery(fs);
    int32_t default_sizex = fontsize_default_sizex(fs);

    super.cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,2), ma.deswidth, ma.mflags, ma.level + 1));
    super.offx = 0;
    sizex = super.cbox->sizex;
    centery = default_centery;
    sizey = super.cbox->sizey + centery;
    super.offy = centery;
}

void superscriptbox::draw_pre(boxdrawarg da)
{
    super.cbox->draw_pre(boxdrawarg(da, super.offx, super.offy));
}

void superscriptbox::draw_main(boxdrawarg da)
{
    super.cbox->draw_main(boxdrawarg(da, super.offx, super.offy));
}

void superscriptbox::draw_post(boxdrawarg da)
{
    super.cbox->draw_post(boxdrawarg(da, super.offx, super.offy));
}
