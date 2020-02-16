/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "factor/fmpq_mpoly_factor.h"

void fmpq_mpoly_factor_init(fmpq_mpoly_factor_t fac, const fmpq_mpoly_ctx_t ctx)
{
    fmpq_init(fac->content);
	fmpq_one(fac->content);
    fac->poly   = NULL;
    fac->exp    = NULL;
    fac->length = 0;
    fac->alloc  = 0;
}

void fmpq_mpoly_factor_init2(fmpq_mpoly_factor_t fac, slong alloc, const fmpq_mpoly_ctx_t ctx)
{
    fmpq_init(fac->content);
	fmpq_one(fac->content);

    if (alloc)
    {
        slong i;

        fac->poly = (fmpq_mpoly_struct *) flint_malloc(alloc * sizeof(fmpq_mpoly_struct));
        fac->exp  = (fmpz *) flint_malloc(alloc * sizeof(fmpz));

        for (i = 0; i < alloc; i++)
        {
            fmpq_mpoly_init(fac->poly + i, ctx);
			fmpz_init(fac->exp + i);
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

