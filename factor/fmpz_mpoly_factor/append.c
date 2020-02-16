/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "factor/fmpz_mpoly_factor.h"

void fmpz_mpoly_factor_append_fmpz(
    fmpz_mpoly_factor_t f,
    const fmpz_mpoly_t A,
    const fmpz_t e,
    const fmpz_mpoly_ctx_t ctx)
{
    slong i = f->length;
    fmpz_mpoly_factor_fit_length(f, i + 1, ctx);
    fmpz_mpoly_set(f->poly + i, A, ctx);
    fmpz_set(f->exp + i, e);
    f->length = i + 1;
}

void fmpz_mpoly_factor_append_ui(
    fmpz_mpoly_factor_t f,
    const fmpz_mpoly_t A,
    ulong e,
    const fmpz_mpoly_ctx_t ctx)
{
    slong i = f->length;
    fmpz_mpoly_factor_fit_length(f, i + 1, ctx);
    fmpz_mpoly_set(f->poly + i, A, ctx);
    fmpz_set_ui(f->exp + i, e);
    f->length = i + 1;
}

