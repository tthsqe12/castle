#include <stack>
#include "boxes.h"
#include "ex_parse.h"
#include "notebook.h"
#include "box_convert.h"

void monobox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) mono: size (%d,%d:%d) cursor (%d,%d)\n", offx, offy, sizex, sizey, centery, row_cursor, col_cursor);

    size_t r = 0;

    for (auto y = array.begin(); y != array.end(); ++y)
    {
        for (size_t i = 0; i <= indent; i++)
            printf("    ");

        std::cout << "line " << r << ": ";

        for (auto x = y->begin(); x != y->end(); ++x)
            std::cout << char(*x);

        std::cout << std::endl;
    }
}

visitRet monobox::visit(visitArg m)
{
    return visitret_OK;
}

boxbase * monobox::copy()
{
    return new monobox(row_cursor, col_cursor, row_cursor2, col_cursor2, array);
}

void monobox::key_copy(boxbase*&b)
{
    assert(b == nullptr);

    size_t starty = row_cursor;
    size_t stopy = row_cursor2;
    size_t startx = col_cursor;
    size_t stopx = col_cursor2;

    if (row_cursor < row_cursor2)
    {
    }
    else if (row_cursor > row_cursor2)
    {
        std::swap(startx, stopx);
        std::swap(starty, stopy);
    }
    else
    {
        if (col_cursor < col_cursor2)
        {
            std::swap(startx, stopx);
            std::swap(starty, stopy);
        }
        else if (col_cursor < col_cursor2)
        {
        }
        else
        {
            return;
        }
    }

    monobox* r = new monobox(0, 0, 0, 0, 0);
    for (size_t y = starty; y <= stopy; y++)
    {
        r->array.push_back(std::vector<int32_t>());
        for (size_t x = (y == starty ? startx : 0); x < (y == stopy ? stopx : array[y].size()); ++x)
            r->array.back().push_back(array[y][x]);
    }

    b = r;
}

void monobox::key_paste(boxbase*&b)
{
    boxbase* d = nullptr;
    if (b->get_type() == BNTYPE_MONO)
    {
        monobox* B = dynamic_cast<monobox*>(b);
        for (auto y = B->array.begin(); y != B->array.end(); ++y)
        {
            for (auto x = y->begin(); x != y->end(); ++x)
                insert_char(*x);

            insert(d, insertarg_Newline);
        }
        delete b;
        b = nullptr;
    }
    else if (b->get_type() == BNTYPE_ROW)
    {
        rowbox* B = dynamic_cast<rowbox*>(b);
        for (auto i = B->child.begin(); i != B->child.end(); ++i)
        {
            int32_t t = ibox_type(i->cibox);
            if (t >= 0)
            {
                insert_char(t);
            }
            else if (t == BNTYPE_NULLER)
            {
                insert(d, insertarg_Newline);                
            }
        }
        delete b;
        b = nullptr;
    }
}

void monobox::delete_selection()
{
    size_t starty = row_cursor;
    size_t stopy = row_cursor2;
    size_t startx = col_cursor;
    size_t stopx = col_cursor2;

    if (row_cursor < row_cursor2)
    {
    }
    else if (row_cursor > row_cursor2)
    {
        std::swap(startx, stopx);
        std::swap(starty, stopy);
    }
    else
    {
        if (col_cursor < col_cursor2)
        {
            std::swap(startx, stopx);
            std::swap(starty, stopy);
        }
        else if (col_cursor < col_cursor2)
        {
        }
        else
        {
            return;
        }
    }

    for (size_t y = stopy; y > starty; y--)
    {
        if (y == stopy)
        {
            array[y].erase(array[y].begin(), array[y].begin() + stopx);
        }
        else
        {
            array.erase(array.begin() + y);
        }
    }

    if (stopy == starty)
        array[starty].erase(array[starty].begin() + startx, array[starty].begin() + stopx);        
    else
        array[starty].erase(array[starty].begin() + startx, array[starty].end());

    col_cursor = col_cursor2 = startx;
    row_cursor = row_cursor2 = starty;

    return;
}

void monobox::insert_char(int32_t c)
{
    delete_selection();

    assert(row_cursor < array.size());
    assert(col_cursor <= array[row_cursor].size());
    
    array[row_cursor].insert(array[row_cursor].begin() + col_cursor, c);

    col_cursor++;
    col_cursor2 = col_cursor;
    row_cursor2 = row_cursor;
}

moveRet monobox::move(boxbase*&b, moveArg m)
{
    moveRet r = moveret_OK;
    switch (m)
    {
        case movearg_Left:
        case movearg_ShiftLeft:
        {
            if (col_cursor > 0)
            {
                col_cursor--;
            }
            else if (row_cursor > 0)
            {
                row_cursor--;
                col_cursor = array[row_cursor].size();
            }
            else
            {
                r = moveret_End;
            }

            if (m == movearg_Left)
            {
                col_cursor2 = col_cursor;
                row_cursor2 = row_cursor;                
            }
            return r;
        }

        case movearg_Right:
        case movearg_ShiftRight:
        {
            if (col_cursor < array[row_cursor].size())
            {
                col_cursor++;
            }
            else if (row_cursor + 1 < array.size())
            {
                row_cursor++;
                col_cursor = 0;
            }
            else
            {
                r = moveret_End;
            }

            if (m == movearg_Right)
            {
                col_cursor2 = col_cursor;
                row_cursor2 = row_cursor;                
            }
            return r;
        }

        case movearg_Up:
        case movearg_ShiftUp:
        {
            if (row_cursor > 0)
            {
                row_cursor--;
                col_cursor = std::min(col_cursor, array[row_cursor].size());
            }
            else
            {
                r = moveret_End;
            }

            if (m == movearg_Up)
            {
                col_cursor2 = col_cursor;
                row_cursor2 = row_cursor;                
            }
            return r;
        }

        case movearg_Down:
        case movearg_ShiftDown:
        {
            if (row_cursor + 1 < array.size())
            {
                row_cursor++;
                col_cursor = std::min(col_cursor, array[row_cursor].size());
            }
            else
            {
                r = moveret_End;
            }

            if (m == movearg_Down)
            {
                col_cursor2 = col_cursor;
                row_cursor2 = row_cursor;                
            }
            return r;
        }

        case movearg_Last:
        {
            row_cursor2 = row_cursor = array.size() - 1;
            col_cursor2 = col_cursor = array[row_cursor].size();
            return r;
        }
        case movearg_First:
        {
            row_cursor2 = row_cursor = 0;
            col_cursor2 = col_cursor = 0;
            return r;
        }
        case movearg_Home:
        {
            row_cursor2 = row_cursor;
            col_cursor2 = col_cursor = 0;
            return r;
        }
        case movearg_End:
        {
            row_cursor2 = row_cursor;
            col_cursor2 = col_cursor = array[row_cursor].size();
            return r;
        }
        case movearg_Tab:
        {
            do {
                insert_char(' ');
            } while ((col_cursor % 4) != 0);
            return r;
        }
        default:
        {
            return r;
        }
    }
}

insertRet monobox::insert(boxbase*&b, insertArg m)
{
    insertRet r = insertret_Done;
    switch (m)
    {
        case insertarg_Newline:
        {
            delete_selection();
            array.insert(array.begin() + row_cursor + 1, std::vector<int32_t>());
            for (size_t x = col_cursor; x < array[row_cursor].size(); x++)
                array[row_cursor + 1].push_back(array[row_cursor][x]);
            array[row_cursor].resize(col_cursor);
            row_cursor2 = row_cursor = row_cursor + 1;
            col_cursor2 = col_cursor = 0;
            return r;
        }
        default:
        {
            return r;
        }
    }
}

removeRet monobox::remove(boxbase*&b, removeArg m)
{
    assert(b == nullptr);
    removeRet r = removeret_OK;

    if (row_cursor == row_cursor2 && col_cursor == col_cursor2)
    {
        switch (m)
        {
            case removearg_Left:
            {
                if (col_cursor > 0)
                {
                    col_cursor--;
                    array[row_cursor].erase(array[row_cursor].begin() + col_cursor);
                }
                else if (row_cursor > 0)
                {
                    for (size_t x = 0; x < array[row_cursor].size(); x++)
                        array[row_cursor - 1].push_back(array[row_cursor][x]);
                    array.erase(array.begin() + row_cursor);
                    row_cursor--;
                    col_cursor = array[row_cursor].size();
                }
                else
                {
                    r = removeret_End;
                }

                row_cursor2 = row_cursor;
                col_cursor2 = col_cursor;
                return r;
            }
            case removearg_Right:
            {
                if (col_cursor < array[row_cursor].size())
                {
                    array[row_cursor].erase(array[row_cursor].begin() + col_cursor);
                }
                else if (row_cursor + 1 < array.size())
                {
                    for (size_t x = 0; x < array[row_cursor + 1].size(); x++)
                        array[row_cursor].push_back(array[row_cursor + 1][x]);
                    array.erase(array.begin() + row_cursor + 1);
                }
                else
                {
                    r = removeret_End;
                }

                row_cursor2 = row_cursor;
                col_cursor2 = col_cursor;
                return r;
            }
            default:
            {
                assert(false);
                return r;
            }
        }
    }
    else
    {
        delete_selection();
        return removeret_OK;
    }
}

ex monobox::get_ex()
{
    std::string s;
    uex r(gs.sym_sList.get(), array.size());
    for (auto y = array.begin(); y != array.end(); ++y)
    {
        s.clear();
        for (auto x = y->begin(); x != y->end(); ++x)
            stdstring_pushback_char16(s, char16_t(*x));
        r.push_back(emake_str_move(s));
    }
    return emake_node(gs.sym_sMonoBox.copy(), r.release());
}


void monobox::get_cursor(aftransform * T)
{
    T->orig_x = mx + col_cursor*dx;
    T->orig_y = my + row_cursor*dy + dy/2;
    T->theta = 0;
    T->cos_theta = 1.0;
    T->sin_theta = 0.0;
}

void monobox::measure(boxmeasurearg ma)
{
    fs = fontint_to_fontsize(fontint_to_courier_reg(fontint_smaller(ma.fi,1)));
    dx = fontsize_char_sizex(fs, 'm');
    dy = fontsize_char_sizey(fs, 'm');
    mx = dx/4;
    my = dy/4;
    size_t m = 1;
    size_t lsize = 0;
    for (auto y = array.begin(); y != array.end(); ++y)
    {
        lsize += y->size() + 1;
        m = std::max(m, y->size());
    }
    sizex = m*dx + 2*mx;
    sizey = array.size()*dy + 2*my;
    centery = sizey/2;

    blexer L(lsize);
    for (auto y = array.begin(); y != array.end(); ++y)
    {
        for (auto x = y->begin(); x != y->end(); ++x)
        {
            L.add_char(*x);
        }
        L.add_newline();
    }

    size_t loff = 0;
    for (auto y = array.begin(); y != array.end(); ++y)
    {
        for (auto x = y->begin(); x != y->end(); ++x)
        {
            *x = (L.type[loff] << 16) + (*x & 65535);
            loff++;
        }
        loff++;
    }
}

void monobox::draw_pre(boxdrawarg da)
{
    drawtrect(da.globx, da.globx + sizex,
              da.globy, da.globy + sizey, 0x00F8F8F8, da.T);

    size_t starty = row_cursor;
    size_t stopy = row_cursor2;
    size_t startx = col_cursor;
    size_t stopx = col_cursor2;

    if (row_cursor < row_cursor2)
    {
    }
    else if (row_cursor > row_cursor2)
    {
        std::swap(startx, stopx);
        std::swap(starty, stopy);
    }
    else
    {
        if (col_cursor < col_cursor2)
        {
            std::swap(startx, stopx);
            std::swap(starty, stopy);
        }
        else if (col_cursor < col_cursor2)
        {
        }
        else
        {
            goto draw_cursor;
        }
    }

    for (size_t y = stopy; y > starty; y--)
    {
        if (y == stopy)
        {
//            r->array[y].erase(r->array[y].begin(), r->array[y].begin() + stopx);
            drawtrect(da.globx + mx + 0*dx, da.globx + mx + stopx*dx,
                      da.globy + my + y*dy, da.globy + my + y*dy + dy, da.nb->cSelectionBackground, da.T);
        }
        else
        {
//            r->array.erase(r->array.begin() + y);
            drawtrect(da.globx + mx + 0*dx, da.globx + mx + array[y].size()*dx,
                      da.globy + my + y*dy, da.globy + my + y*dy + dy, da.nb->cSelectionBackground, da.T);
        }
    }

    if (stopy == starty)
    {
//        r->array[starty].erase(r->array[starty].begin() + startx, r->array[starty].begin() + stopx);
        drawtrect(da.globx + mx + startx*dx, da.globx + mx + stopx*dx,
                  da.globy + my + starty*dy, da.globy + my + starty*dy + dy, da.nb->cSelectionBackground, da.T);
    }
    else
    {
//        r->array[starty].erase(r->array[starty].begin() + startx, r->array[starty].end());
        drawtrect(da.globx + mx + startx*dx, da.globx + mx + array[starty].size()*dx,
                  da.globy + my + starty*dy, da.globy + my + starty*dy + dy, da.nb->cSelectionBackground, da.T);
    }

draw_cursor:   

    int32_t usx = da.globx + mx + col_cursor*dx;
    int32_t usy = da.globy + my + row_cursor*dy;
    drawtline(usx, usy,
              usx, usy + dy,
              6.0 + fontsize_size(fs)*0.04, da.nb->cCursor, da.T);
}

void monobox::draw_main(boxdrawarg da)
{
    if (da.dflags & DFLAG_SCOLOR)
    {
        for (size_t y = 0; y < array.size(); y++)
        {
            for (size_t x = 0; x < array[y].size(); x++)
            {
                int32_t child_type = array[y][x];
                uint32_t color = da.nb->pallet1[(child_type>>16)&255];
                if (    ((child_type>>16)&255) == lextype_symbol_1st
                     || ((child_type>>16)&255) == lextype_pattern_1st
                     || ((child_type>>16)&255) == lextype_symbol)
                {
                    color = da.nb->pallet1[lextype_MAX + ((child_type>>24)&255)];
                }
//                if (selstart <= i && i < selend)
//                {
//                    color = da.nb->cSelectionForeground;
//                }

                drawtchar(fs, child_type&65535, dx, dy, da.globx + mx + x*dx, da.globy + my + y*dy, color, da.T);
            }
        }
    }
    else
    {
        uint32_t color = glb_image.fcolor;
        for (size_t y = 0; y < array.size(); y++)
        {
            for (size_t x = 0; x < array[y].size(); x++)
            {
                drawtchar(fs, array[y][x]&65535, dx, dy, da.globx + mx + x*dx, da.globy + my + y*dy, color, da.T);
            }
        }
    }
}

void monobox::draw_post(boxdrawarg da)
{

}

