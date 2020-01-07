#include "globalstate.h"
#include "code.h"
#include "timing.h"
#include "ex_print.h"
#include "ex_cont.h"

ex dcode_sIntegerQ(er e)
{
//std::cout << "dcode_sIntegerQ: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sIntegerQ.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    return emake_boole(eis_int(echild(e,1)));
}


ex dcode_sCyclotomic(er e)
{
//std::cout << "dcode_sCyclotomic: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sCyclotomic.get()));

    if (elength(e) != 2)
        return _handle_message_argx2(e);

    er e1 = echild(e,1);
    if (!eis_number(e1))
        return ecopy(e);

    if (!eis_intnm(e1))
    {
        _gen_message(echild(e,0), "intnm", nullptr, ecopy(e), emake_cint(1));
        return ecopy(e);
    }

    ulong n = eintm_get(e1);
    fmpz * coeffs;
    n_factor_t factors;
    ulong s, phi;
    er x = echild(e,2);

    if (n <= 2)
    {
        return (n == 0) ? emake_cint(1)
                        : emake_node(gs.sym_sPlus.copy(), emake_cint((n == 1) ? -1 : 1), ecopy(x));
    }

    /* Write n = q * s where q is squarefree, compute the factors of q,
      and compute phi(s) which determines the degree of the polynomial. */
    n_factor_init(&factors);
    n_factor(&factors, n, 1);
    s = phi = 1;
    for (slong i = 0; i < factors.num; i++)
    {
        phi *= factors.p[i] - 1;
        while (factors.exp[i] > 1)
        {
            s *= factors.p[i];
            factors.exp[i]--;
        }
    }

    if (phi == 1)
    {
        ex xpow = emake_node(gs.sym_sPower.copy(), ecopy(x), emake_int_ui(s));
        return emake_node(gs.sym_sPlus.copy(), emake_cint(1), xpow);
    }

    assert((phi%2) == 0);

    coeffs = _fmpz_vec_init(phi/2 + 1);
    /* Evaluate lower half of Phi_s(x) */
    _fmpz_poly_cyclotomic(coeffs, n / s, factors.p, factors.num, phi);

    size_t length = 0;
    for (size_t i = 0; i < phi/2; i++)
        length += !fmpz_is_zero(coeffs + i);
    length += length + !fmpz_is_zero(coeffs + phi/2);

    uex r(gs.sym_sPlus.get(), length);
    assert(fmpz_is_one(coeffs + 0));
    r.push_back(emake_cint(1));
    for (size_t i = 1; i <= phi/2; i++)
    {
        if (!fmpz_is_zero(coeffs + i))
        {
            ex xpow = emake_node(gs.sym_sPower.copy(), ecopy(x), emake_int_ui(s*i));
            if (fmpz_is_one(coeffs + i))
            {
                r.push_back(xpow);
            }
            else
            {
                r.push_back(emake_node(gs.sym_sTimes.copy(), emake_int_move(coeffs + i), xpow));
                fmpz_set_ui(coeffs + i, 2);
            }
        }
    }
    size_t j = length/2;
    assert(j = elength(r.get()) - !fmpz_is_zero(coeffs + phi/2));
    for (size_t i = phi/2 + 1; i <= phi; i++)
    {
        if (!fmpz_is_zero(coeffs + phi - i))
        {
            ex xpow = emake_node(gs.sym_sPower.copy(), ecopy(x), emake_int_ui(s*i));
            if (fmpz_is_one(coeffs + phi - i))
                r.push_back(xpow);
            else
                r.push_back(emake_node(gs.sym_sTimes.copy(), ecopychild(r.get(), j, 1), xpow));
            j--;
        }
    }

    assert(j == 0);

    free(coeffs); // all have been demoted  TODO classy wrapper

    return r.release();
}

ex dcode_sDedekindSum(er e)
{
//std::cout << "dcode_sDedekindSum: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sDedekindSum.get()));

    if (elength(e) == 1)
    {
        er X = echild(e,1);
        if (eis_int(X))
        {
            return emake_cint(0);
        }
        else if (eis_rat(X))
        {
            ex z = emake_rat();
            fmpq_dedekind_sum(erat_data(z), fmpq_numref(erat_data(X)), fmpq_denref(erat_data(X)));
            return efix_rat(z);
        }
        else
        {
            return ecopy(e);
        }
    }
    else
    {
        return _handle_message_argx1(e);
    }
}

ex dcode_sPrimeQ(er e)
{
//std::cout << "dcode_sPrimeQ: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sPrimeQ.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    er x = echild(e,1);
    return emake_boole(eis_int(x) && fmpz_is_probabprime(eint_data(x))
                                  && fmpz_is_prime(eint_data(x)));
}

ex dcode_sEulerPhi(er e)
{
//std::cout << "dcode_sEulerPhi: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sEulerPhi.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);
    er x = echild(e,1);

    if (eis_int(x))
    {
        ex Z = emake_int();
        fmpz_euler_phi(eint_data(Z), eint_data(x));
        return efix_int(Z);
    }
    else
    {
        return ecopy(e);
    }
}

ex dcode_sMoebiusMu(er e)
{
//std::cout << "dcode_sMoebiusMu: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sMoebiusMu.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);
    er x = echild(e,1);

    if (eis_int(x))
    {
        slong z = fmpz_moebius_mu(eint_data(x));
        return emake_cint(z);
    }
    else
    {
        return ecopy(e);
    }
}

ex dcode_sNextPrime(er e)
{
//std::cout << "dcode_sNextPrime: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sNextPrime.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    er x = echild(e,1);
    if (!eis_int(x))
        return ecopy(e);

    ex Z = emake_int();
    fmpz_nextprime(eint_data(Z), eint_data(x), 1);
    return efix_int(Z);
}

ex dcode_sFactorInteger(er e)
{
//std::cout << "dcode_sFactorInteger: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sFactorInteger.get()));

    if (elength(e) == 1)
    {
        if (eis_int(echild(e,1)))
        {
            fmpz_factor_t fs;
            fmpz_factor_init(fs);
            fmpz_factor(fs, eint_data(echild(e,1)));
            uex v(gs.sym_sList.get(), fs->num);
            for (slong i = 0; i < fs->num; i++)
            {
                v.push_back(emake_node(gs.sym_sList.copy(),
                                             emake_int_copy(fs->p + i),
                                             emake_int_ui(fs->exp[i])));
            }
            fmpz_factor_clear(fs);
            return v.release();
        }
        else if (eis_rat(echild(e,1)))
        {
            fmpz_factor_t fs1, fs2;
            fmpz_factor_init(fs1);
            fmpz_factor_init(fs2);
            fmpz_factor(fs1, fmpq_numref(erat_data(echild(e,1))));
            fmpz_factor(fs2, fmpq_denref(erat_data(echild(e,1))));
            uex v(gs.sym_sList.get(), fs1->num + fs2->num);
            slong i1 = 0;
            slong i2 = 0;
            while (i1 < fs1->num || i2 < fs2->num)
            {
                ex prime, exp;
                if (i1 < fs1->num && (i2 >= fs2->num || fmpz_cmp(fs1->p + i1, fs2->p + i2) < 0))
                {
                    prime = emake_int_copy(fs1->p + i1);
                    exp = emake_int_si(fs1->exp[i1]);
                    i1++;
                }
                else
                {
                    prime = emake_int_copy(fs2->p + i2);
                    exp = emake_int_si(-fs2->exp[i2]);
                    i2++;
                }
                v.push_back(emake_node(gs.sym_sList.copy(), prime, exp));
            }
            fmpz_factor_clear(fs1);
            fmpz_factor_clear(fs2);
            return v.release();
        }
        else
        {
            return ecopy(e);
        }
    }
    else
    {
        return ecopy(e);
    }
}
/*
struct wex_ipair_compare {
    bool operator()(const std::pair<wex, wex>& a,
                    const std::pair<wex, wex>& b) const
    {
        return fmpz_cmp(eint_data(a.first.get()), eint_data(b.first.get())) < 0;
    }
};
*/
struct _ipair_compare {
    bool operator()(const std::pair<xfmpz, xfmpz>& a,
                    const std::pair<xfmpz, xfmpz>& b) const
    {
        return fmpz_cmp(a.first.data, b.first.data) < 0;
    }
};


ex dcode_sDivisors(er e)
{
//std::cout << "dcode_sDivisors: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sDivisors.get()));

    if (elength(e) != 1)
        _handle_message_argx1(e);
    er n = echild(e,1);

    if (!eis_int(n))
        return ecopy(e);

    // make divisors of n
    if (fmpz_is_zero(eint_data(n)))
        return emake_node(gs.sym_sList.copy());

    xfmpz_factor fs;
    fmpz_factor(fs.data, eint_data(n));

    ulong numdiv = 1;
    for (ulong i = 0; i < fs.data->num; i++)
    {
        if (UMUL(numdiv, numdiv, fs.data->exp[i] + 1))
        {
            _gen_message(echild(e,0), "toomany", NULL, ecopy(e));
            return ecopy(e);
        }
    }

    uex r(emake_parray_rank1(0, numdiv));
    fmpz * R = eparray_int_data(r.get());

    std::set<std::pair<xfmpz, xfmpz>, _ipair_compare> H;
    H.insert(std::pair<xfmpz, xfmpz>(xfmpz(ulong(1)), xfmpz(eint_data(n))));
    xfmpz anext, bnext, rem;

    ulong aoff = 0;
    ulong boff = numdiv - 1;
    while (!H.empty())
    {
        if (aoff >= boff)
        {
            assert(aoff == boff);
            assert(fmpz_equal(H.begin()->first.data, H.begin()->second.data));
            fmpz_swap(R + aoff, (fmpz*)H.begin()->first.data);
            break;
        }
        else
        {
            assert(aoff < boff);
            fmpz_swap(R + aoff++, (fmpz*)H.begin()->first.data);
            fmpz_swap(R + boff--, (fmpz*)H.begin()->second.data);
            if (aoff > boff)
                break;
        }
        H.erase(H.begin());
        for (ulong i = 0; i < fs.data->num; i++)
        {
            fmpz_fdiv_qr(bnext.data, rem.data, R + boff + 1, fs.data->p + i);
            if (!fmpz_is_zero(rem.data))
                continue;
            fmpz_mul(anext.data, R + aoff - 1, fs.data->p + i);
            if (fmpz_cmp(anext.data, bnext.data) > 0)
                continue;
            H.insert(std::pair<xfmpz, xfmpz>(xfmpz(std::move(anext)), xfmpz(std::move(bnext))));
        }
    }

    return r.release();
}

ex dcode_sGCD(er e)
{
//std::cout << "dcode_sGCD: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sGCD.get()));

    size_t n = elength(e);
    if (n == 2)
    {
        er x = echild(e,1);
        er y = echild(e,2);
        if (eis_int(x) && eis_int(y))
        {
            ex z = emake_int();
            fmpz_gcd(eint_data(z), eint_data(x), eint_data(y));
            return efix_int(z);
        }
        else if (eis_int(x) && eis_rat(y))
        {
            ex z = emake_rat();
            _fmpq_gcd(fmpq_numref(erat_data(z)), fmpq_denref(erat_data(z)),
                    eint_data(x), eint_data(eget_cint(1)),
                    fmpq_numref(erat_data(y)), fmpq_denref(erat_data(y)));
            return efix_rat(z);
        }
        else if (eis_rat(x) && eis_int(y))
        {
            ex z = emake_rat();
            _fmpq_gcd(fmpq_numref(erat_data(z)), fmpq_denref(erat_data(z)),
                    eint_data(y), eint_data(eget_cint(1)),
                    fmpq_numref(erat_data(x)), fmpq_denref(erat_data(x)));
            return efix_rat(z);
        }
        else if (eis_rat(x) && eis_rat(y))
        {
            ex z = emake_rat();
            _fmpq_gcd(fmpq_numref(erat_data(z)), fmpq_denref(erat_data(z)),
                    fmpq_numref(erat_data(x)), fmpq_denref(erat_data(x)),
                    fmpq_numref(erat_data(y)), fmpq_denref(erat_data(y)));
            return efix_rat(z);
        }
        else
        {
            return ecopy(e);
        }
    }
    else
    {
        xfmpq t, g(0,1);
        for (size_t i = 1; i <= n; i++)
        {
            er x = echild(e,i);
            if (eis_int(x))
            {
                _fmpq_gcd(fmpq_numref(t.data), fmpq_denref(t.data),
                    fmpq_numref(g.data), fmpq_denref(g.data),
                    eint_data(x), eint_data(eget_cint(1)));
            }
            else if (eis_rat(x))
            {
                _fmpq_gcd(fmpq_numref(t.data), fmpq_denref(t.data),
                    fmpq_numref(g.data), fmpq_denref(g.data),
                    fmpq_numref(erat_data(x)), fmpq_denref(erat_data(x)));
            }
            else
            {
                return ecopy(e);
            }
            fmpq_swap(t.data, g.data);
        }
        return emake_rat_move(g);
    }
    return ecopy(e);
}

ex dcode_sExtendedGCD(er e)
{
    ulong n = elength(e);
    if (n == 2)
    {
        if (eis_int(echild(e,1)) && eis_int(echild(e,2)))
        {
            uex g(emake_int()), a(emake_int()), b(emake_int());
            fmpz_zero(eint_data(a.get())); // xgcd may leave output undefined
            fmpz_zero(eint_data(b.get())); //
            fmpz_xgcd(eint_data(g.get()), eint_data(a.get()), eint_data(b.get()), eint_data(echild(e,1)), eint_data(echild(e,2)));
            ex t = emake_node(gs.sym_sList.copy(), a.release(), b.release());
            return emake_node(gs.sym_sList.copy(), g.release(), t);
        }
        else
        {
            return ecopy(e);
        }
    }
    else
    {
        return ecopy(e);
    }
}

ex dcode_sLCM(er e)
{
//std::cout << "dcode_sLCM: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sLCM.get()));

    ulong n = elength(e);
    if (n == 2)
    {
        er x = echild(e,1);
        er y = echild(e,2);
        if (eis_int(x) && eis_int(y))
        {
            ex Z = emake_int();
            fmpz_lcm(eint_data(Z), eint_data(x), eint_data(y));
            return efix_int(Z);
        }
        else
        {
            return ecopy(e);
        }
    }
    else
    {
        xfmpz t, g;
        for (ulong i = 1; i <= n; i++)
        {
            er x = echild(e,i);
            if (eis_int(x))
            {
                fmpz_lcm(t.data, g.data, eint_data(x));
            }
            else
            {
                return ecopy(e);
            }
            fmpz_swap(t.data, g.data);
        }
        return emake_int_move(g);
    }
    return ecopy(e);
}
