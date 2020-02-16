/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#ifndef FQ_NMOD_MPOLY_FACTOR_H
#define FQ_NMOD_MPOLY_FACTOR_H

#ifdef FQ_NMOD_MPOLY_FACTOR_INLINES_C
#define FQ_NMOD_MPOLY_FACTOR_INLINE FLINT_DLL
#else
#define FQ_NMOD_MPOLY_FACTOR_INLINE static __inline__
#endif

#undef ulong
#define ulong ulongxx /* interferes with system includes */
#include <stdio.h>
#undef ulong
#include <gmp.h>
#define ulong mp_limb_t

#include "flint/fq_nmod_mpoly.h"

#ifdef __cplusplus
 extern "C" {
#endif

FLINT_DLL void fq_nmod_mpoly_evaluate_one_fq_nmod(
    fq_nmod_mpoly_t A,
    const fq_nmod_mpoly_t B,
    slong var,
    const fq_nmod_t val,
    const fq_nmod_mpoly_ctx_t ctx);

typedef struct {
    fq_nmod_t content;
    fq_nmod_mpoly_struct * poly;
    fmpz * exp;
    slong length;
    slong alloc;
} fq_nmod_mpoly_factor_struct;

typedef fq_nmod_mpoly_factor_struct fq_nmod_mpoly_factor_t[1];

FLINT_DLL void fq_nmod_mpoly_factor_init(fq_nmod_mpoly_factor_t f, const fq_nmod_mpoly_ctx_t ctx);

FLINT_DLL void fq_nmod_mpoly_factor_realloc(fq_nmod_mpoly_factor_t f, slong alloc, const fq_nmod_mpoly_ctx_t ctx);

FLINT_DLL void fq_nmod_mpoly_factor_fit_length(fq_nmod_mpoly_factor_t f, slong len, const fq_nmod_mpoly_ctx_t ctx);

FLINT_DLL void fq_nmod_mpoly_factor_clear(fq_nmod_mpoly_factor_t f, const fq_nmod_mpoly_ctx_t ctx);

FLINT_DLL void fq_nmod_mpoly_factor_set(fq_nmod_mpoly_factor_t a, const fq_nmod_mpoly_factor_t b, const fq_nmod_mpoly_ctx_t ctx);

FLINT_DLL void fq_nmod_mpoly_factor_print_pretty(const fq_nmod_mpoly_factor_t f, const char ** vars, const fq_nmod_mpoly_ctx_t ctx);

FLINT_DLL void fq_nmod_mpoly_factor_append_ui(fq_nmod_mpoly_factor_t f, const fq_nmod_mpoly_t A, ulong e, const fq_nmod_mpoly_ctx_t ctx);

FLINT_DLL void fq_nmod_mpoly_factor_append_fmpz(fq_nmod_mpoly_factor_t f, const fq_nmod_mpoly_t A, const fmpz_t e, const fq_nmod_mpoly_ctx_t ctx);

FLINT_DLL int fq_nmod_mpoly_factor(fq_nmod_mpoly_factor_t f, const fq_nmod_mpoly_t A, int full, const fq_nmod_mpoly_ctx_t ctx);

FQ_NMOD_MPOLY_FACTOR_INLINE void fq_nmod_mpoly_factor_swap(fq_nmod_mpoly_factor_t A, fq_nmod_mpoly_factor_t B, const fq_nmod_mpoly_ctx_t ctx)
{
   fq_nmod_mpoly_factor_struct t = *A;
   *A = *B;
   *B = t;
}


#ifdef __cplusplus
}
#endif

#endif

