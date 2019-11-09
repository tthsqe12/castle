#include "ex.h"
#include "globalstate.h"
#include "code.h"

ex dcode_sUpdate(er e)
{
//std::cout << "dcode_sUpdate: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sUpdate.get()));

    if (elength(e) == 0)
    {
        gs.update_buckets(gs.active_buckets_u | gs.active_buckets_aosd, ++gs.timestamp);
        for (auto it = gs.live_symbols.begin(); it != gs.live_symbols.end(); ++it)
            esym_timestamp(it->get()) = gs.timestamp;
        for (auto it = gs.live_tsymbols.begin(); it != gs.live_tsymbols.end(); ++it)
            esym_timestamp(*it) = gs.timestamp;
    }
    else
    {
        uint32_t cf = 0;
        for (size_t i = 1; i <= elength(e); i++)
        {
            if (!eis_sym(echild(e,i)))
            {
                ex t = emake_int_ui(i);
                _gen_message(echild(e,0), "sym", NULL, ecopychild(e,i), t);
                return ecopy(e);
            }
            cf |= esym_bucketflags(echild(e,i));
        }
        gs.update_buckets(cf, ++gs.timestamp);
        for (size_t i = 1; i <= elength(e); i++)
        {
            esym_timestamp(echild(e,i)) = gs.timestamp;
        }
    }
    return gs.sym_sNull.copy();
}
