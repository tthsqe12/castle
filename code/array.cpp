#include "globalstate.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"
#include "ex_cont.h"

ex dcode_sConstantArray(er e)
{
//std::cout << "dcode_sConstantArray: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sConstantArray.get()));

    if (elength(e) != 2)
        return _handle_message_argx(e, 2);

    er e2 = echild(e,2);
    parray_dims dims;
    if (eis_intnm(e2))
    {
        dims.set_rank(1);
        dims.set_index(0, eintm_get(e2));
    }
    else if (ehas_head_sym(e2, gs.sym_sList.get()))
    {
        ulong r = elength(e2);
        if (r == 0)
            return _handle_message_ilsmn(e, 2);
        dims.set_rank(r);
        for (ulong i = 0; i < r; i++)
        {
            er e2i = echild(e2,i+1);
            if (eis_intnm(e2i))
                dims.set_index(i, eintm_get(e2i));
            else
                return _handle_message_ilsmn(e, 2);
        }
    }
    else if (eis_parray(e2))
    {
        if (eparray_rank(e2) != 1 || eparray_type(e2) != 0)
            return _handle_message_ilsmn(e, 2);

        ulong r = eparray_dimension(e2, 0);
        assert(r > 0);
        dims.set_rank(r);
        fmpz * d = eparray_int_data(e2);
        for (ulong i = 0; i < r; i++)
        {
            if (fmpz_fits_si(d + i) && fmpz_sgn(d + i) > 0)
                dims.set_index(i, fmpz_get_si(d + i));
            else
                return _handle_message_ilsmn(e, 2);
        }       
    }
    else
    {
        return _handle_message_ilsmn(e, 2);
    }

    er e1 = echild(e,1);

    ulong size;
    if (dims.rank <= 3 && dims.size_fits_ui() && (size = dims.size()) > 0)
    {
        if (eis_int(e1))
        {
            if (!COEFF_IS_MPZ(*eint_data(e1)))
            {
                uex r(emake_parray(0, dims, size));
                fmpz * d = eparray_int_data(r.get());
                for (ulong i = 0; i < size; i++)
                    d[i] = *eint_data(e1);
                return r.release();
            }
        }
        else if (eis_rat(e1))
        {
            if (   !COEFF_IS_MPZ(*fmpq_numref(erat_data(e1)))
                && !COEFF_IS_MPZ(*fmpq_denref(erat_data(e1))))
            {
                uex r(emake_parray(1, dims, size));
                fmpq * d = eparray_rat_data(r.get());
                for (ulong i = 0; i < size; i++)
                    d[i] = *erat_data(e1);
                return r.release();
            }
        }
        else if (eis_double(e1))
        {
            uex r(emake_parray(2, dims, size));
            double * d = eparray_double_data(r.get());
            for (ulong i = 0; i < size; i++)
                d[i] = edouble_number(e1);
            return r.release();
        }
    }

    uex r(ecopy(e1));
    for (ulong i = dims.rank; i > 0; i--)
    {
        ulong m = dims.get_index(i - 1);
        uex newr(gs.sym_sList.get(), m);
        for (ulong j = 0; j < m; j++)
            newr.push_back(r.copy());
        r.setnz(newr.release());
    }
    return r.release();
}


static ex _do_append(er a, er b, er e)
{
    if (eis_node(a))
    {
        size_t n = elength(a);
        uex r(echild(a, 0), n + 1);
        for (size_t i = 1; i <= n; i++)
            r.push_back(ecopychild(a,i));
        r.push_back(ecopy(b));
        return r.release();
    }
    else if (eis_parray(a))
    {
        slong rank = eparray_rank(a);
        if (rank > 1)
        {
            if (eis_parray(b) && rank - 1 == eparray_rank(b))
            {
                bool dim_ok = true;
                for (ulong i = 0; i + 1 < rank; i++)
                {
                    if (eparray_dimension(a, i + 1) != eparray_dimension(b, i))
                        dim_ok = false;
                }
                if (dim_ok)
                {
                    ulong bsize = eto_parray(b)->dimensions.size();
                    ulong asize = eto_parray(a)->dimensions.size();
                    ulong newsize;
                    if (!UADD(newsize, asize, bsize))
                    {
                        parray_dims newdims(eto_parray(a)->dimensions);
                        newdims.index(0)++;
                        int atyp = eparray_type(a);
                        int btyp = eparray_type(b);
                        if (atyp == 0)
                        {
                            fmpz * ad = eparray_int_data(a);
                            if (btyp == 0)
                            {
                                fmpz * bd = eparray_int_data(b);
                                uex r(emake_parray(0, newdims, newsize));
                                fmpz * R = eparray_int_data(r.get());
                                for (slong i = 0; i < asize; i++)
                                    fmpz_set(R + i, ad + i);
                                for (slong i = 0; i < bsize; i++)
                                    fmpz_set(R + asize + i, bd + i);
                                return r.release();
                            }
                            else if (btyp == 1)
                            {
                                fmpq * bd = eparray_rat_data(b);
                                uex r(emake_parray(1, newdims, newsize));
                                fmpq * R = eparray_rat_data(r.get());
                                for (slong i = 0; i < asize; i++)
                                    fmpz_set(fmpq_numref(R + i), ad + i);
                                for (slong i = 0; i < bsize; i++)
                                    fmpq_set(R + asize + i, bd + i);
                                return r.release();
                            }
                        }
                        else if (atyp == 1)
                        {
                            fmpq * ad = eparray_rat_data(a);
                            if (btyp == 0)
                            {
                                fmpz * bd = eparray_int_data(b);
                                uex r(emake_parray(1, newdims, newsize));
                                fmpq * R = eparray_rat_data(r.get());
                                for (slong i = 0; i < asize; i++)
                                    fmpq_set(R + i, ad + i);
                                for (slong i = 0; i < bsize; i++)
                                    fmpz_set(fmpq_numref(R + asize + i), bd + i);
                                return r.release();
                            }
                            else if (btyp == 1)
                            {
                                fmpq * bd = eparray_rat_data(b);
                                uex r(emake_parray(1, newdims, newsize));
                                fmpq * R = eparray_rat_data(r.get());
                                for (slong i = 0; i < asize; i++)
                                    fmpq_set(R + i, ad + i);
                                for (slong i = 0; i < bsize; i++)
                                    fmpq_set(R + asize + i, bd + i);
                                return r.release();
                            }
                        }
                        else if (atyp == 2)
                        {
                            double * ad = eparray_double_data(a);
                            if (btyp == 2)
                            {
                                double * bd = eparray_double_data(b);
                                uex r(emake_parray(2, newdims, newsize));
                                double * R = eparray_double_data(r.get());
                                for (slong i = 0; i < asize; i++)
                                    R[i] = ad[i];
                                for (slong i = 0; i < bsize; i++)
                                    R[asize + i] = bd[i];
                                return r.release();
                            }
                        }
                        else
                        {
                            assert(atyp == 3);
                        }
                    }
                }
            }
        }
        else
        {
            slong n = eparray_dimension(a, 0);
            int atyp = eparray_type(a);
            if (atyp == 0)
            {
                if (eis_int(b))
                {
                    uex r(emake_parray_rank1(0, n + 1));
                    fmpz * rd = eparray_int_data(r.get());
                    fmpz * ad = eparray_int_data(a);
                    fmpz_set(rd + n, eint_data(b));
                    for (slong i = 0; i < n; i++)
                        fmpz_set(rd + i, ad + i);
                    return r.release();
                }
                else if (eis_rat(b))
                {
                    uex r(emake_parray_rank1(1, n + 1));
                    fmpq * rd = eparray_rat_data(r.get());
                    fmpz * ad = eparray_int_data(a);
                    fmpq_set(rd + n, erat_data(b));
                    for (slong i = 0; i < n; i++)
                        fmpz_set(fmpq_numref(rd + i), ad + i);
                    return r.release();
                }
            }
            else if (atyp == 1)
            {
                if (eis_int(b) || eis_rat(b))
                {
                    uex r(emake_parray_rank1(1, n + 1));
                    fmpq * rd = eparray_rat_data(r.get());
                    fmpq * ad = eparray_rat_data(a);
                    if (eis_int(b))
                        fmpz_set(fmpq_numref(rd + n), eint_data(b));
                    else
                        fmpq_set(rd + n, erat_data(b));
                    for (slong i = 0; i < n; i++)
                        fmpq_set(rd + i, ad + i);
                    return r.release();
                }
            }
            else if (atyp == 2)
            {
                if (eis_double(b))
                {
                    uex r(emake_parray_rank1(2, n + 1));
                    double * rd = eparray_double_data(r.get());
                    double * ad = eparray_double_data(a);
                    rd[n] = edouble_number(b);
                    for (slong i = 0; i < n; i++)
                        rd[i] = ad[i];
                    return r.release();
                }
            }
            else
            {
                assert(atyp == 3);
            }
        }

        er an = eparray_get_normal(a);
        size_t n = elength(an);
        uex r(echild(a, 0), n + 1);
        for (size_t i = 1; i <= n; i++)
            r.push_back(ecopychild(an,i));
        r.push_back(ecopy(b));
        return r.release();        
    }
    else if (eis_hmap(a))
    {
        uex r(emake_hmap());
        ehmap_data(r.get()) = ehmap_data(a);
        if (ehas_head_sym(b, gs.sym_sList.get()))
        {
            for (slong i = 1; i <= elength(b); i++)
            {
                er bi = echild(e,i);
                if (ehas_head_sym_length(bi, gs.sym_sRule.get(), 2))
                {
                    ehmap_assign(r.get(), ecopychild(bi,1), ecopychild(bi,2), false);
                }
                else if (ehas_head_sym_length(bi, gs.sym_sRuleDelayed.get(), 2))
                {
                    ehmap_assign(r.get(), ecopychild(bi,1), ecopychild(bi,2), true);
                }
                else
                {
                    _gen_message(esymbolic_head(e), "invdt", "The argument `1` is not a rule or a list of rules.", emake_cint(1), ecopy(b));
                    return ecopy(e);
                }
            }
        }
        else if (ehas_head_sym_length(b, gs.sym_sRule.get(), 2))
        {
            ehmap_assign(r.get(), ecopychild(b,1), ecopychild(b,2), false);
        }
        else if (ehas_head_sym_length(b, gs.sym_sRuleDelayed.get(), 2))
        {
            ehmap_assign(r.get(), ecopychild(b,1), ecopychild(b,2), true);
        }
        else
        {
            _gen_message(esymbolic_head(e), "invdt", "The argument `1` is not a rule or a list of rules.", emake_cint(1), ecopy(b));
            return ecopy(e);
        }
        return r.release();
    }
    else
    {
        _gen_message(esymbolic_head(e), "normal", NULL, emake_cint(1), ecopy(e));
        return ecopy(e);
    }
}


ex scode_sAppend(er e)
{
//std::cout << "scode_sAssociation: " << ex_tostring_full(e) << std::endl;
    assert(eis_node(e));

    er h = echild(e,0);
    if (!ehas_head_sym_length(h, gs.sym_sList.get(), 1))
        return ecopy(e);

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    return _do_append(echild(h,1), echild(e,1), e);
}

ex dcode_sAppend(er e)
{
    assert(ehas_head_sym(e, gs.sym_sAppend.get()));

    if (elength(e) == 2)
    {
        return _do_append(echild(e,1), echild(e,2), e);
    }
    else if (elength(e) == 1)
    {
        return ecopy(e);
    }
    else
    {
        return _handle_message_argt(e, (1 << 0) + (2 << 8));
    }
}