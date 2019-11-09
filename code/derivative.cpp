#include "timing.h"
#include "uex.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"


ex eval_diff_Log(ex F, er e)
{
    ex d;
    uex f(F);
    d = etake_reciprocal(ecopychild(e,1));
    return ex_mul(f.release(), d);
}

ex eval_diff_Exp(ex F, er e)
{
    return ex_mul(F, ecopy(e));
}

ex eval_diff_Sin(ex F, er e)
{
    ex d;
    uex f(F);
    d = emake_node(ecopy(gs.sym_sCos.get()), ecopychild(e,1));
    return ex_mul(f.release(), d);
}

ex eval_diff_Cos(ex F, er e)
{
    ex d;
    uex f(F);
    d = emake_node(ecopy(gs.sym_sSin.get()), ecopychild(e,1));
    return ex_mul(emake_cint(-1), f.release(), d);
}

ex eval_diff_Tan(ex F, er e)
{
    ex d;
    uex f(F);
    d = emake_node(ecopy(gs.sym_sSec.get()), ecopychild(e,1));
    d = emake_node(ecopy(gs.sym_sPower.get()), d, emake_cint(2));
    return ex_mul(f.release(), d);
}

ex eval_diff_Csc(ex F, er e)
{
    ex d;
    uex f(F);
    d = emake_node(ecopy(gs.sym_sCot.get()), ecopychild(e,1));
    return ex_mul(emake_cint(-1), f.release(), d, ecopy(e));
}

ex eval_diff_Sec(ex F, er e)
{
    ex d;
    uex f(F);
    d = emake_node(ecopy(gs.sym_sTan.get()), ecopychild(e,1));
    return ex_mul(f.release(), ecopy(e), d);
}

ex eval_diff_Cot(ex F, er e)
{
    ex d;
    uex f(F);
    d = emake_node(ecopy(gs.sym_sCsc.get()), ecopychild(e,1));
    d = emake_node(ecopy(gs.sym_sPower.get()), d, emake_cint(2));
    return ex_mul(emake_cint(-1), f.release(), d);
}

ex eval_diff_Sinh(ex F, er e)
{
    ex d;
    uex f(F);
    d = emake_node(ecopy(gs.sym_sCosh.get()), ecopychild(e,1));
    return ex_mul(f.release(), d);
}

ex eval_diff_Cosh(ex F, er e)
{
    ex d;
    uex f(F);
    d = emake_node(ecopy(gs.sym_sSinh.get()), ecopychild(e,1));
    return ex_mul(f.release(), d);
}

ex eval_diff_Tanh(ex F, er e)
{
    ex d;
    uex f(F);
    d = emake_node(ecopy(gs.sym_sSech.get()), ecopychild(e,1));
    d = emake_node(ecopy(gs.sym_sPower.get()), d, emake_cint(2));
    return ex_mul(f.release(), d);
}

ex eval_diff_Csch(ex F, er e)
{
    ex d;
    uex f(F);
    d = emake_node(ecopy(gs.sym_sCoth.get()), ecopychild(e,1));
    return ex_mul(emake_cint(-1), f.release(), d, ecopy(e));
}

ex eval_diff_Sech(ex F, er e)
{
    ex d;
    uex f(F);
    d = emake_node(ecopy(gs.sym_sTanh.get()), ecopychild(e,1));
    return ex_mul(emake_cint(-1), f.release(), ecopy(e), d);
}

ex eval_diff_Coth(ex F, er e)
{
    ex d;
    uex f(F);
    d = emake_node(ecopy(gs.sym_sCsch.get()), ecopychild(e,1));
    d = emake_node(ecopy(gs.sym_sPower.get()), d, emake_cint(2));
    return ex_mul(emake_cint(-1), f.release(), d);
}


/* return a + b*E^i */
static ex _arctrig_helper(ex E, slong a, slong b, slong i)
{
    if (i != 1)
        E = ex_pow(E, emake_cint(i));
    if (b != 1)
        E = ex_mul(emake_cint(b), E);
    if (a != 0)
        E = ex_add(emake_cint(a), E);
    return E;
}

ex eval_diff_ArcSin(ex F, er e)
{
    ex d;
    uex f(F);
    d = _arctrig_helper(ecopychild(e,1), 1, -1, 2);
    d = emake_node(ecopy(gs.sym_sPower.get()), d, emake_crat(-1,2));
    return ex_mul(f.release(), d);
}

ex eval_diff_ArcCos(ex F, er e)
{
    ex d;
    uex f(F);
    d = _arctrig_helper(ecopychild(e,1), 1, -1, 2);
    d = emake_node(ecopy(gs.sym_sPower.get()), d, emake_crat(-1,2));
    return ex_mul(emake_cint(-1), f.release(), d);
}

ex eval_diff_ArcTan(ex F, er e)
{
    ex d;
    uex f(F);
    d = _arctrig_helper(ecopychild(e,1), 1, 1, 2);
    d = etake_reciprocal(d);
    return ex_mul(f.release(), d);
}

ex eval_diff_ArcCsc(ex F, er e)
{
    ex d;
    uex f(F);
    uex g(_arctrig_helper(ecopychild(e,1), 0, 1, -2));
    d = _arctrig_helper(ecopychild(e,1), 1, -1, -2);
    d = emake_node(ecopy(gs.sym_sPower.get()), d, emake_crat(-1,2));
    return ex_mul(emake_cint(-1), f.release(), g.release(), d);
}

ex eval_diff_ArcSec(ex F, er e)
{
    ex d;
    uex f(F);
    uex g(_arctrig_helper(ecopychild(e,1), 0, 1, -2));
    d = _arctrig_helper(ecopychild(e,1), 1, -1, -2);
    d = emake_node(ecopy(gs.sym_sPower.get()), d, emake_crat(-1,2));
    return ex_mul(f.release(), g.release(), d);
}

ex eval_diff_ArcCot(ex F, er e)
{
    ex d;
    uex f(F);
    d = _arctrig_helper(ecopychild(e,1), 1, 1, 2);
    d = etake_reciprocal(d);
    return ex_mul(emake_cint(-1), f.release(), d);
}



ex eval_diff_ArcSinh(ex F, er e)
{
    ex d;
    uex f(F);
    d = _arctrig_helper(ecopychild(e,1), 1, 1, 2);
    d = emake_node(ecopy(gs.sym_sPower.get()), d, emake_crat(-1,2));
    return ex_mul(f.release(), d);
}

ex eval_diff_ArcCosh(ex F, er e)
{
    uex f(F);
    uex d1(_arctrig_helper(ecopychild(e,1), -1, 1, 1));
    uex d2(_arctrig_helper(ecopychild(e,1), 1, 1, 1));
    d1.reset(ex_pow(d1.release(), emake_crat(-1,2)));
    d2.reset(ex_pow(d2.release(), emake_crat(-1,2)));
    return ex_mul(f.release(), d1.release(), d2.release());
}

ex eval_diff_ArcTanh(ex F, er e)
{
    ex d;
    uex f(F);
    d = _arctrig_helper(ecopychild(e,1), 1, -1, 2);
    d = etake_reciprocal(d);
    return ex_mul(f.release(), d);
}

ex eval_diff_ArcCsch(ex F, er e)
{
    ex d;
    uex f(F);
    uex g(_arctrig_helper(ecopychild(e,1), 0, 1, -2));
    d = _arctrig_helper(ecopychild(e,1), 1, 1, -2);
    d = ex_pow(d, emake_crat(-1,2));
    return ex_mul(emake_cint(-1), f.release(), g.release(), d);
}

ex eval_diff_ArcSech(ex F, er e)
{
    ex d;
    uex f(F);
    uex d1(_arctrig_helper(ecopychild(e,1), 1, +1, 1));
    uex d2(_arctrig_helper(ecopychild(e,1), 1, -1, 1));
    d2.reset(ex_div(d2.release(), d1.copy()));
    d2.reset(ex_pow(d2.release(), emake_crat(-1,2)));
    d1.reset(etake_reciprocal(d1.release()));
    ex g = etake_reciprocal(ecopychild(e,1));
    return ex_mul(f.release(), g, d2.release(), d1.release());
}

ex eval_diff_ArcCoth(ex F, er e)
{
    ex d;
    uex f(F);
    d = _arctrig_helper(ecopychild(e,1), 1, -1, 2);
    d = etake_reciprocal(d);
    return ex_mul(emake_cint(-1), f.release(), d);
}

ex eval_diff(er e, er var);

void _diff_arg(std::vector<wex> &v, er e, er child, std::vector<size_t> &idx, er var)
{
//std::cout << "_diff_arg called child: " << ex_tostring_full(child) << std::endl;

	if (ehas_head_sym(child, gs.sym_sList.get()))
	{
		for (size_t i = 1; i <= elength(child); i++)
		{
			idx.push_back(i);
			idx.push_back(elength(child));
			_diff_arg(v, e, echild(child,i), idx, var);
			idx.pop_back();
			idx.pop_back();
		}
	}
	else
	{
		uex f(eval_diff(child, var));
		if (!eis_zero(f.get()))
		{
			assert(idx.size() >= 2);
			assert((idx.size() % 2) == 0);
			uex cur(emake_cint(1));
			for (size_t j = idx.size(); j >= 2; j -= 2)
			{
				uex new_cur(j > 2 ? gs.sym_sList.get() : gs.sym_sDerivative.get(), idx[j - 1]);
				for (size_t k = 1; k <= idx[j - 1]; k++)
				{
					new_cur.push_back(k == idx[j - 2] ? cur.copy() : emake_cint(0));
				}
				cur.setnz(new_cur.release());
			}
			uex ne(ecopy(e));
			er e0 = echild(e,0);
			if (eis_node(e0)
				 && elength(e0) == 1
				 && ehas_head_sym_length(echild(e0,0), gs.sym_sDerivative.get(), elength(cur.get())))
			{
				uex new_cur(gs.sym_sDerivative.get(), elength(cur.get()));
				for (size_t k = 1; k <= elength(cur.get()); k++)
				{
					new_cur.push_back(ex_add(cur.child(k), echild(e0,0,k)));
				}
				ne.replacechild(0, emake_node(new_cur.release(), ecopychild(e0,1)));
			}
			else
			{
				ne.replacechild(0, emake_node(cur.release(), ecopy(e0)));
			}
			v.push_back(ex_mul(f.release(), ne.release()));
		}
	}
}

ex eval_diff(er e, er var)
{
//std::cout << "eval_diff: e: " << ex_tostring_full(e) << " var: " << ex_tostring_full(var) << std::endl;

    if (!eis_node(e))
    {
        return emake_cint(eis_sym(e, var) ? 1 : 0);
    }
	if (ex_same(e, var))
	{
        return emake_cint(1);
	}
    size_t n = elength(e);
    er h = echild(e,0);
    if (h == gs.sym_sPlus.get())
    {
        uex d(gs.sym_sPlus.get(), n);
        for (size_t i = 1; i <= n; i++)
        {
            d.push_back(eval_diff(echild(e,i),var));
        }
        assert(evalid_bucketflags(d.get()));
        return dcode_sPlus(d.get());
    }
    else if (h == gs.sym_sTimes.get())
    {
        uex d(gs.sym_sPlus.get(), n);
        assert(evalid_bucketflags(d.get()));
        for (size_t i = 1; i <= n; i++)
        {
            uex f(ecopy(e));
            f.replacechild(i, eval_diff(echild(e,i), var));
            assert(evalid_bucketflags(echild(f.get(),i)));
            assert(evalid_bucketflags(f.get()));
            d.push_back(dcode_sTimes(f.get()));
        }
        assert(evalid_bucketflags(d.get()));
        return dcode_sPlus(d.get());
    }
    else if (n == 2 && h == gs.sym_sPower.get())
    {
        er a = echild(e,1);
        er b = echild(e,2);
        uex y(eval_diff(b, var));
        if (eis_zero(y.get()))
        {
            y.reset(_arctrig_helper(ecopy(b), -1, 1, 1));
            y.reset(ex_pow(ecopy(a), y.release()));
            ex x = eval_diff(a,var);
            return ex_mul(ecopy(b), x, y.release());
        }
        else
        {
            uex x(emake_node(gs.sym_sLog.copy(), ecopy(a)));
            x.reset(dcode_sLog(x.get()));
            y.reset(ex_mul(y.release(), x.release()));
            x.set(ex_mul(ecopy(b), eval_diff(a,var), etake_reciprocal(ecopy(a))));
            return ex_mul(ecopy(e), ex_add(x.release(), y.release()));
        }
    }
    else if (h == gs.sym_sList.get())
    {
        uex d(gs.sym_sList.get(), n);
        for (size_t i = 1; i <= n; i++)
        {
            d.push_back(eval_diff(echild(e,i), var));
        }
        return d.release();
    }
    else if (h == gs.sym_sMinus.get())
    {
        uex d(gs.sym_sMinus.get(), n);
        for (size_t i = 1; i <= n; i++)
        {
            d.push_back(eval_diff(echild(e,i),var));
        }
        return dcode_sMinus(d.get());
    }
    else if (n == 1)
    {
        ex (*df)(ex, er) = nullptr;
             if (false) df = nullptr;
#define CHECK(name) else if (h == gs.sym_s##name.get()) df = eval_diff_##name;
        CHECK(Log)
        CHECK(Exp)
        CHECK(Sin)
        CHECK(Cos)
        CHECK(Tan)
        CHECK(Csc)
        CHECK(Sec)
        CHECK(Cot)
        CHECK(Sinh)
        CHECK(Cosh)
        CHECK(Tanh)
        CHECK(Csch)
        CHECK(Sech)
        CHECK(Coth)
        CHECK(ArcSin)
        CHECK(ArcCos)
        CHECK(ArcTan)
        CHECK(ArcCsc)
        CHECK(ArcSec)
        CHECK(ArcCot)
        CHECK(ArcSinh)
        CHECK(ArcCosh)
        CHECK(ArcTanh)
        CHECK(ArcCsch)
        CHECK(ArcSech)
        CHECK(ArcCoth)
#undef CHECK

        if (df != nullptr)
        {
            ex F = eval_diff(echild(e,1), var);
            if (eis_zero(F))
            {
                return F;
            }
            else
            {
                return df(F, e);
            }
        }
    }

	std::vector<wex> v;
	ex hd = eval_diff(echild(e,0), var);
	if (eis_zero(hd))
	{
		eclear(hd);
	}
	else
	{
		wex t(ecopy(e));
		t.replacechild(0, hd);
		v.push_back(t);
	}

	std::vector<size_t> idx;
	for (size_t i = 1; i <= elength(e); i++)
	{
		idx.clear();
		idx.push_back(i);
		idx.push_back(elength(e));
		_diff_arg(v, e, echild(e,i), idx, var);
	}

	uex s(emake_node(gs.sym_sPlus.copy(), v));
	return dcode_sPlus(s.get());

}


ex dcode_sD(er e)
{
//std::cout << "dcode_sD: " << ex_tostring_full(e) << std::endl;

	if (elength(e) == 0)
	{
		return _handle_message_argm(e, 1);
	}

	uex c(ecopychild(e,1));
	for (size_t i = 2; i <= elength(e); i++)
	{
		if (ehas_head_sym(echild(e,i), gs.sym_sList.get()))
		{
			return ecopy(e);
		}
		else
		{
			c.setnz(eval_diff(c.get(), echild(e,i)));
		}
	}
	return c.release();
}
