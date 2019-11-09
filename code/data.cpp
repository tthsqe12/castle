#include "globalstate.h"
#include "code.h"


ex dcode_dBuckets(er e)
{
    assert(ehas_head_sym(e, gs.sym_dBuckets.get()));

    if (elength(e) == 0)
    {
        std::vector <wex> vaosd, vu;
        uint32_t m = 1;
        for (int i = 0; i < 32; i++)
        {
            if ((gs.active_buckets_aosd & m) != 0)
            {
                vaosd.push_back(wex(emake_cint(i)));
            }
            if ((gs.active_buckets_u & m) != 0)
            {
                vu.push_back(wex(emake_cint(i)));
            }
            m = m << 1;
        }

        uex t1(emake_node(gs.sym_sList.copy(), vaosd));
        ex t2 = emake_node(gs.sym_sList.copy(), vu);
        return emake_node(gs.sym_sList.copy(), t1.release(), t2);
    }
    else if (elength(e) == 1)
    {
        std::vector <wex> v;
        uint32_t m = 1;
        uint32_t b = ebucketflags(echild(e,1));
        for (int i = 0; i < 32; i++)
        {
            if ((b & m) != 0)
            {
                v.push_back(wex(emake_cint(i)));
            }
            m = m << 1;
        }

        return emake_node(gs.sym_sList.copy(), v);
    }
    else
    {
        return _handle_message_argt(e, 0 + (1 << 8));
    }


}

ex dcode_dHashCode(er e)
{
//std::cout << "dcode_sHashCode: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_dHashCode.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    return emake_int_ui(ex_hash(echild(e,1)));
}
