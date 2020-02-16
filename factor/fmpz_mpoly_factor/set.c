/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "factor/fmpz_mpoly_factor.h"

void fmpz_mpoly_factor_set(fmpz_mpoly_factor_t res, const fmpz_mpoly_factor_t fac, const fmpz_mpoly_ctx_t ctx)
{
    slong i;

    if (res == fac)
        return;

    fmpz_mpoly_factor_fit_length(res, fac->length, ctx);
    fmpz_set(res->content, fac->content);
    for (i = 0; i < fac->length; i++)
    {
        fmpz_mpoly_set(res->poly + i, fac->poly + i, ctx);
        res->exp[i] = fac->exp[i];
    }
    res->length = fac->length;
}
