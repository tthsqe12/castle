/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "factor/fmpz_mpoly_factor.h"
#include "factor/fmpq_mpoly_factor.h"

int fmpq_mpoly_factor(fmpq_mpoly_factor_t fac, const fmpq_mpoly_t A, int full, const fmpq_mpoly_ctx_t ctx)
{
	slong i;
    int success;
    fmpz_mpoly_factor_t zfac;

	fmpz_mpoly_factor_init(zfac, ctx->zctx);
	success = fmpz_mpoly_factor(zfac, A->zpoly, full, ctx->zctx);

	fmpq_mpoly_factor_fit_length(fac, zfac->length, ctx);
	fmpq_mul_fmpz(fac->content, A->content, zfac->content);
	for (i = 0; i < zfac->length; i++)
	{
		fmpq_one((fac->poly + i)->content);
		fmpz_mpoly_swap((fac->poly + i)->zpoly, zfac->poly + i, ctx->zctx);
		fmpz_set_si(fac->exp + i, zfac->exp[i]);
	}
	fac->length = zfac->length;

	fmpz_mpoly_factor_clear(zfac, ctx->zctx);
	return success;
}

