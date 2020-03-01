#include <cmath>
#include <cfloat>

#include "timing.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"
#include "flint/arith.h"
#include "flint/profiler.h"

/********************* start factor integer stuff ****************************/

#define UI_ARRAY_PRODUCT_SPLIT_CUTOFF 16

typedef struct {
  ulong fac;
  ulong pow;
  ulong nxt;
} sieve_t;

static sieve_t *sieve;
static long int sieve_size;

static void build_sieve(ulong n, sieve_t *s)
{
  ulong m, i, j, k;

  sieve_size = n;
  m = (ulong) sqrt(n);
  memset(s, 0, sizeof(sieve_t)*n/2);

  s[1/2].fac = 1;
  s[1/2].pow = 1;

  for (i=3; i<=n; i+=2) {
    if (s[i/2].fac == 0) {
      s[i/2].fac = i;
      s[i/2].pow = 1;
      if (i<=m) {
	    for (j=i*i, k=i/2; j<=n; j+=i+i, k++) {
	      if (s[j/2].fac==0) {
	        s[j/2].fac = i;
	        if (s[k].fac == i) {
	          s[j/2].pow = s[k].pow + 1;
	          s[j/2].nxt = s[k].nxt;
	        } else {
	          s[j/2].pow = 1;
	          s[j/2].nxt = k;
	        }
	      }
	    }
      }
    }
  }
}


struct ui_base_pow_pair {
    ulong base, pow;
};

// representation of an integer as prod_i base[i]^pow[i]
class ui_factor_t {
public:
    std::vector<ui_base_pow_pair> data;
    ulong length = 0;

    ulong get_fmpz_2exp(fmpz_t x) const;

    void fit_length(ulong n)
    {
        if (data.size() < n)
            data.resize(n);
    }

    std::string tostring() const
    {
        std::string s;
        bool first = true;
        const ui_base_pow_pair * fd = data.data();
        ulong fn = length;
        ulong fi;
        for (fi = 0; fi < fn; fi++)
        {
            if (!first)
                s += " * ";
            s += stdstring_tostring(fd[fi].base) + "^" + stdstring_tostring(fd[fi].pow);
            first = false;
        }
        if (first)
            s = "1";
        return s;
    }

    bool is_canonical() const
    {
        const ui_base_pow_pair * fd = data.data();
        ulong fn = length;
        ulong fi;
        for (fi = 0; fi < fn; fi++)
        {
            if (fd[fi].pow == 0)
                return false;

            if (fi > 0 && fd[fi-1].base >= fd[fi].base)
                return false;
        }
        return true;
    }
};


void ui_factor_one(ui_factor_t & f)
{
    f.length = 0;
}

void ui_factor_push_factor(ui_factor_t & f, ulong base, ulong pow)
{
    f.fit_length(f.length + 1);
    f.data[f.length] = ui_base_pow_pair{base, pow};
    f.length++;
}

/* f = base^pow */
void ui_factor_set_ui_pow_ui(ui_factor_t & f, ulong base, ulong pow)
{
    ulong fi;

    assert(base < sieve_size);

    f.data.clear();
    fi = 0;

    if ((base % 2) == 0)
    {
        ulong e = 0;
        do {
            e += pow;
            base /= 2;
        } while ((base % 2) == 0);

        f.data.push_back(ui_base_pow_pair{ulong(2), e});    
        fi++;    
    }

    for (base/=2; base>0; base = sieve[base].nxt)
    {
        f.data.push_back(ui_base_pow_pair{sieve[base].fac, sieve[base].pow*pow});
        fi++;
    }

    f.length = fi;
}


/* x should have at least 3*len limbs allocated */
static mp_size_t ui_array_product(mp_limb_t * x, const ulong * v, ulong len, ulong stride)
{
    mp_size_t xlen, ylen, zlen;
    mp_limb_t out;

    assert(0 < len);

    if (len <= UI_ARRAY_PRODUCT_SPLIT_CUTOFF)
    {
        xlen = 1;
        x[0] = v[0];
        for (ulong i = 1; i < len; i++)
        {
            out = mpn_mul_1(x, x, xlen, v[stride*i]);
            if (out != 0)
            {
                assert(xlen < len);
                x[xlen] = out;
                xlen++;
            }
        }
    }
    else
    {
        ylen = ui_array_product(x + len, v, len/2 + (len % 2), 2*stride);
        zlen = ui_array_product(x + len + ylen, v + stride, len/2, 2*stride);
        if (ylen >= zlen)
            mpn_mul(x, x + len, ylen, x + len + ylen, zlen);
        else
            mpn_mul(x, x + len + ylen, zlen, x + len, ylen);
        xlen = ylen + zlen;
        xlen -= (x[xlen - 1] == 0);
    }
    assert(xlen <= len);
    assert(xlen > 0);
    assert(x[xlen - 1] != 0);
    return xlen;
}

// product of ulong with no exponents;
class ui_product_t
{
    std::vector<ulong> v;
    ulong top = 1;

public:

    void one()
    {
        v.clear();
        top = 1;
    }

    void push(ulong b)
    {
        ulong hi, lo;
        umul_ppmm(hi, lo, top, b);
        if (hi == 0)
        {
            top = lo;
        }
        else
        {
            v.push_back(top);
            top = b;
        }
    }

    void get_fmpz(fmpz_t x)
    {
        v.push_back(top);
        ulong vlen = v.size();
        mpz_ptr X = _fmpz_promote(x);
        X->_mp_size = ui_array_product(FLINT_MPZ_REALLOC(X, 3*vlen), v.data(), vlen, 1);
        _fmpz_demote_val(x);
        top = 1;
    }
};

void _ui_factor_get_fmpz(fmpz_t x, ui_factor_t & f, ui_product_t & yy)
{
    ulong fj, fi, p;
    ulong fn = f.length;
    ui_base_pow_pair * fd = f.data.data();
    std::vector<xfmpz_t> y;
    xfmpz_t z, t;

    while (fn > 0)
    {
        yy.one();

        fj = 0;
        for (fi = 0; fi < fn; fi++)
        {
            p = fd[fi].pow;
            if (p % 2)
                yy.push(fd[fi].base);

            p = p/2;
            if (p > 0)
            {
                fd[fj].base = fd[fi].base;
                fd[fj].pow = p;
                fj++;
            }
        }
        fn = fj;
        f.length = fn;

        y.push_back(xfmpz_t());
        yy.get_fmpz(y.back().data);
    }

    if (y.empty())
    {
        fmpz_one(x);
    }
    else
    {
        while (y.size() > 1)
        {
            fmpz_mul(z.data, y.back().data, y.back().data);
            fmpz_swap(t.data, y.back().data);
            y.pop_back();
            fmpz_mul(t.data, z.data, y.back().data);
            fmpz_swap(t.data, y.back().data);
        }
        fmpz_swap(x, y.back().data);
    }
}

/* x*2^e = f */
ulong ui_factor_get_fmpz_2exp(fmpz_t x, const ui_factor_t & f)
{
    ulong fi;
    ulong fn = f.length;
    const ui_base_pow_pair * fd = f.data.data();
    ulong p, e = 0;
    xfmpz_t y, z;
    ui_product_t yy;

    ui_factor_t t;
    t.fit_length(fn);

    fi = 0;

    if (fi < fn && fd[fi].base == 2)
    {
        e = fd[fi].pow;
        fi++;
    }

    yy.one();
    ui_factor_one(t);

    for ( ; fi < fn; fi++)
    {
        p = fd[fi].pow;
        if (p % 2)
            yy.push(fd[fi].base);

        p = p/2;
        if (p > 0)
            ui_factor_push_factor(t, fd[fi].base, p);
    }

    yy.get_fmpz(y.data);

    if (t.length > 0)
    {
        _ui_factor_get_fmpz(x, t, yy);
        fmpz_mul(z.data, x, x);
        fmpz_mul(x, y.data, z.data);
    }
    else
    {
        fmpz_swap(x, y.data);
    }

    return e;
}

bool ui_factor_equal_fmpz(const ui_factor_t & f, const fmpz_t x)
{
    xfmpz_t y;
    ulong e = ui_factor_get_fmpz_2exp(y.data, f);
    fmpz_mul_2exp(y.data, y.data, e);
    return fmpz_equal(y.data, x);
}


// {f, g} = {f, g}/GCD[f, g]
void ui_factor_remove_gcd(ui_factor_t & f, ui_factor_t & g)
{
    assert(f.is_canonical());
    assert(g.is_canonical());

    ulong fi, gi, fj, gj;
    ulong fn = f.length;
    ulong gn = g.length;
    ui_base_pow_pair * fd = f.data.data();
    ui_base_pow_pair * gd = g.data.data();

    fi = gi = 0;
    fj = gj = 0;
    while (fj < fn && gj < gn)
    {
        if (fd[fj].base == gd[gj].base)
        {
            if (fd[fj].pow < gd[gj].pow)
            {
                assert(gi <= gj);
                gd[gi].base = gd[gj].base;
                gd[gi].pow = gd[gj].pow - fd[fj].pow;
                gi++;
            }
            else if (fd[fj].pow > gd[gj].pow)
            {
                assert(fi <= fj);
                fd[fi].base = fd[fj].base;
                fd[fi].pow = fd[fj].pow - gd[gj].pow;
                fi++;
            }
            fj++;
            gj++;
        }
        else if (fd[fj].base < gd[gj].base)
        {
            assert(fi <= fj);
            fd[fi].base = fd[fj].base;
            fd[fi].pow = fd[fj].pow;
            fi++;
            fj++;
        }
        else
        {
            assert(gi <= gj);
            gd[gi].base = gd[gj].base;
            gd[gi].pow = gd[gj].pow;
            gi++;
            gj++;
        }
    }

    while (fj < fn)
    {
        assert(fi <= fj);
        fd[fi].base = fd[fj].base;
        fd[fi].pow = fd[fj].pow;
        fi++;
        fj++;
    }

    while (gj < gn)
    {
        assert(gi <= gj);
        gd[gi].base = gd[gj].base;
        gd[gi].pow = gd[gj].pow;
        gi++;
        gj++;
    }

    f.length = fi;
    g.length = gi;

    assert(f.is_canonical());
    assert(g.is_canonical());
}


/* z = f*g */
static void ui_factor_mul(ui_factor_t & z, const ui_factor_t & f, const ui_factor_t & g)
{
    assert(f.is_canonical());
    assert(g.is_canonical());

    ulong fn = f.length;
    ulong gn = g.length;
    z.fit_length(fn + gn);
    const ui_base_pow_pair * fd = f.data.data();
    const ui_base_pow_pair * gd = g.data.data();
    ui_base_pow_pair * zd = z.data.data();

    ulong fi = 0, gi = 0, zi = 0;
    while (fi < fn && gi < gn)
    {
        if (fd[fi].base == gd[gi].base)
        {
            zd[zi].base = fd[fi].base;
            zd[zi].pow = fd[fi].pow + gd[gi].pow;
            fi++;
            gi++;
        }
        else if (fd[fi].base < gd[gi].base)
        {
            zd[zi].base = fd[fi].base;
            zd[zi].pow = fd[fi].pow;
            fi++;
        }
        else
        {
            zd[zi].base = gd[gi].base;
            zd[zi].pow = gd[gi].pow;
            gi++;
        }

        zi++;
    }

    while (fi < fn)
    {
        zd[zi].base = fd[fi].base;
        zd[zi].pow = fd[fi].pow;
        fi++;
        zi++;
    }

    while (gi < gn)
    {
        zd[zi].base = gd[gi].base;
        zd[zi].pow = gd[gi].pow;
        gi++;
        zi++;
    }

    z.length = zi;

    assert(z.is_canonical());
}


/**************************** start pi stuff *********************************/

/* timings in ms

     digits   | arb_const_pi | gmp-chudnov |        here |
     ---------+--------------+-------------+-------------+
      100000  |           46 |         42  |         39  |
      200000  |           96 |         93  |         86  |
      400000  |          225 |        216  |        183  |
      800000  |          534 |        482  |        414  |
at many thousands of digits:   ~10% faster,  ~25% faster than arb
     1000000  |          708 |        642  |        540  |
     2000000  |         1688 |       1533  |       1238  |
     4000000  |         4098 |       3532  |       2865  |
     8000000  |         9806 |       8396  |       6793  |
      at millions of digits:   ~16% faster,  ~40% faster than arb
*/

#define CONST_PI_SUM_SPLIT_CUTOFF 50

/*
    Set {p1, r1, q1} = sum of terms in start..stop inclusive.
    The factored number mult is 640320^3/24.

    TODO: This basecase is quite important. The small word-sized mul's are not
    the bottleneck. It is the ui_factor_ functions that want optimizations.
    Or, a completely different approach to summation ...
*/
static void pi_sum_basecase(
    fmpz_t p1, ui_factor_t & r1f, ui_factor_t & q1f,
    ulong start, ulong stop, const ui_factor_t & mult)
{
    xfmpz_t p2, r1, q1, r2, q2;
    ui_factor_t r2f, q2f;
    ui_factor_t s1, s2;
    xfmpz_t t;
    ulong A0, A1, B1;

    assert(1 <= start);
    assert(start <= stop);

    fmpz_zero(p1);
    fmpz_one(r1.data);
    fmpz_one(q1.data);
    ui_factor_one(r1f);
    ui_factor_one(q1f);

    ulong j = start;

    do {
        // q = j^3*640320^3/24
        ui_factor_set_ui_pow_ui(s1, j, 3);
        ui_factor_mul(q2f, s1, mult);
        umul_ppmm(A1, A0, 10939058860032000, j*j); // dirty FLINT_BITS = 64
        umul_ppmm(B1, A0, A0, j); A1 = B1 + A1*(j);
        fmpz_set_uiui(q2.data, A1, A0);

        // r = (6j-5)(2j-1)(6j-1)
        ui_factor_set_ui_pow_ui(r2f, 6*j - 5, 1);
        ui_factor_set_ui_pow_ui(s1, 2*j - 1, 1);
        ui_factor_mul(s2, r2f, s1);
        ui_factor_set_ui_pow_ui(s1, 6*j - 1, 1);
        ui_factor_mul(r2f, s1, s2);
        umul_ppmm(A1, A0, 6*j-1, 2*j-1);
        umul_ppmm(B1, A0, A0, 6*j-5); A1 = B1 + A1*(6*j-5);
        fmpz_set_uiui(r2.data, A1, A0);

        // p = (-1)^j(6j-5)(2j-1)(6j-1)(13591409+545140134j)
        fmpz_mul_ui(p2.data, r2.data, 13591409+545140134*j);

        assert(ui_factor_equal_fmpz(q2f, q2.data));
        assert(ui_factor_equal_fmpz(r1f, r1.data));

        fmpz_mul(t.data, p1, q2.data);
        fmpz_swap(p1, t.data);
        if (j % 2)
            fmpz_submul(p1, r1.data, p2.data);
        else
            fmpz_addmul(p1, r1.data, p2.data);

        fmpz_mul(t.data, r1.data, r2.data);
        fmpz_swap(t.data, r1.data);

        ui_factor_mul(s2, r1f, r2f);
        std::swap(r1f, s2);

        ui_factor_mul(s1, q1f, q2f);
        std::swap(q1f, s1);

    } while (++j <= stop);
}


// {p1, r1, q1} = {p1*q2 + r1*p2, r1*r2, q1*q2} / GCD[r1, q2]
static void fold(
    fmpz_t p1, ui_factor_t & r1, ui_factor_t & q1,
    fmpz_t p2, ui_factor_t & r2, ui_factor_t & q2,
    bool needr)
{
    xfmpz_t t, t1, t2;
    ui_factor_t s;

    ui_factor_remove_gcd(r1, q2);

    ulong e1 = ui_factor_get_fmpz_2exp(t1.data, q2);
    ulong e2 = ui_factor_get_fmpz_2exp(t2.data, r1);

    if (e1 >= e2)
    {
        fmpz_mul(t.data, p1, t1.data);
        fmpz_mul_2exp(t.data, t.data, e1 - e2);
        fmpz_addmul(t.data, p2, t2.data);
        fmpz_mul_2exp(t.data, t.data, e2);
    }
    else
    {
        fmpz_mul(t.data, p2, t2.data);
        fmpz_mul_2exp(t.data, t.data, e2 - e1);
        fmpz_addmul(t.data, p1, t1.data);
        fmpz_mul_2exp(t.data, t.data, e1);
    }
    fmpz_swap(p1, t.data);

    ui_factor_mul(s, q1, q2);
    std::swap(q1, s);

    if (needr)
    {
        ui_factor_mul(s, r1, r2);
        std::swap(r1, s);
    }
}

// Set {p, r, q} = sum of terms in start..stop inclusive.
static void pi_sum_split(
    fmpz_t p, ui_factor_t & r, ui_factor_t & q,
    ulong start, ulong stop, bool needr, const ui_factor_t & mult)
{
    xfmpz_t p1;
    ui_factor_t r1, q1;

    assert(start <= stop);

    ulong diff = stop - start;

    if (diff > 2*CONST_PI_SUM_SPLIT_CUTOFF)
    {
        ulong mid = diff/16*9 + start;
        pi_sum_split(p1.data, r1, q1, mid + 1, stop, true, mult);
        pi_sum_split(p, r, q, start, mid, true, mult);
        fold(p, r, q, p1.data, r1, q1, needr);
    }
    else if (diff > CONST_PI_SUM_SPLIT_CUTOFF)
    {
        ulong mid = diff/2 + start;
        pi_sum_basecase(p, r, q, start, mid, mult);
        pi_sum_basecase(p1.data, r1, q1, mid + 1, stop, mult);
        fold(p, r, q, p1.data, r1, q1, needr);
    }
    else
    {
        pi_sum_basecase(p, r, q, start, stop, mult);
    }
}

ex ncode_sPi(er e, slong prec)
{
    if (!eis_sym(e, gs.sym_sPi.get()))
        return ecopy(e);

    slong wp = prec + 3;
    ulong num_terms = ceil(prec * 0.021226729578153557) + 1;

    xfmpz_t p, q;
    ui_factor_t rf, qf, mult;
    xarb_t P, Q, U, T;

timeit_t timer;
timeit_start(timer);

    sieve_size = std::max(ulong(3*5*23*29+1), num_terms*6);
    sieve = (sieve_t *)malloc(sizeof(sieve_t)*sieve_size/2);
    build_sieve(sieve_size, sieve);

    // mult = 640320^3/24
    ui_factor_one(mult);
    ui_factor_push_factor(mult, 2, 15);
    ui_factor_push_factor(mult, 3, 2);
    ui_factor_push_factor(mult, 5, 3);
    ui_factor_push_factor(mult, 23, 3);
    ui_factor_push_factor(mult, 29, 3);
    pi_sum_split(p.data, rf, qf, 1, num_terms, false, mult);

    /* The sum of terms 1..num_terms came out in p/q. We need

                         q*640320/12
           U = ------------------------------
               (p + 13591409*q)*rsqrt(640320)
    */

    ulong qe = ui_factor_get_fmpz_2exp(q.data, qf);
    arb_set_round_fmpz(Q.data, q.data, wp);
    arb_mul_2exp_si(Q.data, Q.data, qe);
    arb_mul_ui(Q.data, Q.data, 640320/12, wp);

    fmpz_mul_ui(q.data, q.data, 13591409);
    fmpz_mul_2exp(q.data, q.data, qe);
    fmpz_add(q.data, q.data, p.data);
    arb_set_round_fmpz(P.data, q.data, wp);

    arb_rsqrt_ui(U.data, 640320, wp);
    arb_mul(T.data, P.data, U.data, wp);

    arb_div(U.data, Q.data, T.data, wp);

    free(sieve);

//std::cout << "U: " << U.tostring() << std::endl;

timeit_stop(timer);
flint_printf("new time: %wd\n", timer->wall);

    ex zzz = emake_real();
timeit_start(timer);
    arb_const_pi(ereal_data(zzz), prec);
//std::cout << "z: " << ereal_number(zzz).tostring() << std::endl;
timeit_stop(timer);
flint_printf("arb time: %wd\n", timer->wall);

    return zzz;
}

/**************************** end pi stuff ***********************************/

ex ncode_sE(er e, slong prec)
{
    if (!eis_sym(e, gs.sym_sE.get()))
        return ecopy(e);

    ex z = emake_real();
    arb_const_e(ereal_data(z), prec);
    return z;
}

ex ncode_sEulerGamma(er e, slong prec)
{
    if (!eis_sym(e, gs.sym_sEulerGamma.get()))
        return ecopy(e);

    ex z = emake_real();
    arb_const_euler(ereal_data(z), prec);
    return z;
}

ex ncode_sCatalan(er e, slong prec)
{
    if (!eis_sym(e, gs.sym_sCatalan.get()))
        return ecopy(e);

    ex z = emake_real();
    arb_const_catalan(ereal_data(z), prec);
    return z;
}

ex ncode_sKhinchin(er e, slong prec)
{
    if (!eis_sym(e, gs.sym_sKhinchin.get()))
        return ecopy(e);

    ex z = emake_real();
    arb_const_khinchin(ereal_data(z), prec);
    return z;
}

ex ncode_sGlaisher(er e, slong prec)
{
    if (!eis_sym(e, gs.sym_sGlaisher.get()))
        return ecopy(e);

    ex z = emake_real();
    arb_const_glaisher(ereal_data(z), prec);
    return z;
}

ex ncode_sGoldenRatio(er e, slong prec)
{
    if (!eis_sym(e, gs.sym_sGoldenRatio.get()))
        return ecopy(e);

    ex z = emake_real();
    arb_sqrt_ui(ereal_data(z), 5, prec);
    arb_add_ui(ereal_data(z), ereal_data(z), 1, prec);
    arb_mul_2exp_si(ereal_data(z), ereal_data(z), -1);
    return z;
}

ex ncode_sPlus(er e, slong prec)
{
    if (!ehas_head_sym(e, gs.sym_sPlus.get()))
        return ecopy(e);

    uex f(gs.sym_sPlus.get(), elength(e));

    for (size_t i = 1; i <= elength(e); i++)
    {
        f.push_back(eval_num(echild(e,i), prec + 1));
    }

    f.setnz(dcode_sPlus(f.get()));
    return f.release();
}

ex ncode_sTimes(er e, slong prec)
{
    if (!ehas_head_sym(e, gs.sym_sTimes.get()))
        return ecopy(e);

    uex f(gs.sym_sTimes.get(), elength(e));

    for (size_t i = 1; i <= elength(e); i++)
    {
        f.push_back(eval_num(echild(e,i), prec + 1));
    }

    f.setnz(dcode_sTimes(f.get()));
    return f.release();
}

ex ncode_sPower(er e, slong prec)
{
    if (!ehas_head_sym_length(e, gs.sym_sPower.get(), 2))
        return ecopy(e);

    if (eis_sym(echild(e,1), gs.sym_sE.get()))
    {
        ex t = eval_num(echild(e,2), prec + 1);
        uex g(emake_node(gs.sym_sExp.copy(), t));
        g.setnz(dcode_sExp(g.get()));
        return g.release();
    }

    uex f(gs.sym_sPower.get(), elength(e));

    for (size_t i = 1; i <= elength(e); i++)
    {
        f.push_back(eval_num(echild(e,i), prec + 1));
    }

    f.setnz(dcode_sPower(f.get()));
    return f.release();
}



ex convert_num_to_double(ex E)
{
    uex e(E);
    er X = e.get();
    switch (etype(X))
    {
        case ETYPE_INT:
        {
            return emake_double(fmpz_get_d(eint_data(X)));
        }
        case ETYPE_RAT:
        {
            return emake_double(fmpq_get_d(erat_data(X)));
        }
        case ETYPE_REAL:
        {
            return emake_double(arf_get_d(arb_midref(ereal_data(X)), ARF_RND_NEAR));
        }
        case ETYPE_CMPLX:
        {
            uex re(convert_num_to_double(ecopy(ecmplx_real(X))));
            ex im = convert_num_to_double(ecopy(ecmplx_imag(X)));
            return emake_cmplx(re.release(), im);
        }
        default:
        {
            return e.release();
        }
    }
}


ex eval_num(er e, slong prec)
{
//std::cout << "eval_num: " << ex_tostring(e) << std::endl;

    if (eis_sym(e))
    {
        // apply ncode and nvalues of e
        if (eis_sym(e, gs.sym_sPi.get()))
            return ncode_sPi(e, prec);
        if (eis_sym(e, gs.sym_sE.get()))
            return ncode_sE(e, prec);
        if (eis_sym(e, gs.sym_sEulerGamma.get()))
            return ncode_sEulerGamma(e, prec);
        if (eis_sym(e, gs.sym_sGlaisher.get()))
            return ncode_sGlaisher(e, prec);
        if (eis_sym(e, gs.sym_sCatalan.get()))
            return ncode_sCatalan(e, prec);
        if (eis_sym(e, gs.sym_sKhinchin.get()))
            return ncode_sKhinchin(e, prec);
        if (eis_sym(e, gs.sym_sPi.get()))
            return ncode_sPi(e, prec);
        if (eis_sym(e, gs.sym_sGoldenRatio.get()))
            return ncode_sGoldenRatio(e, prec);
        return ecopy(e);        
    }

    if (!eis_node(e))
    {
        switch (etype(e))
        {
            case ETYPE_INT:
            {
                xarb_t z;
                arb_set_round_fmpz(z.data, eint_data(e), prec);
                z.limit_prec(prec);
                return emake_real_move(z);
            }
            case ETYPE_RAT:
            {
                xarb_t z;
                arb_set_fmpq(z.data, erat_data(e), prec);
                z.limit_prec(prec);
                return emake_real_move(z);
            }
            case ETYPE_REAL:
            {
                xarb_t z;
                arb_set(z.data, ereal_data(e));
                z.limit_prec(prec);
                return emake_real_move(z);
            }
            default:
            {
                return ecopy(e);
            }
        }
    }

    slong cur_prec = prec + 10;

    uex f;

try_again:

std::cout << "using cur_prec = " << cur_prec << " for " << ex_tostring(e) << std::endl;

    ulong len = elength(e);
    ex h = eval_num(echild(e,0), cur_prec);
    uint32_t attr = enormalattr(etor(h));
    f.reset(nullptr);
    f.init_push_back(h, len);

    if (eis_sym(h, gs.sym_sPlus.get()))
    {
        for (size_t i = 1; i <= len; i++)
            f.push_back(ecopychild(e,i));
        f.setnz(ncode_sPlus(e, cur_prec));
    }
    else if (eis_sym(h, gs.sym_sTimes.get()))
    {
        for (size_t i = 1; i <= len; i++)
            f.push_back(ecopychild(e,i));
        f.setnz(ncode_sTimes(f.get(), cur_prec));
    }
    else if (eis_sym(h, gs.sym_sPower.get()))
    {
        for (size_t i = 1; i <= len; i++)
            f.push_back(ecopychild(e,i));
        f.setnz(ncode_sPower(f.get(), cur_prec));
    }
    else
    {
        uint32_t test = ATTR_NHoldFirst;
        for (ulong i = 1; i <= len; i++)
        {
            f.push_back((attr & test) ? ecopychild(e,i) : eval_num(echild(e,i), cur_prec));
            test = ATTR_NHoldRest;
        }

        er sh = esymbolic_head(f.child(0));

        // apply ncode and nvalues of sh
        if (eis_sym(sh, gs.sym_sChampernowneNumber.get()))
            f.setnz(ncode_sChampernowneNumber(f.get(), cur_prec));
    }

    f.set(eval(f.release()));

    if (eis_number(f.get()))
    {
        if (eis_real(f.get()) && ereal_number(f.get()).prec_bits() < prec)
        {
            cur_prec += prec - ereal_number(f.get()).prec_bits();
            if (cur_prec - prec > 2000) // TODO: $MaxExtraPrecision
                return f.release();
            else
                goto try_again;
        }
        return f.release();
    }
    else
    {
        return f.release();
    }
}


ex eval_num_limit_prec(er e, double p)
{
    if (eis_real(e))
    {
        double t = 3.3219280948873623479*p + 1;
        slong n = t;
        t -= n;
        mag_t z1, z2;
        mag_init(z1);
        mag_init(z2);
        arb_get_mag_lower(z1, ereal_data(e));
        mag_set_d_lower(z2, pow(2.0, -t));
        mag_mul_2exp_si(z2, z2, -n);
        mag_mul_lower(z1, z1, z2);
        xarb_t z;
        arb_set(z.data, ereal_data(e));
        if (mag_cmp(z1, arb_radref(z.data)))
            mag_swap(z1, arb_radref(z.data));
        return emake_real_move(z);
    }
    else
    {
        return ecopy(e);
    }
}

ex eval_num_limit_prec(er e)
{
    return convert_num_to_double(ecopy(e));
}



ex topeval_num(er e, double p) // prec in decimal
{
    uex f(eval_num(e, p * 3.3219280948873623479 + 4));
    f.setnz(eval_num_limit_prec(f.get(), p));
    return f.release();
}

ex topeval_num(er e) // machine precision
{
    uex f(eval_num(e, 53));
    f.setnz(eval_num_limit_prec(f.get()));
    return f.release();
}



ex dcode_sN(er e)
{
    if (elength(e) == 1)
    {
        return convert_num_to_double(eval_num(echild(e,1), 53));
        
    }
    else if (elength(e) == 2)
    {
        uex p(eval_num(echild(e,2), 53));
        p.set(convert_num_to_double(p.release()));
        if (eis_double(p.get()))
        {
            return topeval_num(echild(e,1), edouble_number(p.get()));
        } 
        else
        {
            _gen_message(echild(e,0), "precbd", NULL, ecopychild(e,2));
            return ecopy(e);
        }
    }
    else
    {
        return ecopy(e);
    }
}


ex dcode_sPrecision(er e)
{
//std::cout << "dcode_sPrecision: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sPrecision.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    er x = echild(e,1);

    if (!eis_number(x))
        return ecopy(e);

    if (!eis_real(x))
        return gs.const_infinity.copy();

    if (arb_contains_zero(ereal_data(x)))
        return emake_cint(0);

    ex z = emake_real();
    arb_get_rad_arb(ereal_data(z), ereal_data(x));
    arb_div(ereal_data(z), ereal_data(z), ereal_data(x), 32);
    arb_mul_2exp_si(ereal_data(z), ereal_data(z), 1);
    arb_abs(ereal_data(z), ereal_data(z));
    arb_log_base_ui(ereal_data(z), ereal_data(z), 10, 32 + fmpz_bits(ARF_EXPREF(arb_midref(ereal_data(z)))));
    arb_add_error_2exp_si(ereal_data(z), -30);
    arb_neg(ereal_data(z), ereal_data(z));
    return efix_real(z);
}

ex dcode_sAccuracy(er e)
{
//std::cout << "dcode_sAccuracy: " << ex_tostring_full(e) << std::endl; 
    assert(ehas_head_sym(e, gs.sym_sAccuracy.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    er x = echild(e,1);

    if (!eis_number(x))
        return ecopy(e);

    if (!eis_real(x))
        return gs.const_infinity.copy();

    ex z = emake_real();
    arb_get_rad_arb(ereal_data(z), ereal_data(x));
    arb_mul_2exp_si(ereal_data(z), ereal_data(z), 1);
    arb_abs(ereal_data(z), ereal_data(z));
    arb_log_base_ui(ereal_data(z), ereal_data(z), 10, 32 + fmpz_bits(ARF_EXPREF(arb_midref(ereal_data(z)))));
    arb_add_error_2exp_si(ereal_data(z), -30);
    arb_neg(ereal_data(z), ereal_data(z));
    return efix_real(z);
}


ex ex_add(ex a, ex b)
{
    wex t(emake_node(gs.sym_sPlus.copy(), a, b));
    return dcode_sPlus(t.get());
}

ex ex_add(er a, er b)
{
    wex t(emake_node(gs.sym_sPlus.copy(), ecopy(a), ecopy(b)));
    return dcode_sPlus(t.get());
}

ex ex_mul(ex a, ex b)
{
    wex t(emake_node(gs.sym_sTimes.copy(), a, b));
    return dcode_sTimes(t.get());
}

ex ex_mul(ex a, ex b, ex c)
{
    wex t(emake_node(gs.sym_sTimes.copy(), a, b, c));
    return dcode_sTimes(t.get());
}

ex ex_mul(ex a, ex b, ex c, ex d)
{
    wex t(emake_node(gs.sym_sTimes.copy(), a, b, c, d));
    return dcode_sTimes(t.get());
}

ex ex_mul(er a, er b)
{
    wex t(emake_node(gs.sym_sTimes.copy(), ecopy(a), ecopy(b)));
    return dcode_sTimes(t.get());
}

ex ex_mul_si(ex a, slong b)
{
    wex t(emake_node(gs.sym_sTimes.copy(), a, emake_int_si(b)));
    return dcode_sTimes(t.get());
}

ex ex_sub(ex a, ex b)
{
    wex t(emake_node(gs.sym_sMinus.copy(), a, b));
    return dcode_sMinus(t.get());
}

ex ex_sub(er a, er b)
{
    wex t(emake_node(gs.sym_sMinus.copy(), ecopy(a), ecopy(b)));
    return dcode_sMinus(t.get());
}

ex ex_div(ex a, ex b)
{
    wex t(emake_node(gs.sym_sDivide.copy(), a, b));
    return dcode_sDivide(t.get());
}

ex ex_div(er a, er b)
{
    wex t(emake_node(gs.sym_sDivide.copy(), ecopy(a), ecopy(b)));
    return dcode_sDivide(t.get());
}


ex ex_pow(ex a, ex b)
{
    wex t(emake_node(gs.sym_sPower.copy(), a, b));
    return dcode_sPower(t.get());
}

ex ex_pow(er a, er b)
{
    wex t(emake_node(gs.sym_sPower.copy(), ecopy(a), ecopy(b)));
    return dcode_sPower(t.get());
}


ex ex_floor(ex A)
{
    uex a(A);
    if (eis_int(A))
    {
        return a.release();
    }
    else if (eis_rat(A))
    {
        ex z = emake_int();
        fmpz_fdiv_q(eint_data(z), fmpq_numref(erat_data(A)), fmpq_denref(erat_data(A)));
        return z;
    }
    return gs.sym_sNull.copy();
}
