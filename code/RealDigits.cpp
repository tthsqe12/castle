#include "globalstate.h"
#include "code.h"

ex dcode_sRealDigits(er e)
{
//std::cout << "dcode_sRealDigits: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sRealDigits.get()));

    if (elength(e) == 1)
    {
        er x = echild(e,1);
        if (eis_int(x))
        {
            uex r(gs.sym_sList.copy(), 1 + fmpz_sizeinbase(eint_data(x), 10));
            char * s = fmpz_get_str(NULL, 10, eint_data(x));
            char * t = s;
            if (*t == '-')
                t++;
            while (*t)
                r.push_back(emake_cint(*t++ - '0'));
            flint_free(s); // ok - r.push_back cannot throw
            ex t1 = emake_int_ui(elength(r.get()));
            return emake_node(gs.sym_sList.copy(), r.release(), t1);
        }
        else if (eis_rat(x))
        {
            xfmpz a(fmpq_numref(erat_data(x)));
            xfmpz b(fmpq_denref(erat_data(x)));
            xfmpz u, v, t, s, exp;
            xfmpz radix(UWORD(10));

            fmpz_abs(a.data, a.data);
            fmpz_set_si(exp.data, fmpz_remove(a.data, a.data, radix.data)
                                - fmpz_remove(b.data, b.data, radix.data));

            fmpz_gcd(t.data, b.data, radix.data);
            while (!fmpz_is_one(t.data))
            {
                fmpz_divexact(b.data, b.data, t.data);
                fmpz_divexact(u.data, radix.data, t.data);
                fmpz_mul(a.data, a.data, u.data);
                fmpz_sub_ui(exp.data, exp.data, 1);
                fmpz_gcd(t.data, b.data, radix.data);
            }

            fmpz_fdiv_qr(u.data, a.data, a.data, b.data);

            uex answer(gs.sym_sList.copy(), 2 + fmpz_sizeinbase(u.data, 10));
            if (!fmpz_is_zero(u.data))
            {
                char * S = fmpz_get_str(NULL, 10, u.data);
                char * T = S;
                while (*T)
                    answer.push_back(emake_cint(*T++ - '0'));
                flint_free(S);
                fmpz_add_ui(exp.data, exp.data, T - S);
            }

            if (!fmpz_is_zero(a.data))
            {
                std::vector<wex> reppart;

                if (fmpz_cmp_ui(radix.data, INT_CACHE_MAX) <= 0
                    && fmpz_abs_fits_ui(b.data))
                {
                    ulong R = fmpz_get_ui(radix.data);
                    ulong B = fmpz_get_ui(b.data);
                    ulong A = fmpz_get_ui(a.data);
                    ulong V = A;
                    do {
                        ulong U, Hi, Lo;
                        umul_ppmm(Hi, Lo, A, R);
                        udiv_qrnnd(U, A, Hi, Lo, B);
                        reppart.push_back(emake_cint(slong(U)));
                    } while (A != V);
                }
                else
                {
                    fmpz_set(v.data, a.data);
                    do {
                        fmpz_mul(a.data, a.data, radix.data);
                        fmpz_fdiv_qr(u.data, a.data, a.data, b.data);
                        reppart.push_back(emake_int_move(u));
                    } while (!fmpz_equal(a.data, v.data));
                }
                answer.push_back(emake_node(gs.sym_sList.copy(), reppart));
            }
            return emake_node(gs.sym_sList.copy(), answer.release(), emake_int_move(exp));
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
