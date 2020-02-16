#include "globalstate.h"
#include "arithmetic.h"
#include "ex_print.h"


int num_CmpIntInt(er x, er y)
{
//std::cout << "num_CmpIntInt: " << ex_tostring_full(x) << ", " << ex_tostring_full(y) << std::endl;
    assert(eis_int(x));
    assert(eis_int(y));
    return fmpz_cmp(eint_data(x), eint_data(y));
}

int num_CmpIntRat(er x, er y)
{
    assert(eis_int(x));
    assert(eis_rat(y));
    fmpq_t t;
    *fmpq_numref(t) = *(eint_data(x));
    *fmpq_denref(t) = WORD(1);
    return fmpq_cmp(t, erat_data(y));
}

int num_CmpRatRat(er x, er y)
{
//std::cout << "num_CmpRatRat: " << ex_tostring_full(x) << ", " << ex_tostring_full(y) << std::endl;
    assert(eis_rat(x));
    assert(eis_rat(y));
    return fmpq_cmp(erat_data(x), erat_data(y));
}

int num_CmpIntDouble(er X, er Y)
{
    double x = fmpz_get_d(eint_data(X));
    double y = edouble_number(Y);
    return (x > y) ? 1 : -1;
}

int num_CmpRatDouble(er X, er Y)
{
    return 0;
}

int num_CmpDoubleDouble(er X, er Y)
{
    double x = edouble_number(X);
    double y = edouble_number(Y);
    if (x < y)
        return -1;
    else if (x > y)
        return 1;
    else
        return 0;
}


int num_CmpIntReal(er X, er Y)
{
    arf_t f;
    arf_init(f);
    arf_set_fmpz(f, eint_data(X));
    int cmp = arf_cmp(f, arb_midref(ereal_data(Y)));
    arf_clear(f);
    return cmp > 0 ? 1 : -1;
}

int num_CmpRatReal(er X, er Y)
{
    arf_t f;
    arf_init(f);
    slong p = arf_bits(arb_midref(ereal_data(Y)));
    arf_fmpz_div_fmpz(f, fmpq_numref(erat_data(X)), fmpq_denref(erat_data(X)), p + 10, ARF_RND_NEAR);
    int cmp = arf_cmp(f, arb_midref(ereal_data(Y)));
    arf_clear(f);
    return cmp > 0 ? 1 : -1;
}

int num_CmpDoubleReal(er X, er Y)
{
    return 0;
}

int num_CmpRealReal(er X, er Y)
{
    int cmp = arf_cmp(arb_midref(ereal_data(X)), arb_midref(ereal_data(Y)));
    if (cmp != 0)
        return cmp > 0 ? 1 : -1;
    return mag_cmp(arb_radref(ereal_data(X)), arb_radref(ereal_data(Y)));
}

int num_CmpIntCplx(er X, er Y)
{
    int cmp = num_Cmp2(X, ecmplx_real(Y));
    if (cmp != 0)
        return cmp > 0 ? 1 : -1;
    return num_Cmp2(eget_cint(0), ecmplx_imag(Y));
}

int num_CmpRatCplx(er X, er Y)
{
    int cmp = num_Cmp2(X, ecmplx_real(Y));
    if (cmp != 0)
        return cmp > 0 ? 1 : -1;
    return num_Cmp2(eget_cint(0), ecmplx_imag(Y));
}

int num_CmpRealCplx(er X, er Y)
{
    int cmp = num_Cmp2(X, ecmplx_real(Y));
    if (cmp != 0)
        return cmp > 0 ? 1 : -1;
    return num_Cmp2(eget_cint(0), ecmplx_imag(Y));
}

int num_CmpCplxCplx(er X, er Y)
{
    int cmp = num_Cmp2(ecmplx_real(X), ecmplx_real(Y));
    if (cmp != 0)
        return cmp > 0 ? 1 : -1;
    return num_Cmp2(ecmplx_imag(X), ecmplx_imag(Y));
}

int num_Cmp2(er X, er Y)
{
//std::cout << "num_Cmp2: " << ex_tostring(X) << ", " << ex_tostring(Y) << std::endl;
    assert(eis_number(X));
    assert(eis_number(Y));
    uint32_t tx = etype(X);
    uint32_t ty = etype(Y);
    switch (ETYPE_NUMBER * tx + ty)
    {
        case ETYPE_NUMBER * ETYPE_INT + ETYPE_INT:
            return num_CmpIntInt(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_RAT:
            return num_CmpIntRat(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_INT:
            return -num_CmpIntRat(Y, X);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_RAT:
            return num_CmpRatRat(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_DOUBLE:
            return num_CmpIntDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_INT:
            return -num_CmpIntDouble(Y, X);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_DOUBLE:
            return num_CmpRatDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_RAT:
            return -num_CmpRatDouble(Y, X);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_DOUBLE:
            return num_CmpDoubleDouble(X, Y);


        case ETYPE_NUMBER * ETYPE_INT + ETYPE_REAL:
            return num_CmpIntReal(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_INT:
            return -num_CmpIntReal(Y, X);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_REAL:
            return num_CmpRatReal(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_RAT:
            return -num_CmpRatReal(Y, X);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_REAL:
            return num_CmpDoubleReal(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_DOUBLE:
            return -num_CmpDoubleReal(Y, X);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_REAL:
            return num_CmpRealReal(X, Y);



        case ETYPE_NUMBER * ETYPE_INT + ETYPE_CMPLX:
            return num_CmpIntCplx(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_CMPLX:
            return num_CmpRatCplx(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_CMPLX:
            return num_CmpRealCplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_INT:
            return -num_CmpIntCplx(Y, X);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_RAT:
            return -num_CmpRatCplx(Y, X);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_REAL:
            return -num_CmpRealCplx(Y, X);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_CMPLX:
            return num_CmpCplxCplx(X, Y);

        default:
            return 0;
    }
}


bool num_Less_IntInt(er x, er y)
{
//std::cout << "num_CmpIntInt: " << ex_tostring_full(x) << ", " << ex_tostring_full(y) << std::endl;
    assert(eis_int(x));
    assert(eis_int(y));
    return fmpz_cmp(eint_data(x), eint_data(y)) < 0;
}

bool num_Less_IntRat(er x, er y)
{
    assert(eis_int(x));
    assert(eis_rat(y));
    fmpq_t t;
    *fmpq_numref(t) = *(eint_data(x));
    *fmpq_denref(t) = WORD(1);
    return fmpq_cmp(t, erat_data(y)) < 0;
}

bool num_Less_RatInt(er y, er x)
{
    assert(eis_int(x));
    assert(eis_rat(y));
    fmpq_t t;
    *fmpq_numref(t) = *(eint_data(x));
    *fmpq_denref(t) = WORD(1);
    return fmpq_cmp(erat_data(y), t) < 0;
}

bool num_Less_RatRat(er x, er y)
{
//std::cout << "num_CmpRatRat: " << ex_tostring_full(x) << ", " << ex_tostring_full(y) << std::endl;
    assert(eis_rat(x));
    assert(eis_rat(y));
    return fmpq_cmp(erat_data(x), erat_data(y)) < 0;
}

bool num_Less_RealReal(er X, er Y)
{
    return arb_lt(ereal_data(X), ereal_data(Y));
}

bool num_Less2(er X, er Y)
{
//std::cout << "num_Less2: " << ex_tostring(X) << ", " << ex_tostring(Y) << std::endl;
    assert(eis_number(X));
    assert(eis_number(Y));
    uint32_t tx = etype(X);
    uint32_t ty = etype(Y);
    switch (ETYPE_NUMBER * tx + ty)
    {
        case ETYPE_NUMBER * ETYPE_INT + ETYPE_INT:
            return num_Less_IntInt(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_RAT:
            return num_Less_IntRat(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_INT:
            return num_Less_RatInt(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_RAT:
            return num_Less_RatRat(X, Y);

        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_REAL:
            return num_Less_RealReal(X, Y);

        default:
            assert(false);
            return false;
    }
}


bool num_LessEqual_IntInt(er x, er y)
{
//std::cout << "num_CmpIntInt: " << ex_tostring_full(x) << ", " << ex_tostring_full(y) << std::endl;
    assert(eis_int(x));
    assert(eis_int(y));
    return fmpz_cmp(eint_data(x), eint_data(y)) <= 0;
}

bool num_LessEqual_IntRat(er x, er y)
{
    assert(eis_int(x));
    assert(eis_rat(y));
    fmpq_t t;
    *fmpq_numref(t) = *(eint_data(x));
    *fmpq_denref(t) = WORD(1);
    return fmpq_cmp(t, erat_data(y)) <= 0;
}

bool num_LessEqual_RatInt(er y, er x)
{
    assert(eis_int(x));
    assert(eis_rat(y));
    fmpq_t t;
    *fmpq_numref(t) = *(eint_data(x));
    *fmpq_denref(t) = WORD(1);
    return fmpq_cmp(erat_data(y), t) <= 0;
}

bool num_LessEqual_RatRat(er x, er y)
{
//std::cout << "num_CmpRatRat: " << ex_tostring_full(x) << ", " << ex_tostring_full(y) << std::endl;
    assert(eis_rat(x));
    assert(eis_rat(y));
    return fmpq_cmp(erat_data(x), erat_data(y)) <= 0;
}

bool num_LessEqual_RealReal(er X, er Y)
{
    return arb_le(ereal_data(X), ereal_data(Y));
}

bool num_LessEqual2(er X, er Y)
{
//std::cout << "num_Cmp2: " << ex_tostring(X) << ", " << ex_tostring(Y) << std::endl;
    assert(eis_number(X));
    assert(eis_number(Y));
    uint32_t tx = etype(X);
    uint32_t ty = etype(Y);
    switch (ETYPE_NUMBER * tx + ty)
    {
        case ETYPE_NUMBER * ETYPE_INT + ETYPE_INT:
            return num_LessEqual_IntInt(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_RAT:
            return num_LessEqual_IntRat(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_INT:
            return num_LessEqual_RatInt(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_RAT:
            return num_LessEqual_RatRat(X, Y);

        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_REAL:
            return num_LessEqual_RealReal(X, Y);

        default:
            assert(false);
            return false;
    }
}


ex num_AbsInt(ex X)
{
    assert(eis_int(X));

    if (fmpz_sgn(eint_data(X)) >= 0)
    {
        return X;
    }
    else
    {
        uex x(X);
        ex Z = emake_int();
        uex z(Z);
        fmpz_neg(eint_data(Z), eint_data(X));
        return efix_int(z.release());
    }
}

ex num_AbsRat(ex X)
{
    assert(eis_rat(X));

    if (fmpq_sgn(erat_data(X)) >= 0)
    {
        return X;
    }
    else
    {
        uex x(X);
        ex Z = emake_rat();
        uex z(Z);
        fmpq_neg(erat_data(Z), erat_data(X));
        return efix_rat(z.release());
    }
}



ex num_PlusIntInt(er X, er Y)
{
//std::cout << "num_PlusIntInt: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_int(Y));

    ex Z = emake_int();
    fmpz_add(eint_data(Z), eint_data(X), eint_data(Y));
    return efix_int(Z);
}

ex num_PlusIntRat(er X, er Y)
{
//std::cout << "num_PlusIntRat: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_rat(Y));

    ex Z = emake_rat();
    fmpq_add_fmpz(erat_data(Z), erat_data(Y), eint_data(X));
    return efix_rat(Z);
}

ex num_PlusRatInt(er Y, er X)
{
//std::cout << "num_PlusRatInt: " << ex_tostring_full(Y) << ", " << ex_tostring_full(X) << std::endl;
    assert(eis_int(X));
    assert(eis_rat(Y));

    ex Z = emake_rat();
    fmpq_add_fmpz(erat_data(Z), erat_data(Y), eint_data(X));
    return efix_rat(Z);
}

ex num_PlusRatRat(er X, er Y)
{
//std::cout << "num_PlusRatRat: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_rat(Y));

    ex Z = emake_rat();
    fmpq_add(erat_data(Z), erat_data(Y), erat_data(X));
    return efix_rat(Z);
}

ex num_PlusIntDouble(er X, er Y)
{
//std::cout << "num_PlusIntDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_double(Y));

	return emake_double(edouble_number(Y) + fmpz_get_d(eint_data(X)));
}

ex num_PlusRatDouble(er X, er Y)
{
//std::cout << "num_PlusRatDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_double(Y));

	return emake_double(edouble_number(Y) + num_todouble(X));
}

ex num_PlusDoubleDouble(er X, er Y)
{
//std::cout << "num_PlusDoubleDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_double(Y));

	return emake_double(edouble_number(X) + edouble_number(Y));    
}


ex num_PlusIntReal(er X, er Y)
{
//std::cout << "num_PlusIntReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_real(Y));

    slong p = arb_rel_accuracy_bits(eto_real(Y)->number.data);
    xarb_t z;
    arb_add_fmpz(z.data, eto_real(Y)->number.data, eint_data(X), p + EXTRA_PRECISION_BASIC);
    return emake_real_move(z);
}

ex num_PlusRatReal(er X, er Y)
{
//std::cout << "num_PlusRatReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_real(Y));

    slong p = arb_rel_accuracy_bits(eto_real(Y)->number.data);
    xarb_t z;
    arb_mul_fmpz(z.data, eto_real(Y)->number.data, fmpq_denref(erat_data(X)), p + EXTRA_PRECISION_BASIC);
    arb_add_fmpz(z.data, z.data, fmpq_numref(erat_data(X)), p + EXTRA_PRECISION_BASIC);
    arb_div_fmpz(z.data, z.data, fmpq_denref(erat_data(X)), p + EXTRA_PRECISION_BASIC);
    return emake_real_move(z);
}

ex num_PlusDoubleReal(er X, er Y)
{
//std::cout << "num_PlusDoubleReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_real(Y));

	return emake_double(edouble_number(X) + num_todouble(Y));    
}   


ex num_PlusRealReal(er X, er Y)
{
//std::cout << "num_PlusRealReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_real(Y));

    slong p = arb_rel_accuracy_bits(eto_real(Y)->number.data);
    p = FLINT_MAX(p, arb_rel_accuracy_bits(eto_real(X)->number.data));
    xarb_t z;
    arb_add(z.data, eto_real(X)->number.data, eto_real(Y)->number.data, p + EXTRA_PRECISION_BASIC);
    return emake_real_move(z);
}


ex num_PlusIntCmplx(er X, er Y)
{
//std::cout << "num_PlusIntComplex: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_cmplx(Y));

	ex re = num_Plus2(X, ecmplx_real(Y));
    return emake_cmplx(re, ecopy(ecmplx_imag(Y)));
}

ex num_PlusRatCmplx(er X, er Y)
{
//std::cout << "num_PlusRatCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_cmplx(Y));

	ex re = num_Plus2(X, ecmplx_real(Y));
    return emake_cmplx(re, ecopy(ecmplx_imag(Y)));
}

ex num_PlusDoubleCmplx(er X, er Y)
{
//std::cout << "num_PlusDoubleCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_cmplx(Y));

	ex re = num_Plus2(X, ecmplx_real(Y));
    return emake_cmplx(re, ecopy(ecmplx_imag(Y)));
}

ex num_PlusRealCmplx(er X, er Y)
{
//std::cout << "num_PlusRealCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_cmplx(Y));

	ex re = num_Plus2(X, ecmplx_real(Y));
    return emake_cmplx(re, ecopy(ecmplx_imag(Y)));
}

ex num_PlusCmplxCmplx(er X, er Y)
{
//std::cout << "num_PlusCmplxCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_cmplx(Y));
 
	uex re(num_Plus2(ecmplx_real(X), ecmplx_real(Y)));
	ex im = num_Plus2(ecmplx_imag(X), ecmplx_imag(Y));
    return emake_cmplx(re.release(), im);
}

ex num_PlusIntNan(er X, er Y)
{
//std::cout << "num_PlusIntNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_nan(Y));
 
    return emake_nan_Indeterminate();
}

ex num_PlusRatNan(er X, er Y)
{
//std::cout << "num_PlusRatNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_nan(Y));

    return emake_nan_Indeterminate();
}

ex num_PlusDoubleNan(er X, er Y)
{
//std::cout << "num_PlusDoubleNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_nan(Y));

	return emake_nan_Indeterminate();
}

ex num_PlusRealNan(er X, er Y)
{
//std::cout << "num_PlusRealNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_nan(Y));

	return emake_nan_Indeterminate();
}

ex num_PlusCmplxNan(er X, er Y)
{
//std::cout << "num_PlusCmplxNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_nan(Y));

    return emake_nan_Indeterminate();
}

ex num_PlusNanNan(er X, er Y)
{
//std::cout << "num_PlusNanNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_nan(X));
    assert(eis_nan(Y));

    return emake_nan_Indeterminate();
}




ex num_Plus2(er X, er Y)
{
//std::cout << "num_Plus2: " << ex_tostring(X) << ", " << ex_tostring(Y) << std::endl;
    assert(eis_number(X));
    assert(eis_number(Y));
    uint32_t tx = etype(X);
    uint32_t ty = etype(Y);
    switch (ETYPE_NUMBER * tx + ty)
    {
        case ETYPE_NUMBER * ETYPE_INT + ETYPE_INT:
            return num_PlusIntInt(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_RAT:
            return num_PlusIntRat(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_INT:
            return num_PlusIntRat(Y, X);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_RAT:
            return num_PlusRatRat(Y, X);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_DOUBLE:
            return num_PlusIntDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_INT:
            return num_PlusIntDouble(Y, X);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_DOUBLE:
            return num_PlusRatDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_RAT:
            return num_PlusRatDouble(Y, X);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_DOUBLE:
            return num_PlusDoubleDouble(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_REAL:
            return num_PlusIntReal(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_INT:
            return num_PlusIntReal(Y, X);
		case ETYPE_NUMBER * ETYPE_RAT + ETYPE_REAL:
            return num_PlusRatReal(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_RAT:
            return num_PlusRatReal(Y, X);
		case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_REAL:
            return num_PlusDoubleReal(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_DOUBLE:
            return num_PlusDoubleReal(Y, X);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_REAL:
            return num_PlusRealReal(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_CMPLX:
            return num_PlusIntCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_INT:
            return num_PlusIntCmplx(Y, X);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_CMPLX:
            return num_PlusRatCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_RAT:
            return num_PlusRatCmplx(Y, X);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_CMPLX:
            return num_PlusDoubleCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_DOUBLE:
            return num_PlusDoubleCmplx(Y, X);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_CMPLX:
            return num_PlusRealCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_REAL:
            return num_PlusRealCmplx(Y, X);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_CMPLX:
            return num_PlusCmplxCmplx(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_NAN:
            return num_PlusIntNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_INT:
            return num_PlusIntNan(Y, X);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_NAN:
            return num_PlusRatNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_RAT:
            return num_PlusRatNan(Y, X);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_NAN:
            return num_PlusDoubleNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_DOUBLE:
            return num_PlusDoubleNan(Y, X);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_NAN:
            return num_PlusRealNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_REAL:
            return num_PlusRealNan(Y, X);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_NAN:
            return num_PlusCmplxNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_CMPLX:
            return num_PlusCmplxNan(Y, X);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_NAN:
            return num_PlusNanNan(X, Y);

        default:
            assert(false);
            return nullptr;
    }
}




ex num_MinusInt(er X)
{
//std::cout << "num_MinusInt: " << ex_tostring_full(X) << std::endl;
    assert(eis_int(X));

    ex Z = emake_int();
    fmpz_neg(eint_data(Z), eint_data(X));
    return efix_int(Z);
}

ex num_MinusRat(er X)
{
//std::cout << "num_MinusRat: " << ex_tostring_full(X) << std::endl;
    assert(eis_rat(X));

    ex Z = emake_rat();
    fmpq_neg(erat_data(Z), erat_data(X));
    return efix_rat(Z);
}

ex num_MinusDouble(er X)
{
//std::cout << "num_MinusDouble: " << ex_tostring_full(X) << std::endl;
    assert(eis_double(X));

    return emake_double(-eto_double(X)->number);
}

ex num_MinusReal(er X)
{
//std::cout << "num_MinusReal: " << ex_tostring_full(X) << std::endl;
    assert(eis_real(X));

    xarb_t z;
    arb_neg(z.data, eto_real(X)->number.data);
    return emake_real_move(z);        
}

ex num_MinusCmplx(er X)
{
//std::cout << "num_MinusCmplx: " << ex_tostring_full(X) << std::endl;
    assert(eis_cmplx(X));

	uex re(num_Minus1(ecmplx_real(X)));
	ex im = num_Minus1(ecmplx_imag(X));
    return emake_cmplx(re.release(), im);
}

ex num_Minus1(er X)
{
//std::cout << "num_Minus1: " << ex_tostring_full(X) << std::endl;
    assert(eis_number(X));

    uint32_t tx = etype(X);
    switch (tx)
    {
        case ETYPE_INT:
            return num_MinusInt(X);
        case ETYPE_RAT:
            return num_MinusRat(X);
        case ETYPE_DOUBLE:
            return num_MinusDouble(X);
        case ETYPE_REAL:
            return num_MinusReal(X);
        case ETYPE_CMPLX:
            return num_MinusCmplx(X);
        default:
            assert(false);
            return nullptr;
    }
}


ex num_MinusIntInt(er X, er Y)
{
//std::cout << "num_MinusIntInt: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_int(Y));

    ex z = emake_int();
    fmpz_sub(eint_data(z), eint_data(X), eint_data(Y));
    return efix_int(z);
}

ex num_MinusRatInt(er X, er Y)
{
//std::cout << "num_MinusIntRat: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(Y));
    assert(eis_rat(X));

    ex z = emake_rat();
    fmpq_sub_fmpz(erat_data(z), erat_data(X), eint_data(Y));
    return efix_rat(z);
}

ex num_MinusIntRat(er X, er Y)
{
//std::cout << "num_MinusIntRat: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_rat(Y));

    ex z = emake_rat();
    fmpq_sub_fmpz(erat_data(z), erat_data(Y), eint_data(X));
	fmpz_neg(fmpq_numref(erat_data(z)), fmpq_numref(erat_data(z)));
    return efix_rat(z);
}


ex num_MinusRatRat(er X, er Y)
{
//std::cout << "num_MinusRatRat: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_rat(Y));

    ex z = emake_rat();
    fmpq_sub(erat_data(z), erat_data(Y), erat_data(X));
    return efix_rat(z);
}

ex num_MinusIntDouble(er X, er Y)
{
//std::cout << "num_MinusIntDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_double(Y));

	return emake_double(fmpz_get_d(eint_data(X)) - edouble_number(Y));    
}

ex num_MinusDoubleInt(er X, er Y)
{
//std::cout << "num_MinusIntDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(Y));
    assert(eis_double(X));

	return emake_double(edouble_number(X) - fmpz_get_d(eint_data(Y)));    
}

ex num_MinusRatDouble(er X, er Y)
{
//std::cout << "num_MinusRatDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_double(Y));

	return emake_double(num_todouble(X) - edouble_number(Y));
}

ex num_MinusDoubleRat(er X, er Y)
{
//std::cout << "num_MinusRatDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(Y));
    assert(eis_double(X));

	return emake_double(edouble_number(X) - num_todouble(Y));
}

ex num_MinusDoubleDouble(er X, er Y)
{
//std::cout << "num_MinusDoubleDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_double(Y));

	return emake_double(edouble_number(X) - edouble_number(Y));    
}


ex num_MinusIntReal(er X, er Y)
{
//std::cout << "num_MinusIntFlt: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_real(Y));

    ex z = emake_real();
    slong p = arb_rel_accuracy_bits(ereal_data(X));
    arb_sub_fmpz(ereal_data(z), ereal_data(Y), eint_data(X), p + EXTRA_PRECISION_BASIC);
    arb_neg(ereal_data(z), ereal_data(z));
    return efix_real(z);
}

ex num_MinusRealInt(er X, er Y)
{
//std::cout << "num_MinusIntFlt: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_int(Y));

    ex z = emake_real();
    slong p = arb_rel_accuracy_bits(ereal_data(X));
    arb_sub_fmpz(ereal_data(z), ereal_data(X), eint_data(Y), p + EXTRA_PRECISION_BASIC);
    return efix_real(z);
}

ex num_MinusRatReal(er X, er Y)
{
//std::cout << "num_MinusRatReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_real(Y));

    ex z = emake_real();
    slong p = arb_rel_accuracy_bits(ereal_data(Y));
    arb_mul_fmpz(ereal_data(z), eto_real(Y)->number.data, fmpq_denref(erat_data(X)), p + EXTRA_PRECISION_BASIC);
    arb_sub_fmpz(gs.tmpreal[0].data, ereal_data(z), fmpq_numref(erat_data(X)), p + EXTRA_PRECISION_BASIC);
    arb_div_fmpz(ereal_data(z), gs.tmpreal[0].data, fmpq_denref(erat_data(X)), p + EXTRA_PRECISION_BASIC);
	arb_neg(ereal_data(z), ereal_data(z));
    return efix_real(z);
}

ex num_MinusRealRat(er X, er Y)
{
//std::cout << "num_MinusRatReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_rat(Y));

    ex z = emake_real();
    slong p = arb_rel_accuracy_bits(eto_real(X)->number.data);
    arb_mul_fmpz(ereal_data(z), eto_real(X)->number.data, fmpq_denref(erat_data(Y)), p + EXTRA_PRECISION_BASIC);
    arb_sub_fmpz(gs.tmpreal[0].data, ereal_data(z), fmpq_numref(erat_data(Y)), p + EXTRA_PRECISION_BASIC);
    arb_div_fmpz(ereal_data(z), gs.tmpreal[0].data, fmpq_denref(erat_data(Y)), p + EXTRA_PRECISION_BASIC);
    return efix_real(z);
}

ex num_MinusDoubleReal(er X, er Y)
{
//std::cout << "num_MinusDoubleReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_real(Y));

	return emake_double(edouble_number(X) - num_todouble(Y));
}   

ex num_MinusRealDouble(er X, er Y)
{
//std::cout << "num_MinusDoubleReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(Y));
    assert(eis_real(X));

	return emake_double(num_todouble(X) - edouble_number(Y));
}

ex num_MinusRealReal(er X, er Y)
{
//std::cout << "num_MinusRealReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_real(Y));

	ex z = emake_real();
    slong p = arb_rel_accuracy_bits(ereal_data(Y));
    p = FLINT_MAX(p, arb_rel_accuracy_bits(ereal_data(X)));
    arb_sub(ereal_data(z), ereal_data(X), ereal_data(Y), p + EXTRA_PRECISION_BASIC);
    return efix_real(z);
}


ex num_MinusIntCmplx(er X, er Y)
{
//std::cout << "num_MinusIntComplex: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_cmplx(Y));

	uex re(num_Minus2(X, ecmplx_real(Y)));
	ex im = num_Minus1(ecmplx_imag(Y));
    return emake_cmplx(re.release(), im);
}

ex num_MinusCmplxInt(er X, er Y)
{
//std::cout << "num_MinusIntComplex: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_int(Y));
 
	ex re = num_Minus2(ecmplx_real(X), Y);
    return emake_cmplx(re, ecopy(ecmplx_imag(X)));
}

ex num_MinusRatCmplx(er X, er Y)
{
//std::cout << "num_MinusRatCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(Y));
    assert(eis_rat(X));
  
	uex re(num_Minus2(X, ecmplx_real(Y)));
	ex im = num_Minus1(ecmplx_imag(Y));
    return emake_cmplx(re.release(), im);

}

ex num_MinusCmplxRat(er X, er Y)
{
//std::cout << "num_MinusIntComplex: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_rat(Y));
   
	ex re = num_Minus2(ecmplx_real(X), Y);
    return emake_cmplx(re, ecopy(ecmplx_imag(X)));
}

ex num_MinusDoubleCmplx(er X, er Y)
{
//std::cout << "num_MinusDoubleCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_cmplx(Y));
   
	uex re(num_Minus2(X, ecmplx_real(Y)));
	ex im = num_Minus1(ecmplx_imag(Y));
    return emake_cmplx(re.release(), im);
}

ex num_MinusCmplxDouble(er X, er Y)
{
//std::cout << "num_MinusDoubleCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_double(Y));

	ex re = num_Minus2(ecmplx_real(X), Y);
    return emake_cmplx(re, ecopy(ecmplx_imag(X)));
}

ex num_MinusRealCmplx(er X, er Y)
{
//std::cout << "num_MinusRealCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_cmplx(Y));
   
	uex re(num_Minus2(X, ecmplx_real(Y)));
	ex im = num_Minus1(ecmplx_imag(Y));
    return emake_cmplx(re.release(), im);
}

ex num_MinusCmplxReal(er X, er Y)
{
//std::cout << "num_MinusCmplxReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_real(Y));

	ex re = num_Minus2(ecmplx_real(X), Y);
    return emake_cmplx(re, ecopy(ecmplx_imag(X)));
}

ex num_MinusCmplxCmplx(er X, er Y)
{
//std::cout << "num_MinusCmplxCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_cmplx(Y));

	uex re(num_Minus2(ecmplx_real(X), ecmplx_real(Y)));
	ex im = num_Minus2(ecmplx_imag(X), ecmplx_imag(Y));
    return emake_cmplx(re.release(), im);
}

ex num_MinusIntNan(er X, er Y)
{
//std::cout << "num_MinusIntNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_nan(Y));

    return emake_nan_Indeterminate();
}

ex num_MinusNanInt(er X, er Y)
{
//std::cout << "num_MinusIntNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_nan(X));
    assert(eis_int(Y));

    return emake_nan_Indeterminate();
}

ex num_MinusRatNan(er X, er Y)
{
//std::cout << "num_MinusRatNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_nan(Y));

    return emake_nan_Indeterminate();
}

ex num_MinusNanRat(er X, er Y)
{
//std::cout << "num_MinusRatNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_nan(X));
    assert(eis_rat(Y));

    return emake_nan_Indeterminate();
}

ex num_MinusDoubleNan(er X, er Y)
{
//std::cout << "num_MinusDoubleNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_nan(Y));

	return emake_nan_Indeterminate();
}

ex num_MinusNanDouble(er X, er Y)
{
//std::cout << "num_MinusDoubleNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_nan(X));
    assert(eis_double(Y));

	return emake_nan_Indeterminate();
}

ex num_MinusRealNan(er X, er Y)
{
//std::cout << "num_MinusRealNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_nan(Y));

	return emake_nan_Indeterminate();
}

ex num_MinusNanReal(er X, er Y)
{
//std::cout << "num_MinusRealNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_nan(X));
    assert(eis_real(Y));

	return emake_nan_Indeterminate();
}

ex num_MinusCmplxNan(er X, er Y)
{
//std::cout << "num_MinusCmplxNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_nan(Y));

    return emake_nan_Indeterminate();
}

ex num_MinusNanCmplx(er X, er Y)
{
//std::cout << "num_MinusCmplxNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_nan(X));
    assert(eis_cmplx(Y));

    return emake_nan_Indeterminate();
}

ex num_MinusNanNan(er X, er Y)
{
//std::cout << "num_MinusNanNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_nan(X));
    assert(eis_nan(Y));

    return emake_nan_Indeterminate();
}




ex num_Minus2(er X, er Y)
{
//std::cout << "num_Minus2: " << ex_tostring(X) << ", " << ex_tostring(Y) << std::endl;
    assert(eis_number(X));
    assert(eis_number(Y));
    uint32_t tx = etype(X);
    uint32_t ty = etype(Y);
    switch (ETYPE_NUMBER * tx + ty)
    {
        case ETYPE_NUMBER * ETYPE_INT + ETYPE_INT:
            return num_MinusIntInt(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_RAT:
            return num_MinusIntRat(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_INT:
            return num_MinusRatInt(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_RAT:
            return num_MinusRatRat(Y, X);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_DOUBLE:
            return num_MinusIntDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_INT:
            return num_MinusDoubleInt(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_DOUBLE:
            return num_MinusRatDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_RAT:
            return num_MinusDoubleRat(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_DOUBLE:
            return num_MinusDoubleDouble(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_REAL:
            return num_MinusIntReal(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_INT:
            return num_MinusRealInt(X, Y);
		case ETYPE_NUMBER * ETYPE_RAT + ETYPE_REAL:
            return num_MinusRatReal(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_RAT:
            return num_MinusRealRat(X, Y);
		case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_REAL:
            return num_MinusDoubleReal(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_DOUBLE:
            return num_MinusRealDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_REAL:
            return num_MinusRealReal(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_CMPLX:
            return num_MinusIntCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_INT:
            return num_MinusCmplxInt(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_CMPLX:
            return num_MinusRatCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_RAT:
            return num_MinusCmplxRat(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_CMPLX:
            return num_MinusDoubleCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_DOUBLE:
            return num_MinusCmplxDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_CMPLX:
            return num_MinusRealCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_REAL:
            return num_MinusCmplxReal(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_CMPLX:
            return num_MinusCmplxCmplx(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_NAN:
            return num_MinusIntNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_INT:
            return num_MinusNanInt(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_NAN:
            return num_MinusRatNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_RAT:
            return num_MinusNanRat(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_NAN:
            return num_MinusDoubleNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_DOUBLE:
            return num_MinusNanDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_NAN:
            return num_MinusRealNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_REAL:
            return num_MinusNanReal(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_NAN:
            return num_MinusCmplxNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_CMPLX:
            return num_MinusNanCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_NAN:
            return num_MinusNanNan(X, Y);

        default:
            assert(false);
            return nullptr;
    }
}


ex num_TimesIntInt(er X, er Y)
{
//std::cout << "num_TimesIntInt: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_int(Y));

    ex z = emake_int();
    fmpz_mul(eint_data(z), eint_data(X), eint_data(Y));
    return efix_int(z);
}

ex num_TimesIntRat(er X, er Y)
{
//std::cout << "num_TimesIntRat: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_rat(Y));

    ex z = emake_rat();
    fmpq_mul_fmpz(erat_data(z), erat_data(Y), eint_data(X));
    return efix_rat(z);
}

ex num_TimesRatInt(ex Y, ex X)
{
//std::cout << "num_TimesRatInt: " << ex_tostring_full(Y) << ", " << ex_tostring_full(X) << std::endl;
    assert(eis_int(X));
    assert(eis_rat(Y));

    ex z = emake_rat();
    fmpq_mul_fmpz(erat_data(z), erat_data(Y), eint_data(X));
    return efix_rat(z);
}

ex num_TimesRatRat(er X, er Y)
{
//std::cout << "num_TimesRatRat: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_rat(Y));

    ex z = emake_rat();
    fmpq_mul(erat_data(z), erat_data(Y), erat_data(X));
    return efix_rat(z);
}

ex num_TimesIntDouble(er X, er Y)
{
//std::cout << "num_TimesIntDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_double(Y));

	if (eis_zero(X))
		return ecopy(X);

	return emake_double(fmpz_get_d(eint_data(X)) * edouble_number(Y));
}


/*
double fmpq_get_d(const fmpq_t x)
{
    mpq_t z;
    double d;
    flint_mpq_init_set_readonly(z, x);
    d = mpq_get_d(z);
    flint_mpq_clear_readonly(z);
    return d;
}
*/

ex num_TimesRatDouble(er X, er Y)
{
//std::cout << "num_TimesRatDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_double(Y));

	return emake_double(fmpq_get_d(erat_data(X)) * edouble_number(Y));
}

ex num_TimesDoubleDouble(er X, er Y)
{
//std::cout << "num_TimesDoubleDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_double(Y));

	return emake_double(edouble_number(X) * edouble_number(Y));
}


ex num_TimesIntReal(er X, er Y)
{
//std::cout << "num_TimesIntFlt: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_real(Y));

	if (eis_zero(X))
		return ecopy(X);

	ex z = emake_real();
    slong p = arb_rel_accuracy_bits(ereal_data(Y));
    arb_mul_fmpz(ereal_data(z), ereal_data(Y), eint_data(X), p + EXTRA_PRECISION_BASIC);
    return efix_real(z);
}

ex num_TimesRatReal(er X, er Y)
{
//std::cout << "num_TimesRatReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_real(Y));

	ex z = emake_real();
    slong p = arb_rel_accuracy_bits(ereal_data(Y));
	arb_div_fmpz(ereal_data(z), ereal_data(Y), fmpq_denref(erat_data(X)), p + EXTRA_PRECISION_BASIC);
    arb_mul_fmpz(ereal_data(z), ereal_data(z), fmpq_numref(erat_data(X)), p + EXTRA_PRECISION_BASIC);
    return efix_real(z);
}

ex num_TimesDoubleReal(er X, er Y)
{
//std::cout << "num_TimesDoubleReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_real(Y));

	return emake_double(edouble_number(X) * num_todouble(Y));    
}   


ex num_TimesRealReal(er X, er Y)
{
//std::cout << "num_TimesRealReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_real(Y));

	ex z = emake_real();
    slong p = arb_rel_accuracy_bits(ereal_data(Y));
    p = std::max(p, arb_rel_accuracy_bits(ereal_data(X)));
    arb_mul(ereal_data(z), ereal_data(X), ereal_data(Y), p + EXTRA_PRECISION_BASIC);
    return efix_real(z);
}


ex num_TimesIntCmplx(er X, er Y)
{
//std::cout << "num_TimesIntComplex: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_cmplx(Y));
   
	uex re(num_Times2(X, ecmplx_real(Y)));
	ex im = num_Times2(X, ecmplx_imag(Y));
    return emake_cmplx(re.release(), im);
}

ex num_TimesRatCmplx(er X, er Y)
{
//std::cout << "num_TimesRatCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_cmplx(Y));

	uex re(num_Times2(X, ecmplx_real(Y)));
	ex im = num_Times2(X, ecmplx_imag(Y));
    return emake_cmplx(re.release(), im);
}

ex num_TimesDoubleCmplx(er X, er Y)
{
//std::cout << "num_TimesDoubleCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_cmplx(Y));

	uex re(num_Times2(X, ecmplx_real(Y)));
	ex im = num_Times2(X, ecmplx_imag(Y));
    return emake_cmplx(re.release(), im);
}

ex num_TimesRealCmplx(er X, er Y)
{
//std::cout << "num_TimesRealCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_cmplx(Y));

	uex re(num_Times2(X, ecmplx_real(Y)));
	ex im = num_Times2(X, ecmplx_imag(Y));
    return emake_cmplx(re.release(), im);
}

ex num_TimesCmplxCmplx(er X, er Y)
{
//std::cout << "num_TimesCmplxCmplx: " << ex_tostring(X) << ", " << ex_tostring(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_cmplx(Y));

	uex ad(num_Times2(ecmplx_real(X), ecmplx_imag(Y)));
	uex bc(num_Times2(ecmplx_imag(X), ecmplx_real(Y)));
	uex ac(num_Times2(ecmplx_real(X), ecmplx_real(Y)));
	uex bd(num_Times2(ecmplx_imag(X), ecmplx_imag(Y)));
	uex re(num_Minus2(ac.get(), bd.get()));
	uex im(num_Plus2(ad.get(), bc.get()));
    return emake_cmplx(re.release(), im.release());
}

ex num_TimesIntNan(er X, er Y)
{
//std::cout << "num_TimesIntNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_nan(Y));

    return emake_nan_Indeterminate();
}

ex num_TimesRatNan(er X, er Y)
{
//std::cout << "num_TimesRatNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_nan(Y));

    return emake_nan_Indeterminate();
}

ex num_TimesDoubleNan(er X, er Y)
{
//std::cout << "num_TimesDoubleNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_nan(Y));

	return emake_nan_Indeterminate();
}

ex num_TimesRealNan(er X, er Y)
{
//std::cout << "num_TimesRealNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_nan(Y));

	return emake_nan_Indeterminate();
}

ex num_TimesCmplxNan(er X, er Y)
{
//std::cout << "num_TimesCmplxNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_nan(Y));

    return emake_nan_Indeterminate();
}

ex num_TimesNanNan(er X, er Y)
{
//std::cout << "num_TimesNanNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_nan(X));
    assert(eis_nan(Y));

    return emake_nan_Indeterminate();
}




ex num_Times2(er X, er Y)
{
//std::cout << "num_Times2: " << ex_tostring(X) << ", " << ex_tostring(Y) << std::endl;
    assert(eis_number(X));
    assert(eis_number(Y));
    uint32_t tx = etype(X);
    uint32_t ty = etype(Y);
    switch (ETYPE_NUMBER * tx + ty)
    {
        case ETYPE_NUMBER * ETYPE_INT + ETYPE_INT:
            return num_TimesIntInt(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_RAT:
            return num_TimesIntRat(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_INT:
            return num_TimesIntRat(Y, X);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_RAT:
            return num_TimesRatRat(Y, X);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_DOUBLE:
            return num_TimesIntDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_INT:
            return num_TimesIntDouble(Y, X);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_DOUBLE:
            return num_TimesRatDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_RAT:
            return num_TimesRatDouble(Y, X);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_DOUBLE:
            return num_TimesDoubleDouble(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_REAL:
            return num_TimesIntReal(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_INT:
            return num_TimesIntReal(Y, X);
		case ETYPE_NUMBER * ETYPE_RAT + ETYPE_REAL:
            return num_TimesRatReal(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_RAT:
            return num_TimesRatReal(Y, X);
		case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_REAL:
            return num_TimesDoubleReal(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_DOUBLE:
            return num_TimesDoubleReal(Y, X);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_REAL:
            return num_TimesRealReal(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_CMPLX:
            return num_TimesIntCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_INT:
            return num_TimesIntCmplx(Y, X);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_CMPLX:
            return num_TimesRatCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_RAT:
            return num_TimesRatCmplx(Y, X);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_CMPLX:
            return num_TimesDoubleCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_DOUBLE:
            return num_TimesDoubleCmplx(Y, X);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_CMPLX:
            return num_TimesRealCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_REAL:
            return num_TimesRealCmplx(Y, X);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_CMPLX:
            return num_TimesCmplxCmplx(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_NAN:
            return num_TimesIntNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_INT:
            return num_TimesIntNan(Y, X);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_NAN:
            return num_TimesRatNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_RAT:
            return num_TimesRatNan(Y, X);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_NAN:
            return num_TimesDoubleNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_DOUBLE:
            return num_TimesDoubleNan(Y, X);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_NAN:
            return num_TimesRealNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_REAL:
            return num_TimesRealNan(Y, X);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_NAN:
            return num_TimesCmplxNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_CMPLX:
            return num_TimesCmplxNan(Y, X);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_NAN:
            return num_TimesNanNan(X, Y);

        default:
            assert(false);
            return nullptr;
    }
}





ex num_DivideInt(er X)
{
//std::cout << "num_DivideInt: " << ex_tostring_full(X) << std::endl;
    assert(eis_int(X));

    if (fmpz_is_pm1(eint_data(X)))
    {
        return ecopy(X);
    }
    else
    {
        if (fmpz_is_zero(eint_data(X)))
        {
            return gs.const_complexinfinity.copy();
        }
        else
        {
			ex z = emake_rat();
            fmpz_one(fmpq_numref(erat_data(z)));
            fmpz_set(fmpq_denref(erat_data(z)), eint_data(X));
            return efix_rat(z);
        }
    }
}

ex num_DivideRat(er X)
{
//std::cout << "num_DivideRat: " << ex_tostring_full(X) << std::endl;
    assert(eis_rat(X));

    xfmpq_t z;
    if (fmpz_sgn(fmpq_denref(erat_data(X))) < 0)
    {
        fmpz_neg(fmpq_denref(z.data), fmpq_numref(erat_data(X)));
        fmpz_neg(fmpq_numref(z.data), fmpq_denref(erat_data(X)));
    }
    else
    {
        fmpz_set(fmpq_denref(z.data), fmpq_numref(erat_data(X)));
        fmpz_set(fmpq_numref(z.data), fmpq_denref(erat_data(X)));
    }
    return emake_rat_move(z);      
}


ex num_DivideReal(er X)
{
//std::cout << "num_DivideReal: " << ex_tostring_full(X) << std::endl;
    assert(eis_real(X));

	ex z = emake_real();
    slong p = ereal_number(X).wprec();
    arb_ui_div(ereal_data(z), 1, ereal_data(X), p + EXTRA_PRECISION_BASIC);
    return efix_real(z);
}

ex num_DivideCmplx(er Y)
{
//std::cout << "num_DivideCmplx: " << ex_tostring(Y) << std::endl;
    assert(eis_cmplx(Y));

	uex yre2(num_Times2(ecmplx_real(Y), ecmplx_real(Y)));
	uex yim2(num_Times2(ecmplx_imag(Y), ecmplx_imag(Y)));
	uex yabs2(num_Plus2(yre2.get(), yim2.get()));
	uex re(num_Divide2(ecmplx_real(Y), yabs2.get()));
	uex im(num_Divide2(ecmplx_imag(Y), yabs2.get()));
	ex t = num_Minus1(im.get());
    return emake_cmplx(re.release(), t);
}

ex num_Divide1(er X)
{
//std::cout << "num_Divide: " << ex_tostring(X) << std::endl;
    assert(eis_number(X));

    uint32_t tx = etype(X);
    switch (tx)
    {
        case ETYPE_INT:
            return num_DivideInt(X);
        case ETYPE_RAT:
            return num_DivideRat(X);
        case ETYPE_REAL:
            return num_DivideReal(X);
        case ETYPE_CMPLX:
            return num_DivideCmplx(X);
        default:
            assert(false);
            return nullptr;
    }
}



ex num_DivideIntInt(er X, er Y)
{
//std::cout << "num_DivideIntInt: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_int(Y));

    if (fmpz_is_zero(eint_data(Y)))
        return fmpz_is_zero(eint_data(X)) ? gs.const_indeterminate.copy()
										  : gs.const_complexinfinity.copy();

    if (fmpz_is_one(eint_data(Y)) || fmpz_is_zero(eint_data(X)))
        return ecopy(X);

	ex z = emake_rat();

	fmpz_t u;
    fmpz_init(u);
    fmpz_gcd(u, eint_data(X), eint_data(Y));
    if (!fmpz_is_one(u))
    {
        fmpz_divexact(fmpq_numref(erat_data(z)), eint_data(X), u);
        fmpz_divexact(fmpq_denref(erat_data(z)), eint_data(Y), u);
    }
	else
	{
        fmpz_set(fmpq_numref(erat_data(z)), eint_data(X));
        fmpz_set(fmpq_denref(erat_data(z)), eint_data(Y));
	}
    fmpz_clear(u);

    if (fmpz_sgn(fmpq_denref(erat_data(z))) < 0)
    {
        fmpz_neg(fmpq_numref(erat_data(z)), fmpq_numref(erat_data(z)));
        fmpz_neg(fmpq_denref(erat_data(z)), fmpq_denref(erat_data(z)));
    }

    return efix_rat(z);
}

ex num_DivideIntRat(er X, er Y)
{
//std::cout << "num_DivideIntRat: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_rat(Y));

    xfmpq_t z;
    if (fmpz_sgn(fmpq_denref(erat_data(Y))) < 0)
    {
        fmpz_neg(fmpq_denref(z.data), fmpq_numref(erat_data(Y)));
        fmpz_neg(fmpq_numref(z.data), fmpq_denref(erat_data(Y)));
    }
    else
    {
        fmpz_set(fmpq_denref(z.data), fmpq_numref(erat_data(Y)));
        fmpz_set(fmpq_numref(z.data), fmpq_denref(erat_data(Y)));
    }
    fmpq_mul_fmpz(z.data, z.data, eint_data(X));
    return emake_rat_move(z);
}

ex num_DivideRatInt(er X, er Y)
{
//std::cout << "num_DivideRatInt: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_int(Y));

    if (fmpz_is_zero(eint_data(Y)))
        return gs.const_complexinfinity.copy();

    ex z = emake_rat();
    fmpq_div_fmpz(erat_data(z), erat_data(X), eint_data(Y));
    return efix_rat(z);
}

ex num_DivideRatRat(er X, er Y)
{
//std::cout << "num_DivideRatRat: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_rat(Y));

    ex z = emake_rat();
    fmpq_div(erat_data(z), erat_data(X), erat_data(Y));
    return efix_rat(z);
}

ex num_DivideIntDouble(er X, er Y)
{
//std::cout << "num_DivideIntDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_double(Y));

	return emake_double(fmpz_get_d(eint_data(X))/edouble_number(Y));    
}

ex num_DivideDoubleInt(er X, er Y)
{
//std::cout << "num_DivideDoubleInt: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(Y));
    assert(eis_double(X));

	return emake_double(edouble_number(X)/fmpz_get_d(eint_data(Y)));    
}

ex num_DivideRatDouble(er X, er Y)
{
//std::cout << "num_DivideRatDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_double(Y));

	return emake_double(num_todouble(X)/edouble_number(Y));
}

ex num_DivideDoubleRat(er X, er Y)
{
//std::cout << "num_DivideDoubleRat: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_rat(Y));

	return emake_double(edouble_number(X)/fmpq_get_d(erat_data(Y)));
}

ex num_DivideDoubleDouble(er X, er Y)
{
//std::cout << "num_DivideDoubleDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_double(Y));

	return emake_double(edouble_number(X)/edouble_number(Y));    
}

ex num_DivideIntReal(er X, er Y)
{
//std::cout << "num_MinusIntFlt: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_real(Y));

	ex z = emake_real();
    slong p = ereal_number(Y).wprec();
	arb_set_round_fmpz(gs.tmpreal[0].data, eint_data(X), p + EXTRA_PRECISION_BASIC);
    arb_div(ereal_data(z), gs.tmpreal[0].data, ereal_data(Y), p + EXTRA_PRECISION_BASIC);
    return efix_real(z);
}

ex num_DivideRealInt(er X, er Y)
{
//std::cout << "num_MinusIntFlt: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_int(Y));

	ex z = emake_real();
    slong p = ereal_number(X).wprec();
    arb_div_fmpz(ereal_data(z), ereal_data(X), eint_data(Y), p + EXTRA_PRECISION_BASIC);
    return efix_real(z);
}

ex num_DivideRatReal(er X, er Y)
{
//std::cout << "num_MinusRatReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_real(Y));

	ex z = emake_real();
    slong p = ereal_number(Y).wprec();
	arb_mul_fmpz(gs.tmpreal[0].data, ereal_data(Y), fmpq_denref(erat_data(X)), p + EXTRA_PRECISION_BASIC);
	arb_set_round_fmpz(gs.tmpreal[1].data, fmpq_numref(erat_data(X)), p + EXTRA_PRECISION_BASIC);
    arb_div(ereal_data(z), gs.tmpreal[1].data, gs.tmpreal[0].data, p + EXTRA_PRECISION_BASIC);
    return efix_real(z);
}

ex num_DivideRealRat(er X, er Y)
{
//std::cout << "num_MinusRatReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_rat(Y));

	ex z = emake_real();
    slong p = ereal_number(X).wprec();
    arb_mul_fmpz(gs.tmpreal[0].data, ereal_data(X), fmpq_denref(erat_data(Y)), p + EXTRA_PRECISION_BASIC);
    arb_div_fmpz(ereal_data(z), gs.tmpreal[0].data, fmpq_numref(erat_data(Y)), p + EXTRA_PRECISION_BASIC);
    return efix_real(z);
}

ex num_DivideDoubleReal(er X, er Y)
{
//std::cout << "num_MinusDoubleReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_real(Y));

	return emake_double(edouble_number(X)/num_todouble(Y));    
}   

ex num_DivideRealDouble(er X, er Y)
{
//std::cout << "num_DivideRealDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(Y));
    assert(eis_real(X));

	return emake_double(num_todouble(X)/edouble_number(Y));    
}

ex num_DivideRealReal(er X, er Y)
{
//std::cout << "num_DivideRealReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_real(Y));

    slong p = ereal_number(Y).wprec();
    p = FLINT_MAX(p, ereal_number(X).wprec());
    xarb_t z;
    arb_div(z.data, ereal_data(X), ereal_data(Y), p + EXTRA_PRECISION_BASIC);
    return emake_real_move(z);
}


ex num_DivideIntCmplx(er X, er Y)
{
//std::cout << "num_DivideIntCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_cmplx(Y));

	uex yre2(num_Times2(ecmplx_real(Y), ecmplx_real(Y)));
	uex yim2(num_Times2(ecmplx_imag(Y), ecmplx_imag(Y)));
	uex yabs2(num_Plus2(yre2.get(), yim2.get()));
	uex s(num_Divide2(X, yabs2.get()));
	uex re(num_Times2(s.get(), ecmplx_real(Y)));
	uex im(num_Times2(s.get(), ecmplx_imag(Y)));
	ex t = num_Minus1(im.get());
    return emake_cmplx(re.release(), t);
}

ex num_DivideCmplxInt(er X, er Y)
{
//std::cout << "num_DivideCmplxInt: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_int(Y));

	uex re(num_Divide2(ecmplx_real(X), Y));
	uex im(num_Divide2(ecmplx_imag(X), Y));
    return emake_cmplx(re.release(), im.release());
}

ex num_DivideRatCmplx(er X, er Y)
{
//std::cout << "num_MinusRatCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_cmplx(Y));

	uex yre2(num_Times2(ecmplx_real(Y), ecmplx_real(Y)));
	uex yim2(num_Times2(ecmplx_imag(Y), ecmplx_imag(Y)));
	uex yabs2(num_Plus2(yre2.get(), yim2.get()));
	uex s(num_Divide2(X, yabs2.get()));
	uex re(num_Times2(s.get(), ecmplx_real(Y)));
	uex im(num_Times2(s.get(), ecmplx_imag(Y)));
	ex t = num_Minus1(im.get());
    return emake_cmplx(re.release(), t);
}

ex num_DivideCmplxRat(er X, er Y)
{
//std::cout << "num_MinusIntComplex: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_rat(Y));

	uex re(num_Divide2(ecmplx_real(X), Y));
	ex im = num_Divide2(ecmplx_imag(X), Y);
    return emake_cmplx(re.release(), im);
}

ex num_DivideDoubleCmplx(er X, er Y)
{
//std::cout << "num_MinusDoubleCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_cmplx(Y));

	uex yre2(num_Times2(ecmplx_real(Y), ecmplx_real(Y)));
	uex yim2(num_Times2(ecmplx_imag(Y), ecmplx_imag(Y)));
	uex yabs2(num_Plus2(yre2.get(), yim2.get()));
	uex s(num_Divide2(X, yabs2.get()));
	uex re(num_Times2(s.get(), ecmplx_real(Y)));
	uex im(num_Times2(s.get(), ecmplx_imag(Y)));
	ex t = num_Minus1(im.get());
    return emake_cmplx(re.release(), t);
}

ex num_DivideCmplxDouble(er X, er Y)
{
//std::cout << "num_MinusDoubleCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_double(Y));

	uex re(num_Divide2(ecmplx_real(X), Y));
	ex im = num_Divide2(ecmplx_imag(X), Y);
    return emake_cmplx(re.release(), im);
}

ex num_DivideRealCmplx(er X, er Y)
{
//std::cout << "num_MinusRealCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_cmplx(Y));

	uex re(num_Minus2(X, ecmplx_real(Y)));
	ex im = num_Minus1(ecmplx_imag(Y));
    return emake_cmplx(re.release(), im);
}

ex num_DivideCmplxReal(er X, er Y)
{
//std::cout << "num_MinusCmplxReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_real(Y));

	uex re(num_Divide2(ecmplx_real(X), Y));
	ex im = num_Divide2(ecmplx_imag(X), Y);
    return emake_cmplx(re.release(), im);
}

ex num_DivideCmplxCmplx(er X, er Y)
{
//std::cout << "num_MinusCmplxCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_cmplx(Y));

	uex z(num_Divide1(Y));
	return num_Times2(X, z.get());
}

ex num_DivideIntNan(er X, er Y)
{
//std::cout << "num_DivideIntNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_nan(Y));

    return emake_nan_Indeterminate();
}

ex num_DivideNanInt(er X, er Y)
{
//std::cout << "num_DivideNanInt: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_nan(X));
    assert(eis_int(Y));

    return emake_nan_Indeterminate();
}

ex num_DivideRatNan(er X, er Y)
{
//std::cout << "num_DivideRatNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_nan(Y));

    return emake_nan_Indeterminate();
}

ex num_DivideNanRat(er X, er Y)
{
//std::cout << "num_DivideNanRat: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_nan(X));
    assert(eis_rat(Y));

    return emake_nan_Indeterminate();
}

ex num_DivideDoubleNan(er X, er Y)
{
//std::cout << "num_DivideDoubleNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_double(X));
    assert(eis_nan(Y));

	return emake_nan_Indeterminate();
}

ex num_DivideNanDouble(er X, er Y)
{
//std::cout << "num_DivideNanDouble: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_nan(X));
    assert(eis_double(Y));

	return emake_nan_Indeterminate();
}

ex num_DivideRealNan(er X, er Y)
{
//std::cout << "num_DivideRealNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_nan(Y));

	return emake_nan_Indeterminate();
}

ex num_DivideNanReal(er X, er Y)
{
//std::cout << "num_DivideNanReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_nan(X));
    assert(eis_real(Y));

	return emake_nan_Indeterminate();
}

ex num_DivideCmplxNan(er X, er Y)
{
//std::cout << "num_DivideCmplxNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_cmplx(X));
    assert(eis_nan(Y));

    return emake_nan_Indeterminate();
}

ex num_DivideNanCmplx(er X, er Y)
{
//std::cout << "num_DivideNanCmplx: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_nan(X));
    assert(eis_cmplx(Y));

    return emake_nan_Indeterminate();
}

ex num_DivideNanNan(er X, er Y)
{
//std::cout << "num_DivideNanNan: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_nan(X));
    assert(eis_nan(Y));

    return emake_nan_Indeterminate();
}

ex num_Divide2(er X, er Y)
{
//std::cout << "num_Minus2: " << ex_tostring(X) << ", " << ex_tostring(Y) << std::endl;
    assert(eis_number(X));
    assert(eis_number(Y));
    uint32_t tx = etype(X);
    uint32_t ty = etype(Y);
    switch (ETYPE_NUMBER * tx + ty)
    {
        case ETYPE_NUMBER * ETYPE_INT + ETYPE_INT:
            return num_DivideIntInt(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_RAT:
            return num_DivideIntRat(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_INT:
            return num_DivideRatInt(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_RAT:
            return num_DivideRatRat(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_DOUBLE:
            return num_DivideIntDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_INT:
            return num_DivideDoubleInt(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_DOUBLE:
            return num_DivideRatDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_RAT:
            return num_DivideDoubleRat(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_DOUBLE:
            return num_DivideDoubleDouble(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_REAL:
            return num_DivideIntReal(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_INT:
            return num_DivideRealInt(X, Y);
		case ETYPE_NUMBER * ETYPE_RAT + ETYPE_REAL:
            return num_DivideRatReal(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_RAT:
            return num_DivideRealRat(X, Y);
		case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_REAL:
            return num_DivideDoubleReal(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_DOUBLE:
            return num_DivideRealDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_REAL:
            return num_DivideRealReal(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_CMPLX:
            return num_DivideIntCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_INT:
            return num_DivideCmplxInt(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_CMPLX:
            return num_DivideRatCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_RAT:
            return num_DivideCmplxRat(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_CMPLX:
            return num_DivideDoubleCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_DOUBLE:
            return num_DivideCmplxDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_CMPLX:
            return num_DivideRealCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_REAL:
            return num_DivideCmplxReal(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_CMPLX:
            return num_DivideCmplxCmplx(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_NAN:
            return num_DivideIntNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_INT:
            return num_DivideNanInt(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_NAN:
            return num_DivideRatNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_RAT:
            return num_DivideNanRat(X, Y);
        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_NAN:
            return num_DivideDoubleNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_DOUBLE:
            return num_DivideNanDouble(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_NAN:
            return num_DivideRealNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_REAL:
            return num_DivideNanReal(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_NAN:
            return num_DivideCmplxNan(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_CMPLX:
            return num_DivideNanCmplx(X, Y);
        case ETYPE_NUMBER * ETYPE_NAN + ETYPE_NAN:
            return num_DivideNanNan(X, Y);

        default:
            assert(false);
            return nullptr;
    }
}





ex num_PowerIntInt(er X, er Y)
{
//std::cout << "num_PowerIntInt: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_int(Y));

    if (fmpz_is_zero(eint_data(X)))
    {
        int s = fmpz_sgn(eint_data(Y));
        if (s > 0)
            return ecopy(X);
        else if (s < 0)
            return gs.const_complexinfinity.copy();
        else
            return gs.const_indeterminate.copy();
    }
    else if (fmpz_is_pm1(eint_data(X)))
    {
        if (fmpz_is_one(eint_data(X)) || fmpz_is_even(eint_data(Y)))
            return emake_cint(1);
        else
            return emake_cint(-1);
    }
    else if (fmpz_fits_si(eint_data(Y)))
    {
        slong n = fmpz_get_si(eint_data(Y));
        if (n > 0)
        {
            ex z = emake_int();
            fmpz_pow_ui(eint_data(z), eint_data(X), n);
            return efix_int(z);
        }
        else if (n < 0)
        {
            ex Z = emake_rat();
            fmpz_pow_ui(fmpq_denref(erat_data(Z)), eint_data(X), -n);
            if (fmpz_sgn(fmpq_denref(erat_data(Z))) < 0)
            {
                fmpz_set_si(fmpq_numref(erat_data(Z)), -1);
                fmpz_neg(fmpq_denref(erat_data(Z)), fmpq_denref(erat_data(Z)));
            }
            else
            {
                fmpz_set_ui(fmpq_numref(erat_data(Z)), 1);
            }
            return efix_rat(Z);
        }
        else
        {
            return emake_cint(1);
        }
    }
    else
    {
        return fmpz_sgn(eint_data(Y)) > 0 ? gs.const_overflow.copy() : gs.const_underflow.copy();
    }
}

ex num_PowerIntRat(er X, er Y)
{
    return emake_node(gs.sym_sPower.copy(), ecopy(X), ecopy(Y));
}

ex num_PowerRatInt(er X, er Y)
{
//std::cout << "num_PowerRatInt: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_rat(X));
    assert(eis_int(Y));

    if (fmpz_fits_si(eint_data(Y)))
    {
        slong n = fmpz_get_si(eint_data(Y));
        ex z = emake_rat();
        fmpq_pow_si(erat_data(z), erat_data(X), n);
        return efix_rat(z);
    }
    else
    {
        return emake_nan_Overflow();
    }
}

ex num_PowerRatRat(er X, er Y)
{
    assert(false);
    return nullptr;
}

ex num_PowerDoubleInt(er X, er Y)
{
    assert(eis_double(X));
    assert(eis_int(Y));

    double rr = 1;
    double xx = edouble_number(X);

    ulong bits = fmpz_bits(eint_data(Y));

    for (ulong i = 0; i < bits; i++)
    {
        if (fmpz_tstbit(eint_data(Y), i))
            rr *= xx;
        xx *= xx;
    }

    if (fmpz_sgn(eint_data(Y)) < 0)
        rr = 1/rr;

    return emake_double(rr);
}

ex num_PowerRealInt(er X, er Y)
{
//std::cout << "num_TimesRatReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_int(Y));

	ex z = emake_real();
    slong p = ereal_number(X).wprec();
    arb_pow_fmpz(ereal_data(z), ereal_data(X), eint_data(Y), p + EXTRA_PRECISION_BASIC);
    return efix_real(z);
}

ex num_PowerRealReal(er X, er Y)
{
//std::cout << "num_TimesRatReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_real(Y));

    ex z = emake_real();
    slong p = ereal_number(X).wprec();
    arb_pow(ereal_data(z), ereal_data(X), ereal_data(Y), p + EXTRA_PRECISION_BASIC);
    return efix_real(z);
}

ex num_PowerIntReal(er X, er Y)
{
//std::cout << "num_PowerIntReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_int(X));
    assert(eis_real(Y));

    slong p = ereal_number(Y).wprec();
    ex z = emake_real();
	arb_set_round_fmpz(ereal_data(z), eint_data(X), p + EXTRA_PRECISION_BASIC);
    arb_pow(ereal_data(z), ereal_data(z), ereal_data(Y), p + EXTRA_PRECISION_BASIC);
    return efix_real(z);
}

ex num_PowerRealRat(er X, er Y)
{
//std::cout << "num_TimesRatReal: " << ex_tostring_full(X) << ", " << ex_tostring_full(Y) << std::endl;
    assert(eis_real(X));
    assert(eis_rat(Y));

    slong p = ereal_number(X).wprec();
    xarb_t z;
    arb_pow_fmpq(z.data, ereal_data(X), erat_data(Y), p + EXTRA_PRECISION_BASIC);
    return emake_real_move(z);
}

ex num_PowerRatFlt(er X, er Y)
{
    assert(false);
    return nullptr;
}
ex num_PowerFltRat(er X, er Y)
{
    assert(false);
    return nullptr;
}

ex num_PowerFltFlt(er X, er Y)
{
    assert(false);
    return nullptr;
}

ex num_PowerIntCplx(er X, er Y)
{
    assert(false);
    return nullptr;
}
ex num_PowerCplxInt(er X, er Y)
{
    assert(false);
    return nullptr;
}

ex num_PowerRatCplx(er X, er Y)
{
    assert(false);
    return nullptr;
}
ex num_PowerCplxRat(er X, er Y)
{
    assert(false);
    return nullptr;
}

ex num_PowerFltCplx(er X, er Y)
{
    assert(false);
    return nullptr;
}
ex num_PowerCplxFlt(er X, er Y)
{
    assert(false);
    return nullptr;
}

ex num_PowerCplxCplx(er X, er Y)
{
    assert(false);
    return nullptr;
}






ex num_Power2(er X, er Y)
{
//std::cout << "num_Power2: " << ex_tostring(X) << ", " << ex_tostring(Y) << std::endl;

    assert(eis_number(X));
    assert(eis_number(Y));
    uint32_t tx = etype(X);
    uint32_t ty = etype(Y);
    switch (ETYPE_NUMBER * tx + ty)
    {
        case ETYPE_NUMBER * ETYPE_INT + ETYPE_INT:
            return num_PowerIntInt(X, Y);

        case ETYPE_NUMBER * ETYPE_INT + ETYPE_RAT:
            return num_PowerIntRat(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_INT:
            return num_PowerRatInt(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_RAT:
            return num_PowerRatRat(X, Y);

        case ETYPE_NUMBER * ETYPE_DOUBLE + ETYPE_INT:
            return num_PowerDoubleInt(X, Y);

        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_INT:
            return num_PowerRealInt(X, Y);
        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_RAT:
            return num_PowerRealRat(X, Y);

        case ETYPE_NUMBER * ETYPE_REAL + ETYPE_REAL:
            return num_PowerRealReal(X, Y);
        case ETYPE_NUMBER * ETYPE_INT + ETYPE_REAL:
            return num_PowerIntReal(X, Y);


        case ETYPE_NUMBER * ETYPE_INT + ETYPE_CMPLX:
            return num_PowerIntCplx(X, Y);
        case ETYPE_NUMBER * ETYPE_RAT + ETYPE_CMPLX:
            return num_PowerRatCplx(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_INT:
            return num_PowerCplxInt(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_RAT:
            return num_PowerCplxRat(X, Y);
        case ETYPE_NUMBER * ETYPE_CMPLX + ETYPE_CMPLX:
            return num_PowerCplxCplx(X, Y);

        default:
            assert(false);
            return nullptr;
    }
}


double num_todouble(er x)
{
//std::cout << "num_todouble: " << ex_tostring_full(x) << std::endl;
    assert(eis_number(x));

    uint32_t tx = etype(x);
    switch (tx)
    {
        case ETYPE_INT:
            return fmpz_get_d(eint_data(x));
        case ETYPE_RAT:
            return fmpq_get_d(erat_data(x));
        case ETYPE_DOUBLE:
            return eto_double(x)->number;
        case ETYPE_REAL:
        {
            double d = arf_get_d(arb_midref(ereal_data(x)), ARF_RND_NEAR);
            switch(std::fpclassify(d))
            {
                case FP_NORMAL:
                case FP_ZERO:
                default:
                    return d;
                case FP_INFINITE:
                case FP_NAN:
                case FP_SUBNORMAL:
                    return NAN;
            }
        }
        default:
            return NAN;
    }
}

double econvert_todouble(er e)
{
    if (eis_number(e))
    {
        return num_todouble(e);
    }
    else
    {
        return NAN;
    }
}

