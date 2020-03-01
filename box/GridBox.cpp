#include <stack>
#include "boxes.h"
#include "ex_parse.h"
#include "notebook.h"
#include "box_convert.h"

void gridbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) grid: size (%d,%d:%d) cursor (%d,%d)\n", offx, offy, sizex, sizey, centery, col_cursor, row_cursor);

    for (auto i = array.begin(); i != array.end(); ++i)
        for (auto j = i->begin(); j != i->end(); ++j)
            j->cbox->print(indent + 1, j->offx, j->offy);
}

visitRet gridbox::visit(visitArg m)
{
    for (auto i = array.begin(); i != array.end(); ++i)
        for (auto j = i->begin(); j != i->end(); ++j)
            j->cbox->visit(m);
    return visitret_OK;
}

boxbase * gridbox::copy()
{
    gridbox * r = new gridbox(array, row_cursor, col_cursor);
    for (auto i = r->array.begin(); i != r->array.end(); ++i)
        for (auto j = i->begin(); j != i->end(); ++j)
            j->cbox = dynamic_cast<rowbox*>(j->cbox->copy());
    return r;
}

void gridbox::key_copy(boxbase*&b)
{
    if (row_cursor2 == row_cursor && col_cursor2 == col_cursor)
    {
        return;
    }
    else
    {
        size_t xa = std::min(col_cursor, col_cursor2);
        size_t xb = std::max(col_cursor, col_cursor2);
        size_t ya = std::min(row_cursor, row_cursor2);
        size_t yb = std::max(row_cursor, row_cursor2);

        std::vector<std::vector<rowboxarrayelem>> newarray;
        for (size_t i = ya; i <= yb; i++)
        {
            std::vector<rowboxarrayelem> newrow;
            for (size_t j = xa; j <= xb; j++)
            {
                newrow.push_back(rowboxarrayelem(dynamic_cast<rowbox*>(array[i][j].cbox->copy())));
            }
            newarray.push_back(newrow);
        }

        assert(b == nullptr);
        b = new gridbox(newarray, 0, 0);
    }
}

void gridbox::key_paste(boxbase*&b)
{
    return;
}

void gridbox::insert_char(int32_t c)
{
    array[row_cursor][col_cursor].cbox->insert_char(c);
}

moveRet gridbox::move(boxbase*&b, moveArg m)
{
    assert(b == nullptr);
    moveRet r;

    switch (m)
    {
        case movearg_Left:
        {
            row_cursor2 = row_cursor;
            col_cursor2 = col_cursor;
            r = array[row_cursor][col_cursor].cbox->move(b, m);
            assert(r == moveret_OK || r == moveret_End);
            if (r == moveret_End)
            {
                if (col_cursor > 0)
                {
                    col_cursor2 = col_cursor = col_cursor - 1;
                    array[row_cursor][col_cursor].cbox->move(b, movearg_Last);
                    return moveret_OK;
                }
                else if (row_cursor > 0)
                {
                    row_cursor2 = row_cursor = row_cursor - 1;
                    col_cursor2 = col_cursor = array[row_cursor].size() - 1;
                    array[row_cursor][col_cursor].cbox->move(b, movearg_Last);
                    return moveret_OK;
                }
            }
            return r;
        }
        case movearg_ShiftLeft:
        {
            if (row_cursor2 == row_cursor && col_cursor2 == col_cursor)
            {
                r = array[row_cursor][col_cursor].cbox->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End && col_cursor > 0)
                {
                    col_cursor -= 1;
                    return moveret_OK;
                }
                else
                {
                    return r;
                }
            }
            else if (col_cursor > 0)
            {
                col_cursor -= 1;
                return moveret_OK;
            }
            else
            {
                return moveret_End;
            }
        }
        case movearg_Right:
        {
            row_cursor2 = row_cursor;
            col_cursor2 = col_cursor;
            r = array[row_cursor][col_cursor].cbox->move(b, m);
            assert(r == moveret_OK || r == moveret_End);
            if (r == moveret_End)
            {
                if (col_cursor < array[row_cursor].size() - 1)
                {
                    col_cursor2 = col_cursor = col_cursor + 1;
                    array[row_cursor][col_cursor].cbox->move(b, movearg_First);
                    return moveret_OK;
                }
                else if (row_cursor < array.size() - 1)
                {
                    row_cursor2 = row_cursor = row_cursor + 1;
                    col_cursor2 = col_cursor = 0;
                    array[row_cursor][col_cursor].cbox->move(b, movearg_First);
                    return moveret_OK;
                }
            }
            return r;
        }
        case movearg_ShiftRight:
        {
            if (row_cursor2 == row_cursor && col_cursor2 == col_cursor)
            {
                r = array[row_cursor][col_cursor].cbox->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End && col_cursor < array[row_cursor].size() - 1)
                {
                    col_cursor += 1;
                    return moveret_OK;
                }
                else
                {
                    return r;
                }
            }
            else if (col_cursor < array[row_cursor].size() - 1)
            {
                col_cursor += 1;
                return moveret_OK;
            }
            else
            {
                return moveret_End;
            }
        }
        case movearg_Last:
        {
            row_cursor = row_cursor2 = array.size() - 1;
            col_cursor = col_cursor2 = array[row_cursor].size() - 1;
            array[row_cursor][col_cursor].cbox->move(b, movearg_Last);
            return moveret_OK;
        }
        case movearg_First:
        {
            row_cursor = row_cursor2 = 0;
            col_cursor = col_cursor2 = 0;
            array[row_cursor][col_cursor].cbox->move(b, movearg_First);
            return moveret_OK;
        }
        case movearg_Home:
        case movearg_End:
        {
            row_cursor2 = row_cursor;
            col_cursor2 == col_cursor;
            r = array[row_cursor][col_cursor].cbox->move(b, m);
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

insertRet gridbox::insert(boxbase*&b, insertArg m)
{
    return array[row_cursor][col_cursor].cbox->insert(b, m);
}

removeRet gridbox::remove(boxbase*&b, removeArg m)
{
    return removeret_OK;
}

ex gridbox::get_ex()
{
    std::vector<uex> l;
    l.push_back(uex(gs.sym_sList.get(), array.size()));
    for (auto i = array.begin(); i != array.end(); ++i)
    {
        uex r(gs.sym_sList.get(), i->size());
        for (auto j = i->begin(); j != i->end(); ++j)
            r.push_back(j->cbox->get_ex());
        l.back().push_back(r.release());
    }

    if (!eis_sym(alignment.get(), gs.sym_sNull.get()))
    {
        l.push_back(emake_node(gs.sym_sRule.copy(), gs.sym_sGridBoxAlignment.copy(),
                                                    alignment.copy()));
    }

    return emake_node(gs.sym_sGridBox.copy(), l);
}



void gridbox::get_cursor(aftransform * T)
{
    assert(false);
    return;
}

void gridbox::measure(boxmeasurearg ma)
{
    uint32_t fs = fontint_to_fontsize(ma.fi);

    ulong row_count = array.size();
    ulong col_count = 0;

    std::vector<int32_t> max_width, acc_width;
    std::vector<int32_t> max_above(row_count, 0);
    std::vector<int32_t> max_below(row_count, 0);

    for (size_t j = 0; j < array.size(); j++)
    {
        col_count = std::max(col_count, array[j].size());
        for (size_t i = 0; i < array[j].size(); i++)
        {
            array[j][i].cbox->measure(boxmeasurearg(fontint_smaller(ma.fi,1), 4*ma.deswidth, ma.mflags, ma.level + 1));
            while (max_width.size() <= i)
                max_width.push_back(0);
            max_width[i] = std::max(max_width[i], array[j][i].cbox->sizex);
            max_above[j] = std::max(max_above[j], array[j][i].cbox->centery);
            max_below[j] = std::max(max_below[j], array[j][i].cbox->sizey - array[j][i].cbox->centery);
        }
    }

    /* read alignment options */
    std::vector<int32_t> row_option(row_count, 1);
    std::vector<int32_t> col_option(col_count, 1);
    er t = alignment.get();
    if (ehas_head_sym(t, gs.sym_sList.get()))
    {
        for (ulong i = 0; i < elength(t); i++)
        {
            er ti = echild(t,i+1);
            if (ehas_head_sym_length(ti, gs.sym_sRule.get(), 2))
            {
                er o = echild(ti,2);
                if (eis_str(echild(ti,1), estr_string(gs.strColumns.get())))
                {
                    if (ehas_head_sym(o, gs.sym_sList.get()))
                    {
                        ulong h = 0;
                        for (ulong j = 0; j < elength(o); j++)
                        {
                            if (h >= col_count)
                                break;

                            if (ehas_head_sym(echild(o,j+1), gs.sym_sList.get()))
                            {
                                for (ulong k = 0; ; k++)
                                {
                                    if (h >= col_count)
                                        break;

                                    if (k >= elength(echild(o,j+1)))
                                        k -= elength(echild(o,j+1));
                                    col_option[h++] = (eis_sym(echild(o,j+1,k+1), gs.sym_sLeft.get())) ? 0 :
                                                      (eis_sym(echild(o,j+1,k+1), gs.sym_sRight.get())) ? 2 : 1;
                                }
                            }
                            else
                            {
                                col_option[h++] = (eis_sym(echild(o,j+1), gs.sym_sLeft.get())) ? 0 :
                                                  (eis_sym(echild(o,j+1), gs.sym_sRight.get())) ? 2 : 1;
                            }
                        }
                    }
                }
                else if (eis_str(echild(ti,1), "Rows"))
                {
                    
                }
            }
        }
    }


    int32_t accx = (fs&65535)*GRID_EXTRAX1;
    for (int32_t i = 0; i < max_width.size(); i++)
    {
        acc_width.push_back(accx);
        accx += max_width[i] + int32_t((fs&65535)*GRID_EXTRAX2);
    }
    accx += int32_t((fs&65535)*GRID_EXTRAX3) - int32_t((fs&65535)*GRID_EXTRAX2);

    int32_t accy = (fs&65535)*GRID_EXTRAY1;
    for (size_t j = 0; j < array.size(); j++)
    {
        for (size_t i = 0; i < array[j].size(); i++)
        {
            array[j][i].offx = acc_width[i] + col_option[i]*(max_width[i] - array[j][i].cbox->sizex)/2;
            array[j][i].offy = accy + (max_above[j] - array[j][i].cbox->centery);
        }
        accy += max_above[j] + max_below[j] + int32_t((fs&65535)*GRID_EXTRAY2);
    }
    accy += int32_t((fs&65535)*GRID_EXTRAY3) - int32_t((fs&65535)*GRID_EXTRAY2);

    sizex = accx;
    sizey = accy;
    centery = sizey/2;
}



void gridbox::draw_pre(boxdrawarg da)
{
    if (row_cursor2 == row_cursor && col_cursor2 == col_cursor)
    {
        array[row_cursor][col_cursor].cbox->draw_pre(boxdrawarg(da, array[row_cursor][col_cursor].offx,
                                                                    array[row_cursor][col_cursor].offy));
    }
    else
    {
        size_t xa = std::min(col_cursor, col_cursor2);
        size_t xb = std::max(col_cursor, col_cursor2);
        size_t ya = std::min(row_cursor, row_cursor2);
        size_t yb = std::max(row_cursor, row_cursor2);

        for (size_t i = ya; i <= yb; i++)
        {
            for (size_t j = xa; j <= xb; j++)
            {
                int32_t child_sizex = array[i][j].cbox->sizex;
                int32_t child_sizey = array[i][j].cbox->sizey;
                int32_t usx = da.globx + array[i][j].offx;
                int32_t usy = da.globy + array[i][j].offy;
                drawtrect(usx, usx + child_sizex,
                          usy, usy + child_sizey, da.nb->cSelectionBackground, da.T);
            }
        }
    }
}

void gridbox::draw_main(boxdrawarg da)
{
    if (da.dflags & DFLAG_SELECTION)
    {
        for (size_t i = 0; i < array.size(); i++)
        {
            for (size_t j = 0; j < array[i].size(); j++)
            {
                array[i][j].cbox->draw_main(boxdrawarg(da, array[i][j].offx, array[i][j].offy, DFLAG_SELECTION));
                
            }
        }

    }
    else if (da.dflags & DFLAG_IGNORESEL)
    {
        for (size_t i = 0; i < array.size(); i++)
        {
            for (size_t j = 0; j < array[i].size(); j++)
            {
                array[i][j].cbox->draw_main(boxdrawarg(da, array[i][j].offx, array[i][j].offy, 0));
                
            }
        }

    }
    else if (row_cursor2 == row_cursor && col_cursor2 == col_cursor)
    {
        for (size_t i = 0; i < array.size(); i++)
        {
            for (size_t j = 0; j < array[i].size(); j++)
            {
                array[i][j].cbox->draw_main(boxdrawarg(da, array[i][j].offx, array[i][j].offy,
                                         (i == row_cursor && j == col_cursor) ? 0 : DFLAG_IGNORESEL));
                
            }
        }
    }    
    else
    {
        size_t xa = std::min(col_cursor, col_cursor2);
        size_t xb = std::max(col_cursor, col_cursor2);
        size_t ya = std::min(row_cursor, row_cursor2);
        size_t yb = std::max(row_cursor, row_cursor2);
        for (size_t i = 0; i < array.size(); i++)
        {
            for (size_t j = 0; j < array[i].size(); j++)
            {
                array[i][j].cbox->draw_main(boxdrawarg(da, array[i][j].offx, array[i][j].offy,
                                         (ya <= i && i <= yb && xa <= j && j <= xb) ? DFLAG_SELECTION : DFLAG_IGNORESEL));
                
            }
        }
    }
}

void gridbox::draw_post(boxdrawarg da)
{
    if (row_cursor2 == row_cursor && col_cursor2 == col_cursor)
    {
        array[row_cursor][col_cursor].cbox->draw_post(boxdrawarg(da,
                                                        array[row_cursor][col_cursor].offx,
                                                        array[row_cursor][col_cursor].offy));
    }
}
