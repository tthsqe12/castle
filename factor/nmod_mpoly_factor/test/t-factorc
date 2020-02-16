/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "nmod_mpoly_factor.h"
#include "profiler.h"

int
main(void)
{
    int result;
    FLINT_TEST_INIT(state);

    flint_printf("factor....");
    fflush(stdout);

    {
        nmod_mpoly_ctx_t ctx;
        nmod_mpoly_t a;
        nmod_mpoly_factor_t fac;
        const char * vars[] = {"x", "y", "z", "t"};
        nmod_mpoly_ctx_init(ctx, 3, ORD_LEX, 2);
        nmod_mpoly_init(a, ctx);
        nmod_mpoly_set_str_pretty(a,
"(z^8*x^8+x^1+y^16+y^1+z^8+z^3)*((y^4+z^3+z)*x^8+x^1+y^16+y^1+z^8+z^3)"
, vars, ctx);
printf("\n******** starting example 3 vars ********\n");
printf("       factoring: "); nmod_mpoly_print_pretty(a, vars, ctx); printf("\n");
        nmod_mpoly_factor_init(fac, ctx);
        result = nmod_mpoly_factor(fac, a, 1, ctx);
        nmod_mpoly_factor_sort(fac, ctx);
printf("factorization(%d): ", result); nmod_mpoly_factor_print_pretty(fac, vars, ctx); printf("\n");
        nmod_mpoly_factor_clear(fac, ctx);
        nmod_mpoly_clear(a, ctx);
        nmod_mpoly_ctx_clear(ctx);
    }
#if 0
    {
timeit_t timer;
        nmod_mpoly_ctx_t ctx;
        nmod_mpoly_t a;
        nmod_mpoly_factor_t fac;
        const char * vars[] = {"x", "y", "z", "t"};

        nmod_mpoly_ctx_init(ctx, 2, ORD_LEX, 2);
        nmod_mpoly_init(a, ctx);
        nmod_mpoly_set_str_pretty(a, "(x^8+x+y^16+y)*(x^8+x+y^4+y)", vars, ctx);

printf("\n******** starting univar nmod_mpoly ********\n");
printf("    factoring: "); nmod_mpoly_print_pretty(a, vars, ctx); printf("\n");

        nmod_mpoly_factor_init(fac, ctx);
timeit_start(timer);
        nmod_mpoly_factor(fac, a, 1, ctx);
        nmod_mpoly_factor_sort(fac, ctx);
timeit_stop(timer);
printf("factorization: "); nmod_mpoly_factor_print_pretty(fac, vars, ctx); printf("\n");
flint_printf("         time: %wd\n", timer->wall);

        nmod_mpoly_factor_clear(fac, ctx);
        nmod_mpoly_clear(a, ctx);
        nmod_mpoly_ctx_clear(ctx);
    }

    for (i = 0; i < 4; i++)
    {
timeit_t timer;
        nmod_mpoly_ctx_t ctx;
        nmod_mpoly_t a;
        nmod_mpoly_factor_t fac;
        const char * vars[] = {"x", "y", "z", "t"};
        const mp_limb_t moduli[] = {3, 7, 13, 8191};

        nmod_mpoly_ctx_init(ctx, 2, ORD_LEX, moduli[i]);
        nmod_mpoly_init(a, ctx);

flint_printf("\n******* starting bernardin example mod %wu *********\n", ctx->ffinfo->mod.n);


        nmod_mpoly_set_str_pretty(a, "(x^2+y+1)*(x+y^2+1)^2*(x*y+1)^13*(x*y+2)^14*(x+y+1)^113", vars, ctx);
/*
printf("    factoring: "); nmod_mpoly_print_pretty(a, vars, ctx); printf("\n");
*/
        nmod_mpoly_factor_init(fac, ctx);
timeit_start(timer);
        nmod_mpoly_factor(fac, a, 1, ctx);
        nmod_mpoly_factor_sort(fac, ctx);
timeit_stop(timer);
printf("factorization: "); nmod_mpoly_factor_print_pretty(fac, vars, ctx); printf("\n");
flint_printf("         time: %wd\n", timer->wall);

        nmod_mpoly_factor_clear(fac, ctx);
        nmod_mpoly_clear(a, ctx);
        nmod_mpoly_ctx_clear(ctx);
    }

    {
timeit_t timer;
        nmod_mpoly_ctx_t ctx;
        nmod_mpoly_t a;
        nmod_mpoly_factor_t fac;
        const char * vars[] = {"x", "y", "z", "t"};

        nmod_mpoly_ctx_init(ctx, 2, ORD_LEX, 2);
        nmod_mpoly_init(a, ctx);
        nmod_mpoly_set_str_pretty(a, "(x+1)^4097", vars, ctx);

printf("\n******** starting univar nmod_mpoly ********\n");
printf("    factoring: "); nmod_mpoly_print_pretty(a, vars, ctx); printf("\n");

        nmod_mpoly_factor_init(fac, ctx);
timeit_start(timer);
        nmod_mpoly_factor(fac, a, 1, ctx);
        nmod_mpoly_factor_sort(fac, ctx);
timeit_stop(timer);
printf("factorization: "); nmod_mpoly_factor_print_pretty(fac, vars, ctx); printf("\n");
flint_printf("         time: %wd\n", timer->wall);

        nmod_mpoly_factor_clear(fac, ctx);
        nmod_mpoly_clear(a, ctx);
        nmod_mpoly_ctx_clear(ctx);
    }

	{
timeit_t timer;
		nmod_poly_factor_t fac;
		nmod_poly_t a;

		nmod_poly_factor_init(fac);
		nmod_poly_init(a, 2);

		nmod_poly_set_coeff_ui(a, 1, 1);
		nmod_poly_set_coeff_ui(a, 0, 1);
		nmod_poly_pow(a, a, 4097);

printf("\n******** starting univar nmod_poly ********\n");
printf("    factoring: "); nmod_poly_print_pretty(a, "x"); printf("\n");
timeit_start(timer);
        nmod_poly_factor(fac, a);
timeit_stop(timer);
printf("factorization: "); nmod_poly_factor_print(fac);
flint_printf("         time: %wd\n", timer->wall);

		nmod_poly_factor_clear(fac);
		nmod_poly_clear(a);
	}
#endif
    FLINT_TEST_CLEANUP(state);
    
    flint_printf("PASS\n");
    return 0;
}
