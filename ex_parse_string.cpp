#include "ex_parse_string.h"
#include <fstream>

void ex_parse_file(std::vector<uex> &ans, std::ifstream & is, bool toplevel, syntax_report & sr)
{
    ans.clear();
    install_live_psymbols LPS;
    eparser P(toplevel);
    unsigned int unistate = 0;
    char rawc_;
    char16_t c = 0;

    while (is.get(rawc_) && P.error == erNone)
    {
        unsigned char rawc = rawc_;

        switch (unistate)
        {
            case 0:
            {
                c = rawc;
                if ((rawc & 0x80) == 0)
                {
                    break;
                }
                else if ((rawc & 0xE0) == 0xC0)
                {
                    unistate = 1;
                    continue;
                }
                else
                {
                    assert((rawc & 0xE0) == 0xE0);
                    unistate = 2;
                    continue;
                }
            }
            case 1:
            {
                assert((rawc & 0xC0) == 0x80);
                c = ((c & 0x1F) << 6) + ((rawc & 0x3F) << 0);
                break;
            }
            case 2:
            {
                assert((rawc & 0xC0) == 0x80);
                c = ((c & 0x0F) << 6) + ((rawc & 0x3F) << 0);
                unistate = 3;
                continue;
            }
            default:
            {
                assert(unistate == 3);
                assert((rawc & 0xC0) == 0x80);
                c = ((c) << 6) + ((rawc & 0x3F) << 0);
            }
        }

        unistate = 0;

        P.handle_rawchar(c);
        if (P.error)
        {
            sr.handle_cstr_error(P.error, NULL, 0);
            break;
        }
    }
    if (sr.error)
    {
        sr.around.reset(gs.sym_sPi.copy());
        return;
    }
    P.handle_end();
    if (P.have_one_ex() || (P.have_no_ex() && !P.out.empty()))
    {
        if (P.have_one_ex())
        {
            P.out.push_back(uex(P.estack[0].copy()));
        }
        std::swap(ans, P.out);
        gs.send_psymbols();
        return;
    }
    else
    {
        sr.error = erUnexpectedEnd;
        sr.around.reset(gs.sym_sPi.copy());
        return;
    }
}


void ex_parse_string(std::vector<uex> &ans, const char* s, size_t sn, bool toplevel, syntax_report & sr)
{
    ans.clear();
    install_live_psymbols LPS;
    eparser P(toplevel);

    size_t si = 0;
    while (si < sn)
    {
        char16_t c;
        si += readonechar16(c, s + si);
        P.handle_rawchar(c);
        if (P.error)
        {
            sr.handle_cstr_error(P.error, s, si);
            break;
        }
    }
    if (sr.error)
    {
        sr.around.reset(gs.sym_sPi.copy());
        return;
    }
    P.handle_end();
    if (P.have_one_ex() || (P.have_no_ex() && !P.out.empty()))
    {
        if (P.have_one_ex())
        {
            P.out.push_back(uex(P.estack[0].copy()));
        }
        std::swap(ans, P.out);
        gs.send_psymbols();
        return;
    }
    else
    {
        sr.error = erUnexpectedEnd;
        sr.around.reset(gs.sym_sPi.copy());
        return;
    }
}


ex ex_parse_string(const char* s, int& error, size_t& pos)
{
    error = erNone;
    pos = 0;
    eparser P;
    while (*s)
    {
        P.handle_rawchar(*s);
        if (P.error)
        {
            error = P.error;
            return ecopy(gs.sym_s$Failed.get());
        }
        pos++;
        s++;
    }
    P.handle_end();
    if (!P.have_one_ex())
    {
        error = 1; return ecopy(gs.sym_s$Failed.get());
    }

    assert(evalid_bucketflags(P.estack[0].get()));

    return ecopy(P.estack[0].get());
}

ex ex_parse_string(const char* s)
{
    int error;
    size_t pos;
    return ex_parse_string(s, error, pos);
}
