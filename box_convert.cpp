#include "box_convert.h"
#include "ex_parse.h"
#include "ex_cont.h"
#include "eval.h"
#include "arithmetic.h"


rowbox* rowbox_from_ex(er e)
{
    boxbase* b = boxbase_from_ex(e);
    if (b->get_type() == BNTYPE_ROW)
    {
        return dynamic_cast<rowbox*>(b);
    }
    else if (b->get_type() == BNTYPE_ROOT || b->get_type() == BNTYPE_CELLGROUP)
    {
        delete b;
        b = new errorbox(e);
    }
    rowbox* r = new rowbox(2, 0,0);
    r->child[0].cibox = iboxptr_make(b);
    r->child[1].cibox = iboxptr_make(new nullbox());
    return r;
}

boxbase * boxbase_from_ex(er e)
{
//std::cout << "boxbase_convert_from_ex: " << ex_tostring_full(e) << std::endl;
    if (eis_leaf(e))
    {
        if (eis_str(e))
        {
            rowbox* newrow = new rowbox(0, 0,0);
            size_t n = eto_str(e)->string.size();
            const char * a = eto_str(e)->string.c_str();
            for (size_t i = 0; i < n;)
            {
                char16_t c;
                i += readonechar16(c, a + i);
                newrow->child.push_back(iboxarrayelem(iboximm_make(c)));
            }
            newrow->child.push_back(iboxarrayelem(new nullbox()));
            return newrow;
        }
        else if (eis_sym(e, gs.sym_sNull.get()))
        {
            return new nullbox();
        }
        else
        {
            return new errorbox(e);
        }
    }
    else if (eis_cont(e))
    {
        return new errorbox(e);
    }

    assert(eis_node(e));

    if (ehas_head_sym(e, gs.sym_sRowBox.get()))
    {
        if (elength(e) == 0 || !ehas_head_sym(echild(e,1), gs.sym_sList.get())) {return nullptr;}
        rowbox* us = new rowbox(0, 0,0);
        er f = echild(e,1);
        for (size_t i = 1; i <= elength(f); i++)
        {
            if (eis_str(echild(f,i)))
            {
                size_t n = estr_string(echild(f,i)).size();
                const char * a = estr_string(echild(f,i)).c_str();
                for (size_t j = 0; j < n;)
                {
                    char16_t c;
                    j += readonechar16(c, a + j);
                    us->child.push_back(iboxarrayelem(iboximm_make(c)));
                }
            }
            else if (eis_sym(echild(f,i), gs.sym_sNull.get()))
            {
                us->child.push_back(iboxarrayelem(new nullbox));
            }
            else
            {
                boxbase * child = boxbase_from_ex(echild(f,i));
                if (child->get_type() == BNTYPE_ROW)
                {
                    rowbox* r = dynamic_cast<rowbox*>(child);
                    for (int32_t j = 0; j + 1 < r->child.size(); j++)
                    {
                        us->child.push_back(iboxarrayelem(r->child[j].cibox));
                        r->child[j].cibox = iboximm_make(0);
                    }
                    delete child;
                }
                else if (child->get_type() == BNTYPE_ROOT || child->get_type() == BNTYPE_CELLGROUP)
                {
                    delete child;
                    us->child.push_back(iboxarrayelem(new errorbox(echild(f,i))));
                }
                else
                {
                    us->child.push_back(iboxarrayelem(child));
                }
            }
        }
        us->child.push_back(iboxarrayelem(new nullbox));
        return us;
    }
    else if (ehas_head_sym(e, gs.sym_sMonoBox.get()))
    {
        if (elength(e) < 1)
        {
            return new errorbox(e);
        }
        
        er f = echild(e,1);
        if (!ehas_head_sym(f, gs.sym_sList.get()))
        {
            return new errorbox(e);
        }

        monobox* us = new monobox(0,0,0,0,0);
        for (size_t i = 1; i <= elength(f); i++)
        {
            us->array.push_back(std::vector<int32_t>());
            if (eis_str(echild(f,i)))
            {
                const char * sp = estr_string(echild(f,i)).c_str();
                size_t sn = estr_string(echild(f,i)).size();
                for (size_t si = 0; si < sn;)
                {
                    char16_t c;
                    si += readonechar16(c, sp + si);
                    us->array.back().push_back(c);
                }
            }
            else
            {
                delete us;
                return new errorbox(e);
            }
        }
        if (us->array.empty())
            us->array.push_back(std::vector<int32_t>());
        return us;
    }
    else if (ehas_head_sym_length(e, gs.sym_sFractionBox.get(), 2))
    {
        rowbox* num = rowbox_from_ex(echild(e,1));
        rowbox* den = rowbox_from_ex(echild(e,2));
        return new fractionbox(num, den, 0);
    }
    else if (ehas_head_sym_length(e, gs.sym_sSqrtBox.get(), 1))
    {
        rowbox* inside = rowbox_from_ex(echild(e,1));
        return new sqrtbox(inside);
    }
    else if (ehas_head_sym_length(e, gs.sym_sRotationBox.get(), 4))
    {
        if (!eis_int(echild(e,3)) || !eis_int(echild(e,4))) {return new errorbox(e);}
        xfmpz_t base(UWORD(1)), r;
        fmpz_mul_2exp(base.data, base.data, 32);
        fmpz_mod(r.data, eint_data(echild(e,3)), base.data);
        uint32_t extra0 = fmpz_get_ui(r.data);
        fmpz_mod(r.data, eint_data(echild(e,4)), base.data);
        uint32_t extra1 = fmpz_get_ui(r.data);
        rowbox* inside = rowbox_from_ex(echild(e,1));
        return new rotationbox(dynamic_cast<rowbox*>(inside), extra0, extra1);
    }
    else if (ehas_head_sym_length(e, gs.sym_sGridBox.get(), 1))
    {
        er f = echild(e,1);
        if (!eis_matrix(f)
            || elength(f) > (1<<15)
            || elength(echild(f,1)) > (1<<15)
            || elength(f)*elength(echild(f,1)) > (1<<20))
        {
            return new errorbox(e);
        }
        gridbox * us = new gridbox(elength(f), elength(echild(f,1)), 0, 0);
        for (size_t j=1; j<=elength(f); j++)
        {
            for (size_t i=1; i<=elength(echild(f,j)); i++)
            {
                us->array[j-1][i-1].cbox = rowbox_from_ex(echild(f,j,i));
            }
        }
        return us;
    }
    else if (ehas_head_sym_length(e, gs.sym_sGraphics3DBox.get(), 10))
    {
        graphics3dbox * us = new graphics3dbox(echild(e,1), echild(e,2), echild(e,3), echild(e,4),
            econvert_todouble(echild(e,5)),
            econvert_todouble(echild(e,6)),
            econvert_todouble(echild(e,7)),
            econvert_todouble(echild(e,8)),
            econvert_todouble(echild(e,9)),
            econvert_todouble(echild(e,10)));
/*
        er vertices = echild(echild(e,1),1);
        er triangles = echild(echild(e,1),2,1);

        us->verts.resize(elength(vertices));
        us->tris.resize(3*elength(triangles));

        double boundXmin = 100000000000000;
        double boundXmax =-100000000000000;
        double boundYmin = 100000000000000;
        double boundYmax =-100000000000000;
        double boundZmin = 100000000000000;
        double boundZmax =-100000000000000;
        for (size_t i = 0; i < elength(vertices); i++)
        {
            us->verts[i].x = econvert_todouble(echild(vertices, i+1, 1));
            us->verts[i].y = econvert_todouble(echild(vertices, i+1, 2));
            us->verts[i].z = econvert_todouble(echild(vertices, i+1, 3));
            boundXmin = std::min(boundXmin, us->verts[i].x);
            boundXmax = std::max(boundXmax, us->verts[i].x);
            boundYmin = std::min(boundYmin, us->verts[i].y);
            boundYmax = std::max(boundYmax, us->verts[i].y);
            boundZmin = std::min(boundZmin, us->verts[i].z);
            boundZmax = std::max(boundZmax, us->verts[i].z);
        }

        double boundXmid = (boundXmax + boundXmin)/2;
        double boundXwid = (boundXmax - boundXmin)/2;
        double boundYmid = (boundYmax + boundYmin)/2;
        double boundYwid = (boundYmax - boundYmin)/2;
        double boundZmid = (boundZmax + boundZmin)/2;
        double boundZwid = (boundZmax - boundZmin)/2;
        double boundCorner = 1/sqrt(boundXwid*boundXwid + boundYwid*boundYwid + boundZwid*boundZwid);
        us->boxX = boundXwid*boundCorner;
        us->boxY = boundYwid*boundCorner;
        us->boxZ = boundZwid*boundCorner;

        for (size_t i = 0; i < elength(vertices); i++)
        {
            us->verts[i].x = (us->verts[i].x - boundXmid)*boundCorner;
            us->verts[i].y = (us->verts[i].y - boundYmid)*boundCorner;
            us->verts[i].z = (us->verts[i].z - boundZmid)*boundCorner;
        }
        
        for (size_t i = 0; i < elength(triangles); i++)
        {
            us->tris[3*i + 0] = fmpz_get_si(eint_data(echild(triangles,i+1,1))) - 1;
            us->tris[3*i + 1] = fmpz_get_si(eint_data(echild(triangles,i+1,2))) - 1;
            us->tris[3*i + 2] = fmpz_get_si(eint_data(echild(triangles,i+1,3))) - 1;
        }

        us->qangle->w = 0.8;
        us->qangle->x = -0.6;
        us->qangle->y = 0.0;
        us->qangle->z = 0.0;
        us->fov = 1.8;

        us->image.fill(0.9,0.9,0.9);
        us->render();
*/
        return us;
    }

/*
    else if (ehas_head_sym_length(e, gs.sym_sButtonBox.get(), 1))
    {
        boxnode * us = boxnode_create(BNTYPE_BUTTON, 1);
        for (size_t i = 1; i <= 1; i++)
        {
            boxnode * child = boxnode_convert_from_ex(echild(e,i));
            if (child == nullptr) {box_node_delete(us); return nullptr;}
            if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
            boxnode_append(us, bfrom_node(child));
        }
        return us;
    }
*/
    else if (ehas_head_sym_length(e, gs.sym_sSuperscriptBox.get(), 1))
    {
        rowbox* super = rowbox_from_ex(echild(e,1));
        return new superscriptbox(super);
    }
    else if (ehas_head_sym_length(e, gs.sym_sSubscriptBox.get(), 1))
    {
        rowbox* sub = rowbox_from_ex(echild(e,1));
        return new subscriptbox(sub);
    }
    else if (ehas_head_sym_length(e, gs.sym_sSubsuperscriptBox.get(), 2))
    {
        rowbox* sub = rowbox_from_ex(echild(e,1));
        rowbox* super = rowbox_from_ex(echild(e,2));
        return new subsuperscriptbox(sub, super, 0);
    }
    else if (ehas_head_sym_length(e, gs.sym_sOverscriptBox.get(), 2))
    {
        rowbox* body = rowbox_from_ex(echild(e,1));
        rowbox* over = rowbox_from_ex(echild(e,2));
        return new overscriptbox(body, over, 0);
    }
    else if (ehas_head_sym_length(e, gs.sym_sUnderscriptBox.get(), 2))
    {
        rowbox* body = rowbox_from_ex(echild(e,1));
        rowbox* under = rowbox_from_ex(echild(e,2));
        return new overscriptbox(body, under, 0);

    }
    else if (ehas_head_sym_length(e, gs.sym_sUnderoverscriptBox.get(), 3))
    {
        rowbox* body = rowbox_from_ex(echild(e,1));
        rowbox* under = rowbox_from_ex(echild(e,2));
        rowbox* over = rowbox_from_ex(echild(e,3));
        return new underoverscriptbox(body, under, over, 0);
    }
    else if (ehas_head_sym(e, gs.sym_sCell.get()))
    {
        if (elength(e) < 2)
        {
            return new errorbox(e);
        }

        rowbox* body = rowbox_from_ex(echild(e,1));
        if (body == nullptr) {assert(false); return nullptr;}
        if (body->get_type() != BNTYPE_ROW) {assert(false); delete body; return nullptr;}
        cellbox* us = new cellbox(dynamic_cast<rowbox*>(body), cellt_INPUT);

        if (eis_str(echild(e,2)))
        {
            for (int i = 0; i < cellt_max; i++)
            {
                if (eto_str(echild(e,2))->string.compare(cell_type_names[i]) == 0)
                    us->celltype = cellType(i);
            }
        }
        // process options
        for (size_t i = 3; i <= elength(e); i++)
        {
            if (ehas_head_sym_length(echild(e,i), gs.sym_sRule.get(), 2)
                && eis_sym(echild(e,i,1), gs.sym_sCellLabel.get())
                && us->label.cbox == nullptr)
            {
                us->label.cbox = rowbox_from_ex(echild(e,i,2));
            }
        }
        return us;
    }
    else if (ehas_head_sym(e, gs.sym_sCellGroupData.get()))
    {
        if (elength(e) < 1)
        {
            return new errorbox(e);
        }
        
        er f = echild(e,1);
        if (!ehas_head_sym(f, gs.sym_sList.get()) || elength(f)<2)
        {
            return new errorbox(e);
        }

        cellgroupbox* us = new cellgroupbox(elength(f));
        for (size_t i = 1; i <= elength(f); i++)
        {
            boxbase* c = boxbase_from_ex(echild(f,i));
            if (!(c->get_type() == BNTYPE_CELL || c->get_type() == BNTYPE_CELLGROUP))
            {
                delete c;
                rowbox* r = new rowbox(2, 0,0);
                r->child[0].cibox = iboxptr_make(new errorbox(echild(f,i)));
                r->child[1].cibox = iboxptr_make(new nullbox());
                c = new cellbox(r, cellt_INPUT);
            }
            us->child[i-1].cbox = c;
        }
        // TODO process options
        return us;
    }
    else if (ehas_head_sym(e, gs.sym_sNotebook.get()))
    {
        if (elength(e) < 1)
        {
            return new errorbox(e);
        }
        
        er f = echild(e,1);
        if (!ehas_head_sym(f, gs.sym_sList.get()))
        {
            return new errorbox(e);
        }

        rootbox* us = new rootbox(elength(f), 0, 0);
        for (size_t i = 1; i <= elength(f); i++)
        {
            boxbase* c = boxbase_from_ex(echild(f,i));
            if (!(c->get_type() == BNTYPE_CELL || c->get_type() == BNTYPE_CELLGROUP))
            {
                delete c;
                rowbox* r = new rowbox(2, 0,0);
                r->child[0].cibox = iboxptr_make(new errorbox(echild(f,i)));
                r->child[1].cibox = iboxptr_make(new nullbox());
                c = new cellbox(r, cellt_INPUT);
            }
            us->child[i-1].cbox = c;
        }
        us->goto_first_cellbreak();
        us->cursor_a = us->cursor_t;
        us->cursor_b = us->cursor_t;
        // TODO process options
        return us;
    }
    else
    {
        return new errorbox(e);
    }
}
