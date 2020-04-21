#include <cmath>
#include <cfloat>

#include "timing.h"
#include "ex_print.h"
#include "ex_cont.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"
#include "flint/arith.h"
#include "flint/ui_factor.h"
#include "gmp.h"


#define PROFILE_THIS 0

#if PROFILE_THIS
static ulong timer_basecase, timer_fold, timer_float, timer_final_expand;
static ulong timer_fold_expand, timer_fold_fmpz, timer_fold_factor;
#endif

static ui_factor_sieve_t siever;

int ui_factor_equal_mpz(const ui_factor_t f, const mpz_t x)
{
    xfmpz_t y;
    mpz_ptr Y = _fmpz_promote(y.data);
    ui_factor_stack_t S;
    ui_factor_stack_init(S);
    ulong e = ui_factor_get_mpz_2exp(Y, f, S);
    mpz_mul_2exp(Y, Y, e);
    ui_factor_stack_clear(S);
    int ans = mpz_cmp(Y, x) == 0;
    _fmpz_demote_val(y.data);
    return ans;
}

/*
    Set {p, r, q} = sum of term_j with j in [start,stop] inclusive.
    The factored number mult is 640320^3/24.
    s is temporary working space
*/
static void pi_sum_basecase(
    mpz_t p, mpz_t s, ui_factor_t r, ui_factor_t q,
    ulong start, ulong stop, const ui_factor_t mult)
{
    mp_limb_t out, a[2];
    mp_size_t p_n, s_n;
    mp_limb_t * p_d, * s_d;
    ulong j;

    assert(1 <= start);
    assert(start <= stop);

    /* good guesses on how big things will get, ok if wrong */
    ui_factor_fit_length(r, 8*(stop - start + 1));
    ui_factor_fit_length(q, 4*(stop - start + 1));
    p_d = flint_mpz_fit_length(p, 2*(stop - start) + 20);
    s_d = flint_mpz_fit_length(s, 1*(stop - start) + 10);

    ui_factor_one(r);
    ui_factor_one(q);

    j = start;

    // q = j;
    ui_factor_push_ui_with_sieve(q, j, siever);

    // r = s = (2*j-1)*(6*j-1)*(6*j-5), s will be expanded form of r
    ui_factor_push_ui_with_sieve(r, 2*j-1, siever);
    ui_factor_push_ui_with_sieve(r, 6*j-1, siever);
    ui_factor_push_ui_with_sieve(r, 6*j-5, siever);

    s_d[0] = 6*j-1;
    s_n = flint_mpn_mul_11(s_d, s_d, 1, 6*j-5, 2*j-1);

    // a = 13591409 + 545140134*j
    umul_ppmm(a[1], a[0], j, 545140134);
    add_ssaaaa(a[1], a[0], a[1], a[0], 0, 13591409);

    // p = s*a
    if (a[1] == 0)
    {
        p_d[s_n] = mpn_mul_1(p_d, s_d, s_n, a[0]);
        p_n = s_n + (p_d[s_n] != 0);
    }
    else
    {
        out = mpn_mul_1(p_d + 1, s_d, s_n, a[1]);
        p_d[s_n + 1] = out;
        p_n = s_n + 1 + (out != 0);

        out = mpn_addmul_1(p_d, s_d, s_n, a[0]);
        if (out != 0)
            out = mpn_add_1(p_d + s_n, p_d + s_n, p_n - s_n, out);
        p_d[p_n] = out;
        p_n += (out != 0);
    }

    for (++j; j <= stop; ++j)
    {
        // q *= j
        ui_factor_push_ui_with_sieve(q, j, siever);

        // r, s *= (2*j-1)*(6*j-1)*(6*j-5)
        ui_factor_push_ui_with_sieve(r, 2*j-1, siever);
        ui_factor_push_ui_with_sieve(r, 6*j-5, siever);
        ui_factor_push_ui_with_sieve(r, 6*j-1, siever);
        s_n = flint_mpn_mul_111(s_d, s_d, s_n, 2*j-1, 6*j-5, 6*j-1);

        // p *= 10939058860032000*j^3
        p_n = flint_mpn_mul_111(p_d, p_d, p_n, j, j, j);
        if (FLINT_BITS == 64)
        {
            p_d[p_n] = mpn_mul_1(p_d, p_d, p_n, 10939058860032000); p_n += (p_d[p_n] != 0);
        }
        else
        {
            p_d[p_n] = mpn_mul_1(p_d, p_d, p_n, 296740963); p_n += (p_d[p_n] != 0);
            p_d[p_n] = mpn_mul_1(p_d, p_d, p_n, 36864000); p_n += (p_d[p_n] != 0);
        }

        // a = 13591409 + 545140134*j
        add_ssaaaa(a[1], a[0], a[1], a[0], 0, 545140134);

        // p +-= s*a
        assert(p_n >= s_n);
        if ((j & 1) ^ (start & 1))
        {
            if (a[1] != 0)
            {
                out = mpn_submul_1(p_d + 1, s_d, s_n, a[1]);
                if (p_n > s_n + 1 && out != 0)
                    out = mpn_sub_1(p_d + (s_n + 1), p_d + (s_n + 1), p_n - (s_n + 1), out);
                assert(out == 0);
                while (p_d[p_n - 1] == 0)
                {
                    assert(p_n > 0);
                    p_n--;
                }
            }

            assert(p_n >= s_n);

            out = mpn_submul_1(p_d, s_d, s_n, a[0]);
            if (p_n > s_n && out != 0)
                out = mpn_sub_1(p_d + s_n, p_d + s_n, p_n - s_n, out);
            assert(out == 0);
            while (p_d[p_n - 1] == 0)
            {
                assert(p_n > 0);
                p_n--;
            }
        }
        else
        {
            out = mpn_addmul_1(p_d, s_d, s_n, a[0]);
            if (p_n > s_n && out != 0)
                out = mpn_add_1(p_d + s_n, p_d + s_n, p_n - s_n, out);
            p_d[p_n] = out;
            p_n += (out != 0);

            if (a[1] != 0)
            {
                assert(p_n >= s_n + 1);                
                out = mpn_addmul_1(p_d + 1, s_d, s_n, a[1]);
                if (p_n > s_n + 1 && out != 0)
                    out = mpn_add_1(p_d + s_n + 1, p_d + s_n + 1, p_n - (s_n + 1), out);
                p_d[p_n] = out;
                p_n += (out != 0);
            }
        }

        s_d = flint_mpz_fit_length(s, s_n + 3);
        p_d = flint_mpz_fit_length(p, p_n + 4);
    }

    s->_mp_size = s_n;
    p->_mp_size = (start & 1) ? -p_n : p_n;

    ui_factor_canonicalise(q);
    ui_factor_pow_inplace(q, 3);
    ui_factor_mulpow_inplace(q, mult, stop - start + 1);

    ui_factor_canonicalise(r);

    assert(ui_factor_equal_mpz(r, s));
}

// {p1, r1, q1} = {p1*q2 + r1*p2, r1*r2, q1*q2} / GCD[r1, q2]
static void fold(
    mpz_t p1, ui_factor_t r1, ui_factor_t q1,
    mpz_t p2, ui_factor_t r2, ui_factor_t q2,
    bool needr,
    ui_factor_stack_t S)
{
    ulong q2e, r1e;
    mpz_ptr t, q2t, r1t;
    ui_factor_struct * s;

#if PROFILE_THIS
timeit_t timer;
timeit_ustart(timer);
#endif

    ui_factor_remove_gcd(r1, q2);

#if PROFILE_THIS
ulong ttt1 = timeit_uquery(timer);
#endif


    ui_factor_stack_fit_request_mpz(S, 3);
    ui_factor_stack_fit_request_factor(S, 1);

    // p1 = p1*q2 + r1*p2
    t = ui_factor_stack_take_top_mpz(S);
    q2t = ui_factor_stack_take_top_mpz(S);
    r1t = ui_factor_stack_take_top_mpz(S);
    q2e = ui_factor_get_mpz_2exp(q2t, q2, S);
    r1e = ui_factor_get_mpz_2exp(r1t, r1, S);

#if PROFILE_THIS
ulong ttt2 = timeit_uquery(timer);
#endif

    if (q2e >= r1e)
    {
        mpz_mul(t, p1, q2t);
        mpz_mul_2exp(t, t, q2e - r1e);
        mpz_addmul(t, p2, r1t);
        mpz_mul_2exp(t, t, r1e);
    }
    else
    {
        mpz_mul(t, p2, r1t);
        mpz_mul_2exp(t, t, r1e - q2e);
        mpz_addmul(t, p1, q2t);
        mpz_mul_2exp(t, t, q2e);
    }
    mpz_swap(p1, t);

#if PROFILE_THIS
ulong ttt3 = timeit_uquery(timer);
#endif

    // q1 = q1*q2
    s = ui_factor_stack_take_top_factor(S);
    ui_factor_mul(s, q1, q2);
    ui_factor_swap(q1, s);

    if (needr)
    {
        // r1 = r1*r2
        ui_factor_mul(s, r1, r2);
        ui_factor_swap(r1, s);
    }

    ui_factor_stack_give_back_factor(S, 1);
    ui_factor_stack_give_back_mpz(S, 3);

#if PROFILE_THIS
ulong ttt4 = timeit_uquery(timer);
timer_fold += ttt4;
timer_fold_expand += ttt2-ttt1;
timer_fold_factor += ttt1 + ttt4-ttt3;
timer_fold_fmpz += ttt3-ttt2;
#endif
}

// Set {p, r, q} = sum of terms in start..stop inclusive.
static void pi_sum_split(
    mpz_t p, ui_factor_t r, ui_factor_t q,
    ulong start, ulong stop, bool needr, const ui_factor_t mult,
    ui_factor_stack_t S)
{
    mpz_ptr p1, s;
    ui_factor_struct * r1, * q1;
    ulong diff, mid;

#if PROFILE_THIS
timeit_t timer;
#endif

    assert(start <= stop);

    ui_factor_stack_fit_request_mpz(S, 2);
    p1 = ui_factor_stack_take_top_mpz(S);

    ui_factor_stack_fit_request_factor(S, 2);
    r1 = ui_factor_stack_take_top_factor(S);
    q1 = ui_factor_stack_take_top_factor(S);

    diff = stop - start;
    if (diff > 120)
    {
        mid = diff/16*9 + start;
        pi_sum_split(p, r, q, start, mid, true, mult, S);
        pi_sum_split(p1, r1, q1, mid + 1, stop, true, mult, S);
        fold(p, r, q, p1, r1, q1, needr, S);
    }
    else if (diff > 60)
    {
        mpz_ptr t = ui_factor_stack_take_top_mpz(S);
        mid = diff/2 + start;
#if PROFILE_THIS
timeit_ustart(timer);
#endif
        pi_sum_basecase(p, t, r, q, start, mid, mult);
        pi_sum_basecase(p1, t, r1, q1, mid + 1, stop, mult);
#if PROFILE_THIS
timer_basecase += timeit_uquery(timer);
#endif
        fold(p, r, q, p1, r1, q1, needr, S);
        ui_factor_stack_give_back_mpz(S, 1);
    }
    else
    {
#if PROFILE_THIS
timeit_ustart(timer);
#endif
        pi_sum_basecase(p, p1, r, q, start, stop, mult);
#if PROFILE_THIS
timer_basecase += timeit_uquery(timer);
#endif
    }

    ui_factor_stack_give_back_factor(S, 2);
    ui_factor_stack_give_back_mpz(S, 1);
}


typedef struct {
    mpz_t p2;
    ui_factor_t r2;
    ui_factor_t q2;
    ui_factor_struct * mult;
    ui_factor_stack_t St;
    ulong start;
    ulong stop;
    ulong q2e;
    mpz_ptr p1q2;
    mpz_ptr p1;
} worker_arg;

void worker_proc(void * varg)
{
    worker_arg * arg = (worker_arg *) varg;
    pi_sum_split(arg->p2, arg->r2, arg->q2, arg->start, arg->stop, true, arg->mult, arg->St);
}

void worker_proc2(void * varg)
{
    mpz_ptr q2t;
    worker_arg * arg = (worker_arg *) varg;

    ui_factor_stack_fit_request_mpz(arg->St, 2);

    q2t = ui_factor_stack_take_top_mpz(arg->St);
    arg->q2e = ui_factor_get_mpz_2exp(q2t, arg->q2, arg->St);

    arg->p1q2 = ui_factor_stack_take_top_mpz(arg->St);
    mpz_mul(arg->p1q2, arg->p1, q2t);
}


ulong pi_sum(fmpz_t P, fmpz_t Q, ulong num_terms)
{
    ulong qe;
    ui_factor_t r1, q1, mult;
    ui_factor_stack_t St;
    mpz_ptr p1 = _fmpz_promote(P);
    mpz_ptr q = _fmpz_promote(Q);

    ui_factor_init(r1);
    ui_factor_init(q1);
    ui_factor_init(mult);
    ui_factor_stack_init(St);

    ui_factor_sieve_init(siever);
    ui_factor_sieve_build(siever, std::max(ulong(3*5*23*29), 6*num_terms-1));

    ui_factor_one(mult);
    ui_factor_push_factor(mult, 2, 15);
    ui_factor_push_factor(mult, 3, 2);
    ui_factor_push_factor(mult, 5, 3);
    ui_factor_push_factor(mult, 23, 3);
    ui_factor_push_factor(mult, 29, 3);

    if (global_thread_pool_initialized && num_terms > 20)
    {
        thread_pool_handle handles[1];
        slong num_handles = thread_pool_request(global_thread_pool, handles, 1);
        if (num_handles > 0)
        {
            ulong mid = num_terms/16*9;
            ulong r1e;
            mpz_ptr r1t, p2r1;
            ui_factor_struct * q1q2;
            worker_arg warg;

            assert(num_handles == 1);

            mpz_init(warg.p2);
            ui_factor_init(warg.r2);
            ui_factor_init(warg.q2);
            warg.mult = mult;
            warg.p1 = p1;
            warg.start = mid + 1;
            warg.stop = num_terms;
            ui_factor_stack_init(warg.St);

            /* calculate [1, mid] and [mid + 1, num_terms] */

            thread_pool_wake(global_thread_pool, handles[0], 1, &worker_proc, &warg);
            pi_sum_split(p1, r1, q1, 1, mid, true, mult, St);
            thread_pool_wait(global_thread_pool, handles[0]);

            /* join the two pieces */

            ui_factor_remove_gcd(r1, warg.q2);

            thread_pool_wake(global_thread_pool, handles[0], 1, &worker_proc2, &warg);

            ui_factor_stack_fit_request_factor(St, 1);
            ui_factor_stack_fit_request_mpz(St, 2);

            r1t = ui_factor_stack_take_top_mpz(St);
            r1e = ui_factor_get_mpz_2exp(r1t, r1, St);

            p2r1 = ui_factor_stack_take_top_mpz(St);
            mpz_mul(p2r1, warg.p2, r1t);

            q1q2 = ui_factor_stack_take_top_factor(St);
            ui_factor_mul(q1q2, q1, warg.q2);
            qe = ui_factor_get_mpz_2exp(q, q1q2, St);

            thread_pool_wait(global_thread_pool, handles[0]);

            if (warg.q2e >= r1e)
            {
                mpz_mul_2exp(p1, warg.p1q2, warg.q2e - r1e);
                mpz_add(p1, p1, p2r1);
                mpz_mul_2exp(p1, p1, r1e);
            }
            else
            {
                mpz_mul_2exp(p1, p2r1, r1e - warg.q2e);
                mpz_add(p1, p1, warg.p1q2);
                mpz_mul_2exp(p1, p1, warg.q2e);
            }

            thread_pool_give_back(global_thread_pool, handles[0]);

            ui_factor_stack_give_back_mpz(St, 2);
            ui_factor_stack_give_back_factor(St, 1);
            ui_factor_stack_give_back_mpz(warg.St, 2);

            ui_factor_clear(warg.r2);
            ui_factor_clear(warg.q2);
            ui_factor_stack_clear(warg.St);

            goto cleanup;
        }
    }

    /* serial */
    pi_sum_split(p1, r1, q1, 1, num_terms, false, mult, St);
#if PROFILE_THIS
timeit_t timer;
timeit_ustart(timer);
#endif
    qe = ui_factor_get_mpz_2exp(q, q1, St);
#if PROFILE_THIS
timer_final_expand = timeit_uquery(timer);
#endif

cleanup:

    ui_factor_sieve_clear(siever);

    ui_factor_clear(r1);
    ui_factor_clear(q1);
    ui_factor_clear(mult);
    ui_factor_stack_clear(St);

    _fmpz_demote_val(Q);
    _fmpz_demote_val(P);

    return qe;
}

/* timings in ms  (ratio against arb in parenthesis)

     digits   | arb_const_pi | gmp-chudnovsky |        here   |  here 2 threads
     ---------+--------------+----------------+---------------+-----------------
      100000  |           46 |      41 (1.12) |     26 (1.77) |      21 (2.19)
      200000  |          100 |      95 (1.01) |     63 (1.59) |      47 (2.12)
      400000  |          239 |     211 (1.06) |    146 (1.64) |     102 (2.34)
      800000  |          557 |     480 (1.11) |    342 (1.63) |     236 (2.36)
     1000000  |          751 |     647 (1.10) |    457 (1.64) |     314 (2.39)
     2000000  |         1747 |    1501 (1.12) |   1066 (1.64) |     764 (2.28)
     4000000  |         4205 |    3565 (1.14) |   2515 (1.67) |    1698 (2.47)
     8000000  |         9936 |    8486 (1.14) |   6052 (1.64) |    4054 (2.45)
    10000000  |        13077 |   10927 (1.17) |   8100 (1.61) |    5308 (2.46)
    20000000  |        30976 |   25370 (1.19) |  18800 (1.65) |   12179 (2.54)
*/

ex ncode_sPi(er e, slong prec)
{
//std::cout << "ncode_sPi: " << ex_tostring_full(e) << ", " << prec << std::endl;

    if (!eis_sym(e, gs.sym_sPi.get()))
        return ecopy(e);

    slong wp = prec + 3;
    ulong num_terms = ceil(prec * 0.021226729578153557) + 1;

    xfmpz_t p, q;
    xarb_t P, Q, U, T;

#if PROFILE_THIS
std::cout << "---------- num_terms: " << num_terms << " --------------" << std::endl;
timer_basecase = 0;
timer_fold = 0;
timer_float = 0;
timer_fold_expand = 0;
timer_fold_fmpz = 0;
timer_fold_factor = 0;
timeit_t timer;
timeit_ustart(timer);
#endif

    ulong qe = pi_sum(p.data, q.data, num_terms);

#if PROFILE_THIS
flint_bitcnt_t pbits = fmpz_bits(p.data);
flint_bitcnt_t qbits = fmpz_bits(q.data);
#endif

    /*
        we now have p/(q*2^qe) = sum from 1 to num_terms
        finish off the pi calculation with

                 q*2^qe * 640320/12
        -----------------------------------
        (p + 13591409*q*2^qe)*rsqrt(640320)
    */

#if PROFILE_THIS
ulong tt1 = timeit_uquery(timer);
#endif

    arb_set_round_fmpz(Q.data, q.data, wp);
    arb_mul_2exp_si(Q.data, Q.data, qe);
    arb_mul_ui(Q.data, Q.data, 640320/12, wp);
    fmpz_mul_ui(q.data, q.data, 13591409);
    fmpz_mul_2exp(q.data, q.data, qe);
    fmpz_add(q.data, q.data, p.data);
    arb_set_round_fmpz(P.data, q.data, wp);
    arb_rsqrt_ui(U.data, 640320, wp);
    arb_mul(T.data, P.data, U.data, wp);
    arb_div(U.data, Q.data, T.data, wp);

#if PROFILE_THIS
ulong tt2 = timeit_uquery(timer);
timer_float = tt2 - tt1;
timeit_stop(timer);
flint_printf("bits: %wu/%wu  exp: %wu\n", pbits, qbits, qe);
flint_printf("     new time: %wd ms\n", tt2/1000);
flint_printf("   float time: %wd ms\n", (tt2 - tt1)/1000);
flint_printf("fin expand me: %wd ms\n", timer_final_expand/1000);
flint_printf("     sum time: %wd ms\n", tt1/1000);
flint_printf("basecase time: +%wd ms\n", timer_basecase/1000);
flint_printf("    fold time: +%wd ms\n", timer_fold/1000);
flint_printf("  expand time: ++%wd ms\n", timer_fold_expand/1000);
flint_printf("    fmpz time: ++%wd ms\n", timer_fold_fmpz/1000);
flint_printf("  factor time: ++%wd ms\n", timer_fold_factor/1000);
#endif

//std::cout << "U: " << U.tostring() << std::endl;

#if PROFILE_THIS
    ex zzz = emake_real();
timeit_ustart(timer);
//m1 = GetMS();

    arb_const_pi(ereal_data(zzz), prec);
flint_printf("     arb time: %wd ms\n", timeit_uquery(timer)/1000);
//std::cout << "arb time: " << GetMS() - m1 << std::endl;
//std::cout << "z: " << ereal_number(zzz).tostring() << std::endl;

    eclear(zzz);
#endif

    return emake_real_move(U);
}

