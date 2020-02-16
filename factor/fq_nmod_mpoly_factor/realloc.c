/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "factor/fq_nmod_mpoly_factor.h"

void fq_nmod_mpoly_factor_realloc(
    fq_nmod_mpoly_factor_t f,
    slong alloc,
    const fq_nmod_mpoly_ctx_t ctx)
{
    if (alloc == 0)             /* Clear up, reinitialise */
    {
        fq_nmod_mpoly_factor_clear(f, ctx);
        fq_nmod_mpoly_factor_init(f, ctx);
    }
    else if (f->alloc > 0)            /* Realloc */
    {
        if (f->alloc > alloc)
        {
            slong i;

            for (i = alloc; i < f->length; i++)
			{
                fq_nmod_mpoly_clear(f->poly + i, ctx);
				fmpz_clear(f->exp + i);
			}

            f->poly = (fq_nmod_mpoly_struct *) flint_realloc(f->poly, alloc * sizeof(fq_nmod_mpoly_struct));
            f->exp  = (fmpz *) flint_realloc(f->exp, alloc * sizeof(fmpz));
            f->alloc = alloc;
        }
        else if (f->alloc < alloc)
        {
            slong i;

            f->poly = (fq_nmod_mpoly_struct *) flint_realloc(f->poly, alloc * sizeof(fq_nmod_mpoly_struct));
            f->exp  = (fmpz *) flint_realloc(f->exp, alloc * sizeof(fmpz));

            for (i = f->alloc; i < alloc; i++)
            {
                fq_nmod_mpoly_init(f->poly + i, ctx);
				fmpz_init(f->exp + i);
            }
            f->alloc = alloc;
        }
    }
    else /* Nothing allocated already so do it now */
    {
        slong i;

        f->poly = (fq_nmod_mpoly_struct *) flint_malloc(alloc * sizeof(fq_nmod_mpoly_struct));
        f->exp  = (fmpz *) flint_malloc(alloc * sizeof(fmpz));

        for (i = 0; i < alloc; i++)
        {
            fq_nmod_mpoly_init(f->poly + i, ctx);
			fmpz_init(f->exp + i);
        }
        f->length = 0;
        f->alloc  = alloc;
    }
}
