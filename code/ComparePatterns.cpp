#include "globalstate.h"
#include "code.h"

ex dcode_iComparePatterns(er e)
{
//std::cout << "dcode_iComparePatterns: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_iComparePatterns.get()));

    if (elength(e) != 2)
    {
        return _handle_message_argx2(e);
    }

    switch (compare_patterns(echild(e,1), echild(e,2)))
    {
        case PATTERN_CMP_IDENTICAL:
            return emake_str("Identical");
        case PATTERN_CMP_EQUIVALENT:
            return emake_str("Equivalent");
        case PATTERN_CMP_LESS:
            return emake_str("Specific");
        case PATTERN_CMP_GREATER:
            return emake_str("General");
        case PATTERN_CMP_DISJOINT:
            return emake_str("Disjoint");
        case PATTERN_CMP_UNKNOWN:
        default:
            return emake_str("Incomparable");
    }
}
