/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "factor/fmpq_mpoly_factor.h"

void fmpq_mpoly_factor_realloc(fmpq_mpoly_factor_t fac, slong alloc, const fmpq_mpoly_ctx_t ctx)
{
    if (alloc == 0)             /* Clear up, reinitialise */
    {
        fmpq_mpoly_factor_clear(fac, ctx);
        fmpq_mpoly_factor_init(fac, ctx);
    }
    else if (fac->alloc > 0)            /* Realloc */
    {
        if (fac->alloc > alloc)
        {
            slong i;

            for (i = alloc; i < fac->length; i++)
			{
                fmpq_mpoly_clear(fac->poly + i, ctx);
				fmpz_clear(fac->exp + i);
			}

            fac->poly = (fmpq_mpoly_struct *) flint_realloc(fac->poly, alloc * sizeof(fmpq_mpoly_struct));
            fac->exp  = (fmpz *) flint_realloc(fac->exp, alloc * sizeof(fmpz));
            fac->alloc = alloc;
        }
        else if (fac->alloc < alloc)
        {
            slong i;

            fac->poly = (fmpq_mpoly_struct *) flint_realloc(fac->poly, alloc * sizeof(fmpq_mpoly_struct));
            fac->exp  = (fmpz *) flint_realloc(fac->exp, alloc * sizeof(fmpz));

            for (i = fac->alloc; i < alloc; i++)
            {
                fmpq_mpoly_init(fac->poly + i, ctx);
				fmpz_init(fac->exp + i);
            }
            fac->alloc = alloc;
        }
    }
    else                        /* Nothing allocated already so do it now */
    {
        slong i;

        fac->poly = (fmpq_mpoly_struct *) flint_malloc(alloc * sizeof(fmpq_mpoly_struct));
        fac->exp  = (fmpz *) flint_malloc(alloc * sizeof(fmpz));

        for (i = 0; i < alloc; i++)
        {
            fmpq_mpoly_init(fac->poly + i, ctx);
			fmpz_init(fac->exp + i);
        }
        fac->length = 0;
        fac->alloc  = alloc;
    }
}
