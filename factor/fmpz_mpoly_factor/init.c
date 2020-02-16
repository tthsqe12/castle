/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "factor/fmpz_mpoly_factor.h"

void fmpz_mpoly_factor_init(fmpz_mpoly_factor_t fac, const fmpz_mpoly_ctx_t ctx)
{
    fmpz_init_set_ui(fac->content, 1);
    fac->poly   = NULL;
    fac->exp    = NULL;
    fac->length = 0;
    fac->alloc  = 0;
}

void fmpz_mpoly_factor_init2(fmpz_mpoly_factor_t fac, slong alloc, const fmpz_mpoly_ctx_t ctx)
{
    fmpz_init_set_ui(fac->content, 1);

    if (alloc)
    {
        slong i;

        fac->poly = (fmpz_mpoly_struct *) flint_malloc(alloc * sizeof(fmpz_mpoly_struct));
        fac->exp  = (slong *) flint_malloc(alloc * sizeof(slong));

        for (i = 0; i < alloc; i++)
        {
            fmpz_mpoly_init(fac->poly + i, ctx);
            fac->exp[i] = WORD(0);
        }
    }
    else
    {
        fac->poly = NULL;
        fac->exp  = NULL;
    }

    fac->length = 0;
    fac->alloc  = alloc;
}

