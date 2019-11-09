#include "types.h"
#include "ex.h"
#include "uex.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"


ex dcode_sAttributes(er e)
{
//std::cout << "dcode_sAttributes: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sAttributes.get()));

    if (elength(e) == 1 && eis_sym(echild(e,1)))
    {
        e = echild(e,1);
        uint32_t nat = esym_normalattr(e);
        uint32_t xat = esym_extraattr(e);
        std::vector<uex> l;

#define check(name) if (nat & ATTR_##name){l.push_back(uex(gs.sym_s##name.copy()));}
#define xcheck(name) if (xat & XATTR_##name){l.push_back(uex(gs.sym_s##name.copy()));}

        check(Constant)
        check(Flat)
        if (xat & ATTR_HoldAll)
        {
            er f = ((xat & ATTR_HoldAll) == ATTR_HoldFirst) ? gs.sym_sHoldFirst.get()
                  : ((xat & ATTR_HoldAll) == ATTR_HoldRest) ? gs.sym_sHoldRest.get()
                  : gs.sym_sHoldAll.get();
            l.push_back(uex(ecopy(f)));
        }
        check(HoldAllComplete)
        check(Listable)
//        check(Locked)
        if (xat & ATTR_NHoldAll)
        {
            er f = ((xat & ATTR_NHoldAll) == ATTR_NHoldFirst) ? gs.sym_sNHoldFirst.get()
                  : ((xat & ATTR_NHoldAll) == ATTR_NHoldRest) ? gs.sym_sNHoldRest.get()
                  : gs.sym_sHoldAll.get();
            l.push_back(uex(ecopy(f)));
        }
        check(NumericFunction)
        check(OneIdentity)
        check(Orderless)
        check(Protected)
        check(ReadProtected)
        check(SequenceHold)
//        check(Stub)
        xcheck(Temporary)

#undef check

        return emake_node(gs.sym_sList.copy(), l);
    }
    else
    {
        return ecopy(e);
    }
}


static int32_t parse_attr(er e)
{
#define check(name) if (e == gs.sym_s##name.get()){return ATTR_##name;}
    check(Constant)
    check(Flat)
    check(HoldAll)
    check(HoldAllComplete)
    check(HoldFirst)
    check(HoldRest)
    check(Listable)
//    check(Locked)
    check(NHoldAll)
    check(NHoldFirst)
    check(NHoldRest)
    check(NumericFunction)
    check(OneIdentity)
    check(Orderless)
    check(Protected)
    check(ReadProtected)
    check(SequenceHold)
//    check(Stub)
//    check(Temporary)
#undef check
    return -1;
}

static int32_t parse_attr_full(er e)
{
    if (ehas_head_sym(e, gs.sym_sList.get()))
    {
        int32_t r = 0;
        for (size_t i = 1; i <= elength(e); i++)
            r |= parse_attr(echild(e,i));
        return r;
    }
    else
    {
        return parse_attr(e);
    }
}

ex dcode_sClearAttributes(er e)
{
//std::cout << "dcode_sAttributes: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sClearAttributes.get()));

    if (elength(e) == 2 && eis_sym(echild(e,1)))
    {
        int32_t attr = parse_attr_full(echild(e,2));
        if (attr >= 0)
        {
            esym_normalattr(echild(e,1)) &= ~attr;
            return gs.sym_sNull.copy();
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

ex dcode_sSetAttributes(er e)
{
//std::cout << "dcode_sAttributes: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sSetAttributes.get()));

    if (elength(e) == 2 && eis_sym(echild(e,1)))
    {
        int32_t attr = parse_attr_full(echild(e,2));
        if (attr >= 0)
        {
            esym_normalattr(echild(e,1)) |= attr;
            return gs.sym_sNull.copy();
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

ex dcode_sProtect(er e)
{
    return ecopy(e);
}

ex dcode_sUnprotect(er e)
{
    return ecopy(e);
}
