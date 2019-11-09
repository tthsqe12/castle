#include "globalstate.h"
#include "eval.h"
#include "code.h"

ex dcode_sNest(er e)
{
//std::cout << "dcode_sNest: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sNest.get()));

	if (elength(e) != 3)
		return _handle_message_argx(e, 3);

	er e3 = echild(e,3);
	slong n;
    if (eis_int(e3) && fmpz_fits_si(eint_data(e3))
					&& (n = fmpz_get_si(eint_data(e3))) >= 0)
    {
        uex r(ecopychild(e,2));
        while (--n >= 0)
        {
            r.reset(eval(emake_node(ecopychild(e,1), r.release())));
        }
        return r.release();
    }
    else
    {
	    _gen_message(echild(e,0), "intnm", nullptr, ecopy(e), emake_cint(3));
        return ecopy(e);
    }
}


ex dcode_sNestList(er e)
{
//std::cout << "dcode_sNestList: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sNestList.get()));

	if (elength(e) != 3)
		return _handle_message_argx(e, 3);

	er e3 = echild(e,3);
	slong n;
    if (eis_int(e3) && fmpz_fits_si(eint_data(e3))
					&& (n = fmpz_get_si(eint_data(e3))) >= 0)
    {
        uex r(gs.sym_sList.get(), n + 1);
		r.push_back(ecopychild(e,2));
        for (slong i = 1; i <= n; i++)
        {
            r.push_back(eval(emake_node(ecopychild(e,1), r.copychild(i))));
        }
        return r.release();
    }
    else
    {
	    _gen_message(echild(e,0), "intnm", nullptr, ecopy(e), emake_cint(3));
        return ecopy(e);
    }
}


ex dcode_sFold(er e)
{
//std::cout << "dcode_sFold: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sFold.get()));
	ex badidx;

	if (elength(e) == 3)
	{
		er l = echild(e,3);
		if (eis_node(l))
		{
	        uex r(ecopychild(e,2));
	        for (size_t i = 1; i <= elength(l); i++)
	        {
	            r.reset(eval(emake_node(ecopychild(e,1), r.release(), ecopychild(l,i))));
	        }
	        return r.release();			
		}

		badidx = emake_cint(3);
	}
	else if (elength(e) == 2)
	{
		er l = echild(e,2);
		if (eis_node(l))
		{
			if (elength(l) > 0)
			{
		        uex r(ecopychild(l,1));
		        for (size_t i = 2; i <= elength(l); i++)
		        {
		            r.reset(eval(emake_node(ecopychild(e,1), r.release(), ecopychild(l,i))));
		        }
		        return r.release();
			}
			else
			{
				return ecopy(e);
			}
		}

		badidx = emake_cint(2);
	}
	else
	{
		return _handle_message_argt(e, 2 + (3 << 8));
	}

    _gen_message(echild(e,0), "normal", nullptr, badidx, ecopy(e));
	return ecopy(e);
}

ex dcode_sFoldList(er e)
{
//std::cout << "dcode_sFoldList: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sFoldList.get()));

	ex badidx;

	if (elength(e) == 3)
	{
		er l = echild(e,3);
		if (eis_node(l))
		{
	        uex r(echild(l,0), elength(l) + 1);
			r.push_back(ecopychild(e,2));
	        for (size_t i = 1; i <= elength(l); i++)
	        {
	            r.push_back(eval(emake_node(ecopychild(e,1), r.copychild(i), ecopychild(l,i))));
	        }
	        return r.release();			
		}

		badidx = emake_cint(3);
	}
	else if (elength(e) == 2)
	{
		er l = echild(e,2);
		if (eis_node(l))
		{
	        uex r(echild(l,0), elength(l));
			if (elength(l) > 0)
			{
				r.push_back(ecopychild(l,1));
		        for (size_t i = 1; i < elength(l); i++)
		        {
		            r.push_back(eval(emake_node(ecopychild(e,1), r.copychild(i), ecopychild(l,i+1))));
		        }
			}
	        return r.release();
		}

		badidx = emake_cint(2);
	}
	else
	{
		return _handle_message_argt(e, 2 + (3 << 8));
	}

    _gen_message(echild(e,0), "normal", nullptr, badidx, ecopy(e));
	return ecopy(e);
}