#include "globalstate.h"
#include "code.h"
#include "ex_cont.h"

ex dcode_sRealDigits(er e)
{
//std::cout << "dcode_sRealDigits: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sRealDigits.get()));

    if (elength(e) == 1)
    {
        er x = echild(e,1);
        if (eis_int(x))
        {
			ulong bound = 1 + fmpz_sizeinbase(eint_data(x), 10);
			uex r(emake_parray_rank1(0, bound));
            char * s = fmpz_get_str(NULL, 10, eint_data(x));
            char * t = s;
			fmpz * d = eparray_int_data(r.get());
            if (*t == '-')
                t++;
			ulong n = 0;
			do {
				assert(!COEFF_IS_MPZ(d[n]));
				assert(n < bound);
				d[n] = *t - '0';
				n++;
				t++;
			} while (*t);
			eto_parray(r.get())->dimensions.set_index(0, n);
            flint_free(s);
			ex tn = emake_int_ui(n);
			return emake_node(gs.sym_sList.copy(), r.release(), tn);
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
				x_fmpz_vector reppart;
                if (fmpz_cmp_ui(radix.data, COEFF_MAX + 1) <= 0 && fmpz_abs_fits_ui(b.data))
                {
					assert(fmpz_abs_fits_ui(a.data));
                    ulong R = fmpz_get_ui(radix.data);
                    ulong B = fmpz_get_ui(b.data);
                    ulong A = fmpz_get_ui(a.data);
                    ulong V = A;
					slong vlength = reppart.data->length;
					slong valloc = reppart.data->alloc;
					fmpz * varray = reppart.data->array;
					assert(vlength >= 0);
                    do {
                        ulong U, Hi, Lo;
                        umul_ppmm(Hi, Lo, A, R);
                        udiv_qrnnd(U, A, Hi, Lo, B);
					    if (unlikely(vlength + 1 > valloc))
						{
						    _fmpz_vector_fit_length(reppart.data, vlength + 1);
							valloc = reppart.data->alloc;
							varray = reppart.data->array;
						}
						assert(!COEFF_IS_MPZ(varray[vlength]));
						assert(U <= COEFF_MAX);
					    varray[vlength] = U;
					    vlength = vlength + 1;
                    } while (A != V);
					reppart.data->length = vlength;
                }
                else
                {
                    fmpz_set(v.data, a.data);
                    do {
                        fmpz_mul(a.data, a.data, radix.data);
					    _fmpz_vector_fit_length(reppart.data, reppart.data->length + 1);
                        fmpz_fdiv_qr(reppart.data->array + reppart.data->length, a.data, a.data, b.data);
					    reppart.data->length++;
                    } while (!fmpz_equal(a.data, v.data));
                }
                answer.push_back(emake_parray_fmpz_vector(reppart.data));
            }
			ex texp = emake_int_move(exp);
            return emake_node(gs.sym_sList.copy(), answer.release(), texp);
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
