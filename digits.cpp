#include "ex_parse.h"
#include "digits.h"
#include "dtoa_milo.h"

std::string stdstring_tostring(ulong x)
{
    std::string s;
    while (x != 0)
    {
        s.push_back('0' + (x%10));
        x = x/10;
    }

    if (s.empty())
        s.push_back('0');
    else
        std::reverse(s.begin(), s.end());

    return s;
}

char* print_hex64short(char* s, uint64_t x) {
    bool got = false;
    for (int i = 15; i>=0; i--){
        unsigned int y = (x>>(4*i))&15;
        if (got || y!=0) {
            *s++ = (y < 10 ? '0'+y : 'a'+y-10);
            got = true;
        }
    }
    return s;
}

char* print_hex64(char* s, uint64_t x) {
    for (int i = 15; i>=0; i--){
        unsigned int y = (x>>(4*i))&15;
        *s++ = (y < 10 ? '0'+y : 'a'+y-10);
    }
    return s;
}

char* int_print(char* s, uint64_t x) {
    uint64_t r = x%10;
    uint64_t q = x/10;
    if (q!=0) {s = int_print(s, q);}
    *s++ = r + '0';
    return s; 
}

char* int_print(char* s, int64_t x) {
    if (x<0) {*s++='-';x=-x;}
    return int_print(s,(uint64_t)x);
}

char* int_print_full19(char* s, uint64_t x) {
    s+=19;
    uint64_t q, r;
    for (int64_t i=1; i<=19; i++) {
        r = x%10;
        x = x/10;
        s[-i]='0'+r;
    }
    assert(x==0);
    return s;
}


std::string hex_tostring(uint64_t x) {
    char buffer[20];
    char* end = print_hex64short(buffer, x);
    *end = 0;
    std::string s(buffer);
    return s;
}

std::string hex_tostring_full(uint64_t x) {
    char buffer[20];
    char* end = print_hex64(buffer, x);
    *end = 0;
    std::string s(buffer);
    return s;
}

std::string int_tostring(int64_t x) {
    char buffer[30];
    char* end = int_print(buffer, x);
    *end = 0;
    std::string s(buffer);
    return s;
}
std::string uint_tostring(uint64_t x) {
    char buffer[30];
    char* end = int_print(buffer, x);
    *end = 0;
    std::string s(buffer);
    return s;
}
std::string size_t_tostring(size_t x) {
    char buffer[30];
    char* end = int_print(buffer, uint64_t(x));
    *end = 0;
    std::string s(buffer);
    return s;
}
/*
    return Floor[a / Log2[radix]]
    3 word accuracy suffices by the continued fractions for 1/Log2[2], ..., 1/Log2[36]
    TODO: get rid of this. this is overkill.
*/
ulong fdiv_log2(ulong a, ulong radix)
{
    assert(radix >= 2);
    assert(radix <= 36);
    mp_limb_t t[3];
    mp_limb_t data[][3] =
    {
        {0x090a48ddb0de33c5ULL,0x043eaf7791f52142ULL,0xa1849cc1a9a9e94eULL},
        {0x0000000000000000ULL,0x0000000000000000ULL,0x8000000000000000ULL},
        {0x40584d3d310b8061ULL,0x33d522368f0d1d89ULL,0x6e40d1a4143dcb94ULL},
        {0x8a2cae211bb63e21ULL,0xff85a5c1b80aaa91ULL,0x6308c91b702a7cf4ULL},
        {0x1e1e26dce5b2b5b9ULL,0x9bd82cc11a7209d2ULL,0x5b3064eb3aa6d388ULL},
        {0x5555555555555555ULL,0x5555555555555555ULL,0x5555555555555555ULL},
        {0x0485246ed86f19e2ULL,0x021f57bbc8fa90a1ULL,0x50c24e60d4d4f4a7ULL},
        {0x13569862a1e8f9a4ULL,0x47c4acd605be48bcULL,0x4d104d427de7fbccULL},
        {0xffb081ec66f6c1e5ULL,0x70b466920e51e1f7ULL,0x4a00270775914e88ULL},
        {0xcada9779fa551f76ULL,0x7f122e2f4c79f9caULL,0x4768ce0d05818e12ULL},
        {0x39bd7ac4868ca6a3ULL,0x2bf75000cfb72251ULL,0x452e53e365907bdaULL},
        {0xde00f47ce216659bULL,0xc2d2e89586d2b763ULL,0x433cfffb4b5aae55ULL},
        {0x324183196c5d392dULL,0x37bbdca4fca609deULL,0x41867711b4f85355ULL},
        {0x0000000000000000ULL,0x0000000000000000ULL,0x4000000000000000ULL},
        {0x87b2e17d82daae00ULL,0xe1c51ddbeac65f02ULL,0x3ea16afd58b10966ULL},
        {0xe39f2dabe0641d60ULL,0x0da34544e21084a1ULL,0x3d64598d154dc4deULL},
        {0x4147294d6211b5ccULL,0x0369e97d641961e5ULL,0x3c43c23018bb5563ULL},
        {0xa52f16a2d22d9ebdULL,0x02cceaea8207233fULL,0x3b3b9a42873069c7ULL},
        {0xb48d1499334f53e9ULL,0x90409adae68a5d43ULL,0x3a4898f06cf41ac9ULL},
        {0x2ac45d8b01cf78b3ULL,0x76f62d7317e2d8bdULL,0x39680b13582e7c18ULL},
        {0xeb175f850572ac2bULL,0xb0f3e4b3bda6639cULL,0x3897b2b751ae561aULL},
        {0x600af2c1222df99dULL,0xcd9850af9a126d7eULL,0x37d5aed131f19c98ULL},
        {0xa02c269e9885c030ULL,0x19ea911b47868ec4ULL,0x372068d20a1ee5caULL},
        {0x1f67af426b836fc7ULL,0x1912e33748b4029fULL,0x3676867e5d60de29ULL},
        {0x5858c2f4904a1141ULL,0x56bf8fd285fc606bULL,0x35d6deeb388df86fULL},
        {0xdb53f138ad019f05ULL,0x37ac410062da9305ULL,0x354071d61c77fa2eULL},
        {0x8048d04059cce041ULL,0xf3315689e7fc958fULL,0x34b260c5671b18acULL},
        {0x5a108a5385ece85eULL,0x8d5dad3f1f35ccc3ULL,0x342be986572b45ccULL},
        {0x362dee09a17a047eULL,0xb55bac355a82ee98ULL,0x33ac61b998fbbdf2ULL},
        {0x3333333333333333ULL,0x3333333333333333ULL,0x3333333333333333ULL},
        {0x3ac9e5c14273aa66ULL,0xc220c028e9dbc15aULL,0x32bfd90114c12861ULL},
        {0x15fd5da403d1d56fULL,0xbed2f23982c11654ULL,0x3251dcf6169e45f2ULL},
        {0x545b9f421c3b12b5ULL,0x9a55d658e0cac095ULL,0x31e8d59f180dc630ULL},
        {0xc51657108ddb1f10ULL,0x7fc2d2e0dc055548ULL,0x3184648db8153e7aULL}
    };

    if (radix == 2)
    {
        return a;
    }
    else
    {
        return mpn_mul_1(t, data[radix - 3], 3, a);
    }
}

slong double_digitslen_in_base(ulong radix)
{
    assert(radix >= 2);
    if (radix == 2)
    {
        return 53;
    }
    else
    {
        return 2 + fdiv_log2(53, radix);
    }
}


std::string double_tostring_full(double y) {
    std::string s;
    int ye;
    uint64_t yl = static_cast<uint64_t>(ldexp(frexp(std::abs(y), &ye), 56));
    if (y<0) {
        s.append("-");
    }
    s.append("0x.");
    s.append(hex_tostring(yl));
    s.append("*^");
    s.append(int_tostring(ye));
    return s;
}

std::string double_tostring(double x)
{
    ulong Radix = 10;
    Radix = FLINT_MIN(Radix, 36);
    Radix = FLINT_MAX(Radix, 2);
//    slong digits_len = double_digitslen_in_base(Radix);

    assert(Radix == 10);
    char buffer[50];
    dtoa_milo(x, buffer);

    return std::string(buffer);
}







/*
    make_num either returns a number or $Failed
    the number could be not a number :-)
    it should probably set eparser::error in case of error but currently does not
*/
ex eparser::make_num()
{

//std::cout<< "make_num called:" << std::endl;
//std::cout<< "      base = " << num_base << std::endl;
//std::cout<< "  mantissa = " << num_mantissa << std::endl;
//std::cout<< " after dot = " << digits_after_dot << std::endl;
//std::cout<< "      prec = " << num_prec << std::endl;
//std::cout<< "       exp = " << num_exp << std::endl;

    int failed;

    xfmpz_t exp(UWORD(0));
    if (!num_exp.empty())
    {
        failed = fmpz_set_str(exp.data, num_exp.c_str(), 10);
        if (failed)
        {
            return ecopy(gs.sym_s$Failed.get());
        }
    }

    slong base = 10;
    if (!num_base.empty())
    {
        xfmpz_t b(UWORD(10));
        failed = fmpz_set_str(b.data, num_base.c_str(), 10);
        if (failed || !fmpz_fits_si(b.data))
            return gs.sym_s$Failed.copy();
        base = fmpz_get_si(b.data);
        if (base < 2 || base > 36)
            return gs.sym_s$Failed.copy();
    }

    slong prec = 0;
    if (!num_prec.empty())
    {
        xfmpz_t p(UWORD(0));
        failed = fmpz_set_str(p.data, num_prec.c_str(), 10);
        if (failed || !fmpz_fits_si(p.data))
            return gs.sym_s$Failed.copy();
        prec = fmpz_get_si(p.data);
    }

    slong sigfigs = 0;
    if (num_mantissa.size() == 0)
        return gs.sym_s$Failed.copy();
    if (num_mantissa[sigfigs] == '-')
        sigfigs++;
    while (sigfigs < num_mantissa.size() && num_mantissa[sigfigs] == '0')
        sigfigs++;
    sigfigs = num_mantissa.size() - sigfigs;
    assert(sigfigs >= 0);

    if (digits_after_dot >= 0)
    {
        fmpz_sub_ui(exp.data, exp.data, digits_after_dot);
        if (num_tick_count == 0)
        {
            /* real */
            xarb_t x, u;
            xfmpz_t f;
            failed = fmpz_set_str(f.data, num_mantissa.c_str(), base);
            if (failed)
                return ecopy(gs.sym_s$Failed.get());

            arb_set_fmpz(x.data, f.data);
            arb_add_error_2exp_si(x.data, -1);

            slong p = fmpz_bits(f.data);
            p = FLINT_MAX(0, p) + 100;
            xfmpz_t b(base);
            my_arb_fmpz_pow_fmpz(u.data, b.data, exp.data, p);
            arb_mul(x.data, x.data, u.data, p);

            if (sigfigs > double_digitslen_in_base(base))
            {
                return emake_real_move(x);
            }
            else
            {
                double d = arf_get_d(arb_midref(x.data), ARF_RND_NEAR);
                switch(std::fpclassify(d))
                {
                    case FP_INFINITE:
                    case FP_NAN:
                    case FP_SUBNORMAL:
                        return emake_real_move(x);
                    case FP_NORMAL:
                    case FP_ZERO:
                    default:
                        return emake_double(d);
                }
            }
        }
        else if (num_tick_count == 1)
        {
            /* real with specified precision */
            xarb_t x, u;
            xfmpz_t f;
            failed = fmpz_set_str(f.data, num_mantissa.c_str(), base);
            if (failed)
                return ecopy(gs.sym_s$Failed.get());
            if (fmpz_is_zero(f.data))
                return gs.const_double_zero.copy();

            if (prec <= 0)
            {
                mag_set_fmpz(arb_radref(x.data), f.data);
            }
            else
            {
                arf_set_fmpz(arb_midref(x.data), f.data);
                double t = 3.3219280948873623479*prec + 1;
                slong n = t;
                t -= n;
                mag_set_d_lower(arb_radref(x.data), pow(2.0, -t));
                mag_mul_2exp_si(arb_radref(x.data), arb_radref(x.data), -n);
                mag_mul_fmpz_lower(arb_radref(x.data), arb_radref(x.data), f.data);
                slong p = fmpz_bits(f.data) + n;
                p = FLINT_MAX(0, p) + 40;
                xfmpz_t b(base);
                my_arb_fmpz_pow_fmpz(u.data, b.data, exp.data, p);
                arb_mul(x.data, x.data, u.data, p);
            }
            return emake_real_move(x);
        }
        else if (num_tick_count == 2)
        {
            /* real with specified accuracy */
            xarb_t x, u;
            xfmpz_t f;
            failed = fmpz_set_str(f.data, num_mantissa.c_str(), base);
            if (failed)
                return ecopy(gs.sym_s$Failed.get());
            if (fmpz_is_zero(f.data))
                return gs.const_double_zero.copy();

            arf_set_fmpz(arb_midref(x.data), f.data);
            fmpz_add_si(f.data, exp.data, prec);
            if (!fmpz_fits_si(f.data))
                return gs.sym_s$Failed.copy();
            prec = fmpz_get_si(f.data);

            double t = 3.3219280948873623479*prec + 1;
            slong n = t;
            t -= n;
            mag_set_d_lower(arb_radref(x.data), pow(2.0, -t));
            mag_mul_2exp_si(arb_radref(x.data), arb_radref(x.data), -n);
            slong p = fmpz_bits(f.data) + n;
            p = FLINT_MAX(0, p) + 40;
            xfmpz_t b(base);
            my_arb_fmpz_pow_fmpz(u.data, b.data, exp.data, p);
            arb_mul(x.data, x.data, u.data, p);

            return emake_real_move(x);
        }
        else if (num_tick_count == 3)
        {
            /* rational */
            xfmpq_t x;
            failed = fmpz_set_str(fmpq_numref(x.data), num_mantissa.c_str(), base);
            if (failed)
            {
                return ecopy(gs.sym_s$Failed.get());
            }
            if (prec > 0)
            {
                xfmpz_t q, basepow(base);
                fmpz_pow_ui(basepow.data, basepow.data, prec);
                fmpz_tdiv_q(q.data, fmpq_numref(x.data), basepow.data);
                fmpz_sub(fmpq_numref(x.data), fmpq_numref(x.data), q.data);
                fmpz_sub_ui(fmpq_denref(x.data), basepow.data, 1);
                fmpq_canonicalise(x.data);
                fmpz_add_ui(exp.data, exp.data, prec);                
            }
            else
            {
                fmpz_set_ui(fmpq_denref(x.data), 1);
            }

            if (!fmpz_fits_si(exp.data))
            {
                return ecopy(gs.sym_s$Failed.get());
            }
            slong e = fmpz_get_si(exp.data);
            if (e > 0)
            {
                xfmpz_t basepow(base);
                fmpz_pow_ui(basepow.data, basepow.data, e);
                fmpq_mul_fmpz(x.data, x.data, basepow.data);
            }
            else if (e < 0)
            {
                xfmpz_t basepow(base);
                fmpz_pow_ui(basepow.data, basepow.data, -e);
                fmpq_div_fmpz(x.data, x.data, basepow.data);
            }
            return emake_rat_move(x);
        }
    }
    else
    {
        if (num_tick_count == 0)
        {
            /* integer */
            xfmpz_t x;
            failed = fmpz_set_str(x.data, num_mantissa.c_str(), base);
            if (failed)
            {
                return ecopy(gs.sym_s$Failed.get());
            }
            if (!fmpz_fits_si(exp.data))
            {
                return ecopy(gs.sym_s$Failed.get());
            }
            slong e = fmpz_get_si(exp.data);
            if (e > 0)
            {
                xfmpz_t basepow(base);
                fmpz_pow_ui(basepow.data, basepow.data, e);
                fmpz_mul(x.data, x.data, basepow.data);
                return emake_int_move(x);
            }
            else if (e < 0)
            {
                xfmpq_t y;
                xfmpz_t basepow(base);
                fmpz_swap(fmpq_numref(y.data), x.data);
                fmpz_pow_ui(fmpq_denref(y.data), basepow.data, -e);
                fmpq_canonicalise(y.data);
                return emake_rat_move(y);
            }
            else
            {
                return emake_int_move(x);
            }
        }
    }

    return gs.sym_s$Failed.copy();
}



ex eparser::make_hexnum1()
{
    int failed;

    xfmpz_t exp(UWORD(0));
    if (!num_exp.empty())
    {
        failed = fmpz_set_str(exp.data, num_exp.c_str(), 10);
        if (failed)
        {
            return ecopy(gs.sym_s$Failed.get());
        }
    }
    if (digits_after_dot > 0)
    {
        fmpz_sub_ui(exp.data, exp.data, 2*digits_after_dot);
        fmpz_sub_ui(exp.data, exp.data, 2*digits_after_dot);
    }

    xfmpz_t f;
    failed = fmpz_set_str(f.data, num_mantissa.c_str(), 16);
    if (failed)
    {
        return ecopy(gs.sym_s$Failed.get());
    }

    xarb_t x;
    arf_set_fmpz(arb_midref(x.data), f.data);
    arf_mul_2exp_fmpz(arb_midref(x.data), arb_midref(x.data), exp.data);

    double d = arf_get_d(arb_midref(x.data), ARF_RND_NEAR);
    switch(std::fpclassify(d))
    {
        case FP_INFINITE:
        case FP_NAN:
        case FP_SUBNORMAL:
            return ecopy(gs.sym_s$Failed.get());
        case FP_NORMAL:
        case FP_ZERO:
        default:
            return emake_double(d);
    }
}

ex eparser::make_hexnum2()
{
    int failed;

    xfmpz_t exp(UWORD(0));
    if (!num_exp.empty())
    {
        failed = fmpz_set_str(exp.data, num_exp.c_str(), 10);
        if (failed)
        {
            return ecopy(gs.sym_s$Failed.get());
        }
    }
    if (digits_after_dot > 0)
    {
        fmpz_sub_ui(exp.data, exp.data, 2*digits_after_dot);
        fmpz_sub_ui(exp.data, exp.data, 2*digits_after_dot);
    }

    xfmpz_t exp2(UWORD(0));
    if (!num_exp2.empty())
    {
        failed = fmpz_set_str(exp2.data, num_exp2.c_str(), 10);
        if (failed)
        {
            return ecopy(gs.sym_s$Failed.get());
        }
    }
    if (digits_after_dot2 > 0)
    {
        fmpz_sub_ui(exp2.data, exp2.data, 2*digits_after_dot2);
        fmpz_sub_ui(exp2.data, exp2.data, 2*digits_after_dot2);
    }

    xfmpz_t midz;
    failed = fmpz_set_str(midz.data, num_mantissa.c_str(), 16);
    if (failed)
    {
        return ecopy(gs.sym_s$Failed.get());
    }

    xfmpz_t radz;
    failed = fmpz_set_str(radz.data, num_mantissa2.c_str(), 16);
    if (failed)
    {
        return ecopy(gs.sym_s$Failed.get());
    }

    xarb_t x;
    x.set_abcd(midz.data, exp.data, radz.data, exp2.data);

    return emake_real_move(x);
}

void xarb_t::set_abcd(const fmpz_t a, const fmpz_t b, const fmpz_t c, const fmpz_t d)
{
    arb_init(data);
    arf_set_fmpz(arb_midref(data), a);
    arf_mul_2exp_fmpz(arb_midref(data), arb_midref(data), b);

    xfmpz_t cabs;
    fmpz_abs(cabs.data, c);
    ulong tzc = fmpz_val2(c);
    fmpz_tdiv_q_2exp(cabs.data, cabs.data, tzc);
    if (fmpz_abs_fits_ui(cabs.data))
    {
        mag_set_ui(arb_radref(data), fmpz_get_ui(cabs.data));
    }
    else
    {
        mag_set_fmpz(arb_radref(data), cabs.data);
    }
    mag_mul_2exp_fmpz(arb_radref(data), arb_radref(data), d);
    mag_mul_2exp_si(arb_radref(data), arb_radref(data), tzc);
}

void xarb_t::get_abcd(xfmpz_t &a, xfmpz_t &b, xfmpz_t &c, xfmpz_t &d)
{
    arf_get_fmpz_2exp(a.data, b.data, arb_midref(data));
    fmpz_set_ui(c.data, uint32_t(MAG_MAN(arb_radref(data))<<(32-MAG_BITS)));
    fmpz_set(d.data, MAG_EXPREF(arb_radref(data)));
    fmpz_sub_ui(d.data, d.data, 32);
}

std::string xarb_t::tostring_full() const
{
    std::string s;
    xfmpz_t e;

    if (ARF_SGNBIT(arb_midref(data)))
    {
        s.push_back('-');
    }
    s.append("0x.");
    mp_srcptr xptr;
    mp_size_t xn;
    ARF_GET_MPN_READONLY(xptr, xn, arb_midref(data));
    do {
        xn--;
        s.append(hex_tostring(xptr[xn]));
        if (xn > 0)
        {
//            s.push_back('_');
        }
    } while (xn > 0);
    s.append("*^");
    xfstr re(fmpz_get_str(NULL, 10, ARF_EXPREF(arb_midref(data))));
    s.append(re.data);

    assert(MAG_BITS <= 32);
    s.append("+/-");
    s.append("0.");
    s.append(hex_tostring(uint32_t(MAG_MAN(arb_radref(data))<<(32-MAG_BITS))));
    s.append("*^");
    xfstr me(fmpz_get_str(NULL, 10, MAG_EXPREF(arb_radref(data))));
    s.append(me.data);

    return s;
}

std::string xarb_t::tostring() const
{
    std::string s;

    ulong Radix = 10;
    Radix = FLINT_MIN(Radix, 36);
    Radix = FLINT_MAX(Radix, 2);
    xfmpz_t radix(Radix);
    slong p;

    xfmpz_t re;
    fmpz_set(re.data, ARF_EXPREF(arb_radref(data)));

//std::cout << "*************" << std::endl;
//printf("data: "); arb_printd(data, 100); printf("\n");

    fmpz_add_ui(re.data, re.data, UWORD(1));
    fmpz_neg(re.data, re.data);

    p = fmpz_bits(re.data) + 40;

    xarb_t log_two, log_radix, t;
    arb_const_log2(log_two.data, p);
    arb_log_fmpz(log_radix.data, radix.data, p);
    arb_div(t.data, log_two.data, log_radix.data, p);
    arb_mul_fmpz(t.data, t.data, re.data, p);

    xfmpz_t d;
    arf_get_fmpz(d.data, arb_midref(t.data), ARF_RND_CEIL);

    p = arb_rel_accuracy_bits(data);
    p = FLINT_MAX(0, p) + 40;

//std::cout << "p = " << p << std::endl;

    xarb_t u, v;
    my_arb_fmpz_pow_fmpz(v.data, radix.data, d.data, p);
    arb_mul(u.data, data, v.data, p);

//printf(" : "); arb_printn(data, p, 0); printf("\n");
//std::cout << "d = " << d.tostring() << std::endl;
//printf("u: "); arb_printn(u.data, p, 0); printf("\n");

    while (mag_cmp_2exp_si(arb_radref(u.data), -1) > 0)
    {
        fmpz_sub_ui(d.data, d.data, 1);
        arb_div_fmpz(u.data, u.data, radix.data, p);
//std::cout << "d = " << d.tostring() << std::endl;
//printf("u: "); arb_printn(u.data, p, 0); printf("\n");
    }

    assert(fmpz_cmp_si(&arb_midref(u.data)->exp, p) <= 0);

    xfmpz_t f;
    arf_get_fmpz(f.data, arb_midref(u.data), ARF_RND_NEAR);

//std::cout << "f: " << f.tostring() << std::endl;

//flint_printf("printing prec bits: %wd\n", prec_bits());

    if (!fmpz_equal_ui(radix.data, 10))
    {
        s.append(radix.tostring());
        s.append("^^");
    }

    if (fmpz_is_zero(f.data))
    {
        s.append("0.*^");
        fmpz_neg(d.data, d.data);
        s.append(d.tostring());
    }
    else
    {
        if (fmpz_sgn(f.data) < 0)
        {
            s.push_back('-');
            fmpz_neg(f.data, f.data);
        }

        xfstr digits(fmpz_get_str(NULL, Radix, f.data));
        slong digits_len = strlen(digits.data);

        /*
             123456789234523425452 * 10^-d
            1.23456789234523425452 * 10^(k - d)
             |-------- k --------|
        */

        slong i, k = digits_len - 1;

        if (fmpz_fits_si(d.data))
        {
            slong dd = fmpz_get_si(d.data);
            if (dd >= 0)
            {
                if (dd - digits_len > -7 && dd - digits_len < 7)
                {
                    k = dd;
                }
            }
        }

        xfmpz_t k_minus_d;
        fmpz_sub_si(k_minus_d.data, d.data, k);
        fmpz_neg(k_minus_d.data, k_minus_d.data);

        assert(k >= 0);
        if (k >= digits_len)
        {
            s.push_back('0');
            s.push_back('.');
            for (i = k - digits_len; i > 0; i--)
            {
                s.push_back('0');
            }
            for (i = 0; i < digits_len; i++)
            {
                s.push_back(digits.data[i]);
            }
        }
        else
        {
            for (i = 0; i < digits_len - k; i++)
            {
                s.push_back(digits.data[i]);
            }
            s.push_back('.');
            for (i = digits_len - k; i < digits_len; i++)
            {
                s.push_back(digits.data[i]);
            }
            
        }
        if (digits_len <= double_digitslen_in_base(Radix))
        {
            s.append("`");
            s.append(int_tostring(digits_len));
        }
        if (!fmpz_is_zero(k_minus_d.data))
        {
            s.append("*^");
            s.append(k_minus_d.tostring());
        }
    }
    return s;
}

void my_arb_fmpz_pow_fmpz(arb_t res, const fmpz_t base, const fmpz_t e, slong prec)
{
    slong bits = fmpz_bits(e);
    prec += 2 * bits;
    if (bits < 128)
    {
        arb_set_round_fmpz(res, base, prec);
        arb_pow_fmpz_binexp(res, res, e, prec);
    }
    else
    {
        arb_log_fmpz(res, base, prec);
        arb_mul_fmpz(res, res, e, prec);
        arb_exp(res, res, prec);
    }
}
