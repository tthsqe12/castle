#include "globalstate.h"
#include "code.h"
#include "eval.h"
#include "code/local.h"


static bool _check_local_vars(er e)
{
    er ll = echild(e,1);
    if (!ehas_head_sym(ll, gs.sym_sList.get()))
    {
        _gen_message(echild(e,0), "lvlist", "Local variable specification `1` is not a List.", ecopy(ll));
        return true;
    }
    ulong n = elength(ll);
    for (ulong i = 1; i <= n; i++)
    {
        er lli = echild(ll,i);
        if (!eis_sym(lli))
        {        
            if (   !ehas_head_sym_length(lli, gs.sym_sSet.get(), 2)
                && !ehas_head_sym_length(lli, gs.sym_sSetDelayed.get(), 2))
            {
                _gen_message(echild(e,0), "lvsym", "Local variable specification `1` contains `2`, which is not a symbol or an assignment to a symbol.", ecopy(ll), ecopy(lli));
                return true;
            }
            if (!eis_sym(echild(lli,1)))    
            {
                _gen_message(echild(e,0), "lvsym", "Local variable specification `1` contains `2`, which is not a symbol or an assignment to a symbol.", ecopy(ll), ecopy(lli));
                return true;
            }
        }
    }

    return false;
}


ex dcode_sBlock(er e)
{
//std::cout << "dcode_sBlock: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sBlock.get()));

    if (elength(e) != 2)
        _handle_message_argx(e, 2);

    if (_check_local_vars(e))
        return ecopy(e);

    er ll = echild(e,1);
    ulong n = elength(ll);

    std::vector<uex> rhs(n);
    for (ulong i = 0; i < n; i++)
    {
        er lli = echild(ll,i + 1);
        if (ehas_head_sym_length(lli, gs.sym_sSet.get(), 2))
        {
            rhs[i].set(eval(ecopychild(lli,2)));
        }
        else if (ehas_head_sym_length(lli, gs.sym_sSetDelayed.get(), 2))
        {
            rhs[i].set(ecopychild(lli,2));
        }
    }

    sym_localize_mul ml(n);
    for (ulong i = 0; i < n; i++)
    {
        er lli = echild(ll,i + 1);
        if (rhs[i].get() == nullptr)
        {
            assert(eis_sym(lli));
            ml.push_back(lli);
        }
        else
        {
            assert(   ehas_head_sym_length(lli, gs.sym_sSet.get(), 2)
                   || ehas_head_sym_length(lli, gs.sym_sSetDelayed.get(), 2));
            assert(eis_sym(echild(lli,1)));
            ml.push_back(echild(lli,1));
            esym_set_ovalue(ml.back().entry, rhs[i].release());
        }
    }

    return eval(ecopychild(e,2)); // must call eval here
}


static void _increment_numberstring(std::string & s, size_t off)
{
    size_t n = s.length();
    assert(n > off);

    for (size_t i = n; i > off; i--)
    {
        if (s[i - 1] != '9')
        {
            s[i - 1]++;
            for ( ; i < n; i++)
                s[i] = '0';
            return;
        }
    }

    s[off] = '1';
    for (size_t i = off + 1; i < n; i--)
        s[i] = '0';

    s.push_back('0');
}


static ex _unique_symbol(er e)
{
    std::string name;
    wex cont(gs.context.copy());
    size_t pos;

    if (eis_sym(e))
    {
        cont.reset(ecopy(esym_context(e)));
        name = esym_name_string(e);
        pos = 0;
    }
    else if (eis_str(e))
    {
        std::string context;

        const unsigned char * s = reinterpret_cast<const unsigned char *>(estr_string(e).c_str());
        size_t sn = estr_string(e).size();
        size_t si = 0;
        while (si < sn)
        {
            char16_t c;
            si += readonechar16(c, s + si);

            if (c == '`')
            {
                if (name.empty())
                    goto bad;
                context.append(name);
                context.push_back('`');
                name.clear();
            }
            else
            {
                if (!(isletterchar(c) || (!name.empty() && '0' <= c && c <= '9')))
                    goto bad;
                stdstring_pushback_char16(name, c);
            }
        }

        if (name.empty())
            goto bad;

        if (!context.empty())
        {
            auto ret = gs.live_contexts.insert(uex(emake_str(std::move(context))));
            cont.reset((*ret.first).copy());
        }

        pos = 1;
    }
    else
    {
bad:
        _gen_message(gs.sym_sUnique.get(), "usym", "`1` is not a symbol or a valid symbol name.", ecopy(e));
        return nullptr;
    }

    uex x;
    while (true)
    {
        x.reset(emake_sym(cont.copy(), emake_str(name + gs.modulenumberstring.substr(pos))));
        if (gs.live_symbols.count(x) == 0 && gs.live_tsymbols.count(x.get()) == 0)
        {
            auto ret = gs.live_psymbols.insert(x);
            if (ret.second)
            {
                return (*ret.first).copy();
            }
        }
        _increment_numberstring(gs.modulenumberstring, 1);
    }
}

ex dcode_sUnique(er e)
{
//std::cout << "dcode_sUnique: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sUnique.get()));

    if (elength(e) > 1)
        return _handle_message_argb(e, 0 + (1 << 8));

    install_live_psymbols LPS;

    if (elength(e) == 0)
    {
        wex x(emake_str("$"));
        ex r = _unique_symbol(x.get());
        if (r == nullptr)
            return ecopy(e);
        gs.send_psymbols();
        return r;
    }
    else
    {
        er f = echild(e,1);
        if (ehas_head_sym(f, gs.sym_sList.get()))
        {
            size_t n = elength(f);
            uex g(echild(f,0), n);
            for (size_t i = 1; i <= n; i++)
            {
                ex r = _unique_symbol(f);
                if (r == nullptr)
                    return ecopy(e);
                g.push_back(r);
            }
            gs.send_psymbols();
            return g.release();            
        }
        else
        {
            ex r = _unique_symbol(f);
            if (r == nullptr)
                return ecopy(e);
            gs.send_psymbols();
            return r;
        }
    }
}


static ex _replace_vars(er e, er * a, uex * b, size_t n)
{
    if (eis_node(e))
    {
        size_t len = elength(e);
        er h = echild(e,0);
        ex fh = _replace_vars(h, a, b, n);
        bool changed = etor(fh) != h;
        uex f(fh, len);
        for (size_t i = 1; i <= len; i++)
        {
            er c = echild(e,i);
            ex fc = _replace_vars(c, a, b, n);
            changed = changed || (etor(fc) != c);
            f.push_back(fc);
        }

        if (changed)
            return f.release();
        else
            return ecopy(e);
    }
    else if (eis_sym(e))
    {
        for (size_t i = 0; i < n; i++)
        {
            if (e == a[i])
                return b[i].copy();
        }
    }

    return ecopy(e);
}


ex dcode_sModule(er e)
{
//std::cout << "dcode_sModule: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sModule.get()));

    if (elength(e) != 2)
        _handle_message_argx(e, 2);

    if (_check_local_vars(e))
        return ecopy(e);

    er ll = echild(e,1);
    ulong n = elength(ll);

    std::vector<uex> rhs(n);
    for (ulong i = 0; i < n; i++)
    {
        er lli = echild(ll,i + 1);
        if (ehas_head_sym_length(lli, gs.sym_sSet.get(), 2))
        {
            rhs[i].set(eval(ecopychild(lli,2)));
        }
        else if (ehas_head_sym_length(lli, gs.sym_sSetDelayed.get(), 2))
        {
            rhs[i].set(ecopychild(lli,2));
        }
    }

    std::vector<std::string> modulenumbers;
    modulenumbers.push_back(std::move(gs.modulenumberstring));
    std::vector<uex> lhs(n);
    std::vector<er> org(n);
    for (ulong i = 0; i < n; i++)
    {
        er lli = echild(ll,i + 1);
        if (rhs[i].get() == nullptr)
        {
            org[i] = lli;            
        }
        else
        {
            assert(   ehas_head_sym_length(lli, gs.sym_sSet.get(), 2)
                   || ehas_head_sym_length(lli, gs.sym_sSetDelayed.get(), 2));
            org[i] = echild(lli,1);
        }

        assert(eis_sym(org[i]));

        size_t j = 0;
        while (true)
        {
            assert(j < modulenumbers.size());
            lhs[i].reset(emake_sym(ecopy(esym_context(org[i])), emake_str(esym_name_string(org[i]) + modulenumbers[j])));
            if (0 == gs.live_symbols.count(lhs[i]))
            {
                auto ret = gs.live_tsymbols.insert(lhs[i].get());
                if (ret.second)
                    break;
            }
            j++;
            if (j >= modulenumbers.size())
            {
                modulenumbers.push_back(modulenumbers.back());
                _increment_numberstring(modulenumbers.back(), 1);
            }
        }

        esym_extraattr(lhs[i].get()) |= XATTR_Temporary;
        if (rhs[i].get() != nullptr)
            esym_ovalue(lhs[i].get()) = etor(rhs[i].release());
    }

    gs.modulenumberstring = std::move(modulenumbers.back());
    _increment_numberstring(gs.modulenumberstring, 1);

    return _replace_vars(echild(e,2), org.data(), lhs.data(), n); // no need to call eval
}
