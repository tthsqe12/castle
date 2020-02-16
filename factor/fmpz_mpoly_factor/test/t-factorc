/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "fmpz_mpoly_factor.h"
#include "profiler.h"

int
main(void)
{
    slong i, j, k;
    FLINT_TEST_INIT(state);

    flint_printf("factor....");

    {
timeit_t timer;
        fmpz_mpoly_ctx_t ctx;
        fmpz_mpoly_t a;
        fmpz_mpoly_factor_t fac;
        const char * vars[] = {"x", "y", "z", "t"};

        fmpz_mpoly_ctx_init(ctx, 3, ORD_LEX);
        fmpz_mpoly_init(a, ctx);
        fmpz_mpoly_set_str_pretty(a, "((y*z+1)*x+y+z+1)*(x^2+y^2+z^2+2)*(x^3+y^3+z^3+3)", vars, ctx);

flint_printf("\n******* starting example *********\n");

        printf(">>   a: "); fmpz_mpoly_print_pretty(a, vars, ctx); printf("\n");

        fmpz_mpoly_factor_init(fac, ctx);
timeit_start(timer);
        fmpz_mpoly_factor(fac, a, 1, ctx);
        fmpz_mpoly_factor_sort(fac, ctx);
timeit_stop(timer);
        printf("<< fac: "); fmpz_mpoly_factor_print_pretty(fac, vars, ctx); printf("\n");

flint_printf("factor time (length = %wd): %wd\n", a->length, timer->wall);

        fmpz_mpoly_factor_clear(fac, ctx);
        fmpz_mpoly_clear(a, ctx);
        fmpz_mpoly_ctx_clear(ctx);
    }


    if (1) {
timeit_t timer;
        fmpz_mpoly_ctx_t ctx;
        fmpz_mpoly_t a;
        fmpz_mpoly_factor_t fac;
        const char * vars[] = {"x", "y", "z", "t"};

        fmpz_mpoly_ctx_init(ctx, 4, ORD_LEX);
        fmpz_mpoly_factor_init(fac, ctx);
        fmpz_mpoly_init(a, ctx);
        fmpz_mpoly_set_str_pretty(a, "x^99-y^99*z^33", vars, ctx);

flint_printf("\n******* starting example *********\n");

        printf(">>   a: "); fmpz_mpoly_print_pretty(a, vars, ctx); printf("\n");
timeit_start(timer);
        fmpz_mpoly_factor(fac, a, 1, ctx);
        fmpz_mpoly_factor_sort(fac, ctx);
timeit_stop(timer);
        printf("<< fac: "); fmpz_mpoly_factor_print_pretty(fac, vars, ctx); printf("\n");
flint_printf("factor time (length = %wd): %wd\n", a->length, timer->wall);

        fmpz_mpoly_factor_clear(fac, ctx);
        fmpz_mpoly_clear(a, ctx);
        fmpz_mpoly_ctx_clear(ctx);
    }

    if (1) {
timeit_t timer;
        fmpz_mpoly_ctx_t ctx;
        fmpz_mpoly_t a;
        fmpz_mpoly_factor_t fac;
        const char * vars[] = {"x", "y", "z", "t"};

        fmpz_mpoly_ctx_init(ctx, 2, ORD_LEX);
        fmpz_mpoly_factor_init(fac, ctx);
        fmpz_mpoly_init(a, ctx);

flint_printf("\n******* starting bernardin example *********\n");

        fmpz_mpoly_set_str_pretty(a, "(x^2+y+1)*(x+y^2+1)^2*(x*y+1)^13*(x*y+2)^14*(x+y+1)^113", vars, ctx);
/*        printf(">>   a: "); fmpz_mpoly_print_pretty(a, vars, ctx); printf("\n");*/
timeit_start(timer);
        fmpz_mpoly_factor(fac, a, 1, ctx);
        fmpz_mpoly_factor_sort(fac, ctx);
timeit_stop(timer);
        printf("<< fac: "); fmpz_mpoly_factor_print_pretty(fac, vars, ctx); printf("\n");
flint_printf("         time: %wd\n", timer->wall);

        fmpz_mpoly_factor_clear(fac, ctx);
        fmpz_mpoly_clear(a, ctx);
        fmpz_mpoly_ctx_clear(ctx);
    }

    /* fateman */

    for (i = 0; i <= 20; i++)
    {
timeit_t timer;
        fmpz_mpoly_ctx_t ctx;
        fmpz_mpoly_t a, b;
        fmpz_mpoly_factor_t fac;
        const char * vars[] = {"x", "y", "z", "t"};

flint_printf("\n******* starting fateman pow = %wd *********\n", i);

        fmpz_mpoly_ctx_init(ctx, 4, ORD_LEX);
        fmpz_mpoly_factor_init(fac, ctx);
        fmpz_mpoly_init(a, ctx);
        fmpz_mpoly_init(b, ctx);
        fmpz_mpoly_set_str_pretty(b, "1+x+y+z+t", vars, ctx);
        fmpz_mpoly_pow_ui(b, b, i, ctx);
        fmpz_mpoly_add_ui(a, b, 1, ctx);
        fmpz_mpoly_add_ui(b, b, 2, ctx);
        fmpz_mpoly_mul(a, a, b, ctx);
/*
        printf(">>   a: "); fmpz_mpoly_print_pretty(a, vars, ctx); printf("\n");
*/
timeit_start(timer);
        fmpz_mpoly_factor(fac, a, 1, ctx);
        fmpz_mpoly_factor_sort(fac, ctx);
timeit_stop(timer);
/*
        printf("<< fac: "); fmpz_mpoly_factor_print_pretty(fac, vars, ctx); printf("\n");
*/
flint_printf("factor time (length = %wd): %wd\n", a->length, timer->wall);

        k = (i > 0);
        for (j = 1; j <= i; j++)
        {
            if ((j%2) != 0 && (i%j) == 0)
                k++;
        }

        if (k != fac->length)
        {
            flint_printf("FAIL\n");
            flint_printf("fateman power %wd has length %wd, expected %wd\n", i, fac->length, k);
            flint_abort();
        }

        fmpz_mpoly_factor_clear(fac, ctx);
        fmpz_mpoly_clear(a, ctx);
        fmpz_mpoly_clear(b, ctx);
        fmpz_mpoly_ctx_clear(ctx);
    }

    /* bivariate examples */
    if (0) {
        fmpz_mpoly_ctx_t ctx;
        fmpz_mpoly_t a, b;
        fmpz_mpoly_factor_t fac;
        const char * vars[] = {"x", "y", "z1", "z2", "z3"};
        fmpz * shift  = _fmpz_vec_init(5);
        fmpz * stride = _fmpz_vec_init(5);
        fmpz_mpoly_struct * sub[5];

        fmpz_mpoly_ctx_init(ctx, 5, ORD_LEX);
        fmpz_mpoly_init(a, ctx);
        fmpz_mpoly_init(b, ctx);
        fmpz_mpoly_factor_init(fac, ctx);

flint_printf("\n******* starting bivar power 9 *********\n");
        fmpz_mpoly_set_str_pretty(a, "(1+y)^9*x^9-y^9", vars, ctx);
        printf(">>   a: "); fmpz_mpoly_print_pretty(a, vars, ctx); printf("\n");
        fmpz_mpoly_factor(fac, a, 1, ctx);
        printf("<< fac: "); fmpz_mpoly_factor_print_pretty(fac, vars, ctx); printf("\n");

        fmpz_mpoly_set_str_pretty(b, "(x+z1+z2+z3)"
                                    "*(x+z1+z2-z3)"
                                    "*(x+z1-z2+z3)"
                                    "*(x+z1-z2-z3)"
                                    "*(x-z1+z2+z3)"
                                    "*(x-z1+z2-z3)"
                                    "*(x-z1-z2+z3)"
                                    "*(x-z1-z2-z3)", vars, ctx);

        for (i = 0; i < 5; i++)
        {
            sub[i] = (fmpz_mpoly_struct *) flint_malloc(sizeof(fmpz_mpoly_struct));
            fmpz_mpoly_init(sub[i], ctx);
            fmpz_zero(shift + i);
            fmpz_one(stride + i);
        }
        fmpz_set_ui(stride + 2, 2);
        fmpz_set_ui(stride + 3, 2);
        fmpz_set_ui(stride + 4, 2);
        fmpz_mpoly_deflate(b, b, shift, stride, ctx);

flint_printf("\n******* starting bivar y+1, y+2, y+3 *********\n");
        fmpz_mpoly_set_str_pretty(sub[0], "x", vars, ctx);
        fmpz_mpoly_set_str_pretty(sub[1], "y", vars, ctx);
        fmpz_mpoly_set_str_pretty(sub[2], "y+1", vars, ctx);
        fmpz_mpoly_set_str_pretty(sub[3], "y+2", vars, ctx);
        fmpz_mpoly_set_str_pretty(sub[4], "y+3", vars, ctx);
        fmpz_mpoly_compose_fmpz_mpoly(a, b, sub, ctx, ctx);
        printf(">>   a: "); fmpz_mpoly_print_pretty(a, vars, ctx); printf("\n");
        fmpz_mpoly_factor(fac, a, 1, ctx);
        printf("<< fac: "); fmpz_mpoly_factor_print_pretty(fac, vars, ctx); printf("\n");

flint_printf("\n******* starting bivar y, y+1, y+2 *********\n");
        fmpz_mpoly_set_str_pretty(sub[0], "x", vars, ctx);
        fmpz_mpoly_set_str_pretty(sub[1], "y", vars, ctx);
        fmpz_mpoly_set_str_pretty(sub[2], "y", vars, ctx);
        fmpz_mpoly_set_str_pretty(sub[3], "y+1", vars, ctx);
        fmpz_mpoly_set_str_pretty(sub[4], "y+2", vars, ctx);
        fmpz_mpoly_compose_fmpz_mpoly(a, b, sub, ctx, ctx);
        printf(">>   a: "); fmpz_mpoly_print_pretty(a, vars, ctx); printf("\n");
        fmpz_mpoly_factor(fac, a, 1, ctx);
        printf("<< fac: "); fmpz_mpoly_factor_print_pretty(fac, vars, ctx); printf("\n");

flint_printf("\n******* starting bivar y+1, y+4, y+9 *********\n");
        fmpz_mpoly_set_str_pretty(sub[0], "x + y + 2", vars, ctx);
        fmpz_mpoly_set_str_pretty(sub[1], "y", vars, ctx);
        fmpz_mpoly_set_str_pretty(sub[2], "y+1", vars, ctx);
        fmpz_mpoly_set_str_pretty(sub[3], "y+4", vars, ctx);
        fmpz_mpoly_set_str_pretty(sub[4], "y+9", vars, ctx);
        fmpz_mpoly_compose_fmpz_mpoly(a, b, sub, ctx, ctx);
        printf(">>   a: "); fmpz_mpoly_print_pretty(a, vars, ctx); printf("\n");
        fmpz_mpoly_factor(fac, a, 1, ctx);
        printf("<< fac: "); fmpz_mpoly_factor_print_pretty(fac, vars, ctx); printf("\n");

        fmpz_mpoly_factor_clear(fac, ctx);
        fmpz_mpoly_clear(a, ctx);
        fmpz_mpoly_ctx_clear(ctx);
    }

    FLINT_TEST_CLEANUP(state);
    
    flint_printf("PASS\n");
    return 0;
}
