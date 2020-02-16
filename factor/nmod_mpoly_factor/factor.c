/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "factor/nmod_mpoly_factor.h"
#include "factor/fq_nmod_mpoly_factor.h"
void usleep(ulong);


int nmod_mpoly_gcd_cofactors(
    nmod_mpoly_t G,
    nmod_mpoly_t Abar,
    nmod_mpoly_t Bbar,
    const nmod_mpoly_t A,
    const nmod_mpoly_t B,
    const nmod_mpoly_ctx_t ctx)
{
    nmod_mpoly_t Asave, Bsave;
    int success;
    nmod_mpoly_init(Asave, ctx);
    nmod_mpoly_init(Bsave, ctx);
    nmod_mpoly_set(Asave, A, ctx);
    nmod_mpoly_set(Bsave, B, ctx);
    success = nmod_mpoly_gcd(G, A, B, ctx);
    if (success)
    {
        success = nmod_mpoly_divides(Abar, Asave, G, ctx); FLINT_ASSERT(success);
        success = nmod_mpoly_divides(Bbar, Bsave, G, ctx); FLINT_ASSERT(success);
    }
    nmod_mpoly_clear(Asave, ctx);
    nmod_mpoly_clear(Bsave, ctx);
    return success;
}

/********************* univar ************************************************/

int nmod_mpoly_univar_content_mpoly(
    nmod_mpoly_t g,
    const nmod_mpoly_univar_t A,
    const nmod_mpoly_ctx_t ctx)
{
    slong i;

    nmod_mpoly_zero(g, ctx);
    for (i = 0; i < A->length; i++)
    {
		if (!nmod_mpoly_gcd(g, g, A->coeffs + i, ctx))
			return 0;
    }

    return 1;
}

int fq_nmod_mpoly_univar_content_mpoly(
    fq_nmod_mpoly_t g,
    const fq_nmod_mpoly_univar_t A,
    const fq_nmod_mpoly_ctx_t ctx)
{
    slong i;

    fq_nmod_mpoly_zero(g, ctx);
    for (i = 0; i < A->length; i++)
    {
		if (!fq_nmod_mpoly_gcd(g, g, A->coeffs + i, ctx))
			return 0;
    }

    return 1;
}

void nmod_mpoly_univar_divexact_mpoly(
    nmod_mpoly_univar_t A,
    const nmod_mpoly_t b,
    const nmod_mpoly_ctx_t ctx)
{
    int success;
    slong i;

    for (i = 0; i < A->length; i++)
    {
        success = nmod_mpoly_divides(A->coeffs + i, A->coeffs + i, b, ctx);
        FLINT_ASSERT(success);
    }
}

void nmod_mpoly_univar_shift_right(
    nmod_mpoly_univar_t A,
    ulong shift,
    const nmod_mpoly_ctx_t ctx)
{
    slong i;
    for (i = 0; i < A->length; i++)
    {
        FLINT_ASSERT(A->exps[i] >= shift);
        A->exps[i] -= shift;
    }
}

/* nmod_mpoly_factor_t *******************************************************/

void nmod_mpoly_factor_expand(nmod_mpoly_t a, const nmod_mpoly_factor_t f, const nmod_mpoly_ctx_t ctx)
{
    slong i;
    nmod_mpoly_t t1, t2;

    nmod_mpoly_init(t1, ctx);
    nmod_mpoly_init(t2, ctx);

    nmod_mpoly_set_ui(a, f->content, ctx);

    for (i = 0; i < f->length; i++)
    {
        nmod_mpoly_pow_fmpz(t1, f->poly + i, f->exp + i, ctx);
        nmod_mpoly_mul(t2, a, t1, ctx);
        nmod_mpoly_swap(a, t2, ctx);
    }

    nmod_mpoly_clear(t1, ctx);
    nmod_mpoly_clear(t2, ctx);
}

void fq_nmod_mpoly_factor_expand(fq_nmod_mpoly_t a, const fq_nmod_mpoly_factor_t f, const fq_nmod_mpoly_ctx_t ctx)
{
    slong i;
    fq_nmod_mpoly_t t1, t2;

    fq_nmod_mpoly_init(t1, ctx);
    fq_nmod_mpoly_init(t2, ctx);

    fq_nmod_mpoly_set_fq_nmod(a, f->content, ctx);

    for (i = 0; i < f->length; i++)
    {
        fq_nmod_mpoly_pow_fmpz(t1, f->poly + i, f->exp + i, ctx);
        fq_nmod_mpoly_mul(t2, a, t1, ctx);
        fq_nmod_mpoly_swap(a, t2, ctx);
    }

    fq_nmod_mpoly_clear(t1, ctx);
    fq_nmod_mpoly_clear(t2, ctx);
}


int nmod_mpoly_factor_matches(const nmod_mpoly_t a, const nmod_mpoly_factor_t f, const nmod_mpoly_ctx_t ctx)
{
    int matches;
    nmod_mpoly_t t;
    nmod_mpoly_init(t, ctx);
    nmod_mpoly_factor_expand(t, f, ctx);
    matches = nmod_mpoly_equal(t, a, ctx);
    nmod_mpoly_clear(t, ctx);
    return matches;
}

int fq_nmod_mpoly_factor_matches(const fq_nmod_mpoly_t a, const fq_nmod_mpoly_factor_t f, const fq_nmod_mpoly_ctx_t ctx)
{
    int matches;
    fq_nmod_mpoly_t t;
    fq_nmod_mpoly_init(t, ctx);
    fq_nmod_mpoly_factor_expand(t, f, ctx);
    matches = fq_nmod_mpoly_equal(t, a, ctx);
    fq_nmod_mpoly_clear(t, ctx);
    return matches;
}

/* 	return 0 no, 1 yes, -1 don't know	*/
int nmod_mpoly_factor_is_pairwise_prime(
	const nmod_mpoly_factor_t f, 
	const nmod_mpoly_ctx_t ctx)
{
	int result;
	slong i, j;
	nmod_mpoly_t g;

	nmod_mpoly_init(g, ctx);

	for (i = 0; i + 1 < f->length; i++)
	for (j = i + 1; j < f->length; j++)
	{
		/* make sure factors are monic */
		if (   (f->poly + i)->length == 0
			|| (f->poly + j)->length == 0
			|| (f->poly + i)->coeffs[0] != 1
			|| (f->poly + j)->coeffs[0] != 1)
		{
			result = 0;
			goto cleanup;
		}

		if (nmod_mpoly_gcd(g, f->poly + i, f->poly + j, ctx))
		{
			if (!nmod_mpoly_is_one(g, ctx))
			{
				result = 0;
				goto cleanup;
			}
		}
		else
		{
			result = -1;
			goto cleanup;
		}
	}

	result = 1;

cleanup:

	nmod_mpoly_clear(g, ctx);

	return result;
}

void nmod_mpoly_factor_one(
	nmod_mpoly_factor_t a,
	const nmod_mpoly_ctx_t ctx)
{
	a->content = 1;
	a->length = 0;
}

void fq_nmod_mpoly_factor_one(
	fq_nmod_mpoly_factor_t a,
	const fq_nmod_mpoly_ctx_t ctx)
{
	fq_nmod_one(a->content, ctx->fqctx);
	a->length = 0;
}

void nmod_mpoly_factor_pow_fmpz(
	nmod_mpoly_factor_t a,
	const nmod_mpoly_factor_t b,
	const fmpz_t e,
	const nmod_mpoly_ctx_t ctx)
{
	if (a != b)
		nmod_mpoly_factor_set(a, b, ctx);
	a->content = nmod_pow_fmpz(a->content, e, ctx->ffinfo->mod);
	_fmpz_vec_scalar_mul_fmpz(a->exp, a->exp, a->length, e);
}

void nmod_mpoly_factor_mul_mpoly_fmpz(
	nmod_mpoly_factor_t fac,
	const nmod_mpoly_t a,
	const fmpz_t e,
	const nmod_mpoly_ctx_t ctx)
{
	if (nmod_mpoly_is_ui(a, ctx))
	{
		ulong t = nmod_mpoly_get_ui(a, ctx);
		t = nmod_pow_fmpz(t, e, ctx->ffinfo->mod);
		fac->content = nmod_mul(fac->content, t, ctx->ffinfo->mod);
		return;
	}
	else
	{
		nmod_mpoly_factor_append_fmpz(fac, a, e, ctx);
	}
}

void nmod_mpoly_factor_mul_mpoly_ui(
	nmod_mpoly_factor_t fac,
	const nmod_mpoly_t a,
	ulong e,
	const nmod_mpoly_ctx_t ctx)
{
	if (nmod_mpoly_is_ui(a, ctx))
	{
		ulong t = nmod_mpoly_get_ui(a, ctx);
		t = nmod_pow_ui(t, e, ctx->ffinfo->mod);
		fac->content = nmod_mul(fac->content, t, ctx->ffinfo->mod);
		return;
	}
	else
	{
		nmod_mpoly_factor_append_ui(fac, a, e, ctx);
	}
}

/* a *= b^e */
void nmod_mpoly_factor_mul_factor_fmpz(
    nmod_mpoly_factor_t a,
    const nmod_mpoly_factor_t b,
    const fmpz_t e,
    const nmod_mpoly_ctx_t ctx)
{
    slong i;
    fmpz_t t;

    fmpz_init(t);

    a->content = nmod_mul(a->content, nmod_pow_fmpz(b->content, e, ctx->ffinfo->mod), ctx->ffinfo->mod);

    for (i = 0; i < b->length; i++)
    {
        fmpz_mul(t, b->exp + i, e);
        nmod_mpoly_factor_append_fmpz(a, b->poly + i, t, ctx);
    }

    fmpz_clear(t);
}

void nmod_mpoly_factor_mul_factor_ui(
    nmod_mpoly_factor_t a,
    const nmod_mpoly_factor_t b,
    ulong e,
    const nmod_mpoly_ctx_t ctx)
{
    slong i;
    fmpz_t t;

    fmpz_init(t);

    a->content = nmod_mul(a->content, nmod_pow_ui(b->content, e, ctx->ffinfo->mod), ctx->ffinfo->mod);

    for (i = 0; i < b->length; i++)
    {
        fmpz_mul_ui(t, b->exp + i, e);
        nmod_mpoly_factor_append_fmpz(a, b->poly + i, t, ctx);
    }

    fmpz_clear(t);
}


/*
	b and c are pairwise prime
	produce a=b*c pairwise prime and return 1
		else return 0 with a undefined
*/
int nmod_mpoly_factor_mul_pairwise_prime(
	nmod_mpoly_factor_t a,
	const nmod_mpoly_factor_t b,
	const nmod_mpoly_factor_t c,
	const nmod_mpoly_ctx_t ctx)
{
	int success;
	slong i, j;
	nmod_mpoly_t T1, T2;
	nmod_mpoly_struct * g;
	fmpz_t t;

	if (a == b || a == c)
	{
		nmod_mpoly_factor_t ta;
		nmod_mpoly_factor_init(ta, ctx);
		success = nmod_mpoly_factor_mul_pairwise_prime(ta, b, c, ctx);
		nmod_mpoly_factor_swap(a, ta, ctx);
		nmod_mpoly_factor_clear(ta, ctx);
		return success;
	}

	fmpz_init(t);
	nmod_mpoly_init(T1, ctx);
	nmod_mpoly_init(T2, ctx);

	FLINT_ASSERT(nmod_mpoly_factor_is_pairwise_prime(b, ctx) == 1);
	FLINT_ASSERT(nmod_mpoly_factor_is_pairwise_prime(c, ctx) == 1);

	g = (nmod_mpoly_struct *) flint_malloc(b->length*c->length*sizeof(nmod_mpoly_struct));
	/* g[i,j] = gcd(b[i], c[j]) */
	for (i = 0; i < b->length; i++)
	for (j = 0; j < c->length; j++)
	{
		nmod_mpoly_init(g + i*c->length + j, ctx);
	}

	a->content = nmod_mul(b->content, c->content, ctx->ffinfo->mod);
	a->length = 0;

	for (i = 0; i < b->length; i++)
	for (j = 0; j < c->length; j++)
	{
		if (!nmod_mpoly_gcd(g + i*c->length + j, b->poly + i, c->poly + j, ctx))
		{
			success = 0;
			goto cleanup;
		}

		fmpz_add(t, b->exp + i, c->exp + j);
		nmod_mpoly_factor_mul_mpoly_fmpz(a, g + i*c->length + j, t, ctx);
	}

	for (i = 0; i < b->length; i++)
	{
		nmod_mpoly_set(T1, b->poly + i, ctx);
		for (j = 0; j < c->length; j++)
		{
			success = nmod_mpoly_divides(T1, T1, g + i*c->length + j, ctx); FLINT_ASSERT(success);
		}
		nmod_mpoly_factor_mul_mpoly_fmpz(a, T1, b->exp + i, ctx);
	}	

	for (j = 0; j < c->length; j++)
	{
		nmod_mpoly_set(T1, c->poly + j, ctx);
		for (i = 0; i < b->length; i++)
		{
			success = nmod_mpoly_divides(T1, T1, g + i*c->length + j, ctx); FLINT_ASSERT(success);
		}
		nmod_mpoly_factor_mul_mpoly_fmpz(a, T1, c->exp + j, ctx);
	}	

	success = 1;

cleanup:

	for (i = 0; i < b->length; i++)
	for (j = 0; j < c->length; j++)
	{
		nmod_mpoly_clear(g + i*c->length + j, ctx);
	}
	flint_free(g);

	nmod_mpoly_clear(T1, ctx);
	nmod_mpoly_clear(T2, ctx);
	fmpz_clear(t);

	if (success)
	{
		nmod_mpoly_t ae, be, ce;
		nmod_mpoly_init(ae, ctx);
		nmod_mpoly_init(be, ctx);
		nmod_mpoly_init(ce, ctx);
		FLINT_ASSERT(nmod_mpoly_factor_is_pairwise_prime(a, ctx) == 1);
		nmod_mpoly_factor_expand(be, b, ctx);
		nmod_mpoly_factor_expand(ce, c, ctx);
		nmod_mpoly_mul(ae, be, ce, ctx);
		FLINT_ASSERT(nmod_mpoly_factor_matches(ae, a, ctx));
		nmod_mpoly_clear(ae, ctx);
		nmod_mpoly_clear(be, ctx);
		nmod_mpoly_clear(ce, ctx);
	}
	return success;
}


/* f *= g^e, no combination */
void nmod_mpoly_factor_append_factor_fmpz(
	nmod_mpoly_factor_t f,
	nmod_mpoly_factor_t g,
	fmpz_t e,
	const nmod_mpoly_ctx_t ctx)
{
	slong i;
	fmpz_t c;

	fmpz_init(c);

	f->content = nmod_mul(f->content, g->content, ctx->ffinfo->mod);
	for (i = 0; i < g->length; i++)
	{
		fmpz_mul(c, g->exp + i, e);
		nmod_mpoly_factor_append_fmpz(f, g->poly + i, c, ctx);
	}

	fmpz_clear(c);
}

void nmod_mpoly_factor_append_factor_ui(
	nmod_mpoly_factor_t f,
	nmod_mpoly_factor_t g,
	ulong e,
	const nmod_mpoly_ctx_t ctx)
{
	slong i;
	fmpz_t c;

	fmpz_init(c);

	f->content = nmod_mul(f->content, nmod_pow_ui(g->content, e, ctx->ffinfo->mod), ctx->ffinfo->mod);
	for (i = 0; i < g->length; i++)
	{
		fmpz_mul_si(c, g->exp + i, e);
		nmod_mpoly_factor_append_fmpz(f, g->poly + i, c, ctx);
	}

	fmpz_clear(c);
}




/******* nmod_bpoly - (Z/nZ)[x,y] ****************************************/

typedef struct
{
    nmod_poly_struct * coeffs;
    slong alloc;
    slong length;
    nmod_t modulus;
} nmod_bpoly_struct;

typedef nmod_bpoly_struct nmod_bpoly_t[1];

typedef struct
{
    fq_nmod_poly_struct * coeffs;
    slong alloc;
    slong length;
} fq_nmod_bpoly_struct;

typedef fq_nmod_bpoly_struct fq_nmod_bpoly_t[1];

void nmod_bpoly_init(nmod_bpoly_t A, const nmod_t modulus)
{
    A->coeffs = NULL;
    A->alloc = 0;
    A->length = 0;
    A->modulus = modulus;
}

void fq_nmod_bpoly_init(fq_nmod_bpoly_t A, const fq_nmod_ctx_t ectx)
{
    A->coeffs = NULL;
    A->alloc = 0;
    A->length = 0;
}

void nmod_bpoly_clear(nmod_bpoly_t A)
{
    if (A->coeffs)
    {
        slong i;
        for (i = 0; i < A->alloc; i++)
            nmod_poly_clear(A->coeffs + i);
        flint_free(A->coeffs);
    }
}

void fq_nmod_bpoly_clear(fq_nmod_bpoly_t A, const fq_nmod_ctx_t ectx)
{
    if (A->coeffs)
    {
        slong i;
        for (i = 0; i < A->alloc; i++)
            fq_nmod_poly_clear(A->coeffs + i, ectx);
        flint_free(A->coeffs);
    }
}

void nmod_bpoly_swap(nmod_bpoly_t A, nmod_bpoly_t B)
{
    nmod_bpoly_struct t = *A;
    *A = *B;
    *B = t;
}

void fq_nmod_bpoly_swap(fq_nmod_bpoly_t A, fq_nmod_bpoly_t B)
{
    fq_nmod_bpoly_struct t = *A;
    *A = *B;
    *B = t;
}

void nmod_bpoly_print(nmod_bpoly_t A, const char * xvar, const char * yvar)
{
    slong i;
    int first;

    first = 1;
    for (i = A->length - 1; i >= 0; i--)
    {
        if (nmod_poly_is_zero(A->coeffs + i))
        {
            FLINT_ASSERT(i < A->length - 1);
            continue;
        }

        if (!first)
            flint_printf(" + ");

        first = 0;

        flint_printf("(");
        nmod_poly_print_pretty(A->coeffs + i, yvar);
        flint_printf(")*%s^%wd", xvar, i);
    }

    if (first)
        flint_printf("0");
}

void fq_nmod_bpoly_print_pretty(
    fq_nmod_bpoly_t A,
    const char * xvar,
    const char * yvar,
    const fq_nmod_ctx_t ectx)
{
    slong i;
    int first;

    first = 1;
    for (i = A->length - 1; i >= 0; i--)
    {
        if (fq_nmod_poly_is_zero(A->coeffs + i, ectx))
        {
            FLINT_ASSERT(i < A->length - 1);
            continue;
        }

        if (!first)
            flint_printf(" + ");

        first = 0;

        flint_printf("(");
        fq_nmod_poly_print_pretty(A->coeffs + i, yvar, ectx);
        flint_printf(")*%s^%wd", xvar, i);
    }

    if (first)
        flint_printf("0");
}

void nmod_bpoly_fit_length(nmod_bpoly_t A, slong len)
{
    slong i;

    if (len <= A->alloc)
        return;

    if (len < 2 * A->alloc)
        len = 2 * A->alloc;

    if (A->alloc == 0)
        A->coeffs = (nmod_poly_struct *) flint_malloc(len * sizeof(nmod_poly_struct));
    else
        A->coeffs = (nmod_poly_struct *) flint_realloc(A->coeffs, len * sizeof(nmod_poly_struct));

    for (i = A->alloc; i < len; i++)
        nmod_poly_init_mod(A->coeffs + i, A->modulus);

    A->alloc = len;
}

void fq_nmod_bpoly_fit_length(
    fq_nmod_bpoly_t A,
    slong len,
    const fq_nmod_ctx_t ectx)
{
    slong i;

    if (len <= A->alloc)
        return;

    if (len < 2 * A->alloc)
        len = 2 * A->alloc;

    if (A->alloc == 0)
        A->coeffs = (fq_nmod_poly_struct *) flint_malloc(len * sizeof(fq_nmod_poly_struct));
    else
        A->coeffs = (fq_nmod_poly_struct *) flint_realloc(A->coeffs, len * sizeof(fq_nmod_poly_struct));

    for (i = A->alloc; i < len; i++)
        fq_nmod_poly_init(A->coeffs + i, ectx);

    A->alloc = len;
}

void nmod_poly_set_nmod(nmod_poly_t A, ulong c)
{
    FLINT_ASSERT(c < A->mod.n);
    nmod_poly_fit_length(A, 1);
    A->length = (c != 0);
    A->coeffs[0] = c;
}

void nmod_bpoly_set_polyx(nmod_bpoly_t A, const nmod_poly_t B)
{
    slong i;
    nmod_bpoly_fit_length(A, B->length);
    A->length = 0;
    for (i = 0; i < B->length; i++)
    {
        nmod_poly_set_nmod(A->coeffs + i, B->coeffs[i]);
        if (!nmod_poly_is_zero(A->coeffs + i))
            A->length = i + 1;
    }    
}

void fq_nmod_bpoly_set_polyx(
    fq_nmod_bpoly_t A,
    const fq_nmod_poly_t B,
    const fq_nmod_ctx_t ectx)
{
    slong i;
    fq_nmod_bpoly_fit_length(A, B->length, ectx);
    A->length = 0;
    for (i = 0; i < B->length; i++)
    {
        fq_nmod_poly_set_fq_nmod(A->coeffs + i, B->coeffs + i, ectx);
        if (!fq_nmod_poly_is_zero(A->coeffs + i, ectx))
            A->length = i + 1;
    }    
}

void nmod_bpoly_set_polyy(nmod_bpoly_t A, const nmod_poly_t B)
{
    nmod_bpoly_fit_length(A, 1);
	nmod_poly_set(A->coeffs + 0, B);
	A->length = !nmod_poly_is_zero(A->coeffs + 0);
}

void fq_nmod_bpoly_set_polyy(
    fq_nmod_bpoly_t A,
    const fq_nmod_poly_t B,
    const fq_nmod_ctx_t ectx)
{
    fq_nmod_bpoly_fit_length(A, 1, ectx);
	fq_nmod_poly_set(A->coeffs + 0, B, ectx);
	A->length = !fq_nmod_poly_is_zero(A->coeffs + 0, ectx);
}

mp_limb_t nmod_bpoly_get_coeff(const nmod_bpoly_t A, slong xi, slong yi)
{
    if (xi >= A->length)
        return 0;
    else
        return nmod_poly_get_coeff_ui(A->coeffs + xi, yi);
}

void fq_nmod_bpoly_get_coeff(fq_nmod_t c, const fq_nmod_bpoly_t A, slong xi, slong yi, const fq_nmod_ctx_t ectx)
{
    if (xi >= A->length)
        fq_nmod_zero(c, ectx);
    else
        fq_nmod_poly_get_coeff(c, A->coeffs + xi, yi, ectx);
}


void nmod_bpoly_make_monic(nmod_bpoly_t A, slong order)
{
    slong i;
    nmod_poly_t t, lcinv;

    FLINT_ASSERT(A->length > 0);

    nmod_poly_init_mod(t, A->modulus);
    nmod_poly_init_mod(lcinv, A->modulus);
    nmod_poly_inv_series(lcinv, A->coeffs + A->length - 1, order);

    for (i = 0; i < A->length; i++)
    {
        nmod_poly_mullow(t, A->coeffs + i, lcinv, order);
        nmod_poly_swap(A->coeffs + i, t);
    }

    nmod_poly_clear(t);
    nmod_poly_clear(lcinv);
}

void fq_nmod_bpoly_make_monic(fq_nmod_bpoly_t A, slong order, const fq_nmod_ctx_t ctx)
{
    slong i;
    fq_nmod_poly_t t, lcinv;

    FLINT_ASSERT(A->length > 0);

    fq_nmod_poly_init(t, ctx);
    fq_nmod_poly_init(lcinv, ctx);
    fq_nmod_poly_inv_series(lcinv, A->coeffs + A->length - 1, order, ctx);

    for (i = 0; i < A->length; i++)
    {
        fq_nmod_poly_mullow(t, A->coeffs + i, lcinv, order, ctx);
        fq_nmod_poly_swap(A->coeffs + i, t, ctx);
    }

    fq_nmod_poly_clear(t, ctx);
    fq_nmod_poly_clear(lcinv, ctx);
}

void nmod_bpoly_mul(nmod_bpoly_t A, const nmod_bpoly_t B, const nmod_bpoly_t C, slong order)
{
    slong i, j;
    nmod_poly_t t;

    FLINT_ASSERT(A != B);
    FLINT_ASSERT(A != C);

    nmod_poly_init_mod(t, B->modulus);

    nmod_bpoly_fit_length(A, B->length + C->length - 1);
    for (i = 0; i < B->length + C->length - 1; i++)
        nmod_poly_zero(A->coeffs + i);

    for (i = 0; i < B->length; i++)
    {
        for (j = 0; j < C->length; j++)
        {
            nmod_poly_mullow(t, B->coeffs + i, C->coeffs + j, order);
            nmod_poly_add(A->coeffs + i + j, A->coeffs + i + j, t);
        }
    }

    A->length = B->length + C->length - 1;

    nmod_poly_clear(t);
}

void fq_nmod_bpoly_mul(
    fq_nmod_bpoly_t A,
    const fq_nmod_bpoly_t B,
    const fq_nmod_bpoly_t C,
    slong order,
    const fq_nmod_ctx_t ectx)
{
    slong i, j;
    fq_nmod_poly_t t;

    FLINT_ASSERT(A != B);
    FLINT_ASSERT(A != C);

    fq_nmod_poly_init(t, ectx);

    fq_nmod_bpoly_fit_length(A, B->length + C->length - 1, ectx);
    for (i = 0; i < B->length + C->length - 1; i++)
        fq_nmod_poly_zero(A->coeffs + i, ectx);

    for (i = 0; i < B->length; i++)
    {
        for (j = 0; j < C->length; j++)
        {
            fq_nmod_poly_mullow(t, B->coeffs + i, C->coeffs + j, order, ectx);
            fq_nmod_poly_add(A->coeffs + i + j, A->coeffs + i + j, t, ectx);
        }
    }

    A->length = B->length + C->length - 1;
    while (A->length > 0 && fq_nmod_poly_is_zero(A->coeffs + A->length - 1, ectx))
        A->length--;

    fq_nmod_poly_clear(t, ectx);
}

void nmod_bpoly_mul_mod(nmod_bpoly_t A, const nmod_bpoly_t B, const nmod_bpoly_t C, const nmod_poly_t m)
{
    slong i, j;
    nmod_poly_t t;

    FLINT_ASSERT(A != B);
    FLINT_ASSERT(A != C);

    nmod_poly_init_mod(t, B->modulus);

    nmod_bpoly_fit_length(A, B->length + C->length - 1);
    for (i = 0; i < B->length + C->length - 1; i++)
        nmod_poly_zero(A->coeffs + i);

    for (i = 0; i < B->length; i++)
    {
        for (j = 0; j < C->length; j++)
        {
            nmod_poly_mul(t, B->coeffs + i, C->coeffs + j);
            nmod_poly_add(A->coeffs + i + j, A->coeffs + i + j, t);
            nmod_poly_rem(A->coeffs + i + j, A->coeffs + i + j, m);
        }
    }

    A->length = B->length + C->length - 1;

    while (A->length > 0 && nmod_poly_is_zero(A->coeffs + A->length - 1))
        A->length--;

    nmod_poly_clear(t);
}


void nmod_bpoly_add_poly_shift(nmod_bpoly_t A, const nmod_poly_t B, slong yshift)
{
    slong i;

    FLINT_ASSERT(A->length > B->length);

    for (i = 0; i < B->length; i++)
    {
        FLINT_ASSERT(0 == nmod_poly_get_coeff_ui(A->coeffs + i, yshift));
        nmod_poly_set_coeff_ui(A->coeffs + i, yshift, B->coeffs[i]);
    }
}

void fq_nmod_bpoly_add_poly_shift(
    fq_nmod_bpoly_t A,
    const fq_nmod_poly_t B,
    slong yshift,
    const fq_nmod_ctx_t ctx)
{
    slong i;

    FLINT_ASSERT(A->length > B->length);

    for (i = 0; i < B->length; i++)
    {
        fq_nmod_t ccc;
        fq_nmod_init(ccc, ctx);
        fq_nmod_poly_get_coeff(ccc, A->coeffs + i, yshift, ctx);
        FLINT_ASSERT(fq_nmod_is_zero(ccc, ctx));
        fq_nmod_poly_set_coeff(A->coeffs + i, yshift, B->coeffs + i, ctx);
        fq_nmod_clear(ccc, ctx);
    }
}


void nmod_bpoly_sub(nmod_bpoly_t A, const nmod_bpoly_t B, const nmod_bpoly_t C)
{
    slong i;

    FLINT_ASSERT(A != B);
    FLINT_ASSERT(A != C);

    nmod_bpoly_fit_length(A, FLINT_MAX(B->length, C->length));

    A->length = 0;
    for (i = 0; i < FLINT_MAX(B->length, C->length) - 1; i++)
    {
        if (i < B->length)
        {
            if (i < C->length)
            {
                nmod_poly_sub(A->coeffs + i, B->coeffs + i, C->coeffs + i);
            }
            else
            {
                nmod_poly_set(A->coeffs + i, B->coeffs + i);
            }
        }
        else
        {
            FLINT_ASSERT(i < C->length);
            nmod_poly_neg(A->coeffs + i, C->coeffs + i);
        }

        if (!nmod_poly_is_zero(A->coeffs + i))
            A->length = i + 1;
    }
}

void fq_nmod_bpoly_sub(
    fq_nmod_bpoly_t A,
    const fq_nmod_bpoly_t B,
    const fq_nmod_bpoly_t C,
    const fq_nmod_ctx_t ectx)
{
    slong i;

    FLINT_ASSERT(A != B);
    FLINT_ASSERT(A != C);

    fq_nmod_bpoly_fit_length(A, FLINT_MAX(B->length, C->length), ectx);
    A->length = 0;
    for (i = 0; i < FLINT_MAX(B->length, C->length) - 1; i++)
    {
        if (i < B->length)
        {
            if (i < C->length)
            {
                fq_nmod_poly_sub(A->coeffs + i, B->coeffs + i, C->coeffs + i, ectx);
            }
            else
            {
                fq_nmod_poly_set(A->coeffs + i, B->coeffs + i, ectx);
            }
        }
        else
        {
            FLINT_ASSERT(i < C->length);
            fq_nmod_poly_neg(A->coeffs + i, C->coeffs + i, ectx);
        }

        if (!fq_nmod_poly_is_zero(A->coeffs + i, ectx))
            A->length = i + 1;
    }
}

void nmod_bpoly_set(nmod_bpoly_t A, const nmod_bpoly_t B)
{
    slong i;

    FLINT_ASSERT(A != B);

    nmod_bpoly_fit_length(A, B->length);
    A->length = B->length;

    for (i = 0; i < B->length; i++)
        nmod_poly_set(A->coeffs + i, B->coeffs + i);
}

void fq_nmod_bpoly_set(fq_nmod_bpoly_t A, const fq_nmod_bpoly_t B, const fq_nmod_ctx_t ctx)
{
    slong i;

    FLINT_ASSERT(A != B);

    fq_nmod_bpoly_fit_length(A, B->length, ctx);
    A->length = B->length;

    for (i = 0; i < B->length; i++)
        fq_nmod_poly_set(A->coeffs + i, B->coeffs + i, ctx);
}

int nmod_bpoly_divides(nmod_bpoly_t Q, const nmod_bpoly_t A, const nmod_bpoly_t B)
{
    slong i, qoff;
    int divides;
    nmod_poly_t q, t;
    nmod_bpoly_t R;

    FLINT_ASSERT(Q != A);
    FLINT_ASSERT(Q != B);
    FLINT_ASSERT(B->length > 0);

    nmod_poly_init_mod(q, A->modulus);
    nmod_poly_init_mod(t, A->modulus);
    nmod_bpoly_init(R, A->modulus);
    nmod_bpoly_set(R, A);

    Q->length = 0;

    while (R->length >= B->length)
    {
        nmod_poly_divrem(q, t, R->coeffs + R->length - 1, B->coeffs + B->length - 1);
        if (!nmod_poly_is_zero(t))
        {
            divides = 0;
            goto cleanup;
        }

        for (i = 0; i < B->length; i++)
        {
            nmod_poly_mul(t, B->coeffs + i, q);
            nmod_poly_sub(R->coeffs + i + R->length - B->length, R->coeffs + i + R->length - B->length, t);
        }

        qoff = R->length - B->length;

        FLINT_ASSERT(qoff >= 0);
        if (qoff >= Q->length)
        {
            nmod_bpoly_fit_length(Q, qoff + 1);
            for (i = Q->length; i <= qoff; i++)
                nmod_poly_zero(Q->coeffs + i);
            Q->length = qoff + 1;
        }

        nmod_poly_set(Q->coeffs + qoff, q);

        while (R->length > 0 && nmod_poly_is_zero(R->coeffs + R->length - 1))
            R->length--;
    }

    divides = (R->length == 0);

cleanup:

    nmod_poly_clear(q);
    nmod_poly_clear(t);
    nmod_bpoly_clear(R);

    return divides;
}

int fq_nmod_bpoly_divides(
    fq_nmod_bpoly_t Q,
    const fq_nmod_bpoly_t A,
    const fq_nmod_bpoly_t B,
    const fq_nmod_ctx_t ctx)
{
    slong i, qoff;
    int divides;
    fq_nmod_poly_t q, t;
    fq_nmod_bpoly_t R;

    FLINT_ASSERT(Q != A);
    FLINT_ASSERT(Q != B);
    FLINT_ASSERT(B->length > 0);

    fq_nmod_poly_init(q, ctx);
    fq_nmod_poly_init(t, ctx);
    fq_nmod_bpoly_init(R, ctx);
    fq_nmod_bpoly_set(R, A, ctx);

    Q->length = 0;

    while (R->length >= B->length)
    {
        fq_nmod_poly_divrem(q, t, R->coeffs + R->length - 1, B->coeffs + B->length - 1, ctx);
        if (!fq_nmod_poly_is_zero(t, ctx))
        {
            divides = 0;
            goto cleanup;
        }

        for (i = 0; i < B->length; i++)
        {
            fq_nmod_poly_mul(t, B->coeffs + i, q, ctx);
            fq_nmod_poly_sub(R->coeffs + i + R->length - B->length, R->coeffs + i + R->length - B->length, t, ctx);
        }

        qoff = R->length - B->length;

        FLINT_ASSERT(qoff >= 0);
        if (qoff >= Q->length)
        {
            fq_nmod_bpoly_fit_length(Q, qoff + 1, ctx);
            for (i = Q->length; i <= qoff; i++)
                fq_nmod_poly_zero(Q->coeffs + i, ctx);
            Q->length = qoff + 1;
        }

        fq_nmod_poly_set(Q->coeffs + qoff, q, ctx);

        while (R->length > 0 && fq_nmod_poly_is_zero(R->coeffs + R->length - 1, ctx))
            R->length--;
    }

    divides = (R->length == 0);

cleanup:

    fq_nmod_poly_clear(q, ctx);
    fq_nmod_poly_clear(t, ctx);
    fq_nmod_bpoly_clear(R, ctx);

    return divides;
}

void nmod_bpoly_make_primitive(nmod_bpoly_t A)
{
    slong i;
    nmod_poly_t g, q;

    nmod_poly_init_mod(g, A->modulus);
    nmod_poly_init_mod(q, A->modulus);

    for (i = 0; i < A->length; i++)
	{
        nmod_poly_gcd(q, g, A->coeffs + i);
		nmod_poly_swap(g, q);
	}

    for (i = 0; i < A->length; i++)
    {
        nmod_poly_div(q, A->coeffs + i, g);
		nmod_poly_swap(A->coeffs + i, q);
    }

    nmod_poly_clear(g);
    nmod_poly_clear(q);
}

void fq_nmod_bpoly_make_primitive(fq_nmod_bpoly_t A, const fq_nmod_ctx_t ctx)
{
    slong i;
    fq_nmod_poly_t g, q, r;

    fq_nmod_poly_init(g, ctx);
    fq_nmod_poly_init(q, ctx);
    fq_nmod_poly_init(r, ctx);

    for (i = 0; i < A->length; i++)
	{
        fq_nmod_poly_gcd(q, g, A->coeffs + i, ctx);
		fq_nmod_poly_swap(g, q, ctx);
	}

    for (i = 0; i < A->length; i++)
    {
        fq_nmod_poly_divrem(q, r, A->coeffs + i, g, ctx);
		fq_nmod_poly_swap(A->coeffs + i, q, ctx);
    }

    fq_nmod_poly_clear(g, ctx);
    fq_nmod_poly_clear(q, ctx);
    fq_nmod_poly_clear(r, ctx);
}


void nmod_bpoly_set_coeff(nmod_bpoly_t A, slong xi, slong yi, mp_limb_t c)
{
    slong i;

    FLINT_ASSERT(0 != c);

    if (xi >= A->length)
    {
        nmod_bpoly_fit_length(A, xi + 1);
        for (i = A->length; i <= xi; i++)
            nmod_poly_zero(A->coeffs + i);
        A->length = xi + 1;
    }

    nmod_poly_set_coeff_ui(A->coeffs + xi, yi, c);
}

void fq_nmod_bpoly_set_coeff(
    fq_nmod_bpoly_t A,
    slong xi, slong yi,
    const fq_nmod_t c,
    const fq_nmod_ctx_t ctx)
{
    slong i;

    FLINT_ASSERT(0 != c);

    if (xi >= A->length)
    {
        fq_nmod_bpoly_fit_length(A, xi + 1, ctx);
        for (i = A->length; i <= xi; i++)
            fq_nmod_poly_zero(A->coeffs + i, ctx);
        A->length = xi + 1;
    }

    fq_nmod_poly_set_coeff(A->coeffs + xi, yi, c, ctx);
}


void nmod_bpoly_zero(nmod_bpoly_t A)
{
    A->length = 0;
}

void fq_nmod_bpoly_zero(fq_nmod_bpoly_t A, const fq_nmod_ctx_t fqctx)
{
    A->length = 0;
}

void nmod_mpoly_to_bpoly(
    nmod_bpoly_t A,
    const nmod_mpoly_t B,
    slong varx,
    slong vary,
    const nmod_mpoly_ctx_t ctx)
{
    slong j;
    slong NB;
    ulong Bexpx, Bexpy;
    slong Boffx, Bshiftx, Boffy, Bshifty;
    ulong mask;

    FLINT_ASSERT(B->bits <= FLINT_BITS);
    NB = mpoly_words_per_exp_sp(B->bits, ctx->minfo);

    mpoly_gen_offset_shift_sp(&Boffx, &Bshiftx, varx, B->bits, ctx->minfo);
    mpoly_gen_offset_shift_sp(&Boffy, &Bshifty, vary, B->bits, ctx->minfo);
    mask = (-UWORD(1)) >> (FLINT_BITS - B->bits);

    nmod_bpoly_zero(A);

    for (j = 0; j < B->length; j++)
    {
        Bexpx = ((B->exps + NB*j)[Boffx] >> Bshiftx) & mask;
        Bexpy = ((B->exps + NB*j)[Boffy] >> Bshifty) & mask;
        nmod_bpoly_set_coeff(A, Bexpx, Bexpy, B->coeffs[j]);
    }
}

void fq_nmod_mpoly_to_bpoly(
    fq_nmod_bpoly_t A,
    const fq_nmod_mpoly_t B,
    slong varx,
    slong vary,
    const fq_nmod_mpoly_ctx_t ctx)
{
    slong j;
    slong NB;
    ulong Bexpx, Bexpy;
    slong Boffx, Bshiftx, Boffy, Bshifty;
    ulong mask;

    FLINT_ASSERT(B->bits <= FLINT_BITS);
    NB = mpoly_words_per_exp_sp(B->bits, ctx->minfo);

    mpoly_gen_offset_shift_sp(&Boffx, &Bshiftx, varx, B->bits, ctx->minfo);
    mpoly_gen_offset_shift_sp(&Boffy, &Bshifty, vary, B->bits, ctx->minfo);
    mask = (-UWORD(1)) >> (FLINT_BITS - B->bits);

    fq_nmod_bpoly_zero(A, ctx->fqctx);

    for (j = 0; j < B->length; j++)
    {
        Bexpx = ((B->exps + NB*j)[Boffx] >> Bshiftx) & mask;
        Bexpy = ((B->exps + NB*j)[Boffy] >> Bshifty) & mask;
        fq_nmod_bpoly_set_coeff(A, Bexpx, Bexpy, B->coeffs + j, ctx->fqctx);
    }
}


void nmod_mpoly_from_nmod_bpoly(
    nmod_mpoly_t A,
    flint_bitcnt_t Abits,
    const nmod_bpoly_t B,
    slong varx,
    slong vary,
    const nmod_mpoly_ctx_t ctx)
{
    slong n = ctx->minfo->nvars;
    slong i, j;
    slong NA;
    slong Alen;
    mp_limb_t * Acoeff;
    ulong * Aexp;
    slong Aalloc;
    ulong * Aexps;
    TMP_INIT;

    FLINT_ASSERT(B->length > 0);
    FLINT_ASSERT(Abits <= FLINT_BITS);
    TMP_START;

    Aexps = (ulong *) TMP_ALLOC(n*sizeof(ulong));
    for (i = 0; i < n; i++)
        Aexps[i] = 0;

    NA = mpoly_words_per_exp(Abits, ctx->minfo);
    nmod_mpoly_fit_bits(A, Abits, ctx);
    A->bits = Abits;

    Acoeff = A->coeffs;
    Aexp = A->exps;
    Aalloc = A->alloc;
    Alen = 0;
    for (i = 0; i < B->length; i++)
    {
        nmod_poly_struct * Bc = B->coeffs + i;
        _nmod_mpoly_fit_length(&Acoeff, &Aexp, &Aalloc, Alen + Bc->length, NA);

        for (j = 0; j < Bc->length; j++)
        {
            if (0 == Bc->coeffs[j])
                continue;

            Aexps[varx] = i;
            Aexps[vary] = j;
            Acoeff[Alen] = Bc->coeffs[j];
            mpoly_set_monomial_ui(Aexp + NA*Alen, Aexps, Abits, ctx->minfo);
            Alen++;
        }
    }
    A->coeffs = Acoeff;
    A->exps = Aexp;
    A->alloc = Aalloc;
    A->length = Alen;

    TMP_END;

    nmod_mpoly_sort_terms(A, ctx);
}

void fq_nmod_mpoly_from_fq_nmod_bpoly(
    fq_nmod_mpoly_t A,
    flint_bitcnt_t Abits,
    const fq_nmod_bpoly_t B,
    slong varx,
    slong vary,
    const fq_nmod_mpoly_ctx_t ctx)
{
    slong n = ctx->minfo->nvars;
    slong i, j;
    slong NA;
    slong Alen;
    fq_nmod_struct * Acoeff;
    ulong * Aexp;
    slong Aalloc;
    ulong * Aexps;
    TMP_INIT;

    FLINT_ASSERT(B->length > 0);
    FLINT_ASSERT(Abits <= FLINT_BITS);
    TMP_START;

    Aexps = (ulong *) TMP_ALLOC(n*sizeof(ulong));
    for (i = 0; i < n; i++)
        Aexps[i] = 0;

    NA = mpoly_words_per_exp(Abits, ctx->minfo);
    fq_nmod_mpoly_fit_bits(A, Abits, ctx);
    A->bits = Abits;

    Acoeff = A->coeffs;
    Aexp = A->exps;
    Aalloc = A->alloc;
    Alen = 0;
    for (i = 0; i < B->length; i++)
    {
        fq_nmod_poly_struct * Bc = B->coeffs + i;
        _fq_nmod_mpoly_fit_length(&Acoeff, &Aexp, &Aalloc, Alen + Bc->length, NA, ctx->fqctx);

        for (j = 0; j < Bc->length; j++)
        {
            if (fq_nmod_is_zero(Bc->coeffs + j, ctx->fqctx))
                continue;

            Aexps[varx] = i;
            Aexps[vary] = j;
            fq_nmod_set(Acoeff + Alen, Bc->coeffs + j, ctx->fqctx);
            mpoly_set_monomial_ui(Aexp + NA*Alen, Aexps, Abits, ctx->minfo);
            Alen++;
        }
    }
    A->coeffs = Acoeff;
    A->exps = Aexp;
    A->alloc = Aalloc;
    A->length = Alen;

    TMP_END;

    fq_nmod_mpoly_sort_terms(A, ctx);
}


void nmod_bpoly_eval(nmod_poly_t E, const nmod_bpoly_t A, mp_limb_t alpha)
{
    slong i;

    nmod_poly_zero(E);
    for (i = A->length - 1; i >= 0; i--)
    {
        nmod_poly_set_coeff_ui(E, i,
                                nmod_poly_evaluate_nmod(A->coeffs + i, alpha));
    }
}

void fq_nmod_bpoly_eval(
    fq_nmod_poly_t E,
    const fq_nmod_bpoly_t A,
    const fq_nmod_t alpha,
    const fq_nmod_ctx_t ctx)
{
    slong i;
    fq_nmod_t t;
    fq_nmod_init(t, ctx);
    fq_nmod_poly_zero(E, ctx);
    for (i = A->length - 1; i >= 0; i--)
    {
        fq_nmod_poly_evaluate_fq_nmod(t, A->coeffs + i, alpha, ctx);
        fq_nmod_poly_set_coeff(E, i, t, ctx);
    }
    fq_nmod_clear(t, ctx);
}

void nmod_bpoly_taylor_shift(nmod_bpoly_t A, mp_limb_t alpha)
{
    slong i;
    for (i = A->length - 1; i >= 0; i--)
        nmod_poly_taylor_shift(A->coeffs + i, A->coeffs + i, alpha);    
}

void _fq_nmod_poly_taylor_shift_horner(
    fq_nmod_struct * poly,
    const fq_nmod_t c,
    slong n,
    const fq_nmod_ctx_t ctx)
{
    slong i, j;
    fq_nmod_t p;
    fq_nmod_init(p, ctx);
    for (i = n - 2; i >= 0; i--)
    {
        for (j = i; j < n - 1; j++)
        {
            fq_nmod_mul(p, poly + j + 1, c, ctx);
            fq_nmod_add(poly + j, poly + j, p, ctx);
        }
    }
    fq_nmod_clear(p, ctx);
}

void fq_nmod_poly_taylor_shift_horner(
    fq_nmod_poly_t g,
    const fq_nmod_poly_t f,
    const fq_nmod_t c,
    const fq_nmod_ctx_t ctx)
{
    if (f != g)
        fq_nmod_poly_set(g, f, ctx);

    _fq_nmod_poly_taylor_shift_horner(g->coeffs, c, g->length, ctx);
}


void fq_nmod_bpoly_taylor_shift(
    fq_nmod_bpoly_t A,
    const fq_nmod_t alpha,
    const fq_nmod_ctx_t ctx)
{
    slong i;
    for (i = A->length - 1; i >= 0; i--)
        fq_nmod_poly_taylor_shift_horner(A->coeffs + i, A->coeffs + i, alpha, ctx);    
}




/*****************************************************************************/



typedef struct {
    slong r; /* number of local factors */
    nmod_t modulus;
    nmod_bpoly_t Btilde;
    nmod_bpoly_struct * newBitilde;
    nmod_poly_struct * P;
    nmod_poly_struct * d;
    nmod_poly_struct * Bitilde;
} nmod_bpoly_info_struct;

typedef nmod_bpoly_info_struct nmod_bpoly_info_t[1];

typedef struct {
    slong r; /* number of local factors */
    fq_nmod_bpoly_t Btilde;
    fq_nmod_bpoly_struct * newBitilde;
    fq_nmod_poly_struct * P;
    fq_nmod_poly_struct * d;
    fq_nmod_poly_struct * Bitilde;
} fq_nmod_smprime_info_struct;

typedef fq_nmod_smprime_info_struct fq_nmod_smprime_info_t[1];

typedef struct {
    slong r; /* number of local factors */
    nmod_bpoly_t Btilde;
    nmod_bpoly_struct * newBitilde;
    fq_nmod_poly_struct * d;
    fq_nmod_poly_struct * Bitilde;
} nmod_lgprime_info_struct;

typedef nmod_lgprime_info_struct nmod_lgprime_info_t[1];


void nmod_bpoly_info_init(nmod_bpoly_info_t I, slong r, nmod_t modulus)
{
    slong i;

    FLINT_ASSERT(r >= 2);

    I->r = r;

    I->modulus = modulus;

    nmod_bpoly_init(I->Btilde, I->modulus);

    I->newBitilde = (nmod_bpoly_struct *) flint_malloc(I->r*sizeof(nmod_bpoly_struct));
    I->P          = (nmod_poly_struct *) flint_malloc(I->r*sizeof(nmod_poly_struct));
    I->d          = (nmod_poly_struct *) flint_malloc(I->r*sizeof(nmod_poly_struct));
    I->Bitilde    = (nmod_poly_struct *) flint_malloc(I->r*sizeof(nmod_poly_struct));

    for (i = 0; i < I->r; i++)
    {
        nmod_bpoly_init(I->newBitilde + i, I->modulus);
        nmod_poly_init_mod(I->P + i, I->modulus);
        nmod_poly_init_mod(I->d + i, I->modulus);
        nmod_poly_init_mod(I->Bitilde + i, I->modulus);
    }
}

void fq_nmod_smprime_info_init(
    fq_nmod_smprime_info_t I,
    slong r,
    const fq_nmod_ctx_t fqctx)
{
    slong i;

    FLINT_ASSERT(r >= 2);

    I->r = r;

    fq_nmod_bpoly_init(I->Btilde, fqctx);

    I->newBitilde = (fq_nmod_bpoly_struct *) flint_malloc(I->r*sizeof(fq_nmod_bpoly_struct));
    I->d          = (fq_nmod_poly_struct *) flint_malloc(I->r*sizeof(fq_nmod_poly_struct));
    I->Bitilde    = (fq_nmod_poly_struct *) flint_malloc(I->r*sizeof(fq_nmod_poly_struct));

    for (i = 0; i < I->r; i++)
    {
        fq_nmod_bpoly_init(I->newBitilde + i, fqctx);
        fq_nmod_poly_init(I->d + i, fqctx);
        fq_nmod_poly_init(I->Bitilde + i, fqctx);
    }
}

void nmod_lgprime_info_init(
    nmod_lgprime_info_t I,
    slong r,
    const fq_nmod_ctx_t ectx)
{
    slong i;

    FLINT_ASSERT(r >= 2);

    I->r = r;

    nmod_bpoly_init(I->Btilde, ectx->modulus->mod);

    I->newBitilde = (nmod_bpoly_struct *) flint_malloc(I->r*sizeof(nmod_bpoly_struct));
    I->d          = (fq_nmod_poly_struct *) flint_malloc(I->r*sizeof(fq_nmod_poly_struct));
    I->Bitilde    = (fq_nmod_poly_struct *) flint_malloc(I->r*sizeof(fq_nmod_poly_struct));

    for (i = 0; i < I->r; i++)
    {
        nmod_bpoly_init(I->newBitilde + i, ectx->modulus->mod);
        fq_nmod_poly_init(I->d + i, ectx);
        fq_nmod_poly_init(I->Bitilde + i, ectx);
    }
}


void nmod_bpoly_info_clear(nmod_bpoly_info_t I)
{
    slong i;

    nmod_bpoly_clear(I->Btilde);

    for (i = 0; i < I->r; i++)
    {
        nmod_bpoly_clear(I->newBitilde + i);
        nmod_poly_clear(I->P + i);
        nmod_poly_clear(I->d + i);
        nmod_poly_clear(I->Bitilde + i);
    }

    flint_free(I->newBitilde);
    flint_free(I->P);
    flint_free(I->d);
    flint_free(I->Bitilde);
}

void fq_nmod_smprime_info_clear(fq_nmod_smprime_info_t I,
    const fq_nmod_ctx_t fqctx)
{
    slong i;

    fq_nmod_bpoly_clear(I->Btilde, fqctx);

    for (i = 0; i < I->r; i++)
    {
        fq_nmod_bpoly_clear(I->newBitilde + i, fqctx);
        fq_nmod_poly_clear(I->d + i, fqctx);
        fq_nmod_poly_clear(I->Bitilde + i, fqctx);
    }

    flint_free(I->newBitilde);
    flint_free(I->d);
    flint_free(I->Bitilde);
}

void nmod_lgprime_info_clear(
    nmod_lgprime_info_t I,
    const fq_nmod_ctx_t ectx)
{
    slong i;

    nmod_bpoly_clear(I->Btilde);

    for (i = 0; i < I->r; i++)
    {
        nmod_bpoly_clear(I->newBitilde + i);
        fq_nmod_poly_clear(I->d + i, ectx);
        fq_nmod_poly_clear(I->Bitilde + i, ectx);
    }

    flint_free(I->newBitilde);
    flint_free(I->d);
    flint_free(I->Bitilde);
}




void nmod_poly_factor_print_pretty(const nmod_poly_factor_t f, const char * x)
{
    slong i;
    printf("1");
    for (i = 0; i < f->num; i++)
    {
        flint_printf("*(");
        nmod_poly_print_pretty(f->p + i, "x");
        flint_printf(")^%wd", f->exp[i]);
    }
}




/*
    set out[i] so that
    1/(f[0]*f[1]*...*f[n-1]) = out[0]/f[0] + ... + out[n-1]/f[n-1]
*/
int nmod_partial_fraction_coeffs(
    nmod_poly_struct * out,
    const nmod_poly_struct * f,
    nmod_t mod,
    slong n)
{
    slong i;
    nmod_poly_t num, den, a, b, g, t;

    FLINT_ASSERT(n > 1);

    nmod_poly_init_mod(num, mod);
    nmod_poly_init_mod(den, mod);
    nmod_poly_init_mod(a, mod);
    nmod_poly_init_mod(b, mod);
    nmod_poly_init_mod(g, mod);
    nmod_poly_init_mod(t, mod);

    nmod_poly_one(num);
    nmod_poly_mul(den, f + 0, f + 1);
    for (i = 2; i < n; i++)
        nmod_poly_mul(den, den, f + i);

    for (i = 0; i < n; i++)
    {
        nmod_poly_divrem(den, t, den, f + i);
        FLINT_ASSERT(nmod_poly_is_zero(t));
        nmod_poly_xgcd(g, a, b, f + i, den);
        if (nmod_poly_degree(g) != 0)
            return 0;
        FLINT_ASSERT(nmod_poly_is_one(g));
        nmod_poly_mul(t, b, num);
        nmod_poly_rem(out + i, t, f + i);
        nmod_poly_mul(t, a, num);
        nmod_poly_rem(num, t, den);
    }

    nmod_poly_clear(num);
    nmod_poly_clear(den);
    nmod_poly_clear(a);
    nmod_poly_clear(b);
    nmod_poly_clear(g);
    nmod_poly_clear(t);
    return 1;
}

/*
    set out[i] so that
    1/(f[0]*f[1]*...*f[n-1]) = out[0]/f[0] + ... + out[n-1]/f[n-1]
*/
int fq_nmod_partial_fraction_coeffs(
    fq_nmod_poly_struct * out,
    const fq_nmod_poly_struct * f,
    slong n,
    const fq_nmod_ctx_t ectx)
{
    slong i;
    fq_nmod_poly_t num, den, a, b, g, t;

    FLINT_ASSERT(n > 1);

    fq_nmod_poly_init(num, ectx);
    fq_nmod_poly_init(den, ectx);
    fq_nmod_poly_init(a, ectx);
    fq_nmod_poly_init(b, ectx);
    fq_nmod_poly_init(g, ectx);
    fq_nmod_poly_init(t, ectx);

    fq_nmod_poly_one(num, ectx);
    fq_nmod_poly_mul(den, f + 0, f + 1, ectx);
    for (i = 2; i < n; i++)
        fq_nmod_poly_mul(den, den, f + i, ectx);

    for (i = 0; i < n; i++)
    {
        fq_nmod_poly_divrem(den, t, den, f + i, ectx);
        FLINT_ASSERT(fq_nmod_poly_is_zero(t, ectx));
        fq_nmod_poly_xgcd(g, a, b, f + i, den, ectx);
        if (fq_nmod_poly_degree(g, ectx) != 0)
            return 0;
        FLINT_ASSERT(fq_nmod_poly_is_one(g, ectx));
        fq_nmod_poly_mul(t, b, num, ectx);
        fq_nmod_poly_rem(out + i, t, f + i, ectx);
        fq_nmod_poly_mul(t, a, num, ectx);
        fq_nmod_poly_rem(num, t, den, ectx);
    }

    fq_nmod_poly_clear(num, ectx);
    fq_nmod_poly_clear(den, ectx);
    fq_nmod_poly_clear(a, ectx);
    fq_nmod_poly_clear(b, ectx);
    fq_nmod_poly_clear(g, ectx);
    fq_nmod_poly_clear(t, ectx);
    return 1;
}

int nmod_bpoly_info_disolve(nmod_bpoly_info_t I)
{
    return nmod_partial_fraction_coeffs(I->d, I->Bitilde, I->modulus, I->r);
}

int fq_nmod_smprime_info_disolve(fq_nmod_smprime_info_t I, const fq_nmod_ctx_t fqctx)
{
    return fq_nmod_partial_fraction_coeffs(I->d, I->Bitilde, I->r, fqctx);
}

int nmod_lgprime_info_disolve(
    nmod_lgprime_info_t I,
    const fq_nmod_ctx_t ectx)
{
    return fq_nmod_partial_fraction_coeffs(I->d, I->Bitilde, I->r, ectx);
}


static int _irreducible_bivar_factors_smprime(
    nmod_mpoly_factor_t fac,
    const nmod_mpoly_t A,
    slong xvar,
    slong yvar,
    const nmod_mpoly_ctx_t ctx)
{
    slong i, j;
    ulong k;
    mp_limb_t alpha;
    nmod_poly_t Beval;
    nmod_bpoly_t B;
    nmod_poly_factor_t Bevalfac;
    slong Blengthx, Blengthy;
    nmod_bpoly_info_t I;
    nmod_bpoly_t tp, tp1, error;
    nmod_poly_t ss, tt;

    nmod_mpoly_factor_one(fac, ctx);

    nmod_poly_init_mod(Beval, ctx->ffinfo->mod);
    nmod_poly_factor_init(Bevalfac);
    nmod_bpoly_init(B, ctx->ffinfo->mod);
    nmod_bpoly_info_init(I, 2, ctx->ffinfo->mod);

    nmod_mpoly_to_bpoly(B, A, xvar, yvar, ctx);
/*
printf("B: "); nmod_bpoly_print(B, "x", "y"); printf("\n");
*/
    Blengthx = B->length;
    FLINT_ASSERT(Blengthx > 1);
/*
flint_printf("Blengthx: %wd\n");
*/
    alpha = 0;
    goto got_alpha;

next_alpha:

    alpha++;
    if (alpha >= ctx->ffinfo->mod.n)
    {
        /* TODO leak */
        return 0;
    }

got_alpha:
/*
flint_printf("<_irreducible_bivar_factors> trying alpha = %wu\n", alpha);
*/
        nmod_bpoly_eval(Beval, B, alpha);
/*
printf("Beval: "); nmod_poly_print_pretty(Beval, "x"); printf("\n");
*/
        /* if killed leading coeff, get new alpha */
        if (Beval->length != Blengthx)
            goto next_alpha;

        Bevalfac->num = 0;
        nmod_poly_factor(Bevalfac, Beval);
/*
flint_printf("<_append_irreducible_bivar_factors> Bevalfac: "); nmod_poly_factor_print_pretty(Bevalfac, "x"); printf("\n");
*/
        /* if multiple factors, get new alpha */
        for (i = 0; i < Bevalfac->num; i++)
        {
            if (Bevalfac->exp[i] != 1)
                goto next_alpha;
        }

		/* if one factor, A is irreducible */
		if (Bevalfac->num == 1)
		{
			nmod_mpoly_factor_append_ui(fac, A, 1, ctx);
            /* TODO leak */
			return 1;
		}

/*
flint_printf("**************\ngood alpha: %wu\n", alpha);
*/
        nmod_bpoly_taylor_shift(B, alpha);
/*
flint_printf("B: "); nmod_bpoly_print(B, "x", "y"); printf("\n");
*/

        Blengthy = 0;
        for (i = 0; i < B->length; i++)
        {
            Blengthy = FLINT_MAX(Blengthy, (B->coeffs + i)->length);
        }
/*
flint_printf("Blengthy: %wd\n", Blengthy);
*/
        nmod_bpoly_info_clear(I);
        nmod_bpoly_info_init(I, Bevalfac->num, ctx->ffinfo->mod);
/*
flint_printf("I->r : %wd\n", I->r);
*/
        nmod_bpoly_set(I->Btilde, B);
/*
printf("I->Btilde: "); nmod_bpoly_print(I->Btilde, "x", "y"); printf("\n");
*/
        nmod_bpoly_make_monic(I->Btilde, Blengthy);
/*
printf("I->Btilde: "); nmod_bpoly_print(I->Btilde, "x", "y"); printf("\n");
*/


        for (i = 0; i < I->r; i++)
        {
            nmod_poly_make_monic(I->Bitilde + i, Bevalfac->p + i);
/*
flint_printf("   I->Bitilde[%wd]: ", i); nmod_poly_print_pretty(I->Bitilde + i, "x"); printf("\n");
*/
            nmod_bpoly_set_polyx(I->newBitilde + i, I->Bitilde + i);
/*
flint_printf("I->newBitilde[%wd]: ", i); nmod_bpoly_print(I->newBitilde + i, "x", "y"); printf("\n");
*/
        }

        nmod_bpoly_info_disolve(I);

/*
        for (i = 0; i < I->r; i++)
        {
flint_printf("  I->d[%wd]: ", i); nmod_poly_print_pretty(I->d + i, "x"); printf("\n");
flint_printf("  I->P[%wd]: ", i); nmod_poly_print_pretty(I->P + i, "x"); printf("\n");

        }
*/


        FLINT_ASSERT(I->r > 1);

        nmod_poly_init_mod(ss, I->modulus);
        nmod_poly_init_mod(tt, I->modulus);
        nmod_bpoly_init(tp, I->modulus);
        nmod_bpoly_init(tp1, I->modulus);
        nmod_bpoly_init(error, I->modulus);

        nmod_bpoly_mul(tp, I->newBitilde + 0, I->newBitilde + 1, Blengthy);
        for (i = 2; i < I->r; i++)
        {
            nmod_bpoly_mul(tp1, tp, I->newBitilde + i, Blengthy);
            nmod_bpoly_swap(tp1, tp);
        }

        nmod_bpoly_sub(error, I->Btilde, tp);
/*
flint_printf("error: "); nmod_bpoly_print(error, "x", "y"); printf("\n");
*/
        for (j = 1; j < Blengthy; j++)
        {
            nmod_poly_zero(ss);
            for (i = error->length - 1; i >= 0; i--)
            {
                nmod_poly_set_coeff_ui(ss, i, nmod_bpoly_get_coeff(error, i, j));
                for (k = 0; k < j; k++)
                {
                    FLINT_ASSERT(0 == nmod_bpoly_get_coeff(error, i, k));
                }
            }

            for (i = 0; i < I->r; i++)
            {
                nmod_poly_mul(tt, ss, I->d + i);
                nmod_poly_rem(tt, tt, I->Bitilde + i);
                nmod_bpoly_add_poly_shift(I->newBitilde + i, tt, j);
            }

            nmod_bpoly_mul(tp, I->newBitilde + 0, I->newBitilde + 1, Blengthy);
            for (i = 2; i < I->r; i++)
            {
                nmod_bpoly_mul(tp1, tp, I->newBitilde + i, Blengthy);
                nmod_bpoly_swap(tp1, tp);
            }
            nmod_bpoly_sub(error, I->Btilde, tp);
/*
flint_printf("j = %wd lift: error: ", j); nmod_bpoly_print(error, "x", "y"); printf("\n");
*/
        }

        nmod_poly_clear(ss);
        nmod_poly_clear(tt);
        nmod_bpoly_clear(tp);
        nmod_bpoly_clear(tp1);
        nmod_bpoly_clear(error);

/*
printf("error: "); nmod_bpoly_print(error, "x", "y"); printf("\n");
for (i = 0; i < I->r; i++)
{
flint_printf("I->newBitilde[%wd]: ", i); nmod_bpoly_print(I->newBitilde + i, "x", "y"); printf("\n");
}
*/
        {
            nmod_bpoly_t f, Q, R, trymez;
            nmod_bpoly_t tryme, trymet;
            nmod_poly_t leadf;
            slong kk, *used_arr, *sub_arr;

            used_arr = (slong *) flint_calloc(2 * I->r, sizeof(slong));
            sub_arr  = used_arr + I->r;

            nmod_bpoly_init(f, ctx->ffinfo->mod);
            nmod_bpoly_init(Q, ctx->ffinfo->mod);
            nmod_bpoly_init(R, ctx->ffinfo->mod);
            nmod_bpoly_init(trymez, ctx->ffinfo->mod);
            nmod_bpoly_init(tryme, ctx->ffinfo->mod);
            nmod_bpoly_init(trymet, ctx->ffinfo->mod);
            nmod_poly_init_mod(leadf, ctx->ffinfo->mod);

            nmod_bpoly_set(f, B);
            FLINT_ASSERT(f->length > 0);
            nmod_poly_set(leadf, f->coeffs + f->length - 1);

            for (kk = 1; kk < I->r; kk++)
            {
                slong count = 0, indx = kk - 1, l;

                for(l = 0; l < kk; l++)
                    sub_arr[l] = l;

                sub_arr[indx]--;
                while ((indx >= 0))
                {
                    sub_arr[indx] = sub_arr[indx] + 1;

                    for (l = indx + 1; l < kk; l++)
                        sub_arr[l] = sub_arr[l - 1] + 1;

                    if (sub_arr[kk - 1] > I->r - 1)
                        indx--;
                    else
                    {
                        for(l = 0; l < kk; l++)
                        {
                            if (used_arr[sub_arr[l]] == 1)
                                break;
                        }

                        nmod_bpoly_set_polyy(tryme, leadf);
                        for (l = 0; l < kk; l++)
                        {
                            nmod_bpoly_mul(trymet, tryme, I->newBitilde + sub_arr[l], Blengthy);
                            nmod_bpoly_swap(trymet, tryme);
                        }

                        nmod_bpoly_set(trymez, tryme);
                        nmod_bpoly_make_primitive(trymez);
                        if (nmod_bpoly_divides(Q, f, trymez))
                        {
                            nmod_mpoly_t goodtry;

                            if (alpha != 0)
                                nmod_bpoly_taylor_shift(trymez, ctx->ffinfo->mod.n - alpha);
                            nmod_mpoly_init(goodtry, ctx);
                            nmod_mpoly_from_nmod_bpoly(goodtry, A->bits, trymez, xvar, yvar, ctx);
                            nmod_mpoly_make_monic(goodtry, goodtry, ctx);
                            nmod_mpoly_factor_append_ui(fac, goodtry, 1, ctx);
                            nmod_mpoly_clear(goodtry, ctx);

                            for(l = 0; l < kk; l++)
                            {
                                used_arr[sub_arr[l]] = 1;
                                count++;
                            }

                            nmod_bpoly_set(f, Q);
                            FLINT_ASSERT(f->length > 0);
                            nmod_poly_set(leadf, f->coeffs + f->length - 1);

                         /* If r - count = kk then the rest are irreducible.  
                            TODO: Add a test for that case */
                        }

                        indx = kk - 1;
                    }
                }
            }

            {
                slong test = 0;

                for (kk = 0; kk < I->r; kk++)
                    test = test + used_arr[kk];

                if (test == 0)
                {
                    nmod_mpoly_t goodtry;
                    if (alpha != 0)
                        nmod_bpoly_taylor_shift(f, ctx->ffinfo->mod.n - alpha);
                    nmod_mpoly_init(goodtry, ctx);
                    nmod_mpoly_from_nmod_bpoly(goodtry, A->bits, f, xvar, yvar, ctx);
                    nmod_mpoly_make_monic(goodtry, goodtry, ctx);
                    nmod_mpoly_factor_append_ui(fac, goodtry, 1, ctx);
                    nmod_mpoly_clear(goodtry, ctx);
                }
            }

            nmod_bpoly_clear(f);
            nmod_bpoly_clear(Q);
            nmod_bpoly_clear(R);
            nmod_bpoly_clear(trymez);
            nmod_bpoly_clear(tryme);
            nmod_bpoly_clear(trymet);
            nmod_poly_clear(leadf);
        }

        nmod_bpoly_info_clear(I);

    return 1;
}



static int _fq_nmod_irreducible_bivar_factors_smprime(
    fq_nmod_mpoly_factor_t fac,
    const fq_nmod_mpoly_t A,
    slong xvar,
    slong yvar,
    const fq_nmod_mpoly_ctx_t ctx)
{
    slong i, j;
    ulong k;
    fq_nmod_t alpha;
    fq_nmod_poly_t Beval;
    fq_nmod_bpoly_t B;
    fq_nmod_poly_factor_t Bevalfac;
    fq_nmod_t Bevalfaclc;
    slong Blengthx, Blengthy;
    fq_nmod_smprime_info_t I;
    fq_nmod_bpoly_t tp, tp1, error;
    fq_nmod_poly_t ss, tt;

    fq_nmod_init(alpha, ctx->fqctx);
    fq_nmod_init(Bevalfaclc, ctx->fqctx);

    fq_nmod_mpoly_factor_one(fac, ctx);

    fq_nmod_poly_init(Beval, ctx->fqctx);
    fq_nmod_poly_factor_init(Bevalfac, ctx->fqctx);
    fq_nmod_bpoly_init(B, ctx->fqctx);
    fq_nmod_smprime_info_init(I, 2, ctx->fqctx);

    fq_nmod_mpoly_to_bpoly(B, A, xvar, yvar, ctx);

/*printf("<_fq_nmod_irreducible_bivar_factors_smprime>\n  B: "); fq_nmod_bpoly_print_pretty(B, "x", "y", ctx->fqctx); printf("\n");*/

    Blengthx = B->length;
    FLINT_ASSERT(Blengthx > 1);

    fq_nmod_set_ui(alpha, 0, ctx->fqctx);
    goto got_alpha;

next_alpha:

    if (fq_nmod_next(alpha, ctx->fqctx) == 0)
    {
        /* TODO leak */
        return 0;
    }

got_alpha:

/*flint_printf("<_fq_nmod_irreducible_bivar_factors_smprime>\n  trying alpha = "); fq_nmod_print_pretty(alpha, ctx->fqctx); printf("\n");*/

        fq_nmod_bpoly_eval(Beval, B, alpha, ctx->fqctx);

        /* if killed leading coeff, get new alpha */
        if (Beval->length != Blengthx)
            goto next_alpha;

        Bevalfac->num = 0;
        fq_nmod_poly_factor(Bevalfac, Bevalfaclc, Beval, ctx->fqctx);

/*flint_printf("<_fq_nmod_irreducible_bivar_factors_smprime>\n  Bevalfac: "); fq_nmod_poly_factor_print_pretty(Bevalfac, "x", ctx->fqctx); printf("\n");*/

        /* if multiple factors, get new alpha */
        for (i = 0; i < Bevalfac->num; i++)
        {
            if (Bevalfac->exp[i] != 1)
                goto next_alpha;
        }

		/* if one factor, A is irreducible */
		if (Bevalfac->num == 1)
		{
			fq_nmod_mpoly_factor_append_ui(fac, A, 1, ctx);
            /* TODO leak */
			return 1;
		}

/*flint_printf("<_fq_nmod_irreducible_bivar_factors_smprime>\n  good alpha = "); fq_nmod_print_pretty(alpha, ctx->fqctx); printf("\n");*/

        fq_nmod_bpoly_taylor_shift(B, alpha, ctx->fqctx);

        Blengthy = 0;
        for (i = 0; i < B->length; i++)
        {
            Blengthy = FLINT_MAX(Blengthy, (B->coeffs + i)->length);
        }
        fq_nmod_smprime_info_clear(I, ctx->fqctx);
        fq_nmod_smprime_info_init(I, Bevalfac->num, ctx->fqctx);

        fq_nmod_bpoly_set(I->Btilde, B, ctx->fqctx);
        fq_nmod_bpoly_make_monic(I->Btilde, Blengthy, ctx->fqctx);

        for (i = 0; i < I->r; i++)
        {
            fq_nmod_poly_make_monic(I->Bitilde + i, Bevalfac->poly + i, ctx->fqctx);
            fq_nmod_bpoly_set_polyx(I->newBitilde + i, I->Bitilde + i, ctx->fqctx);
        }

        fq_nmod_smprime_info_disolve(I, ctx->fqctx);

        FLINT_ASSERT(I->r > 1);

        fq_nmod_poly_init(ss, ctx->fqctx);
        fq_nmod_poly_init(tt, ctx->fqctx);
        fq_nmod_bpoly_init(tp, ctx->fqctx);
        fq_nmod_bpoly_init(tp1, ctx->fqctx);
        fq_nmod_bpoly_init(error, ctx->fqctx);

        fq_nmod_bpoly_mul(tp, I->newBitilde + 0, I->newBitilde + 1, Blengthy, ctx->fqctx);
        for (i = 2; i < I->r; i++)
        {
            fq_nmod_bpoly_mul(tp1, tp, I->newBitilde + i, Blengthy, ctx->fqctx);
            fq_nmod_bpoly_swap(tp1, tp);
        }

        fq_nmod_bpoly_sub(error, I->Btilde, tp, ctx->fqctx);
/*flint_printf("error: "); nmod_bpoly_print(error, "x", "y"); printf("\n");*/
        for (j = 1; j < Blengthy; j++)
        {
            fq_nmod_poly_zero(ss, ctx->fqctx);
            for (i = error->length - 1; i >= 0; i--)
            {
                fq_nmod_t ccc;
                fq_nmod_init(ccc, ctx->fqctx);
                fq_nmod_bpoly_get_coeff(ccc, error, i, j, ctx->fqctx);
                fq_nmod_poly_set_coeff(ss, i, ccc, ctx->fqctx);
                for (k = 0; k < j; k++)
                {
                    fq_nmod_bpoly_get_coeff(ccc, error, i, k, ctx->fqctx);
                    FLINT_ASSERT(fq_nmod_is_zero(ccc, ctx->fqctx));
                }
                fq_nmod_clear(ccc, ctx->fqctx);
            }

            for (i = 0; i < I->r; i++)
            {
                fq_nmod_poly_mul(tt, ss, I->d + i, ctx->fqctx);
                fq_nmod_poly_rem(tt, tt, I->Bitilde + i, ctx->fqctx);
                fq_nmod_bpoly_add_poly_shift(I->newBitilde + i, tt, j, ctx->fqctx);
            }

            fq_nmod_bpoly_mul(tp, I->newBitilde + 0, I->newBitilde + 1, Blengthy, ctx->fqctx);
            for (i = 2; i < I->r; i++)
            {
                fq_nmod_bpoly_mul(tp1, tp, I->newBitilde + i, Blengthy, ctx->fqctx);
                fq_nmod_bpoly_swap(tp1, tp);
            }
            fq_nmod_bpoly_sub(error, I->Btilde, tp, ctx->fqctx);
/*flint_printf("j = %wd lift: error: ", j); nmod_bpoly_print(error, "x", "y"); printf("\n");*/
        }

        fq_nmod_poly_clear(ss, ctx->fqctx);
        fq_nmod_poly_clear(tt, ctx->fqctx);
        fq_nmod_bpoly_clear(tp, ctx->fqctx);
        fq_nmod_bpoly_clear(tp1, ctx->fqctx);
        fq_nmod_bpoly_clear(error, ctx->fqctx);

/*
printf("<_fq_nmod_irreducible_bivar_factors_smprime>\n  error: "); fq_nmod_bpoly_print_pretty(error, "x", "y", ctx->fqctx); printf("\n");
for (i = 0; i < I->r; i++)
{
flint_printf("I->newBitilde[%wd]: ", i); fq_nmod_bpoly_print_pretty(I->newBitilde + i, "x", "y", ctx->fqctx); printf("\n");
}
*/
        {
            fq_nmod_bpoly_t f, Q, R, trymez;
            fq_nmod_bpoly_t tryme, trymet;
            fq_nmod_poly_t leadf;
            slong kk, *used_arr, *sub_arr;

            used_arr = (slong *) flint_calloc(2 * I->r, sizeof(slong));
            sub_arr  = used_arr + I->r;

            fq_nmod_bpoly_init(f, ctx->fqctx);
            fq_nmod_bpoly_init(Q, ctx->fqctx);
            fq_nmod_bpoly_init(R, ctx->fqctx);
            fq_nmod_bpoly_init(trymez, ctx->fqctx);
            fq_nmod_bpoly_init(tryme, ctx->fqctx);
            fq_nmod_bpoly_init(trymet, ctx->fqctx);
            fq_nmod_poly_init(leadf, ctx->fqctx);

            fq_nmod_bpoly_set(f, B, ctx->fqctx);
            FLINT_ASSERT(f->length > 0);
            fq_nmod_poly_set(leadf, f->coeffs + f->length - 1, ctx->fqctx);

            for (kk = 1; kk < I->r; kk++)
            {
                slong count = 0, indx = kk - 1, l;

                for(l = 0; l < kk; l++)
                    sub_arr[l] = l;

                sub_arr[indx]--;
                while ((indx >= 0))
                {
                    sub_arr[indx] = sub_arr[indx] + 1;

                    for (l = indx + 1; l < kk; l++)
                        sub_arr[l] = sub_arr[l - 1] + 1;

                    if (sub_arr[kk - 1] > I->r - 1)
                        indx--;
                    else
                    {
                        for(l = 0; l < kk; l++)
                        {
                            if (used_arr[sub_arr[l]] == 1)
                                break;
                        }

                        fq_nmod_bpoly_set_polyy(tryme, leadf, ctx->fqctx);
                        for (l = 0; l < kk; l++)
                        {
                            fq_nmod_bpoly_mul(trymet, tryme, I->newBitilde + sub_arr[l], Blengthy, ctx->fqctx);
                            fq_nmod_bpoly_swap(trymet, tryme);
                        }

                        fq_nmod_bpoly_set(trymez, tryme, ctx->fqctx);
                        fq_nmod_bpoly_make_primitive(trymez, ctx->fqctx);
                        if (fq_nmod_bpoly_divides(Q, f, trymez, ctx->fqctx))
                        {
                            fq_nmod_mpoly_t goodtry;
                            fq_nmod_neg(alpha, alpha, ctx->fqctx);
                            fq_nmod_bpoly_taylor_shift(trymez, alpha, ctx->fqctx);
                            fq_nmod_neg(alpha, alpha, ctx->fqctx);
                            fq_nmod_mpoly_init(goodtry, ctx);
                            fq_nmod_mpoly_from_fq_nmod_bpoly(goodtry, A->bits, trymez, xvar, yvar, ctx);
                            fq_nmod_mpoly_make_monic(goodtry, goodtry, ctx);
                            fq_nmod_mpoly_factor_append_ui(fac, goodtry, 1, ctx);
                            fq_nmod_mpoly_clear(goodtry, ctx);

                            for(l = 0; l < kk; l++)
                            {
                                used_arr[sub_arr[l]] = 1;
                                count++;
                            }

                            fq_nmod_bpoly_set(f, Q, ctx->fqctx);
                            FLINT_ASSERT(f->length > 0);
                            fq_nmod_poly_set(leadf, f->coeffs + f->length - 1, ctx->fqctx);

                         /* If r - count = k then the rest are irreducible.  
                            TODO: Add a test for that case */
                        }

                        indx = kk - 1;
                    }
                }
            }

            {
                slong test = 0;

                for (kk = 0; kk < I->r; kk++)
                    test = test + used_arr[kk];

                if (test == 0)
                {
                    fq_nmod_mpoly_t goodtry;
                    fq_nmod_neg(alpha, alpha, ctx->fqctx);
                    fq_nmod_bpoly_taylor_shift(f, alpha, ctx->fqctx);
                    fq_nmod_neg(alpha, alpha, ctx->fqctx);
                    fq_nmod_mpoly_init(goodtry, ctx);
                    fq_nmod_mpoly_from_fq_nmod_bpoly(goodtry, A->bits, f, xvar, yvar, ctx);
                    fq_nmod_mpoly_make_monic(goodtry, goodtry, ctx);
                    fq_nmod_mpoly_factor_append_ui(fac, goodtry, 1, ctx);
                    fq_nmod_mpoly_clear(goodtry, ctx);
                }
            }

            fq_nmod_bpoly_clear(f, ctx->fqctx);
            fq_nmod_bpoly_clear(Q, ctx->fqctx);
            fq_nmod_bpoly_clear(R, ctx->fqctx);
            fq_nmod_bpoly_clear(trymez, ctx->fqctx);
            fq_nmod_bpoly_clear(tryme, ctx->fqctx);
            fq_nmod_bpoly_clear(trymet, ctx->fqctx);
            fq_nmod_poly_clear(leadf, ctx->fqctx);
        }

        fq_nmod_smprime_info_clear(I, ctx->fqctx);

    return 1;
}





void nmod_bpoly_eval_fq_nmod_poly(
    fq_nmod_poly_t A,
    const nmod_bpoly_t B,
    const fq_nmod_ctx_t ectx)
{
    slong i;
    fq_nmod_t t;

    fq_nmod_init(t, ectx);

    fq_nmod_poly_zero(A, ectx);
    for (i = B->length - 1; i >= 0; i--)
    {
        nmod_poly_rem(t, B->coeffs + i, ectx->modulus);
        fq_nmod_poly_set_coeff(A, i, t, ectx);
    }

    fq_nmod_clear(t, ectx);
}


/*
void fq_nmod_poly_shift_nmod_poly(
    fq_nmod_poly_t A,
    nmod_poly_t B,
    const fq_nmod_ctx_t ectx)
{
    slong i;
    nmod_poly_t q, b, r;
    nmod_poly_init_mod(q, B->mod);
    nmod_poly_init_mod(b, B->mod);
    nmod_poly_init_mod(r, B->mod);
    nmod_poly_set(b, B);
    fq_nmod_poly_zero(A, ectx);
    for (i = 0; i < B->length; i++)
    {
        nmod_poly_divrem(q, r, b, ectx->modulus);
        fq_nmod_poly_set_coeff(A, i, r, ectx);
        nmod_poly_swap(b, q);
    }
    FLINT_ASSERT(nmod_poly_is_zero(b));
    nmod_poly_clear(b);
    nmod_poly_clear(q);
    nmod_poly_clear(r);
}

void nmod_poly_unshift_fq_nmod_poly(
    nmod_poly_t A,
    fq_nmod_poly_t B,
    const fq_nmod_ctx_t ectx)
{
    slong i;
    nmod_poly_zero(A);
    for (i = B->length - 1; i >= 0; i++)
    {
        nmod_poly_mul(A, A, ectx->modulus);
        nmod_poly_add(A, A, B->coeffs + i);
    }
}

void fq_nmod_bpoly_shift_nmod_bpoly(
    fq_nmod_bpoly_t A,
    nmod_bpoly_t B,
    const fq_nmod_ctx_t ectx)
{
    slong i;
    fq_nmod_bpoly_fit_length(A, B->length, ectx);
    A->length = 0;
    for (i = 0; i < B->length; i++)
    {
        fq_nmod_poly_shift_nmod_poly(A->coeffs + i, B->coeffs + i, ectx);
        if (!fq_nmod_poly_is_zero(A->coeffs + i, ectx))
            A->length = i + 1;
    }
}

void nmod_bpoly_unshift_fq_nmod_bpoly(
    nmod_bpoly_t A,
    fq_nmod_bpoly_t B,
    const fq_nmod_ctx_t ectx)
{
    slong i;
    nmod_bpoly_fit_length(A, B->length);
    A->length = 0;
    for (i = 0; i < B->length; i++)
    {
        nmod_poly_unshift_fq_nmod_poly(A->coeffs + i, B->coeffs + i, ectx);
        if (!nmod_poly_is_zero(A->coeffs + i))
            A->length = i + 1;
    }
}
*/


void nmod_bpoly_add_fq_nmod_poly_mul(
    nmod_bpoly_t A,
    const fq_nmod_poly_t B,
    const nmod_poly_t mpow,
    const fq_nmod_ctx_t ectx)
{
    slong i;
    fq_nmod_t t;

    fq_nmod_init(t, ectx);

    FLINT_ASSERT(A->length > B->length);

    for (i = 0; i < B->length; i++)
    {
        nmod_poly_mul(t, B->coeffs + i, mpow);
        nmod_poly_add(A->coeffs + i, A->coeffs + i, t);
    }

    fq_nmod_clear(t, ectx);
}

void nmod_bpoly_set_fq_nmod_polyx(
    nmod_bpoly_t A,
    const fq_nmod_poly_t B,
    const fq_nmod_ctx_t ectx)
{
    slong i;

    nmod_bpoly_fit_length(A, B->length);
    A->length = B->length;

    for (i = 0; i < B->length; i++)
        nmod_poly_set(A->coeffs + i, B->coeffs + i);
}


static int _irreducible_bivar_factors_lgprime(
    nmod_mpoly_factor_t fac,
    const nmod_mpoly_t A,
    slong xvar,
    slong yvar,
    const nmod_mpoly_ctx_t ctx)
{
    slong i, j, k;
    fq_nmod_poly_t Beval;
    nmod_bpoly_t B;
    fq_nmod_poly_factor_t Bevalfac;
    fq_nmod_t Bfaclc;
    slong Blengthx, Blengthy;
    nmod_lgprime_info_t I;
    nmod_bpoly_t tp, tp1, error;
    fq_nmod_poly_t ss, tt;
	slong deg;
    fq_nmod_ctx_t ectx;
    fmpz_t P;
    nmod_poly_t finalmpow, mpow;

    nmod_poly_init_mod(mpow, ctx->ffinfo->mod);
    nmod_poly_init_mod(finalmpow, ctx->ffinfo->mod);

    nmod_mpoly_factor_one(fac, ctx);

    deg = 2;
    fmpz_init_set_ui(P, ctx->ffinfo->mod.n);
    fq_nmod_ctx_init(ectx, P, deg, "y");

    fq_nmod_poly_init(Beval, ectx);
    fq_nmod_poly_factor_init(Bevalfac, ectx);
    fq_nmod_init(Bfaclc, ectx);
    nmod_bpoly_init(B, ctx->ffinfo->mod);
    nmod_lgprime_info_init(I, 2, ectx);

    nmod_mpoly_to_bpoly(B, A, xvar, yvar, ctx);
/*
printf("\n ************ lgprime B: "); nmod_bpoly_print(B, "x", "y"); printf("\n");
*/
    Blengthx = B->length;
    FLINT_ASSERT(Blengthx > 1);
/*
flint_printf("Blengthx: %wd\n");
*/
    goto got_alpha;

next_alpha:

    deg++;
    if (deg >= 10000)
    {
        /* TODO leak */
        return 0;
    }

	fq_nmod_ctx_clear(ectx);
	fq_nmod_ctx_init(ectx, P, deg, "y");

got_alpha:
/*
printf("***alpha: finite field m: "); nmod_poly_print_pretty(ectx->modulus, "y"); printf("\n");
*/
        nmod_bpoly_eval_fq_nmod_poly(Beval, B, ectx);
/*
printf("Beval: "); fq_nmod_poly_print_pretty(Beval, "x", ectx); printf("\n");
*/
        /* if killed leading coeff, get new alpha */
        if (Beval->length != Blengthx)
            goto next_alpha;

        Bevalfac->num = 0;
        fq_nmod_poly_factor(Bevalfac, Bfaclc, Beval, ectx);
/*
flint_printf("Bevalfac: "); fq_nmod_poly_factor_print_pretty(Bevalfac, "x", ectx); printf("\n");
*/
        /* if multiple factors, get new alpha */
        for (i = 0; i < Bevalfac->num; i++)
        {
            if (Bevalfac->exp[i] != 1)
                goto next_alpha;
        }

		/* if one factor, A is irreducible */
		if (Bevalfac->num == 1)
		{
			nmod_mpoly_factor_append_ui(fac, A, 1, ctx);
            /* TODO leak */
			return 1;
		}
/*
printf("***alpha: good finite field m: "); nmod_poly_print_pretty(ectx->modulus, "y"); printf("\n");
*/
        Blengthy = 0;
        for (i = 0; i < B->length; i++)
        {
            Blengthy = FLINT_MAX(Blengthy, (B->coeffs + i)->length);
        }
/*
flint_printf("Blengthy: %wd\n", Blengthy);
*/

        nmod_poly_one(finalmpow);
        while (nmod_poly_length(finalmpow) <= Blengthy)
            nmod_poly_mul(finalmpow, finalmpow, ectx->modulus);
/*
printf("finalmpow: "); nmod_poly_print_pretty(finalmpow, "y"); printf("\n");
*/
        nmod_lgprime_info_clear(I, ectx);
        nmod_lgprime_info_init(I, Bevalfac->num, ectx);
/*
flint_printf("I->r : %wd\n", I->r);
*/
        nmod_bpoly_set(I->Btilde, B);
        nmod_bpoly_make_monic(I->Btilde, Blengthy);
/*
printf("I->Btilde: "); nmod_bpoly_print(I->Btilde, "x", "y"); printf("\n");
*/
        for (i = 0; i < I->r; i++)
        {
            fq_nmod_poly_make_monic(I->Bitilde + i, Bevalfac->poly + i, ectx);
/*
flint_printf("   I->Bitilde[%wd]: ", i); fq_nmod_poly_print_pretty(I->Bitilde + i, "x", ectx); printf("\n");
*/
            nmod_bpoly_set_fq_nmod_polyx(I->newBitilde + i, I->Bitilde + i, ectx);
/*
flint_printf("I->newBitilde[%wd]: ", i); nmod_bpoly_print(I->newBitilde + i, "x", "y"); printf("\n");
*/
        }

        nmod_lgprime_info_disolve(I, ectx);

/*
        for (i = 0; i < I->r; i++)
        {
flint_printf("  I->d[%wd]: ", i); nmod_poly_print_pretty(I->d + i, "x"); printf("\n");
flint_printf("  I->P[%wd]: ", i); nmod_poly_print_pretty(I->P + i, "x"); printf("\n");

        }
*/


        FLINT_ASSERT(I->r > 1);

        fq_nmod_poly_init(ss, ectx);
        fq_nmod_poly_init(tt, ectx);
        nmod_bpoly_init(tp, ectx->modulus->mod);
        nmod_bpoly_init(tp1, ectx->modulus->mod);
        nmod_bpoly_init(error, ectx->modulus->mod);

        nmod_bpoly_mul_mod(tp, I->newBitilde + 0, I->newBitilde + 1, finalmpow);
        for (i = 2; i < I->r; i++)
        {
            nmod_bpoly_mul_mod(tp1, tp, I->newBitilde + i, finalmpow);
            nmod_bpoly_swap(tp1, tp);
        }

        nmod_bpoly_sub(error, I->Btilde, tp);
/*
flint_printf("\nerror: "); nmod_bpoly_print(error, "x", "y"); printf("\n");
*/
        nmod_poly_one(mpow);

        for (j = 1; j < Blengthy; j++)
        {
            nmod_poly_mul(mpow, mpow, ectx->modulus);
/*
flint_printf("j = %wd, m^j: ",j); nmod_poly_print_pretty(mpow, "y"); printf("\n");
*/
            fq_nmod_poly_zero(ss, ectx);

            for (k = error->length - 1; k >= 0; k--)
            {
                nmod_poly_t q, r;
                nmod_poly_init_mod(q, ctx->ffinfo->mod);
                nmod_poly_init_mod(r, ctx->ffinfo->mod);
                nmod_poly_divrem(q, r, error->coeffs + k, mpow);
                FLINT_ASSERT(nmod_poly_is_zero(r));
                nmod_poly_rem(r, q, ectx->modulus);
                fq_nmod_poly_set_coeff(ss, k, r, ectx);
                nmod_poly_clear(q);
                nmod_poly_clear(r);
            }

            for (i = 0; i < I->r; i++)
            {
                fq_nmod_poly_mul(tt, ss, I->d + i, ectx);
                fq_nmod_poly_rem(tt, tt, I->Bitilde + i, ectx);
/*
printf("tt: "); fq_nmod_poly_print_pretty(tt, "y", ectx); printf("\n");
*/
                nmod_bpoly_add_fq_nmod_poly_mul(I->newBitilde + i, tt, mpow, ectx);
/*
flint_printf("I->newBitilde[%wd]: ", i); nmod_bpoly_print(I->newBitilde + i, "x", "y"); printf("\n");
*/
            }

            nmod_bpoly_mul_mod(tp, I->newBitilde + 0, I->newBitilde + 1, finalmpow);
            for (i = 2; i < I->r; i++)
            {
                nmod_bpoly_mul_mod(tp1, tp, I->newBitilde + i, finalmpow);
                nmod_bpoly_swap(tp1, tp);
            }
            nmod_bpoly_sub(error, I->Btilde, tp);
/*
flint_printf("\nj = %wd lift: error: ", j); nmod_bpoly_print(error, "x", "y"); printf("\n");
*/
        }

        fq_nmod_poly_clear(ss, ectx);
        fq_nmod_poly_clear(tt, ectx);
        nmod_bpoly_clear(tp);
        nmod_bpoly_clear(tp1);
        nmod_bpoly_clear(error);
/*
printf("error: "); nmod_bpoly_print(error, "x", "y"); printf("\n");
for (i = 0; i < I->r; i++)
{
flint_printf(" I->newBitilde[%wd]: ", i); nmod_bpoly_print(I->newBitilde + i, "x", "y"); printf("\n");
}
*/
        {
            nmod_bpoly_t f, Q, R, trymez;
            nmod_bpoly_t tryme, trymet;
            nmod_poly_t leadf;
            slong kk, *used_arr, *sub_arr;

            used_arr = (slong *) flint_calloc(2 * I->r, sizeof(slong));
            sub_arr  = used_arr + I->r;

            nmod_bpoly_init(f, ctx->ffinfo->mod);
            nmod_bpoly_init(Q, ctx->ffinfo->mod);
            nmod_bpoly_init(R, ctx->ffinfo->mod);
            nmod_bpoly_init(trymez, ctx->ffinfo->mod);
            nmod_bpoly_init(tryme, ctx->ffinfo->mod);
            nmod_bpoly_init(trymet, ctx->ffinfo->mod);
            nmod_poly_init_mod(leadf, ctx->ffinfo->mod);

            nmod_bpoly_set(f, B);
            FLINT_ASSERT(f->length > 0);
            nmod_poly_set(leadf, f->coeffs + f->length - 1);

            for (kk = 1; kk < I->r; kk++)
            {
                slong count = 0, indx = kk - 1, l;

                for(l = 0; l < kk; l++)
                    sub_arr[l] = l;

                sub_arr[indx]--;
                while ((indx >= 0))
                {
                    sub_arr[indx] = sub_arr[indx] + 1;

                    for (l = indx + 1; l < kk; l++)
                        sub_arr[l] = sub_arr[l - 1] + 1;

                    if (sub_arr[kk - 1] > I->r - 1)
                        indx--;
                    else
                    {
                        for(l = 0; l < kk; l++)
                        {
                            if (used_arr[sub_arr[l]] == 1)
                                break;
                        }

                        nmod_bpoly_set_polyy(tryme, leadf);
                        for (l = 0; l < kk; l++)
                        {
                            nmod_bpoly_mul_mod(trymet, tryme, I->newBitilde + sub_arr[l], finalmpow);
                            nmod_bpoly_swap(trymet, tryme);
                        }

                        nmod_bpoly_set(trymez, tryme);
                        nmod_bpoly_make_primitive(trymez);
                        if (nmod_bpoly_divides(Q, f, trymez))
                        {
                            nmod_mpoly_t goodtry;
                            nmod_mpoly_init(goodtry, ctx);
                            nmod_mpoly_from_nmod_bpoly(goodtry, A->bits, trymez, xvar, yvar, ctx);
                            nmod_mpoly_make_monic(goodtry, goodtry, ctx);
                            nmod_mpoly_factor_append_ui(fac, goodtry, 1, ctx);
                            nmod_mpoly_clear(goodtry, ctx);

                            for(l = 0; l < kk; l++)
                            {
                                used_arr[sub_arr[l]] = 1;
                                count++;
                            }

                            nmod_bpoly_set(f, Q);
                            FLINT_ASSERT(f->length > 0);
                            nmod_poly_set(leadf, f->coeffs + f->length - 1);

                         /* If r - count = kk then the rest are irreducible.  
                            TODO: Add a test for that case */
                        }

                        indx = kk - 1;
                    }
                }
            }

            {
                slong test = 0;

                for (kk = 0; kk < I->r; kk++)
                    test = test + used_arr[kk];

                if (test == 0)
                {
                    nmod_mpoly_t goodtry;
                    nmod_mpoly_init(goodtry, ctx);
                    nmod_mpoly_from_nmod_bpoly(goodtry, A->bits, f, xvar, yvar, ctx);
                    nmod_mpoly_make_monic(goodtry, goodtry, ctx);
                    nmod_mpoly_factor_append_ui(fac, goodtry, 1, ctx);
                    nmod_mpoly_clear(goodtry, ctx);
                }
            }

            nmod_bpoly_clear(f);
            nmod_bpoly_clear(Q);
            nmod_bpoly_clear(R);
            nmod_bpoly_clear(trymez);
            nmod_bpoly_clear(tryme);
            nmod_bpoly_clear(trymet);
            nmod_poly_clear(leadf);
        }

        nmod_lgprime_info_clear(I, ectx);

    return 1;
}


static void _to_poly(
    nmod_poly_t A,
    const nmod_mpoly_t B,
    const nmod_mpoly_ctx_t ctx)
{
    slong i;
    ulong * shift, * stride;

    if (B->length == 0)
    {
        nmod_poly_zero(A);
        return;
    }

    shift = (ulong *) flint_malloc(ctx->minfo->nvars*sizeof(ulong));
    stride = (ulong *) flint_malloc(ctx->minfo->nvars*sizeof(ulong));
    for (i = 0; i < ctx->minfo->nvars; i++)
    {
        shift[i] = 0;
        stride[i] = 1;
    }

    _nmod_mpoly_to_nmod_poly_deflate(A, B, 0, shift, stride, ctx);

    flint_free(shift);
    flint_free(stride);
}

static void _fq_nmod_to_poly(
    fq_nmod_poly_t A,
    const fq_nmod_mpoly_t B,
    const fq_nmod_mpoly_ctx_t ctx)
{
    slong i;
    ulong * shift, * stride;

    if (B->length == 0)
    {
        fq_nmod_poly_zero(A, ctx->fqctx);
        return;
    }

    shift = (ulong *) flint_malloc(ctx->minfo->nvars*sizeof(ulong));
    stride = (ulong *) flint_malloc(ctx->minfo->nvars*sizeof(ulong));
    for (i = 0; i < ctx->minfo->nvars; i++)
    {
        shift[i] = 0;
        stride[i] = 1;
    }

    _fq_nmod_mpoly_to_fq_nmod_poly_deflate(A, B, 0, shift, stride, ctx);

    flint_free(shift);
    flint_free(stride);
}

static void _from_poly(
    nmod_mpoly_t A,
    flint_bitcnt_t Abits,
    const nmod_poly_t B,
    const nmod_mpoly_ctx_t ctx)
{
    slong i;
    ulong * shift, * stride;

    if (B->length == 0)
    {
        nmod_mpoly_zero(A, ctx);
        return;
    }

    shift = (ulong *) flint_malloc(ctx->minfo->nvars*sizeof(ulong));
    stride = (ulong *) flint_malloc(ctx->minfo->nvars*sizeof(ulong));
    for (i = 0; i < ctx->minfo->nvars; i++)
    {
        shift[i] = 0;
        stride[i] = 1;
    }

    _nmod_mpoly_from_nmod_poly_inflate(A, Abits, B, 0, shift, stride, ctx);

    flint_free(shift);
    flint_free(stride);
}

static void _fq_nmod_from_poly(
    fq_nmod_mpoly_t A,
    flint_bitcnt_t Abits,
    const fq_nmod_poly_t B,
    const fq_nmod_mpoly_ctx_t ctx)
{
    slong i;
    ulong * shift, * stride;

    if (B->length == 0)
    {
        fq_nmod_mpoly_zero(A, ctx);
        return;
    }

    shift = (ulong *) flint_malloc(ctx->minfo->nvars*sizeof(ulong));
    stride = (ulong *) flint_malloc(ctx->minfo->nvars*sizeof(ulong));
    for (i = 0; i < ctx->minfo->nvars; i++)
    {
        shift[i] = 0;
        stride[i] = 1;
    }

    _fq_nmod_mpoly_from_fq_nmod_poly_inflate(A, Abits, B, 0, shift, stride, ctx);

    flint_free(shift);
    flint_free(stride);
}


typedef struct {
    slong n;
    slong r;
    slong l;
    nmod_poly_struct * inv_prod_dbetas;
    nmod_poly_struct * dbetas;
    nmod_mpoly_struct * prod_mbetas;
    nmod_mpoly_struct * mbetas;
    nmod_mpoly_struct * deltas;
} nmod_disolve_struct;

typedef nmod_disolve_struct nmod_disolve_t[1];

typedef struct {
    slong n;
    slong r;
    slong l;
    fq_nmod_poly_struct * inv_prod_dbetas;
    fq_nmod_poly_struct * dbetas;
    fq_nmod_mpoly_struct * prod_mbetas;
    fq_nmod_mpoly_struct * mbetas;
    fq_nmod_mpoly_struct * deltas;
} fq_nmod_disolve_struct;

typedef fq_nmod_disolve_struct fq_nmod_disolve_t[1];


static void nmod_disolve_init(
    nmod_disolve_t I,
    slong l, slong r,
    const nmod_mpoly_struct * betas,
    const mp_limb_t * alpha,
    const nmod_mpoly_ctx_t ctx)
{
    slong i, j, k;
    nmod_poly_t p;
    nmod_poly_t G, S, pq;
/*
flint_printf("_mfactor_disolve_init called(l = %wd, r = %wd)\n",l,r);
*/
    I->l = l;
    I->r = r;

    FLINT_ASSERT(l > 0);

    I->inv_prod_dbetas = (nmod_poly_struct *) flint_malloc(l*sizeof(nmod_poly_struct));
    I->dbetas = (nmod_poly_struct *) flint_malloc(l*sizeof(nmod_poly_struct));
    I->prod_mbetas = (nmod_mpoly_struct *) flint_malloc((r + 1)*l*sizeof(nmod_mpoly_struct));
    I->mbetas = (nmod_mpoly_struct *) flint_malloc((r + 1)*l*sizeof(nmod_mpoly_struct));
    I->deltas = (nmod_mpoly_struct *) flint_malloc((r + 1)*l*sizeof(nmod_mpoly_struct));

    nmod_poly_init_mod(p, ctx->ffinfo->mod);
    nmod_poly_init_mod(G, ctx->ffinfo->mod);
    nmod_poly_init_mod(S, ctx->ffinfo->mod);
    nmod_poly_init_mod(pq, ctx->ffinfo->mod);

    /* initialize deltas */
    for (i = r; i >= 0; i--)
        for (j = 0; j < l; j++)
            nmod_mpoly_init(I->deltas + i*l + j, ctx);

    /* set betas */
    i = r;
    for (j = 0; j < l; j++)
    {
        nmod_mpoly_init(I->mbetas + i*l + j, ctx);
        nmod_mpoly_set(I->mbetas + i*l + j, betas + j, ctx);
/*
flint_printf("mbetas[%wd][%wd]: ",i,j); nmod_mpoly_print_pretty(I->mbetas + i*l + j, NULL, ctx); printf("\n");
*/
    }
    for (i--; i >= 0; i--)
    {
        for (j = 0; j < l; j++)
        {
            nmod_mpoly_init(I->mbetas + i*l + j, ctx);
            nmod_mpoly_evaluate_one_ui(I->mbetas + i*l + j, I->mbetas + (i + 1)*l + j, i + 1, alpha[i], ctx);
/*
flint_printf("mbetas[%wd][%wd]: ",i,j); nmod_mpoly_print_pretty(I->mbetas + i*l + j, NULL, ctx); printf("\n");
*/
        }
    }
    for (j = 0; j < l; j++)
    {
        _to_poly(p, I->mbetas + 0*l + j, ctx);
        nmod_poly_init_mod(I->dbetas + j, ctx->ffinfo->mod);
        nmod_poly_set(I->dbetas + j, p);
/*
flint_printf("dbetas[%wd]: ",j); nmod_poly_print_pretty(I->dbetas + j, "x1"); printf("\n");
*/
    }

    /* set product of betas */
    for (i = r; i >= 0; i--)
    {
        for (j = 0; j < l; j++)
        {
            nmod_mpoly_init(I->prod_mbetas + i*l + j, ctx);
            nmod_mpoly_one(I->prod_mbetas + i*l + j, ctx);
            for (k = 0; k < l; k++)
            {
                if (k == j)
                    continue;
                nmod_mpoly_mul(I->prod_mbetas + i*l + j, I->prod_mbetas + i*l + j, I->mbetas + i*l + k, ctx);
            }
/*
flint_printf("prod_mbetas[%wd][%wd]: ",i,j); nmod_mpoly_print_pretty(I->prod_mbetas + i*l + j, NULL, ctx); printf("\n");
*/
        }        
    }
    for (j = 0; j < l; j++)
    {
        nmod_poly_one(pq);
        for (k = 0; k < l; k++)
        {
            if (k == j)
                continue;
            nmod_poly_mul(pq, pq, I->dbetas + k);
        }
        nmod_poly_init_mod(I->inv_prod_dbetas + j, ctx->ffinfo->mod);
        nmod_poly_xgcd(G, S, I->inv_prod_dbetas + j, I->dbetas + j, pq);
        FLINT_ASSERT(nmod_poly_is_one(G));
/*
flint_printf("inv_prod_dbetas[%wd]: ",j); nmod_poly_print_pretty(I->inv_prod_dbetas + j, "x1"); printf("\n");
*/
    }

    nmod_poly_clear(p);
    nmod_poly_clear(G);
    nmod_poly_clear(S);
    nmod_poly_clear(pq);
/*
printf("_mfactor_disolve_init returning\n");
*/
}

static void fq_nmod_disolve_init(
    fq_nmod_disolve_t I,
    slong l, slong r,
    const fq_nmod_mpoly_struct * betas,
    const fq_nmod_struct * alpha,
    const fq_nmod_mpoly_ctx_t ctx)
{
    slong i, j, k;
    fq_nmod_poly_t p;
    fq_nmod_poly_t G, S, pq;
/*
flint_printf("_mfactor_disolve_init called(l = %wd, r = %wd)\n",l,r);
*/
    I->l = l;
    I->r = r;

    FLINT_ASSERT(l > 0);

    I->inv_prod_dbetas = (fq_nmod_poly_struct *) flint_malloc(l*sizeof(fq_nmod_poly_struct));
    I->dbetas = (fq_nmod_poly_struct *) flint_malloc(l*sizeof(fq_nmod_poly_struct));
    I->prod_mbetas = (fq_nmod_mpoly_struct *) flint_malloc((r + 1)*l*sizeof(fq_nmod_mpoly_struct));
    I->mbetas = (fq_nmod_mpoly_struct *) flint_malloc((r + 1)*l*sizeof(fq_nmod_mpoly_struct));
    I->deltas = (fq_nmod_mpoly_struct *) flint_malloc((r + 1)*l*sizeof(fq_nmod_mpoly_struct));

    fq_nmod_poly_init(p, ctx->fqctx);
    fq_nmod_poly_init(G, ctx->fqctx);
    fq_nmod_poly_init(S, ctx->fqctx);
    fq_nmod_poly_init(pq, ctx->fqctx);

    /* initialize deltas */
    for (i = r; i >= 0; i--)
        for (j = 0; j < l; j++)
            fq_nmod_mpoly_init(I->deltas + i*l + j, ctx);

    /* set betas */
    i = r;
    for (j = 0; j < l; j++)
    {
        fq_nmod_mpoly_init(I->mbetas + i*l + j, ctx);
        fq_nmod_mpoly_set(I->mbetas + i*l + j, betas + j, ctx);
/*
flint_printf("mbetas[%wd][%wd]: ",i,j); nmod_mpoly_print_pretty(I->mbetas + i*l + j, NULL, ctx); printf("\n");
*/
    }
    for (i--; i >= 0; i--)
    {
        for (j = 0; j < l; j++)
        {
            fq_nmod_mpoly_init(I->mbetas + i*l + j, ctx);
            fq_nmod_mpoly_evaluate_one_fq_nmod(I->mbetas + i*l + j, I->mbetas + (i + 1)*l + j, i + 1, alpha + i, ctx);
/*
flint_printf("mbetas[%wd][%wd]: ",i,j); nmod_mpoly_print_pretty(I->mbetas + i*l + j, NULL, ctx); printf("\n");
*/
        }
    }
    for (j = 0; j < l; j++)
    {
        _fq_nmod_to_poly(p, I->mbetas + 0*l + j, ctx);
        fq_nmod_poly_init(I->dbetas + j, ctx->fqctx);
        fq_nmod_poly_set(I->dbetas + j, p, ctx->fqctx);
/*
flint_printf("dbetas[%wd]: ",j); nmod_poly_print_pretty(I->dbetas + j, "x1"); printf("\n");
*/
    }

    /* set product of betas */
    for (i = r; i >= 0; i--)
    {
        for (j = 0; j < l; j++)
        {
            fq_nmod_mpoly_init(I->prod_mbetas + i*l + j, ctx);
            fq_nmod_mpoly_one(I->prod_mbetas + i*l + j, ctx);
            for (k = 0; k < l; k++)
            {
                if (k == j)
                    continue;
                fq_nmod_mpoly_mul(I->prod_mbetas + i*l + j, I->prod_mbetas + i*l + j, I->mbetas + i*l + k, ctx);
            }
/*
flint_printf("prod_mbetas[%wd][%wd]: ",i,j); nmod_mpoly_print_pretty(I->prod_mbetas + i*l + j, NULL, ctx); printf("\n");
*/
        }        
    }
    for (j = 0; j < l; j++)
    {
        fq_nmod_poly_one(pq, ctx->fqctx);
        for (k = 0; k < l; k++)
        {
            if (k == j)
                continue;
            fq_nmod_poly_mul(pq, pq, I->dbetas + k, ctx->fqctx);
        }
        fq_nmod_poly_init(I->inv_prod_dbetas + j, ctx->fqctx);
        fq_nmod_poly_xgcd(G, S, I->inv_prod_dbetas + j, I->dbetas + j, pq, ctx->fqctx);
        FLINT_ASSERT(fq_nmod_poly_is_one(G, ctx->fqctx));
/*
flint_printf("inv_prod_dbetas[%wd]: ",j); nmod_poly_print_pretty(I->inv_prod_dbetas + j, "x1"); printf("\n");
*/
    }

    fq_nmod_poly_clear(p, ctx->fqctx);
    fq_nmod_poly_clear(G, ctx->fqctx);
    fq_nmod_poly_clear(S, ctx->fqctx);
    fq_nmod_poly_clear(pq, ctx->fqctx);
/*
printf("_mfactor_disolve_init returning\n");
*/
}



static void nmod_disolve_clear(nmod_disolve_t I, const nmod_mpoly_ctx_t ctx)
{
    slong i, j;

    for (j = 0; j < I->l; j++)
    {
        nmod_poly_clear(I->inv_prod_dbetas + j);
        nmod_poly_clear(I->dbetas + j);
        for (i = 0; i <= I->r; i++)
        {
            nmod_mpoly_clear(I->prod_mbetas + i*I->l + j, ctx);
            nmod_mpoly_clear(I->mbetas + i*I->l + j, ctx);
            nmod_mpoly_clear(I->deltas + i*I->l + j, ctx);
        }
    }

    flint_free(I->inv_prod_dbetas);
    flint_free(I->dbetas);
    flint_free(I->prod_mbetas);
    flint_free(I->mbetas);
}

static void fq_nmod_disolve_clear(fq_nmod_disolve_t I, const fq_nmod_mpoly_ctx_t ctx)
{
    slong i, j;

    for (j = 0; j < I->l; j++)
    {
        fq_nmod_poly_clear(I->inv_prod_dbetas + j, ctx->fqctx);
        fq_nmod_poly_clear(I->dbetas + j, ctx->fqctx);
        for (i = 0; i <= I->r; i++)
        {
            fq_nmod_mpoly_clear(I->prod_mbetas + i*I->l + j, ctx);
            fq_nmod_mpoly_clear(I->mbetas + i*I->l + j, ctx);
            fq_nmod_mpoly_clear(I->deltas + i*I->l + j, ctx);
        }
    }

    flint_free(I->inv_prod_dbetas);
    flint_free(I->dbetas);
    flint_free(I->prod_mbetas);
    flint_free(I->mbetas);
}


static int nmod_mfactor_disolve(
    flint_bitcnt_t bits,
    slong r,
    slong num,
    const nmod_mpoly_t t,
    const mp_limb_t * alpha,
    const slong * deg,
    const nmod_disolve_t I,
    const nmod_mpoly_ctx_t ctx)
{
    slong i, j;
    int success;
    nmod_mpoly_struct * deltas = I->deltas + r*I->l;
    nmod_mpoly_struct * newdeltas = I->deltas + (r - 1)*I->l;

/*
flint_printf("_mfactor_disolve(r = %wd, num = %wd) called:\n", r, num);
flint_printf("t: "); nmod_mpoly_print_pretty(t, NULL, ctx); printf("\n");
*/
    if (r == 0)
    {
        nmod_poly_t dtq, S, R;

        nmod_poly_init_mod(dtq, ctx->ffinfo->mod);
        nmod_poly_init_mod(S, ctx->ffinfo->mod);
        nmod_poly_init_mod(R, ctx->ffinfo->mod);

        _to_poly(dtq, t, ctx);

        success = 1;
        for (i = 0; i < num; i++)
        {
            nmod_poly_mul(S, dtq, I->inv_prod_dbetas + i);
            nmod_poly_rem(R, S, I->dbetas + i);
            _from_poly(deltas + i, bits, R, ctx);
        }

        nmod_poly_clear(dtq);
        nmod_poly_clear(S);
        nmod_poly_clear(R);
    }
    else
    {
        nmod_mpoly_t e, tt, pow, g, q, newt;

FLINT_ASSERT(num == I->l);

        nmod_mpoly_init(e, ctx);
        nmod_mpoly_init(tt, ctx);
        nmod_mpoly_init(pow, ctx);
        nmod_mpoly_init(g, ctx);
        nmod_mpoly_init(q, ctx);
        nmod_mpoly_init(newt, ctx);
        for (i = 0; i < num; i++)
            nmod_mpoly_zero(deltas + i, ctx);

        nmod_mpoly_set(e, t, ctx);

        nmod_mpoly_one(pow, ctx);
        nmod_mpoly_gen(g, r, ctx);
        nmod_mpoly_sub_ui(g, g, alpha[r - 1], ctx);
        for (j = 0; j <= deg[r]; j++)
        {
            success = nmod_mpoly_divides(q, e, pow, ctx);
            FLINT_ASSERT(success);
            nmod_mpoly_evaluate_one_ui(newt, q, r, alpha[r - 1], ctx);
            success = nmod_mfactor_disolve(bits, r - 1, num, newt, alpha, deg, I, ctx);
            if (!success)
                goto cleanup;

            nmod_mpoly_set(e, t, ctx);
            for (i = 0; i < num; i++)
            {
                nmod_mpoly_mul(tt, newdeltas + i, pow, ctx);
                nmod_mpoly_add(deltas + i, deltas + i, tt, ctx);
                nmod_mpoly_mul(tt, deltas + i, I->prod_mbetas + r*I->l + i, ctx);
                nmod_mpoly_sub(e, e, tt, ctx);
            }

            nmod_mpoly_mul(pow, pow, g, ctx);
        }

        success = nmod_mpoly_is_zero(e, ctx);

cleanup:

        nmod_mpoly_clear(e, ctx);
        nmod_mpoly_clear(tt, ctx);
        nmod_mpoly_clear(pow, ctx);
        nmod_mpoly_clear(g, ctx);
        nmod_mpoly_clear(q, ctx);
        nmod_mpoly_clear(newt, ctx);
    }

/*
flint_printf("_mfactor_disolve(r = %wd, num = %wd) returning %d:\n", r, num, success);
flint_printf("t: "); nmod_mpoly_print_pretty(t, NULL, ctx); printf("\n");
for (i = 0; i < num; i++)
{
flint_printf("deltas[%wd]: ", i); nmod_mpoly_print_pretty(deltas + i, NULL, ctx); printf("\n");
}
*/
    return success;
}


static int fq_nmod_mfactor_disolve(
    flint_bitcnt_t bits,
    slong r,
    slong num,
    const fq_nmod_mpoly_t t,
    const fq_nmod_struct * alpha,
    const slong * deg,
    const fq_nmod_disolve_t I,
    const fq_nmod_mpoly_ctx_t ctx)
{
    slong i, j;
    int success;
    fq_nmod_mpoly_struct * deltas = I->deltas + r*I->l;
    fq_nmod_mpoly_struct * newdeltas = I->deltas + (r - 1)*I->l;

/*
flint_printf("_mfactor_disolve(r = %wd, num = %wd) called:\n", r, num);
flint_printf("t: "); nmod_mpoly_print_pretty(t, NULL, ctx); printf("\n");
*/
    if (r == 0)
    {
        fq_nmod_poly_t dtq, S, R;

        fq_nmod_poly_init(dtq, ctx->fqctx);
        fq_nmod_poly_init(S, ctx->fqctx);
        fq_nmod_poly_init(R, ctx->fqctx);

        _fq_nmod_to_poly(dtq, t, ctx);

        success = 1;
        for (i = 0; i < num; i++)
        {
            fq_nmod_poly_mul(S, dtq, I->inv_prod_dbetas + i, ctx->fqctx);
            fq_nmod_poly_rem(R, S, I->dbetas + i, ctx->fqctx);
            _fq_nmod_from_poly(deltas + i, bits, R, ctx);
        }

        fq_nmod_poly_clear(dtq, ctx->fqctx);
        fq_nmod_poly_clear(S, ctx->fqctx);
        fq_nmod_poly_clear(R, ctx->fqctx);
    }
    else
    {
        fq_nmod_mpoly_t e, tt, pow, g, q, newt;

FLINT_ASSERT(num == I->l);

        fq_nmod_mpoly_init(e, ctx);
        fq_nmod_mpoly_init(tt, ctx);
        fq_nmod_mpoly_init(pow, ctx);
        fq_nmod_mpoly_init(g, ctx);
        fq_nmod_mpoly_init(q, ctx);
        fq_nmod_mpoly_init(newt, ctx);
        for (i = 0; i < num; i++)
            fq_nmod_mpoly_zero(deltas + i, ctx);

        fq_nmod_mpoly_set(e, t, ctx);

        fq_nmod_mpoly_one(pow, ctx);
        fq_nmod_mpoly_gen(g, r, ctx);
        fq_nmod_mpoly_sub_fq_nmod(g, g, alpha + r - 1, ctx);
        for (j = 0; j <= deg[r]; j++)
        {
            success = fq_nmod_mpoly_divides(q, e, pow, ctx);
            FLINT_ASSERT(success);
            fq_nmod_mpoly_evaluate_one_fq_nmod(newt, q, r, alpha + r - 1, ctx);
            success = fq_nmod_mfactor_disolve(bits, r - 1, num, newt, alpha, deg, I, ctx);
            if (!success)
                goto cleanup;

            fq_nmod_mpoly_set(e, t, ctx);
            for (i = 0; i < num; i++)
            {
                fq_nmod_mpoly_mul(tt, newdeltas + i, pow, ctx);
                fq_nmod_mpoly_add(deltas + i, deltas + i, tt, ctx);
                fq_nmod_mpoly_mul(tt, deltas + i, I->prod_mbetas + r*I->l + i, ctx);
                fq_nmod_mpoly_sub(e, e, tt, ctx);
            }

            fq_nmod_mpoly_mul(pow, pow, g, ctx);
        }

        success = fq_nmod_mpoly_is_zero(e, ctx);

cleanup:

        fq_nmod_mpoly_clear(e, ctx);
        fq_nmod_mpoly_clear(tt, ctx);
        fq_nmod_mpoly_clear(pow, ctx);
        fq_nmod_mpoly_clear(g, ctx);
        fq_nmod_mpoly_clear(q, ctx);
        fq_nmod_mpoly_clear(newt, ctx);
    }

/*
flint_printf("_mfactor_disolve(r = %wd, num = %wd) returning %d:\n", r, num, success);
flint_printf("t: "); nmod_mpoly_print_pretty(t, NULL, ctx); printf("\n");
for (i = 0; i < num; i++)
{
flint_printf("deltas[%wd]: ", i); nmod_mpoly_print_pretty(deltas + i, NULL, ctx); printf("\n");
}
*/
    return success;
}






static int nmod_mfactor_lift(
    slong m,
    nmod_mpoly_factor_t lfac,
    const mp_limb_t * alpha,
    const nmod_mpoly_t A,
    const slong * degs,
    const nmod_mpoly_ctx_t ctx)
{
    int success;
    slong i, j;
    slong r = lfac->length;
    nmod_mpoly_t e, t, pow, g, q;
    nmod_mpoly_struct * betas, * deltas;
    nmod_disolve_t I;
/*
flint_printf("_mfactor_lift called (m = %wd)\n", m);
flint_printf("lfac: "); nmod_mpoly_factor_print_pretty(lfac, NULL, ctx); printf("\n");
flint_printf("A: "); nmod_mpoly_print_pretty(A, NULL, ctx); printf("\n");
*/
    FLINT_ASSERT(r > 1);

    nmod_mpoly_init(e, ctx);
    nmod_mpoly_init(t, ctx);
    nmod_mpoly_init(pow, ctx);
    nmod_mpoly_init(g, ctx);
    nmod_mpoly_init(q, ctx);

    betas  = (nmod_mpoly_struct * ) flint_malloc(r*sizeof(nmod_mpoly_struct));
    for (i = 0; i < r; i++)
    {
        nmod_mpoly_init(betas + i, ctx);
        nmod_mpoly_evaluate_one_ui(betas + i, lfac->poly + i, m, alpha[m - 1], ctx);
    }

    nmod_mpoly_mul(t, lfac->poly + 0, lfac->poly + 1, ctx);
    for (i = 2; i < r; i++)
        nmod_mpoly_mul(t, t, lfac->poly + i, ctx);
    nmod_mpoly_sub(e, A, t, ctx);

    nmod_mpoly_one(pow, ctx);
    nmod_mpoly_gen(g, m, ctx);
    nmod_mpoly_sub_ui(g, g, alpha[m - 1], ctx);

    nmod_disolve_init(I, lfac->length, m - 1, betas, alpha, ctx);
    deltas = I->deltas + (m - 1)*I->l;

    for (j = 1; j <= degs[m]; j++)
    {
/*
flint_printf("<_mfactor_lift> j = %wd, error (length: %wd)\n", j, e->length); nmod_mpoly_print_pretty(e, NULL, ctx); printf("\n");
*/
        if (nmod_mpoly_is_zero(e, ctx))
        {
            success = 1;
            goto cleanup;
        }

        nmod_mpoly_mul(pow, pow, g, ctx);
        success = nmod_mpoly_divides(q, e, pow, ctx);
        FLINT_ASSERT(success);
        nmod_mpoly_evaluate_one_ui(t, q, m, alpha[m - 1], ctx);

        success = nmod_mfactor_disolve(A->bits, m - 1, r, t, alpha, degs, I, ctx);
        if (!success)
            goto cleanup;

        for (i = 0; i < r; i++)
        {
            nmod_mpoly_mul(t, deltas + i, pow, ctx);
            nmod_mpoly_add(lfac->poly + i, lfac->poly + i, t, ctx);
        }

        nmod_mpoly_mul(t, lfac->poly + 0, lfac->poly + 1, ctx);
        for (i = 2; i < r; i++)
            nmod_mpoly_mul(t, t, lfac->poly + i, ctx);
        nmod_mpoly_sub(e, A, t, ctx);
    }

    success = nmod_mpoly_is_zero(e, ctx);

cleanup:

    nmod_disolve_clear(I, ctx);

    nmod_mpoly_clear(e, ctx);
    nmod_mpoly_clear(t, ctx);
    nmod_mpoly_clear(pow, ctx);
    nmod_mpoly_clear(g, ctx);
    nmod_mpoly_clear(q, ctx);

    for (i = 0; i < r; i++)
    {
        nmod_mpoly_clear(betas + i, ctx);
    }

    flint_free(betas);

    return success;
}


static int fq_nmod_mfactor_lift(
    slong m,
    fq_nmod_mpoly_factor_t lfac,
    const fq_nmod_struct * alpha,
    const fq_nmod_mpoly_t A,
    const slong * degs,
    const fq_nmod_mpoly_ctx_t ctx)
{
    int success;
    slong i, j;
    slong r = lfac->length;
    fq_nmod_mpoly_t e, t, pow, g, q;
    fq_nmod_mpoly_struct * betas, * deltas;
    fq_nmod_disolve_t I;
/*
flint_printf("_mfactor_lift called (m = %wd)\n", m);
flint_printf("lfac: "); nmod_mpoly_factor_print_pretty(lfac, NULL, ctx); printf("\n");
flint_printf("A: "); nmod_mpoly_print_pretty(A, NULL, ctx); printf("\n");
*/
    FLINT_ASSERT(r > 1);

    fq_nmod_mpoly_init(e, ctx);
    fq_nmod_mpoly_init(t, ctx);
    fq_nmod_mpoly_init(pow, ctx);
    fq_nmod_mpoly_init(g, ctx);
    fq_nmod_mpoly_init(q, ctx);

    betas  = (fq_nmod_mpoly_struct * ) flint_malloc(r*sizeof(fq_nmod_mpoly_struct));
    for (i = 0; i < r; i++)
    {
        fq_nmod_mpoly_init(betas + i, ctx);
        fq_nmod_mpoly_evaluate_one_fq_nmod(betas + i, lfac->poly + i, m, alpha + m - 1, ctx);
    }

    fq_nmod_mpoly_mul(t, lfac->poly + 0, lfac->poly + 1, ctx);
    for (i = 2; i < r; i++)
        fq_nmod_mpoly_mul(t, t, lfac->poly + i, ctx);
    fq_nmod_mpoly_sub(e, A, t, ctx);

    fq_nmod_mpoly_one(pow, ctx);
    fq_nmod_mpoly_gen(g, m, ctx);
    fq_nmod_mpoly_sub_fq_nmod(g, g, alpha + m - 1, ctx);

    fq_nmod_disolve_init(I, lfac->length, m - 1, betas, alpha, ctx);
    deltas = I->deltas + (m - 1)*I->l;

    for (j = 1; j <= degs[m]; j++)
    {
/*
flint_printf("<_mfactor_lift> j = %wd, error (length: %wd)\n", j, e->length); nmod_mpoly_print_pretty(e, NULL, ctx); printf("\n");
*/
        if (fq_nmod_mpoly_is_zero(e, ctx))
        {
            success = 1;
            goto cleanup;
        }

        fq_nmod_mpoly_mul(pow, pow, g, ctx);
        success = fq_nmod_mpoly_divides(q, e, pow, ctx);
        FLINT_ASSERT(success);
        fq_nmod_mpoly_evaluate_one_fq_nmod(t, q, m, alpha + m - 1, ctx);

        success = fq_nmod_mfactor_disolve(A->bits, m - 1, r, t, alpha, degs, I, ctx);
        if (!success)
            goto cleanup;

        for (i = 0; i < r; i++)
        {
            fq_nmod_mpoly_mul(t, deltas + i, pow, ctx);
            fq_nmod_mpoly_add(lfac->poly + i, lfac->poly + i, t, ctx);
        }

        fq_nmod_mpoly_mul(t, lfac->poly + 0, lfac->poly + 1, ctx);
        for (i = 2; i < r; i++)
            fq_nmod_mpoly_mul(t, t, lfac->poly + i, ctx);
        fq_nmod_mpoly_sub(e, A, t, ctx);
    }

    success = fq_nmod_mpoly_is_zero(e, ctx);

cleanup:

    fq_nmod_disolve_clear(I, ctx);

    fq_nmod_mpoly_clear(e, ctx);
    fq_nmod_mpoly_clear(t, ctx);
    fq_nmod_mpoly_clear(pow, ctx);
    fq_nmod_mpoly_clear(g, ctx);
    fq_nmod_mpoly_clear(q, ctx);

    for (i = 0; i < r; i++)
    {
        fq_nmod_mpoly_clear(betas + i, ctx);
    }

    flint_free(betas);

    return success;
}



static int _irreducible_mvar_factors_smprime(
    nmod_mpoly_factor_t fac,
    nmod_mpoly_t A,
    const nmod_mpoly_ctx_t ctx)
{
    int try_count = 0;
    int success;
    const slong n = ctx->minfo->nvars - 1;
    slong i, j, m, r;
    mp_limb_t * alpha;
    nmod_mpoly_struct * Aevals, * newAevals, * lcAevals;
    slong * deg, * degeval, * newdeg;
    nmod_mpoly_t t, g, lcA, newA;
    nmod_mpoly_univar_t u;
    nmod_mpoly_factor_t lfac;
    slong dummyvars[] = {0};
    ulong dummydegs[] = {0};
    flint_rand_t randstate;
	fmpz_t subset;
	nmod_mpoly_factor_t cfac;

    flint_randinit(randstate);

flint_printf("_irreducible_mvar_factors_smprime(n = %wd) called\n", n);
flint_printf("A: "); nmod_mpoly_print_pretty(A, NULL, ctx); printf("\n");

	fmpz_init(subset);
	nmod_mpoly_factor_init(cfac, ctx);
	alpha = (mp_limb_t *) flint_malloc(n*sizeof(mp_limb_t));
    Aevals    = (nmod_mpoly_struct *) flint_malloc(n*sizeof(nmod_mpoly_struct));
    newAevals = (nmod_mpoly_struct *) flint_malloc(n*sizeof(nmod_mpoly_struct));
    lcAevals  = (nmod_mpoly_struct *) flint_malloc(n*sizeof(nmod_mpoly_struct));

    deg     = (slong *) flint_malloc((n + 1)*sizeof(slong));
    degeval = (slong *) flint_malloc((n + 1)*sizeof(slong));
    newdeg  = (slong *) flint_malloc((n + 1)*sizeof(slong));

	for (i = 0; i < n; i++)
	{
		nmod_mpoly_init(Aevals + i, ctx);
		nmod_mpoly_init(newAevals + i, ctx);
		nmod_mpoly_init(lcAevals + i, ctx);
	}

	nmod_mpoly_init(newA, ctx);
	nmod_mpoly_init(lcA, ctx);
	nmod_mpoly_init(t, ctx);
	nmod_mpoly_init(g, ctx);

	nmod_mpoly_univar_init(u, ctx);

    nmod_mpoly_factor_init(lfac, ctx);

	nmod_mpoly_degrees_si(deg, A, ctx);

next_alpha:

    if (++try_count > 10)
	{
		success = 0;
        goto cleanup;
	}

    for (i = 0; i < n; i++)
    {
        alpha[i] = n_urandint(randstate, ctx->ffinfo->mod.n - 1) + 1;
flint_printf("alpha[%wd]: %wu\n", i, alpha[i]);
    }

	/* ensure degrees do not drop under evalutaion */
	i = n - 1;
	nmod_mpoly_evaluate_one_ui(Aevals + i, A, i + 1, alpha[i], ctx);

flint_printf("Aeval[%wd]: ", i ); nmod_mpoly_print_pretty(Aevals + i, NULL, ctx); printf("\n");

	nmod_mpoly_degrees_si(degeval, Aevals + i, ctx);
	for (j = 0; j <= i; j++)
		if (degeval[j] != deg[j])
			goto next_alpha;
	for (i--; i >= 0; i--)
	{
		nmod_mpoly_evaluate_one_ui(Aevals + i, Aevals + i + 1, i + 1, alpha[i], ctx);

flint_printf("Aeval[%wd]: ", i ); nmod_mpoly_print_pretty(Aevals + i, NULL, ctx); printf("\n");

		nmod_mpoly_degrees_si(degeval, Aevals + i, ctx);
		for (j = 0; j <= i; j++)
			if (degeval[j] != deg[j])
				goto next_alpha;
	}

	nmod_mpoly_derivative(t, Aevals + 0, 0, ctx);
	nmod_mpoly_gcd(t, t, Aevals + 0, ctx);

flint_printf("squarefree check gcd: " ); nmod_mpoly_print_pretty(t, NULL, ctx); printf("\n");

	if (!nmod_mpoly_is_one(t, ctx))
		goto next_alpha;

	nmod_mpoly_to_univar(u, Aevals + 1, 0, ctx);
    nmod_mpoly_univar_content_mpoly(t, u, ctx);

flint_printf("content    check gcd: " ); nmod_mpoly_print_pretty(t, NULL, ctx); printf("\n");

	if (!nmod_mpoly_is_one(t, ctx))
		goto next_alpha;

    success = _irreducible_bivar_factors_smprime(lfac, Aevals + 1, 0, 1, ctx);
	if (!success)
        success = _irreducible_bivar_factors_lgprime(lfac, Aevals + 1, 0, 1, ctx);
    if (!success)
        goto next_alpha;

    FLINT_ASSERT(lfac->length > 0);

have_bivar_factors:

flint_printf("nmod have_bivar_factors: \n");
flint_printf("A: "); nmod_mpoly_print_pretty(A, NULL, ctx); printf("\n");
flint_printf("lfac: "); nmod_mpoly_factor_print_pretty(lfac, NULL, ctx); printf("\n");

    if (lfac->length == 1)
    {
	    nmod_mpoly_factor_append_ui(fac, A, 1, ctx);
		success = 1;
		goto cleanup;
    }

	nmod_mpoly_factor_set(cfac, lfac, ctx);

    dummyvars[0] = 0;
    dummydegs[0] = deg[0];
    nmod_mpoly_get_coeff_vars_ui(lcA, A, dummyvars, dummydegs, 1, ctx);

    /* rescale A */
    nmod_mpoly_pow_ui(t, lcA, lfac->length - 1, ctx);
    nmod_mpoly_mul(newA, A, t, ctx);
/*
flint_printf("newA: "); nmod_mpoly_print_pretty(newA, NULL, ctx); printf("\n");
*/
    nmod_mpoly_degrees_si(newdeg, newA, ctx);

    /* evaluations of leading coefficient */
    i = n - 1;
    nmod_mpoly_evaluate_one_ui(lcAevals + i, lcA, i + 1, alpha[i], ctx);
/*
flint_printf("lcAevals[%wd]: ", i); nmod_mpoly_print_pretty(lcAevals + i, NULL, ctx); printf("\n");
*/
    for (i--; i >= 0; i--)
    {
	    nmod_mpoly_evaluate_one_ui(lcAevals + i, lcAevals + i + 1, i + 1, alpha[i], ctx);
/*
flint_printf("lcAevals[%wd]: ", i); nmod_mpoly_print_pretty(lcAevals + i, NULL, ctx); printf("\n");
*/
    }

    /* evaluations */
    i = n - 1;
	nmod_mpoly_evaluate_one_ui(newAevals + i, newA, i + 1, alpha[i], ctx);
/*
flint_printf("newAevals[%wd]: ", i ); nmod_mpoly_print_pretty(newAevals + i, NULL, ctx); printf("\n");
*/
	for (i--; i >= 0; i--)
	{
		nmod_mpoly_evaluate_one_ui(newAevals + i, newAevals + i + 1, i + 1, alpha[i], ctx);
/*
flint_printf("newAevals[%wd]: ", i ); nmod_mpoly_print_pretty(newAevals + i, NULL, ctx); printf("\n");
*/
	}

    /* rescale factors */
    for (i = 0; i < lfac->length; i++)
    {
        dummyvars[0] = 0;
        dummydegs[0] = nmod_mpoly_degree_si(lfac->poly + i, 0, ctx);
        nmod_mpoly_get_coeff_vars_ui(g, lfac->poly + i, dummyvars, dummydegs, 1, ctx);
/*
flint_printf("lfac[%wd]lc: ", i); nmod_mpoly_print_pretty(g, NULL, ctx); printf("\n");
*/
        success = nmod_mpoly_divides(t, lcAevals + 1, g, ctx);
        FLINT_ASSERT(success);
/*
flint_printf("lfac[%wd] t: ", i); nmod_mpoly_print_pretty(g, NULL, ctx); printf("\n");
*/
        nmod_mpoly_mul(lfac->poly + i, lfac->poly + i, t, ctx);
/*
flint_printf("lfac[%wd]: ", i); nmod_mpoly_print_pretty(lfac->poly + i, NULL, ctx); printf("\n");
*/
    }
/*
flint_printf("lift m = %wd, lfac: ", 1); nmod_mpoly_factor_print_pretty(lfac, NULL, ctx); printf("\n");
*/

    for (m = 2; m <= n; m++)
    {
        for (i = 0; i < lfac->length; i++)
        {
            slong dd = nmod_mpoly_degree_si(lfac->poly + i, 0, ctx);
            nmod_mpoly_gen(g, 0, ctx);
            nmod_mpoly_pow_ui(g, g, dd, ctx);
            nmod_mpoly_sub(t, m < n ? lcAevals + m : lcA, lcAevals + m - 1, ctx);
            nmod_mpoly_mul(t, t, g, ctx);
            nmod_mpoly_add(lfac->poly + i, lfac->poly + i, t, ctx);
        }

        success = nmod_mfactor_lift(m, lfac, alpha, m < n ? newAevals + m : newA, newdeg, ctx);
        if (!success)
		{
printf("smprime lift failed!!!!!\n");
            goto combine_factors;
		}
/*
flint_printf("lift m = %wd, lfac: ", m); nmod_mpoly_factor_print_pretty(lfac, NULL, ctx); printf("\n");
*/
    }

    for (i = 0; i < lfac->length; i++)
    {
        nmod_mpoly_to_univar(u, lfac->poly + i, 0, ctx);
        success = nmod_mpoly_univar_content_mpoly(g, u, ctx);
        if (!success)
            goto cleanup;
        success = nmod_mpoly_divides(t, lfac->poly + i, g, ctx);
        FLINT_ASSERT(success);
        nmod_mpoly_factor_append_ui(fac, t, 1, ctx);
    }

	success = 1;
    goto cleanup;

combine_factors:

flint_printf("combine_factors: \n");

	nmod_mpoly_factor_swap(cfac, lfac, ctx);

	FLINT_ASSERT(lfac->length > 1);

	if (lfac->length == 2)
	{
	    nmod_mpoly_factor_append_ui(fac, A, 1, ctx);
        success = 1;
        goto cleanup;
	}

	for (r = 1; r <= lfac->length/2; r++)
	{
		subset_first(subset, lfac->length, r);
		do {

flint_printf("subset(r = %wd): ", r); subset_print(subset, lfac->length);printf("\n");


			nmod_mpoly_factor_one(cfac, ctx);
			nmod_mpoly_factor_fit_length(cfac, 2, ctx);
			cfac->length = 2;
			nmod_mpoly_one(cfac->poly + 0, ctx);
			nmod_mpoly_one(cfac->poly + 1, ctx);
			fmpz_one(cfac->exp + 0);
			fmpz_one(cfac->exp +1);
			for (i = 0; i < lfac->length; i++)
			{
				j = fmpz_tstbit(subset, i);
				nmod_mpoly_mul(cfac->poly + j, cfac->poly + j, lfac->poly + i, ctx);
			}
			
			dummyvars[0] = 0;
			dummydegs[0] = deg[0];
			nmod_mpoly_get_coeff_vars_ui(lcA, A, dummyvars, dummydegs, 1, ctx);

			/* rescale A */
			nmod_mpoly_pow_ui(t, lcA, cfac->length - 1, ctx);
			nmod_mpoly_mul(newA, A, t, ctx);
			nmod_mpoly_degrees_si(newdeg, newA, ctx);

			/* evaluations of leading coefficient */
			i = n - 1;
			nmod_mpoly_evaluate_one_ui(lcAevals + i, lcA, i + 1, alpha[i], ctx);
			for (i--; i >= 0; i--)
			{
			    nmod_mpoly_evaluate_one_ui(lcAevals + i, lcAevals + i + 1, i + 1, alpha[i], ctx);
			}

			/* evaluations */
			i = n - 1;
			nmod_mpoly_evaluate_one_ui(newAevals + i, newA, i + 1, alpha[i], ctx);
			for (i--; i >= 0; i--)
			{
				nmod_mpoly_evaluate_one_ui(newAevals + i, newAevals + i + 1, i + 1, alpha[i], ctx);
			}

			/* rescale factors */
			for (i = 0; i < cfac->length; i++)
			{
			    dummyvars[0] = 0;
			    dummydegs[0] = nmod_mpoly_degree_si(cfac->poly + i, 0, ctx);
			    nmod_mpoly_get_coeff_vars_ui(g, cfac->poly + i, dummyvars, dummydegs, 1, ctx);
			    success = nmod_mpoly_divides(t, lcAevals + 1, g, ctx);
			    FLINT_ASSERT(success);
			    nmod_mpoly_mul(cfac->poly + i, cfac->poly + i, t, ctx);
			}

			for (m = 2; m <= n; m++)
			{
			    for (i = 0; i < cfac->length; i++)
			    {
			        slong dd = nmod_mpoly_degree_si(cfac->poly + i, 0, ctx);
			        nmod_mpoly_gen(g, 0, ctx);
			        nmod_mpoly_pow_ui(g, g, dd, ctx);
			        nmod_mpoly_sub(t, m < n ? lcAevals + m : lcA, lcAevals + m - 1, ctx);
			        nmod_mpoly_mul(t, t, g, ctx);
			        nmod_mpoly_add(cfac->poly + i, cfac->poly + i, t, ctx);
			    }

			    success = nmod_mfactor_lift(m, cfac, alpha, m < n ? newAevals + m : newA, newdeg, ctx);
			    if (!success)
			        goto nextsubset;
			}

		    nmod_mpoly_to_univar(u, cfac->poly + 1, 0, ctx);
		    success = nmod_mpoly_univar_content_mpoly(g, u, ctx);
		    if (!success)
		        goto cleanup;
		    success = nmod_mpoly_divides(t, cfac->poly + 1, g, ctx);
		    FLINT_ASSERT(success);
		    nmod_mpoly_factor_append_ui(fac, t, 1, ctx);

			/* fix A and lfac */
		    success = nmod_mpoly_divides(A, A, t, ctx);
		    FLINT_ASSERT(success);
			nmod_mpoly_degrees_si(deg, A, ctx);
			i = n - 1;
			nmod_mpoly_evaluate_one_ui(Aevals + i, A, i + 1, alpha[i], ctx);
			nmod_mpoly_degrees_si(degeval, Aevals + i, ctx);
			for (j = 0; j <= i; j++)
				FLINT_ASSERT(degeval[j] == deg[j]);
			for (i--; i >= 0; i--)
			{
				nmod_mpoly_evaluate_one_ui(Aevals + i, Aevals + i + 1, i + 1, alpha[i], ctx);
				nmod_mpoly_degrees_si(degeval, Aevals + i, ctx);
				for (j = 0; j <= i; j++)
					FLINT_ASSERT(degeval[j] == deg[j]);
			}

			nmod_mpoly_factor_one(cfac, ctx);
			for (i = 0; i < lfac->length; i++)
			{
				if (!fmpz_tstbit(subset, i))
					nmod_mpoly_factor_append_ui(cfac, lfac->poly + i, 1, ctx);
			}
			nmod_mpoly_factor_swap(lfac, cfac, ctx);
			goto have_bivar_factors;

nextsubset:
			(void) NULL;
		}
		while (subset_next(subset, subset, lfac->length));			
	}

    nmod_mpoly_factor_append_ui(fac, A, 1, ctx);
    success = 1;
	goto cleanup;

cleanup: 
	
	return success;

}














void fq_nmod_mpoly_set_nmod_mpoly(
    fq_nmod_mpoly_t A,
    const fq_nmod_mpoly_ctx_t Actx,
    const nmod_mpoly_t B,
    const nmod_mpoly_ctx_t Bctx)
{
    slong i, N;

    FLINT_ASSERT(Actx->minfo->ord == Bctx->minfo->ord);
    FLINT_ASSERT(Actx->minfo->nvars == Bctx->minfo->nvars);

    fq_nmod_mpoly_fit_bits(A, B->bits, Actx);
    A->bits = B->bits;

    N = mpoly_words_per_exp(B->bits, Bctx->minfo);

    fq_nmod_mpoly_fit_length(A, B->length, Actx);
    A->length = B->length;

    mpoly_copy_monomials(A->exps, B->exps, B->length, N);

    for (i = 0; i < B->length; i++)
        fq_nmod_set_ui(A->coeffs + i, B->coeffs[i], Actx->fqctx);
}



int nmod_mpoly_get_fq_nmod_mpoly(
    nmod_mpoly_t A,
    const nmod_mpoly_ctx_t Actx,
    const fq_nmod_mpoly_t B,
    const fq_nmod_mpoly_ctx_t Bctx)
{
    slong i, N;

    FLINT_ASSERT(Actx->minfo->ord == Bctx->minfo->ord);
    FLINT_ASSERT(Actx->minfo->nvars == Bctx->minfo->nvars);

    nmod_mpoly_fit_bits(A, B->bits, Actx);
    A->bits = B->bits;

    N = mpoly_words_per_exp(B->bits, Bctx->minfo);

    nmod_mpoly_fit_length(A, B->length, Actx);
    A->length = B->length;

    mpoly_copy_monomials(A->exps, B->exps, B->length, N);

    for (i = 0; i < B->length; i++)
    {
        if ((B->coeffs + i)->length == 1)
        {
            A->coeffs[i] = (B->coeffs + i)->coeffs[0];
        }
        else
        {
            return 0;
        }
    }

    return 1;
}


void get_conjugates(fq_nmod_mpoly_struct * C, const fq_nmod_mpoly_t A, slong deg, const fq_nmod_mpoly_ctx_t ctx)
{
    slong i, j;
    for (i = 0; i < deg; i++)
    {
        fq_nmod_mpoly_struct * Ci = C + i;
        fq_nmod_mpoly_set(Ci, A, ctx);
        for (j = 0; j < Ci->length; j++)
        {
            fq_nmod_frobenius(Ci->coeffs + j, Ci->coeffs + j, i, ctx->fqctx);
        }
    }
}


static int _irreducible_mvar_factors_lgprime(
    nmod_mpoly_factor_t fac_,
    const nmod_mpoly_t A_,
    const nmod_mpoly_ctx_t ctx_)
{
    fq_nmod_mpoly_factor_t fac;
    fq_nmod_mpoly_t A;
    fq_nmod_mpoly_ctx_t ctx;
    slong edeg;
    int success;
    const slong n = ctx_->minfo->nvars - 1;
    slong i, j, m, r;
    fq_nmod_struct * alpha;
    fq_nmod_mpoly_struct * Aevals, * newAevals, * lcAevals;
    slong * deg, * degeval, * newdeg;
    fq_nmod_mpoly_t t, g, lcA, newA;
    fq_nmod_mpoly_univar_t u;
    fq_nmod_mpoly_factor_t lfac;
    slong dummyvars[] = {0};
    ulong dummydegs[] = {0};
    flint_rand_t randstate;
	fmpz_t subset;
	fq_nmod_mpoly_factor_t cfac;

	fmpz_init(subset);
    flint_randinit(randstate);

flint_printf("_irreducible_mvar_factors_lgprime(n = %wd) called\n", n);
flint_printf("A_: "); nmod_mpoly_print_pretty(A_, NULL, ctx_); printf("\n");

    edeg = 2;
    FLINT_ASSERT(ctx_->minfo->ord == ORD_LEX);
    fq_nmod_mpoly_ctx_init_deg(ctx, n + 1, ORD_LEX, ctx_->ffinfo->mod.n, edeg);

printf("ctx initialized\n");

    fq_nmod_mpoly_init(A, ctx);
    fq_nmod_mpoly_set_nmod_mpoly(A, ctx, A_, ctx_);

    fq_nmod_mpoly_factor_init(fac, ctx);
	fq_nmod_mpoly_factor_init(cfac, ctx);

flint_printf(" A: "); fq_nmod_mpoly_print_pretty(A, NULL, ctx); printf("\n");


	alpha = (fq_nmod_struct *) flint_malloc(n*sizeof(fq_nmod_struct));
    Aevals    = (fq_nmod_mpoly_struct *) flint_malloc(n*sizeof(fq_nmod_mpoly_struct));
    newAevals = (fq_nmod_mpoly_struct *) flint_malloc(n*sizeof(fq_nmod_mpoly_struct));
    lcAevals  = (fq_nmod_mpoly_struct *) flint_malloc(n*sizeof(fq_nmod_mpoly_struct));

    deg     = (slong *) flint_malloc((n + 1)*sizeof(slong));
    degeval = (slong *) flint_malloc((n + 1)*sizeof(slong));
    newdeg  = (slong *) flint_malloc((n + 1)*sizeof(slong));

	for (i = 0; i < n; i++)
	{
        fq_nmod_init(alpha + i, ctx->fqctx);
        fq_nmod_randtest_not_zero(alpha + i, randstate, ctx->fqctx);
		fq_nmod_mpoly_init(Aevals + i, ctx);
		fq_nmod_mpoly_init(newAevals + i, ctx);
		fq_nmod_mpoly_init(lcAevals + i, ctx);
	}

	fq_nmod_mpoly_init(newA, ctx);
	fq_nmod_mpoly_init(lcA, ctx);
	fq_nmod_mpoly_init(t, ctx);
	fq_nmod_mpoly_init(g, ctx);

	fq_nmod_mpoly_univar_init(u, ctx);

    fq_nmod_mpoly_factor_init(lfac, ctx);

	fq_nmod_mpoly_degrees_si(deg, A, ctx);

    goto got_alpha;

next_alpha:

    edeg++;
    if (edeg > 1000)
        return 0;

    fq_nmod_mpoly_ctx_change_modulus(ctx, edeg);
    fq_nmod_mpoly_set_nmod_mpoly(A, ctx, A_, ctx_);

    for (i = 0; i < n; i++)
    {
        fq_nmod_randtest_not_zero(alpha + i, randstate, ctx->fqctx);
    }

got_alpha:

flint_printf("modulus: "); nmod_poly_print_pretty(ctx->fqctx->modulus, "#"); printf("\n");

    for (i = 0; i < n; i++)
    {
flint_printf("alpha[%wd]: ", i); fq_nmod_print_pretty(alpha + i, ctx->fqctx); printf("\n");
    }

	/* ensure degrees do not drop under evalutaion */
	i = n - 1;
	fq_nmod_mpoly_evaluate_one_fq_nmod(Aevals + i, A, i + 1, alpha + i, ctx);

flint_printf("Aeval[%wd]: ", i ); fq_nmod_mpoly_print_pretty(Aevals + i, NULL, ctx); printf("\n");

	fq_nmod_mpoly_degrees_si(degeval, Aevals + i, ctx);
	for (j = 0; j <= i; j++)
		if (degeval[j] != deg[j])
			goto next_alpha;
	for (i--; i >= 0; i--)
	{
		fq_nmod_mpoly_evaluate_one_fq_nmod(Aevals + i, Aevals + i + 1, i + 1, alpha + i, ctx);

flint_printf("Aeval[%wd]: ", i ); fq_nmod_mpoly_print_pretty(Aevals + i, NULL, ctx); printf("\n");

		fq_nmod_mpoly_degrees_si(degeval, Aevals + i, ctx);
		for (j = 0; j <= i; j++)
			if (degeval[j] != deg[j])
				goto next_alpha;
	}

	fq_nmod_mpoly_derivative(t, Aevals + 0, 0, ctx);
	fq_nmod_mpoly_gcd(t, t, Aevals + 0, ctx);

flint_printf("squarefree check gcd: " ); fq_nmod_mpoly_print_pretty(t, NULL, ctx); printf("\n");

	if (!fq_nmod_mpoly_is_one(t, ctx))
		goto next_alpha;

	fq_nmod_mpoly_to_univar(u, Aevals + 1, 0, ctx);
    fq_nmod_mpoly_univar_content_mpoly(t, u, ctx);

flint_printf("content    check gcd: " ); fq_nmod_mpoly_print_pretty(t, NULL, ctx); printf("\n");

	if (!fq_nmod_mpoly_is_one(t, ctx))
		goto next_alpha;

printf("starting bivar factorization of Aevals[1]: "); fq_nmod_mpoly_print_pretty(Aevals + 1, NULL, ctx); printf("\n");

    success = _fq_nmod_irreducible_bivar_factors_smprime(lfac, Aevals + 1, 0, 1, ctx);
    if (!success)
        goto next_alpha;

have_bivar_factors:

flint_printf("Aevals[1]: "); fq_nmod_mpoly_print_pretty(Aevals + 1, NULL, ctx); printf("\n");
flint_printf("lgprime lfac: "); fq_nmod_mpoly_factor_print_pretty(lfac, NULL, ctx); printf("\n");

#if WANT_ASSERT
	fq_nmod_mpoly_make_monic(t, Aevals + 1, ctx);
	FLINT_ASSERT(fq_nmod_mpoly_factor_matches(t, lfac, ctx));
    FLINT_ASSERT(lfac->length > 0);
#endif

    if (lfac->length == 1)
    {
	    fq_nmod_mpoly_factor_append_ui(fac, A, 1, ctx);
		success = 1;
		goto cleanup;
    }

	fq_nmod_mpoly_factor_set(cfac, lfac, ctx);

    dummyvars[0] = 0;
    dummydegs[0] = deg[0];
    fq_nmod_mpoly_get_coeff_vars_ui(lcA, A, dummyvars, dummydegs, 1, ctx);

    /* rescale A */
    fq_nmod_mpoly_pow_ui(t, lcA, lfac->length - 1, ctx);
    fq_nmod_mpoly_mul(newA, A, t, ctx);
    fq_nmod_mpoly_degrees_si(newdeg, newA, ctx);

    /* evaluations of leading coefficient */
    i = n - 1;
    fq_nmod_mpoly_evaluate_one_fq_nmod(lcAevals + i, lcA, i + 1, alpha + i, ctx);
    for (i--; i >= 0; i--)
	    fq_nmod_mpoly_evaluate_one_fq_nmod(lcAevals + i, lcAevals + i + 1, i + 1, alpha + i, ctx);

    /* evaluations */
    i = n - 1;
	fq_nmod_mpoly_evaluate_one_fq_nmod(newAevals + i, newA, i + 1, alpha + i, ctx);
	for (i--; i >= 0; i--)
		fq_nmod_mpoly_evaluate_one_fq_nmod(newAevals + i, newAevals + i + 1, i + 1, alpha + i, ctx);

    /* rescale factors */
    for (i = 0; i < lfac->length; i++)
    {
        dummyvars[0] = 0;
        dummydegs[0] = fq_nmod_mpoly_degree_si(lfac->poly + i, 0, ctx);
        fq_nmod_mpoly_get_coeff_vars_ui(g, lfac->poly + i, dummyvars, dummydegs, 1, ctx);
        success = fq_nmod_mpoly_divides(t, lcAevals + 1, g, ctx);
        FLINT_ASSERT(success);
        fq_nmod_mpoly_mul(lfac->poly + i, lfac->poly + i, t, ctx);
    }

    for (m = 2; m <= n; m++)
    {
        for (i = 0; i < lfac->length; i++)
        {
            slong dd = fq_nmod_mpoly_degree_si(lfac->poly + i, 0, ctx);
            fq_nmod_mpoly_gen(g, 0, ctx);
            fq_nmod_mpoly_pow_ui(g, g, dd, ctx);
            fq_nmod_mpoly_sub(t, m < n ? lcAevals + m : lcA, lcAevals + m - 1, ctx);
            fq_nmod_mpoly_mul(t, t, g, ctx);
            fq_nmod_mpoly_add(lfac->poly + i, lfac->poly + i, t, ctx);
        }

        success = fq_nmod_mfactor_lift(m, lfac, alpha, m < n ? newAevals + m : newA, newdeg, ctx);
        if (!success)
		{
printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!lgprime lift failed!!!!!\n");
            goto combine_factors;
		}
    }

    for (i = 0; i < lfac->length; i++)
    {
        fq_nmod_mpoly_to_univar(u, lfac->poly + i, 0, ctx);
        success = fq_nmod_mpoly_univar_content_mpoly(g, u, ctx);
        if (!success)
            goto cleanup;
        success = fq_nmod_mpoly_divides(t, lfac->poly + i, g, ctx);
        FLINT_ASSERT(success);
        fq_nmod_mpoly_make_monic(t, t, ctx);
        fq_nmod_mpoly_factor_append_ui(fac, t, 1, ctx);
    }

    success = 1;
	goto cleanup;
	
combine_factors:

	fq_nmod_mpoly_factor_swap(cfac, lfac, ctx);

	FLINT_ASSERT(lfac->length > 1);

	if (lfac->length == 2)
	{
	    fq_nmod_mpoly_factor_append_ui(fac, A, 1, ctx);
        success = 1;
        goto cleanup;
	}

	for (r = 1; r <= lfac->length/2; r++)
	{
		subset_first(subset, lfac->length, r);
		do {

flint_printf("subset(r = %wd): ", r); subset_print(subset, lfac->length);printf("\n");

			fq_nmod_mpoly_factor_one(cfac, ctx);
			fq_nmod_mpoly_factor_fit_length(cfac, 2, ctx);
			cfac->length = 2;
			fq_nmod_mpoly_one(cfac->poly + 0, ctx);
			fq_nmod_mpoly_one(cfac->poly + 1, ctx);
			fmpz_one(cfac->exp + 0);
			fmpz_one(cfac->exp +1);
			for (i = 0; i < lfac->length; i++)
			{
				j = fmpz_tstbit(subset, i);
				fq_nmod_mpoly_mul(cfac->poly + j, cfac->poly + j, lfac->poly + i, ctx);
			}
			
			dummyvars[0] = 0;
			dummydegs[0] = deg[0];
			fq_nmod_mpoly_get_coeff_vars_ui(lcA, A, dummyvars, dummydegs, 1, ctx);

			/* rescale A */
			fq_nmod_mpoly_pow_ui(t, lcA, cfac->length - 1, ctx);
			fq_nmod_mpoly_mul(newA, A, t, ctx);
			fq_nmod_mpoly_degrees_si(newdeg, newA, ctx);

			/* evaluations of leading coefficient */
			i = n - 1;
			fq_nmod_mpoly_evaluate_one_fq_nmod(lcAevals + i, lcA, i + 1, alpha + i, ctx);
			for (i--; i >= 0; i--)
			    fq_nmod_mpoly_evaluate_one_fq_nmod(lcAevals + i, lcAevals + i + 1, i + 1, alpha + i, ctx);

			/* evaluations */
			i = n - 1;
			fq_nmod_mpoly_evaluate_one_fq_nmod(newAevals + i, newA, i + 1, alpha + i, ctx);
			for (i--; i >= 0; i--)
				fq_nmod_mpoly_evaluate_one_fq_nmod(newAevals + i, newAevals + i + 1, i + 1, alpha + i, ctx);

			/* rescale factors */
			for (i = 0; i < cfac->length; i++)
			{
			    dummyvars[0] = 0;
			    dummydegs[0] = fq_nmod_mpoly_degree_si(cfac->poly + i, 0, ctx);
			    fq_nmod_mpoly_get_coeff_vars_ui(g, cfac->poly + i, dummyvars, dummydegs, 1, ctx);

flint_printf("    cfac[%wd]: ",i); fq_nmod_mpoly_print_pretty(cfac->poly + i, NULL, ctx); printf("\n");
flint_printf("          g: "); fq_nmod_mpoly_print_pretty(g, NULL, ctx); printf("\n");
flint_printf("lcAevals[1]: "); fq_nmod_mpoly_print_pretty(lcAevals, NULL, ctx); printf("\n");

			    success = fq_nmod_mpoly_divides(t, lcAevals + 1, g, ctx);
			    FLINT_ASSERT(success);
			    fq_nmod_mpoly_mul(cfac->poly + i, cfac->poly + i, t, ctx);
			}

			for (m = 2; m <= n; m++)
			{
			    for (i = 0; i < cfac->length; i++)
			    {
			        slong dd = fq_nmod_mpoly_degree_si(cfac->poly + i, 0, ctx);
			        fq_nmod_mpoly_gen(g, 0, ctx);
			        fq_nmod_mpoly_pow_ui(g, g, dd, ctx);
			        fq_nmod_mpoly_sub(t, m < n ? lcAevals + m : lcA, lcAevals + m - 1, ctx);
			        fq_nmod_mpoly_mul(t, t, g, ctx);
			        fq_nmod_mpoly_add(cfac->poly + i, cfac->poly + i, t, ctx);
			    }

			    success = fq_nmod_mfactor_lift(m, cfac, alpha, m < n ? newAevals + m : newA, newdeg, ctx);
			    if (!success)
			        goto nextsubset;
			}

		    fq_nmod_mpoly_to_univar(u, cfac->poly + 1, 0, ctx);
		    success = fq_nmod_mpoly_univar_content_mpoly(g, u, ctx);
		    if (!success)
		        goto cleanup;
		    success = fq_nmod_mpoly_divides(t, cfac->poly + 1, g, ctx);
		    FLINT_ASSERT(success);
		    fq_nmod_mpoly_factor_append_ui(fac, t, 1, ctx);

			/* fix A and lfac */
		    success = fq_nmod_mpoly_divides(A, A, t, ctx);
		    FLINT_ASSERT(success);
			fq_nmod_mpoly_degrees_si(deg, A, ctx);
			i = n - 1;
			fq_nmod_mpoly_evaluate_one_fq_nmod(Aevals + i, A, i + 1, alpha + i, ctx);
			fq_nmod_mpoly_degrees_si(degeval, Aevals + i, ctx);
			for (j = 0; j <= i; j++)
				FLINT_ASSERT(degeval[j] == deg[j]);
			for (i--; i >= 0; i--)
			{
				fq_nmod_mpoly_evaluate_one_fq_nmod(Aevals + i, Aevals + i + 1, i + 1, alpha + i, ctx);
				fq_nmod_mpoly_degrees_si(degeval, Aevals + i, ctx);
				for (j = 0; j <= i; j++)
					FLINT_ASSERT(degeval[j] == deg[j]);
			}

			fq_nmod_mpoly_factor_one(cfac, ctx);
			for (i = 0; i < lfac->length; i++)
			{
				if (!fmpz_tstbit(subset, i))
					fq_nmod_mpoly_factor_append_ui(cfac, lfac->poly + i, 1, ctx);
			}
			fq_nmod_mpoly_factor_swap(lfac, cfac, ctx);
			goto have_bivar_factors;

nextsubset:
			(void) NULL;
		}
		while (subset_next(subset, subset, lfac->length));			
	}

    fq_nmod_mpoly_factor_append_ui(fac, A, 1, ctx);
    success = 1;
    goto cleanup;

cleanup:

    nmod_mpoly_factor_one(fac_, ctx_);
    if (success)
	{
        nmod_mpoly_t truefactor_;
        fq_nmod_mpoly_t truefactor;
        fq_nmod_mpoly_struct * conjugates;

printf("now must frob combine\n");

	    fq_nmod_mpoly_set_nmod_mpoly(A, ctx, A_, ctx_);

flint_printf("A: "); fq_nmod_mpoly_print_pretty(A, NULL, ctx); printf("\n");
flint_printf("fac: "); fq_nmod_mpoly_factor_print_pretty(fac, NULL, ctx); printf("\n");

		FLINT_ASSERT(fq_nmod_mpoly_factor_matches(A, fac, ctx));

        conjugates = (fq_nmod_mpoly_struct *) flint_malloc(edeg*sizeof(fq_nmod_mpoly_struct));
        for (i = 0; i < edeg; i++)
            fq_nmod_mpoly_init(conjugates + i, ctx);

        nmod_mpoly_init(truefactor_, ctx_);
        fq_nmod_mpoly_init(truefactor, ctx);

        while (fac->length > 0)
        {
            fq_nmod_mpoly_one(truefactor, ctx);
            get_conjugates(conjugates, fac->poly + 0, edeg, ctx);
flint_printf("fac->poly[0]: "); fq_nmod_mpoly_print_pretty(fac->poly + 0, NULL, ctx);printf("\n");
for (i = 0; i < edeg; i++)
{
flint_printf("conjugate[%wd]: ", i); fq_nmod_mpoly_print_pretty(conjugates + i, NULL, ctx);printf("\n");
}
            for (i = 0; i < fac->length; i++)
            {
                for (j = 0; j < edeg; j++)
                {
                    if (fq_nmod_mpoly_equal(fac->poly + i, conjugates + j, ctx))
                    {
flint_printf("match i = %wd, j= %wd\n", i, j);
                        fq_nmod_mpoly_mul(truefactor, truefactor, fac->poly + i, ctx);
                        fq_nmod_mpoly_swap(fac->poly + i, fac->poly + fac->length - 1, ctx);
                        fac->length--;
                        i--;
						break;
                    }
                }
            }

printf("truefactor: "); fq_nmod_mpoly_print_pretty(truefactor, NULL, ctx); printf("\n");

            success = nmod_mpoly_get_fq_nmod_mpoly(truefactor_, ctx_, truefactor, ctx);
            FLINT_ASSERT(success);
            nmod_mpoly_factor_append_ui(fac_, truefactor_, 1, ctx_);
        } 
    }

    return success;
}


void nmod_mpoly_convert_perm(
    nmod_mpoly_t A,
    flint_bitcnt_t Abits,
    const nmod_mpoly_ctx_t lctx,
    const nmod_mpoly_t B,
    const nmod_mpoly_ctx_t ctx,
    const slong * perm)
{
    slong n = ctx->minfo->nvars;
    slong m = lctx->minfo->nvars;
    slong i, k, l;
    slong NA, NB;
    ulong * Aexps;
    ulong * Bexps;
    TMP_INIT;

    FLINT_ASSERT(B->length > 0);
    FLINT_ASSERT(Abits <= FLINT_BITS);
    FLINT_ASSERT(B->bits <= FLINT_BITS);
    TMP_START;

    Aexps = (ulong *) TMP_ALLOC((m)*sizeof(ulong));
    Bexps = (ulong *) TMP_ALLOC(n*sizeof(ulong));

    NA = mpoly_words_per_exp(Abits, lctx->minfo);
    NB = mpoly_words_per_exp(B->bits, ctx->minfo);

    nmod_mpoly_fit_bits(A, Abits, ctx);
    A->bits = Abits;
    nmod_mpoly_fit_length(A, B->length, lctx);
    A->length = B->length;
    for (i = 0; i < B->length; i++)
    {        
	    A->coeffs[i] = B->coeffs[i];
	    mpoly_get_monomial_ui(Bexps, B->exps + NB*i, B->bits, ctx->minfo);
	    for (k = 0; k < m; k++)
	    {
	        l = perm[k];
	        Aexps[k] = l < 0 ? 0 : Bexps[l];
	    }
	    mpoly_set_monomial_ui(A->exps + NA*(i), Aexps, Abits, lctx->minfo);
     }  
    TMP_END;
    nmod_mpoly_sort_terms(A, lctx);
}

/*
    A is primitive w.r.t to any variable appearing in A.
    A is square free and monic.

    return 1 for success, 0 for failure
*/
static int _irreducible_factors(
    nmod_mpoly_factor_t f,
    const nmod_mpoly_t A,
    const nmod_mpoly_ctx_t ctx)
{
    int success;
    slong i, t, nzdvar, mvars;
    slong * Adegs, * perm, * iperm;
    ulong * shift, * stride;
    nmod_mpoly_t G, Abar, Bbar, nzdpoly;
    TMP_INIT;

    if (A->bits > FLINT_BITS)
        return 0;

    TMP_START;

    nmod_mpoly_init(G, ctx);
    nmod_mpoly_init(Abar, ctx);
    nmod_mpoly_init(Bbar, ctx);
    nmod_mpoly_init(nzdpoly, ctx);

    Adegs = (slong *) TMP_ALLOC(ctx->minfo->nvars*sizeof(slong));
    perm = (slong *) TMP_ALLOC(ctx->minfo->nvars*sizeof(slong));
    iperm = (slong *) TMP_ALLOC(ctx->minfo->nvars*sizeof(slong));
	shift = (ulong *) TMP_ALLOC(ctx->minfo->nvars*sizeof(slong));
    stride = (ulong *) TMP_ALLOC(ctx->minfo->nvars*sizeof(slong));
    nmod_mpoly_degrees_si(Adegs, A, ctx);

    mvars = 0;

    nzdvar = -1;
    for (i = 0; i < ctx->minfo->nvars; i++)
    {
        shift[i] = 0;
        stride[i] = 1;
		iperm[i] = -1;
        if (Adegs[i] > 0)
        {
            perm[mvars] = i;
            mvars++;

            if (nzdvar < 0)
            {
                nmod_mpoly_derivative(nzdpoly, A, i, ctx);
                if (!nmod_mpoly_is_zero(nzdpoly, ctx))
                {
                    nzdvar = i;
                    t = perm[mvars - 1];
                    perm[mvars - 1] = perm[0];
                    perm[0] = t;
                }
            }
        }
    }

    FLINT_ASSERT(nzdvar >= 0);
    FLINT_ASSERT(perm[0] == nzdvar);
    FLINT_ASSERT(!nmod_mpoly_is_zero(nzdpoly, ctx));

    nmod_mpoly_gcd_cofactors(G, Abar, Bbar, A, nzdpoly, ctx);
    if (!nmod_mpoly_is_one(G, ctx))
    {
        nmod_mpoly_factor_t newf;
        nmod_mpoly_factor_init(newf, ctx);
        success = _irreducible_factors(f, Abar, ctx);
        success = success && _irreducible_factors(newf, Bbar, ctx);
        nmod_mpoly_factor_mul_factor_ui(f, newf, 1, ctx);
        nmod_mpoly_factor_clear(newf, ctx);
        return success;
    }

	/* perm[0] should be a variable (nzdvar) with nonzero derivative */

    /* TODO nice permutation */

    /* invert perm */
	for (i = 0; i < mvars; i++)
		iperm[perm[i]] = i;

    if (mvars == 1)
    {
        nmod_mpoly_t tt;
        nmod_poly_t Au;
        nmod_poly_factor_t fu;

        nmod_mpoly_init(tt, ctx);
        nmod_poly_init_mod(Au, ctx->ffinfo->mod);
        nmod_poly_factor_init(fu);

        _nmod_mpoly_to_nmod_poly_deflate(Au, A, perm[0], shift, stride, ctx);

        f->content = nmod_poly_factor(fu, Au); /* leading coeff should be 1 */
        f->length = 0; 
        for (i = 0; i < fu->num; i++)
        {
            _nmod_mpoly_from_nmod_poly_inflate(tt, A->bits, fu->p + i, perm[0], shift, stride, ctx);
            nmod_mpoly_factor_append_ui(f, tt, fu->exp[i], ctx); /* fu->exp[i] should be 1 */
        }

        nmod_mpoly_clear(tt, ctx);
        nmod_poly_clear(Au);
        nmod_poly_factor_clear(fu);

        success = 1;
    }
    else if (mvars == 2)
    {
        success = _irreducible_bivar_factors_smprime(f, A, perm[0], perm[1], ctx);
		if (!success)
	        success = _irreducible_bivar_factors_lgprime(f, A, perm[0], perm[1], ctx);
    }
    else
    {
		nmod_mpoly_ctx_t lctx;
		nmod_mpoly_t Al, B;
		nmod_mpoly_factor_t Amfactors;

		nmod_mpoly_ctx_init(lctx, mvars, ORD_LEX, ctx->ffinfo->mod.n);
		nmod_mpoly_init(Al, lctx);
        nmod_mpoly_init(B, ctx);
		nmod_mpoly_factor_init(Amfactors, lctx);

		nmod_mpoly_convert_perm(Al, A->bits, lctx, A, ctx, perm);

		success = _irreducible_mvar_factors_smprime(Amfactors, Al, lctx);
        if (!success)
    		success = _irreducible_mvar_factors_lgprime(Amfactors, Al, lctx);

		if (success)
        {
            nmod_mpoly_factor_one(f, ctx);
		    for (i = 0; i < Amfactors->length; i++)
		    {
			    nmod_mpoly_convert_perm(B, A->bits, ctx, Amfactors->poly + i, lctx, iperm);
                nmod_mpoly_make_monic(B, B, ctx);
			    nmod_mpoly_factor_append_fmpz(f, B, Amfactors->exp + i, ctx);
		    }
		    nmod_mpoly_factor_clear(Amfactors, lctx);
		    nmod_mpoly_clear(B, ctx);
		    nmod_mpoly_clear(Al, lctx);
		    nmod_mpoly_ctx_clear(lctx);
        }
    }

    TMP_END;

    return success;
}



static int _squarefree_factors(
	nmod_mpoly_factor_t f,
	const nmod_mpoly_t A,
	const nmod_mpoly_ctx_t ctx)
{
	int success;
	slong var;
    ulong k;
	fmpz * shift, * stride;
	nmod_mpoly_factor_t tempf;
	fmpz_t g, gr, p, pk;
    nmod_mpoly_t B, C, U, V, W, G;

    nmod_mpoly_init(B, ctx);
    nmod_mpoly_init(C, ctx);
    nmod_mpoly_init(U, ctx);
    nmod_mpoly_init(V, ctx);
    nmod_mpoly_init(W, ctx);
    nmod_mpoly_init(G, ctx);

	fmpz_init_set_ui(p, ctx->ffinfo->mod.n);
	fmpz_init(pk);
	fmpz_init(g);
	fmpz_init(gr);
	nmod_mpoly_factor_init(tempf, ctx);
	shift = _fmpz_vec_init(ctx->minfo->nvars);
	stride = _fmpz_vec_init(ctx->minfo->nvars);

	nmod_mpoly_factor_one(f, ctx);
	nmod_mpoly_set(C, A, ctx);

	for (var = 0; var < ctx->minfo->nvars; var++)
	{
        nmod_mpoly_derivative(G, C, var, ctx);
        success = nmod_mpoly_gcd_cofactors(C, W, V, C, G, ctx);
        if (!success)
            goto cleanup;

        for (k = 1; k + 1 < ctx->ffinfo->mod.n &&
                            !(nmod_mpoly_derivative(G, W, var, ctx),
                              nmod_mpoly_sub(U, V, G, ctx),
                              nmod_mpoly_is_zero(U, ctx)); k++)
        {
            success = nmod_mpoly_gcd_cofactors(G, W, V, W, U, ctx);
            if (!success)
	            goto cleanup;

            nmod_mpoly_factor_mul_mpoly_ui(f, G, k, ctx);

            if (!nmod_mpoly_is_one(W, ctx))
            {
                success = nmod_mpoly_divides(U, C, W, ctx); FLINT_ASSERT(success);
                nmod_mpoly_swap(C, U, ctx);
            }
        }

        nmod_mpoly_factor_mul_mpoly_ui(f, W, k, ctx);
	}

	if (nmod_mpoly_is_ui(C, ctx))
	{
		f->content = nmod_mul(f->content, nmod_mpoly_get_ui(C, ctx), ctx->ffinfo->mod);
	}
	else
	{
		nmod_mpoly_deflation(shift, stride, C, ctx);
		fmpz_zero(g);
		for (var = 0; var < ctx->minfo->nvars; var++)
		{
			fmpz_gcd(g, g, stride + var);
			fmpz_gcd(g, g, shift + var);
		}

		FLINT_ASSERT(fmpz_remove(gr, g, p) > 0);
		fmpz_pow_ui(pk, p, fmpz_remove(gr, g, p));
		for (var = 0; var < ctx->minfo->nvars; var++)
		{
			fmpz_set(stride + var, pk);
			fmpz_zero(shift + var);
		}

		nmod_mpoly_deflate(C, C, shift, stride, ctx);

		success = _squarefree_factors(tempf, C, ctx);
		if (!success)
			goto cleanup;

		nmod_mpoly_factor_pow_fmpz(tempf, tempf, pk, ctx);
		nmod_mpoly_factor_mul_pairwise_prime(f, f, tempf, ctx);
	}

	success = 1;

cleanup:

    nmod_mpoly_clear(C, ctx);
    nmod_mpoly_clear(U, ctx);
    nmod_mpoly_clear(V, ctx);
    nmod_mpoly_clear(W, ctx);
    nmod_mpoly_clear(G, ctx);

	fmpz_clear(p);
	fmpz_clear(pk);
	fmpz_clear(g);
	fmpz_clear(gr);
	nmod_mpoly_factor_clear(tempf, ctx);
	_fmpz_vec_clear(shift, ctx->minfo->nvars);
	_fmpz_vec_clear(stride, ctx->minfo->nvars);

	return success;
}



int nmod_mpoly_factor(
    nmod_mpoly_factor_t f,
    const nmod_mpoly_t A,
    int full,
    const nmod_mpoly_ctx_t ctx)
{
    int success;
    slong j, v;
    nmod_mpoly_t c;
    nmod_mpoly_univar_t u;
    nmod_mpoly_factor_t newf, tempf;
/*
flint_printf("nmod_mpoly_factor called\nA: "); nmod_mpoly_print_pretty(A, NULL, ctx); printf("\n");
*/
    /* 0. set trivial factorization */
    f->length = 0;
    if (nmod_mpoly_is_ui(A, ctx))
    {
		f->content = nmod_mpoly_get_ui(A, ctx);
        return 1;
    }
    else
    {
		f->content = A->coeffs[0];
		nmod_mpoly_factor_fit_length(f, 1, ctx);
		nmod_mpoly_scalar_mul_ui(f->poly + 0, A, nmod_inv(f->content, ctx->ffinfo->mod), ctx);
		fmpz_one(f->exp + 0);
		f->length = 1;
    }

    if (A->bits > FLINT_BITS)
    {
        return 0;
    }

    nmod_mpoly_factor_init(newf, ctx);
    nmod_mpoly_factor_init(tempf, ctx);
    nmod_mpoly_univar_init(u, ctx);
    nmod_mpoly_init(c, ctx);

    /* 1. ensure factors are primitive w.r.t any variable */
    FLINT_ASSERT(nmod_mpoly_factor_matches(A, f, ctx));

    for (v = 0; v < ctx->minfo->nvars; v++)
    {
        newf->content = f->content;
        newf->length = 0;
        for (j = 0; j < f->length; j++)
        {
            if (fmpz_is_one(f->exp + j))
            {
                nmod_mpoly_to_univar(u, f->poly + j, v, ctx);
                FLINT_ASSERT(u->length > 0);

                success = nmod_mpoly_univar_content_mpoly(c, u, ctx);
                if (!success)
                    goto cleanup;

                nmod_mpoly_univar_divexact_mpoly(u, c, ctx);

                nmod_mpoly_factor_mul_mpoly_ui(newf, c, 1, ctx);

                if (u->exps[u->length - 1] != 0)
                {
                    nmod_mpoly_gen(c, v, ctx);
                    nmod_mpoly_factor_append_ui(newf, c, u->exps[u->length - 1], ctx);
                    nmod_mpoly_univar_shift_right(u, u->exps[u->length - 1], ctx);
                }

                if (u->length > 1)
                {
                    nmod_mpoly_from_univar_bits(c, A->bits, u, v, ctx);
                    nmod_mpoly_factor_append_ui(newf, c, 1, ctx);
                }
                else
                {
                    FLINT_ASSERT(nmod_mpoly_is_one(u->coeffs + 0, ctx));
                }
            }
            else
            {
                FLINT_ASSERT(fmpz_sgn(f->exp + j) > 0);
                nmod_mpoly_factor_append_fmpz(newf, f->poly + j, f->exp + j, ctx);
            }
        }

        nmod_mpoly_factor_swap(f, newf, ctx);
    }

    /* 2. ensure factors are squarefree */
    FLINT_ASSERT(nmod_mpoly_factor_matches(A, f, ctx));
	FLINT_ASSERT(nmod_mpoly_factor_is_pairwise_prime(f, ctx) != 0);

    newf->content = f->content;
    newf->length = 0;
    for (j = 0; j < f->length; j++)
    {
		success = _squarefree_factors(tempf, f->poly + j, ctx);
		if (!success)
			goto cleanup;
		nmod_mpoly_factor_mul_factor_fmpz(newf, tempf, f->exp + j, ctx);
    }
    nmod_mpoly_factor_swap(f, newf, ctx);

    /* skip irreducible factorization if not wanted */
    if (!full)
    {
        success = 1;
        goto cleanup;
    }

    /* 3. ensure factors are irreducible */
    FLINT_ASSERT(nmod_mpoly_factor_matches(A, f, ctx));

    newf->content = f->content;
    newf->length = 0;
    for (j = 0; j < f->length; j++)
    {
        success = _irreducible_factors(tempf, f->poly + j, ctx);
        if (!success)
            goto cleanup;
        nmod_mpoly_factor_mul_factor_fmpz(newf, tempf, f->exp + j, ctx);
    }
    nmod_mpoly_factor_swap(f, newf, ctx);

    success = 1;

cleanup:

	if (success)
	{
	    FLINT_ASSERT(nmod_mpoly_factor_matches(A, f, ctx));
	}

    nmod_mpoly_factor_clear(tempf, ctx);
    nmod_mpoly_factor_clear(newf, ctx);
    nmod_mpoly_univar_clear(u, ctx);
    nmod_mpoly_clear(c, ctx);

    return success;
}
