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

#ifndef FMPQ_MPOLY_FACTOR_H
#define FMPQ_MPOLY_FACTOR_H

#ifdef FMPQ_MPOLY_FACTOR_INLINES_C
#define FMPQ_MPOLY_FACTOR_INLINE FLINT_DLL
#else
#define FMPQ_MPOLY_FACTOR_INLINE static __inline__
#endif

#undef ulong
#define ulong ulongxx /* interferes with system includes */
#include <stdio.h>
#undef ulong
#include <gmp.h>
#define ulong mp_limb_t

#include "flint/fmpq_mpoly.h"

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct {
    fmpq_t content;
    fmpq_mpoly_struct * poly;
    fmpz * exp;
    slong length;
    slong alloc;
} fmpq_mpoly_factor_struct;

typedef fmpq_mpoly_factor_struct fmpq_mpoly_factor_t[1];

FLINT_DLL void fmpq_mpoly_factor_init(fmpq_mpoly_factor_t fac, const fmpq_mpoly_ctx_t ctx);

FLINT_DLL void fmpq_mpoly_factor_init2(fmpq_mpoly_factor_t fac, slong alloc, const fmpq_mpoly_ctx_t ctx);

FLINT_DLL void fmpq_mpoly_factor_realloc(fmpq_mpoly_factor_t fac, slong alloc, const fmpq_mpoly_ctx_t ctx);

FLINT_DLL void fmpq_mpoly_factor_fit_length(fmpq_mpoly_factor_t fac, slong len, const fmpq_mpoly_ctx_t ctx);

FLINT_DLL void fmpq_mpoly_factor_clear(fmpq_mpoly_factor_t fac, const fmpq_mpoly_ctx_t ctx);

FLINT_DLL void fmpq_mpoly_factor_set(fmpq_mpoly_factor_t res, const fmpq_mpoly_factor_t fac, const fmpq_mpoly_ctx_t ctx);

FLINT_DLL void fmpq_mpoly_factor_print_pretty(const fmpq_mpoly_factor_t fac, const char ** vars, const fmpq_mpoly_ctx_t ctx);

FLINT_DLL int fmpq_mpoly_factor(fmpq_mpoly_factor_t fac, const fmpq_mpoly_t A, int full, const fmpq_mpoly_ctx_t ctx);

#ifdef __cplusplus
}
#endif

#endif

