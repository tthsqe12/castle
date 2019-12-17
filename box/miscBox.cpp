#include <stack>
#include "boxes.h"
#include "ex_parse.h"
#include "notebook.h"
#include "box_convert.h"


void nullbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");

    printf("(%d,%d) null: size(%d,%d:%d)\n",
             offx, offy, sizex, sizey, sizey);
}

void charbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) char: (%d,%d,%d) size(%d,%d:%d)\n",
        offx, offy, (get_type()>>24)&255, (get_type()>>16)&255, (get_type())&65535, sizex, sizey, sizey);
}

visitRet nullbox::visit(visitArg m)
{
    return visitret_OK;
}

visitRet charbox::visit(visitArg m)
{
    return visitret_OK;
}

boxbase * nullbox::copy()
{
    return new nullbox();
}

void nullbox::key_copy(boxbase*&b)
{
    assert(false);
}

void nullbox::key_paste(boxbase*&b)
{
    assert(false);
}

boxbase * charbox::copy()
{
    return new charbox(get_type(), sizex, sizey, centery);
}

void charbox::key_copy(boxbase*&b)
{
    assert(false);
}

void charbox::key_paste(boxbase*&b)
{
    assert(false);
}

void charbox::insert_char(int32_t c)
{
    assert(false);
}

void nullbox::insert_char(int32_t c)
{
    assert(false);
}

moveRet charbox::move(boxbase*&b, moveArg m)
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

moveRet nullbox::move(boxbase*&b, moveArg m)
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

insertRet charbox::insert(boxbase*&b, insertArg m)
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

insertRet nullbox::insert(boxbase*&b, insertArg m)
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

removeRet nullbox::remove(boxbase*&b, removeArg m)
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

removeRet charbox::remove(boxbase*&b, removeArg m)
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

ex charbox::get_ex()
{
    std::string s;
    stdstring_pushback_char16(s, get_type()&65535);
    return emake_str_move(s);
}

ex nullbox::get_ex()
{
    return gs.sym_sNull.copy();
}


void nullbox::get_cursor(aftransform * T)
{
    T->orig_x = 0;
    T->orig_y = centery;
    T->theta = 0;
    T->cos_theta = 1.0;
    T->sin_theta = 0.0;
}

void charbox::get_cursor(aftransform * T)
{
    T->orig_x = 0;
    T->orig_y = centery;
    T->theta = 0;
    T->cos_theta = 1.0;
    T->sin_theta = 0.0;
}

void charbox::measure(boxmeasurearg ma)
{
    assert(false);
}

void nullbox::measure(boxmeasurearg ma)
{
    return;
}



void nullbox::draw_pre(boxdrawarg da)
{
    return;
}

void nullbox::draw_main(boxdrawarg da)
{
    return;
}

void nullbox::draw_post(boxdrawarg da)
{
    return;
}

void charbox::draw_pre(boxdrawarg da)
{
    assert(false);
}

void charbox::draw_main(boxdrawarg da)
{
    assert(false);
}

void charbox::draw_post(boxdrawarg da)
{
    assert(false);
}
