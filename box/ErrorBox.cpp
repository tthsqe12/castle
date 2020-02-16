#include <stack>
#include "boxes.h"
#include "ex_parse.h"
#include "notebook.h"
#include "box_convert.h"


void errorbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");

    printf("(%d,%d) error: size(%d,%d:%d)\n",
             offx, offy, sizex, sizey, sizey);
}

visitRet errorbox::visit(visitArg m)
{
    return visitret_OK;
}

boxbase * errorbox::copy()
{
    return new errorbox(nullptr);
}

void errorbox::key_copy(boxbase*&b)
{
    assert(false);
}

void errorbox::key_paste(boxbase*&b)
{
    assert(false);
}

void errorbox::insert_char(int32_t c)
{
    assert(false);
}

moveRet errorbox::move(boxbase*&b, moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        case movearg_Right:
        case movearg_First:
        case movearg_Last:
        {
            return moveret_End;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

insertRet errorbox::insert(boxbase*&b, insertArg m)
{
    insertRet r;
    switch (m)
    {
        default:
        {
            assert(false);
            return insertret_Done;
        }
    }
}

removeRet errorbox::remove(boxbase*&b, removeArg m)
{
    assert(b == nullptr);
    removeRet r;

    switch (m)
    {
        case removearg_Left:
        {
            assert(false);
            return removeret_Bad;
        }
        case removearg_Right:
        {
            assert(false);
            return removeret_Bad;
        }
        default:
        {
            assert(false);
            return removeret_Bad;
        }
    }
}

ex errorbox::get_ex()
{
    return gs.sym_sNull.copy();
}


void errorbox::get_cursor(aftransform * T)
{
    T->orig_x = 0;
    T->orig_y = centery;
    T->theta = 0;
    T->cos_theta = 1.0;
    T->sin_theta = 0.0;
}

void errorbox::measure(boxmeasurearg ma)
{
    sizex = 60;
    sizey = 60;
    centery = 30;
    return;
}


void errorbox::draw_pre(boxdrawarg da)
{
    return;
}

void errorbox::draw_main(boxdrawarg da)
{
    drawtrect(da.globx, da.globx + sizex,
              da.globy, da.globy + sizey, RGB_COLOR(0xFF,0xA0,0xD0), da.T);
    return;
}

void errorbox::draw_post(boxdrawarg da)
{
    return;
}
