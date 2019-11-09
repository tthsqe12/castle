#include "globalstate.h"
#include "code.h"

int _parse_intrange(slong & start, slong & stop, slong & step, er r)
{
    if (eis_int(r))
    {
        if (fmpz_sgn(eint_data(r)) < 0 || !fmpz_fits_si(eint_data(r)))
            return 1;
        start = 0;
        stop = fmpz_get_si(eint_data(r));
        step = 1;
        return 0;
    }
    else if (ehas_head_sym_length(r, gs.sym_sList.get(), 1))
    {
        if (!eis_int(echild(r,1)) || fmpz_sgn(eint_data(echild(r,1))) < 0    || !fmpz_fits_si(eint_data(echild(r,1))))
            return 1;

        start = fmpz_get_si(eint_data(echild(r,1)));
        stop = start;
        step = 1;
        return 0;
    }
    else if (ehas_head_sym_length(r, gs.sym_sList.get(), 2))
    {
        if (fmpz_sgn(eint_data(echild(r,1))) < 0 || !fmpz_fits_si(eint_data(echild(r,1))))
            return 1;
        if (fmpz_sgn(eint_data(echild(r,2))) < 0 || !fmpz_fits_si(eint_data(echild(r,2))))
            return 1;

        start = fmpz_get_si(eint_data(echild(r,1)));
        stop = fmpz_get_si(eint_data(echild(r,2)));
        step = 1;
        return 0;
    }
    else if (ehas_head_sym_length(r, gs.sym_sList.get(), 3))
    {
        if (fmpz_sgn(eint_data(echild(r,1))) < 0 || !fmpz_fits_si(eint_data(echild(r,1))))
            return 1;
        if (fmpz_sgn(eint_data(echild(r,2))) < 0 || !fmpz_fits_si(eint_data(echild(r,2))))
            return 1;
        if (fmpz_sgn(eint_data(echild(r,3))) <= 0 || !fmpz_fits_si(eint_data(echild(r,3))))
            return 1;

        start = fmpz_get_si(eint_data(echild(r,1)));
        stop = fmpz_get_si(eint_data(echild(r,2)));
        step = fmpz_get_si(eint_data(echild(r,3)));
        return 0;
    }
    else
    {
        return 1;
    }
}


void _subsets_of_size(uex & v, er f, ulong n)
{
    ulong k = elength(f);
    assert(n <= k);

    std::vector<ulong> indices(n);
    for (ulong i = 0; i < n; i++)
        indices[i] = i;

    while (true)
    {
        uex s(echild(f,0), n);
        for (ulong i = 0; i < n; i++)
        {
            assert(indices[i] < k);
            s.push_back(ecopychild(f, 1 + indices[i]));
        }

        v.push_back(s.release());
        ulong j = n;
subsetinc:
        if (j == 0)
            break;
        j--;
        size_t a = ++indices[j];
        if (a > k - n + j)
            goto subsetinc;
        for (j++; j < n; j++)
        {
            indices[j] = ++a;
            assert(indices[j] < k);
        }
    }
}


ex _subsets(ex E, slong start, slong stop, slong step)
{
    uex e(E);

    assert(step > 0);
    assert(start >= 0);
    assert(step >= 0);
    er f = echild(E,1);
    ulong k = elength(f);

    start = std::min(start, slong(k));
    stop = std::min(stop, slong(k));

    xfmpz total_size(UWORD(0));
    xfmpz bin;
    for (slong i = start; start <= i && i <= stop; i += step)
    {
        fmpz_bin_uiui(bin.data, k, i);
        fmpz_add(total_size.data, total_size.data, bin.data);
    }

    if (!fmpz_fits_si(total_size.data))
    {
        _gen_message(gs.sym_sSubsets.get(), "toomany", NULL, ecopy(e.get()));
    }

    uex v(gs.sym_sList.get(), fmpz_get_si(total_size.data));
    for (slong i = start; start <= i && i <= stop; i += step)
    {
        _subsets_of_size(v, f, i);
    }

    return v.release();
}

ex dcode_sSubsets(er e)
{
//std::cout << "dcode_sSubsets: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sSubsets.get()));

    if (elength(e) == 1)
    {
        er f = echild(e,1);
        if (!eis_node(f))
        {
            _gen_message(gs.sym_sSubsets.get(), "normal", NULL, emake_cint(1), ecopy(e));
            return ecopy(e);    
        }
        return _subsets(ecopy(e), 0, elength(f), 1);
    }
    else if (elength(e) == 2)
    {
        if (!eis_node(echild(e,1)))
        {
            _gen_message(gs.sym_sSubsets.get(), "normal", NULL, emake_cint(1), ecopy(e));
            return ecopy(e);    
        }
        slong start, stop, step;
        if (_parse_intrange(start, stop, step, echild(e,2)))
        {
            _gen_message(gs.sym_sSubsets.get(), "nninfseq", "Position 2 of `1` must be All, Infinity, a non-negative integer, or a List whose first element (required) is a non-negative integer, second element (optional) is a non-negative integer or Infinity, and third element (optional) is a non-negative integer.", ecopy(e));
            return ecopy(e);    
        }
        return _subsets(ecopy(e), start, stop, step);
    }
    else
    {
        ex t = emake_int_ui(elength(e));
        _gen_message(gs.sym_sSubsets.get(), "argt", NULL, gs.sym_sSubsets.copy(), t, emake_cint(1), emake_cint(2));
        return ecopy(e);    
    }
}
