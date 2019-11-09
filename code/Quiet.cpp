#include "globalstate.h"
#include "code.h"
#include "eval.h"

class install_muffler {
private:
    size_t muffler_count;
public:
    install_muffler() {
        gs.muffler_stack.push_back(emake_node(gs.sym_sList.copy()));
        muffler_count = gs.muffler_stack.size();
    }

    ~install_muffler() {
        assert(muffler_count == gs.muffler_stack.size());
        gs.muffler_stack.pop_back();
    }
};

ex dcode_sQuiet(er e)
{
//std::cout << "dcode_sQuiet: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sQuiet.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    install_muffler M;
    uex r(eval(ecopychild(e,1)));
    return r.release();
}
