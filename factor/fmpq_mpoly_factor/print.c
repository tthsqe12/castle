/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "factor/fmpq_mpoly_factor.h"

void fmpq_mpoly_factor_print_pretty(const fmpq_mpoly_factor_t fac, const char ** vars, const fmpq_mpoly_ctx_t ctx)
{
    slong i;

    flint_printf("content: ");
    fmpq_print(fac->content);
    flint_printf("\n");
    for (i = 0; i < fac->length; i++)
    {
        flint_printf("factor[%wd]: ", i);
        fmpq_mpoly_print_pretty(fac->poly + i, vars, ctx);
        flint_printf(" ^ ");
		fmpz_print(fac->exp + i);
		flint_printf("\n");
    }
}
