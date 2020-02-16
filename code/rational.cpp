#include "globalstate.h"
#include "code.h"
#include "timing.h"
#include "ex_print.h"
#include "ex_cont.h"
#include "math.h"

#include "flint/profiler.h"

static void _get_exact_cfrac(
    _fmpz_vector_t s,
    _fmpz_mat22_t M, bool wantM,
    _fmpq_ball_t x)
{
    assert(x->exact);
    assert(fmpz_sgn(x->left_den) > 0);

    fmpz_t q;
    fmpz_init(q);

    int cmp = fmpz_cmp(x->left_num, x->left_den);
    if (cmp > 0)
    {
        _fmpq_ball_get_cfrac(s, M, wantM, x);
    }
    else
    {
        if (cmp >= 0 || fmpz_sgn(x->left_num) < 0)
            fmpz_fdiv_qr(q, x->left_num, x->left_num, x->left_den);
        fmpz_swap(x->left_num, x->left_den);

        _fmpz_vector_push_back(s, q);

        if (!fmpz_is_zero(x->left_den))
            _fmpq_ball_get_cfrac(s, M, wantM, x);

        if (wantM)
            _fmpz_mat22_lmul_elem(M, q);
    }

    assert(fmpz_is_zero(x->left_den) || _fmpq_ball_gt_one(x));

    while (s->length < s->limit && !fmpz_is_zero(x->left_den))
    {
        fmpz_fdiv_qr(q, x->left_num, x->left_num, x->left_den);
        fmpz_swap(x->left_num, x->left_den);
        _fmpz_vector_push_back(s, q);
        if (wantM)
            _fmpz_mat22_rmul_elem(M, q);
    }

    fmpz_clear(q);
}

static void _get_ball_cfrac(
    _fmpz_vector_t s,
    _fmpz_mat22_t M, bool wantM,
    _fmpq_ball_t x)
{
    assert(!x->exact);
    assert(fmpz_sgn(x->left_den) > 0);
    assert(fmpz_sgn(x->right_den) > 0);
    assert(_fmpq_cmp(x->left_num, x->left_den, x->right_num, x->right_den) <= 0);

    fmpz_t q, r;
    fmpz_init(q);
    fmpz_init(r);

    if (fmpz_cmp(x->left_num, x->left_den) > 0)
    {
timeit_t timer;

timeit_start(timer);
        /* 1 < x */
        _fmpq_ball_get_cfrac(s, M, 1, x);
timeit_stop(timer);
flint_printf("ball cfrac time: %wd\n", timer->wall);

    }
    else if (fmpz_sgn(x->left_num) > 0
                 && fmpz_cmp(x->right_num, x->right_den) < 0)
    {
        /* 0 < x < 1 */
        fmpz_swap(x->left_den, x->right_num);
        fmpz_swap(x->left_num, x->right_den);
        _fmpq_ball_get_cfrac(s, M, 1, x);
        fmpz_zero(q);
        _fmpz_mat22_lmul_elem(M, q);
    }
    else
    {
        _fmpq_ball_t y;

        _fmpq_ball_init(y);
        y->exact = 0;

        fmpz_fdiv_qr(q, r, x->left_num, x->left_den);
        fmpz_set(y->right_num, x->left_den);
        fmpz_swap(y->right_den, r);
        fmpz_set(y->left_num, x->right_den);
        fmpz_set(y->left_den, x->right_num);
        fmpz_submul(y->left_den, x->right_den, q);

        if (_fmpq_ball_gt_one(y))
        {
            _fmpq_ball_swap(x, y);
            _fmpq_ball_get_cfrac(s, M, 1, x);
            _fmpz_mat22_lmul_elem(M, q);
        }
        _fmpq_ball_clear(y);
    }

    fmpz_clear(q);
    fmpz_clear(r);
}


// input ball is sign*OpenInterval[{a/(b+1), (a+1)/b)}]
// output ball is x (if it is wanted)
static void _get_hgcd_cfrac(
    _fmpz_vector_t s,
    _fmpz_mat22_t M, bool wantM,
    _fmpq_ball_t x, bool wantx,
    fmpz_t a, fmpz_t b, int sign)
{
    assert(sign != 0);
    assert(fmpz_sgn(a) > 0);
    assert(fmpz_sgn(b) > 0);

    _fmpz_mat22_one(M);

    int cmp = fmpz_cmp(a, b);

    if (cmp == 0)
    {
        if (wantx)
        {
            fmpz_add_ui(x->right_num, a, 1);
            fmpz_swap(x->left_num, a);
            fmpz_add_ui(x->left_den, b, 1);
            fmpz_swap(x->right_den, b);
            x->exact = 0;
            if (sign < 0)
            {
                fmpz_swap(x->left_num, x->right_num);
                fmpz_swap(x->left_den, x->right_den);
                fmpz_neg(x->left_num, x->left_num);
                fmpz_neg(x->right_num, x->right_num);
            }
        }
        return;
    }

    _fmpz_mat22_t N;
    _fmpz_mat22_init(N);
    _fmpz_mat22_one(N);

    if (sign > 0)
    {
        if (cmp < 0)
        {
            fmpz_swap(a, b);
            _fmpz_vector_push_back_zero(s);
            fmpz_swap(M->_11, M->_12);
            fmpz_swap(M->_21, M->_22);
        }
    }
    else
    {
        bool ok = true;

        fmpz_swap(N->_12, N->_22);
        fmpz_swap(N->_11, N->_21);
        N->det = -1;

        if (cmp > 0)
        {
            fmpz_fdiv_qr(N->_11, a, a, b);
            fmpz_sub(a, a, N->_11);
            ok = fmpz_sgn(a) > 0;
        }
        fmpz_swap(a, b);
        fmpz_sub(b, a, b);

        fmpz_add_ui(N->_11, N->_11, 1);
        if (!ok || fmpz_cmp(b, N->_11) <= 0)
        {
            if (wantx)
            {
                fmpz_sub(b, b, a);
                fmpz_sub(b, b, N->_11);
                fmpz_add_ui(x->right_num, b, 1);
                fmpz_add_ui(x->right_den, a, 1);
                fmpz_swap(x->left_num, b);
                fmpz_swap(x->left_den, a);
                x->exact = 0;
            }
            return;
        }
        fmpz_neg(M->_12, N->_11);
        _fmpz_vector_push_back(s, M->_12);
        fmpz_mul_2exp(M->_12, M->_12, 1);
    }

    _fmpq_hgcd(s, N, a, b);

    if (wantM)
        _fmpz_mat22_rmul(M, N);

    if (wantx)
    {
        if (N->det != 1)
        {
            assert(N->det == -1);
            fmpz_swap(N->_12, N->_22);
            fmpz_swap(N->_11, N->_21);
        }
        fmpz_sub(x->left_num, a, N->_12);
        fmpz_add(x->left_den, b, N->_11);
        fmpz_add(x->right_num, a, N->_22);
        fmpz_sub(x->right_den, b, N->_21);
        x->exact = 0;
    }
}


static int _arb_to_fmpq_ball(_fmpq_ball_t x, const arb_t y)
{
    fmpz_t e;
    fmpz_init(e);
    fmpz_one(x->left_den);
    fmpz_one(x->right_den);

timeit_t timer;

timeit_start(timer);


    arb_get_interval_fmpz_2exp(x->left_num, x->right_num, e, y);
    if (!fmpz_fits_si(e))
    {
        fmpz_clear(e);
        return 1;
    }
    slong n = fmpz_get_si(e);
    if (n < 0)
    {
        fmpz_mul_2exp(x->left_den, x->left_den, -n);
        fmpz_mul_2exp(x->right_den, x->right_den, -n);
    }
    else
    {
        fmpz_mul_2exp(x->left_num, x->left_num, n);
        fmpz_mul_2exp(x->right_num, x->right_num, n);
    }
    x->exact = 0;

timeit_stop(timer);
flint_printf("arb->ball time: %wd\n", timer->wall);


    return 0;
}


/* ball is (-1)^sign*[center +/- radius]*2^-exp */
class nice_ball {
public:
    int bad;
    int sign;
    fmpz_t center;
    ulong radius;
    slong exp;

    nice_ball()
    {
        fmpz_init(center);
    }

    ~nice_ball()
    {
        fmpz_clear(center);
    }

    void set_double(double x);
};

void nice_ball::set_double(double x)
{
    if (likely(std::isfinite(x)))
    {
        bad = 0;
    }
    else
    {
        bad = 1;
        return;
    }

    int e;
    int64_t c = frexp(x, &e)*(1ULL << 53); /* 2^52 <= c < 2^53, i.e. bits(c) = 53 */
    exp = -(e - 53 - (64 - 53));
    uint64_t cen;
    if (c > 0)
    {
        cen = c;
        sign = 1;
    }
    else if (c < 0)
    {
        cen = -c;
        sign = -1;
    }
    else
    {
        cen = 0;
        sign = 0;
    }
    radius = -SAR(-cen, 53 + 1 - (64 - 53));
    fmpz_set_ui(center, cen << (64 - 53));
}


ex _rationalize_double(er dou)
{
    const slong goodness_constant_c = 10000;

    assert(eis_double(dou));

    slong t;
    xfmpz_t s, u, v, w, a, b, B1, B2, B1mid, B2mid, A1, A2;
    x_fmpq_ball_t X;
    x_fmpz_vector_t S;
    x_fmpz_mat22_t M;
    nice_ball x;

    x.set_double(edouble_number(dou));

    if (x.bad)
        return ecopy(dou);

    if (x.sign == 0)
        return emake_cint(0);

    t = std::max(slong(0), x.exp);
    fmpz_mul_2exp(s.data, eget_cint_data(1), t);

    for (ulong n = 2; n <= 4; n++)
    {
        fmpz_sub_ui(B1.data, x.center, n*x.radius);
        fmpz_mul_2exp(B1.data, B1.data, t - x.exp);
        fmpz_add(B1.data, B1.data, s.data);
        fmpz_fdiv_q_2exp(B1.data, B1.data, 1 + t - x.exp);
        fmpz_fdiv_q_ui(B1.data, B1.data, n*x.radius);
        fmpz_add_ui(B1.data, B1.data, 1);

        fmpz_sub_ui(B2.data, x.center, 1*x.radius);
        fmpz_mul_2exp(B2.data, B2.data, t - x.exp);
        fmpz_add(B2.data, B2.data, s.data);
        fmpz_cdiv_q_2exp(B2.data, B2.data, 1 + t - x.exp);
        fmpz_cdiv_q_ui(B2.data, B2.data, 1*x.radius);
        fmpz_sub_ui(B2.data, B2.data, 1);

        if (fmpz_sgn(B1.data) <= 0 || fmpz_cmp(B1.data, B2.data) > 0)
            break;

        fmpz_sub_ui(B1mid.data, x.center, n*x.radius);
        fmpz_mul_2exp(B1mid.data, B1mid.data, t - x.exp);
        fmpz_add(B1mid.data, B1mid.data, s.data);
        fmpz_fdiv_q_2exp(B1mid.data, B1mid.data, t - x.exp);
        fmpz_fdiv_q_ui(B1mid.data, B1mid.data, x.radius + n*x.radius);

        fmpz_sub_ui(B2mid.data, x.center, 1*x.radius);
        fmpz_mul_2exp(B2mid.data, B2mid.data, t - x.exp);
        fmpz_add(B2mid.data, B2mid.data, s.data);
        fmpz_cdiv_q_2exp(B2mid.data, B2mid.data, t - x.exp);
        fmpz_cdiv_q_ui(B2mid.data, B2mid.data, x.radius + n*x.radius);

        for (slong i = 0; i < 100; i++)
        {
            ulong fix;
            fmpz_mul_2exp(w.data, eget_cint_data(1), i);
            if (i&1)
                fmpz_add(b.data, B2mid.data, w.data);
            else
                fmpz_sub(b.data, B2mid.data, w.data);

            if (fmpz_cmp(b.data, B1.data) < 0)
                continue;
            if (fmpz_cmp(b.data, B2.data) > 0)
                continue;

            if (fmpz_cmp(b.data, B1mid.data) < 0)
            {
                fmpz_add_ui(u.data, b.data, 1);
                fmpz_sub_ui(v.data, x.center, n*x.radius);
                fmpz_mul(A1.data, u.data, v.data);
                fix = 1;
            }
            else
            {
                fmpz_add_ui(v.data, x.center, 1*x.radius);
                fmpz_mul(A1.data, b.data, v.data);
                fix = 0;
            }
            if (x.exp > 0) fmpz_fdiv_q_2exp(A1.data, A1.data, x.exp);
                    else   fmpz_mul_2exp(A1.data, A1.data, -x.exp);
            if (fix != 0) fmpz_add_ui(A1.data, A1.data, fix);

            if (fmpz_cmp(b.data, B2mid.data) > 0)
            {
                fmpz_add_ui(u.data, b.data, 1);
                fmpz_sub_ui(v.data, x.center, 1*x.radius);
                fmpz_mul(A2.data, u.data, v.data);
                fix = 1;
            }
            else
            {
                fmpz_add_ui(v.data, x.center, n*x.radius);
                fmpz_mul(A2.data, b.data, v.data);
                fix = 2;
            }
            if (x.exp > 0) fmpz_cdiv_q_2exp(A2.data, A2.data, x.exp);
                    else   fmpz_mul_2exp(A2.data, A2.data, -x.exp);
            fmpz_sub_ui(A2.data, A2.data, fix);

            if (fmpz_sgn(A1.data) <= 0 || fmpz_cmp(A1.data, A2.data) > 0)
                break;

            fmpz_add(a.data, A1.data, A2.data);
            fmpz_fdiv_q_2exp(a.data, a.data, 1);

            _get_hgcd_cfrac(S.data, M.data, true, X.data, true, a.data, b.data, 1);

            goto got_cfrac;
        }
    }

// hgcd ball failed, use fmpq ball

    fmpz_sub_ui(X.data->left_num, x.center, x.radius);
    fmpz_add_ui(X.data->right_num, x.center, x.radius);
    if (x.exp >= 0)
    {
        fmpz_mul_2exp(X.data->left_den, eget_cint_data(1), x.exp);
        fmpz_mul_2exp(X.data->right_den, eget_cint_data(1), x.exp);
    }
    else
    {
        fmpz_mul_2exp(X.data->left_num, X.data->left_num, -x.exp);
        fmpz_mul_2exp(X.data->right_num, X.data->right_num, -x.exp);
        fmpz_one(X.data->left_den);
        fmpz_one(X.data->right_den);
    }
    X.data->exact = 0;

    _get_ball_cfrac(S.data, M.data, true, X.data);

got_cfrac:

    for (slong j = 1; j < S.data->length; j++)
    {
        if (S.data->array[j] >= goodness_constant_c)
        {
            ex r = emake_rat();
            fmpq_set_cfrac(erat_data(r), S.data->array, j);
            if (x.sign < 0)
                fmpq_neg(erat_data(r), erat_data(r));
            return efix_rat(r);
        }
    }

    fmpz_cdiv_qr(A1.data, B1.data, X.data->left_num, X.data->left_den);
    fmpz_fdiv_qr(A2.data, B2.data, X.data->right_num, X.data->right_den);

    fmpz_mul_si(B1.data, B1.data, -goodness_constant_c - 1);
    fmpz_mul_si(B2.data, B2.data, goodness_constant_c);
    assert(fmpz_sgn(B1.data) >= 0);
    assert(fmpz_sgn(B2.data) >= 0);
    assert(fmpz_cmp(A2.data, A1.data) >= 0);
    if (S.data->length > 0 && fmpz_cmp_ui(A1.data, goodness_constant_c) > 0)
    {
        ex r = emake_rat();
        fmpz_swap(fmpq_numref(erat_data(r)), M.data->_11);
        fmpz_swap(fmpq_denref(erat_data(r)), M.data->_21);
        if (x.sign < 0)
            fmpq_neg(erat_data(r), erat_data(r));
        return efix_rat(r);
    }
    else if (fmpz_equal(A1.data, A2.data))
    {
        if (   fmpz_cmp(B1.data, X.data->left_den) < 0
            && fmpz_cmp(B2.data, X.data->right_den) < 0)
        {
            fmpz_addmul(M.data->_12, M.data->_11, A1.data);
            fmpz_addmul(M.data->_22, M.data->_21, A1.data);
            ex r = emake_rat();
            fmpz_swap(fmpq_numref(erat_data(r)), M.data->_12);
            fmpz_swap(fmpq_denref(erat_data(r)), M.data->_22);
            if (x.sign < 0)
                fmpq_neg(erat_data(r), erat_data(r));
            return efix_rat(r);
        }
    }

    return ecopy(dou);
}


ex dcode_iRealContinuedFraction(er e)
{
    if (elength(e) != 2)
        return ecopy(e);

    er X = echild(e,1);
    er N = echild(e,2);

    if (!eis_int(N) || !fmpz_fits_si(eint_data(N)))
        return ecopy(e);

    x_fmpq_ball_t x;
    x_fmpz_vector_t s;
    x_fmpz_mat22_t M;

    _fmpz_mat22_one(M.data);

    s.data->limit = fmpz_get_si(eint_data(N));
    if (s.data->limit <= 0)
        return ecopy(e);

    if (eis_real(X))
    {
        if (_arb_to_fmpq_ball(x.data, ereal_data(X)))
            return ecopy(e);
        _get_ball_cfrac(s.data, M.data, true, x.data);
    }
    else if (eis_rat(X))
    {
        fmpz_set(x.data->left_num, fmpq_numref(erat_data(X)));
        fmpz_set(x.data->left_den, fmpq_denref(erat_data(X)));
        x.data->exact = 1;
        _get_exact_cfrac(s.data, M.data, true, x.data);
    }
    else if (eis_int(X))
    {
        _fmpz_vector_push_back(s.data, eint_data(X));
        _fmpz_mat22_rmul_elem(M.data, eint_data(X));
    }
    else if (eis_double(X))
    {
    }
    else
    {
        return ecopy(e);
    }

    uex r(emake_parray_fmpz_vector(s.data));
    ex m = emake_node(gs.sym_sList.copy(),
                emake_node(gs.sym_sList.copy(), emake_int_move(M.data->_11), emake_int_move(M.data->_12)),
                emake_node(gs.sym_sList.copy(), emake_int_move(M.data->_21), emake_int_move(M.data->_22)));
    return emake_node(gs.sym_sList.copy(), r.release(), m);
}

ex dcode_iQuadraticContinuedFraction(er e)
{
    if (elength(e) != 3
        || !eis_int(echild(e,1))
        || !eis_int(echild(e,2))
        || !eis_int(echild(e,3)))
    {
        return ecopy(e);
    }

    xfmpz_t a(eint_data(echild(e,1)));
    xfmpz_t b(eint_data(echild(e,2)));
    xfmpz_t c(eint_data(echild(e,3)));
    xfmpz_t d, s, t, m, A, B, C;

    fmpz_mul(m.data, b.data, b.data);
    fmpz_mul(t.data, a.data, c.data);
    fmpz_addmul_ui(m.data, t.data, 4);

    if (fmpz_sgn(m.data) <= 0)
        return ecopy(e);
    fmpz_sqrtrem(d.data, t.data, m.data);
    if (fmpz_is_zero(t.data))
        return ecopy(e);

    std::vector<wex> ans1;
    std::vector<wex> ans2;

    while (fmpz_sgn(a.data) <= 0
           || fmpz_sgn(c.data) <= 0
           || (fmpz_sub(t.data, a.data, c.data),
               fmpz_abs(t.data, t.data),
               fmpz_cmp(b.data, t.data) <= 0))
    {
        fmpz_add(t.data, b.data, d.data);
        if (fmpz_sgn(a.data) > 0)
        {
            fmpz_fdiv_q(m.data, t.data, a.data);
            fmpz_fdiv_q_2exp(m.data, m.data, 1);
        }
        else
        {
            fmpz_cdiv_q(m.data, t.data, a.data);
            fmpz_cdiv_q_2exp(m.data, m.data, 1);
            fmpz_sub_ui(m.data, m.data, 1);
        }
        fmpz_mul(t.data, a.data, m.data);
        fmpz_sub(s.data, b.data, t.data);
        fmpz_addmul(c.data, s.data, m.data);
        fmpz_mul_2exp(t.data, t.data, 1);
        fmpz_sub(b.data, t.data, b.data);
        fmpz_swap(a.data, c.data);
        ans1.push_back(wex(emake_int_copy(m.data)));
    }

    fmpz_set(A.data, a.data);
    fmpz_set(B.data, b.data);
    fmpz_set(C.data, c.data);

    do {
        fmpz_add(t.data, b.data, d.data);
        fmpz_fdiv_q(m.data, t.data, a.data);
        fmpz_fdiv_q_2exp(m.data, m.data, 1);
        fmpz_mul(t.data, a.data, m.data);
        fmpz_sub(s.data, b.data, t.data);
        fmpz_addmul(c.data, s.data, m.data);
        fmpz_mul_2exp(t.data, t.data, 1);
        fmpz_sub(b.data, t.data, b.data);
        fmpz_swap(a.data, c.data);
        ans2.push_back(wex(emake_int_copy(m.data)));
    }
    while (!fmpz_equal(A.data, a.data) || !fmpz_equal(B.data, b.data));

    ans1.push_back(wex(emake_node(gs.sym_sList.copy(), ans2)));
    return emake_node(gs.sym_sList.copy(), ans1);
}


ex dcode_sContinuedFraction(er e)
{
//std::cout << "dcode_sContinuedFraction: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sContinuedFraction.get()));

    if (elength(e) == 1)
    {
        x_fmpq_ball_t x;
        x_fmpz_vector_t s;
        x_fmpz_mat22_t M;
        er X = echild(e,1);
        if (eis_real(X))
        {
            if (_arb_to_fmpq_ball(x.data, ereal_data(X)))
                return ecopy(e);
            _get_ball_cfrac(s.data, M.data, false, x.data);
            return emake_parray_fmpz_vector(s.data);
        }
        else if (eis_rat(X))
        {
            fmpz_set(x.data->left_num, fmpq_numref(erat_data(X)));
            fmpz_set(x.data->left_den, fmpq_denref(erat_data(X)));
            x.data->exact = 1;
            _get_exact_cfrac(s.data, M.data, false, x.data);
            return emake_parray_fmpz_vector(s.data);
        }
        else
        {
            return ecopy(e);
        }
    }
    else if (elength(e) == 2)
    {
        er X = echild(e,1);
        er N = echild(e,2);

        if (!eis_int(N) || !fmpz_fits_si(eint_data(N)))
            return ecopy(e);

        return ecopy(e);
    }
    else
    {
        return _handle_message_argt(e, (1 << 0) + (2 << 8));
    }
}

ex dcode_sRationalize(er e)
{
    if (elength(e) == 1)
    {
        er x = echild(e,1);
        if (eis_double(x))
            return _rationalize_double(x);
        else
            return ecopy(e);
    }
    else if (elength(e) == 2)
    {
        return ecopy(e);
    }
    else
    {
        return _handle_message_argt(e, (1 << 0) + (2 << 8));
    }
}


ex dcode_sChampernowneNumber(er e)
{
    if (elength(e) > 1)
        return _handle_message_argt(e, (0 << 0) + (1 << 8));

    if (elength(e) < 1)
        return ecopy(e);

    er x = echild(e,1);

    if (!eis_number(x))
        return ecopy(e);

    if (!eis_int(x) || fmpz_cmp_ui(eint_data(x), 1) <= 0)
        _gen_message(echild(e,0), "ibase", "Base `1` is not an integer greater than 1.", ecopychild(e,1));

    return ecopy(e);
}


ex ncode_sChampernowneNumber(er e, slong prec)
{
//std::cout << "ncode_sChampernowneNumber(" << prec << "): " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sChampernowneNumber.get()));

    if (elength(e) > 1)
        return ecopy(e);

    if (elength(e) == 1 && !eis_int(echild(e,1)))
        return ecopy(e);

    xfmpz_t B(UWORD(10));
    if (elength(e) == 1)
        fmpz_set(B.data, eint_data(echild(e,1)));

    if (fmpz_cmp_ui(B.data, 1) <= 0)
        return ecopy(e);

    xarb_t z, s, d, u, v;
    xfmpz_t a, t0, t1, t2, t3, t4, t6;

    if (!fmpz_abs_fits_ui(B.data))
    {
        // we only need one term
        fmpz_sub_ui(t0.data, B.data, 1);
        fmpz_mul(t2.data, t0.data, t0.data);
        arb_fmpz_div_fmpz(z.data, B.data, t2.data, prec + 1); // TODO add error
        return emake_real_move(z);
    }

    ulong b = fmpz_get_ui(B.data);
    ulong p = prec + 2 + fmpz_bits(B.data); // target is abserror < 2^-p

    double mlog2u = (b - 1)*log2(b);

    std::vector<xfmpz_t> ck;
    ck.push_back(xfmpz_t(UWORD(1)));
    ulong k = 1;
    while (true)
    {
        k++;
        fmpz_pow_ui(t0.data, B.data, k - 1);
        fmpz_mul_ui(t0.data, t0.data, k);
        fmpz_add(t0.data, t0.data, ck[k - 1-1].data);
        if (fmpz_cmp_ui(t0.data, 1 + (p + k + 2)/mlog2u) > 0)
            break;
        ck.push_back(t0);
    }
    ulong n = k - 1;

    arb_zero_pm_one(s.data);
    arb_mul_2exp_si(s.data, s.data, -slong(p + n + 2)); // tail error
    for (k = n; k != 0; k--)
    {
        ulong w1 = 5 + p + k;
        ulong w2 = mlog2u*fmpz_get_d(ck[k - 1].data);
        ulong w = 5 + (w1 > w2 ? w1 - w2 : 0);
        flint_bitcnt_t ckbits = fmpz_bits(ck[k - 1].data);
        fmpz_pow_ui(a.data, B.data, k);
        fmpz_sub_ui(t0.data, a.data, 1);
        fmpz_mul(t1.data, t0.data, a.data);
        fmpz_add_ui(t1.data, t1.data, 1);
        fmpz_mul(t2.data, t0.data, t0.data);
        fmpz_divexact_ui(t0.data, t0.data, b - 1);
        fmpz_mul(t3.data, t0.data, t0.data);
        fmpz_mul(t0.data, a.data, B.data);
        fmpz_sub_ui(t0.data, t0.data, 1);
        fmpz_mul(t4.data, t0.data, a.data);
        fmpz_add_ui(t4.data, t4.data, 1);
        fmpz_divexact_ui(t0.data, t0.data, b - 1);
        fmpz_mul(t6.data, t0.data, t0.data);
        fmpz_mul(t0.data, t2.data, t6.data);
        fmpz_mul(t2.data, t6.data, t1.data);
        fmpz_submul(t2.data, t3.data, t4.data);
        arb_fmpz_div_fmpz(d.data, t2.data, t0.data, w);
        arb_ui_pow_ui(u.data, b, b - 1, w + ckbits);
        arb_pow_fmpz(v.data, u.data, ck[k-1].data, w + ckbits);
        arb_div(z.data, d.data, v.data, w);
        arb_add(s.data, s.data, z.data, w);
    }
    fmpz_set_ui(t2.data, b - 1);
    fmpz_mul_ui(t2.data, t2.data, b - 1);
    arb_fmpz_div_fmpz(z.data, B.data, t2.data, prec + 3);
    arb_sub(z.data, z.data, s.data, prec + 3);
    return emake_real_move(z);
}
