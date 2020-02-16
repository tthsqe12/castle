/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "factor/nmod_mpoly_factor.h"
#include <string.h>

/* comparison is lex on fields */
int mpoly_monomial_cmp_sort(
    const ulong * Aexps, flint_bitcnt_t Abits,
    const ulong * Bexps, flint_bitcnt_t Bbits,
    slong length, const mpoly_ctx_t mctx)
{
    slong i, j;

    if (Abits == Bbits)
    {
        slong N = mpoly_words_per_exp(Abits, mctx);
        for (i = 0; i < length; i++)
        {
            for (j = N - 1; j >= 0; j--)
            {
                if (Aexps[N*i + j] != Bexps[N*i + j])
                {
                    return Aexps[N*i + j] < Bexps[N*i + j] ? -1 : 1;
                }
            }
        }
    }
    else
    {
        slong NA = mpoly_words_per_exp_sp(Abits, mctx);
        slong NB = mpoly_words_per_exp_sp(Bbits, mctx);
        slong nfields = mctx->nfields;
        if (Abits < FLINT_BITS && Bbits < FLINT_BITS)
        {
            ulong * Aunpack, * Bunpack;
            TMP_INIT;

            TMP_START;

            Aunpack = (ulong *) TMP_ALLOC(nfields*sizeof(ulong));
            Bunpack = (ulong *) TMP_ALLOC(nfields*sizeof(ulong));

            for (i = 0; i < length; i++)
            {
                mpoly_unpack_vec_ui(Aunpack, Aexps + NA*i, Abits, nfields, 1);
                mpoly_unpack_vec_ui(Bunpack, Bexps + NB*i, Bbits, nfields, 1);
                for (j = nfields - 1; j >= 0; j--)
                {
                    if (Aunpack[j] != Bunpack[j])
                    {
                        int cmp = Aunpack[j] < Bunpack[j] ? -1 : 1;
                        TMP_END;
                        return cmp;
                    }
                }
            }

            TMP_END;
        }
        else
        {
            fmpz * Aunpack = _fmpz_vec_init(nfields);
            fmpz * Bunpack = _fmpz_vec_init(nfields);

            for (i = 0; i < length; i++)
            {
                mpoly_unpack_vec_fmpz(Aunpack, Aexps + NA*i, Abits, nfields, 1);
                mpoly_unpack_vec_fmpz(Bunpack, Bexps + NB*i, Bbits, nfields, 1);
                for (j = nfields - 1; j >= 0; j--)
                {
                    int cmp = fmpz_cmp(Aunpack + j, Bunpack + j);
                    if (cmp != 0)
                    {
                        _fmpz_vec_clear(Aunpack, nfields);
                        _fmpz_vec_clear(Bunpack, nfields);
                        return cmp;
                    }
                }
            }

            _fmpz_vec_clear(Aunpack, nfields);
            _fmpz_vec_clear(Bunpack, nfields);
        }
    }

    return 0;
}


int nmod_mpoly_cmp_sort(
    const nmod_mpoly_t A,
    const nmod_mpoly_t B,
    const nmod_mpoly_ctx_t ctx)
{
    slong i;
    slong length = A->length;
    mp_limb_t * Acoeffs = A->coeffs;
    mp_limb_t * Bcoeffs = B->coeffs;

    if (A->length != B->length)
    {
        return A->length < B->length ? -1 : 1;
    }

    for (i = 0; i < length; i++)
    {
        if (Acoeffs[i] != Bcoeffs[i])
        {
            return Acoeffs[i] < Bcoeffs[i] ? -1 : 1;
        }
    }

    return mpoly_monomial_cmp_sort(A->exps, A->bits,
                                   B->exps, B->bits, length, ctx->minfo);
}


typedef struct {
    slong idx;
    fmpz exp;
    const nmod_mpoly_struct * polys;
    const nmod_mpoly_ctx_struct * ctx;
} sort_struct;

static int _sort(const void * a_, const void * b_)
{
    int cmp;
    const sort_struct * a = (const sort_struct *) a_;
    const sort_struct * b = (const sort_struct *) b_;
    const nmod_mpoly_struct * apoly = a->polys + a->idx;
    const nmod_mpoly_struct * bpoly = b->polys + b->idx;

    cmp = fmpz_cmp(&a->exp, &b->exp);
    if (cmp != 0)
        return cmp;

    if (apoly->length != bpoly->length)
    {
        return apoly->length < bpoly->length ? -1 : 1;
    }

    return nmod_mpoly_cmp_sort(apoly, bpoly, a->ctx);
}

void nmod_mpoly_factor_sort(
    nmod_mpoly_factor_t f,
    const nmod_mpoly_ctx_t ctx)
{
    slong i;
    sort_struct * data;
    nmod_mpoly_struct * fpolycopy;

    if (f->length == 0)
        return;

    data = (sort_struct *) flint_malloc(f->length*sizeof(sort_struct));
    for (i = 0; i < f->length; i++)
    {
        data[i].idx = i;
        data[i].exp = f->exp[i];
        data[i].polys = f->poly;
        data[i].ctx = ctx;
    }

    qsort(data, f->length, sizeof(sort_struct), _sort);

    /* we will not permute in place */
    fpolycopy = (nmod_mpoly_struct *) flint_malloc(f->length*sizeof(nmod_mpoly_struct));
    memcpy(fpolycopy, f->poly, f->length*sizeof(nmod_mpoly_struct));

    for (i = 0; i < f->length; i++)
    {
        f->exp[i] = data[i].exp;
        f->poly[i] = fpolycopy[data[i].idx];
    }
    
    flint_free(fpolycopy);
    flint_free(data);

    return;
}
