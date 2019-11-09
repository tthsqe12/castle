#include "globalstate.h"
#include "code.h"
#include "ex_parse.h"

static void _string_join(std::vector<uex> &l, std::vector<size_t> &bad, er e)
{
    size_t n = elength(e);
    for (size_t i = 1; i <= n; i++)
    {
        er ei = echild(e,i);
        if (eis_str(ei))
        {
            if (l.empty() || !eis_str(l.back().get()))
            {
                l.push_back(uex(ecopy(ei)));
            }
            else
            {
                if (!erefcnt_is1(l.back().get()))
                {
                    l.back().reset(emake_str_copy(eto_str(l.back().get())->string));
                }
                eto_str(l.back().get())->string.append(eto_str(ei)->string);
            }
        }
        else if (ehas_head_sym(ei, gs.sym_sList.get()))
        {
            _string_join(l, bad, ei);
        }
        else
        {
            l.push_back(uex(ecopy(ei)));
            bad.push_back(l.size());
        }
    }
}

ex dcode_sStringJoin(er e)
{
//std::cout << "dcode_sToCharacterCode: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sStringJoin.get()));

    std::vector<uex> l;
    std::vector<size_t> bad;
    _string_join(l, bad, e);
    if (bad.empty())
    {
        if (!l.empty())
        {
            assert(l.size() == 1);
            assert(eis_str(l[0].get()));
            return ecopy(l[0].get());
        }
        else
        {
            return emake_str();
        }
    }
    else
    {
        uex f(emake_node(gs.sym_sStringJoin.copy(), l));
        for (size_t i = 0; i < bad.size(); i++)
        {
            ex t = emake_int_ui(bad[i]);
            _gen_message(gs.sym_sToCharacterCode.get(), "string", NULL, t, f.copy());
        }
        return f.release();
    }
}

ex dcode_sStringLength(er e)
{
//std::cout << "dcode_sStringLength: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sStringLength.get()));

    return ecopy(e);
}

ex dcode_sToCharacterCode(er e)
{
//std::cout << "dcode_sToCharacterCode: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sToCharacterCode.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    er X = echild(e,1);
    if (eis_str(X))
    {
        std::vector<wex> v;
        size_t n = eto_str(X)->string.size();
        const unsigned char * a = (const unsigned char *) eto_str(X)->string.c_str();
        size_t i = 0;
        while(i < n)
        {
            char16_t c;
            i += readonechar16(c, a + i);
            v.push_back(wex(emake_int_ui(c)));
        }
        return emake_node(gs.sym_sList.copy(), v);
    }
    else if (ehas_head_sym(X, gs.sym_sList.get()))
    {
        uex l(gs.sym_sList.get(), elength(X));
        for (size_t j = 1; j <= elength(X); j++)
        {
            er Y = echild(X, j);
            if (!eis_str(Y))
            {
                _gen_message(gs.sym_sToCharacterCode.get(), "strse", NULL, ecopy(e));
                return ecopy(e);
            }
            std::vector<wex> v;
            size_t n = eto_str(Y)->string.size();
            const unsigned char * a = (const unsigned char *) eto_str(Y)->string.c_str();
            size_t i = 0;
            while (i < n)
            {
                char16_t c;
                i += readonechar16(c, a + i);
                v.push_back(wex(emake_int_ui(c)));
            }
            l.push_back(emake_node(gs.sym_sList.copy(), v));
        }
        return l.release();
    }
    else
    {
        _gen_message(gs.sym_sToCharacterCode.get(), "strse", NULL, ecopy(e));
        return ecopy(e);
    }
}
