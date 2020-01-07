#include <cmath>
#include <cfloat>

#include "timing.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"
#include "flint/arith.h"


ex dcode_sSeedRandom(er e)
{
//std::cout << "dcode_sSeedRandom: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sSeedRandom.get()));

    if (elength(e) > 1)
    {
        return _handle_message_argt(e, 0 + (1 << 8));
    }

    if (elength(e) == 0)
    {
        gs.randgen.seed(nullptr, 0);
    }
    else
    {
        er X = echild(e, 1);
        if (eis_int(X))
        {
            if (!COEFF_IS_MPZ(*eint_data(X)))
            {
                ulong x[1];
                x[0] = std::abs(*eint_data(X));
                gs.randgen.seed(reinterpret_cast<const unsigned char *>(x), x[0] == 0 ? 0 : sizeof(ulong));
            }
            else
            {
                __mpz_struct * x = COEFF_TO_PTR(*eint_data(X));
                size_t len = std::abs(x->_mp_size);
                gs.randgen.seed(reinterpret_cast<const unsigned char *>(x->_mp_d), len*sizeof(ulong));
            }
        }
        else if (eis_str(X))
        {
            gs.randgen.seed(reinterpret_cast<const unsigned char *>(estr_string(X).c_str()), estr_string(X).size());
        }
        else
        {
            _gen_message(echild(e,0), "seed", "Argument `1` in `2` should be an integer or a string.", ecopy(X), ecopy(e));
            return ecopy(e);            
        }
    }

    return gs.sym_sNull.copy();
}

ex dcode_sRandomInteger(er e)
{
//std::cout << "dcode_sRandomInteger: " << ex_tostring_full(e.get()) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sRandomInteger.get()));

    if (elength(e) > 2)
    {
        return _handle_message_argb(e, 0 + (2 << 8));
    }

    if (elength(e) == 0)
    {
        return emake_cint(gs.randgen.getbits(1));
    }
    else if (elength(e) == 1)
    {
        er lower, upper, e1 = echild(e,1);
        if (eis_int(e1))
        {
            lower = eget_cint(0);
            upper = e1;
        }
        else if (ehas_head_sym_length(e1, gs.sym_sList.get(), 1) && eis_int(echild(e1,1)))
        {
            lower = eget_cint(0);
            upper = echild(e1,1);
        }
        else if (ehas_head_sym_length(e1, gs.sym_sList.get(), 2) && eis_int(echild(e1,1)) && eis_int(echild(e1,2)))
        {
            lower = echild(e1,1);
            upper = echild(e1,2);
        }
        else
        {
            _gen_message(echild(e,0), "range", "Range specification `1` is invalid.", ecopy(e1));
            return ecopy(e);
        }

        xfmpz diff;
        fmpz_sub(diff.data, eint_data(upper), eint_data(lower));
        if (fmpz_sgn(diff.data) < 0)
        {
            std::swap(lower, upper);
            fmpz_neg(diff.data, diff.data);
        }

        ex X = emake_int();
        uex x(X);
        do
        {
            gs.randgen.getbits(eint_number(X), fmpz_bits(diff.data));
        }
        while (fmpz_cmpabs(eint_data(X), diff.data) > 0);

        fmpz_add(eint_data(X), eint_data(X), eint_data(lower));

        return efix_int(x.release());
    }
    else
    {
        return ecopy(e);
    }
}

ex dcode_sRandomReal(er e)
{
//std::cout << "dcode_sRandomReal: " << ex_tostring_full(e.get()) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sRandomReal.get()));

    if (elength(e) == 0)
    {
        uint64_t n;
        int exp = 0;
        do
        {
            n = gs.randgen.getbits(52);
            exp -= 52;
            if (exp < -100)
            {
                return emake_double(0);
            }
        }
        while (false); // commented code gives more bits
/*
        while (n == 0);

        u32 msb = MSB(n);
        assert(msb <= 51);
        if (0 < 51 - msb)
        {
            exp -= (51 - msb);
            n = (n << (51 - msb)) + gs.randgen.getbits(51 - msb);
        }
*/
        return emake_double(ldexp(double(n), exp));
    }
    else
    {
        return ecopy(e);
    }
}

ex dcode_sRandomChoice(er e)
{
//std::cout << "dcode_sRandomChoice: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sRandomChoice.get()));

    if (elength(e) == 1)
    {
        er X = echild(e,1);
        if (ehas_head_sym(X, gs.sym_sList.get()))
        {
            size_t n = elength(X);
            if (n > 0)
            {
                unsigned int count = MSB(n) + 1;
                size_t tryx;
                do
                {
                    tryx = gs.randgen.getbits(count);
                }
                while (tryx >= n);
                return ecopychild(X, tryx + 1);
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
    else
    {
        return ecopy(e);
    }
}

ex dcode_sRandomSelection(er e)
{
//std::cout << "dcode_sRandomSelection: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sRandomSelection.get()));

    if (elength(e) == 0)
    {
        return ecopy(e);
    }
    else
    {
        return ecopy(e);
    }
}
