#include "box_convert.h"
#include "ex_parse.h"
#include "ex_cont.h"
#include "eval.h"
#include "arithmetic.h"

ex boxnode_convert_to_ex(boxnode * us)
{
    if (boxnode_type(us) == BNTYPE_ROW)
    {
        std::vector<uex> v;
        for (int32_t i = 0; i + 1 < us->len; i++)
        {
            box child = us->array[i].child;
            if (bis_char(child))
            {
                if (v.empty() || !eis_str(v.back().get()))
                {
                    v.push_back(uex(emake_str()));
                }
                stdstring_pushback_char16(eto_str(v.back().get())->string, bchar_type(child)&65535);
            }
            else if (bptr_type(child) == BNTYPE_NULLER)
            {
                v.push_back(uex(ecopy(gs.sym_sNull.get())));
            }
            else
            {
                v.push_back(uex(boxnode_convert_to_ex(bto_node(child))));
            }
        }
        if (v.empty())
        {
            return emake_str();
        }
        else if (v.size() == 1 && eis_str(v[0].get()))
        {
            return ecopy(v[0].get());
        }
        else
        {
            ex f = emake_node(ecopy(gs.sym_sList.get()), v);
            return emake_node(ecopy(gs.sym_sRowBox.get()), f);
        }
    }
    else if (boxnode_type(us) == BNTYPE_GRID)
    {
        std::vector<uex> matrix;
        std::vector<uex> row;
        for (int32_t j = 0; j < us->extra1; j++)
        {
            for (int32_t i = 0; i < us->extra0; i++)
            {
                row.push_back(uex(boxnode_convert_to_ex(bto_node(us->array[us->extra0*j + i].child))));
            }
            matrix.push_back(uex(emake_node(ecopy(gs.sym_sList.get()), row)));
            row.clear();
        }
        ex f = emake_node(ecopy(gs.sym_sList.get()), matrix);
        return emake_node(ecopy(gs.sym_sGridBox.get()), f);
    }
    else if (boxnode_type(us) == BNTYPE_TABVIEW)
    {
        std::vector<uex> matrix;
        std::vector<uex> row;
        assert((us->len % 2) == 0);
        for (int32_t j = 0; j < us->len/2; j++)
        {
            for (int32_t i = 0; i < 2; i++)
            {
                row.push_back(uex(boxnode_convert_to_ex(bto_node(us->array[2*j + i].child))));
            }
            matrix.push_back(uex(emake_node(ecopy(gs.sym_sList.get()), row)));
            row.clear();
        }
        ex f = emake_node(ecopy(gs.sym_sList.get()), matrix);
        return emake_node(ecopy(gs.sym_sTabViewBox.get()), f, emake_int_ui(us->extra0));
    }
    else if (boxnode_type(us) == BNTYPE_SUPER)
    {
        std::vector<uex> v;
        for (int32_t i = 0; i < boxnode_len(us); i++)
            v.push_back(uex(boxnode_convert_to_ex(bto_node(boxnode_child(us, i)))));
        return emake_node(ecopy(gs.sym_sSuperscriptBox.get()), v);
    }
    else if (boxnode_type(us) ==  BNTYPE_SUB)
    {
        std::vector<uex> v;
        for (int32_t i = 0; i < boxnode_len(us); i++)
            v.push_back(uex(boxnode_convert_to_ex(bto_node(boxnode_child(us, i)))));
        return emake_node(ecopy(gs.sym_sSubscriptBox.get()), v);
    }
    else if (boxnode_type(us) == BNTYPE_SUBSUPER)
    {
        std::vector<uex> v;
        for (int32_t i = 0; i < boxnode_len(us); i++)
            v.push_back(uex(boxnode_convert_to_ex(bto_node(boxnode_child(us, i)))));
        return emake_node(ecopy(gs.sym_sSubsuperscriptBox.get()), v);
    }
    else if (boxnode_type(us) == BNTYPE_FRAC)
    {
        std::vector<uex> v;
        for (int32_t i = 0; i < boxnode_len(us); i++)
            v.push_back(uex(boxnode_convert_to_ex(bto_node(boxnode_child(us, i)))));
        return emake_node(ecopy(gs.sym_sFractionBox.get()), v);
    }
    else if (boxnode_type(us) == BNTYPE_SQRT)
    {
        std::vector<uex> v;
        for (int32_t i = 0; i < boxnode_len(us); i++)
            v.push_back(uex(boxnode_convert_to_ex(bto_node(boxnode_child(us, i)))));
        return emake_node(ecopy(gs.sym_sSqrtBox.get()), v);
    }
    else if (boxnode_type(us) == BNTYPE_BUTTON)
    {
        std::vector<uex> v;
        for (int32_t i = 0; i < boxnode_len(us); i++)
            v.push_back(uex(boxnode_convert_to_ex(bto_node(boxnode_child(us, i)))));
        return emake_node(ecopy(gs.sym_sButtonBox.get()), v);
    }
    else if (boxnode_type(us) == BNTYPE_UNDER)
    {
        std::vector<uex> v;
        for (int32_t i = 0; i < boxnode_len(us); i++)
            v.push_back(uex(boxnode_convert_to_ex(bto_node(boxnode_child(us, i)))));
        return emake_node(ecopy(gs.sym_sUnderscriptBox.get()), v);
    }
    else if (boxnode_type(us) == BNTYPE_OVER)
    {
        std::vector<uex> v;
        for (int32_t i = 0; i < boxnode_len(us); i++)
            v.push_back(uex(boxnode_convert_to_ex(bto_node(boxnode_child(us, i)))));
        return emake_node(ecopy(gs.sym_sOverscriptBox.get()), v);
    }
    else if (boxnode_type(us) == BNTYPE_UNDEROVER)
    {
        std::vector<uex> v;
        for (int32_t i = 0; i < boxnode_len(us); i++)
            v.push_back(uex(boxnode_convert_to_ex(bto_node(boxnode_child(us, i)))));
        return emake_node(ecopy(gs.sym_sUnderoverscriptBox.get()), v);
    }
    else if (boxnode_type(us) == BNTYPE_ROT)
    {
        std::vector<uex> v;
        v.push_back(uex(boxnode_convert_to_ex(bto_node(boxnode_child(us, 0)))));
        v.push_back(uex(ecopy(boxnode_expr(us))));
        v.push_back(uex(emake_int_ui(boxnode_extra0(us))));
        v.push_back(uex(emake_int_ui(boxnode_extra1(us))));
        return emake_node(ecopy(gs.sym_sRotationBox.get()), v);
    }
    else if (boxnode_type(us) == BNTYPE_CELL)
    {
        std::vector<uex> v;
        v.push_back(uex(boxnode_convert_to_ex(bto_node(boxnode_child(us, 0)))));
        v.push_back(uex(emake_str(
            us->extra0 == CELLTYPE_INPUT ? "Input" :
            us->extra0 == CELLTYPE_OUTPUT ? "Output" :
            us->extra0 == CELLTYPE_MESSAGE ? "Message" :
            us->extra0 == CELLTYPE_PRINT ? "Print" :
            us->extra0 == CELLTYPE_TEXT ? "Text" :
            us->extra0 == CELLTYPE_BOLDTEXT ? "BoldText" :
            us->extra0 == CELLTYPE_SUBSUBSECTION ? "Subsubsection" :
            us->extra0 == CELLTYPE_SUBSECTION ? "Subsection" :
            us->extra0 == CELLTYPE_SECTION ?  "Section" :
                                            "Title"      )));
        if (boxnode_len(us) == 2)
        {
            uex f(boxnode_convert_to_ex(bto_node(boxnode_child(us, 0))));
            f.reset(emake_node(ecopy(gs.sym_sRule.get()), ecopy(gs.sym_sCellType.get()), f.release()));
            v.push_back(std::move(f));
        }
        return emake_node(ecopy(gs.sym_sRotationBox.get()), v);
    }
    else if (us->header.type == BNTYPE_CELLGROUP)
    {
        std::vector<uex> v;
        for (int32_t i = 0; i < boxnode_len(us); i++)
            v.push_back(uex(boxnode_convert_to_ex(bto_node(boxnode_child(us, i)))));
        ex f = emake_node(ecopy(gs.sym_sList.get()), v);
        return emake_node(ecopy(gs.sym_sCellGroup.get()), f, ecopy(us->extra1 & BNFLAG_OPEN ? gs.sym_sTrue.get() : gs.sym_sFalse.get()));
    }
    else if (us->header.type == BNTYPE_ROOT)
    {
        std::vector<uex> v;
        for (int32_t i = 0; i + 1 < boxnode_len(us); i++)
            v.push_back(uex(boxnode_convert_to_ex(bto_node(boxnode_child(us, i)))));
        return emake_node(ecopy(gs.sym_sNotebook.get()), v);
    }
    else
    {
        assert(false);
        return gs.sym_s$Failed.copy();
    }
}



boxbase * boxbase_convert_from_ex(er e)
{
//std::cout << "boxbase_convert_from_ex: " << ex_tostring_full(e) << std::endl;
    if (eis_leaf(e))
    {
        if (eis_str(e))
        {
            rowbox* newrow = new rowbox(0, 0,0);
            size_t n = eto_str(e)->string.size();
            const unsigned char * a = (const unsigned char *) eto_str(e)->string.c_str();
            size_t i = 0;
            while (i < n)
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
            return nullptr;
        }
    }
    else if (eis_cont(e))
    {
        return nullptr;
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
                const unsigned char * a = (const unsigned char *) estr_string(echild(f,i)).c_str();
                size_t j = 0;
                while (j < n)
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
                boxbase * child = boxbase_convert_from_ex(echild(f,i));
                if (child == nullptr) {delete us; return nullptr;}
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
                else
                {
                    us->child.push_back(iboxarrayelem(child));
                }
            }
        }
        us->child.push_back(iboxarrayelem(new nullbox));
        return us;
    }
    else if (ehas_head_sym_length(e, gs.sym_sFractionBox.get(), 2))
    {
        boxbase * num = boxbase_convert_from_ex(echild(e,1));
        if (num == nullptr) {return nullptr;}
        if (num->get_type() != BNTYPE_ROW) {delete num; return nullptr;}
        boxbase * den = boxbase_convert_from_ex(echild(e,2));
        if (den == nullptr) {delete num; return nullptr;}
        if (den->get_type() != BNTYPE_ROW) {delete num; delete den; return nullptr;}
        return new fractionbox(dynamic_cast<rowbox*>(num), dynamic_cast<rowbox*>(den), 0);
    }
    else if (ehas_head_sym_length(e, gs.sym_sSqrtBox.get(), 1))
    {
        boxbase * inside = boxbase_convert_from_ex(echild(e,1));
        if (inside == nullptr) {return nullptr;}
        if (inside->get_type() != BNTYPE_ROW) {delete inside; return nullptr;}
        return new sqrtbox(dynamic_cast<rowbox*>(inside));
    }
    else if (ehas_head_sym_length(e, gs.sym_sRotationBox.get(), 4))
    {
        if (!eis_int(echild(e,3)) || !eis_int(echild(e,4))) {return nullptr;}
        xfmpz base(UWORD(1)), r;
        fmpz_mul_2exp(base.data, base.data, 32);
        fmpz_mod(r.data, eint_data(echild(e,3)), base.data);
        uint32_t extra0 = fmpz_get_ui(r.data);
        fmpz_mod(r.data, eint_data(echild(e,4)), base.data);
        uint32_t extra1 = fmpz_get_ui(r.data);
        boxbase * inside = boxbase_convert_from_ex(echild(e,1));
        if (inside == nullptr) {return nullptr;}
        if (inside->get_type() != BNTYPE_ROW) {delete inside; return nullptr;}
        return new rotationbox(dynamic_cast<rowbox*>(inside), extra0, extra1);
    }
    else if (ehas_head_sym_length(e, gs.sym_sGridBox.get(), 1))
    {
        er f = echild(e,1);
        if (!eis_matrix(f)) {return nullptr;}
        if (elength(f) > (1<<15)) {return nullptr;}
        if (elength(echild(f,1)) > (1<<15)) {return nullptr;}
        if (elength(f)*elength(echild(f,1)) > (1<<20)) {return nullptr;}
        gridbox * us = new gridbox(elength(f), elength(echild(f,1)), 0, 0);
        for (size_t j=1; j<=elength(f); j++)
        {
            for (size_t i=1; i<=elength(echild(f,j)); i++)
            {
                boxbase * child = boxbase_convert_from_ex(echild(f,j,i));
                if (child == nullptr) {/* TODO */ return nullptr;}
                if (child->get_type() != BNTYPE_ROW) {/* TODO */ return nullptr;}
                us->array[j-1][i-1].cbox = dynamic_cast<rowbox*>(child);
            }
        }
printf("grid box convert OK\n");
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
        boxbase * super = boxbase_convert_from_ex(echild(e,1));
        if (super == nullptr) {return nullptr;}
        if (super->get_type() != BNTYPE_ROW) {delete super; return nullptr;}
        return new superscriptbox(dynamic_cast<rowbox*>(super));
    }
    else if (ehas_head_sym_length(e, gs.sym_sSubscriptBox.get(), 1))
    {
        boxbase * sub = boxbase_convert_from_ex(echild(e,1));
        if (sub == nullptr) {return nullptr;}
        if (sub->get_type() != BNTYPE_ROW) {delete sub; return nullptr;}
        return new subscriptbox(dynamic_cast<rowbox*>(sub));
    }
    else if (ehas_head_sym_length(e, gs.sym_sSubsuperscriptBox.get(), 2))
    {
        boxbase * sub = boxbase_convert_from_ex(echild(e,1));
        if (sub == nullptr) {return nullptr;}
        if (sub->get_type() != BNTYPE_ROW) {delete sub; return nullptr;}
        boxbase * super = boxbase_convert_from_ex(echild(e,2));
        if (super == nullptr) {delete sub; return nullptr;}
        if (super->get_type() != BNTYPE_ROW) {delete sub; delete super; return nullptr;}
        return new subsuperscriptbox(dynamic_cast<rowbox*>(sub), dynamic_cast<rowbox*>(super), 0);
    }
    else if (ehas_head_sym_length(e, gs.sym_sOverscriptBox.get(), 2))
    {
        boxbase * body = boxbase_convert_from_ex(echild(e,1));
        if (body == nullptr) {return nullptr;}
        if (body->get_type() != BNTYPE_ROW) {delete body; return nullptr;}
        boxbase * over = boxbase_convert_from_ex(echild(e,2));
        if (over == nullptr) {delete body; return nullptr;}
        if (over->get_type() != BNTYPE_ROW) {delete body; delete over; return nullptr;}
        return new overscriptbox(dynamic_cast<rowbox*>(body), dynamic_cast<rowbox*>(over), 0);
    }
    else if (ehas_head_sym_length(e, gs.sym_sUnderscriptBox.get(), 2))
    {
        boxbase * body = boxbase_convert_from_ex(echild(e,1));
        if (body == nullptr) {return nullptr;}
        if (body->get_type() != BNTYPE_ROW) {delete body; return nullptr;}
        boxbase * under = boxbase_convert_from_ex(echild(e,2));
        if (under == nullptr) {delete body; return nullptr;}
        if (under->get_type() != BNTYPE_ROW) {delete body; delete under; return nullptr;}
        return new overscriptbox(dynamic_cast<rowbox*>(body), dynamic_cast<rowbox*>(under), 0);

    }
    else if (ehas_head_sym_length(e, gs.sym_sUnderoverscriptBox.get(), 3))
    {
        boxbase * body = boxbase_convert_from_ex(echild(e,1));
        if (body == nullptr) {return nullptr;}
        if (body->get_type() != BNTYPE_ROW) {delete body; return nullptr;}
        boxbase * under = boxbase_convert_from_ex(echild(e,2));
        if (under == nullptr) {delete body; return nullptr;}
        if (under->get_type() != BNTYPE_ROW) {delete body; delete under; return nullptr;}
        boxbase * over = boxbase_convert_from_ex(echild(e,3));
        if (over == nullptr) {delete body; delete under; return nullptr;}
        if (over->get_type() != BNTYPE_ROW) {delete body; delete under; delete over; return nullptr;}
        return new underoverscriptbox(dynamic_cast<rowbox*>(body), dynamic_cast<rowbox*>(under), dynamic_cast<rowbox*>(over), 0);
    }
/*
    else if (ehas_head_sym_length(e, gs.sym_sRotationBox.get(), 4))
    {
        if (!eis_int(echild(e,3)) || !eis_int(echild(e,4)))
        {
            return nullptr;
        }
        xfmpz base(UWORD(1)), r;
        fmpz_mul_2exp(base.data, base.data, 32);
        fmpz_mod(r.data, eint_data(echild(e,3)), base.data);
        uint32_t extra0 = fmpz_get_ui(r.data);
        fmpz_mod(r.data, eint_data(echild(e,4)), base.data);
        uint32_t extra1 = fmpz_get_ui(r.data);
        boxnode * child = boxnode_convert_from_ex(echild(e,1));
        if (child == nullptr) {return nullptr;}
        if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); return nullptr;}
        boxnode * us = boxnode_create(BNTYPE_ROT, 1);            
        boxnode_append(us, bfrom_node(child));
        us->expr = etor(ecopychild(e,2));
        us->extra0 = extra0;
        us->extra1 = extra1;
        return us;
    }
    else if (ehas_head_sym_length(e, gs.sym_sGridBox.get(), 1))
    {
        er f = echild(e,1);
        if (!eis_matrix(f)) {return nullptr;}
        if (elength(f) > (1<<25)) {return nullptr;}
        if (elength(echild(f,1)) > (1<<25)) {return nullptr;}
        if (elength(f)*elength(echild(f,1)) > (1<<29)) {return nullptr;}
        boxnode * us = boxnode_create(BNTYPE_GRID, elength(f)*elength(echild(f,1)));
        for (size_t j=1; j<=elength(f); j++) {
            for (size_t i=1; i<=elength(echild(f,j)); i++) {
                boxnode * child = boxnode_convert_from_ex(echild(f,j,i));
                if (child == nullptr) {box_node_delete(us); return nullptr;}
                if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                boxnode_append(us, bfrom_node(child));
            }
        }
        us->extra1 = elength(f);
        us->extra0 = elength(echild(f,1));
        return us;
    }
    else if (ehas_head_sym_length(e, gs.sym_sTabViewBox.get(), 2))
    {
        er f = echild(e,1);
        if (!eis_int(echild(e,2)) || !fmpz_fits_si(eint_data(echild(e,2)))) {return nullptr;}
        if (!eis_matrix(f)) {return nullptr;}
        if (elength(f) > (1<<25)) {return nullptr;}
        if (elength(echild(f,1)) != 2) {return nullptr;}
        boxnode * us = boxnode_create(BNTYPE_TABVIEW, elength(f)*2);
        for (size_t j = 1; j <= elength(f); j++)
        {
            for (size_t i = 1; i <= 2; i++)
            {
                boxnode * child = boxnode_convert_from_ex(echild(f,j,i));
                if (child == nullptr) {box_node_delete(us); return nullptr;}
                if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                boxnode_append(us, bfrom_node(child));
            }
        }
        us->extra0 = fmpz_get_si(eint_data(echild(e,2))) % elength(f);
        return us;
    }
    else if (ehas_head_sym(e, gs.sym_sCell.get()))
    {
        if (elength(e) < 2) {return nullptr;}
        boxnode * us = boxnode_create(BNTYPE_CELL, 1);
//std::cout << "convert cell 1" << std::endl;
        boxnode * child = boxnode_convert_from_ex(echild(e,1));
//std::cout << "convert cell 2" << std::endl;
        if (child == nullptr) {box_node_delete(us); return nullptr;}
//std::cout << "convert cell 3" << std::endl;
        if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
//std::cout << "convert cell 4" << std::endl;
        boxnode_append(us, bfrom_node(child));
//std::cout << "convert cell 5" << std::endl;
        if (!eis_str(echild(e,2))) {box_node_delete(us); return nullptr;}
//std::cout << "convert cell 6" << std::endl;
        if (eto_str(echild(e,2))->string.compare("Input") == 0) {
            us->extra0 = CELLTYPE_INPUT;
        } else if (eto_str(echild(e,2))->string.compare("Output") == 0) {
            us->extra0 = CELLTYPE_OUTPUT;
        } else if (eto_str(echild(e,2))->string.compare("Print") == 0) {
            us->extra0 = CELLTYPE_PRINT;
        } else if (eto_str(echild(e,2))->string.compare("Message") == 0) {
            us->extra0 = CELLTYPE_MESSAGE;
        } else if (eto_str(echild(e,2))->string.compare("Text") == 0) {
            us->extra0 = CELLTYPE_TEXT;
        } else if (eto_str(echild(e,2))->string.compare("BoldText") == 0) {
            us->extra0 = CELLTYPE_BOLDTEXT;
        } else if (eto_str(echild(e,2))->string.compare("Subsubsection") == 0) {
            us->extra0 = CELLTYPE_SUBSUBSECTION;
        } else if (eto_str(echild(e,2))->string.compare("Subsection") == 0) {
            us->extra0 = CELLTYPE_SUBSECTION;
        } else if (eto_str(echild(e,2))->string.compare("Section") == 0) {
            us->extra0 = CELLTYPE_SECTION;
        } else if (eto_str(echild(e,2))->string.compare("Title") == 0) {
            us->extra0 = CELLTYPE_TITLE;
        } else {
            box_node_delete(us); return nullptr;
        }
//std::cout << "convert cell 7" << std::endl;
        // process options
        for (size_t i = 3; i <= elength(e); i++) {
            if (ehas_head_sym_length(echild(e,i), gs.sym_sRule.get(), 2)) {
                if (eis_sym(echild(e,i,1), gs.sym_sCellLabel.get())) {
                    child = boxnode_convert_from_ex(echild(e,i,2));
                    if (child == nullptr) {box_node_delete(us); return nullptr;}
                    if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                    if (boxnode_len(us) == 1) {
                        boxnode_append(us, bfrom_node(child));
                    } else {
                        box_delete(boxnode_replace(us, 1, bfrom_node(child)));
                    }
                }
            }
        }
//std::cout << "convert cell 8" << std::endl;

        return us;
    }
    else if (ehas_head_sym_length(e, gs.sym_sCellGroup.get(), 2))
    {
        er f = echild(e,1);
        if (!ehas_head_sym(f, gs.sym_sList.get()) || elength(f)<2) {return nullptr;}
        boxnode * us = boxnode_create(BNTYPE_CELLGROUP, elength(f));
        for (size_t i = 1; i <= elength(f); i++) {
            boxnode * child = boxnode_convert_from_ex(echild(f,i));
            if (child == nullptr) {box_node_delete(us); return nullptr;}
            if (boxnode_type(child) != BNTYPE_CELL && boxnode_type(child) != BNTYPE_CELLGROUP) {box_node_delete(child); box_node_delete(us); return nullptr;}
            boxnode_append(us, bfrom_node(child));
        }
        us->extra1 = BNFLAG_OPEN;
        if (eis_sym(echild(e,2), gs.sym_sFalse.get())) {
            us->extra1 = 0;
        }
        return us;
    }
    else if (ehas_head_sym(e, gs.sym_sNotebook.get()))
    {
        if (elength(e) == 0 || !ehas_head_sym(echild(e,1), gs.sym_sList.get()))
        {
            return nullptr;
        }
        er f = echild(e,1);
        size_t n = elength(f);
        boxnode * newroot = boxnode_create(BNTYPE_ROOT, n + 1);
        for (size_t i = 1; i <= n; i++)
        {
            boxnode * cell = boxnode_convert_from_ex(echild(f,i));
            if (cell == nullptr) {box_node_delete(newroot); return nullptr;}
            if (boxnode_type(cell) != BNTYPE_CELL && boxnode_type(cell) != BNTYPE_CELLGROUP) {box_node_delete(cell); box_node_delete(newroot); return nullptr;}
            boxnode_append(newroot, bfrom_node(cell));
        }
        boxnode_append(newroot, bfrom_ptr(&box_null));
        return newroot;
    }
*/
    else
    {
        return nullptr;
    }
}




boxnode * boxnode_convert_from_ex(er e)
{
//std::cout << "boxnode_convert_from_ex: " << ex_tostring_full(e) << std::endl;
    if (eis_leaf(e))
    {
        if (eis_str(e))
        {
            boxnode * us = bto_node(boxnode_make_from_string(eto_str(e)->string));
            return us;
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        if (ehas_head_sym(e, gs.sym_sRowBox.get()))
        {
            if (elength(e) == 0 || !ehas_head_sym(echild(e,1), gs.sym_sList.get())) {return nullptr;}
            boxnode * us = boxnode_create(BNTYPE_ROW, 2);
            er f = echild(e,1);
            for (size_t i = 1; i <= elength(f); i++)
            {
                if (eis_sym(echild(f,i), gs.sym_sNull.get()))
                {
                    boxnode_append(us, bfrom_ptr(&box_null));
                }
                else
                {
                    boxnode * child = boxnode_convert_from_ex(echild(f,i));
                    if (child == nullptr) {box_node_delete(us); return nullptr;}
                    if (boxnode_type(child) == BNTYPE_ROW)
                    {
                        boxnode_append_row(us, child);
                    }
                    else
                    {
                        boxnode_append(us, bfrom_node(child));
                    }
                }
            }
            boxnode_append(us, bfrom_ptr(&box_null));
            us->extra1 = 0;
            return us;
        }
        else if (ehas_head_sym(e, gs.sym_sColumnBox.get()))
        {
            if (elength(e) == 0 || !ehas_head_sym(echild(e,1), gs.sym_sList.get())) {return nullptr;}
            boxnode * us = boxnode_create(BNTYPE_COL, 2);
            er f = echild(e,1);
            for (size_t i = 1; i <= elength(f); i++)
            {
                boxnode * child = boxnode_convert_from_ex(echild(f,i));
                if (child == nullptr) {box_node_delete(us); return nullptr;}
                if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                boxnode_append(us, bfrom_node(child));
            }
            return us;
        }
        else if (ehas_head_sym_length(e, gs.sym_sFractionBox.get(), 2))
        {
            boxnode * us = boxnode_create(BNTYPE_FRAC, 2);
            for (size_t i = 1; i <= 2; i++)
            {
                boxnode * child = boxnode_convert_from_ex(echild(e,i));
                if (child == nullptr) {box_node_delete(us); return nullptr;}
                if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                boxnode_append(us, bfrom_node(child));
            }
            return us;
        }
        else if (ehas_head_sym_length(e, gs.sym_sSqrtBox.get(), 1))
        {
            boxnode * us = boxnode_create(BNTYPE_SQRT, 1);
            for (size_t i = 1; i <= 1; i++)
            {
                boxnode * child = boxnode_convert_from_ex(echild(e,i));
                if (child == nullptr) {box_node_delete(us); return nullptr;}
                if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                boxnode_append(us, bfrom_node(child));
            }
            return us;
        }
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
        else if (ehas_head_sym_length(e, gs.sym_sSuperscriptBox.get(), 1))
        {
            boxnode * us = boxnode_create(BNTYPE_SUPER, 1);
            for (size_t i = 1; i <= 1; i++)
            {
                boxnode * child = boxnode_convert_from_ex(echild(e,i));
                if (child == nullptr) {box_node_delete(us); return nullptr;}
                if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                boxnode_append(us, bfrom_node(child));
            }
            return us;
        }
        else if (ehas_head_sym_length(e, gs.sym_sSubscriptBox.get(), 1))
        {
            boxnode * us = boxnode_create(BNTYPE_SUB, 1);
            for (size_t i = 1; i <= 1; i++)
            {
                boxnode * child = boxnode_convert_from_ex(echild(e,i));
                if (child == nullptr) {box_node_delete(us); return nullptr;}
                if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                boxnode_append(us, bfrom_node(child));
            }
            return us;
        }
        else if (ehas_head_sym_length(e, gs.sym_sSubsuperscriptBox.get(), 2))
        {
            boxnode * us = boxnode_create(BNTYPE_SUBSUPER, 2);
            for (size_t i = 1; i <= 2; i++)
            {
                boxnode * child = boxnode_convert_from_ex(echild(e,i));
                if (child == nullptr) {box_node_delete(us); return nullptr;}
                if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                boxnode_append(us, bfrom_node(child));
            }
            return us;
        }
        else if (ehas_head_sym_length(e, gs.sym_sUnderoverscriptBox.get(), 3))
        {
            boxnode * us = boxnode_create(BNTYPE_UNDEROVER, 3);
            for (size_t i = 1; i <= 3; i++)
            {
                boxnode * child = boxnode_convert_from_ex(echild(e,i));
                if (child == nullptr) {box_node_delete(us); return nullptr;}
                if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                boxnode_append(us, bfrom_node(child));
            }
            return us;
        }
        else if (ehas_head_sym_length(e, gs.sym_sOverscriptBox.get(), 2))
        {
            boxnode * us = boxnode_create(BNTYPE_OVER, 2);
            for (size_t i = 1; i <= 2; i++)
            {
                boxnode * child = boxnode_convert_from_ex(echild(e,i));
                if (child == nullptr) {box_node_delete(us); return nullptr;}
                if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                boxnode_append(us, bfrom_node(child));
            }
            return us;
        }
        else if (ehas_head_sym_length(e, gs.sym_sUnderscriptBox.get(), 2))
        {
            boxnode * us = boxnode_create(BNTYPE_UNDER, 2);
            for (size_t i = 1; i <= 2; i++)
            {
                boxnode * child = boxnode_convert_from_ex(echild(e,i));
                if (child == nullptr) {box_node_delete(us); return nullptr;}
                if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                boxnode_append(us, bfrom_node(child));
            }
            return us;
        }
        else if (ehas_head_sym_length(e, gs.sym_sRotationBox.get(), 4))
        {
            if (!eis_int(echild(e,3)) || !eis_int(echild(e,4)))
            {
                return nullptr;
            }
            xfmpz base(UWORD(1)), r;
            fmpz_mul_2exp(base.data, base.data, 32);
            fmpz_mod(r.data, eint_data(echild(e,3)), base.data);
            uint32_t extra0 = fmpz_get_ui(r.data);
            fmpz_mod(r.data, eint_data(echild(e,4)), base.data);
            uint32_t extra1 = fmpz_get_ui(r.data);
            boxnode * child = boxnode_convert_from_ex(echild(e,1));
            if (child == nullptr) {return nullptr;}
            if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); return nullptr;}
            boxnode * us = boxnode_create(BNTYPE_ROT, 1);            
            boxnode_append(us, bfrom_node(child));
            us->expr = etor(ecopychild(e,2));
            us->extra0 = extra0;
            us->extra1 = extra1;
            return us;
        }
        else if (ehas_head_sym_length(e, gs.sym_sGridBox.get(), 1))
        {
            er f = echild(e,1);
            if (!eis_matrix(f)) {return nullptr;}
            if (elength(f) > (1<<25)) {return nullptr;}
            if (elength(echild(f,1)) > (1<<25)) {return nullptr;}
            if (elength(f)*elength(echild(f,1)) > (1<<29)) {return nullptr;}
            boxnode * us = boxnode_create(BNTYPE_GRID, elength(f)*elength(echild(f,1)));
            for (size_t j=1; j<=elength(f); j++) {
                for (size_t i=1; i<=elength(echild(f,j)); i++) {
                    boxnode * child = boxnode_convert_from_ex(echild(f,j,i));
                    if (child == nullptr) {box_node_delete(us); return nullptr;}
                    if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                    boxnode_append(us, bfrom_node(child));
                }
            }
            us->extra1 = elength(f);
            us->extra0 = elength(echild(f,1));
            return us;
        }
        else if (ehas_head_sym_length(e, gs.sym_sTabViewBox.get(), 2))
        {
            er f = echild(e,1);
            if (!eis_int(echild(e,2)) || !fmpz_fits_si(eint_data(echild(e,2)))) {return nullptr;}
            if (!eis_matrix(f)) {return nullptr;}
            if (elength(f) > (1<<25)) {return nullptr;}
            if (elength(echild(f,1)) != 2) {return nullptr;}
            boxnode * us = boxnode_create(BNTYPE_TABVIEW, elength(f)*2);
            for (size_t j = 1; j <= elength(f); j++)
            {
                for (size_t i = 1; i <= 2; i++)
                {
                    boxnode * child = boxnode_convert_from_ex(echild(f,j,i));
                    if (child == nullptr) {box_node_delete(us); return nullptr;}
                    if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                    boxnode_append(us, bfrom_node(child));
                }
            }
            us->extra0 = fmpz_get_si(eint_data(echild(e,2))) % elength(f);
            return us;
        }
        else if (ehas_head_sym(e, gs.sym_sCell.get()))
        {
            if (elength(e) < 2) {return nullptr;}
            boxnode * us = boxnode_create(BNTYPE_CELL, 1);
//std::cout << "convert cell 1" << std::endl;
            boxnode * child = boxnode_convert_from_ex(echild(e,1));
//std::cout << "convert cell 2" << std::endl;
            if (child == nullptr) {box_node_delete(us); return nullptr;}
//std::cout << "convert cell 3" << std::endl;
            if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
//std::cout << "convert cell 4" << std::endl;
            boxnode_append(us, bfrom_node(child));
//std::cout << "convert cell 5" << std::endl;
            if (!eis_str(echild(e,2))) {box_node_delete(us); return nullptr;}
//std::cout << "convert cell 6" << std::endl;
            if (eto_str(echild(e,2))->string.compare("Input") == 0) {
                us->extra0 = CELLTYPE_INPUT;
            } else if (eto_str(echild(e,2))->string.compare("Output") == 0) {
                us->extra0 = CELLTYPE_OUTPUT;
            } else if (eto_str(echild(e,2))->string.compare("Print") == 0) {
                us->extra0 = CELLTYPE_PRINT;
            } else if (eto_str(echild(e,2))->string.compare("Message") == 0) {
                us->extra0 = CELLTYPE_MESSAGE;
            } else if (eto_str(echild(e,2))->string.compare("Text") == 0) {
                us->extra0 = CELLTYPE_TEXT;
            } else if (eto_str(echild(e,2))->string.compare("BoldText") == 0) {
                us->extra0 = CELLTYPE_BOLDTEXT;
            } else if (eto_str(echild(e,2))->string.compare("Subsubsection") == 0) {
                us->extra0 = CELLTYPE_SUBSUBSECTION;
            } else if (eto_str(echild(e,2))->string.compare("Subsection") == 0) {
                us->extra0 = CELLTYPE_SUBSECTION;
            } else if (eto_str(echild(e,2))->string.compare("Section") == 0) {
                us->extra0 = CELLTYPE_SECTION;
            } else if (eto_str(echild(e,2))->string.compare("Title") == 0) {
                us->extra0 = CELLTYPE_TITLE;
            } else {
                box_node_delete(us); return nullptr;
            }
//std::cout << "convert cell 7" << std::endl;
            // process options
            for (size_t i = 3; i <= elength(e); i++) {
                if (ehas_head_sym_length(echild(e,i), gs.sym_sRule.get(), 2)) {
                    if (eis_sym(echild(e,i,1), gs.sym_sCellLabel.get())) {
                        child = boxnode_convert_from_ex(echild(e,i,2));
                        if (child == nullptr) {box_node_delete(us); return nullptr;}
                        if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                        if (boxnode_len(us) == 1) {
                            boxnode_append(us, bfrom_node(child));
                        } else {
                            box_delete(boxnode_replace(us, 1, bfrom_node(child)));
                        }
                    }
                }
            }
//std::cout << "convert cell 8" << std::endl;

            return us;
        }
        else if (ehas_head_sym_length(e, gs.sym_sCellGroup.get(), 2))
        {
            er f = echild(e,1);
            if (!ehas_head_sym(f, gs.sym_sList.get()) || elength(f)<2) {return nullptr;}
            boxnode * us = boxnode_create(BNTYPE_CELLGROUP, elength(f));
            for (size_t i = 1; i <= elength(f); i++) {
                boxnode * child = boxnode_convert_from_ex(echild(f,i));
                if (child == nullptr) {box_node_delete(us); return nullptr;}
                if (boxnode_type(child) != BNTYPE_CELL && boxnode_type(child) != BNTYPE_CELLGROUP) {box_node_delete(child); box_node_delete(us); return nullptr;}
                boxnode_append(us, bfrom_node(child));
            }
            us->extra1 = BNFLAG_OPEN;
            if (eis_sym(echild(e,2), gs.sym_sFalse.get())) {
                us->extra1 = 0;
            }
            return us;
        }
        else if (ehas_head_sym(e, gs.sym_sNotebook.get()))
        {
            if (elength(e) == 0 || !ehas_head_sym(echild(e,1), gs.sym_sList.get()))
            {
                return nullptr;
            }
            er f = echild(e,1);
            size_t n = elength(f);
            boxnode * newroot = boxnode_create(BNTYPE_ROOT, n + 1);
            for (size_t i = 1; i <= n; i++)
            {
                boxnode * cell = boxnode_convert_from_ex(echild(f,i));
                if (cell == nullptr) {box_node_delete(newroot); return nullptr;}
                if (boxnode_type(cell) != BNTYPE_CELL && boxnode_type(cell) != BNTYPE_CELLGROUP) {box_node_delete(cell); box_node_delete(newroot); return nullptr;}
                boxnode_append(newroot, bfrom_node(cell));
            }
            boxnode_append(newroot, bfrom_ptr(&box_null));
            return newroot;
        }
        else
        {
            return nullptr;
        }
    }
}
