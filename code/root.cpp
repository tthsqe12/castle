#include <cmath>
#include <cfloat>

#include "uex.h"
#include "timing.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"
#include "polynomial.h"
#include "flint/fmpz_poly.h"
#include "arb_fmpz_poly.h"
#include "flint/profiler.h"


enum Algo {Algo_interpolate, Algo_pgcd/*, Algo_newton*/};
static const Algo algo = Algo_pgcd;

//*********************** ZZ[x] *************************
class rfmpz_poly_t {
public:

	typedef xfmpz_poly_t elem_t;

	int is_zero(const xfmpz_poly_t & a)
	{
	    return fmpz_poly_is_zero(a.data);
	}

	void zero(xfmpz_poly_t & a)
	{
	    fmpz_poly_zero(a.data);
	}

	void swap(xfmpz_poly_t & a, xfmpz_poly_t & b)
	{
	    fmpz_poly_swap(a.data, b.data);
	}

	void set(xfmpz_poly_t & a, const fmpz_t b)
	{
	    fmpz_poly_set_fmpz(a.data, b);
	}

	void set(xfmpz_poly_t & a, const xfmpz_poly_t & b)
	{
	    fmpz_poly_set(a.data, b.data);
	}

	void neg(xfmpz_poly_t & a, const xfmpz_poly_t & b)
	{
	    fmpz_poly_neg(a.data, b.data);
	}

	void add(xfmpz_poly_t & a, const xfmpz_poly_t & b, const xfmpz_poly_t & c)
	{
	    fmpz_poly_add(a.data, b.data, c.data);
	}

	void sub(xfmpz_poly_t & a, const xfmpz_poly_t & b, const xfmpz_poly_t & c)
	{
	    fmpz_poly_sub(a.data, b.data, c.data);
	}

	void mul(xfmpz_poly_t & a, const xfmpz_poly_t & b, const xfmpz_poly_t & c)
	{
	    fmpz_poly_mul(a.data, b.data, c.data);
	}

	void pow_ui(xfmpz_poly_t & a, const xfmpz_poly_t & b, ulong c)
	{
	    fmpz_poly_pow(a.data, b.data, c);
	}

	void divrem(xfmpz_poly_t & a, xfmpz_poly_t & r, const xfmpz_poly_t & b, const xfmpz_poly_t & c)
	{
	    fmpz_poly_divrem(a.data, r.data, b.data, c.data);
	}

	void divexact(xfmpz_poly_t & a, const xfmpz_poly_t & b, const xfmpz_poly_t & c)
	{
		#ifndef NDEBUG
		    xfmpz_poly_t r;
		    fmpz_poly_divrem(a.data, r.data, b.data, c.data);
		    assert(r.data->length == 0);
		#else
		    fmpz_poly_div(a.data, b.data, c.data);
		#endif
	}
};

//************************** ZZ[[x]] *************************
class xfmpz_poly_series_t {
public:
    xfmpz_poly_t terms;
    slong start;
    slong prec;

    bool is_canonical() const
    {
        if (prec < 0 || terms.data->length > prec)
            return false;

        if (terms.data->length <= 0)
            return true;

        if (fmpz_is_zero(terms.data->coeffs + 0))
            return false;

        return true;
    };

    slong absolute_prec() const
    {
        return start + prec;
    }

    void normalize()
    {
        assert(terms.data->length <= prec);
        while (terms.data->length > 0 && fmpz_is_zero(terms.data->coeffs + 0))
        {
            fmpz_poly_shift_right(terms.data, terms.data, 1);
            prec -= 1;
            start += 1;
        }
    };

    std::string tostring() const
    {
        std::string s = "#^" + stdstring_to_string(start) + "*(";
        s += terms.tostring() + " + O(#^" + stdstring_to_string(prec) + "))";
        return s;
    }
};

class rfmpz_poly_series_t {
public:

	slong prec;

	rfmpz_poly_series_t(slong prec_) : prec(prec_) {}

    void set_prec(slong prec_)
    {
        prec = prec_;
    }

	typedef xfmpz_poly_series_t elem_t;

	int is_zero(const xfmpz_poly_series_t & a)
	{
        assert(a.is_canonical());

        if (!fmpz_poly_is_zero(a.terms.data))
            return false;

        // TODO: exact representation so we throw much less often
        throw exception_arithmetic(17);

        return true;
	}

	void zero(xfmpz_poly_series_t & a)
	{
        a.start = 0;
        a.prec = 2*prec;
	    fmpz_poly_zero(a.terms.data);
	}

	void swap(xfmpz_poly_series_t & a, xfmpz_poly_series_t & b)
	{
	    fmpz_poly_swap(a.terms.data, b.terms.data);
        std::swap(a.start, b.start);
        std::swap(a.prec, b.prec);
	}

	void set(xfmpz_poly_series_t & a, const xfmpz_poly_series_t & b)
	{
	    fmpz_poly_set(a.terms.data, b.terms.data);
        a.start = b.start;
        a.prec = b.prec;
	}

	void set(xfmpz_poly_series_t & a, const fmpz_t b)
	{
	    fmpz_poly_set_fmpz(a.terms.data, b);
        a.start = 0;
        a.prec = prec;
	}

	void set(xfmpz_poly_series_t & a, const xfmpz_poly_t & b)
	{
	    fmpz_poly_set(a.terms.data, b.data);
        a.start = 0;
        while (a.terms.data->length > 0 && fmpz_is_zero(a.terms.data->coeffs + 0))
        {
            fmpz_poly_shift_right(a.terms.data, a.terms.data, 1);
            a.start += 1;
        }
        a.prec = prec;
        assert(a.is_canonical());
	}

	void neg(xfmpz_poly_series_t & a, const xfmpz_poly_series_t & b)
	{
	    fmpz_poly_neg(a.terms.data, b.terms.data);
        a.start = b.start;
        a.prec = b.prec;
	}

	void add(xfmpz_poly_series_t & a, const xfmpz_poly_series_t & b, const xfmpz_poly_series_t & c)
	{
        assert(b.is_canonical());
        assert(c.is_canonical());

        slong bs = b.start;
        slong cs = c.start;
        slong bp = b.prec;
        slong cp = c.prec;
        const fmpz_poly_struct * bt = b.terms.data;
        const fmpz_poly_struct * ct = c.terms.data;

        slong as = 0;
        if (bs > cs)
        {
            std::swap(bs, cs);
            std::swap(bp, cp);
            std::swap(bt, ct);
        }

        as = bs;
        cs -= bs;
        slong ap = std::min(bp, cp + cs);

        if (cs > 0)
        {
            fmpz_poly_t t;
            fmpz_poly_init(t);
            fmpz_poly_shift_left(t, ct, cs);
    	    fmpz_poly_add_series(a.terms.data, bt, t, ap);
            fmpz_poly_clear(t);
        }
        else
        {
    	    fmpz_poly_add_series(a.terms.data, bt, ct, ap);
        }
        a.start = as;
        a.prec = ap;
        a.normalize();
        assert(a.is_canonical());

	}

	void sub(xfmpz_poly_series_t & a, const xfmpz_poly_series_t & b, const xfmpz_poly_series_t & c)
	{
        assert(b.is_canonical());
        assert(c.is_canonical());
        bool neg = false;
        slong bs = b.start;
        slong cs = c.start;
        slong bp = b.prec;
        slong cp = c.prec;
        const fmpz_poly_struct * bt = b.terms.data;
        const fmpz_poly_struct * ct = c.terms.data;

        slong as = 0;
        if (bs > cs)
        {
            neg = true;
            std::swap(bs, cs);
            std::swap(bp, cp);
            std::swap(bt, ct);
        }

        as = bs;
        cs -= bs;
        slong ap = std::min(bp, cp + cs);

        if (cs > 0)
        {
            fmpz_poly_t t;
            fmpz_poly_init(t);
            fmpz_poly_shift_left(t, ct, cs);
            if (neg)
        	    fmpz_poly_sub_series(a.terms.data, t, bt, ap);
            else
        	    fmpz_poly_sub_series(a.terms.data, bt, t, ap);
            fmpz_poly_clear(t);
        }
        else
        {
            if (neg)
        	    fmpz_poly_sub_series(a.terms.data, ct, bt, ap);
            else
        	    fmpz_poly_sub_series(a.terms.data, bt, ct, ap);
        }

        a.start = as;
        a.prec = ap;
        a.normalize();
        assert(a.is_canonical());
	}

	void mul(xfmpz_poly_series_t & a, const xfmpz_poly_series_t & b, const xfmpz_poly_series_t & c)
	{
        assert(b.is_canonical());
        assert(c.is_canonical());
        slong ap = std::min(b.prec, c.prec);
        fmpz_poly_mullow(a.terms.data, b.terms.data, c.terms.data, ap);
        a.prec = ap;
        if (add_si_checked(a.start, b.start, c.start))
            throw exception_arithmetic(17);
        a.normalize();
        assert(a.is_canonical());
	}

	void pow_ui(xfmpz_poly_series_t & a, const xfmpz_poly_series_t & b, ulong c)
	{
        assert(b.is_canonical());
        fmpz_poly_pow_trunc(a.terms.data, b.terms.data, c, b.prec);        
        a.prec = b.prec;
        if (mul_si_checked(a.start, b.start, c))
            throw exception_arithmetic(17);
        a.normalize();
        assert(a.is_canonical());
	}

	void divrem(xfmpz_poly_series_t & a, xfmpz_poly_series_t & r, const xfmpz_poly_series_t & b, const xfmpz_poly_series_t & c)
	{
		assert(false);
	}

	void divexact(xfmpz_poly_series_t & a, const xfmpz_poly_series_t & b, const xfmpz_poly_series_t & c)
	{
        assert(b.is_canonical());
        assert(c.is_canonical());
        slong ap = std::min(b.prec, c.prec);
        fmpz_poly_div_series(a.terms.data, b.terms.data, c.terms.data, ap);
        a.prec = ap;
        if (sub_si_checked(a.start, b.start, c.start))
            throw exception_arithmetic(17);
        a.normalize();
        assert(a.is_canonical());
	}
};

// ************** R[y]: used for ZZ[x][y] and ZZ[[x]][y] ****************
template <class X> class sparse_poly {
public:
    std::vector<X> coeffs;
    std::vector<ulong> exps;
    slong length;

    void fit_length(slong alloc)
    {
        if (coeffs.size() < alloc)
            coeffs.resize(alloc);
        if (exps.size() < alloc)
            exps.resize(alloc);
    }

    void swap(sparse_poly<X> & a)
    {
        std::swap(coeffs, a.coeffs);
        std::swap(exps, a.exps);
        std::swap(length, a.length);
    }

    std::string tostring() {
        std::string s;
        bool first = true;
        for (slong i = 0; i < length; i++)
        {
            if (!first)
                s.append(" + ");
            s.push_back('(');
            s.append(coeffs[i].tostring());
            s.append(")*x^");
            s += stdstring_to_string(exps[i]);
            first = false;
        }
        if (first)
            s = "0";
        return s;
    }

};


template <class R>
void set_univar(
    sparse_poly<typename R::elem_t> & z,
    const xfmpz_poly_t & a,
    R & r)
{
    fmpz * a_coeffs = a.data->coeffs;
    slong i = a.data->length;
    z.fit_length(i);
    z.length = 0;
    for (i--; i >= 0; i--)
    {
        if (fmpz_is_zero(a_coeffs + i))
            continue;
        r.set(z.coeffs[z.length], a_coeffs + i);
        z.exps[z.length] = i;
        z.length++; // TODO: proper length
    }
}

template <class R>
void set_univar_shift(
    sparse_poly<typename R::elem_t> & z,
    xfmpz_poly_t a, // clobbered
    slong dir,
    R & r)
{
    z.fit_length(a.data->length);
    z.length = a.data->length;
    if (z.length <= 0)
        return;
    
    slong j = 0;
    slong i = z.length - 1; // TODO: proper length
    r.set(z.coeffs[i], a);
    z.exps[i] = j;
    for (i--; i >= 0; i--)
    {
        z.exps[i] = ++j;
        fmpz_poly_derivative(a.data, a.data);
        fmpz_poly_scalar_divexact_si(a.data, a.data, dir*j);
        r.set(z.coeffs[i], a);
    }
}

void set_univar_scale(sparse_poly<xfmpz_poly_t> & t, const xfmpz_poly_t a, slong dir)
{
    t.fit_length(a.data->length);
    fmpz * a_coeffs = a.data->coeffs;
    slong adeg = a.data->length - 1;
    t.length = 0;
    for (slong j = adeg; j >= 0; j--)
    {
        slong i = dir > 0 ? j : adeg - j;
        if (fmpz_is_zero(a_coeffs + i))
            continue;
        fmpz_poly_zero(t.coeffs[t.length].data);
        fmpz_poly_set_coeff_fmpz(t.coeffs[t.length].data, i, a_coeffs + i);
        t.exps[t.length] = j;
        t.length++;
    }
}

void set(sparse_poly<xfmpz_poly_t> & t, const sparse_poly<xfmpz_poly_t> & a)
{
    t.fit_length(a.length);
    for (slong i = a.length - 1; i >= 0; i--)
    {
        t.exps[i] = a.exps[i];
        fmpz_poly_set(t.coeffs[i].data, a.coeffs[i].data);
    }
    t.length = a.length;
}


/*
    A = prem(A, -B)
    C is used for working space
*/
template <class R>
void prem(
	sparse_poly<typename R::elem_t> & A,
	sparse_poly<typename R::elem_t> & B,
	sparse_poly<typename R::elem_t> & C,
	R & r)
{
    slong a_len, b_len, c_len;
    slong a_deg, b_deg;
    ulong * a_exp, * b_exp, * c_exp;
    typename R::elem_t * a_coeff, * b_coeff, * c_coeff;
    slong i, j, delta, delta_org;
    typename R::elem_t u, v;

//std::cout << "prem called" << std::endl;
//std::cout << "A: " << A.tostring() << std::endl;
//std::cout << "B: " << B.tostring() << std::endl;

    assert(A.length > 0);
    assert(B.length > 0);
    assert(A.exps[0] >= B.exps[0]);

    delta_org = A.exps[0] - B.exps[0] + 1;

    A.fit_length(A.length + B.exps[0]);
    C.fit_length(A.length + B.exps[0]);

    b_len = B.length;
    b_deg = B.exps[0];
    b_exp = B.exps.data();
    b_coeff = B.coeffs.data();

looper:

    a_len = A.length;
    a_deg = A.exps[0];
    a_exp = A.exps.data();
    a_coeff = A.coeffs.data();

    c_exp = C.exps.data();
    c_coeff = C.coeffs.data();

    delta = a_deg - b_deg;

    if (a_len == 0 || delta < 0)
        goto done;

    c_len = 0;
    i = 1;
    j = 1;
    while (i < a_len || j < b_len)
    {
        if (i < a_len && (j >= b_len || a_exp[i] > b_exp[j] + delta))
        {
            r.mul(c_coeff[c_len], a_coeff[i], b_coeff[0]);
            r.neg(c_coeff[c_len], c_coeff[c_len]);
            c_exp[c_len++] = a_exp[i++];
        }
        else if (j < b_len && (i >= a_len || b_exp[j] + delta > a_exp[i]))
        {
            r.mul(c_coeff[c_len], a_coeff[0], b_coeff[j]);
            c_exp[c_len++] = b_exp[j++] + delta;
        }
        else
        {
            assert(i < a_len && j < b_len && a_exp[i] == b_exp[j] + delta);
            r.mul(u, a_coeff[i], b_coeff[0]);
            r.mul(v, a_coeff[0], b_coeff[j]);
            r.sub(c_coeff[c_len], v, u);
            c_exp[c_len] = a_exp[i];
            c_len += !r.is_zero(c_coeff[c_len]);
            i++;
            j++;
        }
    }

    C.length = c_len;
    A.swap(C);

    delta_org--;

    goto looper;

done:

    if (delta_org != 0)
    {
        assert(delta_org > 0);
        r.neg(v, b_coeff[0]);
        r.pow_ui(u, v, delta_org);
        for (i = 0; i < A.length; i++)
        {
            r.mul(v, A.coeffs[i], u);
            r.swap(A.coeffs[i], v);
        }
    }

//std::cout << "prem returning" << std::endl;
//std::cout << "A: " << A.tostring() << std::endl;
//std::cout << "B: " << B.tostring() << std::endl;

}


template <class R>
void pgcd(
	sparse_poly<typename R::elem_t> & F,
	sparse_poly<typename R::elem_t> & B,
	sparse_poly<typename R::elem_t> & A,
	R & r)
{
//std::cout << "pgcd called" << std::endl;

    slong i, d, e;
    typename R::elem_t u, v, w, s;
    sparse_poly<typename R::elem_t> C, D;
    sparse_poly<typename R::elem_t> * last;

    assert(B.length > 0);
    assert(A.length > 0);
    assert(B.exps[0] >= A.exps[0]);
    assert(A.exps[0] >= 1);

    i = std::max(B.exps[0], A.exps[0]);
    A.fit_length(i + 1);
    B.fit_length(i + 1);
    C.fit_length(i + 1);
    D.fit_length(i + 1);

    last = &A;

    r.pow_ui(s, A.coeffs[0], B.exps[0] - A.exps[0]);

    prem<R>(B, A, D, r);

looper:

    d = A.exps[0];
    e = B.exps[0];
    if (B.length <= 0)
        goto done;

    if (d - e > 1)
    {
        r.pow_ui(u, B.coeffs[0], d - e - 1); 
        r.pow_ui(v, s, d - e - 1);

        for (i = 0; i < B.length; i++)
        {
            r.mul(w, u, B.coeffs[i]);
            r.divexact(C.coeffs[i], w, v);
            C.exps[i] = B.exps[i];
        }
        C.length = B.length;
        r.mul(w, s, A.coeffs[0]);
        r.mul(u, v, w);
    }
    else
    {
        for (i = 0; i < B.length; i++)
        {
            set(C.coeffs[i], B.coeffs[i]);
            C.exps[i] = B.exps[i];
        }
        C.length = B.length;
        r.mul(u, s, A.coeffs[0]);
    }

    last = &C;
    if (e == 0)
    {
        goto done;
    }

    prem<R>(A, B, D);
    for (i = 0; i < A.length; i++)
    {
        r.divexact(B.coeffs[i], A.coeffs[i], u);
        B.exps[i] = A.exps[i];
    }
    B.length = A.length;

    A.swap(C);
    r.set(s, A.coeffs[0]);

    last = &A;

    goto looper;

done:

    F.swap(*last);

    return;
}


template <class R>
void pgcd_ducos(
	sparse_poly<typename R::elem_t> & F,
    sparse_poly<typename R::elem_t> & B,
	sparse_poly<typename R::elem_t> & A,
	R & r)
{
    ulong exp;
    slong i, j, k, d, e;
    slong alpha, n, J, aJ, ae;
    slong a_len, b_len, c_len, d_len, h_len, t_len;
    ulong * a_exp, * b_exp, * c_exp, * d_exp, * h_exp, * t_exp;
    typename R::elem_t * a_coeff, * b_coeff, * c_coeff, * d_coeff, * h_coeff, * t_coeff;
    int iexists, jexists, kexists;
    typename R::elem_t u, v, w, s;
    sparse_poly<typename R::elem_t> C, D, H, T;
    sparse_poly<typename R::elem_t> * last;

    assert(B.length > 0);
    assert(A.length > 0);
    assert(B.exps[0] >= A.exps[0]);
    assert(A.exps[0] >= 1);

    i = std::max(B.exps[0], A.exps[0]);
    A.fit_length(1 + i);
    B.fit_length(1 + i);
    C.fit_length(1 + i);
    D.fit_length(1 + i);
    H.fit_length(1 + i);
    T.fit_length(1 + i);

    last = &A;

    r.pow_ui(s, A.coeffs[0], B.exps[0] - A.exps[0]);

    prem<R>(B, A, D, r);

looper:

    d = A.exps[0];
    e = B.exps[0];
    if (B.length <= 0)
        goto done;

    last = &B;

    if (d - e == 1)
    {
        a_len = A.length;
        a_exp = A.exps.data();
        a_coeff = A.coeffs.data();

        b_len = B.length;
        b_exp = B.exps.data();
        b_coeff = B.coeffs.data();

        d_len = D.length;
        d_exp = D.exps.data();
        d_coeff = D.coeffs.data();

        if (e == 0)
            goto done;

        /* D = (B[e]*A - A[e]*B)/A[d] */
        /*           i        j       */
        i = 1;
        j = 1;
        if (a_len > 1 && a_exp[1] == e)
            i++;
        else
            j = b_len;
        d_len = 0;
        while (i < a_len || j < b_len)
        {
            if (i < a_len && j < b_len && a_exp[i] == b_exp[j])
            {
                r.mul(u, a_coeff[i], b_coeff[0]);
                r.mul(v, a_coeff[1], b_coeff[j]);
                r.sub(w, u, v);
                r.divexact(d_coeff[d_len], w, a_coeff[0]);
                d_exp[d_len] = a_exp[i];
                d_len += !r.is_zero(d_coeff[d_len]);
                i++;
                j++;                
            }
            else if (i < a_len && (j >= b_len || a_exp[i] > b_exp[j]))
            {
                r.mul(u, a_coeff[i], b_coeff[0]);
                r.divexact(d_coeff[d_len], u, a_coeff[0]);
                d_exp[d_len++] = a_exp[i];
                i++;
            }
            else
            {
                assert(j < b_len && (i >= a_len || b_exp[j] > a_exp[i]));
                r.mul(v, a_coeff[1], b_coeff[j]);
                r.divexact(d_coeff[d_len], v, a_coeff[0]);
                r.neg(d_coeff[d_len], d_coeff[d_len]);
                d_exp[d_len++] = b_exp[j];
                j++;
            }
        }
        D.length = d_len;

        /* A = (B[e]*(D - B*x) + B[e-1]*B)/s */
        /*            i    j            k    */
        i = 0;
        if (b_len > 1 && b_exp[1] == e - 1) {
            j = 2;            
            k = 1;
        } else {
            j = 1;
            k = b_len;
        }
        a_len = 0;
        while (i < d_len || j < b_len || k < b_len)
        {
            exp = 0;
            if (i < d_len)
                exp = std::max(exp, d_exp[i]);
            if (j < b_len)
                exp = std::max(exp, b_exp[j] + 1);
            if (k < b_len)
                exp = std::max(exp, b_exp[k]);

            a_exp[a_len] = exp;

            iexists = (i < d_len) && (exp == d_exp[i]);
            jexists = (j < b_len) && (exp == b_exp[j] + 1);
            kexists = (k < b_len) && (exp == b_exp[k]);

            assert(iexists || jexists || kexists);

            if (iexists)
            {
                if (jexists)
                {
                    r.sub(w, d_coeff[i], b_coeff[j]);
                    r.mul(u, b_coeff[0], w);
                }
                else
                {
                    r.mul(u, b_coeff[0], d_coeff[i]);
                }
                if (kexists)
                {
                    r.mul(v, b_coeff[1], b_coeff[k]);
                    r.add(w, u, v);
                    r.divexact(a_coeff[a_len], w, s);
                }
                else
                {
                    r.divexact(a_coeff[a_len], u, s);
                }
            }
            else if (kexists)
            {
                r.mul(u, b_coeff[1], b_coeff[k]);
                if (jexists)
                {
                    r.mul(v, b_coeff[0], b_coeff[j]);
                    r.sub(w, u, v);
                    r.divexact(a_coeff[a_len], w, s);
                }
                else
                {
                    r.divexact(a_coeff[a_len], u, s);
                }
            }
            else
            {
                r.mul(u, b_coeff[0], b_coeff[j]);
                r.divexact(a_coeff[a_len], u, s);
                r.neg(a_coeff[a_len], a_coeff[a_len]);
            }

            a_len += !r.is_zero(a_coeff[a_len]);

            i += iexists;
            j += jexists;
            k += kexists;
        }
        A.length = a_len;

        /* A <-> B */
        A.swap(B);

        r.set(s, A.coeffs[0]);
        last = &A;
    }
    else
    {
        a_len = A.length;
        a_exp = A.exps.data();
        a_coeff = A.coeffs.data();
        b_len = B.length;
        b_exp = B.exps.data();
        b_coeff = B.coeffs.data();
        c_len = C.length;
        c_exp = C.exps.data();
        c_coeff = C.coeffs.data();
        d_len = D.length;
        d_exp = D.exps.data();
        d_coeff = D.coeffs.data();
        h_len = H.length;
        h_exp = H.exps.data();
        h_coeff = H.coeffs.data();
        t_len = T.length;
        t_exp = T.exps.data();
        t_coeff = T.coeffs.data();

        n = d - e - 1;
        assert(n > 0);

        alpha = 1;
        while (2*alpha <= n)
            alpha = 2*alpha;

        r.set(u, b_coeff[0]);
        n = n - alpha;
        while (alpha > 1)
        {
            alpha = alpha/2;
            r.mul(v, u, u);
            r.divexact(u, v, s);
            if (n >= alpha)
            {
                r.mul(v, u, b_coeff[0]);
                r.divexact(u, v, s);
                n = n - alpha;
            }
        }
        for (i = 0; i < b_len; i++)
        {
            r.mul(v, u, b_coeff[i]);
            r.divexact(c_coeff[i], v, s);
            c_exp[i] = b_exp[i];
        }
        c_len = b_len;
        C.length = c_len;

        last = &C;

        if (e == 0)
            goto done;

        /* H = C - C[e]*x^e */
        for (i = 1; i < c_len; i++)
        {
            r.set(h_coeff[i - 1], c_coeff[i]);
            h_exp[i - 1] = c_exp[i];
        }
        h_len = c_len - 1;
        H.length = h_len;

        /* D = C[e]*A - A[e]*H  (truncated to powers of x < e) */
        i = 0;
        j = h_len;
        ae = a_len;
        while (i < a_len && a_exp[i] >= e)
        {
            if (a_exp[i] == e)
            {
                j = 0;
                ae = i;
            }
            i++;
        }
        d_len = 0;
        while (i < a_len || j < h_len)
        {
            if (i < a_len && j < h_len && a_exp[i] == h_exp[j])
            {
                r.mul(u, a_coeff[i], c_coeff[0]);
                r.mul(v, a_coeff[ae], h_coeff[j]);
                r.sub(d_coeff[d_len], u, v);
                d_exp[d_len] = a_exp[i];
                d_len += !r.is_zero(d_coeff[d_len]);
                i++;
                j++;
            }
            else if (i < a_len && (j >= h_len || a_exp[i] > h_exp[j]))
            {
                r.mul(d_coeff[d_len], a_coeff[i], c_coeff[0]);
                d_exp[d_len++] = a_exp[i];
                i++;
            }
            else
            {
                assert(j < h_len && (i >= a_len || h_exp[j] > a_exp[i]));
                r.mul(d_coeff[d_len], a_coeff[ae], h_coeff[j]);
                r.neg(d_coeff[d_len], d_coeff[d_len]);
                d_exp[d_len++] = h_exp[j];
                j++;
            }
        }
        D.length = d_len;

        for (J = e + 1; J < d; J++)
        {
            if (h_len == 0)
                break;

            /* H = H*x - H[e-1]*B/B[e] */
            if (h_exp[0] == e - 1)
            {
                i = 1;
                j = 1;
                t_len = 0;
                while (i < h_len || j < b_len)
                {
                    if (i < h_len && j < b_len && h_exp[i] + 1 == b_exp[j])
                    {
                        r.mul(u, h_coeff[0], b_coeff[j]);
                        r.divexact(v, u, b_coeff[0]);
                        r.sub(t_coeff[t_len], h_coeff[i], v);
                        t_exp[t_len] = b_exp[j];
                        t_len += !r.is_zero(t_coeff[t_len]);
                        i++;
                        j++;
                    }
                    else if (i < h_len && (j >= b_len || h_exp[i] + 1 > b_exp[j]))
                    {
                        r.swap(t_coeff[t_len], h_coeff[i]);
                        t_exp[t_len++] = h_exp[i] + 1;
                        i++;
                    }
                    else
                    {
                        assert(j < b_len && (i >= h_len || b_exp[j] > h_exp[i] + 1));
                        r.mul(u, h_coeff[0], b_coeff[j]);
                        r.divexact(t_coeff[t_len], u, b_coeff[0]);
                        r.neg(t_coeff[t_len], t_coeff[t_len]);
                        t_exp[t_len++] = b_exp[j];
                        j++;
                    }
                }
                T.length = t_len;

                H.swap(T);
                h_len = H.length;
                h_exp = H.exps.data();
                h_coeff = H.coeffs.data();
                t_len = T.length;
                t_exp = T.exps.data();
                t_coeff = T.coeffs.data();
            }
            else
            {
                assert(h_exp[0] < e - 1);
                for (i = 0; i < h_len; i++)
                    h_exp[i]++;
            }

            /* find coefficient of x^J in A */
            aJ = 0;
            while (aJ < a_len && a_exp[aJ] != J)
                aJ++;
            if (aJ >= a_len)
                continue;

            /* D = D - A[J]*H */
            i = 0;
            j = 0;
            t_len = 0;
            while (i < d_len || j < h_len)
            {
                if (i < d_len && j < h_len && d_exp[i] == h_exp[j])
                {
                    r.mul(u, h_coeff[j], a_coeff[aJ]);
                    r.sub(t_coeff[t_len], d_coeff[i], u);
                    t_exp[t_len] = d_exp[i];
                    t_len += !r.is_zero(t_coeff[t_len]);
                    i++;
                    j++;                
                }
                else if (i < d_len && (j >= h_len || d_exp[i] > h_exp[j]))
                {
                    r.swap(t_coeff[t_len], d_coeff[i]);
                    t_exp[t_len++] = d_exp[i];
                    i++;
                }
                else
                {
                    assert(j < h_len && (i >= d_len || h_exp[j] > d_exp[i]));
                    r.mul(t_coeff[t_len], h_coeff[j], a_coeff[aJ]);
                    r.neg(t_coeff[t_len], t_coeff[t_len]);
                    t_exp[t_len++] = h_exp[j];
                    j++;
                }
            }
            T.length = t_len;

            D.swap(T);
            d_len = D.length;
            d_exp = D.exps.data();
            d_coeff = D.coeffs.data();
            t_len = T.length;
            t_exp = T.exps.data();
            t_coeff = T.coeffs.data();
        }

        /* B = (-1)^(d-e+1) * (B[e]*(D/A[d] - H*x) +  H[e-1]*B)/s */
        i = 0;
        if (h_len > 0 && h_exp[0] == e - 1) {
            j = 1;
            k = 1;
        } else {
            j = 0;
            k = b_len;
        }
        t_len = 0;
        while (i < d_len || j < h_len || k < b_len)
        {
            exp = 0;
            if (i < d_len)
                exp = std::max(exp, d_exp[i]);
            if (j < h_len)
                exp = std::max(exp, h_exp[j] + 1);
            if (k < b_len)
                exp = std::max(exp, b_exp[k]);

            t_exp[t_len] = exp;

            iexists = (i < d_len && exp == d_exp[i]);
            jexists = (j < h_len && exp == h_exp[j] + 1);
            kexists = (k < b_len && exp == b_exp[k]);

            assert(iexists || jexists || kexists);

            if (iexists)
            {
                if (jexists)
                {
                    r.divexact(u, d_coeff[i], a_coeff[0]);
                    r.sub(w, u, h_coeff[j]);
                    r.mul(u, b_coeff[0], w);
                }
                else
                {
                    r.divexact(u, d_coeff[i], a_coeff[0]);
                    r.mul(u, b_coeff[0], u);
                }
                if (kexists)
                {
                    r.mul(v, h_coeff[0], b_coeff[k]);
                    r.add(w, u, v);
                    r.divexact(t_coeff[t_len], w, s);
                }
                else
                {
                    r.divexact(t_coeff[t_len], u, s);
                }
            }
            else if (kexists)
            {
                r.mul(u, h_coeff[0], b_coeff[k]);
                if (jexists)
                {
                    r.mul(v, b_coeff[0], h_coeff[j]);
                    r.sub(w, u, v);
                    r.divexact(t_coeff[t_len], w, s);
                }
                else
                {
                    r.divexact(t_coeff[t_len], u, s);
                }
            }
            else
            {
                r.mul(u, b_coeff[0], h_coeff[j]);
                r.divexact(t_coeff[t_len], u, s);
                r.neg(t_coeff[t_len], t_coeff[t_len]);
            }

            if (((d - e) & 1) == 0)
                r.neg(t_coeff[t_len], t_coeff[t_len]);

            t_len += !r.is_zero(t_coeff[t_len]);

            i += iexists;
            j += jexists;
            k += kexists;
        }
        T.length = t_len;

        /* B <-> T */
        B.swap(T);
        b_len = T.length;
        b_exp = T.exps.data();
        b_coeff = T.coeffs.data();
        t_len = T.length;
        t_exp = T.exps.data();
        t_coeff = T.coeffs.data();

        /* A <-> C */
        A.swap(C);
        a_len = A.length;
        a_exp = A.exps.data();
        a_coeff = A.coeffs.data();
        c_len = C.length;
        c_exp = C.exps.data();
        c_coeff = C.coeffs.data();

        r.set(s, A.coeffs[0]);

        last = &A;
    }

    goto looper;

done:

    F.swap(*last);
}




template <class R>
void resultant(
	typename R::elem_t & z,
	sparse_poly<typename R::elem_t> & a,
	sparse_poly<typename R::elem_t> & b,
	R & r)
{
//std::cout << "resultant called" << std::endl;
//std::cout << "a: " << a.tostring() << std::endl;
//std::cout << "b: " << b.tostring() << std::endl;

    sparse_poly<typename R::elem_t> t;
    bool neg = false;

    if (a.exps[0] >= b.exps[0])
	{
        pgcd_ducos<R>(t, a, b, r);
	}
    else
	{
        neg = (a.exps[0] & b.exps[0] & 1);
        pgcd_ducos<R>(t, b, a, r);
	}

    if (t.length != 1 || t.exps[0] != 0)
    {
        r.zero(z);
    }
    else
    {
        r.swap(z, t.coeffs[0]);
        if (neg)
            r.neg(z, z);
    }

//std::cout << "resultant returning" << std::endl;
//std::cout << "z: " << z.tostring() << std::endl;
}


/* wp = working precision */

slong binary_wp(const acb_t a, const acb_t b)
{
    slong pa = acb_rel_accuracy_bits(a);
    slong pb = acb_rel_accuracy_bits(b);
    slong p = std::max(pa, pb);
    assert(p < ARF_PREC_EXACT - 100);
    return std::max(p, slong(0)) + 100;
}

slong unary_wp(const acb_t a)
{
    slong p = acb_rel_accuracy_bits(a);
    assert(p < ARF_PREC_EXACT - 100);
    return std::max(p, slong(0)) + 100;
}


/* copy the midpoint and set the radius */
void acb_set_radius(acb_t y, const acb_t x, const mag_t e)
{
    arf_set(arb_midref(acb_realref(y)), arb_midref(acb_realref(x)));
    mag_set(arb_radref(acb_realref(y)), e);
    if (arb_is_zero(acb_imagref(x)))
    {
        arb_zero(acb_imagref(y));
    }
    else
    {
        arf_set(arb_midref(acb_imagref(y)), arb_midref(acb_imagref(x)));
        mag_set(arb_radref(acb_imagref(y)), e);
    }
}

void acb_chop_imag(acb_t x)
{
    if (mag_is_zero(arb_radref(acb_realref(x))))
        mag_set(arb_radref(acb_realref(x)), arb_radref(acb_imagref(x)));
    arb_zero(acb_imagref(x));
}

void arb_limit_precision(arb_t x, slong p)
{
    mag_t t;
    mag_init(t);
    arf_get_mag(t, arb_midref(x));
    mag_mul_2exp_si(t, t, -p);
    if (mag_is_zero(t))
        mag_set_ui_2exp_si(t, 1, -p);
    else if (mag_cmp(t, arb_radref(x)) > 0)
        mag_swap(arb_radref(x), t);
    mag_clear(t);
}


void arb_canonicalize(arb_t x, slong p)
{
    if (!mag_is_zero(arb_radref(x)))
        return;

    arf_get_mag(arb_radref(x), arb_midref(x));
    if (mag_is_zero(arb_radref(x)))
        mag_one(arb_radref(x));
    mag_mul_2exp_si(arb_radref(x), arb_radref(x), -p);
}

void acb_canonicalize(acb_t x, slong p)
{
    if (arb_is_zero(acb_imagref(x)))
    {
        arb_canonicalize(acb_realref(x), p);
    }
    else if (mag_is_zero(arb_radref(acb_realref(x))) && mag_is_zero(arb_radref(acb_imagref(x))))
    {
        arb_canonicalize(acb_realref(x), p);
        arb_canonicalize(acb_imagref(x), p);
    }
}

bool acb_is_canonical(const acb_t x)
{
    if (!acb_is_finite(x))
        return false;

    if (mag_is_zero(arb_radref(acb_realref(x))) && mag_is_zero(arb_radref(acb_imagref(x))))
        return false;

    return true;
}

/* if true, the members of the qbarelem class are valid */
bool newton_test(const fmpz_poly_t f, const acb_t x, mag_t e, slong extra = 0)
{
    assert(acb_is_canonical(x));
    acb_t y; /* TODO: could be shallow copies of other stuff */
    xfmpz_poly_t fp, fpp;
    xacb_t f_eval, fp_eval, fpp_eval;
    xmag_t a0, b0, c, t;
    slong p = unary_wp(x);

    fmpz_poly_derivative(fp.data, f);
    fmpz_poly_derivative(fpp.data, fp.data);

    acb_init(y);

    if (arb_is_zero(acb_imagref(x)))
    {
        arb_fmpz_poly_evaluate_arb(acb_realref(fp_eval.data), fp.data, acb_realref(x), p);
        arb_get_mag_lower(a0.data, acb_realref(fp_eval.data));

        arb_fmpz_poly_evaluate_arb(acb_realref(f_eval.data), f, acb_realref(x), p);
        arb_get_mag(b0.data, acb_realref(f_eval.data));
        mag_div(b0.data, b0.data, a0.data);

        mag_mul_2exp_si(e, b0.data, 1);
        mag_max(e, e, arb_radref(acb_realref(x)));

        arf_set(arb_midref(acb_realref(y)), arb_midref(acb_realref(x)));
        mag_set(arb_radref(acb_realref(y)), e);
        arb_zero(acb_imagref(y));

        arb_fmpz_poly_evaluate_arb(acb_realref(fpp_eval.data), fpp.data, acb_realref(y), p);
        arb_get_mag(c.data, acb_realref(fpp_eval.data));
    }
    else
    {
        arb_fmpz_poly_evaluate_acb(fp_eval.data, fp.data, x, p);
        acb_get_mag_lower(a0.data, fp_eval.data);

        arb_fmpz_poly_evaluate_acb(f_eval.data, f, x, p);
        acb_get_mag(b0.data, f_eval.data);
        mag_div(b0.data, b0.data, a0.data);

        mag_mul_2exp_si(e, b0.data, 1);
        mag_max(e, e, arb_radref(acb_realref(x)));
        mag_max(e, e, arb_radref(acb_imagref(x)));

        arf_set(arb_midref(acb_realref(y)), arb_midref(acb_realref(x)));
        arf_set(arb_midref(acb_imagref(y)), arb_midref(acb_imagref(x)));
        mag_set(arb_radref(acb_realref(y)), e);
        mag_set(arb_radref(acb_imagref(y)), e);
        arb_fmpz_poly_evaluate_acb(fpp_eval.data, fpp.data, y, p);
        acb_get_mag(c.data, fpp_eval.data);
    }

    acb_clear(y);

    mag_mul(c.data, c.data, b0.data);
    mag_div(c.data, c.data, a0.data);

//std::cout << "newton test c: " << c.tostring() << std::endl;

    return mag_is_finite(b0.data) && mag_cmp_2exp_si(c.data, -5 - extra) < 0;
}

void newton_trim(const fmpz_poly_t f, acb_t x, mag_t xe)
{
    xacb_t y;
    xmag_t ye;
    slong p;
    slong lo = 0;
    slong hi = std::max(slong(20), acb_rel_accuracy_bits(x));

again:

    p = (hi + lo)/2;

    if (hi <= 20 || hi - lo < 5)
        return;

    arb_set(acb_realref(y.data), acb_realref(x));
    arb_limit_precision(acb_realref(y.data), p);

    if (arb_is_zero(acb_imagref(x)))
    {
        arb_zero(acb_imagref(y.data));
    }
    else
    {
        arb_set(acb_imagref(y.data), acb_imagref(x));
        arb_limit_precision(acb_imagref(y.data), p);
    }

    if (!newton_test(f, y.data, ye.data, 30))
    {
        lo = p;
        goto again;
    }

    acb_swap(x, y.data);
    mag_swap(xe, ye.data);
    hi = p;
    goto again;
}


/* advance the members of the qbarelem class by one iteration */
void newton_step(const fmpz_poly_t f, acb_t x, mag_t xe)
{
    assert(acb_is_canonical(x));
    xacb_t z;
    xmag_t ze;
    xfmpz_poly_t fp;
    xacb_t t, f_eval, fp_eval;
    slong p = unary_wp(x);

    fmpz_poly_derivative(fp.data, f);

try_again:

    arf_set(arb_midref(acb_realref(z.data)), arb_midref(acb_realref(x)));
    mag_mul_2exp_si(arb_radref(acb_realref(z.data)), arb_radref(acb_realref(x)), -p);
    p = std::min(2*p, slong(ARF_PREC_EXACT));

    if (arb_is_zero(acb_imagref(x)))
    {
        arb_fmpz_poly_evaluate_arb(acb_realref(f_eval.data), f, acb_realref(z.data), p);
        arb_fmpz_poly_evaluate_arb(acb_realref(fp_eval.data), fp.data, acb_realref(z.data), p);
        arb_div(acb_realref(t.data), acb_realref(f_eval.data), acb_realref(fp_eval.data), p);
        arb_sub(acb_realref(z.data), acb_realref(z.data), acb_realref(t.data), p);
    }
    else
    {
        arf_set(arb_midref(acb_imagref(z.data)), arb_midref(acb_imagref(x)));
        mag_mul_2exp_si(arb_radref(acb_imagref(z.data)), arb_radref(acb_imagref(x)), -p);

        arb_fmpz_poly_evaluate_acb(f_eval.data, f, z.data, p);
        arb_fmpz_poly_evaluate_acb(fp_eval.data, fp.data, z.data, p);
        acb_div(t.data, f_eval.data, fp_eval.data, p);
        acb_sub(z.data, z.data, t.data, p);
    }

    if (newton_test(f, z.data, ze.data) && mag_cmp(ze.data, xe) < 0)
    {
        acb_swap(x, z.data);
        mag_swap(xe, ze.data);
        return;
    }

    assert(0 && "shit hit the fan");

    /* something bad happened. try again with greater precision */
    // TODO: and possibly more than one iteration
    p += 10 + p/16;
    goto try_again;
}


class qbarelem {
public:
/*
    The object is valid if mu <= 2^-6, where
        a0 >= |1/f'(x)|
        b0 >= |f(x)/f'(x)|
        e >= max(2*b0, x.rad)
        y := [x.mid +- e]
        c >= |f''(y)|
        mu = 2*a0*b0*c
    If so, newton's method converges to the unique root in y from any starting
    value in x. The 2^-6 can actually be replaced by 1 but is used for sanity.
*/
    xfmpz_poly_t minpoly; // f irreducible, primitive, and positive leadcoeff
    xacb_t location;      // x
    xmag_t epsilon;       // e

    /* expr conversion */
    bool set_ex(er e);
    bool set_exp_ex(er f, er e);
    ex get_ex() const;
//    slong get_index() const;

    /* basic arithmetic */
    bool set(const fmpz_poly_t f, const acb_t x);
    void set(const qbarelem & a);
    void set(slong a);
    void set(const fmpz_t a);
    void set(const fmpq_t a);
    bool isreal() const;
    bool realpart(const qbarelem & a);
    void neg(const qbarelem & a);
    bool add(const qbarelem & a, const qbarelem & b);
    bool sub(const qbarelem & a, const qbarelem & b);
    bool mul(const qbarelem & a, const qbarelem & b);
    bool div(const fmpz_t a, const fmpz_t b);
    bool div(const qbarelem & a, const qbarelem & b);
    bool pow(const qbarelem & a, slong b, ulong c);
    bool pow(const qbarelem & a, qbarelem & b);
    bool pow(const qbarelem & a, const fmpz_t b);
    bool pow(const qbarelem & a, const fmpq_t b);
    bool set_expipi(const fmpq_t b);
    bool mul_expipi(const qbarelem & a, const fmpq_t b);
    bool apply_moebius(const qbarelem & a, const fmpz_t m11, const fmpz_t m12, const fmpz_t m21, const fmpz_t m22);
    void apply_rat_fxn(const qbarelem & a, const fmpz * num, slong num_length, const fmpz * den, slong den_length);
    void quadraticroot(const fmpz_t a, const fmpz_t b, const fmpz_t c);
    bool circle_root(const qbarelem & a, int sign);

    void swap(qbarelem & a) {
        fmpz_poly_swap(minpoly.data, a.minpoly.data);
        acb_swap(location.data, a.location.data);
        mag_swap(epsilon.data, a.epsilon.data);
    }

    std::string tostring() const {
        return "Root[degree " + std::to_string(fmpz_poly_degree(minpoly.data)) + ", " + location.tostring() + "]";
    }

    template <typename RefineFunc> bool select_root(fmpz_poly_t p, bool must_factor, RefineFunc T);
    void complete_linear(slong e);
};


bool qbarelem::set(const fmpz_poly_t f, const acb_t x)
{
    slong the_correct_one = -1;
    xfmpz_poly_factor_t fac;
    fmpz_poly_factor(fac.data, f);

    acb_set(location.data, x);

    if (!acb_is_finite(location.data))
        return false;

    acb_canonicalize(location.data, FLINT_BITS);

    for (slong i = 0; i < fac.data->num; i++)
    {
        if (newton_test(fac.data->p + i, location.data, epsilon.data))
        {
            if (the_correct_one >= 0)
                return false;
            the_correct_one = i;
        }
    }

    if (the_correct_one < 0)
        return false;

    fmpz_poly_swap(minpoly.data, fac.data->p + the_correct_one);
    return true;
}

void qbarelem::set(const qbarelem & a)
{
    fmpz_poly_set(minpoly.data, a.minpoly.data);
    acb_set(location.data, a.location.data);
}

void qbarelem::complete_linear(slong e)
{
    /* make sure x.rad >= 2^e */
    assert(minpoly.data->length == 2);
    if (mag_cmp_2exp_si(arb_radref(acb_realref(location.data)), e) < 0)
        mag_set_ui_2exp_si(arb_radref(acb_realref(location.data)), 1, e);
    mag_mul_2exp_si(epsilon.data, arb_radref(acb_realref(location.data)), 1);
    arb_zero(acb_imagref(location.data));
}

void qbarelem::set(slong a)
{
    fmpz_poly_fit_length(minpoly.data, 2);
    fmpz_set_si(minpoly.data->coeffs + 0, a);
    fmpz_neg(minpoly.data->coeffs + 0, minpoly.data->coeffs + 0);
    fmpz_one(minpoly.data->coeffs + 1);
    _fmpz_poly_set_length(minpoly.data, 2);

    arf_set_si(arb_midref(acb_realref(location.data)), a);
    mag_set_ui_2exp_si(arb_radref(acb_realref(location.data)), 1, -FLINT_BITS);
    complete_linear(-FLINT_BITS);
}

void qbarelem::set(const fmpz_t a)
{
    fmpz_poly_fit_length(minpoly.data, 2);
    fmpz_neg(minpoly.data->coeffs + 0, a);
    fmpz_one(minpoly.data->coeffs + 1);
    _fmpz_poly_set_length(minpoly.data, 2);

    arf_set_fmpz(arb_midref(acb_realref(location.data)), a);
    mag_set_ui_2exp_si(arb_radref(acb_realref(location.data)), 1, -FLINT_BITS);
    complete_linear(-FLINT_BITS);
}

void qbarelem::set(const fmpq_t a)
{
    div(fmpq_numref(a), fmpq_denref(a));
}

bool qbarelem::div(const fmpz_t a, const fmpz_t b)
{
    if (fmpz_is_zero(b))
        return false;

    fmpz_poly_fit_length(minpoly.data, 2);
    fmpz_neg(minpoly.data->coeffs + 0, a);
    fmpz_set(minpoly.data->coeffs + 1, b);
    _fmpz_poly_set_length(minpoly.data, 2);
    _fmpq_canonicalise(minpoly.data->coeffs + 0, minpoly.data->coeffs + 1);

    slong p = (slong)fmpz_bits(a) - (slong)fmpz_bits(b);
    arb_fmpz_div_fmpz(acb_realref(location.data), a, b, 2*FLINT_BITS);
    complete_linear(p - 2*FLINT_BITS);

    return true;
}

bool qbarelem::pow(const qbarelem & a, const fmpz_t b)
{
    if (fmpz_fits_si(b))
        return pow(a, fmpz_get_si(b), 1);

    if (fmpz_poly_length(a.minpoly.data) > 2)
        return false;

    if (!fmpz_is_one(a.minpoly.data->coeffs + 1))
        return false;

    if (fmpz_is_zero(a.minpoly.data->coeffs + 0))
    {
        set(slong(0));
        return fmpz_sgn(b) > 0;
    }

    if (!fmpz_is_pm1(a.minpoly.data->coeffs + 0))
        return false;

    slong r = 1;
    if (!fmpz_is_one(a.minpoly.data->coeffs + 0) && fmpz_is_odd(b))
        r = -1;
    set(r);
    return true;
}

bool qbarelem::pow(const qbarelem & a, const fmpq_t b)
{
    if (fmpz_fits_si(fmpq_numref(b)) && fmpz_fits_si(fmpq_denref(b)))
        return pow(a, fmpz_get_si(fmpq_numref(b)), fmpz_get_si(fmpq_denref(b)));

    assert(0 && "don't want to do this shit anymore");
    return false;
}



void irred_fmpz_poly_roots(std::vector<qbarelem> & v, fmpz_poly_t f)
{
    slong n = f->length - 1;
    v.resize(n);

    acb_struct * roots = (acb_struct *) malloc(n*sizeof(acb_struct));
    mag_struct * eps = (mag_struct *) malloc(n*sizeof(mag_struct));
    for (slong i = 0; i < n; i++)
    {
        acb_init(roots + i);
        mag_init(eps + i);
        fmpz_poly_set(v[i].minpoly.data, f);
        _fmpz_poly_primitive_part(v[i].minpoly.data->coeffs,
                                  v[i].minpoly.data->coeffs, n + 1);
    }

    slong p = FLINT_BITS;

try_again:

    p += (FLINT_BITS + p)/16;

    arb_fmpz_poly_complex_roots(roots, v[0].minpoly.data, 0, p);

    for (slong i = 0; i < n; i++)
    {
        acb_canonicalize(roots + i, p);
        if (!newton_test(v[0].minpoly.data, roots + i, eps + i))
            goto try_again;
    }

    for (slong i = 0; i < n; i++)
    {
        newton_trim(v[i].minpoly.data, roots + i, eps + i);
        acb_swap(v[i].location.data, roots + i);
        mag_swap(v[i].epsilon.data, eps + i);
        acb_clear(roots + i);
        mag_clear(eps + i);
    }
    free(roots);
    free(eps);
}


/* given a function for refining our location, set us to the right root of p */
template <typename RefineFunc>
bool qbarelem::select_root(fmpz_poly_t p, bool must_factor, RefineFunc refine)
{
    slong prec;
    xacb_t t;
    xfmpz_poly_factor_t f;
timeit_t timer;

    if (must_factor)
    {
timeit_start(timer);
        fmpz_poly_factor(f.data, p);
timeit_stop(timer);
flint_printf("factor time: %wd\n", timer->wall);
    }
    else
    {
        _fmpz_poly_primitive_part(p->coeffs, p->coeffs, p->length);
        fmpz_poly_factor_fit_length(f.data, 1);
        f.data->num = 1;
        fmpz_poly_swap(f.data->p + 0, p);
    }

timeit_start(timer);

try_again:

    refine(location.data, f.data->num > 1);

    prec = unary_wp(location.data);

std::cout << "trying location: " << location.tostring() << std::endl;

    if (f.data->num > 1)
    {
        for (slong i = 0; i < f.data->num; i++)
        {
            arb_fmpz_poly_evaluate_acb(t.data, f.data->p + i, location.data, prec);
            if (!acb_contains_zero(t.data))
            {
                fmpz_poly_swap(f.data->p + i, f.data->p + f.data->num - 1);
                f.data->num--;
                i--;
            }
        }

        if (f.data->num > 1)
            goto try_again;
    }

    assert(f.data->num == 1);

    if (!newton_test(f.data->p + 0, location.data, epsilon.data))
        goto try_again;

    fmpz_poly_swap(minpoly.data, f.data->p + 0);

timeit_stop(timer);
flint_printf("select time: %wd\n", timer->wall);

    return true;
}


/************** (m11*a + m12)/(m21*a + m22) *********/

class moebius_refiner {
    xacb_t a, ta;
    xmag_t ae;
    const fmpz * m11, * m12, * m21, * m22;
    const fmpz_poly_struct * aminpoly;
    bool first;

public:

    moebius_refiner(const qbarelem & A, const fmpz_t M11, const fmpz_t M12,
                                        const fmpz_t M21, const fmpz_t M22)
    {
        m11 = M11;
        m12 = M12;
        m22 = M22;
        m21 = M21;
        first = true;
        aminpoly = A.minpoly.data;
        acb_set(a.data, A.location.data);
        mag_set(ae.data, A.epsilon.data);
    }
    void operator() (acb_t x, bool need_containment)
    {
        xacb_t n, d;

    try_again:

        if (!first)
        {
            newton_step(aminpoly, a.data, ae.data);
        }
        first = false;

        slong p = unary_wp(a.data);

        if (need_containment)
            acb_set_radius(x, a.data, ae.data);
        else
            acb_set(x, a.data);

        acb_mul_fmpz(n.data, x, m11, p);
        acb_add_fmpz(n.data, n.data, m12, p);
        acb_mul_fmpz(d.data, x, m21, p);
        acb_add_fmpz(d.data, d.data, m22, p);
        acb_div(x, n.data, d.data, p);

        if (!acb_is_finite(x))
            goto try_again;
    }
};


bool qbarelem::apply_moebius(const qbarelem & a, const fmpz_t m11, const fmpz_t m12,
                                                 const fmpz_t m21, const fmpz_t m22)
{
//std::cout << "++++++++++++++++++++++++++++++++++++" << std::endl;
//std::cout << "moebius " << a.tostring() << std::endl;
//printf("m11: "); fmpz_print(m11); printf("\n");
//printf("m12: "); fmpz_print(m12); printf("\n");
//printf("m21: "); fmpz_print(m21); printf("\n");
//printf("m22: "); fmpz_print(m22); printf("\n");

    xfmpz_t n, d;

    if (fmpz_poly_length(a.minpoly.data) <= 2)
    {
        fmpz_mul(n.data, a.minpoly.data->coeffs + 0, m11);
        fmpz_mul(d.data, a.minpoly.data->coeffs + 0, m21);
        fmpz_submul(n.data, a.minpoly.data->coeffs + 1, m12);
        fmpz_submul(d.data, a.minpoly.data->coeffs + 1, m22);
        return div(n.data, d.data);
    }

    fmpz_mul(d.data, m11, m22);
    fmpz_submul(d.data, m12, m21);
    if (fmpz_is_zero(d.data))
        return fmpz_is_zero(m22) ? div(m11, m21) : div(m12, m22);

    xfmpz_poly_t t, u, v, U, V, W;
    slong da = fmpz_poly_degree(a.minpoly.data);

    fmpz_poly_set_coeff_fmpz(u.data, 1, m22);
    fmpz_poly_set_coeff_fmpz(u.data, 0, m12);
    fmpz_neg(u.data->coeffs + 0, u.data->coeffs + 0);

    fmpz_poly_set_coeff_fmpz(v.data, 1, m21);
    fmpz_poly_set_coeff_fmpz(v.data, 0, m11);
    fmpz_neg(v.data->coeffs + 0, v.data->coeffs + 0);
    fmpz_poly_neg(v.data, v.data);

    for (slong i = 0; i <= da; i++)
    {
        fmpz_poly_pow(U.data, u.data, i);
        fmpz_poly_pow(V.data, v.data, da - i);
        fmpz_poly_mul(W.data, U.data, V.data);
        fmpz_poly_scalar_addmul_fmpz(t.data, W.data, a.minpoly.data->coeffs + i);
    }

    select_root(t.data, false, moebius_refiner(a, m11, m12, m21, m22));

    return true;
}

bool qbarelem::set_expipi(const fmpq_t a)
{
    if (!fmpz_fits_si(fmpq_denref(a)))
        return false;

    ulong n = fmpz_get_si(fmpq_denref(a));
    fmpz_poly_cyclotomic(minpoly.data, fmpz_is_even(fmpq_numref(a)) ? n : 2*n);

    slong p = 10;

try_again:

    p += FLINT_BITS;

    arb_sin_cos_pi_fmpq(acb_imagref(location.data), acb_realref(location.data), a, p);
    acb_canonicalize(location.data, p);

    if (!newton_test(minpoly.data, location.data, epsilon.data))
        goto try_again;

    return true;
}

bool qbarelem::mul_expipi(const qbarelem & a, const fmpq_t b)
{
    qbarelem t;
    return t.set_expipi(b) && mul(a, t);
}

/************* a + b ***************/

class add_refiner {
    xacb_t a, b, ta, tb;
    xmag_t ae, be;
    const fmpz_poly_struct * aminpoly, * bminpoly;
    bool first;

public:

    add_refiner(const qbarelem & A, const qbarelem & B)
    {
        first = true;
        aminpoly = A.minpoly.data;
        bminpoly = B.minpoly.data;
        acb_set(a.data, A.location.data);
        acb_set(b.data, B.location.data);
        mag_set(ae.data, A.epsilon.data);
        mag_set(be.data, B.epsilon.data);
    }
    void operator() (acb_t x, bool need_containment)
    {
        if (!first)
        {
            newton_step(aminpoly, a.data, ae.data);
            newton_step(bminpoly, b.data, be.data);
            first = false;
        }

        slong p = binary_wp(a.data, b.data);

        if (need_containment)
        {
            acb_set_radius(ta.data, a.data, ae.data);
            acb_set_radius(tb.data, b.data, be.data);
            acb_add(x, ta.data, tb.data, p);
        }
        else
        {
            acb_add(x, a.data, b.data, p);
        }
    }
};

bool qbarelem::add(const qbarelem & a, const qbarelem & b)
{
std::cout << "++++++++++++++++++++++++++++++++++++" << std::endl;
std::cout << "add " << a.tostring() << std::endl;
std::cout << "  + " << b.tostring() << std::endl;

    if (fmpz_poly_length(a.minpoly.data) <= 2)
    {
        xfmpz_t m12(a.minpoly.data->coeffs + 0);
        fmpz_neg(m12.data, m12.data);
        return apply_moebius(b, a.minpoly.data->coeffs + 1, m12.data,
                                eget_cint_data(0), a.minpoly.data->coeffs + 1);
    }

    if (fmpz_poly_length(b.minpoly.data) <= 2)
    {
        xfmpz_t m12(b.minpoly.data->coeffs + 0);
        fmpz_neg(m12.data, m12.data);
        return apply_moebius(a, b.minpoly.data->coeffs + 1, m12.data,
                                eget_cint_data(0), b.minpoly.data->coeffs + 1);
    }

timeit_t timer;
timeit_start(timer);

    xfmpz_poly_t t;

    if (algo == Algo_pgcd)
    {
		rfmpz_poly_t r;
        sparse_poly<xfmpz_poly_t> A, B;
        set_univar<rfmpz_poly_t>(B, a.minpoly, r);
        set_univar_shift<rfmpz_poly_t>(A, b.minpoly, -1, r);
        resultant<rfmpz_poly_t>(t, A, B, r);
    }
    else
    {
        x_fmpz_vector_t xs, ys;

        slong ad = fmpz_poly_degree(a.minpoly.data);
        slong bd = fmpz_poly_degree(b.minpoly.data);
        slong rd = ad*bd;

        _fmpz_vector_fit_length(xs.data, rd + 1);
        _fmpz_vector_fit_length(ys.data, rd + 1);

        fmpz_poly_set(t.data, b.minpoly.data);
        for (slong i = 1; i <= bd; i += 2)
            fmpz_neg(t.data->coeffs + i, t.data->coeffs + i);

        for (slong i = 0; i <= rd; i++)
        {
            fmpz_set_si(xs.data->array + i, i);
            fmpz_poly_resultant(ys.data->array + i, a.minpoly.data, t.data);
            if (i < rd)
                _fmpz_poly_taylor_shift(t.data->coeffs, eget_cint_data(-1), bd + 1);
        }
        fmpz_poly_interpolate_fmpz_vec(t.data, xs.data->array, ys.data->array, rd + 1);
    }

timeit_stop(timer);
flint_printf("poly + time: %wd\n", timer->wall);

    select_root(t.data, true, add_refiner(a, b));

std::cout << "add return: " << tostring() << std::endl;
    return true;
}



/************* a - b ***************/

class sub_refiner {
    xacb_t a, b, ta, tb;
    xmag_t ae, be;
    const fmpz_poly_struct * aminpoly, * bminpoly;
    bool first;

public:

    sub_refiner(const qbarelem & A, const qbarelem & B)
    {
        first = true;
        aminpoly = A.minpoly.data;
        bminpoly = B.minpoly.data;
        acb_set(a.data, A.location.data);
        acb_set(b.data, B.location.data);
        mag_set(ae.data, A.epsilon.data);
        mag_set(be.data, B.epsilon.data);
    }

    void operator() (acb_t x, bool need_containment)
    {
        if (!first)
        {
            newton_step(aminpoly, a.data, ae.data);
            newton_step(bminpoly, b.data, be.data);
            first = false;
        }

        slong p = binary_wp(a.data, b.data);

        if (need_containment)
        {
            acb_set_radius(ta.data, a.data, ae.data);
            acb_set_radius(tb.data, b.data, be.data);
            acb_sub(x, ta.data, tb.data, p);
        }
        else
        {
            acb_sub(x, a.data, b.data, p);
        }
    }
};


void qbarelem::neg(const qbarelem & a)
{
    if (this != &a)
        set(a);

    for (slong i = minpoly.data->length - 2; i >= 0; i -= 2)
        fmpz_neg(minpoly.data->coeffs + i, minpoly.data->coeffs + i);

    acb_neg(location.data, location.data);
}

bool qbarelem::sub(const qbarelem & a, const qbarelem & b)
{
//std::cout << "++++++++++++++++++++++++++++++++++++" << std::endl;
//std::cout << "sub " << a.tostring() << " - " << b.tostring() << std::endl;

    if (fmpz_poly_length(a.minpoly.data) <= 2)
    {
        if (!apply_moebius(b, a.minpoly.data->coeffs + 1, a.minpoly.data->coeffs + 0,
                              eget_cint_data(0),          a.minpoly.data->coeffs + 1))
        {
            return false;
        }
        neg(*this);
        return true;
    }

    if (fmpz_poly_length(b.minpoly.data) <= 2)
    {
        return apply_moebius(a, b.minpoly.data->coeffs + 1, b.minpoly.data->coeffs + 0,
                                eget_cint_data(0),          b.minpoly.data->coeffs + 1);
    }

    xfmpz_poly_t t;
	rfmpz_poly_t r;
    sparse_poly<xfmpz_poly_t> A, B;
    set_univar<rfmpz_poly_t>(A, a.minpoly, r);
    set_univar_shift<rfmpz_poly_t>(B, b.minpoly, +1, r);
    resultant<rfmpz_poly_t>(t, A, B, r);

    select_root(t.data, true, sub_refiner(a, b));

//std::cout << "sub return: " << tostring() << std::endl;
    return true;
}



/************* Re[a] ***************/

bool qbarelem::isreal() const
{
    if (minpoly.data->length <= 2 || arb_is_zero(acb_imagref(location.data)))
        return true;

    /* t will always contain a unique root and TODO: can be a shallow object */
    xacb_t a, t;
    xmag_t ae, te;

    acb_set_radius(t.data, location.data, epsilon.data);
    if (!arb_contains_zero(acb_imagref(t.data)))
        return false;

    acb_set(a.data, location.data);
    mag_set(ae.data, epsilon.data);

try_again:

    /* t intersects the real axis */
    arb_zero(acb_imagref(t.data));
    if (newton_test(minpoly.data, t.data, te.data))
        return true;

    newton_step(minpoly.data, a.data, ae.data);

    acb_set_radius(t.data, a.data, ae.data);
    if (!arb_contains_zero(acb_imagref(t.data)))
        return false;

    goto try_again;
}

class realpart_refiner {
    xacb_t a, ta;
    xmag_t ae;
    const fmpz_poly_struct * aminpoly;
    bool first;

public:

    realpart_refiner(const qbarelem & A)
    {
        first = true;
        aminpoly = A.minpoly.data;
        acb_set(a.data, A.location.data);
        mag_set(ae.data, A.epsilon.data);
    }
    void operator() (acb_t x, bool need_containment)
    {
        if (!first)
        {
            newton_step(aminpoly, a.data, ae.data);
            first = false;
        }

        if (need_containment)
            acb_set_radius(x, a.data, ae.data);
        else
            acb_set(x, a.data);

        acb_chop_imag(x);
        acb_mul_2exp_si(x, x, 1);
    }
};

bool qbarelem::realpart(const qbarelem & a)
{
std::cout << "++++++++++++++++++++++++++++++++++++" << std::endl;
std::cout << "realpart " << a.tostring() << std::endl;

    if (a.isreal())
    {
        set(a);
        return true;
    }

timeit_t timer;
timeit_start(timer);

    slong ad = fmpz_poly_degree(a.minpoly.data);
    slong td;
    if (mul_si_checked(td, ad, ad - 1))
        throw exception_arithmetic(17);

    td = td/2;

    xfmpz_poly_t t1;

    if (algo == Algo_pgcd)
    {
        xfmpz_poly_series_t t2;

        try {
            slong prec = td + ad;
		    rfmpz_poly_series_t r(prec);
            sparse_poly<xfmpz_poly_series_t> A, B;

            while (true)
            {
                set_univar<rfmpz_poly_series_t>(A, a.minpoly, r);
                set_univar_shift<rfmpz_poly_series_t>(B, a.minpoly, -1, r);
                resultant<rfmpz_poly_series_t>(t2, A, B, r);
                if (t2.absolute_prec() > td)
                    break;
                prec += td - t2.absolute_prec() + 1;
                r.set_prec(prec);
            }
        }
        catch (exception_arithmetic & e)
        {
	        rfmpz_poly_t r;
            sparse_poly<xfmpz_poly_t> A, B;
            set_univar<rfmpz_poly_t>(A, a.minpoly, r);
            set_univar_shift<rfmpz_poly_t>(B, a.minpoly, -1, r);
            resultant<rfmpz_poly_t>(t2.terms, A, B, r);
            t2.start = 0;
            t2.prec = 2*td + ad + 1;
            t2.normalize();
        }

        // divide the terms of t2 by a(x/2)*2^deg(a)
        fmpz_poly_scalar_mul_fmpz(t1.data, a.minpoly.data, a.minpoly.data->coeffs + ad);
        for (slong i = 1; i <= ad; i += 1)
            fmpz_mul_2exp(t1.data->coeffs + ad - i, t1.data->coeffs + ad - i, i);
        fmpz_poly_div_series(t1.data, t2.terms.data, t1.data, td + 1);

        assert((t2.start % 2) == 0);
		fmpz_poly_sqrt_series(t1.data, t1.data, td - t2.start/2 + 1);
        fmpz_poly_shift_left(t1.data, t1.data, t2.start/2);
    }
    else
    {
        x_fmpz_vector_t xs, ys;
        xfmpz_poly_t t2, a2;
        xfmpz_t u, res;

        _fmpz_vector_fit_length(xs.data, 2*td + 1);
        _fmpz_vector_fit_length(ys.data, 2*td + 1);

        fmpz_poly_set(t1.data, a.minpoly.data);
        fmpz_poly_scalar_mul_fmpz(a2.data, a.minpoly.data, a.minpoly.data->coeffs + ad);
        for (slong i = 1; i <= ad; i += 1)
        {
            fmpz_mul_2exp(a2.data->coeffs + ad - i, a2.data->coeffs + ad - i, i);
            if (i & 1)
                fmpz_neg(t1.data->coeffs + i, t1.data->coeffs + i);
        }
        fmpz_poly_set(t2.data, t1.data);

        fmpz_zero(xs.data->array + 0);
        fmpz_poly_resultant(res.data, a.minpoly.data, t1.data);
        fmpz_divexact(ys.data->array + 0, res.data, a2.data->coeffs + 0);

        for (slong i = 1; i <= td; i++)
        {
            _fmpz_poly_taylor_shift(t1.data->coeffs, eget_cint_data(-1), ad + 1);
            fmpz_set_si(xs.data->array + 2*i - 1, i);
            fmpz_poly_resultant(res.data, a.minpoly.data, t1.data);
            fmpz_poly_evaluate_fmpz(u.data, a2.data, xs.data->array + 2*i - 1);
            fmpz_divexact(ys.data->array + 2*i - 1, res.data, u.data);

            _fmpz_poly_taylor_shift(t2.data->coeffs, eget_cint_data(+1), ad + 1);
            fmpz_set_si(xs.data->array + 2*i - 0, -i);
            fmpz_poly_resultant(res.data, a.minpoly.data, t2.data);
            fmpz_poly_evaluate_fmpz(u.data, a2.data, xs.data->array + 2*i - 0);
            fmpz_divexact(ys.data->array + 2*i - 0, res.data, u.data);
        }

        // t1 is a square TODO how to get its sqrt better
        fmpz_poly_interpolate_fmpz_vec(t1.data, xs.data->array, ys.data->array, 2*td + 1);
    }

timeit_stop(timer);
flint_printf("poly re time: %wd\n", timer->wall);

    select_root(t1.data, true, realpart_refiner(a));

    apply_moebius(*this, eget_cint_data(1), eget_cint_data(0),
                         eget_cint_data(0), eget_cint_data(2));

std::cout << "realpart return: " << tostring() << std::endl;

    return true;
}




/************* a*b ***************/

class mul_refiner {
    xacb_t a, b, ta, tb;
    xmag_t ae, be;
    const fmpz_poly_struct * aminpoly, * bminpoly;
    bool first;

public:

    mul_refiner(const qbarelem & A, const qbarelem & B)
    {
        first = true;
        aminpoly = A.minpoly.data;
        bminpoly = B.minpoly.data;
        acb_set(a.data, A.location.data);
        acb_set(b.data, B.location.data);
        mag_set(ae.data, A.epsilon.data);
        mag_set(be.data, B.epsilon.data);
    }

    void operator() (acb_t x, bool need_containment)
    {
        if (!first)
        {
            newton_step(aminpoly, a.data, ae.data);
            newton_step(bminpoly, b.data, be.data);
            first = false;
        }

        slong p = binary_wp(a.data, b.data);

        if (need_containment)
        {
            acb_set_radius(ta.data, a.data, ae.data);
            acb_set_radius(tb.data, b.data, be.data);
            acb_mul(x, ta.data, tb.data, p);
        }
        else
        {
            acb_mul(x, a.data, b.data, p);
        }
    }
};


bool qbarelem::mul(const qbarelem & a, const qbarelem & b)
{
std::cout << "++++++++++++++++++++++++++++++++++++" << std::endl;
std::cout << "mul " << a.tostring() << std::endl;
std::cout << "  * " << b.tostring() << std::endl;

    if (fmpz_poly_length(a.minpoly.data) <= 2)
    {
        xfmpz_t m11(a.minpoly.data->coeffs + 0);
        fmpz_neg(m11.data, m11.data);
        return apply_moebius(b, m11.data, eget_cint_data(0),
                                eget_cint_data(0), a.minpoly.data->coeffs + 1);
    }

    if (fmpz_poly_length(b.minpoly.data) <= 2)
    {
        xfmpz_t m11(b.minpoly.data->coeffs + 0);
        fmpz_neg(m11.data, m11.data);
        return apply_moebius(a, m11.data, eget_cint_data(0),
                                eget_cint_data(0), b.minpoly.data->coeffs + 1);
    }

    xfmpz_poly_t t;

timeit_t timer;
timeit_start(timer);

    if (algo == Algo_pgcd)
    {
		rfmpz_poly_t r;
        sparse_poly<xfmpz_poly_t> A, B;
        set_univar<rfmpz_poly_t>(A, a.minpoly, r);
        set_univar_scale(B, b.minpoly, -1);
        resultant<rfmpz_poly_t>(t, A, B, r);
    }
    else
    {
        x_fmpz_vector_t xs, ys;
        xfmpz_t acc;
        slong ad = fmpz_poly_degree(a.minpoly.data);
        slong bd = fmpz_poly_degree(b.minpoly.data);
        slong rd = ad*bd;

        _fmpz_vector_fit_length(xs.data, rd + 1);
        _fmpz_vector_fit_length(ys.data, rd + 1);
        fmpz_poly_fit_length(t.data, bd + 1);
        _fmpz_poly_set_length(t.data, bd + 1);

        for (slong i = 0; i <= rd; i++)
        {
            slong scale = (i & 1) ? -(i/2 + 1) : i/2 + 1;
            fmpz_set_si(xs.data->array + i, scale);
            fmpz_one(acc.data);
            for (slong j = 0; j <= bd; j++)
            {
                fmpz_mul(t.data->coeffs + bd - j, b.minpoly.data->coeffs + j, acc.data);
                fmpz_mul_si(acc.data, acc.data, scale);
            }
            fmpz_poly_resultant(ys.data->array + i, a.minpoly.data, t.data);
        }

        fmpz_poly_interpolate_fmpz_vec(t.data, xs.data->array, ys.data->array, rd + 1);
    }

timeit_stop(timer);
flint_printf("poly * time: %wd\n", timer->wall);

    select_root(t.data, true, mul_refiner(a, b));

std::cout << "mul return: " << tostring() << std::endl;
    return true;
}


/************* a/b ***************/

class div_refiner {
    xacb_t a, b, ta, tb;
    xmag_t ae, be;

    const fmpz_poly_struct * aminpoly, * bminpoly;
    bool first;

public:

    div_refiner(const qbarelem & A, const qbarelem & B)
    {
        first = true;
        aminpoly = A.minpoly.data;
        bminpoly = B.minpoly.data;
        acb_set(a.data, A.location.data);
        acb_set(b.data, B.location.data);
        mag_set(ae.data, A.epsilon.data);
        mag_set(be.data, B.epsilon.data);
    }
    void operator() (acb_t x, bool need_containment)
    {
    try_again:

        if (!first)
        {
            newton_step(aminpoly, a.data, ae.data);
            newton_step(bminpoly, b.data, be.data);
            first = false;
        }

        slong p = binary_wp(a.data, b.data);

        if (need_containment)
        {
            acb_set_radius(ta.data, a.data, ae.data);
            acb_set_radius(tb.data, b.data, be.data);
            acb_div(x, ta.data, tb.data, p);
        }
        else
        {
            acb_div(x, a.data, b.data, p);
        }

        if (!acb_is_finite(x))
            goto try_again;
    }
};


bool qbarelem::div(const qbarelem & a, const qbarelem & b)
{
//std::cout << "++++++++++++++++++++++++++++++++++++" << std::endl;
//std::cout << "mul " << a.tostring() << " * " << b.tostring() << std::endl;

    if (fmpz_poly_length(a.minpoly.data) <= 2)
    {
        xfmpz_t m11(a.minpoly.data->coeffs + 0);
        fmpz_neg(m11.data, m11.data);
        return apply_moebius(b, eget_cint_data(0),          m11.data,
                                a.minpoly.data->coeffs + 1, eget_cint_data(0));
    }

    if (fmpz_poly_length(b.minpoly.data) <= 2)
    {
        xfmpz_t m11(b.minpoly.data->coeffs + 0);
        fmpz_neg(m11.data, m11.data);
        return apply_moebius(a, b.minpoly.data->coeffs + 1, eget_cint_data(0),
                                eget_cint_data(0),          m11.data);
    }

    xfmpz_poly_t t;
	rfmpz_poly_t r;
    sparse_poly<xfmpz_poly_t> A, B;
    set_univar<rfmpz_poly_t>(A, a.minpoly, r);
    set_univar_scale(B, b.minpoly, +1);
    resultant<rfmpz_poly_t>(t, A, B, r);

    select_root(t.data, true, div_refiner(a, b));

//std::cout << "div return: " << tostring() << std::endl;
    return true;
}


/************ a^(b/c) **************/
class pow_refiner {
    xacb_t a;
    xmag_t ae;
    const fmpz_poly_struct * aminpoly;
    slong b;
    ulong c;
    xfmpq_t pow;
    bool first;

public:

    pow_refiner(const qbarelem & A, slong B, ulong C)
    {
        b = B;
        c = C;
        first = true;
        aminpoly = A.minpoly.data;
        fmpz_set_si(fmpq_numref(pow.data), B);
        fmpz_set_ui(fmpq_denref(pow.data), C);
        acb_set(a.data, A.location.data);
        mag_set(ae.data, A.epsilon.data);
    }

    void operator() (acb_t x, bool need_containment)
    {
        xacb_t ta, s;
        bool ok;

    try_again:

        if (!first)
        {
            newton_step(aminpoly, a.data, ae.data);
            first = false;
        }

        slong p = unary_wp(a.data);

        /* ta is our ball containing a unique root of aminpoly */
        acb_set_radius(ta.data, a.data, ae.data);

        if (c < 2)
        {
            /* no shenanigans because a -> a^b is continuous */
            if (need_containment)
                acb_pow_si(x, ta.data, b, p);
            else
                acb_pow_si(x, a.data, b, p);
            return;
        }

        if (acb_contains_zero(ta.data))
            goto try_again;

        if (arb_is_zero(acb_imagref(ta.data)))
        {
    try_real:
            assert(arb_is_zero(acb_imagref(ta.data)));

            if (arb_is_positive(acb_realref(ta.data)))
            {
                arb_pow_fmpq(acb_realref(x), acb_realref(ta.data), pow.data, p);
                arb_zero(acb_imagref(x));
            }
            else if (arb_is_negative(acb_realref(ta.data)))
            {
                arb_sin_cos_pi_fmpq(acb_imagref(s.data), acb_realref(s.data), pow.data, p);
                arb_abs(acb_realref(ta.data), acb_realref(ta.data));
                arb_pow_fmpq(acb_realref(ta.data), acb_realref(ta.data), pow.data, p);
                arb_mul(acb_realref(x), acb_realref(s.data), acb_realref(ta.data), p);
                arb_mul(acb_imagref(x), acb_imagref(s.data), acb_realref(ta.data), p);
            }
            else
            {
                /* hmm, ta was supposed to not contain zero */
                goto try_again;
            }
        }
        else if (!arb_contains_zero(acb_imagref(ta.data)) || arb_is_positive(acb_realref(ta.data)))
        {
            /* if this is not good enough this time, eventually it will be */
            acb_root_ui(s.data, ta.data, c, p);
            acb_pow_si(x, s.data, b, p);
        }
        else
        {
            /* intersect ta with real axis and see if this real ball is ok */
            acb_chop_imag(ta.data);
            if (newton_test(aminpoly, ta.data, arb_radref(acb_realref(s.data))))
                goto try_real;
            else
                goto try_again;
        }

        /* may have had trouble if relative precision is too low */
        if (!acb_is_finite(x))
            goto try_again;
    }
};

bool qbarelem::pow(const qbarelem & a, slong b, ulong c)
{
std::cout << "++++++++++++++++++++++++++++++++++++" << std::endl;
std::cout << "pow: " << a.tostring() << "^(" << b << "/" << c << ")" << std::endl;

    assert(c > 0);
    if (fmpz_is_zero(a.minpoly.data->coeffs + 0))
    {
        set(slong(0));
        return b > 0;
    }
    if (b == 0)
    {
        set(slong(1));
        return true;
    }

timeit_t timer;
timeit_start(timer);

    xfmpz_poly_t t;
    slong ad = fmpz_poly_degree(a.minpoly.data);
    slong rd = ad*c;
    slong absb = std::abs(b);

    if (b == 1)
    {
        fmpz_poly_inflate(t.data, a.minpoly.data, c);
    }
    else if (b == -1)
    {
        fmpz_poly_reverse(t.data, a.minpoly.data, ad + 1);
        fmpz_poly_inflate(t.data, t.data, c);
    }
    else if (algo == Algo_pgcd)
    {
		rfmpz_poly_t r;
        sparse_poly<xfmpz_poly_t> P, Q;
        Q.fit_length(2);
        Q.length = 2;
        Q.exps[0] = absb;
        Q.exps[1] = 0;
        fmpz_poly_one(Q.coeffs[0].data);
        fmpz_poly_neg(Q.coeffs[1].data, Q.coeffs[0].data);
        fmpz_poly_shift_left(Q.coeffs[b > 0].data, Q.coeffs[b > 0].data, c);
        set_univar<rfmpz_poly_t>(P, a.minpoly, r);
        resultant<rfmpz_poly_t>(t, P, Q, r);
    }
    else
    {
        x_fmpz_vector_t xs, ys;
        _fmpz_vector_fit_length(xs.data, rd + 1);
        _fmpz_vector_fit_length(ys.data, rd + 1);
        fmpz_poly_fit_length(t.data, absb + 1);
        _fmpz_poly_set_length(t.data, absb + 1);
        for (slong i = 0; i <= absb; i++)
            fmpz_zero(t.data->coeffs + i);

        for (slong i = 0; i <= rd; i++)
        {
            fmpz_set_si(xs.data->array + i, i + 1);
            if (b > 0)
            {
                fmpz_set_si(t.data->coeffs + absb, -1);
                fmpz_pow_ui(t.data->coeffs + 0, xs.data->array + i, c);
            }
            else
            {
                fmpz_set_si(t.data->coeffs + 0, -1);
                fmpz_pow_ui(t.data->coeffs + absb, xs.data->array + i, c);
            }
            fmpz_poly_resultant(ys.data->array + i, a.minpoly.data, t.data);
        }

        fmpz_poly_interpolate_fmpz_vec(t.data, xs.data->array, ys.data->array, rd + 1);
    }

timeit_stop(timer);
flint_printf("poly ^ time: %wd\n", timer->wall);

    select_root(t.data, c > 1 || absb > 1, pow_refiner(a, b, c));

std::cout << "pow return: " << tostring() << std::endl;
    return true;
}


/********* a +- I*Sqrt[1 - a^2] **************/

class circle_root_refiner {
    xacb_t a;
    xmag_t ae;
    const fmpz_poly_struct * aminpoly;
    int sign;
    bool first;

public:

    circle_root_refiner(const qbarelem & A, int sign_)
    {
        sign = sign_;
        first = true;
        aminpoly = A.minpoly.data;
        acb_set(a.data, A.location.data);
        mag_set(ae.data, A.epsilon.data);
    }

    void operator() (acb_t x, bool need_containment)
    {
        xacb_t ta, s;
        bool ok;

    try_again:

        if (!first)
            newton_step(aminpoly, a.data, ae.data);

        first = false;

        slong p = unary_wp(a.data);

        /* ta is our ball containing a unique root of aminpoly */
        acb_set_radius(ta.data, a.data, ae.data);

        /* nasty shit because the damn function has a discontinuity along Abs[Re[a]] > 1 */

        if (arb_is_zero(acb_imagref(ta.data)))
        {
    try_real:
            assert(arb_is_zero(acb_imagref(ta.data)));

            arb_sqr(acb_realref(s.data), acb_realref(ta.data), p);
            arb_sub_ui(acb_realref(s.data), acb_realref(s.data), 1, p);
            arb_neg(acb_realref(s.data), acb_realref(s.data));

            if (arb_is_positive(acb_realref(s.data)))
            {
                arb_sqrt(acb_imagref(s.data), acb_realref(s.data), p);
                arb_zero(acb_realref(s.data));
            }
            else if (arb_is_negative(acb_realref(s.data)))
            {
                arb_neg(acb_realref(s.data), acb_realref(s.data));
                arb_sqrt(acb_realref(s.data), acb_realref(s.data), p);
                arb_neg(acb_realref(s.data), acb_realref(s.data));
                arb_zero(acb_imagref(s.data));
            }
            else
            {
                goto try_again;
            }
        }
        else if (!arb_contains_zero(acb_imagref(ta.data)))
        {
            /* if this is not good enough this time, eventually it will be */
            acb_sqr(s.data, ta.data, p);
            arb_sub_ui(acb_realref(s.data), acb_realref(s.data), 1, p);
            acb_neg(s.data, s.data);
            acb_sqrt(s.data, s.data, p);
            acb_mul_onei(s.data, s.data);
        }
        else
        {
            /* intersect ta with real axis and see if this real ball is ok */
            acb_chop_imag(ta.data);
            if (newton_test(aminpoly, ta.data, arb_radref(acb_realref(s.data))))
                goto try_real;
            else
                goto try_again;
        }

        if (sign > 0)
            acb_add(x, ta.data, s.data, p);
        else
            acb_sub(x, ta.data, s.data, p);

        /* may have had trouble if relative precision is too low */
        if (!acb_is_finite(x))
            goto try_again;
    }
};

void qbarelem::quadraticroot(const fmpz_t a, const fmpz_t b, const fmpz_t c)
{
    xfmpz_t d, r, s;

    fmpz_mul(d.data, a, a);
    fmpz_mul(s.data, b, c);
    fmpz_submul_ui(d.data, s.data, 4);

    if (fmpz_sgn(d.data) >= 0)
    {
        fmpz_sqrtrem(s.data, r.data, d.data);
        if (fmpz_is_zero(r.data))
        {
            fmpz_sub(s.data, s.data, r.data);
            fmpz_mul_2exp(r.data, a, 1);
            div(s.data, r.data);
            return;
        }
    }

    fmpz_poly_fit_length(minpoly.data, 3);
    fmpz_set(minpoly.data->coeffs + 0, c);
    fmpz_set(minpoly.data->coeffs + 1, b);
    fmpz_set(minpoly.data->coeffs + 2, a);
    _fmpz_poly_set_length(minpoly.data, 3);
    _fmpz_poly_primitive_part(minpoly.data->coeffs, minpoly.data->coeffs, 3);

    flint_bitcnt_t d_bits = fmpz_bits(d.data)/2;
    flint_bitcnt_t a_bits = fmpz_bits(a);
    flint_bitcnt_t b_bits = fmpz_bits(b);
    b_bits = std::max(b_bits, d_bits);
    if (b_bits > a_bits + d_bits)
        b_bits -= a_bits + d_bits;

    slong p = b_bits > a_bits + d_bits ? b_bits - (a_bits + d_bits) : b_bits;

try_again:

    p += FLINT_BITS;

    if (fmpz_sgn(d.data) >= 0)
    {
        arb_sqrt_fmpz(acb_realref(location.data), d.data, p);
        arb_sub_fmpz(acb_realref(location.data), acb_realref(location.data), b, p);
        arb_div_fmpz(acb_realref(location.data), acb_realref(location.data), a, p);
        arb_mul_2exp_si(acb_realref(location.data), acb_realref(location.data), -1);
        arb_zero(acb_imagref(location.data));
    }
    else
    {
        fmpz_neg(d.data, d.data);
        arb_sqrt_fmpz(acb_imagref(location.data), d.data, p);
        arb_div_fmpz(acb_imagref(location.data), acb_imagref(location.data), a, p);
        arb_fmpz_div_fmpz(acb_realref(location.data), b, a, p);
        arb_neg(acb_realref(location.data), acb_realref(location.data));
        acb_mul_2exp_si(location.data, location.data, -1);        
    }

    if (!newton_test(minpoly.data, location.data, epsilon.data))
        goto try_again;
}

bool qbarelem::circle_root(const qbarelem & a, int sign)
{
//std::cout << "++++++++++++++++++++++++++++++++++++" << std::endl;
//std::cout << "circleroot: " << a.tostring() << std::endl;

    if (fmpz_poly_length(a.minpoly.data) <= 2)
    {
        xfmpz_t a0(a.minpoly.data->coeffs + 0);
        xfmpz_t a1(a.minpoly.data->coeffs + 1);
        fmpz_mul_2exp(a0.data, a0.data, 1);

        if (fmpz_cmpabs(a.minpoly.data->coeffs + 0, a.minpoly.data->coeffs + 1) >= 0)
        {
            fmpz_neg(a0.data, a0.data);
            fmpz_neg(a1.data, a1.data);
        }

        if (sign < 0)
        {
            fmpz_neg(a0.data, a0.data);
            fmpz_neg(a1.data, a1.data);
        }

        quadraticroot(a1.data, a0.data, a1.data);
        return true;
    }

    xfmpz_poly_t t;

	rfmpz_poly_t r;
    sparse_poly<xfmpz_poly_t> P, Q;
    Q.fit_length(2);
    Q.length = 2;
    Q.exps[0] = 1;
    Q.exps[1] = 0;
    fmpz_poly_zero(Q.coeffs[0].data);
    fmpz_poly_set_coeff_si(Q.coeffs[0].data, 1, -2);
    fmpz_poly_zero(Q.coeffs[1].data);
    fmpz_poly_set_coeff_si(Q.coeffs[1].data, 0, 1);
    fmpz_poly_set_coeff_si(Q.coeffs[1].data, 2, 1);
    set_univar<rfmpz_poly_t>(P, a.minpoly, r);
    resultant<rfmpz_poly_t>(t, P, Q, r);

    select_root(t.data, true, circle_root_refiner(a, sign));

//std::cout << "circleroot return: " << tostring() << std::endl;
    return true;
}




/********* apply rat fxn **************/

class apply_rat_fxn_refiner {
    xacb_t a;
    xmag_t ae;
    const fmpz_poly_struct * aminpoly;
    const fmpz * num; slong num_length;
    const fmpz * den; slong den_length;
    bool first;

public:

    apply_rat_fxn_refiner(const qbarelem & A, const fmpz * num_, slong num_length_,
                                              const fmpz * den_, slong den_length_)
    {
        first = true;
        aminpoly = A.minpoly.data;
        acb_set(a.data, A.location.data);
        mag_set(ae.data, A.epsilon.data);
        num = num_;
        den = den_;
        num_length = num_length_;
        den_length = den_length_;
    }

    void operator() (acb_t x, bool need_containment)
    {
        xacb_t ta, u, v;

    try_again:

        if (!first)
        {
            newton_step(aminpoly, a.data, ae.data);
            first = false;
        }

        slong p = unary_wp(a.data);

        if (need_containment)
        {
            acb_set_radius(ta.data, a.data, ae.data);
            _arb_fmpz_poly_evaluate_acb(u.data, num, num_length, ta.data, p);
            _arb_fmpz_poly_evaluate_acb(v.data, den, den_length, ta.data, p);
        }
        else
        {
            _arb_fmpz_poly_evaluate_acb(u.data, num, num_length, a.data, p);
            _arb_fmpz_poly_evaluate_acb(v.data, den, den_length, a.data, p);
        }

        acb_div(x, u.data, v.data, p);
        if (!acb_is_finite(x))
            goto try_again;
    }
};

void qbarelem::apply_rat_fxn(const qbarelem & a, const fmpz * num, slong num_length,
                                                 const fmpz * den, slong den_length)
{
    xfmpz_poly_t t;

	rfmpz_poly_t r;
    sparse_poly<xfmpz_poly_t> A, B;

    set_univar<rfmpz_poly_t>(A, a.minpoly, r);

    slong max_length = std::max(num_length, den_length);

    B.fit_length(max_length);
    B.length = 0;
    for (slong i = max_length - 1; i >= 0; i--)
    {
        B.exps[B.length] = i;
        fmpz_poly_zero(B.coeffs[B.length].data);
        if (i < num_length)
            fmpz_poly_set_coeff_fmpz(B.coeffs[B.length].data, 0, num + i);
        fmpz_poly_neg(B.coeffs[B.length].data, B.coeffs[B.length].data);
        if (i < den_length)
            fmpz_poly_set_coeff_fmpz(B.coeffs[B.length].data, 1, den + i);
        B.length += !fmpz_poly_is_zero(B.coeffs[B.length].data);
    }

    resultant<rfmpz_poly_t>(t, A, B, r);
    select_root(t.data, true, apply_rat_fxn_refiner(a, num, num_length, den, den_length));
}



/***************************************************************/

bool eget_fmpq_poly(fmpq_poly_t q, er f, er v)
{
    poly p(1);
    wex vars(emake_node(gs.sym_sList.copy(), ecopy(v)));

    if (!ex_to_polynomial(p, f, vars.get()))
    {
        return false;
    }

    xfmpq_t ai;
    slong ei;

    fmpq_poly_zero(q);

    for (ulong i = 0; i < p.coeffs.size(); i++)
    {
        if (!eget_fmpq(ai.data, p.coeffs[i].get()) ||
            COEFF_IS_MPZ(*p.exps[i].data) ||
            (ei = *p.exps[i].data, ei < 0))
        {
            return false;
        }

        fmpq_poly_set_coeff_fmpq(q, ei, ai.data);
    }

    return true;
}

bool eget_arb(arb_t x, er e)
{
//std::cout << "eget_arb e: " << ex_tostring_full(e) << std::endl;

    if (eis_double(e))
    {
        arb_set_d(x, edouble_number(e));
        arb_limit_precision(x, 53);
        return true;
    }
    else if (eis_real(e))
    {
        arb_set(x, ereal_data(e));
        return true;
    }
    else
    {
        return false;
    }
}

bool eget_acb(acb_t x, er e)
{
//std::cout << "eget_acb e: " << ex_tostring_full(e) << std::endl;

    if (eis_cmplx(e))
    {
        return eget_arb(acb_realref(x), ecmplx_real(e)) &&
               eget_arb(acb_imagref(x), ecmplx_imag(e));
    }
    else
    {
        arb_zero(acb_imagref(x));
        return eget_arb(acb_realref(x), e);
    }
}


ex qbarelem::get_ex() const
{
    std::vector<wex> p;
    wex slot(emake_node(gs.sym_sSlot.copy(), emake_cint(1)));
    uex number;

    if (minpoly.data->length <= 2)
    {
        ex r = emake_rat();
        fmpz_neg(fmpq_numref(erat_data(r)), minpoly.data->coeffs + 0);
        fmpz_set(fmpq_denref(erat_data(r)), minpoly.data->coeffs + 1);
        return efix_rat(r);
    }

    if (!arb_is_zero(acb_realref(location.data)))
    {
        number.setz(emake_real_copy(acb_realref(location.data)));
    }
    else
    {
        number.setz(emake_cint(0));
    }

    if (!arb_is_zero(acb_imagref(location.data)))
    {
        number.setz(emake_cmplx(number.release(),
                               emake_real_copy(acb_imagref(location.data))));
    }

    for (slong i = 0; i < minpoly.data->length; i++)
    {
        if (fmpz_is_zero(minpoly.data->coeffs + i))
            continue;
        ex t;
        if (i > 0)
        {
            t = emake_node(gs.sym_sPower.copy(), slot.copy(), emake_int_si(i));
            if (!fmpz_is_one(minpoly.data->coeffs + i))
            {
                t = emake_node(gs.sym_sTimes.copy(),
                        emake_int_copy(minpoly.data->coeffs + i), t);
            }
        }
        else
        {
            t = emake_int_copy(minpoly.data->coeffs + i);
        }
        p.push_back(wex(t));
    }

    return emake_node(gs.sym_sRoot.copy(), emake_node(gs.sym_sList.copy(),
                emake_node(gs.sym_sFunction.copy(),
                    emake_node(gs.sym_sPlus.copy(), p)
                ),
                number.release()
           ));
}


// for Exp[f*e]  where f is a number
bool qbarelem::set_exp_ex(er f, er e)
{
//std::cout << "set_exp_ex called e: " << ex_tostring_full(e) << std::endl;
//std::cout << "                  f: " << ex_tostring_full(f) << std::endl;

    xfmpq_t a;
    qbarelem t, s;

    if (ehas_head_sym_length(e, gs.sym_sTimes.get(), 2) && eis_finite_number(echild(e,1)))
    {
        wex p(num_Times2(f, echild(e,1)));
        er e1 = p.get();
        er e2 = echild(e,2);
        if (ehas_head_sym_length(e2, gs.sym_sArcCos.get(), 1))
        {
            if (eis_zero(ecmplx_real(e1)) && eget_fmpq(a.data, ecmplx_imag(e1)))
            {
                return t.set_ex(echild(e2,1)) &&
                       t.circle_root(t, +1) &&
                       pow(t, a.data);
            }
        }
        else if (ehas_head_sym_length(e2, gs.sym_sArcSin.get(), 1))
        {
            if (eis_zero(ecmplx_real(e1)) && eget_fmpq(a.data, ecmplx_imag(e1)))
            {
                return t.set_ex(echild(e2,1)) &&
                       t.circle_root(t, -1) &&
                       t.mul_expipi(t, eget_crat_data(1, 2)) &&
                       pow(t, a.data);
            }
        }
        else if (ehas_head_sym_length(e2, gs.sym_sArcTan.get(), 1))
        {
            if (eis_zero(ecmplx_real(e1)) && eget_fmpq(a.data, ecmplx_imag(e1)))
            {
                fmpq_div_2exp(a.data, a.data, 1);
                return t.set_ex(echild(e2,1)) &&
                       t.mul_expipi(t, eget_crat_data(1, 2)) &&
                       s.apply_moebius(t, eget_cint_data(-1), eget_cint_data(1),
                                          eget_cint_data(0), eget_cint_data(1)) &&
                       t.apply_moebius(t, eget_cint_data(+1), eget_cint_data(1),
                                          eget_cint_data(0), eget_cint_data(1)) &&
                       t.pow(t, a.data) &&
                       (fmpq_neg(a.data, a.data), s.pow(s, a.data)) &&
                       mul(s, t);
            }
        }
    }

    return false;
}

bool qbarelem::set_ex(er e)
{
//std::cout << "set_ex called e: " << ex_tostring_full(e) << std::endl;

    if (eis_int(e))
    {
        set(eint_data(e));
        return true;
    }
    else if (eis_rat(e))
    {
        set(erat_data(e));
        return true;
    }
    else if (!eis_node(e))
    {
        return false;
    }

    qbarelem s, t;

    if (ehas_head_sym(e, gs.sym_sPlus.get()))
    {
        set(slong(0));
        for (ulong i = 0; i < elength(e); i++)
        {
            if (!t.set_ex(echild(e, i + 1)))
                return false;
            swap(s);
            add(s, t);
        }
        return true;
    }
    else if (ehas_head_sym(e, gs.sym_sTimes.get()))
    {
        set(slong(1));
        for (ulong i = 0; i < elength(e); i++)
        {
            if (!t.set_ex(echild(e, i + 1)))
                return false;
            swap(s);
            mul(s, t);
        }
        return true;
    }
    else if (ehas_head_sym_length(e, gs.sym_sPower.get(), 2))
    {
        if (eis_int(echild(e,2)))
        {
            return t.set_ex(echild(e,1)) && pow(t, eint_data(echild(e,2)));
        }
        else if (eis_rat(echild(e,2)))
        {
            return t.set_ex(echild(e,1)) && pow(t, erat_data(echild(e,2)));
        }
        else if (eis_sym(echild(e,1), gs.sym_sE.get()))
        {
            return set_exp_ex(eget_cint(1), echild(e,2));
        }
        else 
        {
            return false;
        }
    }
    else if (ehas_head_sym_length(e, gs.sym_sRe.get(), 1))
    {
        if (!set_ex(echild(e,1)))
            return false;
        return realpart(*this);
    }
    else if (ehas_head_sym_length(e, gs.sym_sExp.get(), 1))
    {
        return set_exp_ex(eget_cint(1), echild(e,1));
    }
    else if (ehas_head_sym_length(e, gs.sym_sCos.get(), 1))
    {
        if (!set_exp_ex(gs.const_i.get(), echild(e,1)))
            return false;
        fmpz num[3] = {1, 0, 1};
        fmpz den[2] = {0, 2};
        apply_rat_fxn(*this, num, 3, den, 2);
        return true;
    }
    else if (ehas_head_sym_length(e, gs.sym_sRoot.get(), 1) &&
             ehas_head_sym_length(echild(e,1), gs.sym_sList.get(), 2))
    {
        er body;
        er func = echild(e,1,1);
        wex var(emake_node(gs.sym_sSlot.copy(), emake_cint(1)));
        xfmpq_poly_t q;
        xacb_t x;

        if (elength(func) == 1)
        {
            body = echild(func, 1);
        }
        else if (elength(func) == 2)
        {
            var.reset(ecopychild(func, 1));
            body = echild(func, 2);            
        }
        else
        {
            return false;
        }

        if (!eget_fmpq_poly(q.data, body, var.get()))
            return false;

        if (!eget_acb(x.data, echild(e,1,2)))
            return false;

        return set(q.zpoly(), x.data);
    }

    return false;
}


ex rootreduce(er e)
{
    qbarelem r;

//std::cout << "rootreduce called e: " << ex_tostring(e) << std::endl;

    if (r.set_ex(e))
        return r.get_ex();

    if (!eis_node(e))
        return ecopy(e);

    if (ehas_head_sym(e, gs.sym_sList.get()) || ehas_head_sym(e, gs.sym_sRule.get()))
    {
        uex a(echild(e,0), elength(e));
        bool changed = false;
        for (ulong i = 0; i < elength(e); i++)
        {
            er ei = echild(e, i + 1);
            ex f = rootreduce(ei);
            changed = changed || (etor(f) != ei);
            a.push_back(f);
        }
        return changed ? a.release() : ecopy(e);
    }

    return ecopy(e);
}


ex dcode_sRootReduce(er e)
{
//std::cout << "dcode_sRootReduce: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sRootReduce.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    return rootreduce(echild(e,1));
}





static ex _galois_group_order_irr3(xfmpq_poly_t &p)
{
    assert(fmpq_poly_degree(p.data) == 3);
    xfmpz_t disc;
    fmpz_poly_discriminant(disc.data, p.zpoly());
    return fmpz_is_square(disc.data) ? emake_cint(3) : emake_cint(6);
}

static ex _galois_group_order_irr4(xfmpq_poly_t &p)
{
    assert(fmpq_poly_degree(p.data) == 4);

    xfmpq_t A, B, C, b0, b1, b2, b3, b3b3, u, v, w;

    /* b3 = c3/(4c4), b2 = c2/(4c4), b1 = c1/(8c4), b0 = c0/(4c4)*/
    fmpq_set_fmpz_frac(b0.data, p.data->coeffs + 0, p.data->coeffs + 4);
    fmpq_set_fmpz_frac(b1.data, p.data->coeffs + 1, p.data->coeffs + 4);
    fmpq_set_fmpz_frac(b2.data, p.data->coeffs + 2, p.data->coeffs + 4);
    fmpq_set_fmpz_frac(b3.data, p.data->coeffs + 3, p.data->coeffs + 4);
    fmpq_div_2exp(b0.data, b0.data, 2);
    fmpq_div_2exp(b1.data, b1.data, 3);
    fmpq_div_2exp(b2.data, b2.data, 2);
    fmpq_div_2exp(b3.data, b3.data, 2);

    fmpq_pow_si(b3b3.data, b3.data, 2);

    /* A = b1 - b2 b3 + b3^3 */
    fmpq_sub(v.data, b3b3.data, b2.data);
    fmpq_mul(u.data, b3.data, v.data);
    fmpq_add(A.data, b1.data, u.data);

    /* B = -2 b2 + 3 b3^2 */
    fmpq_mul_fmpz(u.data, b3b3.data, eint_data(eget_cint(3)));
    fmpq_mul_2exp(v.data, b2.data, 1);
    fmpq_sub(B.data, u.data, v.data);

    /* C = -b0 + b2^2 + 2 b1 b3 - 4 b2 b3^2 + 3 b3^4 */
    fmpq_mul_fmpz(u.data, b3b3.data, eint_data(eget_cint(3)));
    fmpq_mul_2exp(v.data, b2.data, 2);
    fmpq_sub(w.data, u.data, v.data);
    fmpq_mul(u.data, b3.data, w.data);
    fmpq_mul_2exp(v.data, b1.data, 1);
    fmpq_add(w.data, v.data, u.data);
    fmpq_mul(u.data, b3.data, w.data);
    fmpq_pow_si(v.data, b2.data, 2);
    fmpq_sub(w.data, v.data, b0.data);
    fmpq_add(C.data, w.data, u.data);

std::cout << "C: " << C.tostring() << std::endl;

    uint8_t Cprog[] = {3,4,0,0,0,0,0,0,0,0,3,5,255,255,255,255,255,255,255,255,1,5,5,0,2,4,4,5,3,5,2,0,0,0,0,0,0,0,1,5,5,1,1,5,5,3,2,4,4,5,3,5,1,0,0,0,0,0,0,0,1,5,5,2,1,5,5,2,2,4,4,5,3,5,252,255,255,255,255,255,255,255,1,5,5,2,1,5,5,3,1,5,5,3,2,4,4,5,3,5,3,0,0,0,0,0,0,0,1,5,5,3,1,5,5,3,1,5,5,3,1,5,5,3,2,4,4,5};
    std::vector<xfmpq_t> stack;
    stack.resize(10);
    fmpq_set(stack[0].data, b0.data);
    fmpq_set(stack[1].data, b1.data);
    fmpq_set(stack[2].data, b2.data);
    fmpq_set(stack[3].data, b3.data);
    eval_poly_fmpq(stack, Cprog, sizeof(Cprog));
std::cout << "C: " << stack[4].tostring() << std::endl;


    if (fmpq_is_zero(A.data))
    {
        fmpq_pow_si(u.data, B.data, 2);
        fmpq_mul_2exp(v.data, C.data, 2);
        fmpq_sub(w.data, u.data, v.data);
        if (w.is_square())
        {
            return emake_cint(4); // C2 x C2
        }
        else
        {
            fmpq_mul(u.data, w.data, C.data);
            return u.is_square() ? emake_cint(4) : emake_cint(8);
        }
    }
    else
    {
        fmpq_div(u.data, C.data, A.data);
        xfmpq_poly_t q;
        fmpq_poly_set_coeff_fmpz(q.data, 3, eint_data(eget_cint(1)));
        fmpq_poly_set_coeff_fmpq(q.data, 2, u.data);
        fmpq_poly_set_coeff_fmpq(q.data, 1, B.data);
        fmpq_poly_set_coeff_fmpq(q.data, 0, A.data);

        fmpz_poly_factor_t fac;
        fmpz_poly_factor_init(fac);
        fmpz_poly_factor(fac, q.zpoly());
        std::cout << "qfac: " << std::endl;
        fmpz_poly_factor_print(fac);

        if (fac->num == 3)
        {
            fmpz_poly_factor_clear(fac);
            return emake_cint(4); // C2 x C2
        }
        else if (fac->num == 2)
        {
            assert(fmpz_poly_degree(fac->p + 0) == 1);
            assert(fmpz_poly_degree(fac->p + 1) == 2);
            fmpz * fcoeffs = (fac->p + 1)->coeffs;
            xfmpz_t c0c2, c1c1, t1, t2;
            fmpz_mul(c0c2.data, fcoeffs + 0, fcoeffs + 2);
            fmpz_mul(c1c1.data, fcoeffs + 1, fcoeffs + 1);
            fmpz_mul_2exp(t1.data, c0c2.data, 2);
            fmpz_sub(t2.data, c1c1.data, t1.data);
            fmpz_mul(t1.data, c0c2.data, t2.data);
            fmpz_poly_factor_clear(fac);
            return fmpz_is_square(t1.data) ? emake_cint(4) : emake_cint(8);            
        }
        else
        {
            assert(fac->num == 1);
            xfmpz_t disc;
            fmpz_poly_discriminant(disc.data, q.zpoly());
            fmpz_poly_factor_clear(fac);
            return fmpz_is_square(disc.data) ? emake_cint(12) : emake_cint(24);
        }
    }
}

static ex _galois_group_order_irr5(xfmpq_poly_t &p)
{
    assert(fmpq_poly_degree(p.data) == 5);
    fmpq_poly_make_monic(p.data, p.data);

    xfmpq_t disc;
    _fmpz_poly_discriminant(fmpq_numref(disc.data), p.data->coeffs, 6);
    fmpz_pow_ui(fmpq_denref(disc.data), p.data->den, 8);
    fmpq_canonicalise(disc.data);

    bool disc_is_square = disc.is_square();
//std::cout << "disc: " << disc.tostring() << std::endl;

    xfmpq_poly_t q, r;
    xfmpq_t t;
    fmpq_poly_get_coeff_fmpq(t.data, p.data, 4);
    fmpq_div_fmpz(t.data, t.data, eint_data(eget_cint(-5)));
    fmpq_poly_set_coeff_fmpz(r.data, 1, eint_data(eget_cint(1)));
    fmpq_poly_set_coeff_fmpq(r.data, 0, t.data);
    fmpq_poly_compose(q.data, p.data, r.data);

//std::cout << "q: " << q.tostring() << std::endl;

    std::vector<xfmpq_t> stack(20);
    fmpq_poly_get_coeff_fmpq(stack[0].data, q.data, 3); // a2
    fmpq_poly_get_coeff_fmpq(stack[1].data, q.data, 2); // a3
    fmpq_poly_get_coeff_fmpq(stack[2].data, q.data, 1); // a4
    fmpq_poly_get_coeff_fmpq(stack[3].data, q.data, 0); // a5

    /* compute 6th degree resolvent with coefficient rules

        {{6} -> 1, {5} -> 8*a4, {4} -> 2*a2*a3^2 - 6*a2^2*a4 + 
           40*a4^2 - 50*a3*a5, {3} -> -2*a3^4 + 21*a2*a3^2*a4 - 
           40*a2^2*a4^2 + 160*a4^3 - 15*a2^2*a3*a5 - 400*a3*a4*a5 + 
           125*a2*a5^2, {2} -> a2^2*a3^4 - 6*a2^3*a3^2*a4 - 
           8*a3^4*a4 + 9*a2^4*a4^2 + 76*a2*a3^2*a4^2 - 
           136*a2^2*a4^3 + 400*a4^4 - 50*a2*a3^3*a5 + 
           90*a2^2*a3*a4*a5 - 1400*a3*a4^2*a5 + 625*a3^2*a5^2 + 
           500*a2*a4*a5^2, {1} -> -2*a2*a3^6 + 19*a2^2*a3^4*a4 - 
           51*a2^3*a3^2*a4^2 + 3*a3^4*a4^2 + 32*a2^4*a4^3 + 
           76*a2*a3^2*a4^3 - 256*a2^2*a4^4 + 512*a4^5 - 
           31*a2^3*a3^3*a5 - 58*a3^5*a5 + 117*a2^4*a3*a4*a5 + 
           105*a2*a3^3*a4*a5 + 260*a2^2*a3*a4^2*a5 - 
           2400*a3*a4^3*a5 - 108*a2^5*a5^2 - 325*a2^2*a3^2*a5^2 + 
           525*a2^3*a4*a5^2 + 2750*a3^2*a4*a5^2 - 500*a2*a4^2*a5^2 + 
           625*a2*a3*a5^3 - 3125*a5^4, 
         {0} -> a3^8 - 13*a2*a3^6*a4 + a2^5*a3^2*a4^2 + 
           65*a2^2*a3^4*a4^2 - 4*a2^6*a4^3 - 128*a2^3*a3^2*a4^3 + 
           17*a3^4*a4^3 + 48*a2^4*a4^4 - 16*a2*a3^2*a4^4 - 
           192*a2^2*a4^5 + 256*a4^6 - 4*a2^5*a3^3*a5 - 
           12*a2^2*a3^5*a5 + 18*a2^6*a3*a4*a5 + 12*a2^3*a3^3*a4*a5 - 
           124*a3^5*a4*a5 + 196*a2^4*a3*a4^2*a5 + 
           590*a2*a3^3*a4^2*a5 - 160*a2^2*a3*a4^3*a5 - 
           1600*a3*a4^4*a5 - 27*a2^7*a5^2 - 150*a2^4*a3^2*a5^2 - 
           125*a2*a3^4*a5^2 - 99*a2^5*a4*a5^2 - 
           725*a2^2*a3^2*a4*a5^2 + 1200*a2^3*a4^2*a5^2 + 
           3250*a3^2*a4^2*a5^2 - 2000*a2*a4^3*a5^2 - 
           1250*a2*a3*a4*a5^3 + 3125*a2^2*a5^4 - 9375*a4*a5^4}
    */
    fmpq_poly_zero(r.data);
    fmpq_poly_set_coeff_fmpz(r.data, 6, eint_data(eget_cint(1)));
    {
    uint8_t prog5[] = {3,4,0,0,0,0,0,0,0,0,3,5,8,0,0,0,0,0,0,0,1,5,5,2,2,4,4,5};
    uint8_t prog4[] = {3,4,0,0,0,0,0,0,0,0,3,5,250,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,2,2,4,4,5,3,5,2,0,0,0,0,0,0,0,1,5,5,0,1,5,5,1,1,5,5,1,2,4,4,5,3,5,206,255,255,255,255,255,255,255,1,5,5,1,1,5,5,3,2,4,4,5,3,5,40,0,0,0,0,0,0,0,1,5,5,2,1,5,5,2,2,4,4,5};
    uint8_t prog3[] = {3,4,0,0,0,0,0,0,0,0,3,5,241,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,3,2,4,4,5,3,5,216,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,2,1,5,5,2,2,4,4,5,3,5,21,0,0,0,0,0,0,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,2,2,4,4,5,3,5,125,0,0,0,0,0,0,0,1,5,5,0,1,5,5,3,1,5,5,3,2,4,4,5,3,5,254,255,255,255,255,255,255,255,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,2,4,4,5,3,5,112,254,255,255,255,255,255,255,1,5,5,1,1,5,5,2,1,5,5,3,2,4,4,5,3,5,160,0,0,0,0,0,0,0,1,5,5,2,1,5,5,2,1,5,5,2,2,4,4,5};
    uint8_t prog2[] = {3,4,0,0,0,0,0,0,0,0,3,5,9,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,2,1,5,5,2,2,4,4,5,3,5,250,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,2,2,4,4,5,3,5,1,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,2,4,4,5,3,5,90,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,2,1,5,5,3,2,4,4,5,3,5,120,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,2,1,5,5,2,1,5,5,2,2,4,4,5,3,5,206,255,255,255,255,255,255,255,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,3,2,4,4,5,3,5,76,0,0,0,0,0,0,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,2,2,4,4,5,3,5,244,1,0,0,0,0,0,0,1,5,5,0,1,5,5,2,1,5,5,3,1,5,5,3,2,4,4,5,3,5,248,255,255,255,255,255,255,255,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,2,2,4,4,5,3,5,113,2,0,0,0,0,0,0,1,5,5,1,1,5,5,1,1,5,5,3,1,5,5,3,2,4,4,5,3,5,136,250,255,255,255,255,255,255,1,5,5,1,1,5,5,2,1,5,5,2,1,5,5,3,2,4,4,5,3,5,144,1,0,0,0,0,0,0,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,2,2,4,4,5};
    uint8_t prog1[] = {3,4,0,0,0,0,0,0,0,0,3,5,148,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,3,1,5,5,3,2,4,4,5,3,5,117,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,2,1,5,5,3,2,4,4,5,3,5,32,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,2,1,5,5,2,1,5,5,2,2,4,4,5,3,5,225,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,3,2,4,4,5,3,5,205,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,2,2,4,4,5,3,5,13,2,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,2,1,5,5,3,1,5,5,3,2,4,4,5,3,5,19,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,2,2,4,4,5,3,5,187,254,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,3,1,5,5,3,2,4,4,5,3,5,4,1,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,2,1,5,5,2,1,5,5,3,2,4,4,5,3,5,0,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,2,2,4,4,5,3,5,254,255,255,255,255,255,255,255,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,2,4,4,5,3,5,105,0,0,0,0,0,0,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,3,2,4,4,5,3,5,76,0,0,0,0,0,0,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,2,1,5,5,2,2,4,4,5,3,5,113,2,0,0,0,0,0,0,1,5,5,0,1,5,5,1,1,5,5,3,1,5,5,3,1,5,5,3,2,4,4,5,3,5,12,254,255,255,255,255,255,255,1,5,5,0,1,5,5,2,1,5,5,2,1,5,5,3,1,5,5,3,2,4,4,5,3,5,198,255,255,255,255,255,255,255,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,3,2,4,4,5,3,5,3,0,0,0,0,0,0,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,2,2,4,4,5,3,5,190,10,0,0,0,0,0,0,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,3,1,5,5,3,2,4,4,5,3,5,160,246,255,255,255,255,255,255,1,5,5,1,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,3,2,4,4,5,3,5,0,2,0,0,0,0,0,0,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,2,2,4,4,5,3,5,203,243,255,255,255,255,255,255,1,5,5,3,1,5,5,3,1,5,5,3,1,5,5,3,2,4,4,5};
    uint8_t prog0[] = {3,4,0,0,0,0,0,0,0,0,3,5,229,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,3,1,5,5,3,2,4,4,5,3,5,18,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,2,1,5,5,3,2,4,4,5,3,5,252,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,2,1,5,5,2,1,5,5,2,2,4,4,5,3,5,252,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,3,2,4,4,5,3,5,1,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,2,2,4,4,5,3,5,157,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,2,1,5,5,3,1,5,5,3,2,4,4,5,3,5,106,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,3,1,5,5,3,2,4,4,5,3,5,196,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,2,1,5,5,2,1,5,5,3,2,4,4,5,3,5,48,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,2,2,4,4,5,3,5,12,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,3,2,4,4,5,3,5,128,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,2,1,5,5,2,2,4,4,5,3,5,176,4,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,0,1,5,5,2,1,5,5,2,1,5,5,3,1,5,5,3,2,4,4,5,3,5,244,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,3,2,4,4,5,3,5,65,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,2,2,4,4,5,3,5,43,253,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,3,1,5,5,3,2,4,4,5,3,5,96,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,1,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,3,2,4,4,5,3,5,64,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,2,2,4,4,5,3,5,53,12,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,3,1,5,5,3,1,5,5,3,1,5,5,3,2,4,4,5,3,5,243,255,255,255,255,255,255,255,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,2,2,4,4,5,3,5,131,255,255,255,255,255,255,255,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,3,1,5,5,3,2,4,4,5,3,5,78,2,0,0,0,0,0,0,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,2,1,5,5,3,2,4,4,5,3,5,240,255,255,255,255,255,255,255,1,5,5,0,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,2,2,4,4,5,3,5,30,251,255,255,255,255,255,255,1,5,5,0,1,5,5,1,1,5,5,2,1,5,5,3,1,5,5,3,1,5,5,3,2,4,4,5,3,5,48,248,255,255,255,255,255,255,1,5,5,0,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,3,1,5,5,3,2,4,4,5,3,5,1,0,0,0,0,0,0,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,2,4,4,5,3,5,132,255,255,255,255,255,255,255,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,3,2,4,4,5,3,5,17,0,0,0,0,0,0,0,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,2,1,5,5,2,2,4,4,5,3,5,178,12,0,0,0,0,0,0,1,5,5,1,1,5,5,1,1,5,5,2,1,5,5,2,1,5,5,3,1,5,5,3,2,4,4,5,3,5,192,249,255,255,255,255,255,255,1,5,5,1,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,3,2,4,4,5,3,5,0,1,0,0,0,0,0,0,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,2,1,5,5,2,2,4,4,5,3,5,97,219,255,255,255,255,255,255,1,5,5,2,1,5,5,3,1,5,5,3,1,5,5,3,1,5,5,3,2,4,4,5};
    eval_poly_fmpq(stack, prog5, sizeof(prog5));
    fmpq_poly_set_coeff_fmpq(r.data, 5, stack[4].data);
    eval_poly_fmpq(stack, prog4, sizeof(prog4));
    fmpq_poly_set_coeff_fmpq(r.data, 4, stack[4].data);
    eval_poly_fmpq(stack, prog3, sizeof(prog3));
    fmpq_poly_set_coeff_fmpq(r.data, 3, stack[4].data);
    eval_poly_fmpq(stack, prog2, sizeof(prog2));
    fmpq_poly_set_coeff_fmpq(r.data, 2, stack[4].data);
    eval_poly_fmpq(stack, prog1, sizeof(prog1));
    fmpq_poly_set_coeff_fmpq(r.data, 1, stack[4].data);
    eval_poly_fmpq(stack, prog0, sizeof(prog0));
    fmpq_poly_set_coeff_fmpq(r.data, 0, stack[4].data);
    }

//std::cout << "r: " << r.tostring() << std::endl;

    fmpz_poly_factor_t fac;
    fmpz_poly_factor_init(fac);
    fmpz_poly_factor(fac, r.zpoly());
//std::cout << "r fac: " << std::endl;
//fmpz_poly_factor_print(fac);

    xfmpq_t theta;
    bool solvable = false;
    for (slong i = 0; i < fac->num; i++)
    {
        /* look for a linear^1 or linear^6 */
        if ((fac->p + i)->length == 2 && (fac->exp[i] == 1 || fac->exp[i] == 6))
        {
            solvable = true;
            fmpq_set_fmpz_frac(theta.data, (fac->p + i)->coeffs + 0, (fac->p + i)->coeffs + 1);
            fmpq_neg(theta.data, theta.data);
            break;
        }
    }
    fmpz_poly_factor_clear(fac);

    if (!solvable)
    {
        return disc_is_square ? emake_cint(60) : emake_cint(120);
    }

    if (!disc_is_square)
    {
        return emake_cint(20);
    }

    xfmpq_t disc_sqrt;
    fmpz_sqrt(fmpq_numref(disc_sqrt.data), fmpq_numref(disc.data));
    fmpz_sqrt(fmpq_denref(disc_sqrt.data), fmpq_denref(disc.data));

    /* compute 10th degree resolvent with coefficient rules

        {{10} -> 1, {8} -> 3*a2, {7} -> a3, {6} -> 3*a2^2 - 3*a4, 
         {5} -> 2*a2*a3 - 11*a5, {4} -> a2^3 - a3^2 - 2*a2*a4, 
         {3} -> a2^2*a3 - 4*a3*a4 - 4*a2*a5, 
         {2} -> -(a2*a3^2) + a2^2*a4 - 4*a4^2 + 7*a3*a5, 
         {1} -> -a3^3 - a2^2*a5 + 4*a4*a5, 
         {0} -> -(a3^2*a4) + a2*a3*a5 - a5^2}
    */
    fmpq_poly_zero(r.data);
    fmpq_poly_set_coeff_fmpz(r.data, 10, eint_data(eget_cint(1)));
    {
    uint8_t prog8[] = {3,4,0,0,0,0,0,0,0,0,3,5,3,0,0,0,0,0,0,0,1,5,5,0,2,4,4,5};
    uint8_t prog7[] = {3,4,0,0,0,0,0,0,0,0,3,5,1,0,0,0,0,0,0,0,1,5,5,1,2,4,4,5};
    uint8_t prog6[] = {3,4,0,0,0,0,0,0,0,0,3,5,3,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,2,4,4,5,3,5,253,255,255,255,255,255,255,255,1,5,5,2,2,4,4,5};
    uint8_t prog5[] = {3,4,0,0,0,0,0,0,0,0,3,5,2,0,0,0,0,0,0,0,1,5,5,0,1,5,5,1,2,4,4,5,3,5,245,255,255,255,255,255,255,255,1,5,5,3,2,4,4,5};
    uint8_t prog4[] = {3,4,0,0,0,0,0,0,0,0,3,5,1,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,0,2,4,4,5,3,5,254,255,255,255,255,255,255,255,1,5,5,0,1,5,5,2,2,4,4,5,3,5,255,255,255,255,255,255,255,255,1,5,5,1,1,5,5,1,2,4,4,5};
    uint8_t prog3[] = {3,4,0,0,0,0,0,0,0,0,3,5,1,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,1,2,4,4,5,3,5,252,255,255,255,255,255,255,255,1,5,5,0,1,5,5,3,2,4,4,5,3,5,252,255,255,255,255,255,255,255,1,5,5,1,1,5,5,2,2,4,4,5};
    uint8_t prog2[] = {3,4,0,0,0,0,0,0,0,0,3,5,1,0,0,0,0,0,0,0,1,5,5,0,1,5,5,0,1,5,5,2,2,4,4,5,3,5,255,255,255,255,255,255,255,255,1,5,5,0,1,5,5,1,1,5,5,1,2,4,4,5,3,5,7,0,0,0,0,0,0,0,1,5,5,1,1,5,5,3,2,4,4,5,3,5,252,255,255,255,255,255,255,255,1,5,5,2,1,5,5,2,2,4,4,5};
    uint8_t prog1[] = {3,4,0,0,0,0,0,0,0,0,3,5,255,255,255,255,255,255,255,255,1,5,5,0,1,5,5,0,1,5,5,3,2,4,4,5,3,5,255,255,255,255,255,255,255,255,1,5,5,1,1,5,5,1,1,5,5,1,2,4,4,5,3,5,4,0,0,0,0,0,0,0,1,5,5,2,1,5,5,3,2,4,4,5};
    uint8_t prog0[] = {3,4,0,0,0,0,0,0,0,0,3,5,1,0,0,0,0,0,0,0,1,5,5,0,1,5,5,1,1,5,5,3,2,4,4,5,3,5,255,255,255,255,255,255,255,255,1,5,5,1,1,5,5,1,1,5,5,2,2,4,4,5,3,5,255,255,255,255,255,255,255,255,1,5,5,3,1,5,5,3,2,4,4,5};
    eval_poly_fmpq(stack, prog8, sizeof(prog8));
    fmpq_poly_set_coeff_fmpq(r.data, 8, stack[4].data);
    eval_poly_fmpq(stack, prog7, sizeof(prog7));
    fmpq_poly_set_coeff_fmpq(r.data, 7, stack[4].data);
    eval_poly_fmpq(stack, prog6, sizeof(prog6));
    fmpq_poly_set_coeff_fmpq(r.data, 6, stack[4].data);
    eval_poly_fmpq(stack, prog5, sizeof(prog5));
    fmpq_poly_set_coeff_fmpq(r.data, 5, stack[4].data);
    eval_poly_fmpq(stack, prog4, sizeof(prog4));
    fmpq_poly_set_coeff_fmpq(r.data, 4, stack[4].data);
    eval_poly_fmpq(stack, prog3, sizeof(prog3));
    fmpq_poly_set_coeff_fmpq(r.data, 3, stack[4].data);
    eval_poly_fmpq(stack, prog2, sizeof(prog2));
    fmpq_poly_set_coeff_fmpq(r.data, 2, stack[4].data);
    eval_poly_fmpq(stack, prog1, sizeof(prog1));
    fmpq_poly_set_coeff_fmpq(r.data, 1, stack[4].data);
    eval_poly_fmpq(stack, prog0, sizeof(prog0));
    fmpq_poly_set_coeff_fmpq(r.data, 0, stack[4].data);
    }

    fmpz_poly_factor_init(fac);
    fmpz_poly_factor(fac, r.zpoly());
//std::cout << "r fac: " << std::endl;
//fmpz_poly_factor_print(fac);
    assert(fac->num == 2);
    assert((fac->p + 0)->length == 6);
    assert((fac->p + 1)->length == 6);
    assert(fmpz_is_zero((fac->p + 0)->coeffs + 4));
    assert(fmpz_is_zero((fac->p + 1)->coeffs + 4));

    xfmpq_t u, v, w, b[6], c[6];
    for (slong i = 2; i <= 5; i++)
    {
        fmpq_set_fmpz_frac(u.data, (fac->p + 0)->coeffs + 5 - i, (fac->p + 0)->coeffs + 5);
        fmpq_set_fmpz_frac(v.data, (fac->p + 1)->coeffs + 5 - i, (fac->p + 1)->coeffs + 5);
        fmpq_add(b[i].data, u.data, v.data);
        fmpq_sub(c[i].data, u.data, v.data);
        fmpq_div_2exp(b[i].data, b[i].data, 1);
        fmpq_div_2exp(c[i].data, c[i].data, 1);
        fmpq_div(c[i].data, c[i].data, disc_sqrt.data);
    }

    fmpq_set(stack[0].data, b[2].data);
    fmpq_set(stack[1].data, b[3].data);
    fmpq_set(stack[2].data, b[4].data);
    fmpq_set(stack[3].data, b[5].data);
    fmpq_set(stack[4].data, c[2].data);
    fmpq_set(stack[5].data, c[3].data);
    fmpq_set(stack[6].data, c[4].data);
    fmpq_set(stack[7].data, c[5].data);
    fmpq_set(stack[8].data, disc.data);

    /* evaluate A = 
        -81*c2^2(4*b2*c2^2 + 15*c3^2)*disc^2
         - 25*b2^2*(4*b2^3 + 135*b3^2)
         + 45(8*b2^3*c2^2 - 15*b2^2*c3^2 + 180*b3*b2*c2*c3 - 135*b3^2*c2^2)*disc
    */
    xfmpq_t A;
    {
    uint8_t prog[] = {3,9,0,0,0,0,0,0,0,0,3,10,156,255,255,255,255,255,255,255,1,10,10,0,1,10,10,0,1,10,10,0,1,10,10,0,1,10,10,0,2,9,9,10,3,10,104,1,0,0,0,0,0,0,1,10,10,0,1,10,10,0,1,10,10,0,1,10,10,4,1,10,10,4,1,10,10,8,2,9,9,10,3,10,209,242,255,255,255,255,255,255,1,10,10,0,1,10,10,0,1,10,10,1,1,10,10,1,2,9,9,10,3,10,93,253,255,255,255,255,255,255,1,10,10,0,1,10,10,0,1,10,10,5,1,10,10,5,1,10,10,8,2,9,9,10,3,10,164,31,0,0,0,0,0,0,1,10,10,0,1,10,10,1,1,10,10,4,1,10,10,5,1,10,10,8,2,9,9,10,3,10,188,254,255,255,255,255,255,255,1,10,10,0,1,10,10,4,1,10,10,4,1,10,10,4,1,10,10,4,1,10,10,8,1,10,10,8,2,9,9,10,3,10,69,232,255,255,255,255,255,255,1,10,10,1,1,10,10,1,1,10,10,4,1,10,10,4,1,10,10,8,2,9,9,10,3,10,65,251,255,255,255,255,255,255,1,10,10,4,1,10,10,4,1,10,10,5,1,10,10,5,1,10,10,8,1,10,10,8,2,9,9,10};
    eval_poly_fmpq(stack, prog, sizeof(prog));
    fmpq_set(A.data, stack[9].data);
    }
    
    /* evaluate B = 
        c2*(28*b2^2*c2^2 + 90*b4*c2^2 - 135*b3*c3*c2 - 90*b2*c4*c2 + 45*b2*c3^2)*disc
        - 5*b2*(2*b2^3*c2 - 10*b2^2*c4 + 10*b4*b2*c2 + 15*b3*b2*c3 - 45*b3^2*c2)
        - 18*c2^5*disc^2
    */
    xfmpq_t B;
    {
    uint8_t prog[] = {3,9,0,0,0,0,0,0,0,0,3,10,246,255,255,255,255,255,255,255,1,10,10,0,1,10,10,0,1,10,10,0,1,10,10,0,1,10,10,4,2,9,9,10,3,10,50,0,0,0,0,0,0,0,1,10,10,0,1,10,10,0,1,10,10,0,1,10,10,6,2,9,9,10,3,10,181,255,255,255,255,255,255,255,1,10,10,0,1,10,10,0,1,10,10,1,1,10,10,5,2,9,9,10,3,10,206,255,255,255,255,255,255,255,1,10,10,0,1,10,10,0,1,10,10,2,1,10,10,4,2,9,9,10,3,10,28,0,0,0,0,0,0,0,1,10,10,0,1,10,10,0,1,10,10,4,1,10,10,4,1,10,10,4,1,10,10,8,2,9,9,10,3,10,225,0,0,0,0,0,0,0,1,10,10,0,1,10,10,1,1,10,10,1,1,10,10,4,2,9,9,10,3,10,166,255,255,255,255,255,255,255,1,10,10,0,1,10,10,4,1,10,10,4,1,10,10,6,1,10,10,8,2,9,9,10,3,10,45,0,0,0,0,0,0,0,1,10,10,0,1,10,10,4,1,10,10,5,1,10,10,5,1,10,10,8,2,9,9,10,3,10,121,255,255,255,255,255,255,255,1,10,10,1,1,10,10,4,1,10,10,4,1,10,10,5,1,10,10,8,2,9,9,10,3,10,90,0,0,0,0,0,0,0,1,10,10,2,1,10,10,4,1,10,10,4,1,10,10,4,1,10,10,8,2,9,9,10,3,10,238,255,255,255,255,255,255,255,1,10,10,4,1,10,10,4,1,10,10,4,1,10,10,4,1,10,10,4,1,10,10,8,1,10,10,8,2,9,9,10};
    eval_poly_fmpq(stack, prog, sizeof(prog));
    fmpq_set(B.data, stack[9].data);
    }

    /* check if 3*A +- 54*B*disc_sqrt are both squares */

    fmpq_mul_fmpz(u.data, A.data, eint_data(eget_cint(3)));
    fmpq_mul_fmpz(w.data, B.data, eint_data(eget_cint(54)));
    fmpq_mul(v.data, w.data, disc_sqrt.data);

    fmpq_add(w.data, u.data, v.data);
    if (!w.is_square())
        return emake_cint(10);

    fmpq_sub(w.data, u.data, v.data);
    if (!w.is_square())
        return emake_cint(10);

    return emake_cint(5);
}


static ex _galois_group_order_irr(xfmpq_poly_t &p)
{
    slong d = fmpq_poly_degree(p.data);
    switch (d)
    {
        case 5:
            return _galois_group_order_irr5(p);
        case 4:
            return _galois_group_order_irr4(p);
        case 3:
            return _galois_group_order_irr3(p);
        case 2:
            return emake_cint(2);
        case 1:
            return emake_cint(1);
        default:
            return d < 1 ? emake_cint(1) : gs.sym_s$Failed.copy();
    }
}

static ex _galois_group_order(xfmpq_poly_t &p)
{
//std::cout << "_galois_group_order: " << p.tostring() << std::endl;
//std::cout << "_galois_group_order: " << std::endl;
//fmpz_poly_print_pretty(p.zpoly(), "X");
//std::cout << std::endl;

    fmpz_poly_factor_t fac;
    fmpz_poly_factor_init(fac);
    fmpz_poly_factor(fac, p.zpoly());
//std::cout << "fac: " << std::endl;
//fmpz_poly_factor_print(fac);
    bool irr = fac->num == 1;
    fmpz_poly_factor_clear(fac);
    return (!irr) ? emake_int_si(0) : _galois_group_order_irr(p);
}

ex dcode_sGaloisGroup(er e)
{
//std::cout << "dcode_sGaloisGroup: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sGaloisGroup.get()));

    if (elength(e) != 1)
    {
        return ecopy(e);
    }

    er X = echild(e,1);
    if (ehas_head_sym(X, gs.sym_sList.get()))
    {
        size_t n = elength(X);
        xfmpq_poly_t p;
        for (size_t i = 1; i <= n; i++)
        {
            er Xi = echild(X,i);
            if (eis_int(Xi))
            {
                fmpq_poly_set_coeff_fmpz(p.data, i-1, eint_data(Xi));
            }
            else if (eis_rat(Xi))
            {
                fmpq_poly_set_coeff_fmpq(p.data, i-1, erat_data(Xi));
            }
            else
            {
                return ecopy(e);
            }
        }
        return _galois_group_order(p);
    }
    else
    {
        return ecopy(e);
    }
}





static ex _roots(xfmpq_poly_t &p);

static ex _roots_irr1(const fmpz_poly_t p)
{
    assert(fmpz_poly_degree(p) == 1);
    uex r1(emake_rat());
    fmpq_set_fmpz_frac(erat_data(r1.get()), p->coeffs + 0, p->coeffs + 1);
    fmpq_neg(erat_data(r1.get()), erat_data(r1.get()));
    r1.set(efix_rat(r1.release()));
    return emake_node(gs.sym_sList.copy(), r1.release());
}

static ex _roots_irr2(const fmpz_poly_t p)
{
    assert(fmpz_poly_degree(p) == 2);
    uex a(emake_int()), b(emake_int()), d(emake_int());
    fmpz_mul(eint_data(a.get()), p->coeffs + 1, p->coeffs + 1);
    fmpz_mul(eint_data(b.get()), p->coeffs + 0, p->coeffs + 2);
    fmpz_mul_2exp(eint_data(b.get()), eint_data(b.get()), 2);
    fmpz_sub(eint_data(d.get()), eint_data(a.get()), eint_data(b.get()));
    d.set(emake_node(gs.sym_sSqrt.copy(), d.release()));
    fmpz_neg(eint_data(a.get()), p->coeffs + 1);
    fmpz_mul_2exp(eint_data(b.get()), p->coeffs + 2, 1);
    uex r1(ex_add(a.copy(), d.copy()));
    uex r2(ex_sub(a.copy(), d.copy()));
    r1.set(ex_div(r1.release(), b.copy()));
    r2.set(ex_div(r2.release(), b.copy()));
    return emake_node(gs.sym_sList.copy(), r1.release(), r2.release());
}



static ex _roots_irr3(const fmpz_poly_t p)
{
    assert(fmpz_poly_degree(p) == 3);

    /* the u1 u2 are roots of
        -(-p2^2 + 3*p1*p3)/(9*p3^2)
        + (-2*p2^3 + 9*p1*p2*p3 - 27*p0*p3^2)/(3*p3*(-p2^2 + 3*p1*p3))*U
        + U^2
    */

    fmpz_t A, B, C, D, u, v;

    /* A = 3*p1*p3 - p2^2 */
    fmpz_mul(A, p->coeffs + 1, p->coeffs + 3);
    fmpz_mul_ui(A, A, 3);
    fmpz_mul(u, p->coeffs + 2, p->coeffs + 2);
    fmpz_sub(A, A, u);

    /* B = -2*p2^3 + 9*p1*p2*p3 - 27*p0*p3^2 */
    fmpz_mul(B, p->coeffs + 1, p->coeffs + 2);
    fmpz_mul(B, B, p->coeffs + 3);
    fmpz_mul_ui(B, B, 9);
    fmpz_mul(u, p->coeffs + 2, p->coeffs + 2);
    fmpz_mul(u, u, p->coeffs + 2);
    fmpz_mul_ui(u, u, 2);
    fmpz_sub(B, B, u);
    fmpz_mul(u, p->coeffs + 0, p->coeffs + 3);
    fmpz_mul(u, u, p->coeffs + 3);
    fmpz_mul_ui(u, u, 27);
    fmpz_sub(B, B, u);

    if (fmpz_is_zero(A))
    {
        uex a(emake_int()), b(emake_int()), d(emake_int());
        fmpz_neg(eint_data(a.get()), p->coeffs + 2);
        fmpz_mul_ui(eint_data(b.get()), p->coeffs + 3, 3);
        fmpz_set(eint_data(d.get()), B);
        d.set(emake_node(gs.sym_sPower.copy(), d.release(), emake_crat(1,3)));

        uex r1(emake_node(gs.sym_sPower.copy(), emake_cint(-1), emake_crat(0,3)));
        uex r2(emake_node(gs.sym_sPower.copy(), emake_cint(-1), emake_crat(2,3)));
        uex r3(emake_node(gs.sym_sPower.copy(), emake_cint(-1), emake_crat(4,3)));
        r1.set(emake_node(gs.sym_sPlus.copy(), a.copy(), emake_node(gs.sym_sTimes.copy(), r1.release(), d.copy())));
        r2.set(emake_node(gs.sym_sPlus.copy(), a.copy(), emake_node(gs.sym_sTimes.copy(), r2.release(), d.copy())));
        r3.set(emake_node(gs.sym_sPlus.copy(), a.copy(), emake_node(gs.sym_sTimes.copy(), r3.release(), d.copy())));
        r1.set(ex_div(r1.release(), b.copy()));
        r2.set(ex_div(r2.release(), b.copy()));
        r3.set(ex_div(r3.release(), b.copy()));
        return emake_node(gs.sym_sList.copy(), r1.release(), r2.release(), r3.release());
    }
    else
    {
        /* D = 4*A^3 + B^2 */
        fmpz_mul(D, B, B);
        fmpz_mul(u, A, A);
        fmpz_mul(u, u, A);
        fmpz_mul_ui(u, u, 4);
        fmpz_add(D, D, u);

        /* wiith d = ((B + Sqrt[D])/2)^(1/3)
            -(p2/(3 p3)) - (A)/(3 p3 d) + d/(3 p3)
        */

        uex b(emake_int()), d(emake_int());
        fmpz_set(eint_data(d.get()), D);
        fmpz_set(eint_data(b.get()), B);
        d.set(emake_node(gs.sym_sPower.copy(), d.release(), emake_crat(1,2)));
        d.set(emake_node(gs.sym_sPlus.copy(), b.release(), d.release()));
        d.set(emake_node(gs.sym_sTimes.copy(), emake_crat(1,2), d.release()));
        b.set(emake_int());
        fmpz_mul_ui(eint_data(b.get()), p->coeffs + 3, 3);

        uex t0(emake_rat());
        fmpq_set_fmpz_frac(erat_data(t0.get()), p->coeffs + 2, eint_data(b.get()));
        fmpq_neg(erat_data(t0.get()), erat_data(t0.get()));
        t0.set(efix_rat(t0.release()));

        uex t1(emake_rat());
        fmpq_set_fmpz_frac(erat_data(t1.get()), A, eint_data(b.get()));
        fmpq_neg(erat_data(t1.get()), erat_data(t1.get()));
        t1.set(efix_rat(t1.release()));
        ex t = emake_node(gs.sym_sPower.copy(), d.copy(), emake_crat(-1,3));
        t1.set(emake_node(gs.sym_sTimes.copy(), t1.release(), t));
        
        uex t2(emake_rat());
        fmpq_set_fmpz_frac(erat_data(t2.get()), eint_data(eget_cint(1)), eint_data(b.get()));
        t2.set(efix_rat(t2.release()));
        t = emake_node(gs.sym_sPower.copy(), d.copy(), emake_crat(1,3));
        t2.set(emake_node(gs.sym_sTimes.copy(), t2.release(), t));

        uex z1(emake_node(gs.sym_sPower.copy(), emake_cint(-1), emake_crat(2,3)));
        uex z2(emake_node(gs.sym_sPower.copy(), emake_cint(-1), emake_crat(4,3)));
        uex t11(ex_mul(z1.copy(), t1.copy()));
        uex t21(ex_mul(z2.copy(), t1.copy()));
        uex t12(ex_mul(z1.copy(), t2.copy()));
        uex t22(ex_mul(z2.copy(), t2.copy()));
        uex r1, r2, r3;
        r1.set(ex_add(t1.copy(),  t2.copy()));
        r2.set(ex_add(t11.copy(), t22.copy()));
        r3.set(ex_add(t21.copy(), t12.copy()));
        if (!eis_zero(t0.get()))
        {
            r1.reset(ex_add(t0.copy(), r1.copy()));
            r2.reset(ex_add(t0.copy(), r2.copy()));
            r2.reset(ex_add(t0.copy(), r3.copy()));
        }
        return emake_node(gs.sym_sList.copy(), r1.release(), r2.release(), r3.release());
    }
}



static ex _roots_irr4(const fmpz_poly_t p)
{
    assert(fmpz_poly_degree(p) == 4);

    xfmpq_t A, B, C, b0, b1, b2, b3, b3b3, u, v, w;

    /* b3 = c3/(4c4), b2 = c2/(4c4), b1 = c1/(8c4), b0 = c0/(4c4)*/
    fmpq_set_fmpz_frac(b0.data, p->coeffs + 0, p->coeffs + 4);
    fmpq_set_fmpz_frac(b1.data, p->coeffs + 1, p->coeffs + 4);
    fmpq_set_fmpz_frac(b2.data, p->coeffs + 2, p->coeffs + 4);
    fmpq_set_fmpz_frac(b3.data, p->coeffs + 3, p->coeffs + 4);
    fmpq_div_2exp(b0.data, b0.data, 2);
    fmpq_div_2exp(b1.data, b1.data, 3);
    fmpq_div_2exp(b2.data, b2.data, 2);
    fmpq_div_2exp(b3.data, b3.data, 2);

    fmpq_pow_si(b3b3.data, b3.data, 2);

    /* A = b1 - b2 b3 + b3^3 */
    fmpq_sub(v.data, b3b3.data, b2.data);
    fmpq_mul(u.data, b3.data, v.data);
    fmpq_add(A.data, b1.data, u.data);

    /* B = -2 b2 + 3 b3^2 */
    fmpq_mul_fmpz(u.data, b3b3.data, eint_data(eget_cint(3)));
    fmpq_mul_2exp(v.data, b2.data, 1);
    fmpq_sub(B.data, u.data, v.data);

    /* C = -b0 + b2^2 + 2 b1 b3 - 4 b2 b3^2 + 3 b3^4 */
    fmpq_mul_fmpz(u.data, b3b3.data, eint_data(eget_cint(3)));
    fmpq_mul_2exp(v.data, b2.data, 2);
    fmpq_sub(w.data, u.data, v.data);
    fmpq_mul(u.data, b3.data, w.data);
    fmpq_mul_2exp(v.data, b1.data, 1);
    fmpq_add(w.data, v.data, u.data);
    fmpq_mul(u.data, b3.data, w.data);
    fmpq_pow_si(v.data, b2.data, 2);
    fmpq_sub(w.data, v.data, b0.data);
    fmpq_add(C.data, w.data, u.data);
    
    if (fmpq_is_zero(A.data))
    {
        xfmpq_poly_t q;
        fmpq_neg(u.data, B.data);
        fmpq_poly_set_coeff_fmpz(q.data, 2, eint_data(eget_cint(1)));
        fmpq_poly_set_coeff_fmpq(q.data, 1, u.data);
        fmpq_poly_set_coeff_fmpq(q.data, 0, C.data);
        uex u12(_roots(q));
        uex t1(emake_node(gs.sym_sPower.copy(), u12.copychild(1), emake_crat(1,2)));
        uex t2(emake_node(gs.sym_sPower.copy(), u12.copychild(2), emake_crat(1,2)));
        uex t0(emake_rat());
        fmpq_neg(erat_data(t0.get()), b3.data);
        t0.set(efix_rat(t0.release()));
        uex mt1(emake_node(gs.sym_sTimes.copy(), emake_cint(-1), t1.copy()));
        uex mt2(emake_node(gs.sym_sTimes.copy(), emake_cint(-1), t2.copy()));
        uex r1(emake_node(gs.sym_sPlus.copy(), t0.copy(), mt1.copy(), mt2.copy()));
        uex r2(emake_node(gs.sym_sPlus.copy(), t0.copy(), mt1.copy(), t2.copy()));
        uex r3(emake_node(gs.sym_sPlus.copy(), t0.copy(), t1.copy(), mt2.copy()));
        uex r4(emake_node(gs.sym_sPlus.copy(), t0.copy(), t1.copy(), t2.copy()));
        r1.reset(dcode_sPlus(r1.get()));
        r2.reset(dcode_sPlus(r2.get()));
        r3.reset(dcode_sPlus(r3.get()));
        r4.reset(dcode_sPlus(r4.get()));
        return emake_node(gs.sym_sList.copy(), r1.release(), r2.release(), r3.release(), r4.release());
    }
    else
    {
        fmpq_div(u.data, C.data, A.data);
        xfmpq_poly_t q;
        fmpq_poly_set_coeff_fmpz(q.data, 3, eint_data(eget_cint(1)));
        fmpq_poly_set_coeff_fmpq(q.data, 2, u.data);
        fmpq_poly_set_coeff_fmpq(q.data, 1, B.data);
        fmpq_poly_set_coeff_fmpq(q.data, 0, A.data);
        uex u123(_roots(q));
        uex t1(emake_node(gs.sym_sPower.copy(), u123.copychild(1), emake_crat(1,2)));
        uex t2(emake_node(gs.sym_sPower.copy(), u123.copychild(2), emake_crat(1,2)));
        uex t3(emake_node(gs.sym_sPower.copy(), u123.copychild(3), emake_crat(1,2)));
        uex t0(emake_rat());
        fmpq_neg(erat_data(t0.get()), b3.data);
        t0.set(efix_rat(t0.release()));
        uex s1(emake_node(gs.sym_sTimes.copy(), t2.copy(), t3.copy()));
        uex s2(emake_node(gs.sym_sTimes.copy(), t1.copy(), t3.copy()));
        uex s3(emake_node(gs.sym_sTimes.copy(), t1.copy(), t2.copy()));
        uex ms1(emake_node(gs.sym_sTimes.copy(), emake_cint(-1), s1.copy()));
        uex ms2(emake_node(gs.sym_sTimes.copy(), emake_cint(-1), s2.copy()));
        uex ms3(emake_node(gs.sym_sTimes.copy(), emake_cint(-1), s3.copy()));
        uex r1(emake_node(gs.sym_sPlus.copy(), t0.copy(), ms1.copy(), ms2.copy(), s3.copy()));
        uex r2(emake_node(gs.sym_sPlus.copy(), t0.copy(), ms1.copy(), s2.copy(), ms3.copy()));
        uex r3(emake_node(gs.sym_sPlus.copy(), t0.copy(), s1.copy(), ms2.copy(), ms3.copy()));
        uex r4(emake_node(gs.sym_sPlus.copy(), t0.copy(), s1.copy(), s2.copy(), s3.copy()));
        r1.reset(dcode_sPlus(r1.get()));
        r2.reset(dcode_sPlus(r2.get()));
        r3.reset(dcode_sPlus(r3.get()));
        r4.reset(dcode_sPlus(r4.get()));
        return emake_node(gs.sym_sList.copy(), r1.release(), r2.release(), r3.release(), r4.release());
    }
}

static ex _roots_irr5(const fmpz_poly_t p)
{
    assert(fmpz_poly_degree(p) == 5);
    uex r1(emake_cint(0));
    uex r2(emake_cint(0));
    uex r3(emake_cint(0));
    uex r4(emake_cint(0));
    uex r5(emake_cint(0));
    return emake_node(gs.sym_sList.copy(), r1.release(), r2.release(), r3.release(), r4.release(), r5.release());
}


static ex _roots_irr(const fmpz_poly_t p)
{
    slong d = fmpz_poly_degree(p);
    switch (d)
    {
        case 5:
            return _roots_irr5(p);
        case 4:
            return _roots_irr4(p);
        case 3:
            return _roots_irr3(p);
        case 2:
            return _roots_irr2(p);
        case 1:
            return _roots_irr1(p);
        default:
            return d < 1 ? emake_node(gs.sym_sList.copy()) : gs.sym_s$Failed.copy();
    }
}

static ex _roots(xfmpq_poly_t &p)
{
    fmpz_poly_factor_t fac;
    fmpz_poly_factor_init(fac);
    fmpz_poly_factor(fac, p.zpoly());
    uex res(gs.sym_sList.get(), fmpq_poly_degree(p.data));
    for (slong i = 0; i < fac->num; i++)
    {
        wex irroots(_roots_irr(fac->p + i));
        for (size_t j = 1; j <= elength(irroots.get()); j++)
        {
            for (slong k = 0; k < fac->exp[i]; k++)
            {
                res.push_back(irroots.copychild(j));
            }
        }
    }
    return res.release();
}

ex dcode_sRoots(er e)
{
//std::cout << "dcode_sRoots: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sRoots.get()));

    if (elength(e) != 2)
        return _handle_message_argx(e, 2);

    uex var(emake_node(gs.sym_sList.copy(), ecopychild(e,2)));
    uex eq(ecopychild(e,1));
    if (ehas_head_sym_length(echild(e,1), gs.sym_sEqual.get(), 2))
    {
        eq.setnz(emake_node(gs.sym_sMinus.copy(), ecopychild(e,1,1), ecopychild(e,1,2)));
    }

    poly p(1);
    if (!ex_to_polynomial(p, eq.get(), var.get()))
    {
        _gen_message(gs.sym_sRoots.get(), "neq", "`1` is expected to be a polynomial in the variable `2`.", eq.copy(), var.copychild(1));
        return ecopy(e);
    }

    xfmpq_poly_t q;
    xfmpq_t ai;
    slong ei;

    for (ulong i = 0; i < p.coeffs.size(); i++)
    {
        if (!eget_fmpq(ai.data, p.coeffs[i].get()) ||
            COEFF_IS_MPZ(*p.exps[i].data) ||
            (ei = *p.exps[i].data, ei < 0))
        {
            return ecopy(e);
        }

        fmpq_poly_set_coeff_fmpq(q.data, ei, ai.data);
    }

    xfmpz_poly_factor_t fac;
    fmpz_poly_factor(fac.data, q.zpoly());

    std::vector<wex> l;
    std::vector<qbarelem> v;

    for (slong i = 0; i < fac.data->num; i++)
    {
        irred_fmpz_poly_roots(v, fac.data->p + i);
        for (ulong j = 0; j < v.size(); j++)
        {
            wex rt(v[j].get_ex());
            for (slong k = 0; k < fac.data->exp[i]; k++)
                l.push_back(wex(rt.copy()));
        }
    }

    return emake_node(gs.sym_sList.copy(), l);
}
