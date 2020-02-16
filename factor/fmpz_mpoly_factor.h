/*
    Copyright (C) 2006, 2007, 2008, 2009, 2010, 2016 William Hart
    Copyright (C) 2009, 2011 Andy Novocin
    Copyright (C) 2010 Sebastian Pancratz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#ifndef FMPZ_MPOLY_FACTOR_H
#define FMPZ_MPOLY_FACTOR_H

#ifdef FMPZ_MPOLY_FACTOR_INLINES_C
#define FMPZ_MPOLY_FACTOR_INLINE FLINT_DLL
#else
#define FMPZ_MPOLY_FACTOR_INLINE static __inline__
#endif

#undef ulong
#define ulong ulongxx /* interferes with system includes */
#include <stdio.h>
#undef ulong
#include <gmp.h>
#define ulong mp_limb_t

#include "flint/fmpz_mpoly.h"

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct {
    fmpz_t content;
    fmpz_mpoly_struct * poly;
    fmpz * exp;
    slong length;
    slong alloc;
} fmpz_mpoly_factor_struct;

typedef fmpz_mpoly_factor_struct fmpz_mpoly_factor_t[1];

FLINT_DLL void fmpz_mpoly_factor_init(fmpz_mpoly_factor_t fac, const fmpz_mpoly_ctx_t ctx);

FLINT_DLL void fmpz_mpoly_factor_init2(fmpz_mpoly_factor_t fac, slong alloc, const fmpz_mpoly_ctx_t ctx);

FLINT_DLL void fmpz_mpoly_factor_realloc(fmpz_mpoly_factor_t fac, slong alloc, const fmpz_mpoly_ctx_t ctx);

FLINT_DLL void fmpz_mpoly_factor_fit_length(fmpz_mpoly_factor_t fac, slong len, const fmpz_mpoly_ctx_t ctx);

FLINT_DLL void fmpz_mpoly_factor_clear(fmpz_mpoly_factor_t fac, const fmpz_mpoly_ctx_t ctx);

FLINT_DLL void fmpz_mpoly_factor_set(fmpz_mpoly_factor_t res, const fmpz_mpoly_factor_t fac, const fmpz_mpoly_ctx_t ctx);

FLINT_DLL void fmpz_mpoly_factor_append_ui(fmpz_mpoly_factor_t f, const fmpz_mpoly_t A, ulong e, const fmpz_mpoly_ctx_t ctx);

FLINT_DLL void fmpz_mpoly_factor_append_fmpz(fmpz_mpoly_factor_t f, const fmpz_mpoly_t A, const fmpz_t e, const fmpz_mpoly_ctx_t ctx);

FLINT_DLL void fmpz_mpoly_factor_print_pretty(const fmpz_mpoly_factor_t f, const char ** vars, const fmpz_mpoly_ctx_t ctx);

FLINT_DLL int fmpz_mpoly_factor(fmpz_mpoly_factor_t fac, const fmpz_mpoly_t A, int full, const fmpz_mpoly_ctx_t ctx);

FMPZ_MPOLY_FACTOR_INLINE void fmpz_mpoly_factor_swap(fmpz_mpoly_factor_t A, fmpz_mpoly_factor_t B, const fmpz_mpoly_ctx_t ctx)
{
   fmpz_mpoly_factor_struct t = *A;
   *A = *B;
   *B = t;
}

FLINT_DLL void fmpz_mpoly_factor_sort(fmpz_mpoly_factor_t f, const fmpz_mpoly_ctx_t ctx);

void subset_first(fmpz_t a, slong n, slong r);

int subset_next(fmpz_t a, const fmpz_t b, slong n);

void subset_print(const fmpz_t a, slong n);

#ifdef __cplusplus
}
#endif

#endif

