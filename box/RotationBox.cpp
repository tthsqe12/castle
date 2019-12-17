#include <stack>
#include "boxes.h"
#include "ex_parse.h"
#include "notebook.h"
#include "box_convert.h"

void rotationbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) rotation: size (%d,%d:%d)\n", offx, offy, sizex, sizey, centery);

    inside.cbox->print(indent + 1, inside.offx, inside.offy);
}

visitRet rotationbox::visit(visitArg m)
{
    inside.cbox->visit(m);
    return visitret_OK;
}

boxbase * rotationbox::copy()
{
    rowbox * newinside = dynamic_cast<rowbox*>(inside.cbox->copy());
    rotationbox * r = new rotationbox(newinside, angle, original_angle);
    r->inside.offx = inside.offx;
    r->inside.offy = inside.offy;
    return r;    
}

void rotationbox::key_copy(boxbase*&b)
{
    inside.cbox->key_copy(b);
}

void rotationbox::key_paste(boxbase*&b)
{
    inside.cbox->key_paste(b);
}

void rotationbox::insert_char(int32_t c)
{
    if (c < 0 && inside.cbox->cursor_b < inside.cbox->child.size())
    {
        if (c == -11)
        {
            angle -= 1 << 24;
        }
        else if (c == -12)
        {
            angle += 1 << 24;
        }
        return;
    }
    else
    {
        inside.cbox->insert_char(c);
        return;
    }
}

moveRet rotationbox::move(boxbase*&b, moveArg m)
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
        case movearg_Tab:
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

insertRet rotationbox::insert(boxbase*&b, insertArg m)
{
    return inside.cbox->insert(b, m);
}

removeRet rotationbox::remove(boxbase*&b, removeArg m)
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

ex rotationbox::get_ex()
{
    uex t1(inside.cbox->get_ex());
    uex t2(emake_cint(0)); // TODO
    uex t3(emake_int_ui(angle));
    ex t4 = emake_int_ui(original_angle);
    return emake_node(gs.sym_sRotationBox.copy(), t1.release(), t2.release(), t3.release(), t4);
}


void rotationbox::get_cursor(aftransform * T)
{
    inside.cbox->get_cursor(T);
    double x = T->orig_x;
    double y = T->orig_y;
    T->orig_x = inside.offx + T->cos_theta*x - T->sin_theta*y;
    T->orig_y = inside.offy + T->sin_theta*x + T->cos_theta*y;
    T->theta += angle;
    cossinth(T->cos_theta, T->sin_theta, T->theta);
    return;
}

void rotationbox::measure(boxmeasurearg ma)
{
    inside.cbox->measure(boxmeasurearg(ma));
    rotbox_sizexy_offxy(sizex, sizey, inside.offx, inside.offy,
                        angle, inside.cbox->sizex, inside.cbox->sizey);
    centery = sizey/2;
}

void rotationbox::draw_pre(boxdrawarg da)
{
    aftransform S;
    S.orig_x = da.T->orig_x + da.T->cos_theta*(da.globx + inside.offx) - da.T->sin_theta*(da.globy + inside.offy);
    S.orig_y = da.T->orig_y + da.T->sin_theta*(da.globx + inside.offx) + da.T->cos_theta*(da.globy + inside.offy);
    S.theta = da.T->theta + angle;
    S.rix = da.T->rix;
    S.riy = da.T->riy;
    cossinth(S.cos_theta, S.sin_theta, S.theta);
    boxdrawarg nda(da.nb, 0, 0, da.dflags, &S);
    inside.cbox->draw_pre(nda);
}

void rotationbox::draw_main(boxdrawarg da)
{
    aftransform S;
    S.orig_x = da.T->orig_x + da.T->cos_theta*(da.globx + inside.offx) - da.T->sin_theta*(da.globy + inside.offy);
    S.orig_y = da.T->orig_y + da.T->sin_theta*(da.globx + inside.offx) + da.T->cos_theta*(da.globy + inside.offy);
    S.theta = da.T->theta + angle;
    S.rix = da.T->rix;
    S.riy = da.T->riy;
    cossinth(S.cos_theta, S.sin_theta, S.theta);
    boxdrawarg nda(da.nb, 0, 0, da.dflags, &S);
    inside.cbox->draw_main(nda);
}

void rotationbox::draw_post(boxdrawarg da)
{
    aftransform S;
    S.orig_x = da.T->orig_x + da.T->cos_theta*(da.globx + inside.offx) - da.T->sin_theta*(da.globy + inside.offy);
    S.orig_y = da.T->orig_y + da.T->sin_theta*(da.globx + inside.offx) + da.T->cos_theta*(da.globy + inside.offy);
    S.theta = da.T->theta + angle;
    S.rix = da.T->rix;
    S.riy = da.T->riy;
    cossinth(S.cos_theta, S.sin_theta, S.theta);
    boxdrawarg nda(da.nb, 0, 0, da.dflags, &S);
    inside.cbox->draw_post(nda);
}

