#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <cstdio>


#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <string.h>
#include <pthread.h>

#include "flintarb_wrappers.h"

#include "types.h"
#include "ex.h"
#include "globalstate.h"
#include "ex_parse.h"
#include "ex_parse_string.h"
#include "ex_print.h"

#include "eval.h"
#include "hash.h"

#include "timing.h"
#include "digits.h"
#include "serialize.h"
#include "sudcode.h"


std::vector<void*> exs_in_use;
globalstate gs;

#define READ_END 0
#define WRITE_END 1
FILE * fp_out, * fp_in;
int fp_out_type;

#include <windows.h>

int main(int argc, char *argv[]) 
{
    HANDLE hPipeG2K, hPipeK2G; 
    LPTSTR lpvMessage=TEXT("Default message from client."); 
    BOOL   fSuccess = FALSE; 
    DWORD  cbRead, cbToWrite, cbWritten, dwMode; 
    LPTSTR lpszPipenameG2K = TEXT("\\\\.\\pipe\\G2Knamedpipe"); 
    LPTSTR lpszPipenameK2G = TEXT("\\\\.\\pipe\\K2Gnamedpipe"); 
 
    while (1)
    {
        if (!WaitNamedPipe(lpszPipenameG2K, 5000))
        { 
            printf("<kernel>: could not open pipeG2K: 5 second wait timed out."); 
            return -1;
        }

        hPipeG2K = CreateFile(lpszPipenameG2K, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hPipeG2K != INVALID_HANDLE_VALUE)
        {
            break;
        }
        if (GetLastError() != ERROR_PIPE_BUSY) 
        {
            printf("<kernel> could not open pipeG2K. GLE=%d\n", GetLastError()); 
            return -1;
        }
    }

    while (1)
    {
        if (!WaitNamedPipe(lpszPipenameK2G, 5000))
        { 
            printf("<kernel>: could not open pipeK2G: 5 second wait timed out."); 
            return -1;
        }

        hPipeK2G = CreateFile(lpszPipenameK2G, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hPipeK2G != INVALID_HANDLE_VALUE)
        {
            break;
        }
        if (GetLastError() != ERROR_PIPE_BUSY) 
        {
            printf("<kernel>: could not open pipeK2G. GLE=%d\n", GetLastError()); 
            return -1;
        }
    }

    int fd;
    fd = _open_osfhandle((intptr_t)hPipeG2K, _O_APPEND);
    if (fd == -1)
    {
        printf("<kernel>: _open_osfhandle failed, GLE = %d\n", GetLastError()); 
        return -1;
    }
    fp_in = _fdopen(fd,"r");
    if (fp_in == 0)
    {
        printf("<kernel>: _fdopen failed, GLE = %d\n", GetLastError());
        return -1;
    }

    fd = _open_osfhandle((intptr_t)hPipeK2G, _O_APPEND);
    if (fd == -1)
    {
        printf("<kernel>: _open_osfhandle failed, GLE = %d\n", GetLastError()); 
        return -1;
    }
    fp_out = _fdopen(fd,"a");
    if (fp_out == 0)
    {
        printf("<kernel>: _fdopen failed, GLE = %d\n", GetLastError());
        return -1;        
    }
 
    fp_out_type = FP_OUT_BINARY;


    gs.inc_$Line();
    uex m(emake_node(gs.sym_sInputNamePacket.copy(), gs.in_prompt_standardform()));
    swrite_byte(fp_out, CMD_EXPR);
    swrite_ex(fp_out, m.get());
    fflush(fp_out);
    gs.send_alive_symbols();

    while (1)
    {
        unsigned char buffer[8];
        if (1 != fread(buffer, 1, 1, fp_in))
        {
            std::cout << "<kernel>: could not read" << std::endl;
            break;
        }
//std::cout << "<kernel>: got " << int(buffer[0]) << std::endl;
        if (buffer[0] == CMD_EXPR)
        {
            uex e;
            int err = sread_ex(fp_in, e);
std::cout << "<kernel>: CMD_EXPR err: " << err << "  e: " << ex_tostring_full(e.get()) << std::endl;
            if (err != 0)
            {
                std::cerr << "<!kernel>: could not read CMD_EXPR - error " << err << std::endl;
            }
            else if (ehas_head_sym_length(e.get(), gs.sym_sEnterExpressionPacket.get(), 1))
            {
                e.reset(e.copychild(1));
                e.reset(topeval(e.release()));
                e.reset(emake_node(gs.sym_sReturnExpressionPacket.copy(), e.release()));
                swrite_byte(fp_out, CMD_EXPR);
                swrite_ex(fp_out, e.get());
                fflush(fp_out);
            }
            else if (ehas_head_sym_length(e.get(), gs.sym_sEnterTextPacket.get(), 1))
            {
                e.reset(e.copychild(1));
                std::vector<uex> v;
                syntax_report sr;
                ex_parse_exboxs(v, e.get(), true, sr);
                if (sr.have_error())
                {
		            _gen_message(gs.sym_sGeneral.get(), "sntx", "`1` near `2`.", sr.translate_error(), sr.near_error());
                    continue;
                }
//std::cout << "<kernel>: parsed as " << exvec_tostring_full(v) << std::endl;
                for (auto f = v.begin(); f != v.end(); ++f)
                {
                    esym_assign_dvalue(gs.sym_sIn.get(), emake_node(gs.sym_sIn.copy(), ecopy(gs.get_$Line())), f->copy());
                    f->set(topeval(f->release()));
                    esym_assign_dvalue(gs.sym_sOut.get(), emake_node(gs.sym_sOut.copy(), ecopy(gs.get_$Line())), f->copy());
                    if (!eis_sym(f->get(), gs.sym_sNull.get()))
                    {
                        m.reset(emake_node(gs.sym_sOutputNamePacket.copy(), gs.out_prompt_standardform()));
                        swrite_byte(fp_out, CMD_EXPR);
                        swrite_ex(fp_out, m.get());
                        fflush(fp_out);
                        f->setnz(ex_to_exbox_standard(f->get()));
                        f->set(emake_node(gs.sym_sReturnTextPacket.copy(), f->release()));
                        swrite_byte(fp_out, CMD_EXPR);
                        swrite_ex(fp_out, f->get());
                        fflush(fp_out);
                    }
                    gs.inc_$Line();
                }
                m.reset(emake_node(gs.sym_sInputNamePacket.copy(), gs.in_prompt_standardform()));
                swrite_byte(fp_out, CMD_EXPR);
                swrite_ex(fp_out, m.get());
                fflush(fp_out);
                gs.send_alive_symbols();
            }
        }
        else
        {
            std::cerr << "<kernel>: received unknown cmd " << (int)(buffer[0]) << std::endl;
        }
    }


    printf("<kernel quiting>\n");

    fclose(fp_in);
    fclose(fp_out);

    return 0; 
}
