/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#ifndef NMOD_MPOLY_FACTOR_H
#define NMOD_MPOLY_FACTOR_H

#ifdef NMOD_MPOLY_FACTOR_INLINES_C
#define NMOD_MPOLY_FACTOR_INLINE FLINT_DLL
#else
#define NMOD_MPOLY_FACTOR_INLINE static __inline__
#endif

#undef ulong
#define ulong ulongxx /* interferes with system includes */
#include <stdio.h>
#undef ulong
#include <gmp.h>
#define ulong mp_limb_t

#include "flint/nmod_mpoly.h"

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct {
    mp_limb_t content;
    nmod_mpoly_struct * poly;
    fmpz * exp;
    slong length;
    slong alloc;
} nmod_mpoly_factor_struct;

typedef nmod_mpoly_factor_struct nmod_mpoly_factor_t[1];

FLINT_DLL void nmod_mpoly_factor_init(nmod_mpoly_factor_t f, const nmod_mpoly_ctx_t ctx);

FLINT_DLL void nmod_mpoly_factor_init2(nmod_mpoly_factor_t f, slong alloc, const nmod_mpoly_ctx_t ctx);

FLINT_DLL void nmod_mpoly_factor_realloc(nmod_mpoly_factor_t f, slong alloc, const nmod_mpoly_ctx_t ctx);

FLINT_DLL void nmod_mpoly_factor_fit_length(nmod_mpoly_factor_t f, slong len, const nmod_mpoly_ctx_t ctx);

FLINT_DLL void nmod_mpoly_factor_clear(nmod_mpoly_factor_t f, const nmod_mpoly_ctx_t ctx);

FLINT_DLL void nmod_mpoly_factor_append_ui(nmod_mpoly_factor_t f, const nmod_mpoly_t A, ulong e, const nmod_mpoly_ctx_t ctx);

FLINT_DLL void nmod_mpoly_factor_append_fmpz(nmod_mpoly_factor_t f, const nmod_mpoly_t A, const fmpz_t e, const nmod_mpoly_ctx_t ctx);

FLINT_DLL void nmod_mpoly_factor_set(nmod_mpoly_factor_t a, const nmod_mpoly_factor_t b, const nmod_mpoly_ctx_t ctx);

FLINT_DLL void nmod_mpoly_factor_print_pretty(const nmod_mpoly_factor_t f, const char ** vars, const nmod_mpoly_ctx_t ctx);

FLINT_DLL int nmod_mpoly_factor(nmod_mpoly_factor_t f, const nmod_mpoly_t A, int full, const nmod_mpoly_ctx_t ctx);

FLINT_DLL void nmod_mpoly_factor_sort(nmod_mpoly_factor_t f, const nmod_mpoly_ctx_t ctx);

NMOD_MPOLY_FACTOR_INLINE void nmod_mpoly_factor_swap(nmod_mpoly_factor_t A, nmod_mpoly_factor_t B, const nmod_mpoly_ctx_t ctx)
{
   nmod_mpoly_factor_struct t = *A;
   *A = *B;
   *B = t;
}

void subset_first(fmpz_t a, slong n, slong r);

int subset_next(fmpz_t a, const fmpz_t b, slong n);

void subset_print(const fmpz_t a, slong n);


#ifdef __cplusplus
}
#endif

#endif

