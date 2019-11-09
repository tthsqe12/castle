#include <map>
#include "uex.h"
#include "serialize.h"


void swrite_byte(FILE * fp, uint8_t b)
{
    uint8_t buffer[8];
    buffer[0] = b;
    fwrite(buffer, 1, 1, fp);
}

int sread_byte(FILE * fp, uint8_t &b)
{
    uint8_t buffer[8];
    if (1 != fread(buffer, 1, 1, fp)) {return 1;}
    b = buffer[0];
    return 0;
}

void swrite_ulong_nocompress(FILE * fp, ulong d)
{
    uint8_t buffer[FLINT_BITS/8];
    for (int i = 0; i < FLINT_BITS/8; i++) {
        buffer[i] = (d >> (8*i)) & 255;
    }
    fwrite(buffer, 1, 8, fp);
}

int sread_ulong_nocompress(FILE * fp, ulong &d)
{
    uint8_t buffer[8];
    if (8 != fread(buffer, 1, 8, fp)) {return 1;}
    ulong a = 0;
    for (int i = 0; i < FLINT_BITS/8; i++) {
        a |= (((ulong)(buffer[i])) << (8*i));
    }
    d = a;
    return 0;
}

void swrite_ulong(FILE * fp, ulong a)
{
    uint8_t buffer[20];
    ulong u = a;
    size_t len = 0;
    do {
        uint8_t c = u & 127;
        u = u >> 7;
        buffer[len++] = c | (u == 0 ? 0 : 128);
    } while (u != 0);
    fwrite(buffer, 1, len, fp);
}

int sread_ulong(FILE * fp, ulong &a) {
    uint8_t c, buffer[8];
    size_t frs;
    if (1 != fread(buffer, 1, 1, fp)) {return 1;}
    c = buffer[0];
    ulong u = c & 127;
    int off = 7;
    while (c >= 128)
    {
        if (1 != fread(buffer, 1, 1, fp)) {return 1;}
        c = buffer[0];
        if (off >= FLINT_BITS || (off > FLINT_BITS - 7 && (((ulong)(c&127)) << (off - (FLINT_BITS - 7))) >= 128)) {return 1;}
        u |= ((ulong)(c&127)) << off;
        off += 7;
    }
    a = u;
    return 0;
}

void swrite_slong(FILE * fp, slong a)
{
    ulong u = a < 0 ? -1 - 2*a : 0 + 2*a;
    swrite_ulong(fp, u);
}

int sread_slong(FILE * fp, slong &a)
{
    ulong b;
    if (sread_ulong(fp, b)) {return 1;}
    a = (b & 1) ? -1-(b/2) : b/2;
    return 0;
}

void swrite_fmpz(FILE * fp, fmpz_t f)
{
    fmpz d = *f;
    if (!COEFF_IS_MPZ(d))
    {
        swrite_slong(fp, 2*d + 0);
    }
    else
    {
        __mpz_struct * x = COEFF_TO_PTR(d);
        slong a = x->_mp_size;
        swrite_slong(fp, 2*a + 1);
        mp_size_t size = std::abs(a);
        for (mp_size_t k = 0; k < size; k++)
        {
            swrite_ulong_nocompress(fp, x->_mp_d[k]);
        }
    }
}

int sread_fmpz(FILE * fp, fmpz_t f)
{
    slong b;
    if (sread_slong(fp, b)) {return 1;}
    if ((b & 1) == 0)
    {
        slong a = b/2;
        fmpz_set_si(f, a);
    }
    else
    {
        slong a = (b-1)/2;
        mp_size_t size = std::abs(a);
        __mpz_struct * x = _fmpz_promote_val(f);
        if (x->_mp_alloc < size)
        {
            mpz_realloc2(x, size * FLINT_BITS);
        }
        ulong * xl = (ulong *)(x->_mp_d);
        for (mp_size_t k = 0; k < size; k++)
        {
            if (sread_ulong_nocompress(fp, xl[k])) {return 1;}
        }
        x->_mp_size = a < 0 ? -size : size;
        _fmpz_demote_val(f);
    }
    return 0;
}

void swrite_arb(FILE * fp, xarb &x)
{
    xfmpz a, b, c, d;
    x.get_abcd(a, b, c, d);
    swrite_fmpz(fp, a.data);
    swrite_fmpz(fp, b.data);
    swrite_fmpz(fp, c.data);
    swrite_fmpz(fp, d.data);
}

int sread_arb(FILE * fp, xarb &x)
{
    xfmpz a, b, c, d;
    if (sread_fmpz(fp, a.data)) {return 1;}
    if (sread_fmpz(fp, b.data)) {return 1;}
    if (sread_fmpz(fp, c.data)) {return 1;}
    if (sread_fmpz(fp, d.data)) {return 1;}
    x.set_abcd(a.data, b.data, c.data, d.data);
    return 0;
}

void swrite_fmpq(FILE * fp, fmpq_t f)
{
    swrite_fmpz(fp, fmpq_numref(f));
    swrite_fmpz(fp, fmpq_denref(f));
}

int sread_fmpq(FILE * fp, fmpq_t f)
{
    if (sread_fmpz(fp, fmpq_numref(f))) {return 1;}
    return sread_fmpz(fp, fmpq_denref(f));
}

void swrite_string(FILE * fp, const std::string &s)
{
    swrite_ulong(fp, s.length());
    fwrite(s.c_str(), 1, s.length(), fp);
}

int sread_string(FILE * fp, std::string &s)
{
    size_t n;
    if (sread_ulong(fp, n)) {return 1;}
    s.clear();
    s.reserve(n);
    for (size_t i = 0; i < n; i++)
    {
        uint8_t buffer[8];
        if (1 != fread(buffer, 1, 1, fp)) {return 1;}
        s.push_back(buffer[0]);
    }
    return 0;
}

void swrite_double(FILE * fp, double x)
{
    uint8_t buffer[8];
    reinterpret_cast<double *>(buffer)[0] = x;
    fwrite(buffer, 1, 8, fp);
}

int sread_double(FILE * fp, double &x)
{
    uint8_t buffer[8];
    if (8 != fread(buffer, 1, 8, fp)) {return 1;}
    x = reinterpret_cast<double *>(buffer)[0];
    return 0;
}

ulong _swrite_ex(FILE * fp, std::map<er, ulong, ex_compare> &m, er e)
{
    auto search = m.find(e);

    if (search != m.end())
    {
        return search->second;
    }
    else
    {
        switch (etype(e))
        {
            case ETYPE_INT:
            {
                swrite_ulong(fp, ETYPE_INT);
                swrite_fmpz(fp, eint_data(e));
                break;
            }
            case ETYPE_RAT:
            {
                swrite_ulong(fp, ETYPE_RAT);
                swrite_fmpq(fp, erat_data(e));
                break;
            }
            case ETYPE_DOUBLE:
            {
                swrite_ulong(fp, ETYPE_DOUBLE);
                swrite_double(fp, edouble_number(e));
                break;
            }
            case ETYPE_REAL:
            {
                swrite_ulong(fp, ETYPE_REAL);
                swrite_arb(fp, ereal_number(e));
                break;
            }
            case ETYPE_CMPLX:
            {
                ulong re = _swrite_ex(fp, m, eto_cmplx(e)->real);
                ulong im = _swrite_ex(fp, m, eto_cmplx(e)->imag);
                swrite_ulong(fp, ETYPE_CMPLX);
                swrite_ulong(fp, re);
                swrite_ulong(fp, im);
                break;
            }
            case ETYPE_NAN:
            {
                ulong ch = _swrite_ex(fp, m, eto_nan(e)->child);
                swrite_ulong(fp, ETYPE_NAN);
                swrite_ulong(fp, ch);
                break;
            }
            case ETYPE_BIN:
            {
                swrite_ulong(fp, ETYPE_BIN);
                swrite_ulong(fp, ebin_size(e));
                fwrite(ebin_data(e), 1, ebin_size(e), fp);
                break;
            }
            case ETYPE_STR:
            {
                swrite_ulong(fp, ETYPE_STR);
                swrite_string(fp, eto_str(e)->string);
                break;
            }
            case ETYPE_NODE:
            {
                std::vector<ulong> args;
                args.reserve(1 + elength(e));
                for (size_t i = 0; i <= elength(e); i++)
                    args.push_back(_swrite_ex(fp, m, echild(e,i)));
                swrite_ulong(fp, ETYPE_NODE);
                swrite_ulong(fp, elength(e));
                for (size_t i = 0; i < args.size(); i++)
                    swrite_ulong(fp, args[i]);
                break;
            }
            case ETYPE_SYM:
            {
                ulong cntx = _swrite_ex(fp, m, esym_context(e));
                ulong name = _swrite_ex(fp, m, esym_name(e));
                swrite_ulong(fp, ETYPE_SYM);
                swrite_ulong(fp, cntx);
                swrite_ulong(fp, name);
                break;
            }
            default:
                assert(false);
        }

        ulong r = m.size();
        auto ret = m.insert(std::pair<er, ulong>(e,r));
        assert(ret.second);
        return r;
    }
}


void swrite_ex(FILE * fp, er e)
{
    std::map<er, ulong, ex_compare> m;
    ulong eind = _swrite_ex(fp, m, e);
    swrite_ulong(fp, ETYPE_RAW);
    swrite_ulong(fp, eind);
}

int sread_ex(FILE * fp, uex &e)
{
    std::vector<uex> v;

    while (true)
    {
        ulong t;
        if (sread_ulong(fp, t)) {return 1;}

        switch (t)
        {
            case ETYPE_INT:
            {
                xfmpz x;
                if (sread_fmpz(fp, x.data)) {return 1;}
                v.push_back(uex(emake_int_move(x)));
                break;
            }
            case ETYPE_RAT:
            {
                xfmpq x;
                if (sread_fmpq(fp, x.data)) {return 1;}
                v.push_back(uex(emake_rat_move(x)));
                break;
            }
            case ETYPE_DOUBLE:
            {
                double x;
                if (sread_double(fp, x)) {return 1;}
                v.push_back(uex(emake_double(x)));
                break;
            }
            case ETYPE_REAL:
            {
                xarb x;
                if (sread_arb(fp, x)) {return 1;}
                v.push_back(uex(emake_real_move(x)));
                break;
            }
            case ETYPE_CMPLX:
            {
                ulong re, im;
                if (sread_ulong(fp, re)) {return 1;}
                if (sread_ulong(fp, im)) {return 1;}
                if (re >= v.size() | im >= v.size()) {return 1;}
                v.push_back(uex(emake_cmplx(ecopy(v[re].get()), ecopy(v[im].get()))));
                break;
            }
            case ETYPE_NAN:
            {
                ulong ch;
                if (sread_ulong(fp, ch)) {return 1;}
                if (ch >= v.size()) {return 1;}
                v.push_back(uex(emake_nan(ecopy(v[ch].get()))));
                break;
            }
            case ETYPE_BIN:
            {
                ulong sz;
                if (sread_ulong(fp, sz)) {return 1;}
                ex x = emake_bin(sz);
                if (sz != fread(ebin_data(x), 1, sz, fp)) {eclear(x); return 1;}
                v.push_back(uex(x));
                break;
            }
            case ETYPE_STR:
            {
                std::string x;
                if (sread_string(fp, x)) {return 1;}
                v.push_back(uex(emake_str_move(x)));
                break;
            }
            case ETYPE_NODE:
            {
                ulong length;
                if (sread_ulong(fp, length)) {return 1;}
                ex_node f = enode_init(length);
                uint32_t cf = 0;
                for (size_t i = 0; i <= length; i++)
                {
                    ulong a;
                    if (sread_ulong(fp, a) || a >= v.size())
                    {
                        for (size_t j = 0; j < i; j++)
                        {
                            eclear(etox(f->child[j]));
                        }
                        ex_free(f);
                        return 1;
                    }
                    cf |= ebucketflags(v[a].get());
                    f->child[i] = ecopyr(v[a].get());
                }
                f->head.bucketflags = cf;
                v.push_back(uex(reinterpret_cast<ex>(f)));
                break;
            }
            case ETYPE_SYM:
            {
                //TODO handle attributes
                ulong cntx, name;
                if (sread_ulong(fp, cntx)) {return 1;}
                if (sread_ulong(fp, name)) {return 1;}
                if (cntx >= v.size() | name >= v.size()) {return 1;}
                if (!eis_str(v[cntx].get()) || !eis_str(v[name].get())) {return 1;}
                gs.live_contexts.insert(uex(v[cntx].copy()));
                auto ret = gs.live_symbols.insert(uex(emake_sym(v[cntx].copy(), v[name].copy())));
                v.push_back(uex((*ret.first).copy()));
                break;
            }
            case ETYPE_RAW:
            {
                ulong a;
                if (sread_ulong(fp, a)) {return 1;}                
                if (a >= v.size()) {return 1;}
                e.reset(ecopy(v[a].get()));
                return 0;
            }
            default:
            {
                return 1;
            }
        }
    }
}
