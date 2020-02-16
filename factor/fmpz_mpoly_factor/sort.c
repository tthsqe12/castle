/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "factor/fmpz_mpoly_factor.h"
#include <string.h>

int mpoly_monomial_cmp_sort(
    const ulong * Aexps, flint_bitcnt_t Abits,
    const ulong * Bexps, flint_bitcnt_t Bbits,
    slong length, const mpoly_ctx_t mctx);

int fmpz_mpoly_cmp_sort(
    const fmpz_mpoly_t A,
    const fmpz_mpoly_t B,
    const fmpz_mpoly_ctx_t ctx)
{
    slong i;
    slong length = A->length;
    fmpz * Acoeffs = A->coeffs;
    fmpz * Bcoeffs = B->coeffs;

    if (A->length != B->length)
    {
        return A->length < B->length ? -1 : 1;
    }

    for (i = 0; i < length; i++)
    {
        int cmp = fmpz_cmp(Acoeffs + i, Bcoeffs + i);
        if (cmp != 0)
            return cmp;
    }

    return mpoly_monomial_cmp_sort(A->exps, A->bits,
                                   B->exps, B->bits, length, ctx->minfo);
}


typedef struct {
    slong idx;
    fmpz exp;
    const fmpz_mpoly_struct * polys;
    const fmpz_mpoly_ctx_struct * ctx;
} sort_struct;

static int _sort(const void * a_, const void * b_)
{
    int cmp;
    const sort_struct * a = (const sort_struct *) a_;
    const sort_struct * b = (const sort_struct *) b_;
    const fmpz_mpoly_struct * apoly = a->polys + a->idx;
    const fmpz_mpoly_struct * bpoly = b->polys + b->idx;

    cmp = fmpz_cmp(&a->exp, &b->exp);
    if (cmp != 0)
        return cmp;

    if (apoly->length != bpoly->length)
    {
        return apoly->length < bpoly->length ? -1 : 1;
    }

    return fmpz_mpoly_cmp_sort(apoly, bpoly, a->ctx);
}

void fmpz_mpoly_factor_sort(
    fmpz_mpoly_factor_t f,
    const fmpz_mpoly_ctx_t ctx)
{
    slong i;
    sort_struct * data;
    fmpz_mpoly_struct * fpolycopy;

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
    fpolycopy = (fmpz_mpoly_struct *) flint_malloc(f->length*sizeof(fmpz_mpoly_struct));
    memcpy(fpolycopy, f->poly, f->length*sizeof(fmpz_mpoly_struct));

    for (i = 0; i < f->length; i++)
    {
        f->exp[i] = data[i].exp;
        f->poly[i] = fpolycopy[data[i].idx];
    }
    
    flint_free(fpolycopy);
    flint_free(data);

    return;
}
