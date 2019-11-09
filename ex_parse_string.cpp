#include "ex_parse_string.h"
#include <fstream>

void ex_parse_file(std::vector<uex> &ans, std::ifstream & is, bool toplevel, syntax_report & sr)
{
    ans.clear();
    install_live_psymbols LPS;
    eparser P(toplevel);
    char c;

    while (is.get(c) && P.error == erNone)
    {
        // TODO probably should use unicode here
        P.handle_rawchar(c);
        if (P.error)
        {
            sr.handle_cstr_error(P.error, NULL, 0);
            break;
        }
    }
    if (sr.error)
    {
        sr.around.reset(gs.symsPi.copy());
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
        sr.around.reset(gs.symsPi.copy());
        return;
    }
}


void ex_parse_string(std::vector<uex> &ans, const char * ss, size_t sn, bool toplevel, syntax_report & sr)
{
//std::cout << "ex_parse_exbox " << ex_tostring_full(b) << std::endl;
    ans.clear();
    install_live_psymbols LPS;
    eparser P(toplevel);

    const unsigned char * s = reinterpret_cast<const unsigned char *>(ss);
    size_t si = 0;
    while (si < sn)
    {
        char16_t c;
        si += readonechar16(c, s + si);
        P.handle_rawchar(c);
        if (P.error)
        {
            sr.handle_cstr_error(P.error, ss, si);
            break;
        }
    }
    if (sr.error)
    {
        sr.around.reset(gs.symsPi.copy());
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
        sr.around.reset(gs.symsPi.copy());
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
            return ecopy(gs.syms$Failed.get());
        }
        pos++;
        s++;
    }
    P.handle_end();
    if (!P.have_one_ex())
    {
        error = 1; return ecopy(gs.syms$Failed.get());
    }

    assert(evalid_contentflags(P.estack[0].get()));

    return ecopy(P.estack[0].get());
}

ex ex_parse_string(const char* s)
{
    int error;
    size_t pos;
    return ex_parse_string(s, error, pos);
}
