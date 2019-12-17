#include "globalstate.h"
#include "code.h"
#include "timing.h"
#include "ex_print.h"

class ZMatrix {
public:
    xfmpz _11, _12, _21, _22;

    ZMatrix() : _11((ulong)(1)), _22((ulong)(1)) {};

    ZMatrix(slong a11, slong a12, slong a21, slong a22) : _11(a11), _12(a12), _21(a21), _22(a22) {};

    ZMatrix(ZMatrix&& other) noexcept : _11(other._11), _12(other._12), _21(other._21), _22(other._22) {};

    ZMatrix& operator=(ZMatrix&& other) noexcept
    {
        if (this != &other)
        {
            fmpz_swap(_11.data, other._11.data);
            fmpz_swap(_12.data, other._12.data);
            fmpz_swap(_21.data, other._21.data);
            fmpz_swap(_22.data, other._22.data);
            _fmpz_demote(other._11.data);
            _fmpz_demote(other._12.data);
            _fmpz_demote(other._21.data);
            _fmpz_demote(other._22.data);
        }  
        return *this;  
    }

    ZMatrix(const ZMatrix& other) : _11(other._11), _12(other._12), _21(other._21), _22(other._22) {};

    ZMatrix& operator=(const ZMatrix& other)
    {
        fmpz_set(_11.data, other._11.data);
        fmpz_set(_12.data, other._12.data);
        fmpz_set(_21.data, other._21.data);
        fmpz_set(_22.data, other._22.data);
        return *this;  
    }


    void set(const ZMatrix &other)
    {
        fmpz_set(_11.data, other._11.data);
        fmpz_set(_12.data, other._12.data);
        fmpz_set(_21.data, other._21.data);
        fmpz_set(_22.data, other._22.data);
    };

    void set(xfmpz &a11, xfmpz &a12, xfmpz &a21, xfmpz &a22)
    {
        fmpz_set(_11.data, a11.data);
        fmpz_set(_12.data, a12.data);
        fmpz_set(_21.data, a21.data);
        fmpz_set(_22.data, a22.data);
    };

    void set(slong a11, slong a12, slong a21, slong a22)
    {
        fmpz_set_si(_11.data, a11);
        fmpz_set_si(_12.data, a12);
        fmpz_set_si(_21.data, a21);
        fmpz_set_si(_22.data, a22);
    };

    bool set_ex(er e);
    ex get_ex() const;
    ex get_ex_neg() const;

    // this = {{0,1},{1,-a}}.this
    void leftmul_elementary(const fmpz_t a)
    {
		fmpz_submul(_11.data, a, _21.data);
		fmpz_submul(_12.data, a, _22.data);
		fmpz_swap(_11.data, _21.data);
		fmpz_swap(_12.data, _22.data);
    }

    // this = m.this
    void leftmul(const ZMatrix & m)
    {
        xfmpz a, b, c, d;
        fmpz_mul(a.data, m._11.data, _11.data); fmpz_addmul(a.data, m._12.data, _21.data);
        fmpz_mul(b.data, m._11.data, _12.data); fmpz_addmul(b.data, m._12.data, _22.data);
        fmpz_mul(c.data, m._21.data, _11.data); fmpz_addmul(c.data, m._22.data, _21.data);
        fmpz_mul(d.data, m._21.data, _12.data); fmpz_addmul(d.data, m._22.data, _22.data);
        fmpz_swap(_11.data, a.data);
        fmpz_swap(_12.data, b.data);
        fmpz_swap(_21.data, c.data);
        fmpz_swap(_22.data, d.data);
    };

    bool is_one() const
    {
        return fmpz_is_one(_11.data) && fmpz_is_one(_22.data)
           && fmpz_is_zero(_12.data) && fmpz_is_zero(_21.data);
    }

    void one()
    {
        fmpz_one(_11.data);
		fmpz_one(_22.data);
		fmpz_zero(_12.data);
		fmpz_zero(_21.data);
    }

	void map(arb_t x, arb_t t1, arb_t t2, arb_t t3)
	{
		slong p = arb_rel_accuracy_bits(x) + FLINT_BITS;
		arb_mul_fmpz(t1, x, _11.data, p + fmpz_bits(_11.data));
		arb_add_fmpz(t2, t1, _12.data, p + fmpz_bits(_12.data));
		arb_mul_fmpz(t1, x, _21.data, p + fmpz_bits(_21.data));
		arb_add_fmpz(t3, t1, _22.data, p + fmpz_bits(_22.data));
        slong p1 = arb_rel_accuracy_bits(t2);
        slong p2 = arb_rel_accuracy_bits(t3);
		arb_div(x, t2, t3, FLINT_MAX(p1, p2) + FLINT_BITS);
	}
};


static bool try_step(fmpz_t a, arb_t y, const arb_t x, arb_t t, bool first)
{
    arb_floor(y, x, arb_rel_accuracy_bits(x) + FLINT_BITS);
    if (!arb_get_unique_fmpz(a, y))
        return false;

    if (!first && fmpz_sgn(a) <= 0)
        return false;

    arb_sub_fmpz(t, x, a, arb_rel_accuracy_bits(x) + FLINT_BITS);
    arb_inv(y, t, arb_rel_accuracy_bits(t) + FLINT_BITS);
	return true;
}


//#include <flint/profiler.h>

static void cf(std::vector<wex> & A, ZMatrix & M, arb_t x)
{
	xfmpz a;
    xarb t, y, v;
	ZMatrix N;
    slong p;
//timeit_t timer;

    M.one();

over:

    p = arb_rel_accuracy_bits(x);

    if (p < 10000)
    {
doit:

//timeit_start(timer);
        N.one();
        while (try_step(a.data, y.data, x, t.data, A.empty()))
        {
            N.leftmul_elementary(a.data);
            A.push_back(emake_int_move(a));
            arb_swap(y.data, x);
        }
        M.leftmul(N);

//timeit_stop(timer);
//flint_printf("p = %wd, do it: %wd\n", p, timer->wall);

        return;
    }
    arb_set_round(v.data, x, p/2);
    if (arb_rel_accuracy_bits(v.data) >= p - p/4)
        goto doit;

//timeit_start(timer);
    cf(A, N, v.data);
//timeit_stop(timer);
//flint_printf("p = %wd, recur: %wd\n", p, timer->wall);

    if (N.is_one())
    {
        if (try_step(a.data, y.data, x, t.data, A.empty()))
        {
            M.leftmul_elementary(a.data);
            A.push_back(emake_int_move(a));
            arb_swap(x, y.data);
            goto over;
        }
        else
        {
            return;
        }
    }

//timeit_start(timer);
    M.leftmul(N);
    N.map(x, v.data, y.data, t.data);
//timeit_stop(timer);
//flint_printf("p = %wd, mapit: %wd\n", p, timer->wall);

    goto over;
}

ex dcode_sContinuedFraction(er e)
{
//std::cout << "dcode_sContinuedFraction: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sContinuedFraction.get()));

    if (elength(e) > 2 || elength(e) < 1)
        return _handle_message_argt(e, (1 << 0) + (2 << 8));

    if (elength(e) != 1 || !eis_real(echild(e,1)))
        return ecopy(e);

    er x = echild(e,1);

    std::vector<wex> A;
	ZMatrix M;
    xarb t;
    arb_abs(t.data, ereal_data(echild(e,1)));
	cf(A, M, t.data);
    return emake_node(gs.sym_sList.copy(), A);
}


ex dcode_sChampernowneNumber(er e)
{
    if (elength(e) > 1)
        return _handle_message_argt(e, (0 << 0) + (1 << 8));

    if (elength(e) < 1)
        return ecopy(e);

    er x = echild(e,1);

    if (!eis_number(x))
        return ecopy(e);

    if (!eis_int(x) || fmpz_cmp_ui(eint_data(x), 1) <= 0)
        _gen_message(echild(e,0), "ibase", "Base `1` is not an integer greater than 1.", ecopychild(e,1));

    return ecopy(e);
}


ex ncode_sChampernowneNumber(er e, slong prec)
{
//std::cout << "ncode_sChampernowneNumber(" << prec << "): " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sChampernowneNumber.get()));

    if (elength(e) > 1)
        return ecopy(e);

    if (elength(e) == 1 && !eis_int(echild(e,1)))
        return ecopy(e);

    xfmpz B(UWORD(10));
    if (elength(e) == 1)
        fmpz_set(B.data, eint_data(echild(e,1)));

    if (fmpz_cmp_ui(B.data, 1) <= 0)
        return ecopy(e);

    xarb z, s, d, u, v;
    xfmpz a, t0, t1, t2, t3, t4, t6;

    if (!fmpz_abs_fits_ui(B.data))
	{
		// we only need one term
		fmpz_sub_ui(t0.data, B.data, 1);
		fmpz_mul(t2.data, t0.data, t0.data);
	    arb_fmpz_div_fmpz(z.data, B.data, t2.data, prec + 1); // TODO add error
	    return emake_real_move(z);
	}

    ulong b = fmpz_get_ui(B.data);
    ulong p = prec + 2 + fmpz_bits(B.data);	// target is abserror < 2^-p

    double mlog2u = (b - 1)*log2(b);

    std::vector<xfmpz> ck;
    ck.push_back(xfmpz(UWORD(1)));
    ulong k = 1;
    while (true)
    {
        k++;
        fmpz_pow_ui(t0.data, B.data, k - 1);
        fmpz_mul_ui(t0.data, t0.data, k);
        fmpz_add(t0.data, t0.data, ck[k - 1-1].data);
        if (fmpz_cmp_ui(t0.data, 1 + (p + k + 2)/mlog2u) > 0)
            break;
        ck.push_back(t0);
    }
    ulong n = k - 1;

	arb_zero_pm_one(s.data);
	arb_mul_2exp_si(s.data, s.data, -slong(p + n + 2)); // tail error
    for (k = n; k != 0; k--)
    {
        ulong w1 = 5 + p + k;
        ulong w2 = mlog2u*fmpz_get_d(ck[k - 1].data);
        ulong w = 5 + (w1 > w2 ? w1 - w2 : 0);
        flint_bitcnt_t ckbits = fmpz_bits(ck[k - 1].data);
        fmpz_pow_ui(a.data, B.data, k);
        fmpz_sub_ui(t0.data, a.data, 1);
        fmpz_mul(t1.data, t0.data, a.data);
        fmpz_add_ui(t1.data, t1.data, 1);
        fmpz_mul(t2.data, t0.data, t0.data);
		fmpz_divexact_ui(t0.data, t0.data, b - 1);
        fmpz_mul(t3.data, t0.data, t0.data);
        fmpz_mul(t0.data, a.data, B.data);
        fmpz_sub_ui(t0.data, t0.data, 1);
        fmpz_mul(t4.data, t0.data, a.data);
        fmpz_add_ui(t4.data, t4.data, 1);
		fmpz_divexact_ui(t0.data, t0.data, b - 1);
        fmpz_mul(t6.data, t0.data, t0.data);
        fmpz_mul(t0.data, t2.data, t6.data);
        fmpz_mul(t2.data, t6.data, t1.data);
        fmpz_submul(t2.data, t3.data, t4.data);
        arb_fmpz_div_fmpz(d.data, t2.data, t0.data, w);
        arb_ui_pow_ui(u.data, b, b - 1, w + ckbits);
        arb_pow_fmpz(v.data, u.data, ck[k-1].data, w + ckbits);
        arb_div(z.data, d.data, v.data, w);
        arb_add(s.data, s.data, z.data, w);
    }
	fmpz_set_ui(t2.data, b - 1);
    fmpz_mul_ui(t2.data, t2.data, b - 1);
    arb_fmpz_div_fmpz(z.data, B.data, t2.data, prec + 3);
    arb_sub(z.data, z.data, s.data, prec + 3);
    return emake_real_move(z);
}

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

    slong nn;
    if (!eis_int(e1) || !fmpz_fits_si(eint_data(e1))
                     || (nn = fmpz_get_si(eint_data(e1))) < 0)
    {
        _gen_message(echild(e,0), "intnm", nullptr, ecopy(e), emake_cint(1));
        return ecopy(e);
    }

    ulong n = nn;
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
            ex Z = emake_rat();
            fmpq_dedekind_sum(erat_data(Z), fmpq_numref(erat_data(X)), fmpq_denref(erat_data(X)));
            return ereturn_rat(Z);
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

ex dcode_sPrimeQ(er e)
{
//std::cout << "dcode_sPrimeQ: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sPrimeQ.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    e = echild(e,1);

    if (!eis_int(e))
    {
        return gs.sym_sFalse.copy();
    }

    if (!fmpz_is_probabprime(eint_data(e))
        || !fmpz_is_prime(eint_data(e)))
    {
        return gs.sym_sFalse.copy();
    }
    return gs.sym_sTrue.copy();
}

ex dcode_sEulerPhi(er e)
{
//std::cout << "dcode_sEulerPhi: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sEulerPhi.get()));

    if (elength(e) == 1)
    {
        if (eis_int(echild(e,1)))
        {
            ex Z = emake_int();
            fmpz_euler_phi(eint_data(Z), eint_data(echild(e,1)));
            return ereturn_int(Z);
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

ex dcode_sMoebiusMu(er e)
{
//std::cout << "dcode_sMoebiusMu: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sMoebiusMu.get()));

    if (elength(e) == 1)
    {
        if (eis_int(echild(e,1)))
        {
            slong z = fmpz_moebius_mu(eint_data(echild(e,1)));
            return emake_cint(z);
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
    return ereturn_int(Z);
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

struct wex_ipair_compare {
    bool operator()(const std::pair<wex, wex>& a,
                    const std::pair<wex, wex>& b) const
    {
        return fmpz_cmp(eint_data(a.first.get()), eint_data(b.first.get())) < 0;
    }
};

ex dcode_sDivisors(er e)
{
//std::cout << "dcode_sDivisors: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sDivisors.get()));

    if (elength(e) != 1)
        _handle_message_argx1(e);

    if (!eis_int(echild(e,1)))
        return ecopy(e);

    // make divisors of n
    er n = echild(e,1);
    if (fmpz_is_zero(eint_data(n)))
        return emake_node(gs.sym_sList.copy());

    xfmpz_factor fs;
    fmpz_factor(fs.data, eint_data(n));

    slong numdiv = 1, hi;
    for (slong i = 0; i < fs.data->num; i++)
    {
        umul_ppmm(hi, numdiv, numdiv, fs.data->exp[i] + 1);
        if (hi != 0 || numdiv <= 0)
        {
            _gen_message(echild(e,0), "toomany", NULL, ecopy(e));
            return ecopy(e);
        }
    }

    uex r(gs.sym_sList.get(), numdiv);
    for (slong i = 0; i < numdiv; i++)
    {
        r.push_back(emake_cint(0));
    }

    size_t aoff = 0;
    size_t boff = numdiv + 1;

    // H contains pairs {a,b} where a*b = n and a <= b
    std::set<std::pair<wex, wex>, wex_ipair_compare> H;
    H.insert(std::pair<wex, wex>(wex(emake_cint(1)), wex(ecopy(n))));
    xfmpz anext, bnext, rem;
    while (!H.empty())
    {
        aoff++;
        boff--;
        if (aoff == boff)
        {
            assert(fmpz_equal(eint_data(H.begin()->first.get()),
                              eint_data(H.begin()->second.get())));
            r.replacechild(aoff, H.begin()->first.copy());
        }
        else
        {
            assert(aoff < boff);
            r.replacechild(aoff, H.begin()->first.copy());
            r.replacechild(boff, H.begin()->second.copy());
        }
        H.erase(H.begin());
        for (slong i = 0; i < fs.data->num; i++)
        {
            fmpz_fdiv_qr(bnext.data, rem.data, eint_data(r.child(boff)), fs.data->p + i);
            if (!fmpz_is_zero(rem.data))
            {
                continue;
            }
            fmpz_mul(anext.data, eint_data(r.child(aoff)), fs.data->p + i);
            if (fmpz_cmp(anext.data, bnext.data) > 0)
            {
                continue;
            }
            H.insert(std::pair<wex, wex>(wex(emake_int_move(anext)), wex(emake_int_move(bnext))));
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
            ex Z = emake_int();
            fmpz_gcd(eint_data(Z), eint_data(x), eint_data(y));
            return ereturn_int(Z);
        }
        else if (eis_int(x) && eis_rat(y))
        {
            ex Z = emake_rat();
            _fmpq_gcd(fmpq_numref(erat_data(Z)), fmpq_denref(erat_data(Z)),
                    eint_data(x), eint_data(eget_cint(1)),
                    fmpq_numref(erat_data(y)), fmpq_denref(erat_data(y)));
            return ereturn_rat(Z);
        }
        else if (eis_rat(x) && eis_int(y))
        {
            ex Z = emake_rat();
            _fmpq_gcd(fmpq_numref(erat_data(Z)), fmpq_denref(erat_data(Z)),
                    eint_data(y), eint_data(eget_cint(1)),
                    fmpq_numref(erat_data(x)), fmpq_denref(erat_data(x)));
            return ereturn_rat(Z);
        }
        else if (eis_rat(x) && eis_rat(y))
        {
            ex Z = emake_rat();
            _fmpq_gcd(fmpq_numref(erat_data(Z)), fmpq_denref(erat_data(Z)),
                    fmpq_numref(erat_data(x)), fmpq_denref(erat_data(x)),
                    fmpq_numref(erat_data(y)), fmpq_denref(erat_data(y)));
            return ereturn_rat(Z);
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
    size_t n = elength(e);
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

    size_t n = elength(e);
    if (n == 2)
    {
        er x = echild(e,1);
        er y = echild(e,2);
        if (eis_int(x) && eis_int(y))
        {
            ex Z = emake_int();
            fmpz_lcm(eint_data(Z), eint_data(x), eint_data(y));
            return ereturn_int(Z);
        }
        else
        {
            return ecopy(e);
        }
    }
    else
    {
        xfmpz t, g;
        for (size_t i = 1; i <= n; i++)
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
