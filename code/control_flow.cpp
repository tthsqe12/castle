#include "globalstate.h"
#include "code.h"
#include "eval.h"

/*
    our control flow code is exceptionally good
*/

ex dcode_sIf(er e)
{
//std::cout << "dcode_sIf: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sIf.get()));

    er r = gs.sym_sNull.get();
    er e3 = gs.sym_sNull.get();

    switch (elength(e))
    {
        default:
            return _handle_message_argb(e, 2 + (4 << 8));
        case 4:
            r = echild(e,4);
        case 3:
            e3 = echild(e,3);
        case 2:
            NULL;
    }

    er e1 = echild(e,1);
    er e2 = echild(e,2);

    if (eis_sym(e1, gs.sym_sTrue.get()))
        r = e2;

    if (eis_sym(e1, gs.sym_sFalse.get()))
        r = e3;

    return ecopy(r);
}

ex dcode_sWhich(er e)
{
//std::cout << "dcode_sWhile: " << ex_tostring_full(e.get()) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sWhich.get()));

    ulong n = elength(e);

    if ((n % 2) != 0)
        return _handle_message_argct(e);

    for (ulong i = 0; i < n; i += 2)
    {
        er b = echild(e, i + 1);
        ex c = eval(ecopy(b));
        if (eis_sym(c, gs.sym_sTrue.get()))
        {
            eclear(c);
            return ecopychild(e, i + 2);
        }
        else if (eis_sym(c, gs.sym_sFalse.get()))
        {
            eclear(c);
            continue;
        }
        else if (i > 0 || etor(c) != b)
        {
            uex r(echild(e,0), n - i);
            r.push_back(c);        
            for (i++; i < n; i++)
                r.push_back(ecopychild(e, i + 1));
            return r.release();
        }
        else
        {
            eclear(c);
            return ecopy(e);
        }
    }

    return gs.sym_sNull.copy();
}

ex dcode_sWhile(er e)
{
//std::cout << "dcode_sWhile: " << ex_tostring_full(e.get()) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sWhile.get()));

    if (likely(elength(e) == 2))
    {
        ex r = eval(ecopychild(e,1));
        while (eis_sym(r, gs.sym_sTrue.get()))
        {
            eclear(r);
            try
            {
                eclear(eval(ecopychild(e,2)));
            }
            catch (const exception_sym_sBreak &)
            {
                return gs.sym_sNull.copy();
            }
            catch (const exception_sym_sContinue &)
            {
            }
            r = eval(ecopychild(e,1));
        }
        eclear(r);
        return gs.sym_sNull.copy();
    }
    else if (elength(e) == 1)
    {
        ex r = eval(ecopychild(e,1));
        while (eis_sym(r, gs.sym_sTrue.get()))
        {
            eclear(r);
            r = eval(ecopychild(e,1));
        }
        eclear(r);
        return gs.sym_sNull.copy();
    }
    else
    {
        return _handle_message_argt(e, 1 + (2 << 8));
    }
}

ex dcode_sBreak(er e)
{
//std::cout << "dcode_sBreak: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sBreak.get()));

    throw exception_sym_sBreak(nullptr);
    return nullptr;
}

ex dcode_sGoto(er e)
{
//std::cout << "dcode_sGoto: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sGoto.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    throw exception_sym_sGoto(ecopy(e));
    return nullptr;
}

ex dcode_sContinue(er e)
{
//std::cout << "dcode_sContinue: " << ex_tostring_full(e.get()) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sContinue.get()));

    throw exception_sym_sContinue(nullptr);
    return nullptr;
}

ex dcode_sCompoundExpression(er e)
{
//std::cout << "dcode_sCompoundExpression: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sCompoundExpression.get()));

    size_t n = elength(e);
    size_t i = 0;
    while (true)
    {
        i++;
        try
        {
            ex r = eval(ecopychild(e,i));
            if (i < n)
                eclear(r);
            else
                return r;
        }
        catch (const exception_sym_sGoto & X)
        {
            uex f(reinterpret_cast<ex>(X.data));
            for (i = 1; i <= n; i++)
            {
                er ei = echild(e,i);
                if (ehas_head_sym_length(ei, gs.sym_sLabel.get(), 1)
                                 && ex_same(echild(ei,1), f.child(1)))
                {
                    break;
                }
            }
            if (i > n)
            {
                throw exception_sym_sGoto(f.release());
                return nullptr;
            }
        }
    }
}

ex dcode_sReturn(er e)
{
//std::cout << "dcode_sReturn: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sReturn.get()));

    ex f = ecopy(e);
    if (elength(e) < 2)
    {
        throw exception_sym_sReturn(f);
    }
    else
    {
        throw exception_sym_sReturn_2(f);
    }
}

ex dcode_sFor(er e)
{
//std::cout << "dcode_sFor: " << ex_tostring_full(e.get()) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sFor.get()));

    if (elength(e) == 4)
    {
        eclear(eval(ecopychild(e,1)));
        ex r = eval(ecopychild(e,2));
        while (eis_sym(r, gs.sym_sTrue.get()))
        {
            eclear(r);
            try
            {
                eclear(eval(ecopychild(e,4)));
            }
            catch (const exception_sym_sBreak &)
            {
                return gs.sym_sNull.copy();
            }
            catch (const exception_sym_sContinue &)
            {
            }
            eclear(eval(ecopychild(e,3)));
            r = eval(ecopychild(e,2));
        }
        eclear(r);
        return gs.sym_sNull.copy();
    }
    else if (elength(e) == 3)
    {
        eclear(eval(ecopychild(e,1)));
        ex r = eval(ecopychild(e,2));
        while (eis_sym(r, gs.sym_sTrue.get()))
        {
            eclear(r);
            eclear(eval(ecopychild(e,3)));
            r = eval(ecopychild(e,2));
        }
        eclear(r);
        return gs.sym_sNull.copy();
    }
    else
    {
        return _handle_message_argt(e, (3 << 0) + (4 << 8));
    }
}

ex dcode_sSow(er e)
{
//std::cout << "dcode_sSow: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sSow.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    // Sow without enclosing Reap is OK
    if (!gs.reaper_stack.empty())
        gs.reaper_stack.back().push_back(wex(ecopychild(e,1)));

    return ecopychild(e,1);
}

class install_reaper {
private:
    size_t reaper_count;
public:
    install_reaper() {
        gs.reaper_stack.push_back(std::vector<wex>());
        reaper_count = gs.reaper_stack.size();
    }

    ~install_reaper() {
        assert(reaper_count == gs.reaper_stack.size());
        gs.reaper_stack.pop_back();
    }
};

ex dcode_sReap(er e)
{
//std::cout << "dcode_sReap: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sReap.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    install_reaper R;
    uex r(eval(ecopychild(e,1)));
    assert(!gs.reaper_stack.empty());
    ex t = emake_node(gs.sym_sList.copy(), gs.reaper_stack.back());
    t = emake_node(gs.sym_sList.copy(), t);
    return emake_node(gs.sym_sList.copy(), r.release(), t);
}

ex dcode_sThrow(er e)
{
//std::cout << "dcode_sThrow: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sThrow.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    throw exception_sym_sThrow(ecopy(e));
    return nullptr;
}

ex dcode_sCatch(er e)
{
//std::cout << "dcode_sCatch: " << ex_tostring_full(e.get()) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sCatch.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    try
    {
        return eval(ecopychild(e,1));
    }
    catch (const exception_sym_sThrow & X)
    {
        wex f(reinterpret_cast<ex>(X.data));
        assert(ehas_head_sym_length(f.get(), gs.sym_sThrow.get(), 1));
        return f.copychild(1);
    }
}

class install_muffler {
private:
    size_t muffler_count;
public:
    install_muffler() {
        gs.muffler_stack.push_back(emake_node(gs.sym_sList.copy()));
        muffler_count = gs.muffler_stack.size();
    }

    ~install_muffler() {
        assert(muffler_count == gs.muffler_stack.size());
        gs.muffler_stack.pop_back();
    }
};

ex dcode_sQuiet(er e)
{
//std::cout << "dcode_sQuiet: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sQuiet.get()));

    if (elength(e) != 1)
        return _handle_message_argx1(e);

    install_muffler M;
    uex r(eval(ecopychild(e,1)));
    return r.release();
}
