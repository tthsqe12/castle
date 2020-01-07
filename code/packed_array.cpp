#include "globalstate.h"
#include "code.h"
#include "ex_cont.h"

ex dcode_devPackedArrayQ(er e)
{
//std::cout << "dcode_sHashCode: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_devPackedArrayQ.get()));

    slong n = elength(e);
    if (n < 1 || n > 3)
        return _handle_message_argb(e, (1 << 0) + (3 << 8));

    if (!eis_parray(echild(e,1)))
        return gs.sym_sFalse.copy();

    if (n < 2)
        return gs.sym_sTrue.copy();

    int etyp = eparray_type(echild(e,1));

    if (eis_sym(echild(e,2), gs.sym_sInteger.get()))
    {
        if (etyp != 0)
            return gs.sym_sFalse.copy();
    }
    else if (eis_sym(echild(e,2), gs.sym_sRational.get()))
    {
        if (etyp != 1)
            return gs.sym_sFalse.copy();
    }
    else if (eis_sym(echild(e,2), gs.sym_sReal.get()))
    {
        if (etyp != 2)
            return gs.sym_sFalse.copy();
    }
    else if (eis_sym(echild(e,2), gs.sym_sComplex.get()))
    {
        if (etyp != 3)
            return gs.sym_sFalse.copy();
    }
    else
    {
        ex t = emake_node(gs.sym_sList.copy(),
                                gs.sym_sInteger.copy(),
                                gs.sym_sRational.copy(),
                                gs.sym_sReal.copy(),
                                gs.sym_sComplex.copy());
        _gen_message(echild(e,0), "mbrpos", NULL, ecopy(e), emake_cint(2), t);
        return ecopy(e);
    }

    if (n == 2)
        return gs.sym_sTrue.copy();

    if (!eis_intpm(echild(e,3)))
		_handle_message_intpm(e, 3);

    return emake_boole(eparray_rank(echild(e,1)) == eintm_get(echild(e,3)));
}

ex dcode_devToPackedArray(er e)
{
    if (elength(e) == 1)
    {
        return econvert_parray_deep(echild(e,1));
    }
    else if (elength(e) == 2)
    {
        er e1 = echild(e,1);
        er e2 = echild(e,2);

        if (eis_sym(e2, gs.sym_sInteger.get()))
        {
            return econvert_parray_deep_type(e1, 0);
        }
        else if (eis_sym(e2, gs.sym_sRational.get()))
        {
            return econvert_parray_deep_type(e1, 1);
        }
        else if (eis_sym(e2, gs.sym_sReal.get()))
        {
            return econvert_parray_deep_type(e1, 2);
        }
        else if (eis_sym(e2, gs.sym_sComplex.get()))
        {
            return econvert_parray_deep_type(e1, 3);
        }
        else
        {
            return ecopy(e);
        }
    }
    else
    {
        return _handle_message_argt(e, (1 << 0) + (2 << 8));
    }
}

static ex _unpack(er e, ulong depth, ulong maxdepth)
{
	if (depth > maxdepth)
		return ecopy(e);

	uex r(ecopy(e));
	if (eis_parray(e))
		r.setnz(ecopy(eparray_get_normal(e)));

	if (eis_node(r.get()))
	{
		ulong n = elength(r.get());
		uex f(_unpack(r.child(0), depth + 1, maxdepth), n);
		for (ulong i = 1; i <= n; i++)
			f.push_back(_unpack(r.child(i), depth + 1, maxdepth));
		r.setnz(f.release());
	}

	return r.release();
}

ex dcode_devFromPackedArray(er e)
{
    if (elength(e) == 1)
    {
		return _unpack(echild(e,1), 1, -UWORD(1));
    }
    else if (elength(e) == 2)
    {
	    if (!eis_intpm(echild(e,2)))
			return _handle_message_intpm(e, 2);

		return _unpack(echild(e,1), 1, eintm_get(echild(e,2)));
    }
    else
    {
        return _handle_message_argt(e, (1 << 0) + (2 << 8));
    }
    
    return ecopy(e);
}