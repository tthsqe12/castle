#include <stack>
#include "boxes.h"
#include "ex_parse.h"
#include "notebook.h"
#include "box_convert.h"

void cellbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) cell: size(%d,%d:%d)\n", offx, offy, sizex, sizey, centery);

    body.cbox->print(indent + 1, body.offx, body.offy);

    if (0 && label.cbox != nullptr)
        label.cbox->print(indent + 1, label.offx, label.offy);

    if (mexpr != nullptr)
        mexpr->print(indent + 1, 0, 0);
}

visitRet cellbox::visit(visitArg m)
{
    if (mexpr != nullptr)
        mexpr->visit(m);
    body.cbox->visit(m);
    if (label.cbox != nullptr)
        label.cbox->visit(m);
    return visitret_OK;
}

boxbase * cellbox::copy()
{
    monobox* newmexpr = dynamic_cast<monobox*>(mexpr->copy());
    rowbox* newbody = dynamic_cast<rowbox*>(body.cbox->copy());
//    rowbox * newlabel = (label.cbox == nullptr) ? label.cbox : dynamic_cast<rowbox*>(label.cbox->copy());
    rowbox* newlabel = nullptr;
    cellbox* r = new cellbox(newbody, newlabel, newmexpr, celltype);
    r->body.offx = body.offx;
    r->body.offy = body.offy;
//    r->label.offx = label.offx;
//    r->label.offy = label.offy;
    return r;
}

void cellbox::key_copy(boxbase*&b)
{
    if (mexpr != nullptr)
    {
        mexpr->key_copy(b);
        return;
    }

    if (cursor == 0)
    {
        body.cbox->key_copy(b);
    }
    else
    {
        assert(cursor == 1);
        label.cbox->key_copy(b);
    }
}

void cellbox::key_paste(boxbase*&b)
{
    if (mexpr != nullptr)
    {
        mexpr->key_paste(b);
        return;
    }

    if (cursor == 0)
    {
        body.cbox->key_paste(b);
    }
    else
    {
        assert(cursor == 1);
        label.cbox->key_paste(b);
    }
}

void cellbox::insert_char(int32_t c)
{
    if (mexpr != nullptr)
    {
        mexpr->insert_char(c);
        return;
    }

    switch (cursor)
    {
        case 0:
            body.cbox->insert_char(c);
            return;
        case 1:
            assert(label.cbox != nullptr);
            label.cbox->insert_char(c);
            return;
        default:
            assert(false);
    }
}

moveRet cellbox::move(boxbase*&b, moveArg m)
{
    moveRet r;

    if (mexpr != nullptr)
    {
        return mexpr->move(b, m);
    }

    switch (m)
    {
        case movearg_Left:
        case movearg_ShiftLeft:
        case movearg_Right:
        case movearg_ShiftRight:
        case movearg_Up:
        case movearg_ShiftUp:
        case movearg_Down:
        case movearg_ShiftDown:
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
                r = label.cbox->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_First:
        case movearg_Last:
        {
            r = body.cbox->move(b, m);
            cursor = 0;
            assert(r == moveret_OK);
            return r;
        }
        case movearg_Home:
        case movearg_End:
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
                r = label.cbox->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }            
        }
        case movearg_Switch:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(b, movearg_Switch);
                return moveret_OK;
            }
            else
            {
                assert(cursor == 1);
                r = label.cbox->move(b, movearg_Switch);
                return moveret_OK;
            }
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

insertRet cellbox::insert(boxbase*&b, insertArg m)
{
    if (mexpr != nullptr)
    {
        return mexpr->insert(b, m);
    }

    if (cursor == 0)
    {
        return body.cbox->insert(b, m);
    }
    else
    {
        assert(cursor == 1);
        return label.cbox->insert(b, m);
    }
}

removeRet cellbox::remove(boxbase*&b, removeArg m)
{
//std::cout << "cellbox::remove called" << std::endl;
    assert(b == nullptr);
    removeRet r;

    if (mexpr != nullptr)
    {
        r = mexpr->remove(b, m);
        assert(r == removeret_OK || r == removeret_End);
        return removeret_OK;
    }

    if (cursor == 0)
    {
        r = body.cbox->remove(b, m);
        assert(r == removeret_OK || r == removeret_End);
        return removeret_OK;
    }
    else
    {
        assert(cursor == 1);
        r = label.cbox->remove(b, m);
        assert(r == removeret_OK || r == removeret_End);
        if (r == removeret_End)
        {
            cursor == 0;
            body.cbox->cursor_b = body.cbox->cursor_a = 0;
        }
        return removeret_OK;
    }
}

void cellbox::toggle_cell_expr()
{
    if (mexpr != nullptr)
    {
        install_live_psymbols LPS;
        eparser P;
        syntax_report sr;

        for (auto y = mexpr->array.begin(); y != mexpr->array.end(); ++y)
        {
            for (auto x = y->begin(); x != y->end(); ++x)
            {
                P.handle_rawchar(*x&65535);
                if (P.error)
                {
                    sr.handle_cstr_error(P.error, NULL, 0);
                    break;
                }
            }
            P.handle_newline();
            if (P.error)
            {
                sr.handle_row_error(P.error, NULL, 0);
                break;
            }
        }

        if (sr.error)
        {
            std::cerr << "syntax error" << std::endl;
            return;
        }
        P.handle_end();
        if (P.have_one_ex())
        {
            gs.send_psymbols();
            boxbase* newcell = boxbase_from_ex(P.estack[0].get());
            if (newcell->get_type() == BNTYPE_CELL)
            {
                cellbox* C = dynamic_cast<cellbox*>(newcell);
                std::swap(celltype, C->celltype);
                std::swap(body.cbox, C->body.cbox);
                std::swap(label.cbox, C->label.cbox);
                delete mexpr;
                mexpr = nullptr;
            }
            return;
        }
        else
        {
            std::cerr << "syntax error" << std::endl;
            return;
        }
    }
    else
    {
        wex e(get_ex());
        std::string s = ex_tostring_full(e.get());
        mexpr = new monobox(1, 0, 0, 0, 0);
        for (size_t i = 0; i < s.size(); )
        {
            char16_t c;
            i += readonechar16(c, s.c_str() + i);
            mexpr->array[0].push_back(c);
        }
    }
}

ex cellbox::get_ex()
{
    std::vector<wex> v;
    v.push_back(wex(body.cbox->get_ex()));
    v.push_back(wex(emake_str(cell_type_names[celltype])));
    if (label.cbox != nullptr)
    {
        ex t = label.cbox->get_ex();
        v.push_back(wex(emake_node(gs.sym_sRule.copy(), gs.sym_sCellLabel.copy(), t)));
    }
    return emake_node(gs.sym_sCell.copy(), v);
}


void cellbox::get_cursor(aftransform * T)
{
    if (mexpr != nullptr)
    {
        mexpr->get_cursor(T);
        T->orig_x += bracket_offy;
        T->orig_y += bracket_offy;
    }
    else if (cursor == 0)
    {
        body.cbox->get_cursor(T);
        T->orig_x += body.offx;
        T->orig_y += body.offy;
    }
    else
    {
        assert(cursor == 1);
        label.cbox->get_cursor(T);
        T->orig_x += label.offx;
        T->orig_y += label.offy;        
    }
}



void cellbox::measure(boxmeasurearg ma)
{
    if (mexpr != nullptr)
    {
        mexpr->measure(ma);
        bracket_offy = 30;
        bracket_sizey = mexpr->sizey;
        sizex = mexpr->sizex + 2*bracket_offy;
        sizey = mexpr->sizey + 2*bracket_offy;
        centery = sizey/2;
        return;
    }

    uint32_t fs = fontint_to_fontsize(ma.fi);

    int32_t pad_above = 0;
    int32_t pad_left = 50;
    uint32_t dflag = 0;
    uint32_t fint = ma.fi;
    switch (celltype)
    {
        case cellt_INPUT:
            pad_above = 16; pad_left = 180;
            dflag = MFLAG_EXSPACE;
            fint = fontint_to_courier_bold(fontint_smaller(ma.fi,0));
            break;
        case cellt_OUTPUT:
            pad_above = 16; pad_left = 180;
            dflag = MFLAG_EXSPACE;
            fint = fontint_to_courier_reg(fontint_smaller(ma.fi,0));
            break;
        case cellt_PRINT:
            pad_above = 8;  pad_left = 120;
            dflag = MFLAG_EXSPACE;
            fint = fontint_to_courier_reg(fontint_smaller(ma.fi,1));
            break;
        case cellt_MESSAGE:
            pad_above = 10;  pad_left = 160;
            fint = fontint_to_tahoma_reg(fontint_smaller(ma.fi,1));
            break;
        case cellt_TEXT:
            pad_above = 12; pad_left = 40;
            fint = fontint_to_tahoma_reg(fontint_smaller(ma.fi,1));
            break;
        case cellt_BOLDTEXT:
            pad_above = 12; pad_left = 40;
            fint = fontint_to_tahoma_bold(fontint_smaller(ma.fi,1));
            break;
        case cellt_SUBSUBSECTION:
            pad_above = 16; pad_left = 90;
            fint = fontint_to_tahoma_reg(fontint_larger(ma.fi,2));
            break;
        case cellt_SUBSECTION:
            pad_above = 18; pad_left = 60;
            fint = fontint_to_tahoma_reg(fontint_larger(ma.fi,4));
            break;
        case cellt_SECTION:
            pad_above = 20; pad_left = 30;
            fint = fontint_to_times_reg(fontint_larger(ma.fi,6));
            break;
        case cellt_TITLE:
            pad_above = 24; pad_left = 15;
            fint = fontint_to_times_bold(fontint_larger(ma.fi,8));
            break;
    }
    pad_above += pad_above*fontsize_size(fs)/32;
    int32_t pad_below = pad_above;
    pad_left = pad_left*fontsize_size(fs)/32;
    int32_t pad_right = fontsize_size(fs)/32;

//printf("fi: %x  smaller: %x\n",fi,fontint_to_sansreg(fontint_smaller(fi,2)));

    int32_t cellbracket_w = glb_dingbat.get_char_sizex(DINGBAT_CELLGEN, fontsize_size(fs));
    int32_t cellbracket_h = glb_dingbat.get_char_sizey(DINGBAT_CELLGEN, fontsize_size(fs));

    body.cbox->measure(boxmeasurearg(fint, ma.deswidth - pad_left - pad_right - cellbracket_w*(ma.level + 1), dflag, ma.level + 1));

    int32_t label_sizey = 0;
    bool label_on_left = false;
    if (label.cbox != nullptr)
    {
        label.cbox->measure(boxmeasurearg(fontint_to_tahoma_reg(fontint_smaller(ma.fi,2)), ma.deswidth - pad_left - cellbracket_w*(ma.level + 1), 0, ma.level + 1));

        if (label.cbox->sizex + fontsize_size(fs)/2 >= pad_left)
        {
            label.offx = fontsize_size(fs)/2;
            label.offy = (pad_above+1)/2;
            pad_above = std::max(pad_above, label.offy + label.cbox->sizey);
        }
        else
        {
            label.offx = pad_left - (label.cbox->sizex + fontsize_size(fs)/2);
            label.offy = pad_above + 3;
            label_on_left = true;
        }
        label_sizey = label.offy + label.cbox->sizey;
    }

    do {
        body.offx = pad_left;
        body.offy = pad_above;
        sizex = body.offx + body.cbox->sizex;
        sizey = std::max(label_sizey, body.offy + body.cbox->sizey + pad_below);
        centery = sizey/2;
        pad_above += 1;
        pad_below += 1;
    } while (sizey < cellbracket_h);

    if (label_on_left)
    {
        label.offy = body.offy + body.cbox->child[0].offy + ibox_centery(body.cbox->child[0].cibox)
                              - label.cbox->child[0].offy - ibox_centery(label.cbox->child[0].cibox);
    }

    bracket_offy = body.offy;
    bracket_sizey = body.cbox->sizey;
}



void cellbox::draw_pre(boxdrawarg da)
{
    if (mexpr != nullptr)
    {
        mexpr->draw_pre(boxdrawarg(da, bracket_offy, bracket_offy));
        return;
    }

    if (cursor == 0)
    {
        body.cbox->draw_pre(boxdrawarg(da, body.offx, body.offy));        
    }
    else
    {
        assert(cursor == 1);
        assert(label.cbox != nullptr);
        label.cbox->draw_pre(boxdrawarg(da, label.offx, label.offy));
    }
}

void cellbox::draw_main(boxdrawarg da)
{
    _draw_cellgroup_bracket(this, da);

    if (mexpr != nullptr)
    {
        glb_image.fcolor = da.nb->cCellForegroundInput;
        mexpr->draw_main(boxdrawarg(da, bracket_offy, bracket_offy));
        return;
    }

    switch (celltype)
    {
        case cellt_INPUT:
            da.dflags = DFLAG_SCOLOR | DFLAG_NLINE;
            glb_image.fcolor = da.nb->cCellForegroundInput;
            break;
        case cellt_OUTPUT:
            da.dflags = DFLAG_NLINE;
            glb_image.fcolor = da.nb->cCellForegroundOutput;
            break;
        case cellt_PRINT:
            da.dflags = DFLAG_NLINE;
            glb_image.fcolor = da.nb->cCellForegroundPrint;
            break;
        case cellt_MESSAGE:
            da.dflags = 0;
            glb_image.fcolor = da.nb->cCellForegroundMessage;
            break;
        case cellt_TEXT:
            da.dflags = 0;
            glb_image.fcolor = da.nb->cCellForegroundText;
            break;
        case cellt_SUBSUBSECTION:
            da.dflags = 0;
            glb_image.fcolor = da.nb->cCellForegroundSubsubsection;
            break;
        case cellt_SUBSECTION:
            da.dflags = 0;
            glb_image.fcolor = da.nb->cCellForegroundSubsection;
            break;
        case cellt_SECTION:
            da.dflags = 0;
            glb_image.fcolor = da.nb->cCellForegroundSection;
            break;
        case cellt_TITLE:
            da.dflags = 0;
            glb_image.fcolor = da.nb->cCellForegroundTitle;
            break;
    }

    body.cbox->draw_main(boxdrawarg(da, body.offx, body.offy, cursor == 0 ? 0 : DFLAG_IGNORESEL));
    glb_image.fcolor = da.nb->cCellLabel;
    da.dflags = 0;
    if (label.cbox != nullptr)
    {
        label.cbox->draw_main(boxdrawarg(da, label.offx, label.offy, cursor == 1 ? 0 : DFLAG_IGNORESEL));
    }
}

void cellbox::draw_post(boxdrawarg da)
{
    if (mexpr != nullptr)
    {
        mexpr->draw_post(boxdrawarg(da, 0, 0));
        return;
    }

    if (cursor == 0)
    {
        body.cbox->draw_post(boxdrawarg(da, body.offx, body.offy));        
    }
    else
    {
        assert(cursor == 1);
        assert(label.cbox != nullptr);
        label.cbox->draw_post(boxdrawarg(da, label.offx, label.offy));
    }
}
