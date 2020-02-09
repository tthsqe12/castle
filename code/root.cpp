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

/* copy the midpoint and set the radius */
void acb_set_radius(acb_t y, const acb_t x, const mag_t e)
{
    arf_set(arb_midref(acb_realref(y)), arb_midref(acb_realref(x)));
    mag_set(arb_radref(acb_realref(y)), e);
    arf_set(arb_midref(acb_imagref(y)), arb_midref(acb_imagref(x)));
    mag_set(arb_radref(acb_imagref(y)), e);
}

/* can we not safely rule out x as a root of f */
bool could_be_root(const acb_t x, const fmpz_poly_t f)
{
    xacb f_eval;
    slong p = acb_rel_accuracy_bits(x);
    p = std::max(p, slong(100));
    arb_fmpz_poly_evaluate_acb(f_eval.data, f, x, p);
    return acb_contains_zero(f_eval.data);
}

/* if true, newton converges to the unique root in x.mid +/- e */
bool newton_is_ok(const acb_t x, const fmpz_poly_t f, mag_t e)
{
    xfmpz_poly fp, fpp;
    xacb y, f_eval, fp_eval, fpp_eval;
    xmag a0, b0, c, t;
    slong p = acb_rel_accuracy_bits(x);
    p = std::min(p + 100, slong(ARF_PREC_EXACT));
    p = std::max(p, slong(100));

    fmpz_poly_derivative(fp.data, f);
    arb_fmpz_poly_evaluate_acb(fp_eval.data, fp.data, x, p);
    acb_get_mag_lower(a0.data, fp_eval.data);

    arb_fmpz_poly_evaluate_acb(f_eval.data, f, x, p);
    acb_get_mag(b0.data, f_eval.data);
    mag_div(b0.data, b0.data, a0.data);

    if (!mag_is_finite(b0.data))
        return false;

    mag_mul_2exp_si(e, b0.data, 1);
    mag_max(e, e, arb_radref(acb_realref(x)));
    mag_max(e, e, arb_radref(acb_imagref(x)));

    fmpz_poly_derivative(fpp.data, fp.data);
    acb_set_radius(y.data, x, e);
    arb_fmpz_poly_evaluate_acb(fpp_eval.data, fpp.data, y.data, p);
    acb_get_mag(c.data, fpp_eval.data);

    mag_mul(c.data, c.data, b0.data);
    mag_div(c.data, c.data, a0.data);
    return mag_cmp_2exp_si(c.data, -5) < 0;
}

/* advance x by one iteration of newton's method */
void newton_step(acb_t x, const fmpz_poly_t f)
{
    xfmpz_poly fp;
    xacb t, y, f_eval, fp_eval;
    slong p = acb_rel_accuracy_bits(x);
    p = std::max(p, slong(100));

    acb_set(y.data, x);
    if (p < ARF_PREC_EXACT)
    {
        mag_mul_2exp_si(arb_radref(acb_realref(y.data)), arb_radref(acb_realref(y.data)), -p);
        mag_mul_2exp_si(arb_radref(acb_realref(y.data)), arb_radref(acb_realref(y.data)), -p);
        p = std::min(2*p, slong(ARF_PREC_EXACT));
    }

    fmpz_poly_derivative(fp.data, f);
    arb_fmpz_poly_evaluate_acb(f_eval.data, f, y.data, p);
    arb_fmpz_poly_evaluate_acb(fp_eval.data, fp.data, y.data, p);
    acb_div(t.data, f_eval.data, fp_eval.data, p);
    acb_sub(x, y.data, t.data, p);
}


class qbarelem {
public:
    xfmpz_poly minpoly;
    xacb location;

    /* conversion to/from expressions */
    bool set_ex(er e);
    ex get_ex() const;

    /* basic arithmetic */
    void set(const qbarelem & a);
    void set(slong a);
    void set(const fmpz_t a);
    void set(const fmpq_t a);
    bool add(const qbarelem & a, const qbarelem & b);
    bool sub(const qbarelem & a, const qbarelem & b);
    bool mul(const qbarelem & a, const qbarelem & b);
    bool pow(const qbarelem & a, slong b, ulong c);
    bool pow(const qbarelem & a, qbarelem & b);

    template <typename RefineFunc>
    bool select_root(fmpz_poly_t p, RefineFunc T);

    void swap(qbarelem & a) {
        fmpz_poly_swap(minpoly.data, a.minpoly.data);
        acb_swap(location.data, a.location.data);
    }

    std::string tostring() const {
        return "Root[" + minpoly.tostring() + ", " + location.tostring() + "]";
    }
};

void qbarelem::set(const qbarelem & a)
{
    fmpz_poly_set(minpoly.data, a.minpoly.data);
    acb_set(location.data, a.location.data);
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
    arb_zero(acb_imagref(location.data));
}

void qbarelem::set(const fmpz_t a)
{
    fmpz_poly_fit_length(minpoly.data, 2);
    fmpz_neg(minpoly.data->coeffs + 0, a);
    fmpz_one(minpoly.data->coeffs + 1);
    _fmpz_poly_set_length(minpoly.data, 2);

    arf_set_fmpz(arb_midref(acb_realref(location.data)), a);
    mag_set_ui_2exp_si(arb_radref(acb_realref(location.data)), 1, -FLINT_BITS);
    arb_zero(acb_imagref(location.data));
}

void qbarelem::set(const fmpq_t a)
{
    fmpz_poly_fit_length(minpoly.data, 2);
    fmpz_neg(minpoly.data->coeffs + 0, fmpq_numref(a));
    fmpz_set(minpoly.data->coeffs + 1, fmpq_denref(a));
    _fmpz_poly_set_length(minpoly.data, 2);

    arf_fmpz_div_fmpz(arb_midref(acb_realref(location.data)),
                        fmpq_numref(a), fmpq_denref(a), 2*FLINT_BITS, ARB_RND);
    mag_set_ui_2exp_si(arb_radref(acb_realref(location.data)), 1, -2);
    arb_zero(acb_imagref(location.data));
}

/* given a function for refining our location, set us to the right root of p */
template <typename RefineFunc>
bool qbarelem::select_root(fmpz_poly_t p, RefineFunc T)
{
    xmag e, et;
    xfmpz_poly_factor f;
    xacb rtloc;

    fmpz_poly_factor(f.data, p);

try_again:

    T(location.data);

std::cout << "trying location: " << location.tostring() << std::endl;
SleepMS(1000);

    if (f.data->num > 1)
    {
        mag_inf(e.data);

        for (slong i = 0; i < f.data->num; i++)
        {
            if (newton_is_ok(location.data, f.data->p + i, et.data))
                mag_min(e.data, e.data, et.data);
        }

        if (mag_is_inf(e.data))
            goto try_again;

        acb_set_radius(rtloc.data, location.data, e.data);

        for (slong i = 0; i < f.data->num; i++)
        {
            if (could_be_root(rtloc.data, f.data->p + i))
                continue;

            fmpz_poly_swap(f.data->p + i, f.data->p + f.data->num - 1);
            f.data->num--;
        }

        if (f.data->num > 1)
            goto try_again;
    }

    assert(f.data->num == 1);

    if (!newton_is_ok(location.data, f.data->p + 0, et.data))
        goto try_again;

    fmpz_poly_swap(minpoly.data, f.data->p + 0);
    return true;
}


/************* a + b ***************/

class add_refiner {
    xacb aloc, bloc;
    const fmpz_poly_struct * aminpoly, * bminpoly;
    bool first;
public:
    add_refiner(const qbarelem & a, const qbarelem & b)
    {
        acb_set(aloc.data, a.location.data);
        aminpoly = a.minpoly.data;
        acb_set(bloc.data, b.location.data);
        bminpoly = b.minpoly.data;
        first = true;
    }
    void operator() (acb_t rloc)
    {
        if (!first)
        {
            newton_step(aloc.data, aminpoly);
            newton_step(bloc.data, bminpoly);
        }
        first = false;
        slong pa = acb_rel_accuracy_bits(aloc.data);
        slong pb = acb_rel_accuracy_bits(bloc.data);
        slong p = std::max(pa, pb);
        p = std::min(p + 100, slong(ARF_PREC_EXACT));
        p = std::max(p, slong(100));
        acb_add(rloc, aloc.data, bloc.data, p);
    }
};

bool qbarelem::add(const qbarelem & a, const qbarelem & b)
{
std::cout << "add " << b.tostring() << " + " << a.tostring() << std::endl;

    if (fmpz_is_zero(a.minpoly.data->coeffs + 0))
    {
        set(b);
        return true;
    }

    if (fmpz_is_zero(b.minpoly.data->coeffs + 0))
    {
        set(a);
        return true;
    }

    x_fmpz_vector xs, ys;
    xfmpz_poly t;

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
    select_root(t.data, add_refiner(a, b));

std::cout << "add return: " << tostring() << std::endl;
    return true;
}


/************* a*b ***************/

class mul_refiner {
    xacb aloc, bloc;
    const fmpz_poly_struct * aminpoly, * bminpoly;
    bool first;
public:
    mul_refiner(const qbarelem & a, const qbarelem & b)
    {
        acb_set(aloc.data, a.location.data);
        aminpoly = a.minpoly.data;
        acb_set(bloc.data, b.location.data);
        bminpoly = b.minpoly.data;
        first = true;
    }

    void operator() (acb_t rloc)
    {
        if (!first)
        {
            newton_step(aloc.data, aminpoly);
            newton_step(bloc.data, bminpoly);
        }
        first = false;
        slong pa = acb_rel_accuracy_bits(aloc.data);
        slong pb = acb_rel_accuracy_bits(bloc.data);
        slong p = std::max(pa, pb);
        p = std::min(p + 100, slong(ARF_PREC_EXACT));
        p = std::max(p, slong(100));
        acb_mul(rloc, aloc.data, bloc.data, p);
    }
};


bool qbarelem::mul(const qbarelem & a, const qbarelem & b)
{
std::cout << "mul " << a.tostring() << " * " << b.tostring() << std::endl;

    if (fmpz_is_zero(a.minpoly.data->coeffs + 0) ||
        fmpz_is_zero(b.minpoly.data->coeffs + 0))
    {
        set(slong(0));
        return true;
    }

    x_fmpz_vector xs, ys;
    xfmpz_poly t;
    xfmpz acc;
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
    select_root(t.data, mul_refiner(a, b));

std::cout << "mul return: " << tostring() << std::endl;
    return true;
}


/************ a^(b/c) **************/

class pow_refiner {
    xacb aloc;
    const fmpz_poly_struct * aminpoly;
    slong b;
    ulong c;
    bool first;
public:
    pow_refiner(const qbarelem & a, slong b_, slong c_)
    {
        acb_set(aloc.data, a.location.data);
        aminpoly = a.minpoly.data;
        b = b_;
        c = c_;
        first = true;
    }
    void operator() (acb_t rloc)
    {
        if (!first)
        {
            newton_step(aloc.data, aminpoly);
        }
        first = false;

        if (arb_contains_zero(acb_imagref(aloc.data)))
        {
            /* we need to determine if aloc is real */
            assert(0);

        }
        else
        {
            slong p = acb_rel_accuracy_bits(aloc.data);
            p = std::min(p + 100, slong(ARF_PREC_EXACT));
            p = std::max(p, slong(100));
            acb_root_ui(rloc, aloc.data, c, p);
            acb_pow_si(rloc, rloc, b, p);
        }
    }
};

bool qbarelem::pow(const qbarelem & a, slong b, ulong c)
{
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

    xfmpz_poly t;
    x_fmpz_vector xs, ys;
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
    else
    {
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

    select_root(t.data, pow_refiner(a, b, c));

std::cout << "pow return: " << tostring() << std::endl;
    return true;
}


ex qbarelem::get_ex() const
{
    std::vector<wex> p;
    wex slot(emake_node(gs.sym_sSlot.copy(), emake_cint(1)));
    uex number;

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
        ex t = emake_int_copy(minpoly.data->coeffs + i);
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

bool qbarelem::set_ex(er e)
{
/*
std::cout << "set_ex called e: " << ex_tostring_full(e) << std::endl;
*/
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
            slong b = *eint_data(echild(e,2));
            if (COEFF_IS_MPZ(b) || !t.set_ex(echild(e,1)))
                return false;
            return pow(t, b, 1);
        }
        else if (eis_rat(echild(e,2)))
        {
            slong b = *fmpq_numref(erat_data(echild(e,2)));
            slong c = *fmpq_denref(erat_data(echild(e,2)));
            if (COEFF_IS_MPZ(b) || COEFF_IS_MPZ(c) || !t.set_ex(echild(e,1)))
                return false;
            return pow(t, b, c);
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}



ex dcode_sRootReduce(er e)
{
//std::cout << "dcode_sRootReduce: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sRootReduce.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    qbarelem r;

    if (r.set_ex(echild(e,1)))
    {
        return r.get_ex();
    }

    return ecopy(e);
}





static ex _galois_group_order_irr3(xfmpq_poly &p)
{
    assert(fmpq_poly_degree(p.data) == 3);
    xfmpz disc;
    fmpz_poly_discriminant(disc.data, p.zpoly());
    return fmpz_is_square(disc.data) ? emake_cint(3) : emake_cint(6);
}

static ex _galois_group_order_irr4(xfmpq_poly &p)
{
    assert(fmpq_poly_degree(p.data) == 4);

    xfmpq A, B, C, b0, b1, b2, b3, b3b3, u, v, w;

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
    std::vector<xfmpq> stack;
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
        xfmpq_poly q;
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
            xfmpz c0c2, c1c1, t1, t2;
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
            xfmpz disc;
            fmpz_poly_discriminant(disc.data, q.zpoly());
            fmpz_poly_factor_clear(fac);
            return fmpz_is_square(disc.data) ? emake_cint(12) : emake_cint(24);
        }
    }
}

static ex _galois_group_order_irr5(xfmpq_poly &p)
{
    assert(fmpq_poly_degree(p.data) == 5);
    fmpq_poly_make_monic(p.data, p.data);

    xfmpq disc;
    _fmpz_poly_discriminant(fmpq_numref(disc.data), p.data->coeffs, 6);
    fmpz_pow_ui(fmpq_denref(disc.data), p.data->den, 8);
    fmpq_canonicalise(disc.data);

    bool disc_is_square = disc.is_square();
//std::cout << "disc: " << disc.tostring() << std::endl;

    xfmpq_poly q, r;
    xfmpq t;
    fmpq_poly_get_coeff_fmpq(t.data, p.data, 4);
    fmpq_div_fmpz(t.data, t.data, eint_data(eget_cint(-5)));
    fmpq_poly_set_coeff_fmpz(r.data, 1, eint_data(eget_cint(1)));
    fmpq_poly_set_coeff_fmpq(r.data, 0, t.data);
    fmpq_poly_compose(q.data, p.data, r.data);

//std::cout << "q: " << q.tostring() << std::endl;

    std::vector<xfmpq> stack(20);
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

    xfmpq theta;
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

    xfmpq disc_sqrt;
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

    xfmpq u, v, w, b[6], c[6];
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
    xfmpq A;
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
    xfmpq B;
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


static ex _galois_group_order_irr(xfmpq_poly &p)
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

static ex _galois_group_order(xfmpq_poly &p)
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
        xfmpq_poly p;
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





static ex _roots(xfmpq_poly &p);

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

    xfmpq A, B, C, b0, b1, b2, b3, b3b3, u, v, w;

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
        xfmpq_poly q;
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
        xfmpq_poly q;
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

static ex _roots(xfmpq_poly &p)
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

    if (elength(e) != 1)
    {
        return ecopy(e);
    }

    er X = echild(e,1);
    if (ehas_head_sym(X, gs.sym_sList.get()))
    {
        size_t n = elength(X);
        xfmpq_poly p;
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
        return _roots(p);
    }
    else
    {
        return ecopy(e);
    }
}
