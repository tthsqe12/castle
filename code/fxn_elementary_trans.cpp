#include <cmath>
#include <cfloat>
#include <complex>

#include "timing.h"
#include "uex.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"
#include "flint/arith.h"


typedef struct
{
    ex (* exact_fxn)(ex);
    ex (* double_fxn)(double, double);
    void (* arb_fxn)(acb_t, const acb_t, slong);
    ex (* double_imag_fxn)(double y);
    ex (* arb_imag_fxn)(ex y);
    slong extra_prec;
} CmplxFxn1;

ex trivial(ex E)
{
    return E;
}

static ex _exp_to_power(ex E)
{
    uex e(E);
    return emake_node(ecopy(gs.sym_sPower.get()), ecopy(gs.sym_sE.get()), ecopychild(e.get(),1));
}

ex cmplx_exp(double x, double y)
{
    std::complex<double> z(x,y);
    z = std::exp(z);
    return emake_cmplx(z.real(), z.imag());
}

ex double_imag_exp(double y)
{
//std::cout << "double_imag_exp: " << y <<std::endl;

    return emake_cmplx(cos(y), sin(y));
}

ex arb_imag_exp(ex Y)
{
//std::cout << "arb_imag_exp: " << ex_tostring_full(etor(Y)) <<std::endl;

    uex y(Y);
    uex wx(emake_real());
    uex wy(emake_real());
    slong p = ereal_number(Y).wprec() + EXTRA_PRECISION_BASIC;
    arb_sin_cos(ereal_data(wy.get()), ereal_data(wx.get()), ereal_data(Y), p);
    return emake_cmplx(wx.release(), wy.release());
}

ex cmplx_cos(double x, double y)
{
    std::complex<double> z(x,y);
    z = std::cos(z);
    return emake_cmplx(z.real(), z.imag());
}

ex double_imag_cos(double y)
{
//std::cout << "double_imag_cos: " << y <<std::endl;

    return emake_double(cosh(y));
}

ex arb_imag_cos(ex Y)
{
//std::cout << "arb_imag_cos: " << ex_tostring_full(etor(Y)) <<std::endl;

    uex y(Y);
    uex wx(emake_real());
    slong p = ereal_number(Y).wprec() + EXTRA_PRECISION_BASIC;
    arb_cosh(ereal_data(wx.get()), ereal_data(Y), p);
    return efix_real(wx.release());
}

ex cmplx_sin(double x, double y)
{
    std::complex<double> z(x,y);
    z = std::sin(z);
    return emake_cmplx(z.real(), z.imag());
}

ex double_imag_sin(double y)
{
//std::cout << "double_imag_sin: " << y <<std::endl;

    ex t = emake_double(sinh(y));
    return emake_cmplx(emake_cint(0), t);
}

ex arb_imag_sin(ex Y)
{
//std::cout << "arb_imag_sin: " << ex_tostring_full(etor(Y)) <<std::endl;

    uex y(Y);
    uex wy(emake_real());
    slong p = ereal_number(Y).wprec() + EXTRA_PRECISION_BASIC;
    arb_sinh(ereal_data(wy.get()), ereal_data(Y), p);
    ex t = efix_real(wy.release());
    return emake_cmplx(emake_cint(0), t);
}

ex cmplx_cosh(double x, double y)
{
    std::complex<double> z(x,y);
    z = std::cosh(z);
    return emake_cmplx(z.real(), z.imag());
}

ex double_imag_cosh(double y)
{
//std::cout << "double_imag_cosh: " << y <<std::endl;

    return emake_double(cos(y));
}

ex arb_imag_cosh(ex Y)
{
//std::cout << "arb_imag_cosh: " << ex_tostring_full(etor(Y)) <<std::endl;

    uex y(Y);
    uex wx(emake_real());
    slong p = ereal_number(Y).wprec() + EXTRA_PRECISION_BASIC;
    arb_cos(ereal_data(wx.get()), ereal_data(Y), p);
    return efix_real(wx.release());
}

ex cmplx_sinh(double x, double y)
{
    std::complex<double> z(x,y);
    z = std::sinh(z);
    return emake_cmplx(z.real(), z.imag());
}

ex double_imag_sinh(double y)
{
    ex t = emake_double(sin(y));
    return emake_cmplx(emake_cint(0), t);
}

ex arb_imag_sinh(ex Y)
{
    uex y(Y);
    uex wy(emake_real());
    slong p = ereal_number(Y).wprec() + EXTRA_PRECISION_BASIC;
    arb_sin(ereal_data(wy.get()), ereal_data(Y), p);
    ex t = efix_real(wy.release());
    return emake_cmplx(emake_cint(0), t);
}

ex cmplx_tan(double x, double y)
{
    std::complex<double> z(x,y);
    z = std::tan(z);
    return emake_cmplx(z.real(), z.imag());
}

ex double_imag_tan(double y)
{
    ex t = emake_double(tanh(y));
    return emake_cmplx(emake_cint(0), t);
}

ex arb_imag_tan(ex Y)
{
    uex y(Y);
    uex wy(emake_real());
    slong p = ereal_number(Y).wprec() + EXTRA_PRECISION_BASIC;
    arb_tanh(ereal_data(wy.get()), ereal_data(Y), p);
    ex t = efix_real(wy.release());
    return emake_cmplx(emake_cint(0), t);
}

ex cmplx_tanh(double x, double y)
{
    std::complex<double> z(x,y);
    z = std::tanh(z);
    return emake_cmplx(z.real(), z.imag());
}

ex double_imag_tanh(double y)
{
    ex t = emake_double(tan(y));
    return emake_cmplx(emake_cint(0), t);
}

ex arb_imag_tanh(ex Y)
{
    uex y(Y);
    uex wy(emake_real());
    slong p = ereal_number(Y).wprec() + EXTRA_PRECISION_BASIC;
    arb_tan(ereal_data(wy.get()), ereal_data(Y), p);
    ex t = efix_real(wy.release());
    return emake_cmplx(emake_cint(0), t);
}

ex cmplx_acos(double x, double y)
{
    std::complex<double> z(x,y);
    z = std::acos(z);
    return emake_cmplx(z.real(), z.imag());
}

ex double_imag_acos(double y)
{
//std::cout << "double_imag_acos: " << y <<std::endl;

    ex t = emake_double(-asinh(y));
    return emake_cmplx(ecopy(gs.const_double_halfpi.get()), t);
}

ex arb_imag_acos(ex Y)
{
//std::cout << "arb_imag_acos: " << ex_tostring_full(etor(Y)) <<std::endl;

    uex y(Y);
    uex wx(emake_real());
    uex wy(emake_real());
    slong p = ereal_number(Y).wprec() + EXTRA_PRECISION_BASIC;
    arb_asinh(ereal_data(wy.get()), ereal_data(Y), p);
    arb_neg(ereal_data(wy.get()), ereal_data(wy.get()));
    arb_const_pi(ereal_data(wx.get()), ereal_number(wy.get()).wprec());
    arb_mul_2exp_si(ereal_data(wx.get()), ereal_data(wx.get()), -WORD(1));
    return emake_cmplx(wx.release(), wy.release());
}


ex cmplx_acosh(double x, double y)
{

    std::complex<double> z(x,y);
    z = std::acosh(z);
    return emake_cmplx(z.real(), z.imag());
}

ex double_imag_acosh(double y)
{
//std::cout << "double_imag_acosh: " << y <<std::endl;

    er s;
    ex t;
    if (y >= 0)
    {
        s = gs.const_double_halfpi.get();
    }
    else
    {
        s = gs.const_double_mhalfpi.get();
        y = -y;
    } 
    t = emake_double(asinh(y));
    return emake_cmplx(t, ecopy(s));
}

ex arb_imag_acosh(ex Y)
{
//std::cout << "arb_imag_acosh: " << ex_tostring_full(etor(Y)) <<std::endl;

    uex y(Y);
    uex wx(emake_real());
    uex wy(emake_real());
    slong p = ereal_number(Y).wprec() + EXTRA_PRECISION_BASIC;
    arb_asinh(ereal_data(wx.get()), ereal_data(Y), p);
    arb_const_pi(ereal_data(wy.get()), p);
    arb_mul_2exp_si(ereal_data(wy.get()), ereal_data(wy.get()), -WORD(1));    

    if (arb_is_positive(ereal_data(Y)))
    {
    }
    else if (arb_is_negative(ereal_data(Y)))
    {
        arb_neg(ereal_data(wx.get()), ereal_data(wx.get()));
        arb_neg(ereal_data(wy.get()), ereal_data(wy.get()));
    }
    else
    {
        arb_abs(ereal_data(wx.get()), ereal_data(wx.get()));
    }
    
    return emake_cmplx(wx.release(), wy.release());
}

ex cmplx_asin(double x, double y)
{
    std::complex<double> z(x,y);
    z = std::asin(z);
    return emake_cmplx(z.real(), z.imag());
}

ex double_imag_asin(double y)
{
//std::cout << "double_imag_asin: " << y <<std::endl;

    ex t = emake_double(asinh(y));
    return emake_cmplx(emake_cint(0), t);
}

ex arb_imag_asin(ex Y)
{
//std::cout << "arb_imag_asin: " << ex_tostring_full(etor(Y)) <<std::endl;

    uex y(Y);
    uex wy(emake_real());
    slong p = ereal_number(Y).wprec() + EXTRA_PRECISION_BASIC;
    arb_asinh(ereal_data(wy.get()), ereal_data(Y), p);
    ex t = efix_real(wy.release());
    return emake_cmplx(emake_cint(0), t);
}

ex cmplx_asinh(double x, double y)
{
    std::complex<double> z(x,y);
    z = std::asinh(z);
    return emake_cmplx(z.real(), z.imag());
}

ex double_imag_asinh(double y)
{
//std::cout << "double_imag_asinh: " << y <<std::endl;
    ex wx, wy;
    if(y >= 1)
    {
        wx = emake_double(acosh(y));
        wy = ecopy(gs.const_double_halfpi.get());
    }
    else if (y <= -1)
    {
        wx = emake_double(-acosh(-y));
        wy = ecopy(gs.const_double_mhalfpi.get());
    }
    else 
    {
        wy = emake_double(asin(y));
        wx = emake_cint(0);
    }
    return emake_cmplx(wx, wy);
}

ex arb_imag_asinh(ex Y)
{
//std::cout << "arb_imag_asinh: " << ex_tostring_full(etor(Y)) <<std::endl;
    uex y(Y);
    slong p = ereal_number(Y).wprec() + EXTRA_PRECISION_BASIC;
    ex Z = emake_real();
    uex z(Z);
    xarb_t one;
    arb_one(one.data);
    if (arb_gt(ereal_data(Y), one.data))
    {
        arb_acosh(ereal_data(Z), ereal_data(Y), p);
        uex pi(emake_real());
        arb_const_pi(ereal_data(pi.get()), p);
        arb_mul_2exp_si(ereal_data(pi.get()), ereal_data(pi.get()), -WORD(1));
        ex t = efix_real(z.release());
        return emake_cmplx(t, pi.release());
    }
    arb_neg(one.data, one.data);
    if (arb_lt(ereal_data(Y), one.data))
    {
        arb_neg(ereal_data(Z), ereal_data(Y));
        arb_acosh(ereal_data(Z), ereal_data(Z), p);
        arb_neg(ereal_data(Z), ereal_data(Z));
        uex pi(emake_real());
        arb_const_pi(ereal_data(pi.get()), p);
        arb_neg(ereal_data(pi.get()), ereal_data(pi.get()));
        arb_mul_2exp_si(ereal_data(pi.get()), ereal_data(pi.get()), -WORD(1));
        ex t = efix_real(z.release());
        return emake_cmplx(t, pi.release());
    }
    arb_asin(ereal_data(Z), ereal_data(Y), p);
    ex t = efix_real(z.release());
    return emake_cmplx(emake_cint(0), t);
}

ex cmplx_atan(double x, double y)
{

    std::complex<double> z(x,y);
    z = std::atan(z);
    return emake_cmplx(z.real(), z.imag());
}

ex double_imag_atan(double y)
{
//std::cout << "double_imag_atan: " << y <<std::endl;

    ex wx, wy;
    if (y > 1)
    {
        wy = emake_double(0.5*log1p(2/(y-1)));
        wx = ecopy(gs.const_double_halfpi.get());
    }
    else if (y < -1)
    {
        wy = emake_double(-0.5*log1p(2/(-y-1)));
        wx = ecopy(gs.const_double_mhalfpi.get());
    }
    else
    {
        wy = emake_double(atanh(y));
        wx = emake_cint(0);
    }
    return emake_cmplx(wx, wy);
}

ex arb_imag_atan(ex Y)
{
//std::cout << "arb_imag_atan: " << ex_tostring_full(etor(Y)) <<std::endl;

    uex y(Y);
    slong p = ereal_number(Y).wprec() + EXTRA_PRECISION_BASIC;
    ex Z = emake_real();
    uex z(Z);
    xarb_t one;
    arb_one(one.data);
    if (arb_gt(ereal_data(Y), one.data))
    {
        /* 1/2 Log[1 + 2/(y-1)] */
        arb_sub_ui(ereal_data(Z), ereal_data(Y), 1, p);
        arb_ui_div(ereal_data(Z), 2, ereal_data(Z), p);
        arb_log1p(ereal_data(Z), ereal_data(Z), p);
        arb_mul_2exp_si(ereal_data(Z), ereal_data(Z), -WORD(1));
        uex pi(emake_real());
        arb_const_pi(ereal_data(pi.get()), p);
        arb_mul_2exp_si(ereal_data(pi.get()), ereal_data(pi.get()), -WORD(1));
        ex t = efix_real(z.release());
        return emake_cmplx(pi.release(), t);
    }
    arb_neg(one.data, one.data);
    if (arb_lt(ereal_data(Y), one.data))
    {
        /* -1/2 Log[1 + 2/(-y - 1)] */
        arb_add_ui(ereal_data(Z), ereal_data(Y), UWORD(1), p);
        arb_neg(ereal_data(Z), ereal_data(Z));
        arb_ui_div(ereal_data(Z), UWORD(2), ereal_data(Z), p);
        arb_log1p(ereal_data(Z), ereal_data(Z), p);
        arb_mul_2exp_si(ereal_data(Z), ereal_data(Z), -WORD(1));
        arb_neg(ereal_data(Z), ereal_data(Z));
        uex pi(emake_real());
        arb_const_pi(ereal_data(pi.get()), p);
        arb_mul_2exp_si(ereal_data(pi.get()), ereal_data(pi.get()), -WORD(1));
        arb_neg(ereal_data(pi.get()), ereal_data(pi.get()));
        ex t = efix_real(z.release());
        return emake_cmplx(pi.release(), t);
    }
    bool negate = arb_is_negative(ereal_data(Y));
        /* -1/2 Log[1 + 2y/(-y - 1)] */
        /* 1/2 Log[1 + 2y/(-y + 1)] */
    if (negate)
        arb_add_ui(ereal_data(Z), ereal_data(Y), UWORD(1), p);
    else
        arb_sub_ui(ereal_data(Z), ereal_data(Y), UWORD(1), p);
    arb_neg(ereal_data(Z), ereal_data(Z));
    arb_div(ereal_data(Z), ereal_data(Y), ereal_data(Z), p);
    arb_mul_2exp_si(ereal_data(Z), ereal_data(Z), WORD(1));
    arb_log1p(ereal_data(Z), ereal_data(Z), p);
    if (negate)
        arb_neg(ereal_data(Z), ereal_data(Z));
    arb_mul_2exp_si(ereal_data(Z), ereal_data(Z), -WORD(1));
    ex t = efix_real(z.release());
    return emake_cmplx(emake_cint(0), t);
}


ex cmplx_atanh(double x, double y)
{

    std::complex<double> z(x,y);
    z = std::atanh(z);
    return emake_cmplx(z.real(), z.imag());
}

ex double_imag_atanh(double y)
{
//std::cout << "double_imag_atanh: " << y <<std::endl;

    return emake_cmplx(emake_cint(0), emake_double(atan(y)));
}        

ex arb_imag_atanh(ex Y)
{
//std::cout << "arb_imag_atanh: " << ex_tostring_full(etor(Y)) <<std::endl;

    uex y(Y);
    uex wy(emake_real());
    slong p = ereal_number(Y).wprec() + EXTRA_PRECISION_BASIC;
    arb_atan(ereal_data(wy.get()), ereal_data(Y), p);
    ex t = efix_real(wy.release());
    return emake_cmplx(emake_cint(0), t);
}



ex num_Cmplx_IntDouble(ex X, ex Y, CmplxFxn1 * s)
{
//std::cout << "num_ExpCmplx_IntDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(etor(Y)) << std::endl;
    assert(eis_int(X));
    assert(eis_double(Y));
    if (fmpz_is_zero(eint_data(X)))
    {
        eclear(X);
        double y = edouble_number(Y);
        eclear(Y);
        return s->double_imag_fxn(y);
    }
    double x = num_todouble(X);
    double y = edouble_number(Y);
    eclear(X);
    eclear(Y);
    return s->double_fxn(x,y);
}

ex num_Cmplx_DoubleInt(ex X, ex Y, CmplxFxn1 * s)
{
//std::cout << "num_ExpCmplx_DoubleRat: " << ex_tostring_full(X) << ", " << ex_tostring_full(etor(Y)) << std::endl;
    assert(eis_double(X));
    assert(eis_int(Y));
    double x = edouble_number(X);
    double y = num_todouble(Y);
    eclear(X);
    eclear(Y);
    return s->double_fxn(x,y);
}

ex num_Cmplx_RatDouble(ex X, ex Y, CmplxFxn1 * s)
{
//std::cout << "num_ExpCmplx_RatDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(etor(Y)) << std::endl;
    assert(eis_rat(X));
    assert(eis_double(Y));
    double x = num_todouble(X);
    double y = edouble_number(Y);
    eclear(X);
    eclear(Y);
    return s->double_fxn(x,y);
}

ex num_Cmplx_DoubleRat(ex X, ex Y, CmplxFxn1 * s)
{
//std::cout << "num_ExpCmplx_DoubleRat: " << ex_tostring_full(X) << ", " << ex_tostring_full(etor(Y)) << std::endl;
    assert(eis_double(X));
    assert(eis_rat(Y));
    double x = edouble_number(X);
    double y = num_todouble(Y);
    eclear(X);
    eclear(Y);
    return s->double_fxn(x,y);
}

ex num_Cmplx_DoubleDouble(ex X, ex Y, CmplxFxn1 * s)
{
//std::cout << "num_ExpCmplx_DoubleDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(etor(Y)) << std::endl;
    assert(eis_double(X));
    assert(eis_double(Y));
    double x = edouble_number(X);
    double y = edouble_number(Y);
    eclear(X);
    eclear(Y);
    return s->double_fxn(x,y);
}

ex num_Cmplx_IntReal(ex X, ex Y, CmplxFxn1 * s)
{
//std::cout << "num_ExpCmplx_IntReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(etor(Y)) << std::endl;
    assert(eis_int(X));
    assert(eis_real(Y));
    if (fmpz_is_zero(eint_data(X)))
    {
        eclear(X);
        return s->arb_imag_fxn(Y);
    }
    uex x(X);
    uex y(Y);
    uex wre(emake_real());
    uex wim(emake_real());
    slong p = ereal_number(Y).wprec() + s->extra_prec;
    acb_t z,w;
    arb_init(acb_realref(z));
    arb_set_round_fmpz(acb_realref(z), eint_data(X), p);
    *acb_imagref(z) = *ereal_data(Y);
    *acb_realref(w) = *ereal_data(wre.get());
    *acb_imagref(w) = *ereal_data(wim.get());
    s->arb_fxn(w, z, p);
    *ereal_data(wre.get()) = *acb_realref(w);
    *ereal_data(wim.get()) = *acb_imagref(w);
    arb_clear(acb_realref(z));
    return emake_cmplx(wre.release(), wim.release());
}

ex num_Cmplx_RealInt(ex X, ex Y, CmplxFxn1 * s)
{
//std::cout << "num_ExpCmplx_RealInt: " << ex_tostring_full(X) << ", " << ex_tostring_full(etor(Y)) << std::endl;
    assert(eis_real(X));
    assert(eis_int(Y));
    uex x(X);
    uex y(Y);
    uex wre(emake_real());
    uex wim(emake_real());
    slong p = ereal_number(X).wprec() + s->extra_prec;
    acb_t z,w;
    *acb_realref(z) = *ereal_data(X);
    arb_init(acb_imagref(z));
    arb_set_round_fmpz(acb_imagref(z), eint_data(Y), p);
    *acb_realref(w) = *ereal_data(wre.get());
    *acb_imagref(w) = *ereal_data(wim.get());
    s->arb_fxn(w, z, p);
    *ereal_data(wre.get()) = *acb_realref(w);
    *ereal_data(wim.get()) = *acb_imagref(w);
    arb_clear(acb_imagref(z));
    return emake_cmplx(wre.release(), wim.release());
}

ex num_Cmplx_RatReal(ex X, ex Y, CmplxFxn1 * s)
{
//std::cout << "num_ExpCmplx_RatReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(etor(Y)) << std::endl;
    assert(eis_rat(X));
    assert(eis_real(Y));
    uex x(X);
    uex y(Y);
    uex wre(emake_real());
    uex wim(emake_real());
    slong p = ereal_number(Y).wprec() + s->extra_prec;
    acb_t z,w;
    arb_init(acb_realref(z));
    arb_set_fmpq(acb_realref(z), erat_data(X), p);
    *acb_imagref(z) = *ereal_data(Y);
    *acb_realref(w) = *ereal_data(wre.get());
    *acb_imagref(w) = *ereal_data(wim.get());
    s->arb_fxn(w, z, p);
    *ereal_data(wre.get()) = *acb_realref(w);
    *ereal_data(wim.get()) = *acb_imagref(w);
    arb_clear(acb_realref(z));
    return emake_cmplx(wre.release(), wim.release());
}

ex num_Cmplx_RealRat(ex X, ex Y, CmplxFxn1 * s)
{
//std::cout << "num_ExpCmplx_RealRat: " << ex_tostring_full(X) << ", " << ex_tostring_full(etor(Y)) << std::endl;
    assert(eis_real(X));
    assert(eis_rat(Y));
    uex x(X);
    uex y(Y);
    uex wre(emake_real());
    uex wim(emake_real());
    slong p = ereal_number(X).wprec() + s->extra_prec;
    acb_t z,w;
    *acb_realref(z) = *ereal_data(X);
    arb_init(acb_imagref(z));
    arb_set_fmpq(acb_imagref(z), erat_data(Y), p);
    *acb_realref(w) = *ereal_data(wre.get());
    *acb_imagref(w) = *ereal_data(wim.get());
    s->arb_fxn(w, z, p);
    *ereal_data(wre.get()) = *acb_realref(w);
    *ereal_data(wim.get()) = *acb_imagref(w);
    arb_clear(acb_imagref(z));
    return emake_cmplx(wre.release(), wim.release());
}

ex num_Cmplx_DoubleReal(ex X, ex Y, CmplxFxn1 * s)
{
//std::cout << "num_ExpCmplx_DoubleReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(etor(Y)) << std::endl;
    assert(eis_double(X));
    assert(eis_real(Y));
    double x = edouble_number(X);
    double y = num_todouble(Y);
    eclear(X);
    eclear(Y);
    return s->double_fxn(x,y);
}

ex num_Cmplx_RealDouble(ex X, ex Y, CmplxFxn1 * s)
{
//std::cout << "num_ExpCmplx_RealDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(etor(Y)) << std::endl;
    assert(eis_real(X));
    assert(eis_double(Y));
    double x = num_todouble(X);
    double y = edouble_number(Y);
    eclear(X);
    eclear(Y);
    return s->double_fxn(x,y);
}

ex num_Cmplx_RealReal(ex X, ex Y, CmplxFxn1 * s)
{
//std::cout << "num_ExpCmplx_RealReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(etor(Y)) << std::endl;
    assert(eis_real(X));
    assert(eis_real(Y));
    uex x(X);
    uex y(Y);
    acb_t z,w;
    *acb_realref(z) = *ereal_data(X);
    *acb_imagref(z) = *ereal_data(Y);
    uex wre(emake_real());
    uex wim(emake_real());
    *acb_realref(w) = *ereal_data(wre.get());
    *acb_imagref(w) = *ereal_data(wim.get());
    slong p = FLINT_MIN(ereal_number(X).wprec(),
                        ereal_number(Y).wprec());
    s->arb_fxn(w, z, p + s->extra_prec);
    *ereal_data(wre.get()) = *acb_realref(w);
    *ereal_data(wim.get()) = *acb_imagref(w);
    return emake_cmplx(wre.release(), wim.release());
}

ex num_Cmplx1(ex A, CmplxFxn1 * s)
{
    //assert(ehas_head_sym_length(A, gs.sym_sExp.get(), 1));
    assert(eis_cmplx(echild(A,1)));
    ex X = ecopy(ecmplx_real(echild(A,1)));
    ex Y = ecopy(ecmplx_imag(echild(A,1)));
    assert(eis_number(X));
    assert(eis_number(Y));
    assert(!eis_zero(Y));
    uint32_t tx = etype(X);
    uint32_t ty = etype(Y);
    switch (ETYPE_NUMBER * tx + ty)
    {
        case ETYPE_NUMBER * ETYPE_INT + ETYPE_INT:
        case ETYPE_NUMBER * ETYPE_INT + ETYPE_RAT:
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_INT:
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_RAT:
        default:
            eclear(X);
            eclear(Y);
            return s->exact_fxn(A);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_DOUBLE:
            eclear(A);
            return num_Cmplx_IntDouble(X, Y, s);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_INT:
             eclear(A);
           return num_Cmplx_DoubleInt(X, Y, s);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_DOUBLE:
            eclear(A);
            return num_Cmplx_RatDouble(X, Y, s);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_RAT:
            eclear(A);
            return num_Cmplx_DoubleRat(X, Y, s);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_DOUBLE:
            eclear(A);
            return num_Cmplx_DoubleDouble(X, Y, s);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_REAL:
            eclear(A);
            return num_Cmplx_IntReal(X, Y, s);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_INT:
            eclear(A);
            return num_Cmplx_RealInt(X, Y, s);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_REAL:
            eclear(A);
            return num_Cmplx_RatReal(X, Y, s);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_RAT:
            eclear(A);
            return num_Cmplx_RealRat(X, Y, s);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_REAL:
            eclear(A);
            return num_Cmplx_DoubleReal(X, Y, s);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_DOUBLE:
            eclear(A);
            return num_Cmplx_RealDouble(X, Y, s);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_REAL:
            eclear(A);
            return num_Cmplx_RealReal(X, Y, s);
    }
}

ex dcode_sExp(er e)
{
//std::cout << "dcode_sExp: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sExp.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    er X = echild(e,1);
    if (eis_number(X))
    {
        switch (etype(X))
        {
            case ETYPE_INT:
            {
                if (fmpz_is_zero(eint_data(X)))
                {
                    return emake_cint(1);
                }
                else if (fmpz_is_one(eint_data(X)))
                {
                    return gs.sym_sE.copy();
                }
                else
                {
                    return _exp_to_power(ecopy(e));
                }
            }
            case ETYPE_RAT:
            {
                return _exp_to_power(ecopy(e));
            }
            case ETYPE_DOUBLE:
            {
                return emake_double(exp(edouble_number(X)));
            }
            case ETYPE_REAL:
            {
				ex z = emake_real();
                slong p = ereal_number(X).wprec();
                arb_exp(ereal_data(z), ereal_data(X), p + EXTRA_PRECISION_BASIC);
                return efix_real(z);
            }
            case ETYPE_CMPLX:
            {
                CmplxFxn1 s;
                s.exact_fxn = &_exp_to_power;
                s.double_fxn = &cmplx_exp;
                s.arb_fxn = &acb_exp;
                s.double_imag_fxn = &double_imag_exp;
                s.arb_imag_fxn = &arb_imag_exp;
                s.extra_prec = EXTRA_PRECISION_BASIC;
                return num_Cmplx1(ecopy(e), &s);
            }
            default:
            {
                assert(false);
                return ecopy(e);
            }
        }
    }
    else if (ehas_head_sym_length(X, gs.sym_sLog.get(),1))
    {
        return ecopychild(X,1);
    }
    else if (ehas_head_sym(X, gs.sym_sTimes.get()))
    {
        size_t n = elength(X);
        if (n == 2)
        {
            bool b = ehas_head_sym_length(echild(X,2), gs.sym_sLog.get(), 1);
            if (ehas_head_sym_length(echild(X,1), gs.sym_sLog.get(), 1))
            {
                return b ? _exp_to_power(ecopy(e))
                         : emake_node(gs.sym_sPower.copy(), ecopychild(X,1,1), ecopychild(X,2));
            }
            else
            {
                return !b ? _exp_to_power(ecopy(e))
                         : emake_node(gs.sym_sPower.copy(), ecopychild(X,2,1), ecopychild(X,1));
            }
        }
        else if (n < 2)
        {
            return ecopy(e);
        }
        else
        {   
            size_t c = 0;
            for (size_t i = 1; i <= n; i++)
            {
                if (ehas_head_sym_length(echild(X,i), gs.sym_sLog.get(),1))
                {
                    if (c == 0)
                        c = i;
                    else
                        return _exp_to_power(ecopy(e));
                }
            }
            if (c == 0)
                return _exp_to_power(ecopy(e));
            uex f(ecopy(X));
            f.removechild(c);
            return emake_node(gs.sym_sPower.copy(), ecopychild(X,c,1), f.release());
        }
    }
    else
    {
        return _exp_to_power(ecopy(e));
    }
}

ex dcode_sLog(er e)
{
//std::cout << "dcode_sLog: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sLog.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    er X = echild(e,1);
    if (eis_number(X))
    {
        switch (etype(X))
        {
            case ETYPE_INT:
            {
                if (fmpz_is_zero(eint_data(X)))
                {
                    return gs.const_minfinity.copy();
                }
                else if (fmpz_is_one(eint_data(X)))
                {
                    return emake_cint(0);
                }
                else
                {
                    return ecopy(e);
                }
            }
            case ETYPE_RAT:
            {
                return ecopy(e);
            }
            case ETYPE_DOUBLE:
            {
                ex z = emake_double();
                double d = edouble_number(X);
                if (d > 0)
                {
                    edouble_number(z) = log(d);
                    return z;
                }
                else if (d < 0)
                {
                    edouble_number(z) = log(-d);
                    return emake_cmplx(z, gs.const_double_pi.copy());
                }
                else
                {
                    eclear(z);
                    return gs.const_indeterminate.copy();
                }
            }
            case ETYPE_REAL:
            {
                slong p = ereal_number(X).wprec() + EXTRA_PRECISION_BASIC;
                ex Z = emake_real();
                uex z(Z);
                if (arb_is_positive(ereal_data(X)))
                {
                    arb_log(ereal_data(Z), ereal_data(X), p);
                    return efix_real(z.release());
                }
                else if (arb_is_negative(ereal_data(X)))
                {
                    arb_neg(ereal_data(Z), ereal_data(X));
                    arb_log(ereal_data(Z), ereal_data(Z), p);
                    uex pi(emake_real());
                    arb_const_pi(ereal_data(pi.get()), ereal_number(z.get()).wprec());
                    ex t = efix_real(z.release());
                    return emake_cmplx(t, pi.release());
                }
                else
                {
                    return emake_nan_Indeterminate();
                }
            }
            default:
            {
                assert(false);
                return ecopy(e);
            }
        }
    }
    else if (eis_sym(X, gs.sym_sE.get()))
    {
        return emake_cint(1);
    }
    else
    {
        return ecopy(e);
    }
}


ex dcode_sCos(er e)
{
//std::cout << "dcode_sCos: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sCos.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    er X = echild(e,1);
    if (eis_number(X))
    {
        switch (etype(X))
        {
            case ETYPE_INT:
            {
                if (fmpz_is_zero(eint_data(X)))
                    return emake_cint(1);
                else
                    return ecopy(e);
            }
            case ETYPE_RAT:
            {
                return ecopy(e);
            }
            case ETYPE_DOUBLE:
            {
				return emake_double(cos(edouble_number(X)));
            }
            case ETYPE_REAL:
            {
                ex z = emake_real();
                slong p = ereal_number(X).wprec();
                arb_cos(ereal_data(z), ereal_data(X), p + EXTRA_PRECISION_BASIC);
                return efix_real(z);
            }
            case ETYPE_CMPLX:
            {
                CmplxFxn1 s;
                s.exact_fxn = &trivial;
                s.double_fxn = &cmplx_cos;
                s.arb_fxn = &acb_cos;
                s.double_imag_fxn = &double_imag_cos;
                s.arb_imag_fxn = &arb_imag_cos;
                s.extra_prec = EXTRA_PRECISION_BASIC;
                return num_Cmplx1(ecopy(e), &s);
            }
            default:
            {
                return emake_nan_Overflow();
            }
        }
    }
    else
    {
        return ecopy(e);
    }
}


ex dcode_sSin(er e)
{
//std::cout << "dcode_sSin: " << ex_tostring_full(e.get()) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sSin.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    er X = echild(e,1);
    if (eis_number(X))
    {
        switch (etype(X))
        {
            case ETYPE_INT:
            {
                if (fmpz_is_zero(eint_data(X)))
                    return emake_cint(0);
                else
                    return ecopy(e);
            }
            case ETYPE_RAT:
            {
                return ecopy(e);
            }
            case ETYPE_DOUBLE:
            {
                return emake_double(sin(edouble_number(X)));
            }
            case ETYPE_REAL:
            {
				ex z = emake_real();
                slong p = ereal_number(X).wprec();
                arb_sin(ereal_data(z), ereal_data(X), p + EXTRA_PRECISION_BASIC);
                return efix_real(z);
            }
            case ETYPE_CMPLX:
            {
                CmplxFxn1 s;
                s.exact_fxn = &trivial;
                s.double_fxn = &cmplx_sin;
                s.arb_fxn = &acb_sin;
                s.double_imag_fxn = &double_imag_sin;
                s.arb_imag_fxn = &arb_imag_sin;
                s.extra_prec = EXTRA_PRECISION_BASIC;
                return num_Cmplx1(ecopy(e), &s);
            }
            default:
            {
                return emake_nan_Overflow();
            }
        }
    }
    else
    {
        return ecopy(e);
    }
}


ex dcode_sCosh(er e)
{
//std::cout << "dcode_sCosh: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sCosh.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    er X = echild(e,1);
    if (eis_number(X))
    {
        switch (etype(X))
        {
            case ETYPE_INT:
            {
                if (fmpz_is_zero(eint_data(X)))
                    return emake_cint(1);
                else
                    return ecopy(e);
            }
            case ETYPE_RAT:
            {
                return ecopy(e);
            }
            case ETYPE_DOUBLE:
            {
                return emake_double(sinh(edouble_number(X)));
            }
            case ETYPE_REAL:
            {
				ex z = emake_real();
                slong p = ereal_number(X).wprec();
                arb_sinh(ereal_data(z), ereal_data(X), p + EXTRA_PRECISION_BASIC);
                return efix_real(z);
            }
            case ETYPE_CMPLX:
            {
                CmplxFxn1 s;
                s.exact_fxn = &trivial;
                s.double_fxn = &cmplx_cosh;
                s.arb_fxn = &acb_cosh;
                s.double_imag_fxn = &double_imag_cosh;
                s.arb_imag_fxn = &arb_imag_cosh;
                s.extra_prec = EXTRA_PRECISION_BASIC;
                return num_Cmplx1(ecopy(e), &s);
            }
            default:
            {
                return ecopy(e);
            }
        }
    }
    else
    {
        return ecopy(e);
    }
}

ex dcode_sSinh(er e)
{
//std::cout << "dcode_sSinh: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sSinh.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    er X = echild(e,1);
    if (eis_number(X))
    {
        switch (etype(X))
        {
            case ETYPE_INT:
            {
                if (fmpz_is_zero(eint_data(X)))
                    return emake_cint(0);
                else
                    return ecopy(e);
            }
            case ETYPE_RAT:
            {
                return ecopy(e);
            }
            case ETYPE_DOUBLE:
            {
                return emake_double(sinh(edouble_number(X)));
            }
            case ETYPE_REAL:
            {
				ex z = emake_real();
                slong p = ereal_number(X).wprec();
                arb_sinh(ereal_data(z), ereal_data(X), p + EXTRA_PRECISION_BASIC);
                return efix_real(z);
            }
            case ETYPE_CMPLX:
            {
                CmplxFxn1 s;
                s.exact_fxn = &trivial;
                s.double_fxn = &cmplx_sinh;
                s.arb_fxn = &acb_sinh;
                s.double_imag_fxn = &double_imag_sinh;
                s.arb_imag_fxn = &arb_imag_sinh;
                s.extra_prec = EXTRA_PRECISION_BASIC;
                return num_Cmplx1(ecopy(e), &s);
            }

            default:
            {
                return ecopy(e);
            }
        }
    }
    else
    {
        return ecopy(e);
    }
}

ex dcode_sTan(er e)
{
//std::cout << "dcode_sTan: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sTan.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    er X = echild(e,1);
    if (eis_number(X))
    {
        switch (etype(X))
        {
            case ETYPE_INT:
            {
                if (fmpz_is_zero(eint_data(X)))
                    return emake_cint(0);
                else
                    return ecopy(e);
            }
            case ETYPE_RAT:
            {
                return ecopy(e);
            }
            case ETYPE_DOUBLE:
            {
                emake_double(tan(edouble_number(X)));
            }
            case ETYPE_REAL:
            {
				ex z = emake_real();
                slong p = ereal_number(X).wprec();
                arb_tan(ereal_data(z), ereal_data(X), p + EXTRA_PRECISION_BASIC);
                return efix_real(z);
            }
            case ETYPE_CMPLX:
            {
                CmplxFxn1 s;
                s.exact_fxn = &trivial;
                s.double_fxn = &cmplx_tan;
                s.arb_fxn = &acb_tan;
                s.double_imag_fxn = &double_imag_tan;
                s.arb_imag_fxn = &arb_imag_tan;
                s.extra_prec = EXTRA_PRECISION_BASIC;
                return num_Cmplx1(ecopy(e), &s);
            }

            default:
            {
                return ecopy(e);
            }
        }
    }
    else
    {
        return ecopy(e);
    }
}

ex dcode_sTanh(er e)
{
//std::cout << "dcode_sTanh: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sTanh.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    er X = echild(e,1);
    if (eis_number(X))
    {
        switch (etype(X))
        {
            case ETYPE_INT:
            {
                if (fmpz_is_zero(eint_data(X)))
                    return emake_cint(0);
                else
                    return ecopy(e);
            }
            case ETYPE_RAT:
            {
                return ecopy(e);
            }
            case ETYPE_DOUBLE:
            {
                return emake_double(std::tanh(edouble_number(X)));
            }
            case ETYPE_REAL:
            {
				ex z = emake_real();
                slong p = ereal_number(X).wprec();
                arb_tanh(ereal_data(z), ereal_data(X), p + EXTRA_PRECISION_BASIC);
                return efix_real(z);
            }
            case ETYPE_CMPLX:
            {
                CmplxFxn1 s;
                s.exact_fxn = &trivial;
                s.double_fxn = &cmplx_tanh;
                s.arb_fxn = &acb_tanh;
                s.double_imag_fxn = &double_imag_tanh;
                s.arb_imag_fxn = &arb_imag_tanh;
                s.extra_prec = EXTRA_PRECISION_BASIC;
                return num_Cmplx1(ecopy(e), &s);
            }

            default:
            {
                return ecopy(e);
            }
        }
    }
    else
    {
        return ecopy(e);
    }
}

ex dcode_sLog10(er e)
{
    if (elength(e) != 1)
        return _handle_message_argx1(e);

    return ecopy(e);
}


ex dcode_sArcCos(er e)
{
//std::cout << "dcode_sArcCos: " << ex_tostring_full(e.get()) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sArcCos.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    er X = echild(e,1);
    if (eis_number(X))
    {
        switch (etype(X))
        {
            case ETYPE_INT:
            {
                if (fmpz_is_zero(eint_data(X)))
                    return emake_node(gs.sym_sTimes.copy(), emake_crat(1,2), gs.sym_sPi.copy());
                else
                    return ecopy(e);
            }
            case ETYPE_RAT:
            {
                return ecopy(e);
            }
            case ETYPE_DOUBLE:
            {
                double d = edouble_number(X);
                if (d > 1)
                {
                    return emake_cmplx(emake_cint(0), emake_double(std::acosh(d)));
                }
                if (d < -1)
                {
                    return emake_cmplx(gs.const_double_pi.copy(), emake_double(-std::acosh(-d)));
                }
                return emake_double(acos(d));
            }
            case ETYPE_REAL:
            {
                slong p = ereal_number(X).wprec() + EXTRA_PRECISION_BASIC;
                ex Z = emake_real();
                uex z(Z);
                xarb_t one;
                arb_one(one.data);
                if (arb_gt(ereal_data(X), one.data))
                {
                    arb_acosh(ereal_data(Z), ereal_data(X), p);
                    ex t = efix_real(z.release());
                    return emake_cmplx(emake_cint(0), t);
                }
                arb_neg(one.data, one.data);
                if (arb_lt(ereal_data(X), one.data))
                {
                    arb_neg(ereal_data(Z), ereal_data(X));
                    arb_acosh(ereal_data(Z), ereal_data(Z), p);
                    arb_neg(ereal_data(Z), ereal_data(Z));
                    uex pi(emake_real());
                    arb_const_pi(ereal_data(pi.get()), ereal_number(z.get()).wprec());
                    ex t = efix_real(z.release());
                    return emake_cmplx(pi.release(), t);
                }
                arb_acos(ereal_data(Z), ereal_data(X), p);
                return efix_real(z.release());
            }
            case ETYPE_CMPLX:
            {
                CmplxFxn1 s;
                s.exact_fxn = &trivial;
                s.double_fxn = &cmplx_acos;
                s.arb_fxn = &acb_acos;
                s.double_imag_fxn = &double_imag_acos;
                s.arb_imag_fxn = &arb_imag_acos;
                s.extra_prec = EXTRA_PRECISION_BASIC;
                return num_Cmplx1(ecopy(e), &s);
            }
            default:
            {
                return emake_nan_Indeterminate();
            }
        }
    }
    else
    {
        return ecopy(e);
    }
}

ex dcode_sArcCosh(er e)
{
//std::cout << "dcode_sArcCosh: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sArcCosh.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    er X = echild(e,1);
    if (eis_number(X))
    {
        switch (etype(X))
        {
            case ETYPE_INT:
            {
                if (fmpz_is_zero(eint_data(X)))
                {
                    ex t = emake_crat(1,2);
                    t = emake_cmplx(emake_cint(0), t);
                    return emake_node(gs.sym_sTimes.copy(), t, gs.sym_sPi.copy());
                }
                else
                {
                    return ecopy(e);
                }
            }
            case ETYPE_RAT:
            {
                return ecopy(e);
            }
            case ETYPE_DOUBLE:
            {
                double d = edouble_number(X);
                if (d >= 1)
                {
                    return emake_double(acosh(d));
                }
                if (d <= -1)
                {
                    ex t = emake_double(std::acosh(-d));
                    return emake_cmplx(t, ecopy(gs.const_double_pi.get()));
                }
                ex t = emake_double(std::acos(d));
                return emake_cmplx(emake_cint(0), t);
            }
            case ETYPE_REAL:
            {
                slong p = ereal_number(X).wprec() + EXTRA_PRECISION_BASIC;
                ex Z = emake_real();
                uex z(Z);
                xarb_t one;
                arb_one(one.data);
                if (arb_ge(ereal_data(X), one.data))
                {
                    arb_acosh(ereal_data(Z), ereal_data(X), p);
                    return efix_real(z.release());
                }
                arb_neg(one.data, one.data);
                if (arb_le(ereal_data(X), one.data))
                {
                    arb_neg(ereal_data(Z), ereal_data(X));
                    arb_acosh(ereal_data(Z), ereal_data(Z), p);
                    uex pi(emake_real());
                    arb_const_pi(ereal_data(pi.get()), ereal_number(z.get()).wprec());
                    ex t = efix_real(z.release());
                    return emake_cmplx(t, pi.release());
                }
                arb_acos(ereal_data(Z), ereal_data(X), p);
                ex t = efix_real(z.release());
                return emake_cmplx(emake_cint(0), t);
            }
            case ETYPE_CMPLX:
            {
                CmplxFxn1 s;
                s.exact_fxn = &trivial;
                s.double_fxn = &cmplx_acosh;
                s.arb_fxn = &acb_acosh;
                s.double_imag_fxn = &double_imag_acosh;
                s.arb_imag_fxn = &arb_imag_acosh;
                s.extra_prec = EXTRA_PRECISION_BASIC;
                return num_Cmplx1(ecopy(e), &s);
            }

            default:
            {
                return ecopy(e);
            }
        }
    }
    else
    {
        return ecopy(e);
    }
}


ex dcode_sArcSin(er e)
{
//std::cout << "dcode_sArcSin: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sArcSin.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    er X = echild(e,1);
    if (eis_number(X))
    {
        switch (etype(X))
        {
            case ETYPE_INT:
            {
                if (fmpz_is_zero(eint_data(X)))
                {
                    return emake_cint(0);
                }
                else
                {
                    return ecopy(e);
                }
            }
            case ETYPE_RAT:
            {
                return ecopy(e);
            }
            case ETYPE_DOUBLE:
            {
                double d = edouble_number(X);
                if (d > 1)
                {
                    ex t = emake_double(-acosh(d));
                    return emake_cmplx(ecopy(gs.const_double_halfpi.get()), t);
                }
                if (d < -1)
                {
                    ex t = emake_double(acosh(-d));
                    return emake_cmplx(ecopy(gs.const_double_mhalfpi.get()), t);
                }
                return emake_double(asin(d));
            }
            case ETYPE_REAL:
            {
                slong p = ereal_number(X).wprec() + EXTRA_PRECISION_BASIC;
                ex Z = emake_real();
                uex z(Z);
                xarb_t one;
                arb_one(one.data);
                if (arb_gt(ereal_data(X), one.data))
                {
                    arb_acosh(ereal_data(Z), ereal_data(X), p);
                    arb_neg(ereal_data(Z), ereal_data(Z));                    
                    uex pi(emake_real());
                    arb_const_pi(ereal_data(pi.get()), ereal_number(z.get()).wprec());
                    arb_mul_2exp_si(ereal_data(pi.get()), ereal_data(pi.get()), -WORD(1));
                    ex t = efix_real(z.release());
                    return emake_cmplx(pi.release(), t);
                }
                arb_neg(one.data, one.data);
                if (arb_lt(ereal_data(X), one.data))
                {
                    arb_neg(ereal_data(Z), ereal_data(X));
                    arb_acosh(ereal_data(Z), ereal_data(Z), p);
                    uex pi(emake_real());
                    arb_const_pi(ereal_data(pi.get()), ereal_number(z.get()).wprec());
                    arb_neg(ereal_data(pi.get()), ereal_data(pi.get()));
                    arb_mul_2exp_si(ereal_data(pi.get()), ereal_data(pi.get()), -WORD(1));
                    ex t = efix_real(z.release());
                    return emake_cmplx(pi.release(), t);
                }
                arb_asin(ereal_data(Z), ereal_data(X), p);
                return efix_real(z.release());
            }
            case ETYPE_CMPLX:
            {
                CmplxFxn1 s;
                s.exact_fxn = &trivial;
                s.double_fxn = &cmplx_asin;
                s.arb_fxn = &acb_asin;
                s.double_imag_fxn = &double_imag_asin;
                s.arb_imag_fxn = &arb_imag_asin;
                s.extra_prec = EXTRA_PRECISION_BASIC;
                return num_Cmplx1(ecopy(e), &s);
            }

            default:
            {
                return ecopy(e);
            }
        }
    }
    else
    {
        return ecopy(e);
    }
}

ex dcode_sArcSinh(er e)
{
//std::cout << "dcode_sArcSinh: " << ex_tostring_full(e.get()) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sArcSinh.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    er X = echild(e,1);
    if (eis_number(X))
    {
        switch (etype(X))
        {
            case ETYPE_INT:
            {
                if (fmpz_is_zero(eint_data(X)))
                {
                    return emake_cint(0);
                }
                else
                {
                    return ecopy(e);
                }
            }
            case ETYPE_RAT:
            {
                return ecopy(e);
            }
            case ETYPE_DOUBLE:
            {
                double d = edouble_number(X);
                return emake_double(asinh(d));
            }
            case ETYPE_REAL:
            {
                slong p = ereal_number(X).wprec() + EXTRA_PRECISION_BASIC;
                ex Z = emake_real();
                arb_asinh(ereal_data(Z), ereal_data(X), p);
                return efix_real(Z);
            }
            case ETYPE_CMPLX:
            {
                CmplxFxn1 s;
                s.exact_fxn = &trivial;
                s.double_fxn = &cmplx_asinh;
                s.arb_fxn = &acb_asinh;
                s.double_imag_fxn = &double_imag_asinh;
                s.arb_imag_fxn = &arb_imag_asinh;
                s.extra_prec = EXTRA_PRECISION_BASIC;
                return num_Cmplx1(ecopy(e), &s);
            }
            default:
            {
                return ecopy(e);
            }
        }
    }
    else
    {
        return ecopy(e);
    }
}


ex ucode_ArcTan(er e)
{
std::cout << "ucode_ArcTan: " << ex_tostring_full(e) << std::endl;
    assert(eis_node(e));
    
    if (ehas_head_sym_length(e, gs.sym_sPlus.get(), 2)
        && ehas_head_sym_length(echild(e,1), gs.sym_sArcTan.get(), 1)
        && ehas_head_sym_length(echild(e,2), gs.sym_sArcTan.get(), 1)
       )
    {
        uex num(ex_add(echild(e,1,1), echild(e,2,1)));
        ex t = ex_mul(echild(e,1,1), echild(e,2,1));
        uex den(ex_sub(emake_cint(1), t));
        t = ex_div(num.release(), den.release());
        return emake_node(gs.sym_sArcTan.copy(), t);
    }
    return ecopy(e);
}

ex dcode_sArcTan(er e)
{
//std::cout << "dcode_sArcTan: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sArcTan.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    er X = echild(e,1);
    if (eis_number(X))
    {
        switch (etype(X))
        {
            case ETYPE_INT:
            {
                if (fmpz_is_zero(eint_data(X)))
                {
                    return emake_cint(0);
                }
                else
                {
                    return ecopy(e);
                }
            }
            case ETYPE_RAT:
            {
                return ecopy(e);
            }
            case ETYPE_DOUBLE:
            {
                double d = edouble_number(X);
                eclear(ecopy(e));
                return emake_double(atan(d));
            }
            case ETYPE_REAL:
            {
                slong p = ereal_number(X).wprec() + EXTRA_PRECISION_BASIC;
                ex Z = emake_real();
                arb_atan(ereal_data(Z), ereal_data(X), p);
                return efix_real(Z);
            }
            case ETYPE_CMPLX:
            {
                CmplxFxn1 s;
                s.exact_fxn = &trivial;
                s.double_fxn = &cmplx_atan;
                s.arb_fxn = &acb_atan;
                s.double_imag_fxn = &double_imag_atan;
                s.arb_imag_fxn = &arb_imag_atan;
                s.extra_prec = EXTRA_PRECISION_BASIC;
                return num_Cmplx1(ecopy(e), &s);
            }
            default:
            {
                return ecopy(e);
            }
        }
    }
    else
    {
        return ecopy(e);
    }
}


ex arb_real_atanh(ex X)
{
    uex x(X);
    slong p = ereal_number(X).wprec() + EXTRA_PRECISION_BASIC;
    ex Z = emake_real();
    uex z(Z);
    xarb_t one;
    arb_one(one.data);
    if (arb_gt(ereal_data(X), one.data))
    {
        /* 1/2 Log[1 + 2/(y-1)] */
        arb_sub_ui(ereal_data(Z), ereal_data(X), 1, p);
        arb_ui_div(ereal_data(Z), 2, ereal_data(Z), p);
        arb_log1p(ereal_data(Z), ereal_data(Z), p);
        arb_mul_2exp_si(ereal_data(Z), ereal_data(Z), -WORD(1));
        uex pi(emake_real());
        arb_const_pi(ereal_data(pi.get()), p);
        arb_mul_2exp_si(ereal_data(pi.get()), ereal_data(pi.get()), -WORD(1));
        arb_neg(ereal_data(pi.get()), ereal_data(pi.get()));
        ex t = efix_real(z.release());
        return emake_cmplx(t, pi.release());
    }
    arb_neg(one.data, one.data);
    if (arb_lt(ereal_data(X), one.data))
    {
        /* -1/2 Log[1 + 2/(-y - 1)] */
        arb_add_ui(ereal_data(Z), ereal_data(X), UWORD(1), p);
        arb_neg(ereal_data(Z), ereal_data(Z));
        arb_ui_div(ereal_data(Z), UWORD(2), ereal_data(Z), p);
        arb_log1p(ereal_data(Z), ereal_data(Z), p);
        arb_mul_2exp_si(ereal_data(Z), ereal_data(Z), -WORD(1));
        arb_neg(ereal_data(Z), ereal_data(Z));
        uex pi(emake_real());
        arb_const_pi(ereal_data(pi.get()), p);
        arb_mul_2exp_si(ereal_data(pi.get()), ereal_data(pi.get()), -WORD(1));
        ex t = efix_real(z.release());
        return emake_cmplx(t, pi.release());
    }
    bool negate = arb_is_negative(ereal_data(X));
        /* -1/2 Log[1 + 2y/(-y - 1)] */
        /* 1/2 Log[1 + 2y/(-y + 1)] */
    if (negate)
        arb_add_ui(ereal_data(Z), ereal_data(X), UWORD(1), p);
    else
        arb_sub_ui(ereal_data(Z), ereal_data(X), UWORD(1), p);
    arb_neg(ereal_data(Z), ereal_data(Z));
    arb_div(ereal_data(Z), ereal_data(X), ereal_data(Z), p);
    arb_mul_2exp_si(ereal_data(Z), ereal_data(Z), WORD(1));
    arb_log1p(ereal_data(Z), ereal_data(Z), p);
    if (negate)
        arb_neg(ereal_data(Z), ereal_data(Z));
    arb_mul_2exp_si(ereal_data(Z), ereal_data(Z), -WORD(1));
    return efix_real(z.release());
}

ex dcode_sArcTanh(er e)
{
//std::cout << "dcode_sArcTanh: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sArcTanh.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    er X = echild(e,1);
    if (eis_number(X))
    {
        switch (etype(X))
        {
            case ETYPE_INT:
            {
                if (fmpz_is_zero(eint_data(X)))
                {
                    return emake_cint(0);
                }
                else
                {
                    return ecopy(e);
                }
            }
            case ETYPE_RAT:
            {
                return ecopy(e);
            }
            case ETYPE_DOUBLE:
            {
                double y = edouble_number(X);
                eclear(ecopy(e));
                ex wx, wy;
                if (y > 1)
                {
                    wx = emake_double(0.5*log1p(2/(y-1)));
                    wy = ecopy(gs.const_double_mhalfpi.get());
                    return emake_cmplx(wx, wy);
                }
                else if (y < -1)
                {
                    wx = emake_double(-0.5*log1p(2/(-y-1)));
                    wy = ecopy(gs.const_double_halfpi.get());
                    return emake_cmplx(wx, wy);
                }
                else
                {
                    return emake_double(atanh(y));
                }
            }
            case ETYPE_REAL:
            {
                ex t = ecopy(X);
                eclear(ecopy(e));
                return arb_real_atanh(t);
            }
            case ETYPE_CMPLX:
            {
                CmplxFxn1 s;
                s.exact_fxn = &trivial;
                s.double_fxn = &cmplx_atanh;
                s.arb_fxn = &acb_atanh;
                s.double_imag_fxn = &double_imag_atanh;
                s.arb_imag_fxn = &arb_imag_atanh;
                s.extra_prec = EXTRA_PRECISION_BASIC;
                return num_Cmplx1(ecopy(e), &s);
            }
            default:
            {
                return ecopy(e);
            }
        }
    }
    else
    {
        return ecopy(e);
    }
}

ex dcode_sArcCot(er e)
{
//std::cout << "dcode_sArcCot: " << ex_tostring_full(e.get()) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sArcCot.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    return ecopy(e);
}

ex dcode_sArcCoth(er e)
{
//std::cout << "dcode_sArcCoth: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sArcCoth.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    return ecopy(e);
}

ex dcode_sArcCsc(er e)
{
//std::cout << "dcode_sArcCsc: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sArcCsc.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    return ecopy(e);
}

ex dcode_sArcCsch(er e)
{
//std::cout << "dcode_sArcCoth: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sArcCsch.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    return ecopy(e);
}

ex dcode_sArcSec(er e)
{
//std::cout << "dcode_sArcSec: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sArcSec.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    return ecopy(e);
}

ex dcode_sArcSech(er e)
{
//std::cout << "dcode_sArcSech: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sArcSech.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    return ecopy(e);
}


ex dcode_sCot(er e)
{
//std::cout << "dcode_sCot: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sCot.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    return ecopy(e);
}

ex dcode_sCoth(er e)
{
//std::cout << "dcode_sCoth: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sCoth.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    return ecopy(e);
}


ex dcode_sCsc(er e)
{
//std::cout << "dcode_sCot: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sCsc.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    return ecopy(e);
}

ex dcode_sCsch(er e)
{
//std::cout << "dcode_sCoth: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sCsch.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    return ecopy(e);
}


ex dcode_sSec(er e)
{
//std::cout << "dcode_sSec: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sSec.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    return ecopy(e);
}

ex dcode_sSech(er e)
{
//std::cout << "dcode_sSech: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sSech.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    return ecopy(e);
}
