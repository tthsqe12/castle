#include "ex_print.h"
#include "code.h"
#include "assign.h"

ex dcode_sDownValues(er e)
{
//std::cout << "dcode_sDownValues: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sDownValues.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    er e1 = echild(e,1);
    if (!eis_sym(e1))
    {
		return _handle_message_sym(e, 1);
    }

	return esudvalue_print(esym_dvalue(e1));
}

ex dcode_sOwnValues(er e)
{
//std::cout << "dcode_sOwnValues: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sOwnValues.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    er e1 = echild(e,1);
    if (!eis_sym(e1))
    {
		return _handle_message_sym(e, 1);
    }

    er v = esym_ovalue(e1);
    if (v != nullptr)
    {
        ex t = emake_node(gs.sym_sHoldPattern.copy(), ecopy(e1));
        t = emake_node(gs.sym_sRuleDelayed.copy(), t, ecopy(v));
        return emake_node(gs.sym_sList.copy(), t);
    }
    else
    {
        return emake_node(gs.sym_sList.copy());
    }
}

ex dcode_sSubValues(er e)
{
//std::cout << "dcode_sSubValues: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sSubValues.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    er e1 = echild(e,1);
    if (!eis_sym(e1))
    {
		return _handle_message_sym(e, 1);
    }

	return esudvalue_print(esym_svalue(e1));
}

ex dcode_sUpValues(er e)
{
//std::cout << "dcode_sUpValues: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sUpValues.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    er e1 = echild(e,1);
    if (!eis_sym(e1))
    {
		return _handle_message_sym(e, 1);
    }

	return esudvalue_print(esym_uvalue(e1));
}
