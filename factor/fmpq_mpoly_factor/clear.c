/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "factor/fmpq_mpoly_factor.h"

void fmpq_mpoly_factor_clear(fmpq_mpoly_factor_t fac, const fmpq_mpoly_ctx_t ctx)
{
    fmpq_clear(fac->content);

    if (fac->alloc > 0)
    {
        slong i;

        for (i = 0; i < fac->alloc; i++)
        {
            fmpq_mpoly_clear(fac->poly + i, ctx);
			fmpz_clear(fac->exp + i);
        }

        flint_free(fac->poly);
        flint_free(fac->exp);
    }
}
