#include "globalstate.h"
#include "code.h"
#include "assign.h"
#include "eval.h"

ex dcode_sSet(er e)
{
//std::cout << "dcode_sSet: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sSet.get()));

    if (elength(e) == 2)
    {
        eassign(echild(e,1), echild(e,2), echild(e,0));
        return ecopychild(e,2);
    }
    else
    {
        if (elength(e) == 0)
        {
            return _handle_message_argm(e, 1);
        }

        size_t n = 2;
        uex r(gs.sym_sSequence.get(), elength(e) - n + 1);
        for (size_t i = n; i <= elength(e); i++)
        {
            r.push_back(ecopy(e));
        }
        eassign(echild(e,1), r.get(), echild(e,0));
        return r.release();        
    }
}

ex dcode_sSetDelayed(er e)
{
//std::cout << "dcode_sSetDelayed: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sSetDelayed.get()));

    if (elength(e) == 2)
    {
        bool suc = eassign(echild(e,1), echild(e,2), echild(e,0));
        return ecopy(suc ? gs.sym_sNull.get() : gs.sym_s$Failed.get());
    }
    else
    {
        if (elength(e) == 0)
        {
            return _handle_message_argm(e, 1);
        }

        size_t n = 2;
        uex r(gs.sym_sSequence.get(), elength(e) - n + 1);
        for (size_t i = n; i <= elength(e); i++)
        {
            r.push_back(ecopy(e));
        }
        bool suc = eassign(echild(e,1), r.get(), echild(e,0));
        return ecopy(suc ? gs.sym_sNull.get() : gs.sym_s$Failed.get());
    }
}

ex dcode_sTagSetDelayed(er e)
{
//std::cout << "dcode_sSetDelayed: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sTagSetDelayed.get()));

    if (elength(e) == 3)
    {
        bool suc = etagassign(echild(e,1), echild(e,2), echild(e,3), echild(e,0));
        return ecopy(suc ? gs.sym_sNull.get() : gs.sym_s$Failed.get());
    }
    else
    {
        if (elength(e) < 2)
        {
            return _handle_message_argm(e, 1);
        }

        size_t n = 3;
        uex r(gs.sym_sSequence.get(), elength(e) - n + 1);
        for (size_t i = n; i <= elength(e); i++)
        {
            r.push_back(ecopy(e));
        }
        bool suc = etagassign(echild(e,1), echild(e,1), r.get(), echild(e,0));
        return ecopy(suc ? gs.sym_sNull.get() : gs.sym_s$Failed.get());
    }
}

ex dcode_sTagSet(er e)
{
//std::cout << "dcode_sSet: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sTagSet.get()));

    if (elength(e) < 2)
    {
        return _handle_message_argm(e, 2);
    }
	if (!eis_sym(echild(e,1)))
	{
        return _handle_message_sym(e, 1);
	}

    if (elength(e) == 3)
    {
		uex r(eval(ecopychild(e,3)));
        etagassign(echild(e,1), echild(e,2), r.get(), echild(e,0));
        return r.release();
    }
    else
    {

        size_t n = 3;
        uex r(gs.sym_sSequence.get(), elength(e) - n + 1);
        for (size_t i = n; i <= elength(e); i++)
        {
            r.push_back(eval(ecopy(e)));
        }
        etagassign(echild(e,1), echild(e,2), r.get(), echild(e,0));
        return r.release();
    }
}

ex dcode_sClearAll(er e)
{
//std::cout << "dcode_sClearAll: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sClearAll.get()));

    // "a=3; Clear[a,b+c]" generates a message and clears a
    for (size_t i = 1; i <= elength(e); i++)
    {
        er ei = echild(e,i);
        if (eis_sym(ei))
        {
            if (esym_ovalue(ei) != nullptr)
            {
                eclear(etox(esym_ovalue(ei)));
                esym_ovalue(ei) = nullptr;
            }
            if (esym_svalue(ei) != nullptr)
            {
                delete esym_svalue(ei);
                esym_svalue(ei) = nullptr;
            }
            if (esym_uvalue(ei) != nullptr)
            {
                delete esym_uvalue(ei);
                esym_uvalue(ei) = nullptr;
            }
            if (esym_dvalue(ei) != nullptr)
            {
                delete esym_dvalue(ei);
                esym_dvalue(ei) = nullptr;
            }
        }
        else
        {
            // TODO string
            _gen_message(echild(e,0), "ssym", "`1` is not a symbol or a string.", ecopy(ei));
        }
    }

    return gs.sym_sNull.copy();
}
