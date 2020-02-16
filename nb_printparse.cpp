#include "notebook.h"
#include "ex_parse.h"
#include "ex_print.h"
#include "eval.h"
#include "arithmetic.h"


ex eval_num(ex E, slong p);

void notebook::print()
{
#if 0
    printf("*****notebook*******\n");
    printf("selection type: %d\n", selection.type);
    for (size_t i =0; i<cursor1.size(); i++)
    {
        printf("cursor[%d]: %p %d\n",i,cursor1[i].node,cursor1[i].idx);
    }
    for (size_t i =0; i<selection.data.size(); i++)
    {
        printf("selection.data[%d]: %d\n",i,selection.data[i]);
    }
    boxnode_print(cursor1.back().node, bfrom_node(root), 0);
#endif
}

void _notebook_save_indent(FILE * fp, int indent)
{
    int i;
    for (i=0; i<indent; i++)
    {
        fwrite("\t", 1, 1, fp);
    }
}

#if 0
void _notebook_save_helper(FILE * fp, box Us, int indent)
{
    int i, j;
    std::string s;

    assert(bis_node(Us));
    boxnode * us = bto_node(Us);

    if (us->header.type == BNTYPE_ROW)
    {
        _notebook_save_indent(fp, indent); fwrite("RowBox[{", 1, 8, fp);
        bool charopen = false;

        for (i=0; i<us->len-1; i++)
        {
            box child = us->array[i].child;
            if (bis_char(child))
            {
                if (!charopen)
                {
                    if (i > 0)
                    {
                        fwrite(",\n", 1, 2, fp);
                        _notebook_save_indent(fp, indent+1);
                    }
                    fwrite("\"", 1, 1, fp);
                }
                charopen = true;
                stdstring_pushback_cvtascii(s, btype(child)&65535, true);
                fwrite(s.c_str(), 1, s.size(), fp);
                s.clear();
            }
            else
            {
                assert(bis_ptr(child));
                if (bptr_type(child) == BNTYPE_NULLER)
                {
                    if (charopen)
                    {
                        fwrite("\"", 1, 1, fp);
                    }
                    charopen = false;
                    if (i > 0)
                    {
                        fwrite(",", 1, 1, fp);
                    }
                    fwrite("Null", 1, 4, fp);                    
                }
                else
                {
                    assert(bis_node(child));
                    if (charopen)
                    {
                        fwrite("\"", 1, 1, fp);
                    }
                    charopen = false;
                    if (i > 0)
                    {
                        fwrite(",", 1, 1, fp);
                    }
                    fwrite("\n", 1, 1, fp);
                    _notebook_save_helper(fp, child, indent+1);
                }
            }
        }
        if (charopen)
        {
            fwrite("\"}]", 1, 3, fp);
        }
        else
        {
            fwrite("\n", 1, 1, fp);
            _notebook_save_indent(fp, indent); fwrite("}]", 1, 2, fp);
        }
    }
    else if (us->header.type == BNTYPE_GRID)
    {
        _notebook_save_indent(fp, indent); fwrite("GridBox[{{\n", 1, 11, fp);
        for (j = 0; j < us->extra1; j++)
        {
            for (i = 0; i < us->extra0; i++)
            {
                _notebook_save_helper(fp, us->array[us->extra0*j + i].child, indent+1);
                if (i + 1 < us->extra0)
                {
                    fwrite(",", 1, 1, fp);
                }
                else
                {
                    if (j+1<us->extra1)
                    {
                        fwrite("},{", 1, 3, fp);
                    }
                }
                fwrite("\n", 1, 1, fp);
            }
        }
        _notebook_save_indent(fp, indent); fwrite("}}]", 1, 3, fp);
    }
    else if (us->header.type == BNTYPE_TABVIEW)
    {
        _notebook_save_indent(fp, indent); fwrite("TabViewBox[{{\n", 1, 13, fp);
        for (j = 0; j < us->len/2; j++)
        {
            for (i = 0; i < 2; i++)
            {
                _notebook_save_helper(fp, us->array[2*j + i].child, indent+1);
                if (i + 1 < 2)
                {
                    fwrite(",", 1, 1, fp);
                }
                else
                {
                    if (j+1<us->extra1)
                    {
                        fwrite("},{", 1, 3, fp);
                    }
                }
                fwrite("\n", 1, 1, fp);
            }
        }
        _notebook_save_indent(fp, indent); fwrite("}},", 1, 3, fp);
        std::string ss = int_tostring(ulong(us->extra0));
        fwrite(ss.c_str(), 1, ss.size(), fp);
        fwrite("]", 1, 1, fp);
    }
    else if (us->header.type == BNTYPE_SUPER)
    {
        _notebook_save_indent(fp, indent); fwrite("SuperscriptBox[\n", 1, 16, fp);
        for (i=0; i<us->len; i++)
        {
            _notebook_save_helper(fp, us->array[i].child, indent+1);
            if (i+1<us->len)
            {
                fwrite(",", 1, 1, fp);
            }
            fwrite("\n", 1, 1, fp);
        }
        _notebook_save_indent(fp, indent); fwrite("]", 1, 1, fp);
    }
    else if (us->header.type == BNTYPE_SUB)
    {
        _notebook_save_indent(fp, indent); fwrite("SubscriptBox[\n", 1, 14, fp);
        for (i=0; i<us->len; i++) {
            _notebook_save_helper(fp, us->array[i].child, indent+1);
            if (i+1<us->len) {fwrite(",", 1, 1, fp); }
            fwrite("\n", 1, 1, fp);
        }
        _notebook_save_indent(fp, indent); fwrite("]", 1, 1, fp);
    }
    else if (us->header.type == BNTYPE_SUBSUPER)
    {
        _notebook_save_indent(fp, indent); fwrite("SubsuperscriptBox[\n", 1, 19, fp);
        for (i=0; i<us->len; i++)
        {
            _notebook_save_helper(fp, us->array[i].child, indent+1);
            if (i+1<us->len)
            {
                fwrite(",", 1, 1, fp);
            }
            fwrite("\n", 1, 1, fp);
        }
        _notebook_save_indent(fp, indent); fwrite("]", 1, 1, fp);
    }
    else if (us->header.type == BNTYPE_FRAC)
    {
        _notebook_save_indent(fp, indent); fwrite("FractionBox[\n", 1, 13, fp);
        for (i=0; i<us->len; i++) {
            _notebook_save_helper(fp, us->array[i].child, indent+1);
            if (i+1<us->len) {fwrite(",", 1, 1, fp); }
            fwrite("\n", 1, 1, fp);
        }
        _notebook_save_indent(fp, indent); fwrite("]", 1, 1, fp);
    }
    else if (us->header.type == BNTYPE_SQRT)
    {
        _notebook_save_indent(fp, indent); fwrite("SqrtBox[\n", 1, 9, fp);
        for (i=0; i<us->len; i++)
        {
            _notebook_save_helper(fp, us->array[i].child, indent+1);
            if (i+1<us->len)
            {
                fwrite(",", 1, 1, fp);
            }
            fwrite("\n", 1, 1, fp);
        }
        _notebook_save_indent(fp, indent); fwrite("]", 1, 1, fp);
    }
    else if (us->header.type == BNTYPE_BUTTON)
    {
        _notebook_save_indent(fp, indent); fwrite("ButtonBox[\n", 1, 9, fp);
        for (i=0; i<us->len; i++)
        {
            _notebook_save_helper(fp, us->array[i].child, indent+1);
            if (i+1<us->len)
            {
                fwrite(",", 1, 1, fp);
            }
            fwrite("\n", 1, 1, fp);
        }
        _notebook_save_indent(fp, indent); fwrite("]", 1, 1, fp);
    }
    else if (us->header.type == BNTYPE_UNDER)
    {
        _notebook_save_indent(fp, indent); fwrite("UnderscriptBox[\n", 1, 16, fp);
        for (i=0; i<us->len; i++)
        {
            _notebook_save_helper(fp, us->array[i].child, indent+1);
            if (i+1<us->len)
            {
                fwrite(",", 1, 1, fp);
            }
            fwrite("\n", 1, 1, fp);
        }
        _notebook_save_indent(fp, indent); fwrite("]", 1, 1, fp);
    }
    else if (us->header.type == BNTYPE_OVER)
    {
        _notebook_save_indent(fp, indent); fwrite("OverscriptBox[\n", 1, 15, fp);
        for (i=0; i<us->len; i++)
        {
            _notebook_save_helper(fp, us->array[i].child, indent+1);
            if (i+1<us->len)
            {
                fwrite(",", 1, 1, fp);
            }
            fwrite("\n", 1, 1, fp);
        }
        _notebook_save_indent(fp, indent); fwrite("]", 1, 1, fp);
    }
    else if (us->header.type == BNTYPE_UNDEROVER)
    {
        _notebook_save_indent(fp, indent); fwrite("UnderoverscriptBox[\n", 1, 20, fp);
        for (i=0; i<us->len; i++)
        {
            _notebook_save_helper(fp, us->array[i].child, indent+1);
            if (i+1<us->len)
            {
                fwrite(",", 1, 1, fp);
            }
            fwrite("\n", 1, 1, fp);
        }
        _notebook_save_indent(fp, indent); fwrite("]", 1, 1, fp);
    }
    else if (us->header.type == BNTYPE_ROT)
    {
        _notebook_save_indent(fp, indent); fwrite("RotationBox[\n", 1, 13, fp);
        for (i = 0; i < us->len; i++)
        {
            _notebook_save_helper(fp, us->array[i].child, indent+1);
            if (i + 1 < us->len)
            {
                fwrite(",", 1, 1, fp);
            }
            fwrite("\n", 1, 1, fp);
        }
        _notebook_save_indent(fp, indent);
        fwrite(",", 1, 1, fp);
        uex f(ecopy(us->expr));
        if (us->extra0 != us->extra1)
        {
            xfmpq q;
            fmpz_set_ui(fmpq_denref(q.data), UWORD(2147483648));
            fmpz_set_si(fmpq_numref(q.data), -slong(int32_t(us->extra0 - us->extra1)));
            fmpq_canonicalise(q.data);
            uex r(emake_rat_move(q));
            r.reset(emake_node(ecopy(gs.sym_sTimes.get()), r.release(), ecopy(gs.sym_sPi.get())));
            f.reset(emake_node(ecopy(gs.sym_sPlus.get()), f.release(), r.release()));
        }
        f.reset(eval(f.release()));
        std::string ss = ex_tostring(f.get());
        fwrite(ss.c_str(), 1, ss.size(), fp);
        fwrite("]", 1, 1, fp);
    }

    else if (us->header.type == BNTYPE_COL)
    {
        _notebook_save_indent(fp, indent); fwrite("ColumnBox[{\n", 1, 12, fp);
        for (i=0; i<us->len; i++) {
            _notebook_save_helper(fp, us->array[i].child, indent+1);
            if (i+1<us->len) {fwrite(",", 1, 1, fp); }
            fwrite("\n", 1, 1, fp);
        }
        _notebook_save_indent(fp, indent); fwrite("}]", 1, 2, fp);
    }
    else if (us->header.type == BNTYPE_CELL)
    {
        _notebook_save_indent(fp, indent); fwrite("Cell[\n", 1, 6, fp);
        for (i=0; i<1; i++) {
            _notebook_save_helper(fp, us->array[i].child, indent+1);
        }
        fwrite(",\n", 1, 2, fp);
        _notebook_save_indent(fp, indent+1);
        fwrite("\"", 1, 1, fp);
        switch (us->extra0)
        {
            case CELLTYPE_INPUT:
                fwrite("Input", 1, 5, fp); break;
            case CELLTYPE_OUTPUT:
                fwrite("Output", 1, 6, fp); break;
            case CELLTYPE_MESSAGE:
                fwrite("Message", 1, 7, fp); break;
            case CELLTYPE_PRINT:
                fwrite("Print", 1, 5, fp); break;
            case CELLTYPE_TEXT:
                fwrite("Text", 1, 4, fp); break;
            case CELLTYPE_BOLDTEXT:
                fwrite("BoldText", 1, 8, fp); break;
            case CELLTYPE_SUBSUBSECTION:
                fwrite("Subsubsection", 1, 13, fp); break;
            case CELLTYPE_SUBSECTION:
                fwrite("Subsection", 1, 10, fp); break;
            case CELLTYPE_SECTION:
                fwrite("Section", 1, 7, fp); break;
            case CELLTYPE_TITLE:
                fwrite("Title", 1, 5, fp); break;
            default:
                assert(false);
        }
        fwrite("\"", 1, 1, fp);
        if (us->len == 2)
        {
            fwrite(",CellLabel->\n", 1, 13, fp);
            _notebook_save_helper(fp, us->array[1].child, indent+2);
        }
        fwrite("]", 1, 1, fp);
    }
    else if (us->header.type == BNTYPE_CELLGROUP)
    {
        _notebook_save_indent(fp, indent); fwrite("CellGroup[{\n", 1, 12, fp);
        for (i=0; i<us->len; i++)
        {
            _notebook_save_helper(fp, us->array[i].child, indent+1);
            if (i+1<us->len) {fwrite(",", 1, 1, fp); }
            fwrite("\n", 1, 1, fp);
        }
        _notebook_save_indent(fp, indent);
        fwrite("},", 1, 2, fp);
        if (us->extra1 & BNFLAG_OPEN)
        {
            fwrite("True", 1, 4, fp);
        }
        else
        {
            fwrite("False", 1, 5, fp);
        }
        fwrite("]", 1, 1, fp);
    }
    else if (us->header.type == BNTYPE_ROOT)
    {
        _notebook_save_indent(fp, indent); fwrite("Notebook[{\n", 1, 11, fp);
        for (i=0; i<us->len-1; i++)
        {
            _notebook_save_helper(fp, us->array[i].child, indent+1);
            if (i+1<us->len-1) {fwrite(",", 1, 1, fp); }
            fwrite("\n", 1, 1, fp);
        }
        _notebook_save_indent(fp, indent); fwrite("}]", 1, 2, fp);
    }
    else
    {
        assert(false);
    }
}
#endif


void notebook::save(const char * fstr)
{
    FILE *fp = fopen(fstr,"wb");
    if (!fp)
    {
        std::cerr << "save failed" << std::endl;
        return;
    }
    wex e(root->get_ex());
    std::string s = ex_tostring_full(e.get());
    fwrite(s.c_str(), 1, s.size(), fp);
    fclose(fp);
    if (filestring.compare(fstr) != 0)
    {
        filestring.assign(fstr);
    }
}



#if 0
boxnode * _makebox_ex(er e)
{
//std::cout << "_makebox_ex: " << ex_tostring_full(e) << std::endl;
    if (eis_leaf(e))
    {
        if (eis_str(e))
        {
            return bto_node(boxnode_make_from_string(eto_str(e)->string));
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
                    boxnode * child = _makebox_ex(echild(f,i));
                    if (child == nullptr) {box_node_delete(us); return nullptr;}
                    //if (child->type == BNTYPE_COL) {boxnode_delete(child); boxnode_delete(us); return nullptr;}
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
                boxnode * child = _makebox_ex(echild(f,i));
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
                boxnode * child = _makebox_ex(echild(e,i));
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
                boxnode * child = _makebox_ex(echild(e,i));
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
                boxnode * child = _makebox_ex(echild(e,i));
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
                boxnode * child = _makebox_ex(echild(e,i));
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
                boxnode * child = _makebox_ex(echild(e,i));
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
                boxnode * child = _makebox_ex(echild(e,i));
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
                boxnode * child = _makebox_ex(echild(e,i));
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
                boxnode * child = _makebox_ex(echild(e,i));
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
                boxnode * child = _makebox_ex(echild(e,i));
                if (child == nullptr) {box_node_delete(us); return nullptr;}
                if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                boxnode_append(us, bfrom_node(child));
            }
            return us;
        }
        else if (ehas_head_sym_length(e, gs.sym_sRotationBox.get(), 2))
        {
            uex f(emake_node(
                    ecopy(gs.sym_sTimes.get()),
                    ecopychild(e,2),
                    emake_int_ui(2147483648),
                    emake_node(ecopy(gs.sym_sPower.get()), ecopy(gs.sym_sPi.get()), emake_cint(-1))
                ));
//std::cout << "f: " << ex_tostring_full(f.get()) << std::endl;
            f.reset(eval(f.release()));
            f.setnz(eval_num(f.get(), 64));
            uint32_t angle = 0;
            if (eis_number(f.get()))
            {
                double d = num_todouble(f.get());
                if (std::isfinite(d))
                {
                    angle = fmod(d, 4294967296.0);
                }
            }
            boxnode * us = boxnode_create(BNTYPE_ROT, 1);
            us->extra0 = -angle;
            us->extra1 = -angle;
            us->expr = etor(ecopychild(e,2));
            for (size_t i = 1; i <= 1; i++)
            {
                boxnode * child = _makebox_ex(echild(e,i));
                if (child == nullptr) {box_node_delete(us); return nullptr;}
                if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
                boxnode_append(us, bfrom_node(child));
            }
            return us;
        }
        else if (ehas_head_sym_length(e, gs.sym_sGridBox.get(), 1))
        {
            er f = echild(e,1);
            if (elength(f) > (1<<30)) {return nullptr;}
            if (elength(echild(f,1)) > (1<<30)) {return nullptr;}
            if (elength(f)*elength(echild(f,1)) > (1<<30)) {return nullptr;}
            boxnode * us = boxnode_create(BNTYPE_GRID, elength(f)*elength(echild(f,1)));
            for (size_t j=1; j<=elength(f); j++) {
                for (size_t i=1; i<=elength(echild(f,j)); i++) {
                    boxnode * child = _makebox_ex(echild(f,j,i));
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
                    boxnode * child = _makebox_ex(echild(f,j,i));
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
            boxnode * child = _makebox_ex(echild(e,1));
            if (child == nullptr) {box_node_delete(us); return nullptr;}
            if (boxnode_type(child) != BNTYPE_ROW) {box_node_delete(child); box_node_delete(us); return nullptr;}
            boxnode_append(us, bfrom_node(child));
            if (!eis_str(echild(e,2))) {box_node_delete(us); return nullptr;}
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
            // process options
            for (size_t i = 3; i <= elength(e); i++) {
                if (ehas_head_sym_length(echild(e,i), gs.sym_sRule.get(), 2)) {
                    if (eis_sym(echild(e,i,1), gs.sym_sCellLabel.get())) {
                        child = _makebox_ex(echild(e,i,2));
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
            return us;
        }
        else if (ehas_head_sym_length(e, gs.sym_sCellGroup.get(), 2))
        {
            er f = echild(e,1);
            if (!ehas_head_sym(f, gs.sym_sList.get()) || elength(f)<2) {return nullptr;}
            boxnode * us = boxnode_create(BNTYPE_CELLGROUP, elength(f));
            for (size_t i = 1; i <= elength(f); i++) {
                boxnode * child = _makebox_ex(echild(f,i));
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
        else
        {
            return nullptr;
        }
    }
}
#endif

bool notebook::set_ex(er e)
{
    return false;
#if 0
    if (!ehas_head_sym(e, gs.sym_sNotebook.get()))
        return false;
    if (elength(e)==0)
        return false;
    if (!ehas_head_sym(echild(e,1), gs.sym_sList.get()))
        return false;

    er f = echild(e,1);
    size_t n = elength(f);

    boxnode * newroot = boxnode_create(BNTYPE_ROOT, n + 1);
    for (size_t i = 1; i <= n; i++) {
        boxnode * cell = _makebox_ex(echild(f,i));
        if (cell == nullptr) {box_node_delete(newroot); return false;}
        if (boxnode_type(cell) != BNTYPE_CELL && boxnode_type(cell) != BNTYPE_CELLGROUP) {box_node_delete(cell); box_node_delete(newroot); return false;}
        boxnode_append(newroot, bfrom_node(cell));
    }
    boxnode_append(newroot, bfrom_ptr(&box_null));

    box_node_delete(root);
    root = newroot;

//    cursor1.clear();
//    cursor1.push_back(cursorentry(root, 0));
//    cursor1.push_back(cursorentry(root->array[0]));
    goto_first_cellbreak();

//    std::cout << "newbook: " << std::endl;
//    print();
    return true;
#endif
}


bool notebook::open(const char * fstr)
{
    unsigned char buffer[8];

    FILE * fp = fopen(fstr,"rb");
    if (!fp)
    {
        std::cout << "open failed" << std::endl;
        return false;
    }

    eparser P;
    size_t offset = 0;
    while (P.error == erNone && 1 == fread(buffer, 1, 1, fp))
    {
//        printf("%c", buffer[0]);
        P.handle_rawchar(buffer[0]);
        if (P.error)
        {
            std::cout << "syntax error at offset " << offset << std::endl;
            break;
        }
        offset++;
    }

    fclose(fp);

    if (P.error != erNone)
    {
        return false;
    }
    P.handle_end();
    if (!P.have_one_ex())
    {
        std::cout << "syntax error at end" << std::endl;
        return false;
    }
//std::cout << "notebook open successful: " << std::endl;
//std::cout << ex_tostring(P.estack[0].get()) << std::endl;
    if (!set_ex(P.estack[0].get()))
    {
        std::cout << "improper notebook format" << std::endl;
        return false;    
    }
/*
for (size_t i = 0; i < P.estack.size(); i++)
    std::cout << "P.estack[" <<i<< "]: " << ex_tostring(P.estack[i].get()) << std::endl;
std::cout << "P.pattern_name: " <<P.pattern_name.get() << ": "<< ex_tostring(P.pattern_name.get()) << std::endl;
std::cout << "P.blank_type: " <<P.blank_type.get() << ": "<< ex_tostring(P.blank_type.get()) << std::endl;
std::cout << "P.prevTk: " <<P.prevTk.get() << ": "<< ex_tostring(P.prevTk.get()) << std::endl;
std::cout << "P.currTk: " <<P.currTk.get() << ": "<< ex_tostring(P.currTk.get()) << std::endl;
std::cout << "P.nextTk: " <<P.nextTk.get() << ": "<< ex_tostring(P.nextTk.get()) << std::endl;
*/
    filestring.assign(fstr);
    return true;
}
