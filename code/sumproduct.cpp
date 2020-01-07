#include "uex.h"
#include "timing.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"
#include "code/local.h"

class accumulator {
public:
    ulong ncount;
    unsigned int ecount;
    std::vector<uex> estack;
    std::vector<uex> nstack;

    int emptyset;
    ex (*num_combine)(er, er);
    ex (*expr_combine)(er);
    wex head;
    

    accumulator(int emptyset_, er head_, ex (*num_combine_)(er, er), ex (*expr_combine_)(er)) :
        num_combine(num_combine_),
        expr_combine(expr_combine_),
        head(ecopy(head_)),
        emptyset(emptyset_),
        ncount(0),
        ecount(0)
    {
    }

    void push(ex T)
    {
        if (eis_number(T))
        {
            nstack.push_back(uex(T));
            size_t l = nstack.size();
            for (ulong k = ncount++; ((k&1)!=0) && l >= 2; k = k >> 1)
            {
                ex t = num_combine(nstack[l - 1].get(), nstack[l - 2].get());
                nstack.pop_back(); l--;
                nstack[l - 1].setnz(t);
            }
        }
        else
        {
            estack.push_back(uex(T));
            ecount++;
            if ((ecount%1024) == 0 && estack.size() >= 1024)
            {
                uex e1(emake_node(head.copy(), estack.size() - 1024, estack));
                estack.push_back(uex(dcode_sPlus(e1.get())));
            }
        }
    }

    ex release_sum()
    {
        size_t l = nstack.size();
        while (l > 1)
        {
            ex t = num_combine(nstack[l - 1].get(), nstack[l - 2].get());
            nstack.pop_back(); l--;
            nstack[l - 1].setnz(t);
        }

        if (nstack.empty() && estack.empty())
        {
            return emake_cint(emptyset);
        }
        else
        {
            return emake_node(head.copy(), nstack, estack);
        }
    }
};


/* return
    0 for success
    1 for failure (bad input)
*/
static int _sum_nested(accumulator & A, er e, size_t pos)
{
    size_t n = elength(e);
    assert(n >= 2);
    assert(pos <= n);

    er it = echild(e, pos);
    if (ehas_head_sym(it, gs.sym_sList.get()) && elength(it) >= 2 && elength(it) <= 4)
    {
        uex var(ecopychild(it,1));
        uex sym;
        int value_type = 0;
        if (eis_sym(var.get()))
        {
            sym.set(var.copy());
        }
        else if (eis_node(var.get()))
        {
            var.set(eval(var.release()));
            if (eis_sym(var.get()))
            {
                sym.set(var.copy());
            }
            else if (eis_node(var.get()))
            {
                if (eis_sym(var.child(0)))
                {
                    value_type = 1;
                    sym.set(var.copychild(0));
                }
                else
                {
                    value_type = 2;
                    sym.set(ecopy(esymbolic_head(var.child(0))));
                }
            }
            else
            {
                _gen_message(echild(e,0), "itraw", NULL, var.copy());
                return 1;
            }
        }
        else
        {
            _gen_message(echild(e,0), "itraw", NULL, var.copy());
            return 1;
        }

        // sym is the symbol to localize
        // variable the value to set
        // value_type = 0 -> ovalue
        // value_type = 1 -> dvalue
        // value_type = 2 -> svalue
        sym_localize L(sym.get());

        uex lower(eval(ecopychild(it,2)));
        if (elength(it) == 2 && ehas_head_sym(lower.get(), gs.sym_sList.get()))
        {
            size_t count = 0;
            while (++count <= elength(lower.get()))
            {
                if (value_type == 0)
                    esym_assign_ovalue(sym.get(), lower.copychild(count));
                else if (value_type == 1)
                    esym_assign_dvalue(sym.get(), var.copy(), lower.copychild(count));
                else if (value_type == 2)
                    esym_assign_svalue(sym.get(), var.copy(), lower.copychild(count));

                if (pos >= n)
                {
                    assert(pos == n);
                    A.push(eval(ecopychild(e, 1)));
                }
                else
                {
                    int retcode = _sum_nested(A, e, pos + 1);
                    if (retcode != 0)
                        return retcode;
                }
            }
            return 0;
        }
        else
        {
            uex upper(emake_cint(1));
            uex step(emake_cint(1));
            if (elength(it) == 2)
            {
                upper.swap(lower);
            }
            else
            {
                assert(elength(it) >= 3);
                upper.reset(eval(ecopychild(it,3)));
                if (elength(it) != 3)
                {
                    assert(elength(it) == 4);
                    step.reset(eval(ecopychild(it,4)));
                }
            }
            uex scount(ex_sub(upper.copy(), lower.copy()));
            scount.set(ex_div(scount.release(), step.copy()));
            scount.reset(ex_floor(scount.release()));
//std::cout << "scount after floor: " << ex_tostring(scount.get()) << std::endl;
            if (!eis_int(scount.get()) || !fmpz_fits_si(scount.int_data()))
            {
                _gen_message(echild(e,0), "iterb", NULL, emake_node(gs.sym_sList.copy(), var.copy(), lower.copy(), upper.copy(), step.copy()));
                return 1;
            }
            slong countlimit = fmpz_get_si(scount.int_data());
            slong count = -2;
//std::cout << "countlimit: " << countlimit<< std::endl;
            while (++count < countlimit)
            {
                ex itval = ex_add(lower.copy(), ex_mul_si(step.copy(), count + 1));
                if (value_type == 0)
                    esym_assign_ovalue(sym.get(), itval);
                else if (value_type == 1)
                    esym_assign_dvalue(sym.get(), var.copy(), itval);
                else if (value_type == 2)
                    esym_assign_svalue(sym.get(), var.copy(), itval);

                if (pos >= n)
                {
                    A.push(eval(ecopychild(e,1)));
                }
                else
                {
                    int retcode = _sum_nested(A, e, pos + 1);
//std::cout<<"print pos: "<< pos << " retcode:" << retcode << std::endl;
                    if (retcode != 0)
                        return retcode;
                }
            }
            return 0;
        }
    }

    if (ehas_head_sym_length(it, gs.sym_sList.get(), 1))
    {
        it = echild(it,1);
    }
    else
    {
        _gen_message(echild(e,0), "iterb", NULL, ecopychild(e, pos));
        return 1;
    }

    uex scount(ex_floor(ecopy(it)));
//std::cout << "scount after floor: " << ex_tostring(scount.get()) << std::endl;
    if (!eis_int(scount.get()) || !fmpz_fits_si(scount.int_data()))
    {
        _gen_message(echild(e,0), "iterb", NULL, ecopychild(e, pos));
        return 1;
    }
    slong countlimit = fmpz_get_si(scount.int_data());
    slong count = -1;
//std::cout << "countlimit: " << countlimit<< std::endl;
    while (++count < countlimit)
    {
//std::cout << "count: " << count<< std::endl;

        if (pos >= n)
        {
            A.push(eval(ecopychild(e, 1)));
        }
        else
        {
            int retcode = _sum_nested(A, e, pos + 1);
//std::cout<<"print pos: "<< pos << " retcode:" << retcode << std::endl;
            if (retcode != 0)
                return retcode;
        }
    }

    return 0;
}



/*
    Sum[expr, {i,imax}]
    Sum[expr, {i,imin,imax}]
    Sum[expr, {i,imin,imax,istep}]
    Sum[expr, {i,{i1,i2,...,in}}]
*/
ex dcode_sSum(er e)
{
//std::cout << "dcode_sSum: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sSum.get()));

    if (elength(e) < 2)
        _handle_message_argm(e, 2);

    accumulator A(0, gs.sym_sPlus.get(), &num_Plus2, &dcode_sPlus);
    int retcode = _sum_nested(A, e, 2);
    if (retcode == 0)
    {
        return A.release_sum();
    }
    else
    {
        return ecopy(e);
    }
}


ex dcode_sProduct(er e)
{
//std::cout << "dcode_sProduct: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sProduct.get()));

    if (elength(e) < 2)
        _handle_message_argm(e, 2);

    accumulator A(1, gs.sym_sTimes.get(), &num_Times2, &dcode_sTimes);
    int retcode = _sum_nested(A, e, 2);
    if (retcode == 0)
    {
        return A.release_sum();
    }
    else
    {
        return ecopy(e);
    }
}
