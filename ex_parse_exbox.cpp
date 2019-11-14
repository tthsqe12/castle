#include "types.h"
#include "ex.h"
#include "uex.h"
#include "ex_parse.h"
#include "globalstate.h"
#include "ex_print.h"
#include "ex_parse_boxes.h"
#include "sudcode.h"
#include "eval.h"


static void _push_end(std::vector<size_t>&istack, ex b)
{
    if (ehas_head_sym(b, gs.symsRowBox.get()))
    {
        istack.push_back(elength(b) - 1);
    }
    else
    {
        assert(false);
    }
}

static void _pop_end(std::vector<size_t>&istack, ex b)
{
    if (ehas_head_sym(b, gs.symsRowBox.get()))
    {
        istack.pop_back();
    }
    else
    {
        assert(false);
    }
}


/*
    return 0 for not a sum
           1 for a sum with just under
           2 for a sum with under and over
*/
int bnode_sumprod_argc(box b)
{
    if (bnode_type(b) == BNTYPE_UNDEROVER
                    && bnode_type(bnode_child(b,0)) == BNTYPE_ROW
                    && bnode_len(bnode_child(b,0)) == 2
                    && bis_char(bnode_child(bnode_child(b,0),0), CHAR_Sum, CHAR_Product))
    {
        return 2;
    }
    else if (bnode_type(b) == BNTYPE_UNDER
                  && bnode_type(bnode_child(b,0)) == BNTYPE_ROW
                  && bnode_len(bnode_child(b,0)) == 2
                  && bis_char(bnode_child(bnode_child(b,0),0), CHAR_Sum, CHAR_Product))
    {
        return 1;

    }
    else
    {
        return 0;
    }
}

/*
    return 0 for not a sum
           1 for a sum with just under
           2 for a sum with under and over
*/
int bnode_sumprod_argc(er b, Operator &op)
{
//std::cout << "bnode_sumprod_argc " << ex_tostring_full(b) << std::endl;
    int r = 0;
    op = op_none;
    if (ehas_head_sym_length(b, gs.symsUnderoverscriptBox.get(), 3))
    {
        r = 2;
    }
    else if (ehas_head_sym_length(b, gs.symsUnderscriptBox.get(), 2))
    {
        r = 1;
    }
    else
    {
        return 0;
    }
    er f = echild(b, 1);
    if (!eis_str(f))
    {
        if (!ehas_head_sym(f, gs.symsRowBox.get()) || elength(f) == 0)
        {
            return 0;
        }
        f = echild(f, 1);
        if (!ehas_head_sym_length(f, gs.symsList.get(), 1))
        {
            return 0;
        }
        f = echild(f, 1);
        if (!eis_str(f))
        {
            return 0;
        }
    }
    if (eto_str(f)->string == u8"\u2211")
    {
        op = op_pre_Sum;
        return r;
    }
    if (eto_str(f)->string == u8"\u220f")
    {
        op = op_pre_Product;
        return r;
    }
    return 0;
}



void _exbox_to_ex_handle_row(eparser &P, er b, int &error, std::vector<size_t> &iistack)
{
//std::cout << "_exbox_to_ex_handle_row " << ex_tostring_full(b) << std::endl;
    if (eis_str(b))
    {
        const unsigned char * s = reinterpret_cast<const unsigned char *>(eto_str(b)->string.c_str());
        size_t sn = eto_str(b)->string.size();
        size_t si = 0;
        while (si < sn)
        {
            char16_t c;
            si += readonechar16(c, s + si);
            P.handle_rawchar(c);
            if (P.error) {error = P.error; return;}
        }
        return;
    }

    if (   !ehas_head_sym(b, gs.symsRowBox.get())
        || elength(b) == 0
        || !ehas_head_sym(echild(b,1), gs.symsList.get()))
    {
        error = erBad; return;
    }
    b = echild(b,1);

//    istack.push_back(1);
    for (size_t i = 1; i <= elength(b); i++)
    {
        Operator op;
//        istack.back() = i;
        er child = echild(b,i);
        if (eis_str(child))
        {
            const unsigned char * s = reinterpret_cast<const unsigned char *>(eto_str(child)->string.c_str());
            size_t sn = eto_str(child)->string.size();
            size_t si = 0;
            while (si < sn)
            {
                char16_t c;
                si += readonechar16(c, s + si);
//std::cout << "got character: " << c << std::endl;
                if (c == CHAR_Integral && si >= sn
                                       && i + 1 <= elength(b)
                                       && ehas_head_sym_length(echild(b,i+1), gs.symsSubsuperscriptBox.get(), 2))
                {
                    P.handle_char(-1);
                    if (P.error) {error = P.error; return;}
                    ++i;
//                        istack.back() = i;
//                        istack.push_back(0);
                    uex e1(exbox_to_ex(echild(b,i,1), error, iistack));
                    if (error) {return;}
//                        istack.back() = 1;
                    uex e2(exbox_to_ex(echild(b,i,2), error, iistack));
                    if (error) {return;}
//                        istack.pop_back();
                    P.handle_token_ex(emake_raw(op_pre_Integrate, 0, emake_node(ecopy(gs.symsIntegrate.get()),
                                                                                ecopy(gs.symsNull.get()),
                                                                                emake_node(ecopy(gs.symsList.get()),
                                                                                           ecopy(gs.symsNull.get()),
                                                                                           e1.release(),
                                                                                           e2.release()
                                                                                )
                                                                      )));
                }
                else
                {
                    P.handle_rawchar(c);
                }
                if (P.error) {error = P.error; return;}
            }
        }
        else if (child == gs.symsNull.get())
        {
            P.handle_newline();
            if (P.error) {error = P.error; return;}
        }
        else if (ehas_head_sym_length(child, gs.symsSuperscriptBox.get(), 1))
        {
            P.handle_post_oper(op_post_SuperscriptBox);
            if (!P.have_top_ex()) {error = 1; return;}
            // parse the superscript
//            istack.push_back(0);
            uex f(exbox_to_ex(echild(child,1), error, iistack));
            if (error) {return;}
//            istack.pop_back();
            // make the power
            uex e(ecopy(P.estack.back().get()));
            P.estack.pop_back();
            P.estack.push_back(wex(emake_node(gs.symsPower.copy(), e.release(), f.release())));
        }
        else if (ehas_head_sym_length(child, gs.symsSubscriptBox.get(), 1))
        {
            P.handle_post_oper(op_post_SubscriptBox);
            if (!P.have_top_ex()) {error = 1; return;}
            // parse the subscript
//            istack.push_back(0);
            eparser Q;
            _exbox_to_ex_handle_row(Q, echild(child,1), error, iistack);
            if (Q.error) {error = Q.error; return;}
//            istack.push_back(bnode_len(bnode_child(child,0))-1);
            Q.handle_end();
            if (!Q.have_comma_sequence()) {error = 1; return;}
//            istack.pop_back();
//            istack.pop_back();
            // make the subscript
            size_t len = (Q.estack.size()+3)/2;
            uex e(gs.symsSubscript.get(), len);
            e.push_back(P.estack.back().copy());
            for (size_t k=2; k<=len; k++)
            {
                e.push_back(Q.estack[2*k-4].copy());
            }
            P.estack.pop_back();
            Q.estack.clear();
            P.estack.push_back(wex(e.release()));
        }
        else if (ehas_head_sym_length(child, gs.symsSubsuperscriptBox.get(), 2))
        {
//std::cout << "" << std::endl;

            P.handle_post_oper(op_post_SubsuperscriptBox);
            if (!P.have_top_ex()) {error = 1; return;}
            // parse the subscript
//            istack.push_back(0);
            eparser Q;
            _exbox_to_ex_handle_row(Q, echild(child,1), error, iistack);
            if (Q.error) {error = Q.error; return;}
//            istack.push_back(bnode_len(bnode_child(child,0))-1);
            Q.handle_end();
            if (!Q.have_comma_sequence()) {error = 1; return;}
//            istack.pop_back();
//            istack.pop_back();
            // make the subscript
            size_t len = (Q.estack.size()+3)/2;
            uex e(gs.symsSubscript.get(), len);
            e.push_back(P.estack.back().copy());
            for (size_t k=2; k<=len; k++) {
                e.push_back(Q.estack[2*k-4].copy());
            }
            P.estack.pop_back();
            Q.estack.clear();
            // parse the superscript
//            istack.push_back(1);
            uex f(exbox_to_ex(echild(child,2), error, iistack));
            if (error) {return;}
//            istack.pop_back();
            // make the superscript
            P.estack.push_back(wex(emake_node(gs.symsPower.copy(), e.release(), f.release())));
        }
        else if (bnode_sumprod_argc(child, op) == 2)
        {
//std::cout << "in bnode_sumprod_argc = 2 case" << std::endl;
//            Operator op = eto_str(echild(child,1,1,1))->string == u8"\u2211" ? op_pre_Sum : op_pre_Product;
//            istack.push_back(1);
            uex e1(exbox_to_ex(echild(child,2), error, iistack));
            if (error) {return;}
//            istack.back() = 2;
            uex e2(exbox_to_ex(echild(child,3), error, iistack));
            if (error) {return;}
//            istack.pop_back();
            P.handle_char(-1);
            if (P.error) {error = P.error; return;}
            ex l;
            if (ehas_head_sym_length(e1.get(), gs.symsSet.get(), 2))
            {
                l = emake_node(ecopy(gs.symsList.get()), ecopychild(e1.get(),1), ecopychild(e1.get(),2), e2.release());
            } else {
                l = emake_node(ecopy(gs.symsList.get()), e1.release(), e2.release());
            }
            P.handle_token_ex(emake_raw(op,0,emake_node(ecopy(op == op_pre_Sum ? gs.symsSum.get() : gs.symsProduct.get()), l)));
            if (P.error) {error = P.error; return;}
        }
        else
        {
            uex e(exbox_to_ex(child, error, iistack));
            if (error) {return;}
            P.handle_ex(e.release());
            if (P.error) {error = P.error; return;}
        }
    }
//    istack.pop_back();
}

ex exbox_to_ex(er b, int &error, std::vector<size_t> &istack)
{
//std::cout << "exbox_to_ex " << ex_tostring_full(b) << std::endl;

    if (eis_str(b) || ehas_head_sym(b, gs.symsRowBox.get()))
    {
        eparser P;
        _exbox_to_ex_handle_row(P, b, error, istack);
        if (error) {return ecopy(gs.syms$Failed.get());}
//        _push_end(istack, b);
        P.handle_end();
        if (!P.have_one_ex()) {error = 1; return ecopy(gs.syms$Failed.get());}
//        _pop_end(istack, b);
        return ecopy(P.estack[0].get());
    }
    else if (ehas_head_sym_length(b, gs.symsSqrtBox.get(), 1))
    {
//        istack.push_back(0);
        uex e(exbox_to_ex(echild(b,1), error, istack));
        if (error) {return ecopy(gs.syms$Failed.get());}
//        istack.pop_back();
        return emake_node(ecopy(gs.symsSqrt.get()), e.release());
    }
    else if (ehas_head_sym_length(b, gs.symsRotationBox.get(), 4))
    {
        if (!eis_int(echild(b,3)) || !eis_int(echild(b,4))) {error = erBad; return ecopy(gs.syms$Failed.get());}

//        istack.push_back(0);
        uex e(exbox_to_ex(echild(b,1), error, istack));
        if (error) {return ecopy(gs.syms$Failed.get());}
//        istack.pop_back();

        uex r(emake_rat());
        fmpz_one(fmpq_denref(erat_data(r.get())));
        fmpz_mul_2exp(fmpq_denref(erat_data(r.get())), fmpq_denref(erat_data(r.get())), 31);
        fmpz_sub(fmpq_numref(erat_data(r.get())), eint_data(echild(b,4)), eint_data(echild(b,3)));
        fmpq_canonicalise(erat_data(r.get()));
        ex f = ereturn_rat(r.release());
        f = emake_node(ecopy(gs.symsTimes.get()), f, ecopy(gs.symsPi.get()));
        f = emake_node(ecopy(gs.symsPlus.get()), ecopy(echild(b,2)), f);
        return emake_node(ecopy(gs.symsRotate.get()), e.release(), f);
    }
    else if (ehas_head_sym_length(b, gs.symsFractionBox.get(), 2))
    {
//        istack.push_back(0);
        uex e1(exbox_to_ex(echild(b,1), error, istack));
        if (error) {return ecopy(gs.syms$Failed.get());}
//        istack.back() = 1;
        uex e2(exbox_to_ex(echild(b,2), error, istack));
        if (error) {return ecopy(gs.syms$Failed.get());}
//        istack.pop_back();
        return emake_node(ecopy(gs.symsDivide.get()), e1.release(), e2.release());
    }
    else if (ehas_head_sym_length(b, gs.symsUnderscriptBox.get(), 2))
    {
        // parse the body
//        istack.push_back(0);
        uex f(exbox_to_ex(echild(b,1), error, istack));
        if (error) {return ecopy(gs.syms$Failed.get());}
//        istack.pop_back();
        // parse the underscript
//        istack.push_back(1);
        eparser Q;
        _exbox_to_ex_handle_row(Q, echild(b,2), error, istack);
        if (error) {return ecopy(gs.syms$Failed.get());}
//        _push_end(istack, bnode_child(b,1));
        Q.handle_end();
        if (!Q.have_comma_sequence()) {error = 1; return ecopy(gs.syms$Failed.get());}
//        _pop_end(istack, bnode_child(b,1));
//        istack.pop_back();
        // make the underscript
        size_t len = (Q.estack.size()+3)/2;
        uex e(gs.symsUnderscript.get(), len);
        e.push_back(f.release());
        for (size_t k=2; k<=len; k++)
        {
            e.push_back(Q.estack[2*k-4].copy());
        }
        return e.release();
    }
    else if (ehas_head_sym_length(b, gs.symsOverscriptBox.get(), 2))
    {
        // parse the body
//        istack.push_back(0);
        uex f(exbox_to_ex(echild(b,1), error, istack));
        if (error) {return ecopy(gs.syms$Failed.get());}
//        istack.pop_back();
        // parse the overscript
//        istack.push_back(1);
        eparser Q;
        _exbox_to_ex_handle_row(Q, echild(b,2), error, istack);
        if (error) {return ecopy(gs.syms$Failed.get());}
//        _push_end(istack, bnode_child(b,1));
        Q.handle_end();
        if (!Q.have_comma_sequence()) {error = 1; return ecopy(gs.syms$Failed.get());}
//        _pop_end(istack, bnode_child(b,1));
//        istack.pop_back();
        // make the underscript
        size_t len = (Q.estack.size()+3)/2;
        uex e(gs.symsOverscript.get(), len);
        e.push_back(f.release());
        for (size_t k=2; k<=len; k++) {
            e.push_back(Q.estack[2*k-4].copy());
        }
        return e.release();
    }
    else if (ehas_head_sym_length(b, gs.symsUnderoverscriptBox.get(), 3))
    {
//        istack.push_back(0);
        uex e0(exbox_to_ex(echild(b,1), error, istack));
        if (error) {return ecopy(gs.syms$Failed.get());}
//        istack.back() = 1;
        uex e1(exbox_to_ex(echild(b,2), error, istack));
        if (error) {return ecopy(gs.syms$Failed.get());}
//        istack.back() = 2;
        uex e2(exbox_to_ex(echild(b,3), error, istack));
        if (error) {return ecopy(gs.syms$Failed.get());}
//        istack.pop_back();
        return emake_node(ecopy(gs.symsUnderoverscript.get()), e0.release(), e1.release(), e2.release());
    }
    else if (ehas_head_sym_length(b, gs.symsGridBox.get(), 1))
    {
        er m = echild(b,1);
        if (!eis_matrix(m)) {error = erBad; return gs.syms$Failed.copy();}
        size_t nrows = elength(m);
        size_t ncols = elength(echild(m,1));

        std::vector<uex> matrix;
//        istack.push_back(0);
        for (size_t j = 1; j <= nrows; j++)
        {
            std::vector<uex> row;
            for (size_t i = 1; i <= ncols; i++)
            {
//                istack.back() = bnode_extra0(b)*j+i;
                uex e(exbox_to_ex(echild(m,j,i), error, istack));
                if (error) {return ecopy(gs.syms$Failed.get());}
                row.push_back(std::move(e));
            }
            matrix.push_back(uex(emake_node(ecopy(gs.symsList.get()), row)));
        }
//        istack.pop_back();
        return emake_node(ecopy(gs.symsList.get()), matrix);
    }
    else if (ehas_head_sym_length(b, gs.symsColumnBox.get(), 1))
    {
        b = echild(b,1);
        if (!ehas_head_sym(b, gs.symsList.get()) || elength(b) == 0) {error = erBad; return ecopy(gs.syms$Failed.get());}
        std::vector<uex> list;
//        istack.push_back(0);
        for (size_t j = 1; j <= elength(b); j++)
        {
//            istack.back() = j;
            uex e(exbox_to_ex(echild(b,j), error, istack));
            if (error) {return ecopy(gs.syms$Failed.get());}
            list.push_back(std::move(e));
        }
//        istack.pop_back();
        return emake_node(ecopy(gs.symsColumn.get()), emake_node(ecopy(gs.symsList.get()), list));
    }
    else
    {
        error = erBad; return ecopy(gs.syms$Failed.get());
    }
}

void ex_parse_exbox(std::vector<uex> &ans, er b, bool toplevel, int &error, std::vector<size_t> &istack)
{
//std::cout << "ex_parse_exbox " << ex_tostring_full(b) << std::endl;

    ans.clear();
    if (eis_str(b) || ehas_head_sym(b, gs.symsRowBox.get()))
    {
        eparser P;
        P.toplevel = toplevel;
        _exbox_to_ex_handle_row(P, b, error, istack);
        if (error) {return;}
//        _push_end(istack, b);

        P.handle_end();
        if (P.have_one_ex() || (P.have_no_ex() && !P.out.empty()))
        {
            if (P.have_one_ex())
            {
                P.out.push_back(uex(P.estack[0].copy()));
            }
            std::swap(ans, P.out);
        }
        else
        {
            error = 1; return;
        }
    }
    else
    {
        error = 1; return;
    }
}














void _exbox_to_ex_handle_rows(eparser &P, er b, syntax_report & sr)
{
//std::cout << "_exbox_to_ex_handle_row " << ex_tostring_full(b) << std::endl;
    if (sr.error)
    {
        return;
    }

    if (eis_str(b))
    {
        const unsigned char * s = reinterpret_cast<const unsigned char *>(eto_str(b)->string.c_str());
        size_t sn = eto_str(b)->string.size();
        size_t si = 0;
        while (si < sn)
        {
            char16_t c;
            si += readonechar16(c, s + si);
            P.handle_rawchar(c);
            if (P.error)
            {
                sr.handle_string_error(P.error, b, si);
                return;
            }
        }
        return;
    }

    if (   !ehas_head_sym(b, gs.symsRowBox.get())
        || elength(b) == 0
        || !ehas_head_sym(echild(b,1), gs.symsList.get()))
    {
        sr.error = erBad;
        sr.around.reset(nullptr);
        return;
    }
    b = echild(b,1);

    for (size_t i = 1; i <= elength(b); i++)
    {
        Operator op;
        er child = echild(b,i);
        if (eis_str(child))
        {
            const unsigned char * s = reinterpret_cast<const unsigned char *>(eto_str(child)->string.c_str());
            size_t sn = eto_str(child)->string.size();
            size_t si = 0;
            while (si < sn)
            {
                char16_t c;
                si += readonechar16(c, s + si);
                if (c == CHAR_Integral && si >= sn
                                       && i + 1 <= elength(b)
                                       && ehas_head_sym_length(echild(b,i+1), gs.symsSubsuperscriptBox.get(), 2))
                {
                    P.handle_char(CHAR_NONE);
                    if (P.error)
                    {
                        sr.handle_row_error(P.error, b, i, si);
                        return;
                    }
                    ++i;
                    uex e1(exbox_to_exs(echild(b,i,1), sr));
                    if (sr.error)
                    {
                        sr.handle_row_error(erNone, b, i);
                        return;
                    }
                    uex e2(exbox_to_exs(echild(b,i,2), sr));
                    if (sr.error)
                    {
                        sr.handle_row_error(erNone, b, i);
                        return;
                    }
                    ex t = emake_node(gs.symsList.copy(), gs.symsNull.copy(), e1.release(), e2.release());
                    t = emake_node(gs.symsIntegrate.copy(), gs.symsNull.copy(), t);
                    P.handle_token_ex(emake_raw(op_pre_Integrate, 0, t));
                }
                else
                {
                    P.handle_rawchar(c);
                }
                if (P.error)
                {
                    sr.handle_row_error(P.error, b, i, si);
                    return;
                }
            }
        }
        else if (child == gs.symsNull.get())
        {
            P.handle_newline();
            if (P.error)
            {
                sr.handle_row_error(P.error, b, i);
                return;
            }
        }
        else if (ehas_head_sym_length(child, gs.symsSuperscriptBox.get(), 1))
        {
            P.handle_post_oper(op_post_SuperscriptBox);
            if (!P.have_top_ex())
            {
                sr.handle_row_error(erBad, b, i);
                return;
            }
            // parse the superscript
            uex f(exbox_to_exs(echild(child,1), sr));
            if (P.error)
            {
                sr.handle_row_error(P.error, b, i);
                return;
            }
            // make the power
            uex e(P.estack.back().copy());
            P.estack.pop_back();
            P.estack.push_back(wex(emake_node(gs.symsPower.copy(), e.release(), f.release())));
        }
        else if (ehas_head_sym_length(child, gs.symsSubscriptBox.get(), 1))
        {
            P.handle_post_oper(op_post_SubscriptBox);
            if (!P.have_top_ex())
            {
                sr.handle_row_error(P.error ? P.error : erBad, b, i);
                return;
            }
            // parse the subscript
            eparser Q;
            _exbox_to_ex_handle_rows(Q, echild(child,1), sr);
            if (Q.error)
            {
                sr.handle_row_error(Q.error, b, i);
                return;
            }
            Q.handle_end();
            if (!Q.have_comma_sequence())
            {
                sr.handle_row_error(Q.error ? Q.error : erBad, b, i);
                return;
            }
            // make the subscript
            size_t len = (Q.estack.size()+3)/2;
            uex e(gs.symsSubscript.get(), len);
            e.push_back(P.estack.back().copy());
            for (size_t k=2; k<=len; k++)
            {
                e.push_back(Q.estack[2*k-4].copy());
            }
            P.estack.pop_back();
            Q.estack.clear();
            P.estack.push_back(wex(e.release()));
        }
        else if (ehas_head_sym_length(child, gs.symsSubsuperscriptBox.get(), 2))
        {
//std::cout << "" << std::endl;

            P.handle_post_oper(op_post_SubsuperscriptBox);
            if (!P.have_top_ex())
            {
                sr.handle_row_error(erBad, b, i);
                return;
            }
            // parse the subscript
            eparser Q;
            _exbox_to_ex_handle_rows(Q, echild(child,1), sr);
            if (Q.error)
            {
                sr.handle_row_error(Q.error, b, i);
                return;
            }
            Q.handle_end();
            if (!Q.have_comma_sequence())
            {
                sr.handle_row_error(erBad, b, i);
                return;
            }
            // make the subscript
            size_t len = (Q.estack.size()+3)/2;
            uex e(gs.symsSubscript.get(), len);
            e.push_back(P.estack.back().copy());
            for (size_t k=2; k<=len; k++) {
                e.push_back(Q.estack[2*k-4].copy());
            }
            P.estack.pop_back();
            Q.estack.clear();
            // parse the superscript
            uex f(exbox_to_exs(echild(child,2), sr));
            if (sr.error)
            {
                sr.handle_row_error(erBad, b, i);
                return;
            }
            // make the superscript
            P.estack.push_back(wex(emake_node(gs.symsPower.copy(), e.release(), f.release())));
        }
        else if (bnode_sumprod_argc(child, op) == 2)
        {
            uex e1(exbox_to_exs(echild(child,2), sr));
            if (sr.error)
            {
                sr.handle_row_error(erNone, b, i);
                return;
            }
            uex e2(exbox_to_exs(echild(child,3), sr));
            if (sr.error)
            {
                sr.handle_row_error(erNone, b, i);
                return;
            }
            P.handle_char(CHAR_NONE);
            if (P.error)
            {
                sr.handle_row_error(P.error, b, i);
                return;
            }
            ex l;
            if (ehas_head_sym_length(e1.get(), gs.symsSet.get(), 2))
            {
                l = emake_node(gs.symsList.copy(), ecopychild(e1.get(),1), ecopychild(e1.get(),2), e2.release());
            } else {
                l = emake_node(gs.symsList.copy(), e1.release(), e2.release());
            }
            l = emake_node(ecopy(op == op_pre_Sum ? gs.symsSum.get() : gs.symsProduct.get()), l);
            P.handle_token_ex(emake_raw(op,0,l));
            if (P.error)
            {
                sr.handle_row_error(P.error, b, i);
                return;
            }
        }
        else
        {
            uex e(exbox_to_exs(child, sr));
            if (sr.error)
            {
                sr.handle_row_error(erNone, b, i);
                return;
            }
            P.handle_ex(e.release());
            if (P.error)
            {
                sr.handle_row_error(P.error, b, i);
                return;
            }
        }
    }
}

ex exbox_to_exs(er b, syntax_report & sr)
{
//std::cout << "exbox_to_ex " << ex_tostring_full(b) << std::endl;

    if (eis_str(b) || ehas_head_sym(b, gs.symsRowBox.get()))
    {
        eparser P;
        _exbox_to_ex_handle_rows(P, b, sr);
        if (sr.error) {return gs.syms$Failed.copy();}
        P.handle_end();
        if (!P.have_one_ex())
        {
            sr.handle_row_end_error(b);
            return gs.syms$Failed.copy();
        }
        return P.estack[0].copy();
    }
    else if (ehas_head_sym_length(b, gs.symsSqrtBox.get(), 1))
    {
        uex e(exbox_to_exs(echild(b,1), sr));
        if (sr.error) {return gs.syms$Failed.copy();}
        return emake_node(gs.symsSqrt.copy(), e.release());
    }
    else if (ehas_head_sym_length(b, gs.symsRotationBox.get(), 4))
    {
        if (!eis_int(echild(b,3)) || !eis_int(echild(b,4)))
        {
            sr.error = erBad;
            return gs.syms$Failed.copy();
        }
        uex e(exbox_to_exs(echild(b,1), sr));
        if (sr.error) {return gs.syms$Failed.copy();}
        uex r(emake_rat());
        fmpz_one(fmpq_denref(erat_data(r.get())));
        fmpz_mul_2exp(fmpq_denref(erat_data(r.get())), fmpq_denref(erat_data(r.get())), 31);
        fmpz_sub(fmpq_numref(erat_data(r.get())), eint_data(echild(b,4)), eint_data(echild(b,3)));
        fmpq_canonicalise(erat_data(r.get()));
        ex f = ereturn_rat(r.release());
        f = emake_node(gs.symsTimes.copy(), f, gs.symsPi.copy());
        f = emake_node(gs.symsPlus.copy(), ecopychild(b,2), f);
        return emake_node(gs.symsRotate.copy(), e.release(), f);
    }
    else if (ehas_head_sym_length(b, gs.symsFractionBox.get(), 2))
    {
        uex e1(exbox_to_exs(echild(b,1), sr));
        if (sr.error) {return gs.syms$Failed.copy();}
        uex e2(exbox_to_exs(echild(b,2), sr));
        if (sr.error) {return gs.syms$Failed.copy();}
        return emake_node(gs.symsDivide.copy(), e1.release(), e2.release());
    }
    else if (ehas_head_sym_length(b, gs.symsUnderscriptBox.get(), 2))
    {
        // parse the body
        uex f(exbox_to_exs(echild(b,1), sr));
        if (sr.error) {return gs.syms$Failed.copy();}
        // parse the underscript
        eparser Q;
        _exbox_to_ex_handle_rows(Q, echild(b,2), sr);
        if (sr.error) {return gs.syms$Failed.copy();}
        Q.handle_end();
        if (!Q.have_comma_sequence())
        {
            sr.error = erBad;
            return gs.syms$Failed.copy();
        }
        // make the underscript
        size_t len = (Q.estack.size()+3)/2;
        uex e(gs.symsUnderscript.get(), len);
        e.push_back(f.release());
        for (size_t k=2; k<=len; k++)
        {
            e.push_back(Q.estack[2*k-4].copy());
        }
        return e.release();
    }
    else if (ehas_head_sym_length(b, gs.symsOverscriptBox.get(), 2))
    {
        // parse the body
        uex f(exbox_to_exs(echild(b,1), sr));
        if (sr.error) {return gs.syms$Failed.copy();}
        // parse the overscript
        eparser Q;
        _exbox_to_ex_handle_rows(Q, echild(b,2), sr);
        if (sr.error) {return gs.syms$Failed.copy();}
        Q.handle_end();
        if (!Q.have_comma_sequence())
        {
            sr.error = erBad;
            return gs.syms$Failed.copy();
        }
        // make the underscript
        size_t len = (Q.estack.size()+3)/2;
        uex e(gs.symsOverscript.get(), len);
        e.push_back(f.release());
        for (size_t k=2; k<=len; k++) {
            e.push_back(Q.estack[2*k-4].copy());
        }
        return e.release();
    }
    else if (ehas_head_sym_length(b, gs.symsUnderoverscriptBox.get(), 3))
    {
        uex e0(exbox_to_exs(echild(b,1), sr));
        if (sr.error) {return gs.syms$Failed.copy();}
        uex e1(exbox_to_exs(echild(b,2), sr));
        if (sr.error) {return gs.syms$Failed.copy();}
        uex e2(exbox_to_exs(echild(b,3), sr));
        if (sr.error) {return gs.syms$Failed.copy();}
        return emake_node(gs.symsUnderoverscript.copy(), e0.release(), e1.release(), e2.release());
    }
    else if (ehas_head_sym_length(b, gs.symsGridBox.get(), 1))
    {
        er m = echild(b,1);
        if (!eis_matrix(m))
        {
            sr.error = erBad;
            return gs.syms$Failed.copy();
        }
        size_t nrows = elength(m);
        size_t ncols = elength(echild(m,1));
        std::vector<uex> matrix;
        for (size_t j = 1; j <= nrows; j++)
        {
            std::vector<uex> row;
            for (size_t i = 1; i <= ncols; i++)
            {
                uex e(exbox_to_exs(echild(m,j,i), sr));
                if (sr.error) {return gs.syms$Failed.copy();}
                row.push_back(std::move(e));
            }
            matrix.push_back(uex(emake_node(gs.symsList.copy(), row)));
        }
        return emake_node(gs.symsList.copy(), matrix);
    }
    else
    {
        sr.error = erBad;
        return gs.syms$Failed.copy();
    }
}

void ex_parse_exboxs(std::vector<uex> &ans, er b, bool toplevel, syntax_report & sr)
{
//std::cout << "ex_parse_exbox " << ex_tostring_full(b) << std::endl;
    ans.clear();
    if (eis_str(b) || ehas_head_sym(b, gs.symsRowBox.get()))
    {
        install_live_psymbols LPS;
        eparser P(toplevel);
        _exbox_to_ex_handle_rows(P, b, sr);
        if (sr.error)
        {
            sr.around.reset(gs.symsPi.copy());
            return;
        }
        P.handle_end();
        if (P.have_one_ex() || (P.have_no_ex() && !P.out.empty()))
        {
            if (P.have_one_ex())
            {
                P.out.push_back(uex(P.estack[0].copy()));
            }
            std::swap(ans, P.out);
            gs.send_psymbols();
            return;
        }
        else
        {
            sr.error = erUnexpectedEnd;
            if (eis_str(b))
            {
                sr.around.reset(ecopy(b));
            }
            else
            {
                sr.around.reset(gs.symsPi.copy());
            }
            return;
        }
    }
    else
    {
        sr.error = erBad;
        sr.around.reset(gs.symsPi.copy());
        return;
    }
}
