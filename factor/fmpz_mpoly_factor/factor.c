/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "factor/fmpz_mpoly_factor.h"
#include "flint/fmpq_poly.h"
#include "flint/profiler.h"


void fmpz_mpoly_factor_expand(
    fmpz_mpoly_t A,
    const fmpz_mpoly_factor_t f,
    const fmpz_mpoly_ctx_t ctx)
{
    slong i;
    fmpz_mpoly_t t1, t2;

    fmpz_mpoly_init(t1, ctx);
    fmpz_mpoly_init(t2, ctx);

    fmpz_mpoly_set_fmpz(A, f->content, ctx);

    for (i = 0; i < f->length; i++)
    {
        fmpz_mpoly_pow_fmpz(t1, f->poly + i, f->exp + i, ctx);
        fmpz_mpoly_mul(t2, A, t1, ctx);
        fmpz_mpoly_swap(A, t2, ctx);
    }

    fmpz_mpoly_clear(t1, ctx);
    fmpz_mpoly_clear(t2, ctx);
}

int fmpz_mpoly_factor_matches(
    const fmpz_mpoly_t A,
    const fmpz_mpoly_factor_t f,
    const fmpz_mpoly_ctx_t ctx)
{
    int matches;
    fmpz_mpoly_t T;
    fmpz_mpoly_init(T, ctx);
    fmpz_mpoly_factor_expand(T, f, ctx);
    matches = fmpz_mpoly_equal(T, A, ctx);
    fmpz_mpoly_clear(T, ctx);
    return matches;
}


/*********** mpoly_vec ************/

typedef struct
{
    fmpz_mpoly_struct * coeffs;
    slong alloc;
    slong length;
} fmpz_mpolyv_struct;

typedef fmpz_mpolyv_struct fmpz_mpolyv_t[1];

void fmpz_mpolyv_init(fmpz_mpolyv_t A, const fmpz_mpoly_ctx_t ctx)
{
    A->coeffs = NULL;
    A->alloc = 0;
    A->length = 0;
}


void fmpz_mpolyv_clear(fmpz_mpolyv_t A, const fmpz_mpoly_ctx_t ctx)
{
    slong i;
    for (i = 0; i < A->alloc; i++)
        fmpz_mpoly_clear(A->coeffs + i, ctx);
    flint_free(A->coeffs);
}


void fmpz_mpolyv_swap(
    fmpz_mpolyv_t A,
    fmpz_mpolyv_t B,
    const fmpz_mpoly_ctx_t ctx)
{
   fmpz_mpolyv_struct t = *A;
   *A = *B;
   *B = t;
}

void fmpz_mpolyv_print_pretty(
    const fmpz_mpolyv_t poly,
    const char ** x,
    const fmpz_mpoly_ctx_t ctx)
{
    slong i;
    for (i = 0; i < poly->length; i++)
    {
        flint_printf("coeff[%wd]: ", i);
        fmpz_mpoly_print_pretty(poly->coeffs + i, x, ctx);
        flint_printf("\n");
    }
}

void fmpz_mpolyv_fit_length(
    fmpz_mpolyv_t A,
    slong length,
    const fmpz_mpoly_ctx_t ctx)
{
    slong i;
    slong old_alloc = A->alloc;
    slong new_alloc = FLINT_MAX(length, 2*A->alloc);

    if (length > old_alloc)
    {
        if (old_alloc == 0)
        {
            A->coeffs = (fmpz_mpoly_struct *) flint_malloc(
                                          new_alloc*sizeof(fmpz_mpoly_struct));
        }
        else
        {
            A->coeffs = (fmpz_mpoly_struct *) flint_realloc(A->coeffs,
                                          new_alloc*sizeof(fmpz_mpoly_struct));
        }

        for (i = old_alloc; i < new_alloc; i++)
        {
            fmpz_mpoly_init(A->coeffs + i, ctx);
        }
        A->alloc = new_alloc;
    }
}

void fmpz_mpolyv_set_coeff(
    fmpz_mpolyv_t A,
    slong i,
    fmpz_mpoly_t c, /* clobbered */
    const fmpz_mpoly_ctx_t ctx)
{
    slong j;
    FLINT_ASSERT(!fmpz_mpoly_is_zero(c, ctx));
    fmpz_mpolyv_fit_length(A, i + 1, ctx);
    for (j = A->length; j < i; j++)
        fmpz_mpoly_zero(A->coeffs + j, ctx);
    fmpz_mpoly_swap(A->coeffs + i, c, ctx);
    A->length = FLINT_MAX(A->length, i + 1);
}


/************** subsets and tuples ****************************************/

void subset_first(fmpz_t a, slong n, slong r)
{
    FLINT_ASSERT(r >= 0);
    FLINT_ASSERT(r <= n);
    fmpz_one(a);
    fmpz_mul_2exp(a, a, r); 
    fmpz_sub_ui(a, a, 1);
}

int subset_next(fmpz_t a, const fmpz_t b, slong n)
{
    slong t1, t2, i;
    int r;
    if (a == b)
    {
        fmpz_t t;
        fmpz_init(t);
        r = subset_next(t, b, n);
        fmpz_swap(a, t);
        fmpz_clear(t);
        return r;
    }

    i = 0;
    while (i < n && fmpz_tstbit(b, i) == 0)
        i++;
    t1 = i;
/*flint_printf("t1: %wd\n", t1);*/
    while (i<n && fmpz_tstbit(b,i) == 1)
        i++;
    t2 = i;
/*flint_printf("t2: %wd\n", t2);*/
    if (t2 < n)
    {
        fmpz_t t;
        fmpz_init_set_ui(t, 1);
        fmpz_one(a);
        fmpz_mul_2exp(a, a, n - t2);
        fmpz_sub_ui(a, a, 1);
        fmpz_mul_2exp(a, a, t2);
        fmpz_and(a, b, a);
        fmpz_setbit(a, t2);
        if (t2 > t1)
            fmpz_mul_2exp(t, t, t2 - t1 - 1);
        fmpz_sub_ui(t, t, 1);
        fmpz_add(a, a, t);
        fmpz_clear(t);
        return 1;
    }
    else
    {
        return 0;
    }
}

void subset_print(const fmpz_t a, slong n)
{
    slong i;
    for (i = n - 1; i >= 0; i --)
    {
        flint_printf("%d",fmpz_tstbit(a, i));
    }
}

void subset_map_down(fmpz_t a, const fmpz_t b, const fmpz_t m)
{
    ulong i, j, bbits = fmpz_bits(b);

    FLINT_ASSERT(a != b);
    FLINT_ASSERT(a != m);

    j = 0;
    fmpz_zero(a);
    for (i = 0; i < bbits; i++)
    {
        if (fmpz_tstbit(b, i))
        {
            FLINT_ASSERT(!fmpz_tstbit(m, i));
            fmpz_setbit(a, j++);
        }
        else
        {
            j += !fmpz_tstbit(m, i);
        }
    }
}


void tuple_print(fmpz * alpha, slong n)
{
    slong j;
    for (j = 0; j < n; j++)
    {
        fmpz_print(alpha + j);
        printf(j + 1 < n ? ", " : "\n");
    }
}


/* ensure that the first m values change upon the next call to tuple_next*/
void tuple_saturate(fmpz * alpha, slong n, slong m)
{
    slong i;

    for (i = m + 1; i < n; i++)
    {
        fmpz_add(alpha + m, alpha + m, alpha + i);
        fmpz_zero(alpha + i);
    }

    if (m < n && fmpz_is_zero(alpha + m))
    {
        for (i = 0; i < m; i++)
            if (!fmpz_is_zero(alpha + i))
                return;
        fmpz_one(alpha + m);
    }
}



void tuple_next(fmpz * alpha, slong n)
{
    slong i, t1, t2, t3;
    fmpz_t sum;

    fmpz_init(sum);
    for (i = 0; i < n; i++)
        fmpz_add(sum, sum, alpha + i);

    i = n - 1;
    while(i >= 0 && fmpz_is_zero(alpha + i))
        i--;
    t1 = i;
    while(i >= 0 && fmpz_cmp(alpha + i, sum) != 0)
        i--;
    t2 = i;
    while(i >= 0 && fmpz_cmp(alpha + i, sum) == 0)
        i--;
    t3 = i;

    if (t1 > 0 && t1 != t2)
    {
        fmpz_swap(alpha + t1, alpha + n - 1);
        fmpz_sub_ui(alpha + n - 1, alpha + n - 1, 1);
        fmpz_add_ui(alpha + t1 - 1, alpha + t1 - 1, 1);
    }
    else if (t1 > 0 && t1 == t2 && t3 >= 0)
    {
        fmpz_add_ui(alpha + t3, alpha + t3, 1);
        fmpz_zero(alpha + t3 + 1);
        fmpz_sub_ui(alpha + n - 1, sum, 1);
    }
    else
    {
        fmpz_add_ui(alpha + n - 1, alpha + 0, 1);
        if (n > 1)
            fmpz_zero(alpha + 0);
    }

    fmpz_clear(sum);
}


/********* univar ************************************************************/

int fmpz_mpoly_univar_content_mpoly(
    fmpz_mpoly_t g,
    const fmpz_mpoly_univar_t A,
    const fmpz_mpoly_ctx_t ctx)
{
    slong i;
    int success;

    fmpz_mpoly_zero(g, ctx);
    for (i = 0; i < A->length; i++)
    {
        success = fmpz_mpoly_gcd(g, g, A->coeffs + i, ctx);
        if (!success)
            return 0;
    }

    return 1;
}

void fmpz_mpoly_univar_divexact_mpoly(
    fmpz_mpoly_univar_t A,
    const fmpz_mpoly_t b,
    const fmpz_mpoly_ctx_t ctx)
{
    int success;
    slong i;

    for (i = 0; i < A->length; i++)
    {
        success = fmpz_mpoly_divides(A->coeffs + i, A->coeffs + i, b, ctx);
        FLINT_ASSERT(success);
    }
}

void fmpz_mpoly_univar_shift_right(
    fmpz_mpoly_univar_t A,
    ulong shift,
    const fmpz_mpoly_ctx_t ctx)
{
    slong i;
    for (i = 0; i < A->length; i++)
    {
        fmpz_sub_ui(A->exps + i, A->exps + i, shift);
        FLINT_ASSERT(fmpz_sgn(A->exps + i) >= 0);
    }
}

/********* fmpz_mpoly_factor_t ***********************************************/

void fmpz_mpoly_factor_one(
    fmpz_mpoly_factor_t f,
    const fmpz_mpoly_ctx_t ctx)
{
    fmpz_one(f->content);
    f->length = 0;
}

void fmpz_mpoly_factor_mul_mpoly_fmpz(
    fmpz_mpoly_factor_t f,
    const fmpz_mpoly_t A,
    const fmpz_t e,
    const fmpz_mpoly_ctx_t ctx)
{
    if (fmpz_mpoly_is_fmpz(A, ctx))
    {
        fmpz_t t;
        fmpz_init(t);
        fmpz_mpoly_get_fmpz(t, A, ctx);
        fmpz_pow_fmpz(t, t, e);
        fmpz_mul(f->content, f->content, t);
        fmpz_clear(t);
    }
    else
    {
        fmpz_mpoly_factor_append_fmpz(f, A, e, ctx);
    }
}

void fmpz_mpoly_factor_mul_mpoly_ui(
    fmpz_mpoly_factor_t f,
    const fmpz_mpoly_t A,
    ulong e,
    const fmpz_mpoly_ctx_t ctx)
{
    if (fmpz_mpoly_is_fmpz(A, ctx))
    {
        fmpz_t t;
        fmpz_init(t);
        fmpz_mpoly_get_fmpz(t, A, ctx);
        fmpz_pow_ui(t, t, e);
        fmpz_mul(f->content, f->content, t);
        fmpz_clear(t);
    }
    else
    {
        fmpz_mpoly_factor_append_ui(f, A, e, ctx);
    }
}

/* a *= b^e */
void fmpz_mpoly_factor_mul_factor_fmpz(
    fmpz_mpoly_factor_t a,
    const fmpz_mpoly_factor_t b,
    const fmpz_t e,
    const fmpz_mpoly_ctx_t ctx)
{
    slong i;
    fmpz_t t;

    fmpz_init(t);

    fmpz_pow_fmpz(t, b->content, e);
    fmpz_mul(a->content, a->content, t);

    for (i = 0; i < b->length; i++)
    {
        fmpz_mul(t, b->exp + i, e);
        fmpz_mpoly_factor_append_fmpz(a, b->poly + i, t, ctx);
    }

    fmpz_clear(t);
}


/******* fmpz_mod_bpoly - (Z/nZ)[x,y] ****************************************/

typedef struct
{
    fmpz_mod_poly_struct * coeffs;
    slong alloc;
    slong length;
    fmpz_t modulus;
} fmpz_mod_bpoly_struct;

typedef fmpz_mod_bpoly_struct fmpz_mod_bpoly_t[1];


/******* fmpz_bpoly - Z[x,y] *************************************************/

typedef struct
{
    fmpz_poly_struct * coeffs;
    slong alloc;
    slong length;
} fmpz_bpoly_struct;

typedef fmpz_bpoly_struct fmpz_bpoly_t[1];


void fmpz_mod_bpoly_init(fmpz_mod_bpoly_t A, const fmpz_t modulus)
{
    A->coeffs = NULL;
    A->alloc = 0;
    A->length = 0;
    fmpz_init_set(A->modulus, modulus);
}

void fmpz_mod_bpoly_clear(fmpz_mod_bpoly_t A)
{
    fmpz_clear(A->modulus);
    if (A->coeffs)
    {
        slong i;
        for (i = 0; i < A->alloc; i++)
            fmpz_mod_poly_clear(A->coeffs + i);
        flint_free(A->coeffs);
    }
}

void fmpz_mod_bpoly_swap(fmpz_mod_bpoly_t A, fmpz_mod_bpoly_t B)
{
    fmpz_mod_bpoly_struct t = *A;
    *A = *B;
    *B = t;
}

void fmpz_bpoly_swap(fmpz_bpoly_t A, fmpz_bpoly_t B)
{
    fmpz_bpoly_struct t = *A;
    *A = *B;
    *B = t;
}

void fmpz_mod_bpoly_print(fmpz_mod_bpoly_t A, const char * xvar, const char * yvar)
{
    slong i;
    int first;

    first = 1;
    for (i = A->length - 1; i >= 0; i--)
    {
        if (fmpz_mod_poly_is_zero(A->coeffs + i))
            continue;

        if (!first)
            flint_printf(" + ");

        first = 0;

        flint_printf("(");
        fmpz_mod_poly_print_pretty(A->coeffs + i, yvar);
        flint_printf(")*%s^%wd", xvar, i);
    }

    if (first)
        flint_printf("0");
}

void fmpz_mod_bpoly_fit_length(fmpz_mod_bpoly_t A, slong len)
{
    slong i;

    if (len <= A->alloc)
        return;

    if (len < 2 * A->alloc)
        len = 2 * A->alloc;

    if (A->alloc == 0)
        A->coeffs = (fmpz_mod_poly_struct *) flint_malloc(len * sizeof(fmpz_mod_poly_struct));
    else
        A->coeffs = (fmpz_mod_poly_struct *) flint_realloc(A->coeffs, len * sizeof(fmpz_mod_poly_struct));

    for (i = A->alloc; i < len; i++)
        fmpz_mod_poly_init(A->coeffs + i, A->modulus);

    A->alloc = len;
}

void fmpz_mod_bpoly_set_fmpz_bpoly(fmpz_mod_bpoly_t A, const fmpz_bpoly_t B)
{
    slong i;
    fmpz_mod_bpoly_fit_length(A, B->length);
    for (i = 0; i < B->length; i++)
    {
        fmpz_mod_poly_set_fmpz_poly(A->coeffs + i, B->coeffs + i);
        if (!fmpz_mod_poly_is_zero(A->coeffs + i))
            A->length = i + 1;
    }
}

void fmpz_mod_bpoly_set_polyx(fmpz_mod_bpoly_t A, const fmpz_mod_poly_t B)
{
    slong i;
    fmpz_mod_bpoly_fit_length(A, B->length);
    A->length = 0;
    for (i = 0; i < B->length; i++)
    {
        fmpz_mod_poly_set_fmpz(A->coeffs + i, B->coeffs + i);
        if (!fmpz_mod_poly_is_zero(A->coeffs + i))
            A->length = i + 1;
    }    
}

void fmpz_mod_bpoly_set_polyy(fmpz_mod_bpoly_t A, const fmpz_mod_poly_t B)
{
    fmpz_mod_bpoly_fit_length(A, 1);
    fmpz_mod_poly_set(A->coeffs + 0, B);
    A->length = !fmpz_mod_poly_is_zero(A->coeffs + 0);
}


void fmpz_mod_bpoly_get_coeff(fmpz_t c, const fmpz_mod_bpoly_t A, slong xi, slong yi)
{
    if (xi >= A->length)
        fmpz_zero(c);

    fmpz_mod_poly_get_coeff_fmpz(c, A->coeffs + xi, yi);
}

void fmpz_mod_bpoly_make_monic(fmpz_mod_bpoly_t A, slong order)
{
    slong i;
    fmpz_mod_poly_t t, lcinv;

    FLINT_ASSERT(A->length > 0);

    fmpz_mod_poly_init(t, A->modulus);
    fmpz_mod_poly_init(lcinv, A->modulus);
    fmpz_mod_poly_inv_series(lcinv, A->coeffs + A->length - 1, order);

    for (i = 0; i < A->length; i++)
    {
        fmpz_mod_poly_mullow(t, A->coeffs + i, lcinv, order);
        fmpz_mod_poly_swap(A->coeffs + i, t);
    }

    fmpz_mod_poly_clear(t);
    fmpz_mod_poly_clear(lcinv);
}

void fmpz_mod_bpoly_mul(fmpz_mod_bpoly_t A, const fmpz_mod_bpoly_t B, const fmpz_mod_bpoly_t C, slong order)
{
    slong i, j;
    fmpz_mod_poly_t t;

    FLINT_ASSERT(A != B);
    FLINT_ASSERT(A != C);

    fmpz_mod_poly_init(t, B->modulus);

    fmpz_mod_bpoly_fit_length(A, B->length + C->length - 1);
    for (i = 0; i < B->length + C->length - 1; i++)
        fmpz_mod_poly_zero(A->coeffs + i);

    for (i = 0; i < B->length; i++)
    {
        for (j = 0; j < C->length; j++)
        {
            fmpz_mod_poly_mullow(t, B->coeffs + i, C->coeffs + j, order);
            fmpz_mod_poly_add(A->coeffs + i + j, A->coeffs + i + j, t);
        }
    }

    A->length = B->length + C->length - 1;

    fmpz_mod_poly_clear(t);
}

void fmpz_mod_bpoly_add_poly_shift(fmpz_mod_bpoly_t A, const fmpz_mod_poly_t B, slong yshift)
{
    slong i;
    fmpz_t c;

    FLINT_ASSERT(A->length > B->length);

    fmpz_init(c);

    for (i = 0; i < B->length; i++)
    {
        fmpz_mod_poly_get_coeff_fmpz(c, A->coeffs + i, yshift);
        FLINT_ASSERT(fmpz_is_zero(c));
        fmpz_mod_poly_set_coeff_fmpz(A->coeffs + i, yshift, B->coeffs + i);
    }

    fmpz_clear(c);
}

void fmpz_mod_bpoly_sub(fmpz_mod_bpoly_t A, const fmpz_mod_bpoly_t B, const fmpz_mod_bpoly_t C)
{
    slong i;

    FLINT_ASSERT(A != B);
    FLINT_ASSERT(A != C);

    fmpz_mod_bpoly_fit_length(A, FLINT_MAX(B->length, C->length));

    for (i = 0; i < FLINT_MAX(B->length, C->length) - 1; i++)
    {
        if (i < B->length)
        {
            if (i < C->length)
            {
                fmpz_mod_poly_sub(A->coeffs + i, B->coeffs + i, C->coeffs + i);
            }
            else
            {
                fmpz_mod_poly_set(A->coeffs + i, B->coeffs + i);
            }
        }
        else
        {
            FLINT_ASSERT(i < C->length);
            fmpz_mod_poly_neg(A->coeffs + i, C->coeffs + i);
        }

        if (!fmpz_mod_poly_is_zero(A->coeffs + i))
            A->length = i + 1;
    }
}

void fmpz_bpoly_init(fmpz_bpoly_t A)
{
    A->coeffs = NULL;
    A->alloc = 0;
    A->length = 0;
}

void fmpz_bpoly_clear(fmpz_bpoly_t A)
{
    if (A->coeffs)
    {
        slong i;
        for (i = 0; i < A->alloc; i++)
            fmpz_poly_clear(A->coeffs + i);
        flint_free(A->coeffs);
    }
}

void fmpz_bpoly_print(fmpz_bpoly_t A, const char * xvar, const char * yvar)
{
    slong i;
    int first;

    first = 1;
    for (i = A->length - 1; i >= 0; i--)
    {
        if (fmpz_poly_is_zero(A->coeffs + i))
            continue;

        if (!first)
            flint_printf(" + ");

        first = 0;

        flint_printf("(");
        fmpz_poly_print_pretty(A->coeffs + i, yvar);
        flint_printf(")*%s^%wd", xvar, i);
    }

    if (first)
        flint_printf("0");
}

void fmpz_bpoly_fit_length(fmpz_bpoly_t A, slong len)
{
    slong i;

    if (len <= A->alloc)
        return;

    if (len < 2 * A->alloc)
        len = 2 * A->alloc;

    if (A->alloc == 0)
        A->coeffs = (fmpz_poly_struct *) flint_malloc(len * sizeof(fmpz_poly_struct));
    else
        A->coeffs = (fmpz_poly_struct *) flint_realloc(A->coeffs, len * sizeof(fmpz_poly_struct));

    for (i = A->alloc; i < len; i++)
        fmpz_poly_init(A->coeffs + i);

    A->alloc = len;
}

void fmpz_bpoly_set(fmpz_bpoly_t A, const fmpz_bpoly_t B)
{
    slong i;

    FLINT_ASSERT(A != B);

    fmpz_bpoly_fit_length(A, B->length);
    A->length = B->length;

    for (i = 0; i < B->length; i++)
        fmpz_poly_set(A->coeffs + i, B->coeffs + i);
}

void fmpz_bpoly_make_primitive(fmpz_bpoly_t A)
{
    slong i;
    fmpz_poly_t g, q;

    fmpz_poly_init(g);
    fmpz_poly_init(q);

    for (i = 0; i < A->length; i++)
    {
        fmpz_poly_gcd(q, g, A->coeffs + i);
        fmpz_poly_swap(g, q);
    }

    for (i = 0; i < A->length; i++)
    {
        fmpz_poly_div(q, A->coeffs + i, g);
        fmpz_poly_swap(A->coeffs + i, q);
    }

    fmpz_poly_clear(g);
    fmpz_poly_clear(q);
}

void fmpz_bpoly_set_coeff(fmpz_bpoly_t A, slong xi, slong yi, const fmpz_t c)
{
    slong i;

    FLINT_ASSERT(!fmpz_is_zero(c));

    if (xi >= A->length)
    {
        fmpz_bpoly_fit_length(A, xi + 1);
        for (i = A->length; i <= xi; i++)
            fmpz_poly_zero(A->coeffs + i);
        A->length = xi + 1;
    }

    fmpz_poly_set_coeff_fmpz(A->coeffs + xi, yi, c);
}

void fmpz_mod_bpoly_set_coeff(fmpz_mod_bpoly_t A, slong xi, slong yi, const fmpz_t c)
{
    slong i;

    FLINT_ASSERT(!fmpz_is_zero(c));

    if (xi >= A->length)
    {
        fmpz_mod_bpoly_fit_length(A, xi + 1);
        for (i = A->length; i <= xi; i++)
            fmpz_mod_poly_zero(A->coeffs + i);
        A->length = xi + 1;
    }

    fmpz_mod_poly_set_coeff_fmpz(A->coeffs + xi, yi, c);
}


void fmpz_bpoly_zero(fmpz_bpoly_t A)
{
    A->length = 0;
}

void fmpz_mod_bpoly_zero(fmpz_mod_bpoly_t A)
{
    A->length = 0;
}

int fmpz_bpoly_divides(fmpz_bpoly_t Q, fmpz_bpoly_t A, fmpz_bpoly_t B)
{
    slong i, qoff;
    int divides;
    fmpz_poly_t q, t;
    fmpz_bpoly_t R;

    FLINT_ASSERT(Q != A);
    FLINT_ASSERT(Q != B);
    FLINT_ASSERT(B->length > 0);

    fmpz_poly_init(q);
    fmpz_poly_init(t);
    fmpz_bpoly_init(R);
    fmpz_bpoly_set(R, A);

    Q->length = 0;

    while (R->length >= B->length)
    {
        divides = fmpz_poly_divides(q, R->coeffs + R->length - 1, B->coeffs + B->length - 1);
        if (!divides)
            goto cleanup;

        for (i = 0; i < B->length; i++)
        {
            fmpz_poly_mul(t, B->coeffs + i, q);
            fmpz_poly_sub(R->coeffs + i + R->length - B->length, R->coeffs + i + R->length - B->length, t);
        }

        qoff = R->length - B->length;

        FLINT_ASSERT(qoff >= 0);
        if (qoff >= Q->length)
        {
            fmpz_bpoly_fit_length(Q, qoff + 1);
            for (i = Q->length; i <= qoff; i++)
                fmpz_poly_zero(Q->coeffs + i);
            Q->length = qoff + 1;
        }

        fmpz_poly_set(Q->coeffs + qoff, q);

        while (R->length > 0 && fmpz_poly_is_zero(R->coeffs + R->length - 1))
            R->length--;
    }

    divides = (R->length == 0);

cleanup:

    fmpz_poly_clear(q);
    fmpz_poly_clear(t);
    fmpz_bpoly_clear(R);

    return divides;
}


void fmpz_mpoly_from_fmpz_bpoly(
    fmpz_mpoly_t A,
    flint_bitcnt_t Abits,
    const fmpz_bpoly_t B,
    slong varx,
    slong vary,
    const fmpz_mpoly_ctx_t ctx)
{
    slong n = ctx->minfo->nvars;
    slong i, j;
    slong NA;
    slong Alen;
    fmpz * Acoeff;
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
    fmpz_mpoly_fit_bits(A, Abits, ctx);
    A->bits = Abits;

    Acoeff = A->coeffs;
    Aexp = A->exps;
    Aalloc = A->alloc;
    Alen = 0;
    for (i = 0; i < B->length; i++)
    {
        fmpz_poly_struct * Bc = B->coeffs + i;
        _fmpz_mpoly_fit_length(&Acoeff, &Aexp, &Aalloc, Alen + Bc->length, NA);

        for (j = 0; j < Bc->length; j++)
        {
            if (fmpz_is_zero(Bc->coeffs + j))
                continue;
            Aexps[varx] = i;
            Aexps[vary] = j;
            fmpz_set(Acoeff + Alen, Bc->coeffs + j);
            mpoly_set_monomial_ui(Aexp + NA*Alen, Aexps, Abits, ctx->minfo);
            Alen++;
        }
    }
    A->coeffs = Acoeff;
    A->exps = Aexp;
    A->alloc = Aalloc;
    _fmpz_mpoly_set_length(A, Alen, ctx);

    fmpz_mpoly_sort_terms(A, ctx);
    TMP_END;
}

void fmpz_bpoly_set_fmpz_mod_bpoly(fmpz_bpoly_t A, const fmpz_mod_bpoly_t B)
{
    slong i;

    fmpz_bpoly_fit_length(A, B->length);
    A->length = B->length;

    for (i = 0; i < B->length; i++)
    {
        fmpz_poly_fit_length(A->coeffs + i, (B->coeffs + i)->length);
        (A->coeffs + i)->length = (B->coeffs + i)->length;
        _fmpz_vec_scalar_smod_fmpz((A->coeffs + i)->coeffs, (B->coeffs + i)->coeffs, (B->coeffs + i)->length, B->modulus);
    }
}

void fmpz_mpoly_to_bpoly(fmpz_bpoly_t A, const fmpz_mpoly_t B, slong varx, slong vary, const fmpz_mpoly_ctx_t ctx)
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

    fmpz_bpoly_zero(A);

    for (j = 0; j < B->length; j++)
    {
        Bexpx = ((B->exps + NB*j)[Boffx] >> Bshiftx) & mask;
        Bexpy = ((B->exps + NB*j)[Boffy] >> Bshifty) & mask;
        fmpz_bpoly_set_coeff(A, Bexpx, Bexpy, B->coeffs + j);
    }
}

void fmpz_bpoly_eval(fmpz_poly_t E, const fmpz_bpoly_t A, const fmpz_t alpha)
{
    slong i;
    fmpz_t t;

    fmpz_init(t);

    fmpz_poly_zero(E);
    for (i = A->length - 1; i >= 0; i--)
    {
        fmpz_poly_evaluate_fmpz(t, A->coeffs + i, alpha);
        fmpz_poly_set_coeff_fmpz(E, i, t);
    }

    fmpz_clear(t);
}

void fmpz_bpoly_taylor_shift(fmpz_bpoly_t A, const fmpz_t alpha)
{
    slong i;
    for (i = A->length - 1; i >= 0; i--)
        _fmpz_poly_taylor_shift((A->coeffs + i)->coeffs, alpha, (A->coeffs + i)->length);    
}

typedef struct {
    slong r; /* number of local factors */
    ulong k;
    slong lifting_prec;
    fmpz_t p;
    fmpz_t pk;
    fmpz_mod_bpoly_t Btilde;                /* mod p^k */
    fmpz_mod_bpoly_struct * newBitilde;     /* mod p^k */
    fmpz_mod_poly_struct * P;               /* mod p^k */
    fmpz_mod_poly_struct * d;               /* mod p^k */
    fmpz_mod_poly_struct * Bitilde;         /* mod p^k */
    fmpz_mod_poly_struct * d1;              /* mod p */
    fmpz_mod_poly_struct * Bitilde1;        /* mod p */
} bpoly_info_struct;

typedef bpoly_info_struct bpoly_info_t[1];

void bpoly_info_init(bpoly_info_t I, slong r, const fmpz_t p, ulong k)
{
    slong i;

    FLINT_ASSERT(r >= 2);

    I->r = r;

    I->lifting_prec = 0;

    I->k = k;
    fmpz_init_set(I->p, p);
    fmpz_init(I->pk);
    fmpz_pow_ui(I->pk, p, k);

    fmpz_mod_bpoly_init(I->Btilde, I->pk);

    I->newBitilde = (fmpz_mod_bpoly_struct *) flint_malloc(I->r*sizeof(fmpz_mod_bpoly_struct));
    I->P          = (fmpz_mod_poly_struct *) flint_malloc(I->r*sizeof(fmpz_mod_poly_struct));
    I->d          = (fmpz_mod_poly_struct *) flint_malloc(I->r*sizeof(fmpz_mod_poly_struct));
    I->Bitilde    = (fmpz_mod_poly_struct *) flint_malloc(I->r*sizeof(fmpz_mod_poly_struct));
    I->d1         = (fmpz_mod_poly_struct *) flint_malloc(I->r*sizeof(fmpz_mod_poly_struct));
    I->Bitilde1   = (fmpz_mod_poly_struct *) flint_malloc(I->r*sizeof(fmpz_mod_poly_struct));

    for (i = 0; i < I->r; i++)
    {
        fmpz_mod_bpoly_init(I->newBitilde + i, I->pk);
        fmpz_mod_poly_init(I->P + i, I->pk);
        fmpz_mod_poly_init(I->d + i, I->pk);
        fmpz_mod_poly_init(I->Bitilde + i, I->pk);
        fmpz_mod_poly_init(I->d1 + i, I->p);
        fmpz_mod_poly_init(I->Bitilde1 + i, I->p);
    }
}

void bpoly_info_clear(bpoly_info_t I)
{
    slong i;

    fmpz_clear(I->p);
    fmpz_clear(I->pk);

    fmpz_mod_bpoly_clear(I->Btilde);

    for (i = 0; i < I->r; i++)
    {
        fmpz_mod_bpoly_clear(I->newBitilde + i);
        fmpz_mod_poly_clear(I->P + i);
        fmpz_mod_poly_clear(I->d + i);
        fmpz_mod_poly_clear(I->Bitilde + i);
        fmpz_mod_poly_clear(I->d1 + i);
        fmpz_mod_poly_clear(I->Bitilde1 + i);
    }

    flint_free(I->newBitilde);
    flint_free(I->P);
    flint_free(I->d);
    flint_free(I->Bitilde);
    flint_free(I->d1);
    flint_free(I->Bitilde1);
}

/*
    set out[i] so that
    1/(f[0]*f[1]*...*f[n-1]) = out[0]/f[0] + ... + out[n-1]/f[n-1]
*/
int partial_fraction_coeffs(fmpz_mod_poly_struct * out, const fmpz_mod_poly_struct * f, const fmpz_t p, slong n)
{
    slong i;
    fmpz_mod_poly_t num, den, a, b, g, t;

    FLINT_ASSERT(n > 1);

    fmpz_mod_poly_init(num, p);
    fmpz_mod_poly_init(den, p);
    fmpz_mod_poly_init(a, p);
    fmpz_mod_poly_init(b, p);
    fmpz_mod_poly_init(g, p);
    fmpz_mod_poly_init(t, p);

    fmpz_mod_poly_set_ui(num, 1);
    fmpz_mod_poly_mul(den, f + 0, f + 1);
    for (i = 2; i < n; i++)
        fmpz_mod_poly_mul(den, den, f + i);

    for (i = 0; i < n; i++)
    {
        fmpz_mod_poly_divrem(den, t, den, f + i);
        FLINT_ASSERT(fmpz_mod_poly_is_zero(t));
        fmpz_mod_poly_xgcd(g, a, b, f + i, den);
        if (fmpz_mod_poly_degree(g) != 0)
            return 0;
        FLINT_ASSERT(fmpz_is_one(g->coeffs + 0));
        fmpz_mod_poly_mul(t, b, num);
        fmpz_mod_poly_rem(out + i, t, f + i);
        fmpz_mod_poly_mul(t, a, num);
        fmpz_mod_poly_rem(num, t, den);
    }

    fmpz_mod_poly_clear(num);
    fmpz_mod_poly_clear(den);
    fmpz_mod_poly_clear(a);
    fmpz_mod_poly_clear(b);
    fmpz_mod_poly_clear(g);
    fmpz_mod_poly_clear(t);
    return 1;
}


int bpoly_info_disolve(bpoly_info_t I)
{
    slong i, j;
    fmpz_t pj, t1;
    fmpz_mod_poly_t error, t, s, s1, s2;

    if (!partial_fraction_coeffs(I->d1, I->Bitilde1, I->p, I->r))
        return 0;

    fmpz_init(pj);
    fmpz_init(t1);
    fmpz_mod_poly_init(error, I->pk);
    fmpz_mod_poly_init(t, I->pk);
    fmpz_mod_poly_init(s, I->p);
    fmpz_mod_poly_init(s1, I->p);
    fmpz_mod_poly_init(s2, I->pk);

    for (i = 0; i < I->r; i++)
    {
        fmpz_mod_poly_set_ui(I->P + i, 1);
        for (j = 0; j < I->r; j++)
        {
            if (i == j)
                continue;
            fmpz_mod_poly_mul(I->P + i, I->P + i, I->Bitilde + j);
        }
    }

    fmpz_mod_poly_set_ui(error, 1);
    for (i = 0; i < I->r; i++)
    {
        fmpz_mod_poly_set(I->d + i, I->d1 + i); /* slight abuse because moduli are different */
        fmpz_mod_poly_mul(t, I->d + i, I->P + i);
        fmpz_mod_poly_sub(error, error, t);
    }

    fmpz_one(pj);
    for (j = 1; j < I->k; j++)
    {
        fmpz_mul(pj, pj, I->p);
        fmpz_mod_poly_zero(s);
        for (i = error->length - 1; i >= 0; i--)
        {
            FLINT_ASSERT(fmpz_divisible(error->coeffs + i, pj));
            fmpz_divexact(t1, error->coeffs + i, pj);
            fmpz_mod(t1, t1, I->p);
            fmpz_mod_poly_set_coeff_fmpz(s, i, t1);
        }

        for (i = 0; i < I->r; i++)
        {
            fmpz_mod_poly_mul(s1, s, I->d1 + i);
            fmpz_set(&s2->p, I->p);
            fmpz_mod_poly_rem(s2, s1, I->Bitilde1 + i);
            fmpz_set(&s2->p, I->pk);
            fmpz_mod_poly_scalar_mul_fmpz(s2, s2, pj);
            fmpz_mod_poly_add(I->d + i, I->d + i, s2);
        }

        fmpz_mod_poly_set_ui(error, 1);
        for (i = 0; i < I->r; i++)
        {
            fmpz_mod_poly_mul(t, I->d + i, I->P + i);
            fmpz_mod_poly_sub(error, error, t);
        }
    }

    FLINT_ASSERT(fmpz_mod_poly_is_zero(error));

    fmpz_clear(pj);
    fmpz_clear(t1);
    fmpz_mod_poly_clear(error);
    fmpz_mod_poly_clear(s);
    fmpz_mod_poly_clear(s1);
    fmpz_mod_poly_clear(s2);

    return 1;
}



static void _bivar_lift_quintic(bpoly_info_t I)
{
    slong i, j, k;
    fmpz_mod_bpoly_t tp, tp1, error;
    fmpz_mod_poly_t ss, tt;
timeit_t timer;

timeit_start(timer);

    fmpz_mod_poly_init(ss, I->pk);
    fmpz_mod_poly_init(tt, I->pk);
    fmpz_mod_bpoly_init(tp, I->pk);
    fmpz_mod_bpoly_init(tp1, I->pk);
    fmpz_mod_bpoly_init(error, I->pk);

    fmpz_mod_bpoly_mul(tp, I->newBitilde + 0, I->newBitilde + 1, I->lifting_prec);
    for (i = 2; i < I->r; i++)
    {
        fmpz_mod_bpoly_mul(tp1, tp, I->newBitilde + i, I->lifting_prec);
        fmpz_mod_bpoly_swap(tp1, tp);
    }
    fmpz_mod_bpoly_sub(error, I->Btilde, tp);

    for (j = 1; j < I->lifting_prec; j++)
    {
        fmpz_mod_poly_zero(ss);
        for (i = error->length - 1; i >= 0; i--)
        {
            fmpz_t ct;
            fmpz_init(ct);

            fmpz_mod_bpoly_get_coeff(ct, error, i, j);
            fmpz_mod_poly_set_coeff_fmpz(ss, i, ct);
            for (k = 0; k < j; k++)
            {
                fmpz_mod_bpoly_get_coeff(ct, error, i, k);
                FLINT_ASSERT(fmpz_is_zero(ct));
            }

            fmpz_clear(ct);
        }

        for (i = 0; i < I->r; i++)
        {
            fmpz_mod_poly_mul(tt, ss, I->d + i);
            fmpz_mod_poly_rem(tt, tt, I->Bitilde + i);
            fmpz_mod_bpoly_add_poly_shift(I->newBitilde + i, tt, j);
        }

        fmpz_mod_bpoly_mul(tp, I->newBitilde + 0, I->newBitilde + 1, I->lifting_prec);
        for (i = 2; i < I->r; i++)
        {
            fmpz_mod_bpoly_mul(tp1, tp, I->newBitilde + i, I->lifting_prec);
            fmpz_mod_bpoly_swap(tp1, tp);
        }
        fmpz_mod_bpoly_sub(error, I->Btilde, tp);
    }

flint_printf("------------------\n");
for (k = 0; k < I->r; k++)
{
flint_printf("newBitilde[%wd]: ", k); fmpz_mod_bpoly_print(I->newBitilde + k, "x", "y"); printf("\n");
}

timeit_stop(timer);
flint_printf("_bivar_lift_quintic time: %wd\n", timer->wall);

    fmpz_mod_poly_clear(ss);
    fmpz_mod_poly_clear(tt);
    fmpz_mod_bpoly_clear(tp);
    fmpz_mod_bpoly_clear(tp1);
    fmpz_mod_bpoly_clear(error);
}

void fmpz_mod_bpoly_reverse_vars(
    fmpz_mod_bpoly_t A,
    const fmpz_mod_bpoly_t B)
{
    slong i, j;
    fmpz_mod_bpoly_zero(A);
    for (i = 0; i < B->length; i++)
    {
        fmpz_mod_poly_struct * Bi = B->coeffs + i;
        for (j = 0; j < Bi->length; j++)
        {
            if (!fmpz_is_zero(Bi->coeffs + j))
            {
                fmpz_mod_bpoly_set_coeff(A, j, i, Bi->coeffs + j);
            }
        }
    }
}

static void _bivar_lift_quartic2(bpoly_info_t I)
{
    slong i, j, k;
    fmpz_mod_poly_t t, t1;
    fmpz_mod_bpoly_t btilde;
    fmpz_mod_bpoly_struct newbitilde[2];
/*
timeit_t timer;
timeit_start(timer);
*/
    FLINT_ASSERT(I->r == 2);

    fmpz_mod_poly_init(t, I->p);
    fmpz_mod_poly_init(t1, I->p);
    fmpz_mod_bpoly_init(btilde, I->p);
    fmpz_mod_bpoly_reverse_vars(btilde, I->Btilde);
    for (k = 0; k < I->r; k++)
    {
        fmpz_mod_bpoly_init(newbitilde + k, I->p);
        fmpz_mod_bpoly_reverse_vars(newbitilde + k, I->newBitilde + k);
        fmpz_mod_bpoly_fit_length(newbitilde + k, I->lifting_prec);
        FLINT_ASSERT((newbitilde + k)->length == 1);
    }

    for (j = 1; j < I->lifting_prec; j++)
    {
        if (j < btilde->length)
            fmpz_mod_poly_set(t, btilde->coeffs + j);
        else
            fmpz_mod_poly_zero(t);

        for (i = 1; i < j; i++)
        {
            fmpz_mod_poly_mul(t1, newbitilde[0].coeffs + i, newbitilde[1].coeffs + j - i);
            fmpz_mod_poly_sub(t, t, t1);
        }

        for (k = 0; k < I->r; k++)
        {
            fmpz_mod_poly_mul(t1, t, I->d + k);
            fmpz_mod_poly_rem(newbitilde[k].coeffs + j, t1, I->Bitilde + k);
            if (!fmpz_mod_poly_is_zero(newbitilde[k].coeffs + j))
                newbitilde[k].length = j + 1;
        }
    }

    for (k = 0; k < I->r; k++)
        fmpz_mod_bpoly_reverse_vars(I->newBitilde + k, newbitilde + k);
/*
flint_printf("------------------\n");
for (k = 0; k < I->r; k++)
{
flint_printf("newBitilde[%wd]: ", k); fmpz_mod_bpoly_print(I->newBitilde + k, "x", "y"); printf("\n");
}
timeit_stop(timer);
flint_printf("_bivar_lift_quartic2 time: %wd\n", timer->wall);
*/

    fmpz_mod_poly_clear(t);
    fmpz_mod_poly_clear(t1);
    fmpz_mod_bpoly_clear(btilde);
    for (k = 0; k < I->r; k++)
    {
        fmpz_mod_bpoly_clear(newbitilde + k);
    }
}

static void _bivar_lift_quartic(bpoly_info_t I)
{
    slong i, j, k;
    fmpz_mod_poly_t t, t1;
    fmpz_mod_bpoly_t btilde;
    fmpz_mod_bpoly_struct * newbitilde, * U;

    FLINT_ASSERT(I->r > 2);

    newbitilde = (fmpz_mod_bpoly_struct *) flint_malloc(I->r*sizeof(fmpz_mod_bpoly_struct));
    U = (fmpz_mod_bpoly_struct *) flint_malloc(I->r*sizeof(fmpz_mod_bpoly_struct));

    fmpz_mod_poly_init(t, I->pk);
    fmpz_mod_poly_init(t1, I->pk);
    fmpz_mod_bpoly_init(btilde, I->pk);
    fmpz_mod_bpoly_reverse_vars(btilde, I->Btilde);
    for (k = 0; k < I->r; k++)
    {
        fmpz_mod_bpoly_init(U + k, I->pk);
        fmpz_mod_bpoly_fit_length(U + k, I->lifting_prec);
        for (i = 0; i < I->lifting_prec; i++)
        {
            fmpz_mod_poly_zero(U[k].coeffs + i);
        }

        fmpz_mod_bpoly_init(newbitilde + k, I->pk);
        fmpz_mod_bpoly_reverse_vars(newbitilde + k, I->newBitilde + k);
        fmpz_mod_bpoly_fit_length(newbitilde + k, I->lifting_prec);
        FLINT_ASSERT(newbitilde[k].length == 1);
        for (i = 1; i < I->lifting_prec; i++)
        {
            fmpz_mod_poly_zero(newbitilde[k].coeffs + i);
        }
    }

    k = I->r - 2;
    fmpz_mod_poly_mul(U[k].coeffs + 0, newbitilde[k].coeffs + 0, newbitilde[k + 1].coeffs + 0);
    for (k--; k >= 1; k--)
        fmpz_mod_poly_mul(U[k].coeffs + 0, newbitilde[k].coeffs + 0, U[k + 1].coeffs + 0);

    for (j = 1; j < I->lifting_prec; j++)
    {
        k = I->r - 2;
        fmpz_mod_poly_zero(U[k].coeffs + j);
        for (i = 0; i <= j; i++)
        {
            fmpz_mod_poly_mul(t1, newbitilde[k].coeffs + i, newbitilde[k + 1].coeffs + j - i);
            fmpz_mod_poly_add(U[k].coeffs + j, U[k].coeffs + j, t1);
        }
        for (k--; k >= 1; k--)
        {
            fmpz_mod_poly_zero(U[k].coeffs + j);
            for (i = 0; i <= j; i++)
            {
                fmpz_mod_poly_mul(t1, newbitilde[k].coeffs + i, U[k + 1].coeffs + j - i);
                fmpz_mod_poly_add(U[k].coeffs + j, U[k].coeffs + j, t1);
            }
        }

        if (j < btilde->length)
            fmpz_mod_poly_set(t, btilde->coeffs + j);
        else
            fmpz_mod_poly_zero(t);

        for (i = 0; i <= j; i++)
        {
            fmpz_mod_poly_mul(t1, newbitilde[0].coeffs + i, U[1].coeffs + j - i);
            fmpz_mod_poly_sub(t, t, t1);
        }

        for (k = 0; k < I->r; k++)
        {
            fmpz_mod_poly_mul(t1, t, I->d + k);
            fmpz_mod_poly_rem(newbitilde[k].coeffs + j, t1, I->Bitilde + k);
            if (!fmpz_mod_poly_is_zero(newbitilde[k].coeffs + j))
                newbitilde[k].length = j + 1;
        }

        k = I->r - 2;
        fmpz_mod_poly_mul(t, newbitilde[k].coeffs + 0, newbitilde[k + 1].coeffs + j);
        fmpz_mod_poly_mul(t1, newbitilde[k].coeffs + j, newbitilde[k + 1].coeffs + 0);
        fmpz_mod_poly_add(t, t, t1);
        fmpz_mod_poly_add(U[k].coeffs + j, U[k].coeffs + j, t);
        for (k--; k >= 1; k--)
        {
            fmpz_mod_poly_mul(t1, newbitilde[k].coeffs + 0, t);
            fmpz_mod_poly_swap(t, t1);
            fmpz_mod_poly_mul(t1, newbitilde[k].coeffs + j, U[k + 1].coeffs + 0);
            fmpz_mod_poly_add(t, t, t1);
            fmpz_mod_poly_add(U[k].coeffs + j, U[k].coeffs + j, t);
        }
    }

    for (k = 0; k < I->r; k++)
        fmpz_mod_bpoly_reverse_vars(I->newBitilde + k, newbitilde + k);

    fmpz_mod_poly_clear(t);
    fmpz_mod_poly_clear(t1);
    fmpz_mod_bpoly_clear(btilde);
    for (k = 0; k < I->r; k++)
    {
        fmpz_mod_bpoly_clear(U + k);
        fmpz_mod_bpoly_clear(newbitilde + k);
    }

    flint_free(newbitilde);
}


static void _recombine_naive(
    fmpz_mpoly_factor_t fac,
    flint_bitcnt_t bits,
    slong xvar,
    slong yvar,
    const fmpz_mpoly_ctx_t ctx,
    fmpz_bpoly_t B,
    fmpz_t alpha,
    bpoly_info_t I)
{
    fmpz_bpoly_t Q, R, trymez;
    fmpz_mod_bpoly_t tryme, trymet;
    fmpz_mod_poly_t leadB;
    slong i, j, r, len;
    fmpz_t subset, tsubset, test;
    fmpz_mpoly_t goodtry;
    slong * idx;

    fmpz_init(test);
    fmpz_init(tsubset);
    fmpz_init(subset);
    fmpz_mpoly_init(goodtry, ctx);

    fmpz_bpoly_init(Q);
    fmpz_bpoly_init(R);
    fmpz_bpoly_init(trymez);
    fmpz_mod_bpoly_init(tryme, I->pk);
    fmpz_mod_bpoly_init(trymet, I->pk);
    fmpz_mod_poly_init(leadB, I->pk);

    FLINT_ASSERT(B->length > 0);
    fmpz_mod_poly_set_fmpz_poly(leadB, B->coeffs + B->length - 1);

    len = I->r;
    idx = (slong *) flint_malloc(I->r * sizeof(slong));
    for (i = 0; i < len; i++)
        idx[i] = i;

    for (r = 1; r <= len/2; r++)
    {
        subset_first(subset, len, r);
        do {
try_subset:
            fmpz_mod_bpoly_set_polyy(tryme, leadB);

            for (i = 0; i < len; i++)
            {
                if (fmpz_tstbit(subset, i))
                {
                    fmpz_mod_bpoly_mul(trymet, tryme, I->newBitilde + idx[i], I->lifting_prec);
                    fmpz_mod_bpoly_swap(trymet, tryme);
                }
            }
            fmpz_bpoly_set_fmpz_mod_bpoly(trymez, tryme);
            fmpz_bpoly_make_primitive(trymez);

            if (fmpz_bpoly_divides(Q, B, trymez))
            {
                fmpz_neg(alpha, alpha);
                fmpz_bpoly_taylor_shift(trymez, alpha);
                fmpz_neg(alpha, alpha);
                fmpz_mpoly_from_fmpz_bpoly(goodtry, bits, trymez, xvar, yvar, ctx);
                if (fmpz_sgn(goodtry->coeffs) < 0)
                    fmpz_mpoly_neg(goodtry, goodtry, ctx);
                fmpz_mpoly_factor_append_ui(fac, goodtry, 1, ctx);
                fmpz_bpoly_swap(B, Q);
                FLINT_ASSERT(B->length > 0);
                fmpz_mod_poly_set_fmpz_poly(leadB, B->coeffs + B->length - 1);

                /* fix indices */
                j = 0;
                for (i = 0; i < len; i++)
                {
                    if (!fmpz_tstbit(subset, i))
                        idx[j++] = idx[i];
                }
                len -= r;

                /* fix subsets */
                fmpz_set(tsubset, subset);
                do {
                    if (!subset_next(tsubset, tsubset, len))
                        goto rloop_continue;
                    fmpz_and(test, tsubset, subset);
                } while (!fmpz_is_zero(test));
                subset_map_down(test, tsubset, subset);
                fmpz_swap(test, subset);
                goto try_subset;
            }
        }
        while (subset_next(subset, subset, len));
rloop_continue:
        (void)(NULL);
    }

    FLINT_ASSERT(B->length > 1);
    fmpz_neg(alpha, alpha);
    fmpz_bpoly_taylor_shift(B, alpha);
    fmpz_neg(alpha, alpha);
    fmpz_mpoly_from_fmpz_bpoly(goodtry, bits, B, xvar, yvar, ctx);
    if (fmpz_sgn(goodtry->coeffs) < 0)
        fmpz_mpoly_neg(goodtry, goodtry, ctx);
    fmpz_mpoly_factor_append_ui(fac, goodtry, 1, ctx);

    fmpz_bpoly_clear(Q);
    fmpz_bpoly_clear(R);
    fmpz_bpoly_clear(trymez);
    fmpz_mod_bpoly_clear(tryme);
    fmpz_mod_bpoly_clear(trymet);
    fmpz_mod_poly_clear(leadB);

    fmpz_mpoly_clear(goodtry, ctx);
    fmpz_clear(subset);
    fmpz_clear(tsubset);
    fmpz_clear(test);
    flint_free(idx);
}

static int _irreducible_bivar_factors(
    fmpz_mpoly_factor_t fac,
    const fmpz_mpoly_t A,
    slong xvar,
    slong yvar,
    const fmpz_mpoly_ctx_t ctx)
{
    int success;
    slong i;
    fmpz_t alpha;
    fmpz_poly_t Beval;
    fmpz_bpoly_t B;
    fmpz_poly_factor_t Bevalfac;
    slong Blengthx, Blengthy;
    flint_bitcnt_t Bbits;
    ulong pkbits;
    ulong k;
    fmpz_t p;
    bpoly_info_t I;

    fmpz_mpoly_factor_one(fac, ctx);

    k = 1;
    fmpz_init_set_ui(p, UWORD(1) << (FLINT_BITS - 1));
    fmpz_init(alpha);
    fmpz_poly_init(Beval);
    fmpz_poly_factor_init(Bevalfac);
    fmpz_bpoly_init(B);
    bpoly_info_init(I, 2, p, k);

    fmpz_mpoly_to_bpoly(B, A, xvar, yvar, ctx);

    Blengthx = B->length;
    FLINT_ASSERT(Blengthx > 1);

    fmpz_zero(alpha);
    goto got_alpha;

next_alpha:

    fmpz_neg(alpha, alpha);
    fmpz_add_ui(alpha, alpha, fmpz_sgn(alpha) >= 0);

got_alpha:

    fmpz_bpoly_eval(Beval, B, alpha);

    /* if killed leading coeff, get new alpha */
    if (Beval->length != Blengthx)
        goto next_alpha;

    fmpz_one(&Bevalfac->c);
    Bevalfac->num = 0;
    fmpz_poly_factor(Bevalfac, Beval);

    /* if multiple factors, get new alpha */
    for (i = 0; i < Bevalfac->num; i++)
    {
        if (Bevalfac->exp[i] != 1)
            goto next_alpha;
    }

    /* if one factor, A is irreducible */
    if (Bevalfac->num == 1)
    {
        fmpz_mpoly_factor_append_ui(fac, A, 1, ctx);
        success = 1;
        goto cleanup;
    }

    fmpz_bpoly_taylor_shift(B, alpha);

    Blengthy = 0;
    Bbits = 0;
    for (i = 0; i < B->length; i++)
    {
        slong this_bits;
        Blengthy = FLINT_MAX(Blengthy, (B->coeffs + i)->length);
        this_bits = _fmpz_vec_max_bits((B->coeffs + i)->coeffs,
                                       (B->coeffs + i)->length);
        Bbits = FLINT_MAX(Bbits, FLINT_ABS(this_bits));
    }

    pkbits = (FLINT_BIT_COUNT(Blengthx*Blengthy) + 1)/2;
    pkbits += Blengthx + Blengthy + Bbits - 3;

next_prime:

    fmpz_nextprime(p, p, 1);

    FLINT_ASSERT(B->length > 0);
    FLINT_ASSERT((B->coeffs + B->length - 1)->length > 0);
    FLINT_ASSERT(!fmpz_is_zero((B->coeffs + B->length - 1)->coeffs + 0));

    if (fmpz_divisible((B->coeffs + B->length - 1)->coeffs + 0, p))
        goto next_prime;

    k = (pkbits + fmpz_bits(p))/fmpz_bits(p);

    bpoly_info_clear(I);
    bpoly_info_init(I, Bevalfac->num, p, k);
    I->lifting_prec = Blengthy + (B->coeffs + B->length - 1)->length;

    fmpz_mod_bpoly_set_fmpz_bpoly(I->Btilde, B);
    fmpz_mod_bpoly_make_monic(I->Btilde, I->lifting_prec);
    for (i = 0; i < I->r; i++)
    {
        fmpz_mod_poly_set_fmpz_poly(I->Bitilde1 + i, Bevalfac->p + i);
        fmpz_mod_poly_make_monic(I->Bitilde1 + i, I->Bitilde1 + i);
        fmpz_mod_poly_set_fmpz_poly(I->Bitilde + i, Bevalfac->p + i);
        fmpz_mod_poly_make_monic(I->Bitilde + i, I->Bitilde + i);
        fmpz_mod_bpoly_set_polyx(I->newBitilde + i, I->Bitilde + i);
    }

    FLINT_ASSERT(I->r > 1);

    if (!bpoly_info_disolve(I))
        goto next_prime;

/*
flint_printf("I->pk: "); fmpz_print(I->pk); printf("\n");
    for (k = 0; k < I->r; k++)
    {
flint_printf("newBitilde[%wd]: ", k); fmpz_mod_bpoly_print(I->newBitilde + k, "x", "y"); printf("\n");
    }
*/
    if (I->r == 2)
        _bivar_lift_quartic2(I);
    else if (I->r < 20)
        _bivar_lift_quartic(I);
    else
        _bivar_lift_quintic(I);
/*
printf("---------- lifted factors ----------\n");
    for (k = 0; k < I->r; k++)
    {
flint_printf("newBitilde[%wd]: ", k); fmpz_mod_bpoly_print(I->newBitilde + k, "x", "y"); printf("\n");
    }
*/

    _recombine_naive(fac, A->bits, xvar, yvar, ctx, B, alpha, I);
/*
printf("bivarfac: "); fmpz_mpoly_factor_print_pretty(fac, NULL, ctx); printf("\n");
*/
    success = 1;

cleanup:

    bpoly_info_clear(I);
    fmpz_bpoly_clear(B);
    fmpz_poly_factor_clear(Bevalfac);
    fmpz_poly_clear(Beval);
    fmpz_clear(alpha);
    fmpz_clear(p);
    return success;
}





void fmpz_mpoly_convert_perm(
    fmpz_mpoly_t A,
    flint_bitcnt_t Abits,
    const fmpz_mpoly_ctx_t lctx,
    const fmpz_mpoly_t B,
    const fmpz_mpoly_ctx_t ctx,
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

    fmpz_mpoly_fit_bits(A, Abits, ctx);
    A->bits = Abits;
    fmpz_mpoly_fit_length(A, B->length, lctx);
    A->length = B->length;
    for (i = 0; i < B->length; i++)
    {        
        fmpz_set(A->coeffs + i, B->coeffs + i);
        mpoly_get_monomial_ui(Bexps, B->exps + NB*i, B->bits, ctx->minfo);
        for (k = 0; k < m; k++)
        {
            l = perm[k];
            Aexps[k] = l < 0 ? 0 : Bexps[l];
        }
        mpoly_set_monomial_ui(A->exps + NA*(i), Aexps, Abits, lctx->minfo);
     }  
    fmpz_mpoly_sort_terms(A, lctx);
    TMP_END;
}


static void _to_poly(fmpz_poly_t A, const fmpz_mpoly_t B, const fmpz_mpoly_ctx_t ctx)
{
    slong i;
    ulong * shift, * stride;

    if (B->length == 0)
    {
        fmpz_poly_zero(A);
        return;
    }

    shift = (ulong *) flint_malloc(ctx->minfo->nvars*sizeof(ulong));
    stride = (ulong *) flint_malloc(ctx->minfo->nvars*sizeof(ulong));
    for (i = 0; i < ctx->minfo->nvars; i++)
    {
        shift[i] = 0;
        stride[i] = 1;
    }

    _fmpz_mpoly_to_fmpz_poly_deflate(A, B, 0, shift, stride, ctx);

    flint_free(shift);
    flint_free(stride);
}

static void _to_polyq(fmpq_poly_t A, const fmpz_mpoly_t B, const fmpz_mpoly_ctx_t ctx)
{
    slong i;
    ulong mask;
    slong shift, off, N;
    slong Blen = B->length;
    fmpz * Bcoeff = B->coeffs;
    ulong * Bexp = B->exps;

    FLINT_ASSERT(B->bits <= FLINT_BITS);

    fmpq_poly_zero(A);

    N = mpoly_words_per_exp_sp(B->bits, ctx->minfo);
    mpoly_gen_offset_shift_sp(&off, &shift, 0, B->bits, ctx->minfo);

    mask = (-UWORD(1)) >> (FLINT_BITS - B->bits);
    for (i = 0; i < Blen; i++)
        fmpq_poly_set_coeff_fmpz(A, (Bexp[N*i + off] >> shift) & mask, Bcoeff + i);
}

/*
static void _from_poly(fmpz_mpoly_t A, flint_bitcnt_t Abits, const fmpz_poly_t B, const fmpz_mpoly_ctx_t ctx)
{
    slong i;
    ulong * shift, * stride;

    if (B->length == 0)
    {
        fmpz_mpoly_zero(A, ctx);
        return;
    }

    shift = (ulong *) flint_malloc(ctx->minfo->nvars*sizeof(ulong));
    stride = (ulong *) flint_malloc(ctx->minfo->nvars*sizeof(ulong));
    for (i = 0; i < ctx->minfo->nvars; i++)
    {
        shift[i] = 0;
        stride[i] = 1;
    }

    _fmpz_mpoly_from_fmpz_poly_inflate(A, Abits, B, 0, shift, stride, ctx);

    flint_free(shift);
    flint_free(stride);
}
*/

static int _from_polyq(fmpz_mpoly_t A, flint_bitcnt_t Abits, const fmpq_poly_t B, const fmpz_mpoly_ctx_t ctx)
{
    slong var = 0;
    slong N;
    slong k;
    slong Alen;
    fmpz * Acoeff;
    ulong * Aexp;
    slong Aalloc;
    ulong * strideexp;
    TMP_INIT;

    if (B->length == 0)
    {
        fmpz_mpoly_zero(A, ctx);
        return 1;
    }

    if (!fmpz_is_one(fmpq_poly_denref(B)))
    {
        return 0;
    }

    TMP_START;

    FLINT_ASSERT(Abits <= FLINT_BITS);

    N = mpoly_words_per_exp(Abits, ctx->minfo);
    strideexp = (ulong*) TMP_ALLOC(N*sizeof(ulong));
    mpoly_gen_monomial_sp(strideexp, var, Abits, ctx->minfo);

    fmpz_mpoly_fit_bits(A, Abits, ctx);
    A->bits = Abits;

    Acoeff = A->coeffs;
    Aexp = A->exps;
    Aalloc = A->alloc;
    Alen = 0;
    for (k = B->length - 1; k >= 0; k--)
    {
        _fmpz_mpoly_fit_length(&Acoeff, &Aexp, &Aalloc, Alen + 1, N);
        if (!fmpz_is_zero(B->coeffs + k))
        {
            fmpz_swap(Acoeff + Alen, B->coeffs + k);
            mpoly_monomial_mul_ui(Aexp + N*Alen, strideexp, N, k);
            Alen++;
        }
    }
    A->coeffs = Acoeff;
    A->exps = Aexp;
    A->alloc = Aalloc;
    _fmpz_mpoly_set_length(A, Alen, ctx);

    TMP_END;

    return 1;
}


void fmpz_mpoly_to_mpolyv(
    fmpz_mpolyv_t A,
    const fmpz_mpoly_t B,
    slong m,
    const fmpz_t alpha,
    const fmpz_mpoly_ctx_t ctx)
{
    fmpz_mpoly_t g, Q, T;

    fmpz_mpoly_init(g, ctx);
    fmpz_mpoly_init(Q, ctx);
    fmpz_mpoly_init(T, ctx);

    fmpz_mpoly_gen(g, m, ctx);
    fmpz_mpoly_sub_fmpz(g, g, alpha, ctx);

    fmpz_mpolyv_fit_length(A, 8, ctx);
    fmpz_mpoly_divrem(Q, A->coeffs + 0, B, g, ctx);
    A->length = 1;

    while (!fmpz_mpoly_is_zero(Q, ctx))
    {
        fmpz_mpolyv_fit_length(A, A->length + 1, ctx);
        fmpz_mpoly_divrem(T, A->coeffs + A->length, Q, g, ctx);
        fmpz_mpoly_swap(Q, T, ctx);
        A->length++;
    }

    while (A->length > 0 && fmpz_mpoly_is_zero(A->coeffs + A->length - 1, ctx))
        A->length--;

    fmpz_mpoly_clear(g, ctx);
    fmpz_mpoly_clear(Q, ctx);
    fmpz_mpoly_clear(T, ctx);
}

void fmpz_mpoly_from_mpolyv(
    fmpz_mpoly_t A,
    const fmpz_mpolyv_t B,
    slong m,
    const fmpz_t alpha,
    const fmpz_mpoly_ctx_t ctx)
{
    slong i;
    fmpz_mpoly_t g, T;

    fmpz_mpoly_init(g, ctx);
    fmpz_mpoly_init(T, ctx);

    fmpz_mpoly_gen(g, m, ctx);
    fmpz_mpoly_sub_fmpz(g, g, alpha, ctx);

    fmpz_mpoly_zero(A, ctx);
    for (i = B->length - 1; i >= 0; i--)
    {
        fmpz_mpoly_mul(T, A, g, ctx);
        fmpz_mpoly_add(A, T, B->coeffs + i, ctx);
    }

    fmpz_mpoly_clear(g, ctx);
    fmpz_mpoly_clear(T, ctx);
}


typedef struct {
    slong n;
    slong r;
    slong l;
    fmpq_poly_struct * inv_prod_dbetas;
    fmpq_poly_struct * dbetas;
    fmpz_mpoly_struct * prod_mbetas;
    fmpz_mpolyv_struct * prod_mbetas_coeffs;
    fmpz_mpoly_struct * mbetas;
    fmpz_mpoly_struct * deltas;

    fmpz_mpoly_struct * g;
    fmpz_mpoly_struct * q;
    fmpz_mpoly_struct * qt;
    fmpz_mpoly_struct * newt;
    fmpz_mpolyv_struct * delta_coeffs;

    fmpq_poly_t dtq, S, R;

} mfactor_disolve_struct;

typedef mfactor_disolve_struct mfactor_disolve_t[1];


static void _mfactor_disolve_init(
    mfactor_disolve_t I,
    slong l, slong r,
    const fmpz_mpoly_struct * betas,
    const fmpz * alpha,
    const fmpz_mpoly_ctx_t ctx)
{
    slong i, j, k;
    fmpz_poly_t p;
    fmpq_poly_t G, S, pq;
/*
flint_printf("_mfactor_disolve_init called(l = %wd, r = %wd)\n",l,r);
*/
    I->l = l;
    I->r = r;

    FLINT_ASSERT(l > 0);

    I->inv_prod_dbetas = (fmpq_poly_struct *) flint_malloc(l*sizeof(fmpq_poly_struct));
    I->dbetas = (fmpq_poly_struct *) flint_malloc(l*sizeof(fmpq_poly_struct));
    I->prod_mbetas = (fmpz_mpoly_struct *) flint_malloc((r + 1)*l*sizeof(fmpz_mpoly_struct));
    I->prod_mbetas_coeffs = (fmpz_mpolyv_struct *) flint_malloc((r + 1)*l*sizeof(fmpz_mpolyv_struct));
    I->mbetas = (fmpz_mpoly_struct *) flint_malloc((r + 1)*l*sizeof(fmpz_mpoly_struct));
    I->deltas = (fmpz_mpoly_struct *) flint_malloc((r + 1)*l*sizeof(fmpz_mpoly_struct));

    fmpq_poly_init(I->dtq);
    fmpq_poly_init(I->S);
    fmpq_poly_init(I->R);

    I->g = (fmpz_mpoly_struct *) flint_malloc((r + 1)*sizeof(fmpz_mpoly_struct));
    I->q = (fmpz_mpoly_struct *) flint_malloc((r + 1)*sizeof(fmpz_mpoly_struct));
    I->qt = (fmpz_mpoly_struct *) flint_malloc((r + 1)*sizeof(fmpz_mpoly_struct));
    I->newt = (fmpz_mpoly_struct *) flint_malloc((r + 1)*sizeof(fmpz_mpoly_struct));
    I->delta_coeffs = (fmpz_mpolyv_struct *) flint_malloc((r + 1)*l*sizeof(fmpz_mpolyv_struct));
    for (i = 0; i <= r; i++)
    {
        fmpz_mpoly_init(I->g + i, ctx);
        fmpz_mpoly_init(I->q + i, ctx);
        fmpz_mpoly_init(I->qt + i, ctx);
        fmpz_mpoly_init(I->newt + i, ctx);
        for (j = 0; j < l; j++)
            fmpz_mpolyv_init(I->delta_coeffs + i*I->l + j, ctx);
    }

    fmpz_poly_init(p);
    fmpq_poly_init(G);
    fmpq_poly_init(S);
    fmpq_poly_init(pq);

    /* initialize deltas */
    for (i = r; i >= 0; i--)
        for (j = 0; j < l; j++)
            fmpz_mpoly_init(I->deltas + i*l + j, ctx);

    /* set betas */
    i = r;
    for (j = 0; j < l; j++)
    {
        fmpz_mpoly_init(I->mbetas + i*l + j, ctx);
        fmpz_mpoly_set(I->mbetas + i*l + j, betas + j, ctx);
/*
flint_printf("mbetas[%wd][%wd]: ",i,j); fmpz_mpoly_print_pretty(I->mbetas + i*l + j, NULL, ctx); printf("\n");
*/
    }
    for (i--; i >= 0; i--)
    {
        for (j = 0; j < l; j++)
        {
            fmpz_mpoly_init(I->mbetas + i*l + j, ctx);
            fmpz_mpoly_evaluate_one_fmpz(I->mbetas + i*l + j, I->mbetas + (i + 1)*l + j, i + 1, alpha + i, ctx);
/*
flint_printf("mbetas[%wd][%wd]: ",i,j); fmpz_mpoly_print_pretty(I->mbetas + i*l + j, NULL, ctx); printf("\n");
*/
        }
    }
    for (j = 0; j < l; j++)
    {
        _to_poly(p, I->mbetas + 0*l + j, ctx);
        fmpq_poly_init(I->dbetas + j);
        fmpq_poly_set_fmpz_poly(I->dbetas + j, p);
/*
flint_printf("dbetas[%wd]: ",j); fmpq_poly_print_pretty(I->dbetas + j, "x1"); printf("\n");
*/
    }

    /* set product of betas */
    for (i = r; i >= 0; i--)
    {
        for (j = 0; j < l; j++)
        {
            fmpz_mpoly_init(I->prod_mbetas + i*l + j, ctx);
            fmpz_mpoly_one(I->prod_mbetas + i*l + j, ctx);
            for (k = 0; k < l; k++)
            {
                if (k == j)
                    continue;
                fmpz_mpoly_mul(I->prod_mbetas + i*l + j, I->prod_mbetas + i*l + j, I->mbetas + i*l + k, ctx);
            }
            fmpz_mpolyv_init(I->prod_mbetas_coeffs + i*l + j, ctx);
            if (i > 0)
            {
                fmpz_mpoly_to_mpolyv(I->prod_mbetas_coeffs + i*l + j,
                                     I->prod_mbetas + i*l + j, i, alpha + i - 1, ctx);
            }
/*
flint_printf("prod_mbetas[%wd][%wd]: ",i,j); fmpz_mpoly_print_pretty(I->prod_mbetas + i*l + j, NULL, ctx); printf("\n");
fmpz_mpolyv_print_pretty(I->prod_mbetas_coeffs + i*l + j, NULL, ctx);
*/
        }        
    }
    for (j = 0; j < l; j++)
    {
        fmpq_poly_one(pq);
        for (k = 0; k < l; k++)
        {
            if (k == j)
                continue;
            fmpq_poly_mul(pq, pq, I->dbetas + k);
        }
        fmpq_poly_init(I->inv_prod_dbetas + j);
        fmpq_poly_xgcd(G, S, I->inv_prod_dbetas + j, I->dbetas + j, pq);
        FLINT_ASSERT(fmpq_poly_is_one(G));
/*
flint_printf("inv_prod_dbetas[%wd]: ",j); fmpq_poly_print_pretty(I->inv_prod_dbetas + j, "x1"); printf("\n");
*/
    }

    fmpz_poly_clear(p);
    fmpq_poly_clear(G);
    fmpq_poly_clear(S);
    fmpq_poly_clear(pq);
/*
printf("_mfactor_disolve_init returning\n");
*/
}


static void _mfactor_disolve_clear(mfactor_disolve_t I, const fmpz_mpoly_ctx_t ctx)
{
    slong i, j;
/*
printf("_mfactor_disolve_clear called\n");
*/
    fmpq_poly_clear(I->dtq);
    fmpq_poly_clear(I->S);
    fmpq_poly_clear(I->R);


    for (i = 0; i <= I->r; i++)
    {
        fmpz_mpoly_clear(I->g + i, ctx);
        fmpz_mpoly_clear(I->q + i, ctx);
        fmpz_mpoly_clear(I->qt + i, ctx);
        fmpz_mpoly_clear(I->newt + i, ctx);
        for (j = 0; j < I->l; j++)
            fmpz_mpolyv_clear(I->delta_coeffs + i*I->l + j, ctx);
    }
    flint_free(I->g);
    flint_free(I->q);
    flint_free(I->qt);
    flint_free(I->newt);
    flint_free(I->delta_coeffs);

    for (j = 0; j < I->l; j++)
    {
        fmpq_poly_clear(I->inv_prod_dbetas + j);
        fmpq_poly_clear(I->dbetas + j);
        for (i = 0; i <= I->r; i++)
        {
            fmpz_mpoly_clear(I->prod_mbetas + i*I->l + j, ctx);
            fmpz_mpoly_clear(I->mbetas + i*I->l + j, ctx);
            fmpz_mpoly_clear(I->deltas + i*I->l + j, ctx);
        }
    }

    flint_free(I->inv_prod_dbetas);
    flint_free(I->dbetas);
    flint_free(I->prod_mbetas);
    flint_free(I->mbetas);
/*
printf("_mfactor_disolve_clear returning\n");
*/
}


static int _mfactor_disolve(
    flint_bitcnt_t bits,
    slong r,
    slong num,
    fmpz_mpoly_t t,
    const fmpz * alpha,
    const slong * deg,
    mfactor_disolve_t I,
    const fmpz_mpoly_ctx_t ctx)
{
    slong i, j, s, l;
    int success;
    fmpz_mpoly_struct * deltas = I->deltas + r*I->l;
    fmpz_mpoly_struct * newdeltas = I->deltas + (r - 1)*I->l;

    FLINT_ASSERT(num == I->l);

/*
flint_printf("_mfactor_disolve(r = %wd, num = %wd) called:\n", r, num);
flint_printf("t: "); fmpz_mpoly_print_pretty(t, NULL, ctx); printf("\n");
for (i = 0; i < num; i++)
{
flint_printf("betas[%wd]: ", i); fmpz_mpoly_print_pretty(betas + i, NULL, ctx); printf("\n");
}
*/

    if (r == 0)
    {
        _to_polyq(I->dtq, t, ctx);

        success = 1;
        for (i = 0; i < num; i++)
        {
            fmpq_poly_mul(I->S, I->dtq, I->inv_prod_dbetas + i);
            fmpq_poly_rem(I->R, I->S, I->dbetas + i);
            if (!_from_polyq(deltas + i, bits, I->R, ctx))
                return 0;
        }
        return 1;
    }
    else if (1)
    {
        fmpz_mpoly_struct * g = I->g + r;
        fmpz_mpoly_struct * q = I->q + r;
        fmpz_mpoly_struct * qt = I->qt + r;
        fmpz_mpoly_struct * newt = I->newt + r;
        fmpz_mpolyv_struct * delta_coeffs = I->delta_coeffs + r*I->l;

        for (i = 0; i < I->l; i++)
            delta_coeffs[i].length = 0;

        fmpz_mpoly_gen(g, r, ctx);
        fmpz_mpoly_sub_fmpz(g, g, alpha + r - 1, ctx);

        for (l = 0; l <= deg[r]; l++)
        {
            fmpz_mpoly_divrem(q, newt, t, g, ctx);
            fmpz_mpoly_swap(t, q, ctx);
            for (s = 0; s < l; s++)
            {
                for (i = 0; i < I->l; i++)
                {
                    if ( s < delta_coeffs[i].length
                      && l - s < I->prod_mbetas_coeffs[r*I->l + i].length)
                    {
                        fmpz_mpoly_mul(qt, I->prod_mbetas_coeffs[r*I->l + i].coeffs + l - s,
                                           delta_coeffs[i].coeffs + s, ctx);
                        fmpz_mpoly_sub(q, newt, qt, ctx);
                        fmpz_mpoly_swap(newt, q, ctx);
                    }
                }
            }

            if (!_mfactor_disolve(bits, r - 1, num, newt, alpha, deg, I, ctx))
                return 0;

            for (i = 0; i < I->l; i++)
            {
                if (!fmpz_mpoly_is_zero(newdeltas + i, ctx))
                {
                    if (l + I->prod_mbetas_coeffs[r*I->l + i].length - 1 > deg[r])
                        return 0;
                    fmpz_mpolyv_set_coeff(delta_coeffs + i, l, newdeltas + i, ctx);
                }
            }
        }

        for (i = 0; i < I->l; i++)
            fmpz_mpoly_from_mpolyv(deltas + i, delta_coeffs + i, r, alpha + r - 1, ctx);

        return 1;
    }
    else
    {
        fmpz_mpoly_t e, tt, pow, g, q, newt;

FLINT_ASSERT(0);

        fmpz_mpoly_init(e, ctx);
        fmpz_mpoly_init(tt, ctx);
        fmpz_mpoly_init(pow, ctx);
        fmpz_mpoly_init(g, ctx);
        fmpz_mpoly_init(q, ctx);
        fmpz_mpoly_init(newt, ctx);
        for (i = 0; i < num; i++)
            fmpz_mpoly_zero(deltas + i, ctx);

        fmpz_mpoly_set(e, t, ctx);

        fmpz_mpoly_one(pow, ctx);
        fmpz_mpoly_gen(g, r, ctx);
        fmpz_mpoly_sub_fmpz(g, g, alpha + r - 1, ctx);
        for (j = 0; j <= deg[r]; j++)
        {
            success = fmpz_mpoly_divides(q, e, pow, ctx);
            FLINT_ASSERT(success);
            fmpz_mpoly_evaluate_one_fmpz(newt, q, r, alpha + r - 1, ctx);
            success = _mfactor_disolve(bits, r - 1, num, newt, alpha, deg, I, ctx);
            if (!success)
                goto cleanup;

            fmpz_mpoly_set(e, t, ctx);
            for (i = 0; i < num; i++)
            {
                fmpz_mpoly_mul(tt, newdeltas + i, pow, ctx);
                fmpz_mpoly_add(deltas + i, deltas + i, tt, ctx);
                fmpz_mpoly_mul(tt, deltas + i, I->prod_mbetas + r*I->l + i, ctx);
                fmpz_mpoly_sub(e, e, tt, ctx);
            }

            fmpz_mpoly_mul(pow, pow, g, ctx);
        }

        success = fmpz_mpoly_is_zero(e, ctx);

cleanup:

        fmpz_mpoly_clear(e, ctx);
        fmpz_mpoly_clear(tt, ctx);
        fmpz_mpoly_clear(pow, ctx);
        fmpz_mpoly_clear(g, ctx);
        fmpz_mpoly_clear(q, ctx);
        fmpz_mpoly_clear(newt, ctx);
    }

/*
flint_printf("_mfactor_disolve(r = %wd, num = %wd) returning %d:\n", r, num, success);
flint_printf("t: "); fmpz_mpoly_print_pretty(t, NULL, ctx); printf("\n");
for (i = 0; i < num; i++)
{
flint_printf("deltas[%wd]: ", i); fmpz_mpoly_print_pretty(deltas + i, NULL, ctx); printf("\n");
}
*/
    return success;
}



static int _mfactor_lift_quartic2(
    slong m,
    fmpz_mpoly_factor_t lfac,
    const fmpz * alpha,
    const fmpz_mpoly_t A,
    const slong * degs,
    const fmpz_mpoly_ctx_t ctx)
{
    int success;
    slong i, j;
    slong r = lfac->length;
    fmpz_mpoly_t t, t2, t3;
    fmpz_mpoly_struct * betas, * deltas;
    mfactor_disolve_t I;
    fmpz_mpolyv_t Av;
    fmpz_mpolyv_struct B[2];
    slong tdeg;
/*
timeit_t timer;
slong timetot = 0;
*/
/*
flint_printf("_mfactor_lift_quartic2 called (degs[%wd] = %wd, alpha = %wd)\n", m, degs[m], *alpha);
flint_printf("lfac: "); fmpz_mpoly_factor_print_pretty(lfac, NULL, ctx); printf("\n");
flint_printf("A: "); fmpz_mpoly_print_pretty(A, NULL, ctx); printf("\n");
*/
    FLINT_ASSERT(r == 2);

    fmpz_mpoly_init(t, ctx);
    fmpz_mpoly_init(t2, ctx);
    fmpz_mpoly_init(t3, ctx);

    fmpz_mpolyv_init(Av, ctx);
    fmpz_mpoly_to_mpolyv(Av, A, m, alpha + m - 1, ctx);
    fmpz_mpolyv_fit_length(Av, degs[m] + 1, ctx);
    for (j = Av->length; j <= degs[m]; j++)
        fmpz_mpoly_zero(Av->coeffs + j, ctx);
/*
flint_printf("Av: "); fmpz_mpolyv_print_pretty(Av, NULL, ctx); printf("\n");
*/
    for (i = 0; i < r; i++)
    {
        fmpz_mpolyv_init(B + i, ctx);
        fmpz_mpoly_to_mpolyv(B + i, lfac->poly + i, m, alpha + m - 1, ctx);
        fmpz_mpolyv_fit_length(B + i, degs[m] + 1, ctx);
        for (j = Av->length; j <= degs[m]; j++)
            fmpz_mpoly_zero(B[i].coeffs + j, ctx);
/*
flint_printf("B + %wd: ", i); fmpz_mpolyv_print_pretty(B + i, NULL, ctx); printf("\n");
*/
    }
/*
timeit_start(timer);
*/
    betas  = (fmpz_mpoly_struct * ) flint_malloc(r*sizeof(fmpz_mpoly_struct));
    for (i = 0; i < r; i++)
    {
        betas[i] = B[i].coeffs[0];
    }
/*
timeit_start(timer);
*/
    _mfactor_disolve_init(I, lfac->length, m - 1, betas, alpha, ctx);
    deltas = I->deltas + (m - 1)*I->l;
/*
timeit_stop(timer);
timetot += timer->wall;
*/

    for (j = 1; j <= degs[m]; j++)
    {
        if (j < Av->length)
            fmpz_mpoly_set(t, Av->coeffs + j, ctx);
        else
            fmpz_mpoly_zero(t, ctx);

        for (i = 0; i <= j; i++)
        {
            fmpz_mpoly_mul(t2, B[0].coeffs + i, B[1].coeffs + j - i, ctx);
            fmpz_mpoly_sub(t3, t, t2, ctx);
            fmpz_mpoly_swap(t, t3, ctx);
        }
/*
flint_printf("disolve j = %wd, t = ", j); fmpz_mpoly_print_pretty(t, NULL, ctx); printf("\n");
*/
/*
timeit_start(timer);
*/
        success = _mfactor_disolve(A->bits, m - 1, r, t, alpha, degs, I, ctx);
/*
timeit_stop(timer);
timetot += timer->wall;
*/
        if (!success)
        {
            goto cleanup;
        }

        tdeg = -r;
        for (i = 0; i < r; i++)
        {
            fmpz_mpoly_add(t3, B[i].coeffs + j, deltas + i, ctx);
            fmpz_mpoly_swap(B[i].coeffs + j, t3, ctx);
            if (!fmpz_mpoly_is_zero(B[i].coeffs + j, ctx))
                B[i].length = FLINT_MAX(B[i].length, j + 1);
            FLINT_ASSERT(B[i].length > 0);
            tdeg += B[i].length;
        }

        if (tdeg > degs[m])
        {
            success = 0;
            goto cleanup;
        }
    }

    success = 1;

cleanup:

    _mfactor_disolve_clear(I, ctx);

    flint_free(betas);

    fmpz_mpolyv_clear(Av, ctx);
    for (i = 0; i < r; i++)
    {
        if (success)
            fmpz_mpoly_from_mpolyv(lfac->poly + i, B + i, m, alpha + m - 1, ctx);
        fmpz_mpolyv_clear(B + i, ctx);
    }

    fmpz_mpoly_clear(t, ctx);
    fmpz_mpoly_clear(t2, ctx);
    fmpz_mpoly_clear(t3, ctx);
/*
flint_printf("disolve total: %wd\n", timetot);
*/
    return success;
}

static int _mfactor_lift_quartic(
    slong m,
    fmpz_mpoly_factor_t lfac,
    const fmpz * alpha,
    const fmpz_mpoly_t A,
    const slong * degs,
    const fmpz_mpoly_ctx_t ctx)
{
    int success;
    slong i, j, k;
    slong r = lfac->length;
    fmpz_mpoly_t t, t1, t2, t3;
    fmpz_mpoly_struct * betas, * deltas;
    mfactor_disolve_t I;
    fmpz_mpolyv_t Av;
    fmpz_mpolyv_struct * B, * U;
    slong tdeg;
/*
timeit_t timer;
slong timetot = 0;
*/
/*
flint_printf("_mfactor_lift_quartic called (degs[%wd] = %wd, alpha = %wd)\n", m, degs[m], *alpha);
flint_printf("lfac: "); fmpz_mpoly_factor_print_pretty(lfac, NULL, ctx); printf("\n");
flint_printf("A: "); fmpz_mpoly_print_pretty(A, NULL, ctx); printf("\n");
*/
    FLINT_ASSERT(r > 2);

    B = (fmpz_mpolyv_struct *) flint_malloc(r*sizeof(fmpz_mpolyv_struct));
    U = (fmpz_mpolyv_struct *) flint_malloc(r*sizeof(fmpz_mpolyv_struct));

    fmpz_mpoly_init(t, ctx);
    fmpz_mpoly_init(t1, ctx);
    fmpz_mpoly_init(t2, ctx);
    fmpz_mpoly_init(t3, ctx);

    fmpz_mpolyv_init(Av, ctx);
    fmpz_mpoly_to_mpolyv(Av, A, m, alpha + m - 1, ctx);
    fmpz_mpolyv_fit_length(Av, degs[m] + 1, ctx);
    for (j = Av->length; j <= degs[m]; j++)
        fmpz_mpoly_zero(Av->coeffs + j, ctx);
/*
flint_printf("Av: "); fmpz_mpolyv_print_pretty(Av, NULL, ctx); printf("\n");
flint_printf("r = %wd \n", r);
*/
    for (k = 0; k < r; k++)
    {
/*
flint_printf("init loop k = %wd\n", k);
*/
        fmpz_mpolyv_init(U + k, ctx);
        fmpz_mpolyv_fit_length(U + k, degs[m] + 1, ctx);
        for (j = 0; j <= degs[m]; j++)
            fmpz_mpoly_zero(U[k].coeffs + j, ctx);

        fmpz_mpolyv_init(B + k, ctx);
        fmpz_mpoly_to_mpolyv(B + k, lfac->poly + k, m, alpha + m - 1, ctx);
        fmpz_mpolyv_fit_length(B + k, degs[m] + 1, ctx);
        for (j = Av->length; j <= degs[m]; j++)
            fmpz_mpoly_zero(B[k].coeffs + j, ctx);
/*
flint_printf("B + %wd: ", k); fmpz_mpolyv_print_pretty(B + k, NULL, ctx); printf("\n");
*/
    }
/*
timeit_start(timer);
*/

    betas  = (fmpz_mpoly_struct * ) flint_malloc(r*sizeof(fmpz_mpoly_struct));
    for (i = 0; i < r; i++)
    {
        betas[i] = B[i].coeffs[0];
/*
flint_printf("betas[%wd]: ", i); fmpz_mpoly_print_pretty(betas + i, NULL, ctx); printf("\n");
*/
    }

/*
timeit_start(timer);
*/
    _mfactor_disolve_init(I, lfac->length, m - 1, betas, alpha, ctx);
    deltas = I->deltas + (m - 1)*I->l;
/*
timeit_stop(timer);
timetot += timer->wall;
*/
    k = r - 2;
    fmpz_mpoly_mul(U[k].coeffs + 0, B[k].coeffs + 0, B[k + 1].coeffs + 0, ctx);
    for (k--; k >= 1; k--)
        fmpz_mpoly_mul(U[k].coeffs + 0, B[k].coeffs + 0, U[k + 1].coeffs + 0, ctx);

    for (j = 1; j <= degs[m]; j++)
    {
/*
flint_printf("main loop j = %wd ", j);
*/
        k = r -2;
        fmpz_mpoly_zero(U[k].coeffs + j, ctx);
        for (i = 0; i <= j; i++)
        {
            fmpz_mpoly_mul(t1, B[k].coeffs + i, B[k + 1].coeffs + j - i, ctx);
            fmpz_mpoly_add(U[k].coeffs + j, U[k].coeffs + j, t1, ctx);

        }
        for (k--; k >= 1; k--)
        {
            fmpz_mpoly_zero(U[k].coeffs + j, ctx);
            for (i = 0; i <= j; i++)
            {
                fmpz_mpoly_mul(t1, B[k].coeffs + i, U[k + 1].coeffs + j - i, ctx);
                fmpz_mpoly_add(U[k].coeffs + j, U[k].coeffs + j, t1, ctx);
            }
        }

        if (j < Av->length)
            fmpz_mpoly_set(t, Av->coeffs + j, ctx);
        else
            fmpz_mpoly_zero(t, ctx);

        for (i = 0; i <= j; i++)
        {
            fmpz_mpoly_mul(t2, B[0].coeffs + i, U[1].coeffs + j - i, ctx);
            fmpz_mpoly_sub(t3, t, t2, ctx);
            fmpz_mpoly_swap(t, t3, ctx);
        }
/*
flint_printf("disolve j = %wd, t = ", j); fmpz_mpoly_print_pretty(t, NULL, ctx); printf("\n");
*/
        if (fmpz_mpoly_is_zero(t, ctx))
        {
            continue;
        }
/*
timeit_start(timer);
*/
        success = _mfactor_disolve(A->bits, m - 1, r, t, alpha, degs, I, ctx);
/*
timeit_stop(timer);
timetot += timer->wall;
*/
        if (!success)
        {
/*
printf("dsolve failed\n");
*/
            goto cleanup;
        }

        tdeg = -r;
        for (i = 0; i < r; i++)
        {
            fmpz_mpoly_add(t3, B[i].coeffs + j, deltas + i, ctx);
            fmpz_mpoly_swap(B[i].coeffs + j, t3, ctx);
            if (!fmpz_mpoly_is_zero(B[i].coeffs + j, ctx))
                B[i].length = FLINT_MAX(B[i].length, j + 1);
            FLINT_ASSERT(B[i].length > 0);
            tdeg += B[i].length;
        }

        if (tdeg > degs[m])
        {
            success = 0;
            goto cleanup;
        }

        k = r - 2;
        fmpz_mpoly_mul(t, B[k].coeffs + 0, deltas + k + 1, ctx);
        fmpz_mpoly_mul(t1, deltas + k, B[k + 1].coeffs + 0, ctx);
        fmpz_mpoly_add(t, t, t1, ctx);
        fmpz_mpoly_add(U[k].coeffs + j, U[k].coeffs + j, t, ctx);
        for (k--; k >= 1; k--)
        {
            fmpz_mpoly_mul(t1, B[k].coeffs + 0, t, ctx);
            fmpz_mpoly_swap(t, t1, ctx);
            fmpz_mpoly_mul(t1, deltas + k, U[k + 1].coeffs + 0, ctx);
            fmpz_mpoly_add(t, t, t1, ctx);
            fmpz_mpoly_add(U[k].coeffs + j, U[k].coeffs + j, t, ctx);
        }

    }

    success = 1;

cleanup:
/*
printf("cleanup  success = %d\n", success);
*/
    _mfactor_disolve_clear(I, ctx);

    flint_free(betas);

    fmpz_mpolyv_clear(Av, ctx);
    for (i = 0; i < r; i++)
    {
        if (success)
            fmpz_mpoly_from_mpolyv(lfac->poly + i, B + i, m, alpha + m - 1, ctx);
        fmpz_mpolyv_clear(B + i, ctx);
        fmpz_mpolyv_clear(U + i, ctx);
    }

    flint_free(B);
    flint_free(U);
    fmpz_mpoly_clear(t, ctx);
    fmpz_mpoly_clear(t1, ctx);
    fmpz_mpoly_clear(t2, ctx);
    fmpz_mpoly_clear(t3, ctx);
/*
flint_printf("disolve total: %wd\n", timetot);
*/
    return success;
}


static int _mfactor_lift_quintic(
    slong m,
    fmpz_mpoly_factor_t lfac,
    const fmpz * alpha,
    const fmpz_mpoly_t A,
    const slong * degs,
    const fmpz_mpoly_ctx_t ctx)
{
    int success;
    slong i, j;
    slong r = lfac->length;
    fmpz_mpoly_t e, t, pow, g, q;
    fmpz_mpoly_struct * betas, * deltas;
    mfactor_disolve_t I;
/*
timeit_t timer;
slong timetot = 0;
*/
/*
flint_printf("_mfactor_lift_quintic called (m = %wd, alpha = %wd)\n", m, *alpha);
flint_printf("lfac: "); fmpz_mpoly_factor_print_pretty(lfac, NULL, ctx); printf("\n");
flint_printf("A: "); fmpz_mpoly_print_pretty(A, NULL, ctx); printf("\n");
*/
    FLINT_ASSERT(r > 1);
/*
timeit_start(timer);
*/
    fmpz_mpoly_init(e, ctx);
    fmpz_mpoly_init(t, ctx);
    fmpz_mpoly_init(pow, ctx);
    fmpz_mpoly_init(g, ctx);
    fmpz_mpoly_init(q, ctx);

    betas  = (fmpz_mpoly_struct * ) flint_malloc(r*sizeof(fmpz_mpoly_struct));
    for (i = 0; i < r; i++)
    {
        fmpz_mpoly_init(betas + i, ctx);
        fmpz_mpoly_evaluate_one_fmpz(betas + i, lfac->poly + i, m, alpha + m - 1, ctx);
/*
flint_printf("betas[%wd]: ", i); fmpz_mpoly_print_pretty(betas + i, NULL, ctx); printf("\n");
*/
    }

    fmpz_mpoly_mul(t, lfac->poly + 0, lfac->poly + 1, ctx);
    for (i = 2; i < r; i++)
        fmpz_mpoly_mul(t, t, lfac->poly + i, ctx);
    fmpz_mpoly_sub(e, A, t, ctx);

    fmpz_mpoly_one(pow, ctx);
    fmpz_mpoly_gen(g, m, ctx);
    fmpz_mpoly_sub_fmpz(g, g, alpha + m - 1, ctx);
/*
timeit_start(timer);
*/
    _mfactor_disolve_init(I, lfac->length, m - 1, betas, alpha, ctx);
    deltas = I->deltas + (m - 1)*I->l;
/*
timeit_stop(timer);
timetot += timer->wall;
*/
    for (j = 1; j <= degs[m]; j++)
    {
/*
flint_printf("<_mfactor_lift> j = %wd, error (length: %wd)\n", j, e->length); fmpz_mpoly_print_pretty(e, NULL, ctx); printf("\n");
*/
        if (fmpz_mpoly_is_zero(e, ctx))
        {
            success = 1;
            goto cleanup;
        }

        fmpz_mpoly_mul(pow, pow, g, ctx);
        success = fmpz_mpoly_divides(q, e, pow, ctx);
        FLINT_ASSERT(success);
        fmpz_mpoly_evaluate_one_fmpz(t, q, m, alpha + m - 1, ctx);
/*
flint_printf("disolve j = %wd, t = ", j); fmpz_mpoly_print_pretty(t, NULL, ctx); printf("\n");
*/
/*
timeit_start(timer);
*/
        success = _mfactor_disolve(A->bits, m - 1, r, t, alpha, degs, I, ctx);
/*
timeit_stop(timer);
timetot += timer->wall;
*/
        if (!success)
            goto cleanup;

        for (i = 0; i < r; i++)
        {
            fmpz_mpoly_mul(t, deltas + i, pow, ctx);
            fmpz_mpoly_add(lfac->poly + i, lfac->poly + i, t, ctx);
        }

        fmpz_mpoly_mul(t, lfac->poly + 0, lfac->poly + 1, ctx);
        for (i = 2; i < r; i++)
            fmpz_mpoly_mul(t, t, lfac->poly + i, ctx);
        fmpz_mpoly_sub(e, A, t, ctx);
    }

    success = fmpz_mpoly_is_zero(e, ctx);

cleanup:

    _mfactor_disolve_clear(I, ctx);

    fmpz_mpoly_clear(e, ctx);
    fmpz_mpoly_clear(t, ctx);
    fmpz_mpoly_clear(pow, ctx);
    fmpz_mpoly_clear(g, ctx);
    fmpz_mpoly_clear(q, ctx);

    for (i = 0; i < r; i++)
    {
        fmpz_mpoly_clear(betas + i, ctx);
    }

    flint_free(betas);
/*
flint_printf("disolve total: %wd\n", timetot);
*/
    return success;
}



static void _fmpz_mpoly_get_lc(
    fmpz_mpoly_t c,
    const fmpz_mpoly_t A,
    const fmpz_mpoly_ctx_t ctx)
{
    slong dummyvars[] = {0};
    ulong dummydegs[] = {0};

    dummyvars[0] = 0;
    dummydegs[0] = fmpz_mpoly_degree_si(A, 0, ctx);
    fmpz_mpoly_get_coeff_vars_ui(c, A, dummyvars, dummydegs, 1, ctx);
}

static void _fmpz_mpoly_set_lc(
    fmpz_mpoly_t A,
    const fmpz_mpoly_t c,
    const fmpz_mpoly_ctx_t ctx)
{
    slong deg;
    fmpz_mpoly_t t, g;

    fmpz_mpoly_init(t, ctx);
    fmpz_mpoly_init(g, ctx);

    deg = fmpz_mpoly_degree_si(A, 0, ctx);
    FLINT_ASSERT(deg >= 0);
    fmpz_mpoly_gen(g, 0, ctx);
    fmpz_mpoly_pow_ui(g, g, deg, ctx);
    _fmpz_mpoly_get_lc(t, A, ctx);
    fmpz_mpoly_sub(t, c, t, ctx);
    fmpz_mpoly_mul(t, t, g, ctx);
    fmpz_mpoly_add(A, A, t, ctx);

    fmpz_mpoly_clear(t, ctx);
    fmpz_mpoly_clear(g, ctx);
}


/*
have
[pfac1]*[pfac2]*[pfac3] = p = q(0)

change to
[lc(p)/lc(pfac1)*pfac1]*[lc(p)/lc(pfac2)*pfac2]*[lc(p)/lc(pfac2)*pfac3] = lc(p)^2 p

now each lc(p)/lc(pfaci)*pfaci has leading coefficient lc(p)

replace the leading coefficient of each lc(p)/lc(pfaci)*pfaci with lc(q)

lift against lc(q)^2*q

remove content
*/

static int _try_lift(
    fmpz_mpoly_factor_t qfac,
    const fmpz_mpoly_t q,
    const fmpz_mpoly_factor_t pfac,
    const fmpz_mpoly_t p,
    slong m,
    fmpz * alpha,
    slong n,
    const fmpz_mpoly_ctx_t ctx)
{
    int success;
    slong i;
    slong * newdeg;
    fmpz_mpoly_t lcq, lcp, t, newq;
    fmpz_mpoly_univar_t u;

    FLINT_ASSERT(pfac->length > 1);

    newdeg = (slong *) flint_malloc((n + 1)*sizeof(slong));
    fmpz_mpoly_init(lcq, ctx);
    fmpz_mpoly_init(lcp, ctx);
    fmpz_mpoly_init(t, ctx);
    fmpz_mpoly_init(newq, ctx);
    fmpz_mpoly_univar_init(u, ctx);

    FLINT_ASSERT(fmpz_mpoly_factor_matches(p, pfac, ctx));
    FLINT_ASSERT((fmpz_mpoly_evaluate_one_fmpz(t, q, m, alpha + m - 1, ctx),
                 fmpz_mpoly_equal(t, p, ctx)));


    _fmpz_mpoly_get_lc(lcq, q, ctx);
    _fmpz_mpoly_get_lc(lcp, p, ctx);

    fmpz_mpoly_pow_ui(t, lcq, pfac->length - 1, ctx);
    fmpz_mpoly_mul(newq, q, t, ctx);
    fmpz_mpoly_degrees_si(newdeg, newq, ctx);

    fmpz_mpoly_factor_fit_length(qfac, pfac->length, ctx);
    qfac->length = pfac->length;
    for (i = 0; i < pfac->length; i++)
    {
        _fmpz_mpoly_get_lc(t, pfac->poly + i, ctx);
        success = fmpz_mpoly_divides(t, lcp, t, ctx);
        FLINT_ASSERT(success);
        fmpz_mpoly_mul(qfac->poly + i, pfac->poly + i, t, ctx);
        _fmpz_mpoly_set_lc(qfac->poly + i, lcq, ctx);
        fmpz_one(qfac->exp + i);
    }

    if (qfac->length == 2)
        success = _mfactor_lift_quartic2(m, qfac, alpha, newq, newdeg, ctx);
    else if (qfac->length <  20)
        success = _mfactor_lift_quartic(m, qfac, alpha, newq, newdeg, ctx);
    else
        success = _mfactor_lift_quintic(m, qfac, alpha, newq, newdeg, ctx);

    if (!success)
        goto cleanup;

    for (i = 0; i < qfac->length; i++)
    {
        fmpz_mpoly_to_univar(u, qfac->poly + i, 0, ctx);
        success = fmpz_mpoly_univar_content_mpoly(t, u, ctx);
        if (!success)
            goto cleanup;
        success = fmpz_mpoly_divides(qfac->poly + i, qfac->poly + i, t, ctx);
        FLINT_ASSERT(success);
    }

    success = 1;

cleanup:

    flint_free(newdeg);
    fmpz_mpoly_clear(lcq, ctx);
    fmpz_mpoly_clear(lcp, ctx);
    fmpz_mpoly_clear(t, ctx);
    fmpz_mpoly_clear(newq, ctx);
    fmpz_mpoly_univar_clear(u, ctx);

    FLINT_ASSERT(!success || fmpz_mpoly_factor_matches(q, qfac, ctx));
    return success;
}


/* A is square free and primitive w.r.t all variables */
static int _irreducible_mvar_factors(
    fmpz_mpoly_factor_t fac,
    fmpz_mpoly_t A,
    const fmpz_mpoly_ctx_t ctx)
{
    int success;
    const slong n = ctx->minfo->nvars - 1;
    slong i, j, m, r;
    fmpz_t subset;
    fmpz * alpha, * alphait;
    fmpz_mpoly_struct * Aevals;
    slong * deg, * degeval;
    fmpz_mpoly_factor_t qfac, pfac, tfac, dfac;
    fmpz_mpoly_t t, p, q;
    fmpz_mpoly_univar_t u;

    FLINT_ASSERT(n > 1);
/*
flint_printf("_irreducible_mvar_factors(n = %wd) called\nA: ", n); fmpz_mpoly_print_pretty(A, NULL, ctx); printf("\n");
*/
    fmpz_init(subset);
    alphait = _fmpz_vec_init(n);
    alpha = _fmpz_vec_init(n);
    Aevals    = (fmpz_mpoly_struct *) flint_malloc(n*sizeof(fmpz_mpoly_struct));
    deg     = (slong *) flint_malloc((n + 1)*sizeof(slong));
    degeval = (slong *) flint_malloc((n + 1)*sizeof(slong));
    for (i = 0; i < n; i++)
    {
        fmpz_mpoly_init(Aevals + i, ctx);
    }
    fmpz_mpoly_factor_init(pfac, ctx);
    fmpz_mpoly_factor_init(qfac, ctx);
    fmpz_mpoly_factor_init(tfac, ctx);
    fmpz_mpoly_factor_init(dfac, ctx);
    fmpz_mpoly_init(t, ctx);
    fmpz_mpoly_init(p, ctx);
    fmpz_mpoly_init(q, ctx);
    fmpz_mpoly_univar_init(u, ctx);

    fmpz_mpoly_degrees_si(deg, A, ctx);
    goto got_alpha;

next_alpha:

    tuple_next(alphait, n);
    for (i = 0; i < n; i++)
    {
        j = n - 1 - i;
        fmpz_cdiv_q_2exp(alpha + j, alphait + i, 1);
        if (fmpz_is_even(alphait + i))
            fmpz_neg(alpha + j, alpha + j);
    }

got_alpha:
/*
usleep(1000000);
printf("!!!!!!!!!!!!!!!!!! chose alphait = "); tuple_print(alphait, n);
*/
printf("alpha = "); tuple_print(alpha, n);

    /* ensure degrees do not drop under evalutaion */
    for (i = n - 1; i >= 0; i--)
    {
        fmpz_mpoly_evaluate_one_fmpz(Aevals + i, i == n - 1 ? A : Aevals + i + 1, i + 1, alpha + i, ctx);
        fmpz_mpoly_degrees_si(degeval, Aevals + i, ctx);
        for (j = 0; j <= i; j++)
        {
            if (degeval[j] != deg[j])
            {
                tuple_saturate(alphait, n, n - i);
                goto next_alpha;
            }
        }
    }

    fmpz_mpoly_derivative(t, Aevals + 0, 0, ctx);
    fmpz_mpoly_gcd(t, t, Aevals + 0, ctx);
    if (!fmpz_mpoly_is_fmpz(t, ctx))
        goto next_alpha;
    fmpz_mpoly_to_univar(u, Aevals + 1, 0, ctx);
    fmpz_mpoly_univar_content_mpoly(t, u, ctx);
    if (!fmpz_mpoly_is_fmpz(t, ctx))
        goto next_alpha;

    _irreducible_bivar_factors(pfac, Aevals + 1, 0, 1, ctx);

    for (m = 2; m <= n; m++)
    {
        fmpz_mpoly_set(q, m < n ? Aevals + m : A, ctx);
        fmpz_mpoly_set(p, Aevals + m - 1, ctx);

        /* if pfac has only one factor, A must be irreducible */
        if (pfac->length == 1)
        {
            fmpz_mpoly_factor_append_ui(fac, A, 1, ctx);
            success = 1;
            goto cleanup;
        }

        success = _try_lift(qfac, q, pfac, p, m, alpha, n, ctx);
        if (success)
        {
            fmpz_mpoly_factor_swap(qfac, pfac, ctx);
            continue;
        }

        /* if we couldn't lift two local factors, A must be irreducible */
        if (pfac->length == 2)
        {
            fmpz_mpoly_factor_append_ui(fac, A, 1, ctx);
            success = 1;
            goto cleanup;
        }

        qfac->length = 0;
try_again:
        for (r = 1; r <= pfac->length/2; r++)
        {
            slong len = pfac->length;
            subset_first(subset, len, r);
            do {
                fmpz_mpoly_factor_one(dfac, ctx);
                fmpz_mpoly_factor_fit_length(dfac, 2, ctx);
                dfac->length = 2;
                fmpz_mpoly_one(dfac->poly + 0, ctx);
                fmpz_mpoly_one(dfac->poly + 1, ctx);
                fmpz_one(dfac->exp + 0);
                fmpz_one(dfac->exp + 1);
                for (i = 0; i < len; i++)
                {
                    j = fmpz_tstbit(subset, i);
                    fmpz_mpoly_mul(dfac->poly + j, dfac->poly + j, pfac->poly + i, ctx);
                }
                success = _try_lift(tfac, q, dfac, p, m, alpha, n, ctx);
                if (success)
                {
                    for (i = len - 1; i >= 0; i--)
                    {
                        if (fmpz_tstbit(subset, i))
                        {
                            fmpz_mpoly_swap(pfac->poly + i, pfac->poly + pfac->length - 1, ctx);
                            pfac->length--;
                        }
                    }
                    fmpz_mpoly_factor_append_ui(qfac, tfac->poly + 1, 1, ctx);
                    fmpz_mpoly_swap(q, tfac->poly + 0, ctx);
                    fmpz_mpoly_swap(p, dfac->poly + 0, ctx);
                    goto try_again;
                }
            }
            while (subset_next(subset, subset, pfac->length));
        }
        /* if pfac could not be combined, p must be irreducible */
        fmpz_mpoly_factor_append_ui(qfac, q, 1, ctx);
        fmpz_mpoly_factor_swap(qfac, pfac, ctx);
    }

    success = 1;

    for (i = 0; i < pfac->length; i++)
        fmpz_mpoly_factor_append_ui(fac, pfac->poly + i, 1, ctx);

cleanup:

    fmpz_clear(subset);
    _fmpz_vec_clear(alphait, n);
    _fmpz_vec_clear(alpha, n);
    for (i = 0; i < n; i++)
    {
        fmpz_mpoly_clear(Aevals + i, ctx);
    }
    flint_free(Aevals);
    flint_free(deg);
    flint_free(degeval);
    fmpz_mpoly_factor_clear(pfac, ctx);
    fmpz_mpoly_factor_clear(qfac, ctx);
    fmpz_mpoly_factor_clear(tfac, ctx);
    fmpz_mpoly_factor_clear(dfac, ctx);
    fmpz_mpoly_clear(t, ctx);
    fmpz_mpoly_clear(p, ctx);
    fmpz_mpoly_clear(q, ctx);
    fmpz_mpoly_univar_clear(u, ctx);

    return success;
}


/*
    A is primitive w.r.t to any variable appearing in A.
    A is square free with positive lead coeff.

    return 1 for success, 0 for failure
*/
static int _irreducible_factors(
    fmpz_mpoly_factor_t f,
    const fmpz_mpoly_t A,
    const fmpz_mpoly_ctx_t ctx)
{
    int success;
    slong i, mvars;
    slong * Adegs, * perm, * iperm;
    ulong * shift, * stride;
    TMP_INIT;

    if (A->bits > FLINT_BITS)
        return 0;

    TMP_START;

    Adegs = (slong *) TMP_ALLOC(ctx->minfo->nvars*sizeof(slong));
    perm = (slong *) TMP_ALLOC(ctx->minfo->nvars*sizeof(slong));
    iperm = (slong *) TMP_ALLOC(ctx->minfo->nvars*sizeof(slong));
    shift = (ulong *) TMP_ALLOC(ctx->minfo->nvars*sizeof(slong));
    stride = (ulong *) TMP_ALLOC(ctx->minfo->nvars*sizeof(slong));
    fmpz_mpoly_degrees_si(Adegs, A, ctx);

    mvars = 0;

    for (i = 0; i < ctx->minfo->nvars; i++)
    {
        shift[i] = 0;
        stride[i] = 1;
        iperm[i] = -1;
        if (Adegs[i] > 0)
        {
            perm[mvars] = i;
            mvars++;
        }
    }
    /* TODO: figure out nice perm */


    /* invert perm */
    for (i = 0; i < mvars; i++)
        iperm[perm[i]] = i;

    if (mvars == 1)
    {
        fmpz_mpoly_t t;
        fmpz_poly_t Au;
        fmpz_poly_factor_t fu;

        fmpz_mpoly_init(t, ctx);
        fmpz_poly_init(Au);
        fmpz_poly_factor_init(fu);

        _fmpz_mpoly_to_fmpz_poly_deflate(Au, A, perm[0], shift, stride, ctx);
        fmpz_poly_factor(fu, Au);

        fmpz_mpoly_factor_one(f, ctx);
        fmpz_mul(f->content, f->content, &fu->c); /* fu->c should be 1 */
        for (i = 0; i < fu->num; i++)
        {
            _fmpz_mpoly_from_fmpz_poly_inflate(t, A->bits, fu->p + i, perm[0], shift, stride, ctx);
            fmpz_mpoly_factor_append_ui(f, t, fu->exp[i], ctx); /* fu->exp[i] should be 1 */
        }

        fmpz_mpoly_clear(t, ctx);
        fmpz_poly_clear(Au);
        fmpz_poly_factor_clear(fu);

        success = 1;
    }
    else if (mvars == 2)
    {
        success = _irreducible_bivar_factors(f, A, perm[0], perm[1], ctx);
    }
    else
    {
        fmpz_mpoly_ctx_t lctx;
        fmpz_mpoly_t Al, B;
        fmpz_mpoly_factor_t Amfactors;

        fmpz_mpoly_ctx_init(lctx, mvars, ORD_LEX);
        fmpz_mpoly_init(Al, lctx);
        fmpz_mpoly_init(B, ctx);
        fmpz_mpoly_factor_init(Amfactors, lctx);

        fmpz_mpoly_convert_perm(Al, A->bits, lctx, A, ctx, perm);

        success = _irreducible_mvar_factors(Amfactors, Al, lctx);
        if (success)
        {
            fmpz_mpoly_factor_one(f, ctx);
            for (i = 0; i < Amfactors->length; i++)
            {
                fmpz_mpoly_convert_perm(B, A->bits, ctx, Amfactors->poly + i, lctx, iperm);
                FLINT_ASSERT(B->length > 0);
                if (fmpz_sgn(B->coeffs + 0) < 0)
                    fmpz_mpoly_neg(B, B, ctx);
                fmpz_mpoly_factor_append_fmpz(f, B, Amfactors->exp + i, ctx);
            }

            fmpz_mpoly_factor_clear(Amfactors, lctx);
            fmpz_mpoly_clear(B, ctx);
            fmpz_mpoly_clear(Al, lctx);
            fmpz_mpoly_ctx_clear(lctx);
        }
    }

    TMP_END;

    return success;
}


/*
    A is primitive w.r.t each variable appearing in it
    return 1 for success, 0 for failure
*/
static int _squarefree_factors(
    fmpz_mpoly_factor_t f,
    const fmpz_mpoly_t A,
    const fmpz_mpoly_ctx_t ctx)
{
    int success;
    slong v;
    fmpz_t k;
    fmpz_mpoly_t S, Sp, Sm, Ss, Y, Z;

    fmpz_init(k);
    fmpz_mpoly_init(S, ctx);
    fmpz_mpoly_init(Sp, ctx);
    fmpz_mpoly_init(Sm, ctx);
    fmpz_mpoly_init(Ss, ctx);
    fmpz_mpoly_init(Y, ctx);
    fmpz_mpoly_init(Z, ctx);

    fmpz_mpoly_factor_one(f, ctx);

    for (v = 0; v < ctx->minfo->nvars; v++)
    {
        fmpz_mpoly_derivative(Sp, A, v, ctx);

        if (!fmpz_mpoly_is_zero(Sp, ctx))
        {
            success = fmpz_mpoly_gcd_cofactors(Sm, Ss, Y, A, Sp, ctx);
            if (!success)
                goto cleanup;

            for (fmpz_set_ui(k, 1); !(fmpz_mpoly_derivative(Sp, Ss, v, ctx),
                                      fmpz_mpoly_sub(Z, Y, Sp, ctx),
                                      fmpz_mpoly_is_zero(Z, ctx));
                                                         fmpz_add_ui(k, k, 1))
            {
                success = fmpz_mpoly_gcd_cofactors(S, Ss, Y, Ss, Z, ctx);
                if (!success)
                    goto cleanup;

                fmpz_mpoly_factor_mul_mpoly_fmpz(f, S, k, ctx);
            }

            fmpz_mpoly_factor_mul_mpoly_fmpz(f, Ss, k, ctx);

            success = 1;
            goto cleanup;
        }
    }

    FLINT_ASSERT(fmpz_mpoly_is_fmpz(A, ctx));

    fmpz_mpoly_factor_mul_mpoly_ui(f, A, 1, ctx);

    success = 1;

cleanup:

    fmpz_clear(k);
    fmpz_mpoly_clear(S, ctx);
    fmpz_mpoly_clear(Sp, ctx);
    fmpz_mpoly_clear(Sm, ctx);
    fmpz_mpoly_clear(Ss, ctx);
    fmpz_mpoly_clear(Y, ctx);
    fmpz_mpoly_clear(Z, ctx);

    return success;
}

int fmpz_mpoly_factor(
    fmpz_mpoly_factor_t f,
    const fmpz_mpoly_t A,
    int full,
    const fmpz_mpoly_ctx_t ctx)
{
    int success;
    slong j, v;
    fmpz_mpoly_t c;
    fmpz_mpoly_univar_t u;
    fmpz_mpoly_factor_t newf, tempf;

    /* 0. set trivial factorization */
/*
{
timeit_t timer;
timeit_start(timer);
*/
    fmpz_mpoly_factor_one(f, ctx);
    if (fmpz_mpoly_is_fmpz(A, ctx))
    {
        fmpz_mpoly_get_fmpz(f->content, A, ctx);
        return 1;
    }
    else
    {
        _fmpz_vec_content(f->content, A->coeffs, A->length);
        if (fmpz_sgn(A->coeffs + 0) < 0)
            fmpz_neg(f->content, f->content);

        fmpz_mpoly_factor_fit_length(f, 1, ctx);
        fmpz_mpoly_scalar_divexact_fmpz(f->poly + 0, A, f->content, ctx);
        fmpz_one(f->exp + 0);
        f->length = 1;
    }

    if (A->bits > FLINT_BITS)
    {
        return 0;
    }

    fmpz_mpoly_factor_init(newf, ctx);
    fmpz_mpoly_factor_init(tempf, ctx);
    fmpz_mpoly_univar_init(u, ctx);
    fmpz_mpoly_init(c, ctx);
/*
timeit_stop(timer);
flint_printf("trivial time: %wd\n", timer->wall);
}
*/
    /* 1. ensure factors are primitive w.r.t any variable */
    FLINT_ASSERT(fmpz_mpoly_factor_matches(A, f, ctx));
/*
{
timeit_t timer;
timeit_start(timer);
*/
    for (v = 0; v < ctx->minfo->nvars; v++)
    {
        fmpz_set(newf->content, f->content);
        newf->length = 0;
        for (j = 0; j < f->length; j++)
        {
            if (fmpz_is_one(f->exp + j))
            {
                fmpz_mpoly_to_univar(u, f->poly + j, v, ctx);
                FLINT_ASSERT(u->length > 0);

                success = fmpz_mpoly_univar_content_mpoly(c, u, ctx);
                if (!success)
                    goto cleanup;

                fmpz_mpoly_univar_divexact_mpoly(u, c, ctx);

                fmpz_mpoly_factor_mul_mpoly_ui(newf, c, 1, ctx);

                if (u->exps[u->length - 1] != 0)
                {
                    fmpz_mpoly_gen(c, v, ctx);
                    fmpz_mpoly_factor_append_ui(newf, c, u->exps[u->length - 1], ctx);
                    fmpz_mpoly_univar_shift_right(u, u->exps[u->length - 1], ctx);
                }

                if (u->length > 1)
                {
                    fmpz_mpoly_from_univar_bits(c, A->bits, u, v, ctx);
                    fmpz_mpoly_factor_append_ui(newf, c, 1, ctx);
                }
                else
                {
                    FLINT_ASSERT(fmpz_mpoly_is_one(u->coeffs + 0, ctx));
                }
            }
            else
            {
                FLINT_ASSERT(fmpz_sgn(f->exp + j) > 0);
                fmpz_mpoly_factor_append_fmpz(newf, f->poly + j, f->exp + j, ctx);
            }
        }

        fmpz_mpoly_factor_swap(f, newf, ctx);
    }
/*
timeit_stop(timer);
flint_printf("primitive time: %wd\n", timer->wall);
}
*/
    /* 2. ensure factors are squarefree */
    FLINT_ASSERT(fmpz_mpoly_factor_matches(A, f, ctx));
/*
{
timeit_t timer;
timeit_start(timer);
*/
    fmpz_set(newf->content, f->content);
    newf->length = 0;
    for (j = 0; j < f->length; j++)
    {
        success = _squarefree_factors(tempf, f->poly + j, ctx);
        if (!success)
            goto cleanup;
        fmpz_mpoly_factor_mul_factor_fmpz(newf, tempf, f->exp + j, ctx);
    }
    fmpz_mpoly_factor_swap(f, newf, ctx);

    /* skip irreducible factorization if not wanted */
    if (!full)
    {
        success = 1;
        goto cleanup;
    }
/*
timeit_stop(timer);
flint_printf("squarefree time: %wd\n", timer->wall);
}
*/
    /* 3. ensure factors are irreducible */
    FLINT_ASSERT(fmpz_mpoly_factor_matches(A, f, ctx));
/*
{
timeit_t timer;
timeit_start(timer);
*/
    fmpz_set(newf->content, f->content);
    newf->length = 0;
    for (j = 0; j < f->length; j++)
    {
        success = _irreducible_factors(tempf, f->poly + j, ctx);
        if (!success)
            goto cleanup;
        fmpz_mpoly_factor_mul_factor_fmpz(newf, tempf, f->exp + j, ctx);
    }
    fmpz_mpoly_factor_swap(f, newf, ctx);
/*
timeit_stop(timer);
flint_printf("irreducible time: %wd\n", timer->wall);
}
*/
    success = 1;

cleanup:
/*
{
timeit_t timer;
timeit_start(timer);
*/
    FLINT_ASSERT(fmpz_mpoly_factor_matches(A, f, ctx));
    fmpz_mpoly_factor_clear(newf, ctx);
    fmpz_mpoly_factor_clear(tempf, ctx);
    fmpz_mpoly_univar_clear(u, ctx);
    fmpz_mpoly_clear(c, ctx);
/*
timeit_stop(timer);
flint_printf("check time: %wd\n", timer->wall);
}
*/

    return 1;
}

