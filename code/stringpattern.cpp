#include "uex.h"
#include "ex_parse.h"
#include "ex_print.h"
#include "globalstate.h"
#include "eval.h"
#include "code.h"
#include "timing.h"

enum spmnemonic {
    spmnemonic_fail,
    spmnemonic_pass,
    spmnemonic_cmp,
    spmnemonic_test,
    spmnemonic_goto,
    spmnemonic_start,
    spmnemonic_capture,
    spmnemonic_captureconst,
    spmnemonic_stop,

    spmnemonic_next,
    spmnemonic_end,
    spmnemonic_goto1,
    spmnemonic_goto2,
    spmnemonic_goto3,
    spmnemonic_goton,
    spmnemonic_none
};

#define spflag_head 8
#define spflag_toplevel 16


#define print_DELAY 3000

#define UNDO_DOWN           0
#define UNDO_UP             1
#define UNDO_INC_I          2
#define UNDO_START          3
#define UNDO_STOP           4
#define UNDO_CAPTURE_WRITE  5
#define UNDO_CAPTURE_CMP    6
#define UNDO_GOTO           7

class spcallframe
{
public:
    std::vector<char16_t> estring;
    size_t i;

    void print() const
    {
        for (size_t j = 0 ; j< estring.size(); j++)
        {
            if (j == i)
                std::cout << "*";
            std::cout << char(estring[j]);
        }
        if (estring.size()== i)
            std::cout << "*";
        std::cout << std::endl;
    }
/*
    callframe(const callframe & other) :
        e(ecopy(other.e.get())),
        cur(other.cur.get() ? ecopy(other.cur.get()) : NULL),
        i(other.i),
        avail(other.avail)
    {}

    callframe(callframe && other) :
        e(other.e.release()),
        cur(other.cur.release()),
        i(other.i),
        avail(other.avail)
    {}

    callframe() :
        e(nullptr),
        cur(nullptr),
        i(0)
    {}
*/
};

void spcallframe_swap(spcallframe & A, spcallframe & B)
{
    assert(false);
}

bool spcallframe_sameq(const spcallframe & A, const spcallframe & B)
{
    if (A.i != B.i || A.estring.size() != B.estring.size())
        return false;

    for (size_t j = 0; j < A.estring.size(); j++)
    {
        if (A.estring[j] != B.estring[j])
            return false;
    }

    return true;
}

class spvarstate
{
public:
    slong state;
    std::vector <char16_t> val;

    void print() const
    {
        std::cout << "state(" << state << ")" << " <";
        for (size_t k = 0; k < val.size(); k++)
        {
            std::cout << val[k];
            if (k + 1 < val.size())
                std::cout << ", ";
        }
        std::cout << ">";
    }

    spvarstate() : state(0) {};
};

bool spvarstate_sameq(const spvarstate & A, const spvarstate & B)
{
    if (A.state != B.state)
        return false;
    if (A.val.size() != B.val.size())
        return false;
    for (size_t i = 0; i < A.val.size(); i++)
    {
        if (A.val[i] != B.val[i])
            return false;
    }
    return true;
}


const char* spmnemonic_tostring(spmnemonic mnemonic)
{
    switch (mnemonic)
    {
        case spmnemonic_fail:     return "fail     ";
        case spmnemonic_pass:     return "pass     ";
        case spmnemonic_cmp:      return "cmp      ";
        case spmnemonic_test:     return "test     ";
        case spmnemonic_goto:     return "goto     ";
        case spmnemonic_start:    return "start    ";
        case spmnemonic_capture:  return "capture  ";
        case spmnemonic_captureconst:  return "captureconst  ";
        case spmnemonic_stop:     return "stop     ";
        case spmnemonic_next:     return "next     ";
        case spmnemonic_end:      return "end      ";
        case spmnemonic_goto1:    return "goto1    ";
        case spmnemonic_goto2:    return "goto2    ";
        case spmnemonic_goto3:    return "goto3    ";
        case spmnemonic_none:     return "none     ";
        default:                 return "???????? ";
    }
}


void print_cspprogram(std::vector<int> P, int ip)
{
    int i=0, j, k, n;
    while (i < P.size())
    {
        if (i == ip)
            std::cout << "*";
        std::cout << i <<": ";
        switch (static_cast<spmnemonic>(P[i]))
        {
            case spmnemonic_fail:
                std::cout << "fail        <>" << std::endl;
                i+=1;
                break;
            case spmnemonic_pass:
                std::cout << "pass        <>" << std::endl;
                i+=1;
                break;
            case spmnemonic_cmp:
                std::cout << "cmp "<< char16_t(P[i+1])<<"    <"<<P[i+2]<<">" << std::endl;
                i+=3;
                break;
            case spmnemonic_test:
                std::cout << "test e"<< P[i+1]<<"   <"<<P[i+2]<<">" << std::endl;
                i+=3;
                break;
            case spmnemonic_start:
                std::cout << "start  v"<< P[i+1]<<"   <"<<P[i+2]<<">" << std::endl;
                i+=3;
                break;
            case spmnemonic_capture:
                std::cout << "capture v"<< P[i+1]<<"   <"<<P[i+2]<<">" << std::endl;
                i+=3;
                break;
            case spmnemonic_captureconst:
                std::cout << "captureconst v"<< P[i+1]<<" "<< char16_t(P[i+2]) <<"   <"<<P[i+3]<<">" << std::endl;
                i+=4;
                break;
            case spmnemonic_stop:
                std::cout << "stop   v"<< P[i+1]<<"   <"<<P[i+2]<<">" << std::endl;
                i+=3;
                break;
            case spmnemonic_next:
                std::cout << "next        <"<<P[i+1]<<">" << std::endl;
                i+=2;
                break;
            case spmnemonic_end:
                std::cout << "end         <"<<P[i+1]<<">" << std::endl;
                i+=2;
                break;
            case spmnemonic_goto1:
                std::cout << "goto1       <"<<P[i+1]<<">" << std::endl;
                i+=3;
                break;
            case spmnemonic_goto2:
                std::cout << "goto2       <"<<P[i+1]<<","<<P[i+2]<<">" << std::endl;
                i+=3;
                break;
            case spmnemonic_goto3:
                std::cout << "goto3       <"<<P[i+1]<<","<<P[i+2]<<","<<P[i+3]<<">" << std::endl;
                i+=4;
                break;
            case spmnemonic_goton:
                n = P[i+1];
                std::cout << "goton       <";
                for (k=0; k<n; k++) {
                    std::cout << P[i+2+k];
                    if (k+1<n)
                        std::cout << ",";
                }
                std::cout << ">" << std::endl;
                i+=2+n;
                break;
            case spmnemonic_none:
                std::cout << "none        <>";
                i+=1;
                break;
            case spmnemonic_goto:
                assert(false);
                break;
        }
    }
    assert(i == P.size());
}





class spinstruction {
public:
    spmnemonic mnemonic;
    size_t offset;
    er expr;
    int32_t char2;
    size_t idx, idx2;
    std::vector<spinstruction*> nexts;

    spinstruction(spmnemonic mne) {
        mnemonic = mne;
        expr = NULL;
    }
    spinstruction(spmnemonic mne, er e) {
        mnemonic = mne;
        expr = e;
    }
    spinstruction(spmnemonic mne, er e1, int32_t c2) {
        mnemonic = mne;
        expr = e1;
        char2 = c2;
    }
    spinstruction(spmnemonic mne, int32_t c2) {
        mnemonic = mne;
        expr = nullptr;
        char2 = c2;
    }
    void print()
    {
        std::cout << hex_tostring_full(uint64_t(this)) << ": <";
        for (size_t i = 0; i < nexts.size(); i++)
        {
            std::cout << hex_tostring_full(uint64_t(nexts[i]));
            if (i + 1 < nexts.size())
                std::cout << ", ";
        }
        std::cout << "> " << spmnemonic_tostring(mnemonic);
        std::cout << " expr: " << ex_tostring_full(expr) << std::endl;
        std::cout << " char2: " << char16_t(char2) << std::endl;
    }
};


class spprogram {
public:
    std::vector<spinstruction*> instructions;
    spinstruction* start;
    spinstruction* end;

    spprogram() {
        start = NULL;
        end = NULL;
    }

    ~spprogram() {
        for (size_t i = 0; i < instructions.size(); i++) {
            delete instructions[i];
        }
    }
    void print() {
        std::cout << "**********program***********" << std::endl;
        std::cout << "start: " << hex_tostring_full(uint64_t(start)) << std::endl;
        std::cout << "  end: " << hex_tostring_full(uint64_t(  end)) << std::endl;
        for (size_t i = 0; i < instructions.size(); i++) {
            instructions[i]->print();
        }
        std::cout << "****************************" << std::endl;
    }
};

class spstate_debugger {
public:
    std::vector<spvarstate> vars;
    spcallframe cf;
    spstate_debugger(const std::vector<spvarstate> &othervars, const spcallframe & othercf):
        vars(othervars),
        cf(othercf)
    {}
};

class spmachine {
public:
/* machine specification */

    size_t start_ip;
    size_t uselesstotal;
    std::vector<int32_t> code;
    std::vector<uex> symbols;
    std::vector<uex> constants;
    int32_t dummy_var0;
    int32_t dummy_var1;

/* running state */
    size_t ip;
    spcallframe * cf;
    std::vector<spvarstate> vars;

    slong calldepth;
    std::vector<spcallframe> callstack;


    void clear_string()
    {
        clear_undo();
        ip = start_ip;

        for (size_t i = 0; i < vars.size(); i++)
        {
            vars[i].state = 0;
            vars[i].val.clear();
        }

        callstack.resize(1);
        calldepth = 0;

        cf = &callstack[0];
        cf->i = 0;
        cf->estring.clear();
    }

    void push_char(char16_t c)
    {
        cf->estring.push_back(c);        
    }

    void push_string(const std::string & ss)
    {
        const unsigned char * s = reinterpret_cast<const unsigned char *>(ss.c_str());
        size_t sn = ss.size();
        size_t si = 0;
        while (si < sn)
        {
            char16_t c;
            si += readonechar16(c, s + si);
            push_char(c);
        }
    }



#ifndef NDEBUG
    std::vector<spstate_debugger> state_debug_stack;
#endif
    void debugger_push()
    {
#ifndef NDEBUG
        spstate_debugger sd(vars, callstack[calldepth]);
        state_debug_stack.push_back(sd);

/*
    std::cout << "----------push vars-----------" << std::endl;
    for (size_t i = 0; i < vars.size(); i++)
    {
        std::cout << gs.sym_tostring(symbols[i].get()) << ": ";
        vars[i].print();
        std::cout << std::endl;
    }
    std::cout << "----------push debug vars -----------" << std::endl;
    for (size_t i = 0; i < state_debug_stack.back().vars.size(); i++)
    {
        std::cout << gs.sym_tostring(symbols[i].get()) << ": ";
        state_debug_stack.back().vars[i].print();
        std::cout << std::endl;
    }
    std::cout << "----------push callstack----------" << std::endl;
    callstack[calldepth].print();
    std::cout << std::endl;
    std::cout << "----------push debug callstack----------" << std::endl;
    state_debug_stack.back().cf.print();
    std::cout << std::endl;
*/

#endif
    }
    void debugger_pop()
    {
#ifndef NDEBUG
/*
    std::cout << "----------pop vars-----------" << std::endl;
    for (size_t i = 0; i < vars.size(); i++)
    {
        std::cout << gs.sym_tostring(symbols[i].get()) << ": ";
        vars[i].print();
        std::cout << std::endl;
    }
    std::cout << "----------pop debug vars -----------" << std::endl;
    for (size_t i = 0; i < state_debug_stack.back().vars.size(); i++)
    {
        std::cout << gs.sym_tostring(symbols[i].get()) << ": ";
        state_debug_stack.back().vars[i].print();
        std::cout << std::endl;
    }
    std::cout << "----------pop callstack----------" << std::endl;
    callstack[calldepth].print();
    std::cout << std::endl;
    std::cout << "----------pop debug callstack----------" << std::endl;
    state_debug_stack.back().cf.print();
    std::cout << std::endl;
*/
        assert(state_debug_stack.back().vars.size() == vars.size());
        for (size_t i = 0; i < vars.size(); i++)
        {
            assert(spvarstate_sameq(state_debug_stack.back().vars.at(i), vars.at(i)));
        }
        assert(spcallframe_sameq(state_debug_stack.back().cf, callstack[calldepth]));
        state_debug_stack.pop_back();
#endif
    }


    std::vector<uint8_t> last_undo;
    std::vector<spcallframe> _undo_frame;
    std::vector<slong>  _undo_mark;
    std::vector<slong>  _undo_capture;
    std::vector<slong>  _undo_start;
    std::vector<slong>  _undo_stop;
    std::vector<slong>  _undo_slong;
    std::vector<size_t> _undo_goto;
    std::vector<uex>    _undo_cur;


    void set_code(spprogram&P);
    void set_pattern(er pat);
    void set_expr(er e);
    int execute();
    bool didpopstate();
    bool _replacevars(std::vector<uex>&out, er e);
    ex replacevars(ex E);


    void clear_undo()
    {
        last_undo.clear();
        _undo_frame.clear();
        _undo_mark.clear();
        _undo_capture.clear();
        _undo_start.clear();
        _undo_stop.clear();
        _undo_goto.clear();
        _undo_cur.clear();
        _undo_slong.clear();
    }

    bool do_inc_i()
    {
        if (cf->i < cf->estring.size())
        {
            debugger_push();
            cf->i++;
            last_undo.push_back(UNDO_INC_I);
            return true;
        }
        else
        {
            return false;
        }
    }
    void undo_inc_i()
    {
        cf->i--;
        debugger_pop();
    }

    void do_start(slong v)
    {
        _undo_start.push_back(v);
        _undo_start.push_back(vars[v].state);
        last_undo.push_back(UNDO_START);
        debugger_push();
        if (vars[v].state == 0)
        {
            assert(vars[v].val.empty());
            vars[v].state = -1;
        }
        else if (vars[v].state < 0)
        {
            assert(false && "variable start while initializing");
        }
        else
        {
            vars[v].state = 1;
        }
    }
    void undo_start()
    {
        slong state = _undo_start.back();
        _undo_start.pop_back();
        slong v = _undo_start.back();
        _undo_start.pop_back();
        vars[v].state = state;
        debugger_pop();
    }

    bool do_stop(slong v)
    {
        slong prev_state = vars[v].state;
        if (prev_state == 0)
        {
            assert(false && "variable stop while uninitialized");
        }
        else if (prev_state < 0)
        {
            debugger_push();
            vars[v].state = 1;
        }
        else
        {
            if (prev_state != vars[v].val.size() + 1)
            {
                return false;
            }

            debugger_push();
            vars[v].state = 1;
        }
        _undo_stop.push_back(v);
        _undo_stop.push_back(prev_state);
        last_undo.push_back(UNDO_STOP);
        return true;
    }
    void undo_stop()
    {
        slong state = _undo_stop.back();
        _undo_stop.pop_back();
        slong v = _undo_stop.back();
        _undo_stop.pop_back();
        vars[v].state = state;
        debugger_pop();
    }

    bool do_capture(slong v, int32_t e)
    {
        slong state = vars[v].state;
        if (state == 0)
        {
            assert(false && "variable capture while uninitialized");
            return false;
        }
        else if (state < 0)
        {
            vars[v].val.push_back(e);
            assert(vars[v].val.size() == -state);
            vars[v].state = state - 1;
            _undo_capture.push_back(v);
            last_undo.push_back(UNDO_CAPTURE_WRITE);
            return true;
        }
        else
        {
            if (state - 1 >= vars[v].val.size())
            {
                return false;
            }
            if (e != vars[v].val[state - 1])
            {
                return false;
            }
            vars[v].state = state + 1;
            _undo_capture.push_back(v);
            last_undo.push_back(UNDO_CAPTURE_CMP);
            return true;
        }
    }
    void undo_capture_WRITE()
    {
        slong v = _undo_capture.back();
        _undo_capture.pop_back();
        vars[v].state++;
        vars[v].val.pop_back();
    }
    void undo_capture_CMP()
    {
        slong v = _undo_capture.back();
        _undo_capture.pop_back();
        vars[v].state--;
    }

    void print_undo()
    {
        size_t undo_frame_size = _undo_frame.size();
        size_t undo_mark_size = _undo_mark.size();
        size_t undo_capture_size = _undo_capture.size();
        size_t undo_start_size = _undo_start.size();
        size_t undo_stop_size = _undo_stop.size();
        size_t undo_goto_size = _undo_goto.size();
        size_t undo_cur_size = _undo_cur.size();
        size_t undo_slong_size = _undo_slong.size();

        std::cout << "----------undo-----------" << std::endl;

        size_t last_undo_size = last_undo.size();
        while (last_undo_size-- > 0)
        {
            uint8_t type = last_undo[last_undo_size];
            switch (type)
            {
                case UNDO_DOWN:
                {
                    std::cout << "UNDO_DOWN" << std::endl;
                    break;
                }
                case UNDO_UP:
                {
                    std::cout << "UNDO_UP ";
                    _undo_frame.at(--undo_frame_size).print();
                    std::cout << std::endl;
                    break;
                }
                case UNDO_INC_I:
                {
                    std::cout << "UNDO_INC_I" << std::endl;
                    break;
                }
                case UNDO_START:
                {
                    slong v = _undo_start.at(--undo_start_size);
                    slong state = _undo_start.at(--undo_start_size);
                    std::cout << "UNDO_START: v = " << v << ", old state = " << state << std::endl;
                    break;
                }
                case UNDO_STOP:
                {
                    slong v = _undo_start.at(--undo_stop_size);
                    slong state = _undo_start.at(--undo_stop_size);
                    std::cout << "UNDO_STOP: v = " << v << ", old state = " << state << std::endl;
                    break;
                }
                case UNDO_CAPTURE_CMP:
                {
                    slong v = _undo_capture.at(--undo_capture_size);
                    std::cout << "UNDO_CAPTURE_CMP: v = " << v << std::endl;
                    break;
                }
                case UNDO_CAPTURE_WRITE:
                {
                    slong v = _undo_capture.at(--undo_capture_size);
                    std::cout << "UNDO_CAPTURE_WRITE: v = " << v << std::endl;
                    break;
                }
                case UNDO_GOTO:
                {
                    size_t ipp = _undo_goto.at(--undo_goto_size);
                    std::cout << "UNDO_GOTO: ip = " << ipp << std::endl;
                    break;
                }
                default:
                {
                    assert(false);
                }
            }
        }
    }

    void print_code()
    {
        std::cout << "----------code-----------" << std::endl;
        print_cspprogram(code, ip);
        std::cout << "--------constants--------" << std::endl;
        std::cout << "symbols.size(): " << symbols.size() << std::endl;
        for (size_t j = 0; j < symbols.size(); j++)
            std::cout << "v" << j << ": " << ex_tostring_full(symbols[j].get()) << std::endl;
        std::cout << "constants.size(): " << constants.size() << std::endl;
        for (size_t j = 0; j < constants.size(); j++)
            std::cout << "e" << j << ": " << ex_tostring_full(constants[j].get()) << std::endl;
    }

    void print_vars()
    {
        std::cout << "----------vars-----------" << std::endl;
        for (size_t i = 0; i < vars.size(); i++)
        {
            std::cout << ex_tostring_full(symbols[i].get()) << ": ";
            vars[i].print();
            std::cout << std::endl;
        }
        std::cout << "dummy_var0 = " << dummy_var0 << std::endl;
        std::cout << "dummy_var1 = " << dummy_var1 << std::endl;
    }

    void print_framestack()
    {
        std::cout << "----------callstack----------" << std::endl;
        for (slong k = 0; k <= calldepth; k++)
        {
            callstack[k].print();
            std::cout << std::endl;
        }
    }

    void print_frame()
    {
        std::cout << "----------currentframe----------" << std::endl;
        cf->print();
        std::cout << std::endl;
    }

};

bool spmachine::didpopstate()
{
    size_t last_undo_size = last_undo.size();
    while (last_undo_size-- > 0)
    {
//std::cout << "last_undo_size: " << last_undo_size << std::endl;
        uint8_t type = last_undo[last_undo_size];
        switch (type)
        {
            case UNDO_INC_I: //printf("UNDO_INC_I\n");
            {
                undo_inc_i();
                break;
            }
            case UNDO_START: //printf("UNDO_START\n");
            {
                undo_start();
                break;
            }
            case UNDO_STOP: //printf("UNDO_STOP\n");
            {
                undo_stop();
                break;
            }
            case UNDO_CAPTURE_CMP: //printf("UNDO_CAPTURE_CMP\n");
            {
                undo_capture_CMP();
                break;
            }
            case UNDO_CAPTURE_WRITE: //printf("UNDO_CAPTURE_WRITE\n");
            {
                undo_capture_WRITE();
                break;
            }
            case UNDO_GOTO: //printf("UNDO_GOTO\n");
            {
                ip = _undo_goto.back();
                _undo_goto.pop_back();
                last_undo.pop_back();
                return true;
            }
            default:
            {
                assert(false);
            }
        }
        last_undo.resize(last_undo_size);
/*
SleepUS(print_DELAY/5);
std::cout << std::endl;
print_undo();
print_code();
print_vars();
print_framestack();
std::cout << "-------------------------" << std::endl;
*/
    }

//std::cout << "returning false" << std::endl;
    return false;
}


int spmachine::execute()
{
//printf("starting exectute %d\n", ip);

    while (1)
    {
/*
SleepUS(print_DELAY);
std::cout << std::endl;
print_undo();
print_code();
print_vars();
print_framestack();
std::cout << "-------------------------" << std::endl;
*/
        int32_t instr = code[ip];

        switch (instr)
        {
            case spmnemonic_pass:
//printf("pass!!!!!!!!!!!!!!!!!\n");
            {
/*
std::cout << "execute returning 1" << std::endl;
SleepUS(print_DELAY);
*/
                return 1;
            }
            case spmnemonic_fail: fail:
//printf("fail\n");
            {
                if (!didpopstate())
                {
/*
std::cout << "execute returning 0" << std::endl;
SleepUS(print_DELAY);
*/
                    return 0;
                }
                break;
            }
            case spmnemonic_goto1:
//printf("goto1\n");
            {
                ip = code[ip + 1];
                break;
            }
            case spmnemonic_goto2:
//printf("goto2\n");
            {
                last_undo.push_back(UNDO_GOTO);
                _undo_goto.push_back(code[ip + 2]);
                ip = code[ip + 1];
                break;
            }
            case spmnemonic_goto3:
//printf("goto3\n");
            {
                last_undo.push_back(UNDO_GOTO);
                _undo_goto.push_back(code[ip + 3]);
                last_undo.push_back(UNDO_GOTO);
                _undo_goto.push_back(code[ip + 2]);
                ip = code[ip + 1];
                break;
            }
            case spmnemonic_goton:
//printf("goton\n");
            {
                int n = code[ip + 1];
                assert(n >= 1);
                while (--n > 0)
                {
                    last_undo.push_back(UNDO_GOTO);
                    _undo_goto.push_back(code[ip + 2 + n]);
                }
                ip = code[ip + 2];
                break;
            }
            case spmnemonic_next:
//printf("next\n");
            {
                if (!do_inc_i())
                    goto fail;
                ip = code[ip + 1];
                break;
            }

            case spmnemonic_end:
//printf("end\n");
            {
                if (cf->i != cf->estring.size())
                    goto fail;
                ip = code[ip + 1];
                break;
            }

            case spmnemonic_cmp:
//printf("cmp\n");
            {
                if (cf->i == 0 || cf->estring[cf->i - 1] != code[ip + 1])
                    goto fail;
                ip = code[ip + 2];
                break;
            }
            case spmnemonic_test:
//printf("test\n");
            {
                assert(false && "not implemented");
                ip = code[ip + 2];
                break;
            }
            case spmnemonic_start:
//printf("start\n");
            {
                do_start(code[ip + 1]);
                ip = code[ip + 2];
                break;
            }
            case spmnemonic_stop:
//printf("stop\n");
            {
                if (!do_stop(code[ip + 1]))
                    goto fail;
                ip = code[ip + 2];
                break;
            }
            case spmnemonic_capture:
//printf("append %d\n",code[ip+1]);
            {
                if (cf->i == 0)
                    goto fail;
                if (!do_capture(code[ip + 1], cf->estring[cf->i - 1]))
                    goto fail;
                ip = code[ip + 2];
                break;    
            }
            case spmnemonic_captureconst:
//printf("append %d e%d\n",code[ip+1],code[ip+2]);
            {
               if (!do_capture(code[ip + 1], code[ip + 2]))
                    goto fail;
                ip = code[ip + 3];
                break;    
            }
            default:
//printf("default\n");
            {
                assert(false && "unknown instruction type");
            }
        }
    }
}




void spmake_prog(spprogram&P, er e, uint32_t flags, std::vector<er>&cmps, std::vector<er>&appends);

void spmake_prog_pattern(spprogram&P, er e, uint32_t flags, std::vector<er>&cmps, std::vector<er>&appends);

void spmake_prog_next(spprogram&P, uint32_t flags)
{
    spinstruction * in, * prev, * start;
    /* start */
    P.start = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    start = in;
    /* next */
        in = new spinstruction(spmnemonic_next);
        P.instructions.push_back(in);
        start->nexts.push_back(in);
        prev = in;
    /* end */
    P.end = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    return;
}

void spmake_prog_default(spprogram&P, er e, uint32_t flags, std::vector<er>&cmps, std::vector<er>&appends)
{
std::cout << "pmake_prog_default(" << flags << "): " << ex_tostring_full(e) << std::endl;

    assert(false);
}

void spmake_prog_blank0(spprogram&P, er e, uint32_t flags, std::vector<er>&cmps, std::vector<er>&appends)
{
    spinstruction * in, * prev;
    /*  */
    P.start = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev = in;
    /* next */
    spprogram Q;
    spmake_prog_next(Q, flags);
    for (size_t j = 0; j < Q.instructions.size(); j++)
        P.instructions.push_back(Q.instructions[j]);
    prev->nexts.push_back(Q.start);
    prev = Q.end;
    Q.instructions.clear();
    /* cmps */
    for (size_t i = 0; i < cmps.size(); i++) {
        in = new spinstruction(spmnemonic_cmp, cmps[i]);
        P.instructions.push_back(in);
        prev->nexts.push_back(in);
        prev = in;            
    }
    /* appends */
    for (size_t i = 0; i < appends.size(); i++) {
        in = new spinstruction(spmnemonic_capture, appends[i]);
        P.instructions.push_back(in);
        prev->nexts.push_back(in);
        prev = in;            
    }
    /*  */
    P.end = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    return;
}

void spmake_prog_blanksequence0(spprogram&P, er e, uint32_t flags, std::vector<er>&cmps, std::vector<er>&appends)
{
    spinstruction * in, * prev;
    /*  */
    P.start = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev = in;
    /* next */
    spprogram Q;
    spmake_prog_next(Q, flags);
    for (size_t j = 0; j < Q.instructions.size(); j++)
        P.instructions.push_back(Q.instructions[j]);
    prev->nexts.push_back(Q.start);
    prev = Q.end;
    Q.instructions.clear();
    /* cmps */
    for (size_t i = 0; i < cmps.size(); i++) {
        in = new spinstruction(spmnemonic_cmp, cmps[i]);
        P.instructions.push_back(in);
        prev->nexts.push_back(in);
        prev = in;            
    }
    /* appends */
    for (size_t i = 0; i < appends.size(); i++) {
        in = new spinstruction(spmnemonic_capture, appends[i]);
        P.instructions.push_back(in);
        prev->nexts.push_back(in);
        prev = in;            
    }
    /* goto2 */
    in = new spinstruction(spmnemonic_goto2);
    in->nexts.push_back(P.start);
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    prev = in;
    /*  */
    P.end = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    return;
}

void spmake_prog_blanknullsequence0(spprogram&P, er e, uint32_t flags, std::vector<er>&cmps, std::vector<er>&appends)
{
    spinstruction * in, * prev, * Qstart;
    /*  */
    P.start = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev = in;
    /* next */
    spprogram Q;
    spmake_prog_next(Q, flags);
    for (size_t j = 0; j < Q.instructions.size(); j++)
        P.instructions.push_back(Q.instructions[j]);
    Qstart = Q.start;
    prev->nexts.push_back(Q.start);
    prev = Q.end;
    Q.instructions.clear();
    /* cmps */
    for (size_t i = 0; i < cmps.size(); i++) {
        in = new spinstruction(spmnemonic_cmp, cmps[i]);
        P.instructions.push_back(in);
        prev->nexts.push_back(in);
        prev = in;            
    }
    /* appends */
    for (size_t i = 0; i < appends.size(); i++) {
        in = new spinstruction(spmnemonic_capture, appends[i]);
        P.instructions.push_back(in);
        prev->nexts.push_back(in);
        prev = in;            
    }
    /* goto2 */
    in = new spinstruction(spmnemonic_goto2);
    in->nexts.push_back(Qstart);
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    prev = in;
    /*  */
    P.end = in = new spinstruction(spmnemonic_goto);
    P.start->nexts.push_back(in);
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    return;
}

void spmake_prog_pattern(spprogram&P, er e, uint32_t flags, std::vector<er>&cmps, std::vector<er>&appends)
{
//std::cout << "pmake_prog_pattern(" << flags << "): " << ex_tostring_full(e) << std::endl;

    spinstruction * in, * prev;    
    bool newok = (std::find(appends.begin(), appends.end(), echild(e,1)) == appends.end());
    /*  */
    P.start = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev = in;
    /* start sym */
    if (newok)
    {
        appends.push_back(echild(e,1));
        in = new spinstruction(spmnemonic_start, echild(e,1));
        P.instructions.push_back(in);
        prev->nexts.push_back(in);
        prev = in;
    }
    /* e[2] */
    spprogram Q;
    spmake_prog(Q, echild(e,2), flags, cmps, appends);
    for (size_t j = 0; j < Q.instructions.size(); j++) {
        P.instructions.push_back(Q.instructions[j]);
    }
    prev->nexts.push_back(Q.start);
    prev = Q.end;
    Q.instructions.clear();
    /* stop sym */
    if (newok)
    {
        appends.pop_back();
        in = new spinstruction(spmnemonic_stop, echild(e,1));
        P.instructions.push_back(in);
        prev->nexts.push_back(in);
        prev = in;
    }
    /*  */
    P.end = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    return;
}

void spmake_prog_stringexpression(spprogram&P, er e, uint32_t flags, std::vector<er>&cmps, std::vector<er>&appends)
{
    size_t n = elength(e);
    spinstruction * in, * prev;
    /* start */
    P.instructions.clear();
    P.start = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev = in;
    /* p[i] */
    for (size_t i = 1; i <= n; i++)
    {
        spprogram Q;
        spmake_prog(Q, echild(e,i), flags, cmps, appends);
        for (size_t j = 0; j < Q.instructions.size(); j++)
        {
            P.instructions.push_back(Q.instructions[j]);
        }
        prev->nexts.push_back(Q.start);
        prev = Q.end;
        Q.instructions.clear();
    }
    /* end */
    P.end = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    return;

}

void spmake_prog_alternatives(spprogram&P, er e, uint32_t flags, std::vector<er>&cmps, std::vector<er>&appends)
{
    size_t n = elength(e);
    spinstruction * in, * prev;
    /*  */
    P.start = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    /*  */
    P.end = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    /* p[i] */
    for (size_t i = 1; i <= n; i++)
    {
        spprogram Q;
        spmake_prog(Q, echild(e,i), flags, cmps, appends);
        for (size_t j = 0; j < Q.instructions.size(); j++)
        {
            P.instructions.push_back(Q.instructions[j]);
        }
        P.start->nexts.push_back(Q.start);
        Q.end->nexts.push_back(P.end);
        Q.instructions.clear();
    }
    return;
}

void spmake_prog_repeated(spprogram&P, er e, uint32_t flags, std::vector<er>&cmps, std::vector<er>&appends)
{
    spinstruction * in, * prev;    
    /*  */
    P.start = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev = in;
    /* e[1] */
    spprogram Q;
    spmake_prog(Q, echild(e,1), flags, cmps, appends);
    for (size_t j = 0; j < Q.instructions.size(); j++)
    {
        P.instructions.push_back(Q.instructions[j]);
    }
    prev->nexts.push_back(Q.start);
    prev = Q.end;
    /* goto 2 */
    in = new spinstruction(spmnemonic_goto);
    in->nexts.push_back(Q.start);
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    prev = in;
    Q.instructions.clear();
    /*  */
    P.end = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    return;
}

void spmake_prog_repeatednull(spprogram&P, er e, uint32_t flags, std::vector<er>&cmps, std::vector<er>&appends)
{
    spinstruction * in, * prev;    
    /*  */
    P.start = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev = in;
    /* e[1] */
    spprogram Q;
    spmake_prog(Q, echild(e,1), flags, cmps, appends);
    for (size_t j = 0; j < Q.instructions.size(); j++)
    {
        P.instructions.push_back(Q.instructions[j]);
    }
    prev->nexts.push_back(Q.start);
    prev = Q.end;
    Q.instructions.clear();
    /* goto 2 */
    in = new spinstruction(spmnemonic_goto);
    in->nexts.push_back(P.start);
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    prev = in;
    /*  */
    P.end = in = new spinstruction(spmnemonic_goto);
    P.start->nexts.push_back(in);
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    return;            
}

void spmake_prog_condition(spprogram&P, er e, uint32_t flags, std::vector<er>&cmps, std::vector<er>&appends)
{
    spinstruction * in, * prev;    
    /*  */
    P.start = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev = in;
    /* e[1] */
    spprogram Q;
    spmake_prog(Q, echild(e,1), flags, cmps, appends);
    for (size_t j = 0; j < Q.instructions.size(); j++)
    {
        P.instructions.push_back(Q.instructions[j]);
    }
    prev->nexts.push_back(Q.start);
    prev = Q.end;
    Q.instructions.clear();
    /* test e[2] */
    in = new spinstruction(spmnemonic_test, echild(e,2));
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    prev = in;
    /*  */
    P.end = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    return;
}



void spmake_prog(spprogram&P, er e, uint32_t flags, std::vector<er>&cmps, std::vector<er>&appends)
{
//std::cout << "pmake_prog(" << flags << "): " << ex_tostring_full(e) << std::endl;

    P.instructions.clear();

    spinstruction * in, * prev;
    if (eis_str(e))
    {
        /* start */
        P.start = in = new spinstruction(spmnemonic_goto);
        P.instructions.push_back(in);
        prev = in;
        for (size_t i = 0; i < estr_string(e).size(); i++)
        {
            /* next */
            spprogram Q;
            spmake_prog_next(Q, flags);
            for (size_t j = 0; j < Q.instructions.size(); j++)
                P.instructions.push_back(Q.instructions[j]);
            prev->nexts.push_back(Q.start);
            prev = Q.end;
            Q.instructions.clear();
            /* cmp */
            in = new spinstruction(spmnemonic_cmp, estr_string(e)[i]);
            P.instructions.push_back(in);
            prev->nexts.push_back(in);
            prev = in;
        }
        /* cmps */
        for (size_t i = 0; i < cmps.size(); i++)
        {
            in = new spinstruction(spmnemonic_cmp, cmps[i]);
            P.instructions.push_back(in);
            prev->nexts.push_back(in);
            prev = in;            
        }
        /* appends */
        for (size_t i = 0; i < appends.size(); i++)
        {
            in = new spinstruction(spmnemonic_capture, appends[i]);
            P.instructions.push_back(in);
            prev->nexts.push_back(in);
            prev = in;            
        }
        /* end */
        P.end = in = new spinstruction(spmnemonic_goto);
        P.instructions.push_back(in);
        prev->nexts.push_back(in);
        return;
    }

    assert(eis_node(e));
    size_t n = elength(e);
    er ch = echild(e,0);
    if (eis_sym(ch))
    {
        if (ch == gs.sym_sBlank.get() && n == 0)
        {
            spmake_prog_blank0(P, e, flags, cmps, appends);
            return;
        }
        else if (ch == gs.sym_sBlankSequence.get() && n == 0)
        {
            spmake_prog_blanksequence0(P, e, flags, cmps, appends);
            return;
        }
        else if (ch == gs.sym_sBlankNullSequence.get() && n == 0)
        {
            spmake_prog_blanknullsequence0(P, e, flags, cmps, appends);
            return;
        }
        else if (ch == gs.sym_sPattern.get() && n == 2 && eis_sym(echild(e,1)))
        {
            spmake_prog_pattern(P, e, flags, cmps, appends);
            return;
        }
        else if (ch == gs.sym_sStringExpression.get())
        {
            spmake_prog_stringexpression(P, e, flags, cmps, appends);
            return;
        }
        else if (ch == gs.sym_sAlternatives.get())
        {
            spmake_prog_alternatives(P, e, flags, cmps, appends);
            return;
        }
        else if (ch == gs.sym_sRepeated.get() && n == 1)
        {
            spmake_prog_repeated(P, e, flags, cmps, appends);
            return;
        }
        else if (ch == gs.sym_sRepeatedNull.get() && n == 1)
        {
            spmake_prog_repeatednull(P, e, flags, cmps, appends);
            return;
        }
        else if (ch == gs.sym_sCondition.get() && n == 2)
        {
            spmake_prog_condition(P, e, flags, cmps, appends);
            return;
        }
        else if (ch == gs.sym_sPatternTest.get() && n == 2)
        {
            cmps.push_back(echild(e,2));
            spmake_prog(P, echild(e,1), flags, cmps, appends);                
            cmps.pop_back();
            return;
        }
    }
    spmake_prog_default(P, e, flags, cmps, appends);
    return;
}



void spmake_remove_goto1(spprogram&P)
{
    for (size_t i = 0; i < P.instructions.size(); i++)
    {
        if (P.instructions[i]->mnemonic == spmnemonic_goto && P.instructions[i]->nexts.size() == 1)
        {
            spinstruction * curr = P.instructions[i];
            spinstruction * next = P.instructions[i]->nexts[0];
            assert(P.end != curr);
            if (P.start == curr)
            {
                P.start = next;
            }
            for (size_t j = 0; j < P.instructions.size(); j++)
            {
                for (size_t k = 0; k < P.instructions[j]->nexts.size(); k++)
                {
                    if (P.instructions[j]->nexts[k] == curr)
                    {
                        P.instructions[j]->nexts[k] = next;
                    }
                }
            }
            P.instructions[i]->nexts.clear();
            P.instructions[i]->mnemonic = spmnemonic_none;
        }
    }
    return;
}

void spmake_remove_none(spprogram&P)
{
    size_t k = 0;
    size_t n = P.instructions.size();
    for (size_t i = 0; i < n; i++)
    {
        if (P.instructions[i]->mnemonic != spmnemonic_none)
        {
            P.instructions[k] = P.instructions[i];
            k++;
        }
        else
        {
            assert(P.start != P.instructions[i]);
            assert(P.end   != P.instructions[i]);
        }
    }
    P.instructions.resize(k);
    return;
}

size_t exvector_insert(std::vector<uex>&v, er e);


void spmachine::set_code(spprogram&P)
{
    /* set the offset and index members of each instruction */
    /* also build this.constants and this.symbols */
    symbols.clear();
    size_t tot = 0;
    for (std::vector<spinstruction*>::iterator i = P.instructions.begin(); i != P.instructions.end(); ++i)
    {
        spinstruction * p = *i;
        p->offset = tot;
        switch (p->mnemonic)
        {
            case spmnemonic_fail:                                                    tot += 1; break;
            case spmnemonic_pass:                                                    tot += 1; break;
            case spmnemonic_cmp:                                                     tot += 3; break;
            case spmnemonic_test:    p->idx = exvector_insert(constants, p->expr);   tot += 3; break;
            case spmnemonic_goto:            tot += p->nexts.size() + 1 + (p->nexts.size()>3); break;
            case spmnemonic_start:   p->idx = exvector_insert(symbols, p->expr);     tot += 3; break;
            case spmnemonic_capture: p->idx = exvector_insert(symbols, p->expr);     tot += 3; break;
            case spmnemonic_captureconst:
                p->idx = exvector_insert(symbols, p->expr);   
                tot += 4; break;
            case spmnemonic_stop:    p->idx = exvector_insert(symbols, p->expr);     tot += 3; break;
            case spmnemonic_next:                                                    tot += 2; break;
            case spmnemonic_end:                                                     tot += 2; break;
            case spmnemonic_goto1:                                                   tot += 2; break;
            case spmnemonic_goto2:                                                   tot += 3; break;
            case spmnemonic_goto3:                                                   tot += 4; break;
            case spmnemonic_goton:                                 tot += 2 + p->nexts.size(); break;
            case spmnemonic_none:                                                    tot += 1; break;
            default:                assert(false);
        }
    }
    start_ip = P.start->offset;

    /* set_expr assumes that vars has length "symbols.size()" */
    vars.clear();
    dummy_var0 = -1;
    dummy_var1 = -1;
    for (size_t i = 0; i < symbols.size(); i++)
    {
        vars.push_back(spvarstate());
    }

    /* now that the offsets are calculated, build up the code */
    code.clear();
    uselesstotal = 0;
    for (size_t i=0; i<P.instructions.size(); i++)
    {
        spinstruction * p = P.instructions[i];
        code.push_back(p->mnemonic);
        uselesstotal++;
        switch (p->mnemonic)
        {
            case spmnemonic_fail:
                assert(0 == p->nexts.size());
                break;
            case spmnemonic_pass:
                assert(0 == p->nexts.size());
                break;
            case spmnemonic_cmp:
                assert(1 == p->nexts.size());
                code.push_back(p->char2);
                break;
            case spmnemonic_test:
                assert(1 == p->nexts.size());
                code.push_back(p->idx);
                break;
            case spmnemonic_start:
                assert(1 == p->nexts.size());
                code.push_back(p->idx);
                break;
            case spmnemonic_capture:
                assert(1 == p->nexts.size());
                code.push_back(p->idx);
                break;
            case spmnemonic_captureconst:
                assert(1 == p->nexts.size());
                code.push_back(p->idx);
                code.push_back(p->char2);
                break;
            case spmnemonic_stop:
                assert(1 == p->nexts.size());
                code.push_back(p->idx);
                break;
            case spmnemonic_next:     assert(1 == p->nexts.size()); uselesstotal--; break;
            case spmnemonic_end:      assert(1 == p->nexts.size()); uselesstotal--; break;
            case spmnemonic_goto1:    assert(1 == p->nexts.size()); break;
            case spmnemonic_goto2:    assert(2 == p->nexts.size()); break;
            case spmnemonic_goto3:    assert(3 == p->nexts.size()); break;
            case spmnemonic_goton:    code.push_back(p->nexts.size()); break;
            case spmnemonic_none:     assert(0 == p->nexts.size()); break;
            case spmnemonic_goto:
                if (p->nexts.size()>3) {
                    code.back() = spmnemonic_goton;
                    code.push_back(p->nexts.size());
                } else if (p->nexts.size()==3) {
                    code.back() = spmnemonic_goto3;
                } else if (p->nexts.size()==2) {
                    code.back() = spmnemonic_goto2;
                } else if (p->nexts.size()==1) {
                    code.back() = spmnemonic_goto1;
                } else {
                    assert(false);
                }
                break;
            default:                 assert(false);
        }
        for (size_t j=0; j<p->nexts.size(); j++)
        {
            code.push_back(p->nexts[j]->offset);
        }
    }

//std::cout << "machine with program:" <<std::endl;    print_mach();
}


void spmachine::set_pattern(er pat)
{
    spinstruction * in, * prev;
    std::vector<er> cmps;
    std::vector<er> appends;
std::cout << "set_pattern: " << ex_tostring_full(pat) << std::endl;

    /* construct P as . -> pat -> endhead -> pass */
    spprogram P;
    /* start */
    P.instructions.clear();
    P.start = in = new spinstruction(spmnemonic_goto);
    P.instructions.push_back(in);
    prev = in;
    /* pat */
    spprogram Q;
    spmake_prog(Q, pat, 0, cmps, appends);
    assert(cmps.empty());
    assert(appends.empty());
    for (size_t j = 0; j < Q.instructions.size(); j++) {
        P.instructions.push_back(Q.instructions[j]);
    }
    prev->nexts.push_back(Q.start);
    prev = Q.end;
    Q.instructions.clear();
    /* end */
    P.end = in = new spinstruction(spmnemonic_end);
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    prev = in;
    /* pass */
    P.end = in = new spinstruction(spmnemonic_pass);
    P.instructions.push_back(in);
    prev->nexts.push_back(in);
    prev = in;


//P.print();

    /* replace goto1 instructions with none */
    for (size_t i = 0; i < P.instructions.size(); i++)
    {
        if (P.instructions[i]->mnemonic == spmnemonic_goto && P.instructions[i]->nexts.size() == 1)
        {
            spinstruction * curr = P.instructions[i];
            spinstruction * next = P.instructions[i]->nexts[0];
            assert(P.end != curr);
            if (P.start == curr)
            {
                P.start = next;
            }
            for (size_t j = 0; j < P.instructions.size(); j++)
            {
                for (size_t k = 0; k < P.instructions[j]->nexts.size(); k++)
                {
                    if (P.instructions[j]->nexts[k] == curr)
                    {
                        P.instructions[j]->nexts[k] = next;
                    }
                }
            }
            P.instructions[i]->nexts.clear();
            P.instructions[i]->mnemonic = spmnemonic_none;
        }
    }

//std::cout << "after replace goto1" << std::endl;
//P.print();

    /* remove none instructions */
    size_t k = 0;
    size_t n = P.instructions.size();
    for (size_t i = 0; i < n; i++) {
        if (P.instructions[i]->mnemonic != spmnemonic_none) {
            P.instructions[k] = P.instructions[i];
            k++;
        } else {
            assert(P.start != P.instructions[i]);
            assert(P.end   != P.instructions[i]);
            delete P.instructions[i];
        }
    }
    P.instructions.resize(k);

//std::cout << "after remove none" << std::endl;
//P.print();


    set_code(P);
    ip = start_ip;

print_code();
}


void spmachine::set_expr(er e)
{
//std::cout << "set_expr " << ex_tostring_full(e) << std::endl;

    /* init M for a run on e */
    assert(vars.size() == symbols.size());

    clear_undo();
    ip = start_ip;

    for (size_t i = 0; i < vars.size(); i++)
    {
        vars[i].state = 0;
        vars[i].val.clear();
    }

    callstack.resize(1);
    calldepth = 0;

    cf = &callstack[0];
    cf->i = 0;
    cf->estring.clear();
    for (size_t i = 0; i < estr_string(e).size(); i++)
    {
        cf->estring.push_back(estr_string(e)[i]);
    }

//std::cout << "set_expr done " << ex_tostring_full(e) << std::endl;

}


bool spmachine::_replacevars(std::vector<uex>&out, er e)
{
//std::cout << "_apply_function1 called: " << ex_tostring(b) << std::endl;
    if (!eis_node(e))
    {
        if (eis_sym(e))
        {
            for (size_t i = 0; i < symbols.size(); i++)
            {
                if (e == symbols[i].get())
                {
                    out.push_back(uex(emake_str_char16v(vars[i].val)));
                    return true;
                }
            }
        }
        out.push_back(uex(ecopy(e)));
        return true;
    }
    else
    {
        std::vector<uex> v;
        _replacevars(v, echild(e,0));

        uex h;
        if (v.size() == 1)
        {
            h.reset(ecopy(v[0].get()));
        }
        else
        {
            h.reset(emake_node(gs.sym_sSequence.copy(), v));
        }
        v.clear();
        for (size_t i=1; i<=elength(e); i++)
        {
            _replacevars(v, echild(e,i));
        }
        out.push_back(uex(emake_node(h.release(), v)));
        return true;
    }
}

ex spmachine::replacevars(ex E)
{
    uex e(E);
//std::cout << "apply_function1 called: " << ex_tostring(f.get()) << ", " << ex_tostring(e.get()) << std::endl;

    std::vector<uex> v;
    _replacevars(v, e.get());


        if (v.size() == 1)
        {
            return ecopy(v[0].get());
        }
        else
        {
            return emake_node(gs.sym_sSequence.copy(), v);
        }
}




ex dcode_sStringMatchQ(er e)
{
//std::cout << "dcode_sStringMatchQ: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sStringMatchQ.get()));

    if (elength(e) == 2 && eis_str(echild(e,1)))
    {
        spmachine M;
        M.set_pattern(echild(e,2));
        M.set_expr(echild(e,1));
        return emake_boole(M.execute() != 0);
    }
    else
    {
        return ecopy(e);
    }
}



ex dcode_sStringReplace(er e)
{
//std::cout << "dcode_sStringReplace: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sStringReplace.get()));

    return ecopy(e);
}


ex dcode_sNames(er e)
{
//std::cout << "dcode_sNames: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sNames.get()));

    if (elength(e) != 1)
    {
        _handle_message_argx1(e);
    }

    spmachine M;
    M.set_pattern(echild(e,1));

    std::vector<wex> v;

    for (auto it = gs.live_symbols.begin(); it != gs.live_symbols.end(); it++)
    {
        M.clear_string();
        M.push_string(esym_context_string(it->get()));
        M.push_string(esym_name_string(it->get()));
        if (M.execute() != 0)
            v.push_back(wex(ecopy(esym_name(it->get()))));
    }

    for (auto it = gs.live_tsymbols.begin(); it != gs.live_tsymbols.end(); it++)
    {
        M.clear_string();
        M.push_string(esym_context_string(*it));
        M.push_string(esym_name_string(*it));
        if (M.execute() != 0)
            v.push_back(wex(ecopy(esym_name(*it))));
    }

    return emake_node(gs.sym_sList.copy(), v);
}

