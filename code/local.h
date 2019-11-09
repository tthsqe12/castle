#include "uex.h"
#include "timing.h"
#include "ex_print.h"
#include "eval.h"
#include "code.h"
#include "hash.h"
#include "arithmetic.h"

class sym_localize {
public:
    er entry;
    ex_sym_struct save;

    sym_localize(er var)
    {
//std::cout << "localizing: " << ex_tostring_full(var) << std::endl;

        entry = etor(ecopy(var));
        save.normalattr = esym_normalattr(entry);
        save.extraattr = esym_extraattr(entry);

        save.ovalue = esym_ovalue(entry);
        save.dvalue = esym_dvalue(entry);
        save.uvalue = esym_uvalue(entry);
        save.svalue = esym_svalue(entry);

        esym_ovalue(entry) = nullptr;
        esym_dvalue(entry) = nullptr;
        esym_uvalue(entry) = nullptr;
        esym_svalue(entry) = nullptr;

    }

    ~sym_localize()
    {
//std::cout << "restoring: " << ex_tostring_full(original_entry) << std::endl;

        if (esym_ovalue(entry) != nullptr) eclear(etox(esym_ovalue(entry)));
        if (esym_dvalue(entry) != nullptr) delete esym_dvalue(entry);
        if (esym_uvalue(entry) != nullptr) delete esym_uvalue(entry);
        if (esym_svalue(entry) != nullptr) delete esym_svalue(entry);

        esym_ovalue(entry) = save.ovalue;
        esym_dvalue(entry) = save.dvalue;
        esym_uvalue(entry) = save.uvalue;
        esym_svalue(entry) = save.svalue;

        esym_normalattr(entry) = save.normalattr;
        esym_extraattr(entry) = save.extraattr;

        eclear(etox(entry));
    }
};

class sym_localize_mul {
    sym_localize * pdata;
    ulong length;
public:
    sym_localize_mul(ulong n)
    {
        length = 0;
        pdata = (sym_localize *) malloc((n+1) * sizeof(sym_localize));
        if (pdata == nullptr)
            assert(false);
    }
    void push_back(er var)
    {
        new (pdata + length) sym_localize(var);
        length++;
    }
    sym_localize& back()
    {
        assert(length > 0);
        return pdata[length - 1];
    }
    ~sym_localize_mul()
    {
        for (ulong i = length; i > 0; i--)
        {
            (pdata + i - 1)->~sym_localize();
        }
        free(pdata);
    }
};
