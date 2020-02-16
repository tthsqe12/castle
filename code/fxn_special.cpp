#include <cmath>
#include <cfloat>

#include "timing.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"
#include "flint/arith.h"


ex dcode_sZeta(er EE)
{
    uex e(ecopy(EE));
//std::cout << "dcode_sZeta: " << ex_tostring_full(e.get()) << std::endl;

    if (elength(e.get()) != 1)
    {
        return e.release();
    }

    if (eis_int(echild(e.get(),1)))
    {
        er X = echild(e.get(),1);

        if (fmpz_fits_si(eint_data(X)))
        {
            slong a = fmpz_get_si(eint_data(X));
            if (a <= 0)
            {
                if (a & 1)
                {
                    xfmpq_t r;
                    xfmpz_t s(1-a);
                    arith_bernoulli_number(r.data, 1 - a);
                    fmpq_neg(r.data, r.data);
                    fmpq_div_fmpz(r.data, r.data, s.data);
                    return emake_rat_move(r);
                }
                else
                {
                    if (a == 0)
                    {
                        return emake_crat(-1,2);
                    }
                    else
                    {
                        return emake_cint(0);
                    }
                }
            }
            else
            {
                if (a & 1)
                {
                    if (a == 1)
                    {
                        return emake_nan_ComplexInfinity();
                    }
                    else
                    {
                        return e.release();
                    }
                }
                else
                {
                    a = a/2;
                    std::vector<xfmpq_t> v;
                    v.reserve(a);
                    v.push_back(xfmpq_t(1,6));
                    xfmpq_t t, p, u;
                    for (slong n = 2; n <= a; n++)
                    {
                        fmpq_zero(t.data);
                        for (slong k = 1; k <= n - 1; k++)
                        {
                            fmpq_mul(p.data, v[k - 1].data, v[n - k - 1].data);
                            fmpq_add(t.data, t.data, p.data);
                        }
                        fmpq_set_si(u.data, 2, 2*n + 1);
                        fmpq_mul(t.data, t.data, u.data);
                        v.push_back(std::move(t));
                    }
                    uex pipower(emake_node(ecopy(gs.sym_sPower.get()),
                                           ecopy(gs.sym_sPi.get()),
                                           ecopy(X)
                                          ));
                    return emake_node(gs.sym_sTimes.copy(),
                                      emake_rat_move(v.back()),
                                      pipower.release()
                                     );
                }
            }
        }
        else
        {
            return e.release();
        }
    }
    else if (eis_real(echild(e.get(),1)))
    {
        er X = echild(e.get(),1);
		ex z = emake_real();
        slong p = ereal_number(X).wprec();
        arb_zeta(ereal_data(z), ereal_data(X), p + EXTRA_PRECISION_BASIC);
        return efix_real(z);
    }
    else if (eis_double(echild(e.get(),1)))
    {
        er X = echild(e.get(),1);
        slong p = 53;
        arb_set_d(gs.tmpreal[7].data, edouble_number(X));
        arb_zeta(gs.tmpreal[6].data, gs.tmpreal[7].data, p + EXTRA_PRECISION_BASIC);
        return emake_double(arf_get_d(arb_midref(gs.tmpreal[6].data), ARF_RND_NEAR));
    }
    else
    {
        return e.release();
    }
}



ex dcode_sFactorial(er e)
{
//std::cout << "dcode_sFactorial: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sFactorial.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    er X = echild(e,1);
    if (eis_int(X))
    {
        if (fmpz_sgn(eint_data(X)) < 0)
        {
            return emake_nan_ComplexInfinity();
        }
        else
        {
            if (fmpz_fits_si(eint_data(X)))
            {
                slong n = fmpz_get_si(eint_data(X));
                xfmpz_t z;
                fmpz_fac_ui(z.data, n);
                return emake_int_move(z);
            }
            else
            {
                return emake_nan_ComplexInfinity();
            }
        }
    }
    else if (eis_rat(X))
    {
        if (  !fmpz_fits_si(fmpq_numref(erat_data(X)))
            || fmpz_cmp_si(fmpq_denref(erat_data(X)), WORD(2)) != 0)
        {
            return ecopy(e);
        }

        slong n = fmpz_get_si(fmpq_numref(erat_data(X)));
        assert(n & 1);

        ex t = emake_rat(1,2);
        uex sqrtpi(emake_node(ecopy(gs.sym_sPower.get()), ecopy(gs.sym_sPi.get()), t));

        if (n == -1)
        {
            return sqrtpi.release();
        }

        xfmpq_t z(1,1);        
        if (n > 0)
        {
            fmpz_mul_2exp(fmpq_denref(z.data), fmpq_denref(z.data), (n + 1)/2);
            for (slong i = 3; i <= n; i += 2)
            {
                fmpz_mul_ui(fmpq_numref(z.data), fmpq_numref(z.data), i);
            }
        }
        else
        {
            n = -n;
            fmpz_mul_2exp(fmpq_numref(z.data), fmpq_numref(z.data), (n - 1)/2);
            if (n & 2)
            {
                fmpz_neg(fmpq_numref(z.data), fmpq_numref(z.data));
            }
            for (slong i = 3; i < n; i += 2)
            {
                fmpz_mul_ui(fmpq_denref(z.data), fmpq_denref(z.data), i);
            }            
        }
        t = emake_rat_move(z);
        return emake_node(gs.sym_sTimes.copy(), t, sqrtpi.release());
    }
    else if (eis_real(X))
    {
        xarb_t z;
        slong p = ereal_number(X).wprec();
        arb_add_ui(z.data, ereal_number(X).data, 1, p + EXTRA_PRECISION_BASIC);
        arb_gamma(z.data, z.data, p + EXTRA_PRECISION_BASIC);
        return emake_real_move(z);
    }
    else if (eis_double(X))
    {
        xarb_t z;
        slong p = 53;
        arb_set_d(z.data, edouble_number(X));
        arb_add_ui(z.data, z.data, 1, p + EXTRA_PRECISION_BASIC);
        arb_gamma(z.data, z.data, p + EXTRA_PRECISION_BASIC);
        return emake_double(arf_get_d(arb_midref(z.data), ARF_RND_NEAR));
    }
    else
    {
        return ecopy(e);
    }
}


ex dcode_sFibonacci(er e)
{
//std::cout << "dcode_sFibonacci: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sFibonacci.get()));

    if (elength(e) == 1)
    {
        er x = echild(e,1);
        if (eis_int(x) && fmpz_fits_si(eint_data(x)))
        {
            slong n = fmpz_get_si(eint_data(x));
            ex z = emake_int();
            if (n >= 0)
            {
                fmpz_fib_ui(eint_data(z), n);
            }
            else
            {
                fmpz_fib_ui(eint_data(z), -n);
                if (n % 2 == 0)
                    fmpz_neg(eint_data(z), eint_data(z));
            }
            return efix_int(z);
        }
        else
        {
            return ecopy(e);
        }
    }
    else if (elength(e) == 2)
	{
        return ecopy(e);
    }
	else
	{
		return _handle_message_argt(e, (1 << 0) + (2 << 8));
	}
}
