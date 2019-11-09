#include "globalstate.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"

ex dcode_sTuples(er e)
{
//std::cout << "dcode_sTuples: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sTuples.get()));

    if (elength(e) == 2)
    {
        er f = echild(e,1);
        er p = echild(e,2);
        if (!eis_node(f))
        {
            _gen_message(gs.sym_sTuples.get(), "normal", NULL, emake_cint(1), ecopy(e));
            return ecopy(e);    
        }
        size_t k = elength(f);

        if (eis_int(p))
        {
            if (fmpz_sgn(eint_data(p)) < 0 || !fmpz_fits_si(eint_data(p)))
            {
                _gen_message(gs.sym_sTuples.get(), "ilsmn", NULL, emake_cint(2), ecopy(e));
                return ecopy(e);
            }
            size_t n = fmpz_get_si(eint_data(p));
            if (n == 0)
            {
                return emake_node(gs.sym_sList.copy(), emake_node(gs.sym_sList.copy()));
            }
            if (k == 0)
            {
                return emake_node(gs.sym_sList.copy());
            }
            xfmpz total_size(UWORD(1));
            for (size_t i = 0; i < n; i++)
            {
                fmpz_mul_ui(total_size.data, total_size.data, k);
                if (!fmpz_fits_si(total_size.data))
                {
                    _gen_message(gs.sym_sTuples.get(), "toomany", NULL, ecopy(e));
                    return ecopy(e);
                }
            }

            uex v(gs.sym_sList.get(), fmpz_get_si(total_size.data));

            std::vector<size_t> bounds;
            bounds.resize(n, 0);

            while (true)
            {
                uex l(echild(f,0), n);

                for (size_t i = 0; i < n; i++)
                {
                    assert(bounds[i] < k);
                    l.push_back(ecopychild(f, bounds[i] + 1));
                }
                v.push_back(l.release());

                size_t j = n - 1;
            bound_inc2:
                ++bounds[j];
                if (bounds[j] >= k)
                {
                    bounds[j] = 0;
                    if (j == 0)
                        break;
                    j--;
                    goto bound_inc2;
                }
            }

            return v.release();
        }
        else if (ehas_head_sym(p, gs.sym_sList.get()))
        {
            size_t r = elength(p);
            if (r == 0)
            {
                return ecopy(f);
            }
            xfmpz array_size(UWORD(1));
            std::vector<size_t> array_dims; 
            for (size_t i = 1; i <= r; i++)
            {
                er pi = echild(p,i);
                if (!eis_int(pi) || fmpz_sgn(eint_data(pi)) < 0 || !fmpz_fits_si(eint_data(pi)))
                {
                    _gen_message(gs.sym_sTuples.get(), "ilsmn", NULL, emake_cint(2), ecopy(e));
                    return ecopy(e);
                }
                array_dims.push_back(fmpz_get_si(eint_data(pi)));
                fmpz_mul_ui(array_size.data, array_size.data, array_dims.back());
            }
            if (!fmpz_fits_si(array_size.data))
            {
                _gen_message(gs.sym_sTuples.get(), "bigarray", NULL, ecopy(p));
                return ecopy(e);
            }
            size_t n = fmpz_get_si(array_size.data);
            if (n == 0)
            {
                size_t m;
                for (m = 0; m < r; m++)
                {
                    if (array_dims[m] == 0)
                        break;
                }
                uex ans(emake_node(ecopychild(f,0)));
                while (m != 0)
                {
                    m--;
                    assert(array_dims[m] != 0);
                    uex nans(echild(f,0), array_dims[m]);
                    for (size_t i = 0; i < array_dims[m]; i++)
                    {
                        nans.push_back(ans.copy());
                    }
                    ans.reset(nans.release());
                }
                return emake_node(ecopy(gs.sym_sList.get()), ans.release());
            }
            if (k == 0)
            {
                return emake_node(gs.sym_sList.copy());
            }
            xfmpz total_size(UWORD(1));
            for (size_t i = 0; i < n; i++)
            {
                fmpz_mul_ui(total_size.data, total_size.data, k);
                if (!fmpz_fits_si(total_size.data))
                {
                    _gen_message(gs.sym_sTuples.get(), "toomany", NULL, ecopy(e));
                    return ecopy(e);
                }
            }

            uex v(gs.sym_sList.get(), fmpz_get_si(total_size.data));

            std::vector<size_t> bounds;
            bounds.resize(n, 0);

            while (true)
            {
                std::vector<uex> l;
                for (size_t j = 0; j < r; j++)
                {
                    l.push_back(uex(echild(f,0), array_dims[j]));
                }
                for (size_t i = 0; true; i++)
                {
                    assert(i < n);
                    assert(bounds[i] < k);
                    uex g(ecopychild(f, bounds[i] + 1));

                    size_t j = r - 1;
                bound_inc4:
                    l[j].push_back(g.release());
                    if (elength(l[j].get()) >= array_dims[j])
                    {
                        assert(elength(l[j].get()) == array_dims[j]);
                        g.reset(l[j].release());
                        l[j] = uex(echild(f,0), array_dims[j]);
                        if (j == 0)
                        {
                            v.push_back(g.release());
                            break;
                        }
                        j--;
                        goto bound_inc4;
                    }
                }

                size_t j = n - 1;
            bound_inc3:
                ++bounds[j];
                if (bounds[j] >= k)
                {
                    bounds[j] = 0;
                    if (j == 0)
                        break;
                    j--;
                    goto bound_inc3;
                }
            }
            assert(fmpz_cmp_ui(total_size.data, elength(v.get())) == 0);

            return v.release();
        }
        else
        {
            _gen_message(gs.sym_sTuples.get(), "ilsmn", NULL, emake_cint(2), ecopy(e));
            return ecopy(e);
        }
    }
    else if (elength(e) == 1)
    {
        er f = echild(e,1);
        if (!eis_node(f))
        {
            _gen_message(gs.sym_sTuples.get(), "normal", NULL, emake_cint(1), ecopy(e));
            return ecopy(e);
        }
        size_t n = elength(f);
        std::vector<size_t> lengths;
        lengths.resize(n, 0);
        bool have_zero_length = false;
        xfmpz total_size(UWORD(1));

        for (size_t i = 1; i <= n; i++)
        {
            if (!eis_node(echild(f,i)))
            {
                ex t = emake_node(ecopy(gs.sym_sList.get()), emake_cint(1), emake_int_ui(i));
                _gen_message(gs.sym_sTuples.get(), "normal", NULL, t, ecopy(e));
                return ecopy(e);
            }
            lengths[i - 1] = elength(echild(f,i));
            have_zero_length |= (lengths[i - 1] == 0);
            fmpz_mul_ui(total_size.data, total_size.data, lengths[i - 1]);
        }
        if (n == 0 || have_zero_length)
        {
            return emake_node(gs.sym_sList.copy());
        }
        if (!fmpz_fits_si(total_size.data))
        {
            _gen_message(gs.sym_sTuples.get(), "toomany", NULL, ecopy(e));
            return ecopy(e);
        }

        uex v(gs.sym_sList.get(), fmpz_get_si(total_size.data));

        std::vector<size_t> bounds;
        bounds.resize(n, 0);

        while (true)
        {
            uex l(echild(f,0), n);

            for (size_t i = 0; i < n; i++)
            {
                assert(bounds[i] < lengths[i]);
                l.push_back(ecopychild(echild(f, i + 1), bounds[i] + 1));
            }
            v.push_back(l.release());

            size_t j = n - 1;
        bound_inc:
            ++bounds[j];
            if (bounds[j] >= lengths[j])
            {
                bounds[j] = 0;
                if (j == 0)
                    break;
                j--;
                goto bound_inc;
            }
        }

        return v.release();
    }
    else
    {
        ex t = emake_int_ui(elength(e));
        _gen_message(gs.sym_sTuples.get(), "argt", NULL, gs.sym_sTuples.copy(), t, emake_cint(1), emake_cint(2));
        return ecopy(e);
    }
}
