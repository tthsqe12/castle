#include "boxes.h"
#include "ex_parse.h"
#include <stack>
#include "serialize.h"
#include "notebook.h"
#include "box_convert.h"

boxheader box_null =
{
    .type = BNTYPE_NULLER,
    .centery = 1,
    .sizex = 2,
    .sizey = 2
};


ibox iboxchar_set_sizes(ibox b, int32_t sizex, int32_t sizey, int32_t centery)
{
    assert(sizex >= 0);
    assert(sizey >= 0);
    assert(centery >= 0);

    if ((sizex|centery) < (1 << 10) && (sizey) < (1 << 11))
    {
        int32_t btype;
        if (ibox_is_ptr(b))
        {
            btype = ibox_to_ptr(b)->get_type();
            delete ibox_to_ptr(b);
        }
        else
        {
            btype = iboximm_type(b);
        }
        b.imm =  uint64_t(1) + (uint64_t(sizey) << 1) + (uint64_t(centery) << 12) + (uint64_t(sizex) << 22)+ (uint64_t(btype) << 32);
    }
    else
    {
        if (ibox_is_ptr(b))
        {
            ibox_to_ptr(b)->sizex = sizex;
            ibox_to_ptr(b)->sizey = sizey;
            ibox_to_ptr(b)->centery = centery;
        }
        else
        {
            b.ptr = new charbox(iboximm_type(b), sizex, sizey, centery);
        }
    }

    return b;
}

ibox iboxchar_set_sizes(ibox b, int32_t sizey, int32_t centery)
{
    int32_t sizex = ibox_is_ptr(b) ? ibox_to_ptr(b)->sizex : iboximm_sizex(b);
    assert(sizex >= 0);
    assert(sizey >= 0);
    assert(centery >= 0);

    if ((sizex|centery) < (1 << 10) && (sizey) < (1 << 11))
    {
        int32_t btype;
        if (ibox_is_ptr(b))
        {
            btype = ibox_to_ptr(b)->get_type();
            delete ibox_to_ptr(b);
        }
        else
        {
            btype = iboximm_type(b);
        }
        b.imm =  uint64_t(1) + (uint64_t(sizey) << 1) + (uint64_t(centery) << 12) + (uint64_t(sizex) << 22)+ (uint64_t(btype) << 32);
    }
    else
    {
        if (ibox_is_ptr(b))
        {
            ibox_to_ptr(b)->sizex = sizex;
            ibox_to_ptr(b)->sizey = sizey;
            ibox_to_ptr(b)->centery = centery;
        }
        else
        {
            b.ptr = new charbox(iboximm_type(b), sizex, sizey, centery);
        }
    }

    return b;
}


/* print *********************************************************************/

void nullbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");

    printf("(%d,%d) nullbox: size(%d,%d:%d)\n",
             offx, offy, sizex, sizey, sizey);
}

void charbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) char: (%d,%d,%d) size(%d,%d:%d)\n",
        offx, offy, (get_type()>>24)&255, (get_type()>>16)&255, (get_type())&65535, sizex, sizey, sizey);
}

void graphics3dbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");

    printf("(%d,%d) graphics3dbox: size(%d,%d:%d)\n",
             offx, offy, sizex, sizey, sizey);
}

void subscriptbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");

    printf("(%d,%d) subscriptbox: size(%d,%d:%d)\n",
             offx, offy, sizex, sizey, sizey);

    sub.cbox->print(indent + 1, sub.offx, sub.offy);
}

void superscriptbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");

    printf("(%d,%d) superscriptbox: size(%d,%d:%d)\n",
             offx, offy, sizex, sizey, sizey);

    super.cbox->print(indent + 1, super.offx, super.offy);
}

void subsuperscriptbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");

    printf("(%d,%d) subsuperscriptbox: size(%d,%d:%d) cursor(%d)\n",
             offx, offy, sizex, sizey, sizey, cursor);

    sub.cbox->print(indent + 1, sub.offx, sub.offy);
    super.cbox->print(indent + 1, super.offx, super.offy);
}

void rowbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) rowbox: size(%d,%d:%d) cursor(%d,%d)\n",
             offx, offy, sizex, sizey, centery, cursor_a, cursor_b);

    for (auto& c: child)
    {
        if (ibox_is_imm(c.cibox))
        {
            for (size_t i = 0; i <= indent; i++)
                printf("    ");
            printf("(%d,%d) imm: (%d,%d,%d) size(%d,%d:%d)\n", c.offx, c.offy,
                 (iboximm_type(c.cibox)>>24)&255, (iboximm_type(c.cibox)>>16)&255, (iboximm_type(c.cibox)>>0)&65535,
                     iboximm_sizex(c.cibox), iboximm_sizey(c.cibox), iboximm_centery(c.cibox));
        }
        else
        {
            ibox_to_ptr(c.cibox)->print(indent + 1, c.offx, c.offy);
        }
    }
}

void rootbox::print(size_t indent, int32_t offx, int32_t offy)
{
//std::cout << "rootbox::print " << indent << std::endl;

    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) root: size (%d,%d:%d) cursor(%d,%d)\n", offx, offy, sizex, sizey, sizey, cursor_a, cursor_b);

    for (auto& c: childcells)
    {
        if (c.cbox == nullptr)
        {
            for (size_t i = 0; i <= indent; i++)
                printf("    ");
            printf("(%d,%d) null\n", c.offx, c.offy);
        }
        else
        {
            c.cbox->print(indent + 1, c.offx, c.offy);
        }
    }
}

void cellbox::print(size_t indent, int32_t offx, int32_t offy)
{
//std::cout << "cellbox::print " << indent << std::endl;

    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) cell: size(%d,%d:%d)\n", offx, offy, sizex, sizey, sizey);

    body.cbox->print(indent + 1, body.offx, body.offy);

    if (label.cbox != nullptr)
        label.cbox->print(indent + 1, label.offx, label.offy);
}

void fractionbox::print(size_t indent, int32_t offx, int32_t offy)
{
//std::cout << "fractionbox::print " << indent << std::endl;

    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) fraction: size(%d,%d:%d) cursor(%d)\n", offx, offy, sizex, sizey, sizey, cursor);

    num.cbox->print(indent + 1, num.offx, num.offy);
    den.cbox->print(indent + 1, den.offx, den.offy);
}

void sqrtbox::print(size_t indent, int32_t offx, int32_t offy)
{
//std::cout << "fractionbox::print " << indent << std::endl;

    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) sqrt: size (%d,%d:%d)\n", offx, offy, sizex, sizey, sizey);

    inside.cbox->print(indent + 1, inside.offx, inside.offy);
}

void underscriptbox::print(size_t indent, int32_t offx, int32_t offy)
{
//std::cout << "fractionbox::print " << indent << std::endl;

    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) sqrt: size (%d,%d:%d)\n", offx, offy, sizex, sizey, sizey);

    body.cbox->print(indent + 1, body.offx, body.offy);
    under.cbox->print(indent + 1, under.offx, under.offy);
}

void overscriptbox::print(size_t indent, int32_t offx, int32_t offy)
{
//std::cout << "fractionbox::print " << indent << std::endl;

    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) sqrt: size (%d,%d:%d)\n", offx, offy, sizex, sizey, sizey);

    body.cbox->print(indent + 1, body.offx, body.offy);
    over.cbox->print(indent + 1, over.offx, over.offy);
}

void underoverscriptbox::print(size_t indent, int32_t offx, int32_t offy)
{
//std::cout << "fractionbox::print " << indent << std::endl;

    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) sqrt: size (%d,%d:%d)\n", offx, offy, sizex, sizey, sizey);

    body.cbox->print(indent + 1, body.offx, body.offy);
    under.cbox->print(indent + 1, under.offx, under.offy);
    over.cbox->print(indent + 1, over.offx, over.offy);
}


/* insert_char ***************************************************************/

void charbox::insert_char(char16_t c)
{
    assert(false);
}

void nullbox::insert_char(char16_t c)
{
    assert(false);
}

void graphics3dbox::insert_char(char16_t c)
{
    return;
}

void rootbox::delete_selection()
{
    assert(cursor_a <= childcells.size());
    assert(cursor_b <= childcells.size());

    int32_t left = std::min(cursor_a, cursor_b);
    int32_t right = std::max(cursor_a, cursor_b);

    for (auto i = childcells.begin() + left; i != childcells.begin() + right; ++i)
        delete (*i).cbox;
    childcells.erase(childcells.begin() + left, childcells.begin() + right);
    cursor_b = cursor_a = left;
}


void rootbox::insert_char(char16_t c)
{
//std::cout << "rootbox::insert_char " << c << std::endl;
    if (cursor_b > childcells.size())
    {
        assert(cursor_a < childcells.size());
        childcells[cursor_a].cbox->insert_char(c);
        return;
    }

    delete_selection();

    rowbox* newrow = new rowbox(2, 1,1);
    newrow->child[0].cibox = iboximm_make(c);
    newrow->child[1].cibox.ptr = new nullbox();

    cellbox* newcell = new cellbox(newrow, cellt_INPUT);

    assert(cursor_a <= childcells.size());
    childcells.insert(childcells.begin() + cursor_a, cellboxarrayelem());
    childcells[cursor_a].cbox = newcell;
    cursor_b = childcells.size() + 1;
}

void rowbox::delete_selection()
{
    assert(cursor_a < child.size());
    assert(cursor_b < child.size());

    int32_t left = std::min(cursor_a, cursor_b);
    int32_t right = std::max(cursor_a, cursor_b);

    for (auto i = child.begin() + left; i != child.begin() + right; ++i)
        if (ibox_is_ptr((*i).cibox))
            delete ibox_to_ptr((*i).cibox);
    child.erase(child.begin() + left, child.begin() + right);
    cursor_b = cursor_a = left;
}

void rowbox::insert_char(char16_t c)
{
//std::cout << "rowbox::insert_char " << c << std::endl;

    flags &= ~(BNFLAG_MEASURED | BNFLAG_COLORED);

    if (cursor_b >= child.size())
    {
        assert(cursor_a < child.size());
        ibox_to_ptr(child[cursor_a].cibox)->insert_char(c);
        return;
    }

    delete_selection();
    assert(cursor_a < child.size());
    child.insert(child.begin() + cursor_a, iboxarrayelem());
    child[cursor_a].cibox = iboximm_make(c);
    cursor_a++;
    cursor_b = cursor_a;

    int32_t r, l, cc;
    char t[40];

    // look for \[ to the left
    for (l = cursor_a - 1; l >= 0; l--)
    {
        if (!ibox_is_char(child[l].cibox))
            goto alias_scan;
        if (ibox_is_char(child[l].cibox, '\\'))
            break;
    }
    if (l < 0)
        goto alias_scan;
    if (!(l+2 < child.size() && ibox_is_char(child[l+1].cibox, '[')))
        goto alias_scan;
    if (l > 0 && ibox_is_char(child[l-1].cibox, '['))
        goto alias_scan;

    // look for ] to the right
    for (r = cursor_a - 1; r+1 < child.size(); r++)
    {
        if (!ibox_is_char(child[r].cibox))
            goto alias_scan;
        if (ibox_is_char(child[r].cibox, ']'))
            break;
    }
    if (r+1 >= child.size())
        goto alias_scan;

    if (r-l > 30)
        goto alias_scan;

    for (int32_t i=l; i<=r; i++)
        t[i-l] = ibox_type(child[i].cibox)&65535;
    t[r+1-l] = 0;
    cc = escapedname_to_char(t) && 65535;
    if (cc == 0)
        goto alias_scan;

    for (auto i = child.begin() + l; i != child.begin() + r+1; ++i)
        if (ibox_is_ptr((*i).cibox))
            delete ibox_to_ptr((*i).cibox);
    child.erase(child.begin() + l+1, child.begin() + r+1);
    child[l].cibox = iboximm_make(cc);
    cursor_b = cursor_a = l + 1;
    return;

alias_scan:

    // look for \[AliasDelimiter] to the right
    for (r = cursor_a - 1; r+1 < child.size(); r++)
    {
        if (!ibox_is_char(child[r].cibox))
            return;
        if (ibox_is_char(child[r].cibox, CHAR_AliasDelimiter))
            break;
    }
    if (r+1 >= child.size())
        return;

    // look for \[AliasDelimiter] to the left
    for (l = std::min(cursor_a - 1, r-1); l >= 0; l--)
    {
        if (!ibox_is_char(child[l].cibox))
            return;
        if (ibox_is_char(child[l].cibox, CHAR_AliasDelimiter))
            break;
    }
    if (l<0)
        return;

    if (r-l > 30)
        return;

    for (int32_t i=l+1; i<r; i++)
        t[i-(l+1)] = ibox_type(child[i].cibox)&65535;
    t[r-(l+1)] = 0;
    cc = escape_seq_to_action(t);
    if (cc <= 0)
        return;

    for (auto i = child.begin() + l; i != child.begin() + r+1; ++i)
        if (ibox_is_ptr((*i).cibox))
            delete ibox_to_ptr((*i).cibox);
    child.erase(child.begin() + l+1, child.begin() + r+1);
    child[l].cibox = iboximm_make(cc);
    cursor_b = cursor_a = l + 1;
    return;
}

void cellbox::insert_char(char16_t c)
{
//std::cout << "cellbox::insert_char " << c << std::endl;
    switch (cursor)
    {
        case 0:
            body.cbox->insert_char(c);
            return;
        case 1:
            assert(label.cbox != nullptr);
            label.cbox->insert_char(c);
            return;
        default:
            assert(false);
    }
}

void fractionbox::insert_char(char16_t c)
{
//std::cout << "subsuperscriptbox::insert_char " << c << std::endl;
    switch (cursor)
    {
        case 0:
            num.cbox->insert_char(c);
            return;
        case 1:
            den.cbox->insert_char(c);
            return;
        default:
            assert(false);
    }
}

void sqrtbox::insert_char(char16_t c)
{
//std::cout << "sqrtbox::insert_char " << c << std::endl;
    inside.cbox->insert_char(c);
    return;
}

void subscriptbox::insert_char(char16_t c)
{
//std::cout << "subscriptbox::insert_char " << c << std::endl;
    sub.cbox->insert_char(c);
    return;
}

void superscriptbox::insert_char(char16_t c)
{
//std::cout << "superscriptbox::insert_char " << c << std::endl;
    super.cbox->insert_char(c);
    return;
}

void subsuperscriptbox::insert_char(char16_t c)
{
//std::cout << "subsuperscriptbox::insert_char " << c << std::endl;
    switch (cursor)
    {
        case 0:
            sub.cbox->insert_char(c);
            return;
        case 1:
            super.cbox->insert_char(c);
            return;
        default:
            assert(false);
    }
}

void underscriptbox::insert_char(char16_t c)
{
//std::cout << "underscriptbox::insert_char " << c << std::endl;
    switch (cursor)
    {
        case 0:
            body.cbox->insert_char(c);
            return;
        case 1:
            under.cbox->insert_char(c);
            return;
        default:
            assert(false);
    }
}

void overscriptbox::insert_char(char16_t c)
{
//std::cout << "overscriptbox::insert_char " << c << std::endl;
    switch (cursor)
    {
        case 0:
            body.cbox->insert_char(c);
            return;
        case 1:
            over.cbox->insert_char(c);
            return;
        default:
            assert(false);
    }
}

void underoverscriptbox::insert_char(char16_t c)
{
//std::cout << "underoverscriptbox::insert_char " << c << std::endl;
    switch (cursor)
    {
        case 0:
            body.cbox->insert_char(c);
            return;
        case 1:
            under.cbox->insert_char(c);
            return;
        case 2:
            over.cbox->insert_char(c);
            return;
        default:
            assert(false);
    }
}

/* move **********************************************************************/

moveRet charbox::move(moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        case movearg_Right:
        case movearg_First:
        case movearg_Last:
        {
            return moveret_End;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

moveRet nullbox::move(moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        case movearg_Right:
        case movearg_First:
        case movearg_Last:
        {
            return moveret_End;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

moveRet graphics3dbox::move(moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        case movearg_ShiftLeft:
        case movearg_Right:
        case movearg_ShiftRight:
        {
            return moveret_End;
        }
        case movearg_Last:
        case movearg_First:
        {
            return moveret_OK;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

moveRet rootbox::move(moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        {
            if (cursor_b > childcells.size())
            {
                assert(cursor_a < childcells.size());
                r = childcells[cursor_a].cbox->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                    cursor_b = cursor_a;
                return moveret_OK;
            }
            else
            {
                assert(cursor_a <= childcells.size());
                if (cursor_a > 0)
                {
                    cursor_a--;
                    r = childcells[cursor_a].cbox->move(movearg_Last);
                    assert(r == moveret_OK);
                    cursor_b = childcells.size() + 1;
                    return moveret_OK;
                }
                else
                {
                    cursor_a = cursor_b = 0;
                    return moveret_OK;
                }
            }
        }
        case movearg_ShiftLeft:
        {
            if (cursor_b > childcells.size())
            {
                assert(cursor_a < childcells.size());
                r = childcells[cursor_a].cbox->move(movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return moveret_OK;
            }
            else
            {
                return moveret_OK;
            }
        }
        case movearg_Right:
        {
            if (cursor_b > childcells.size())
            {
                assert(cursor_a < childcells.size());
                r = childcells[cursor_a].cbox->move(movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    cursor_a++;
                    cursor_b = cursor_a;
                }
                return moveret_OK;
            }
            else
            {
                assert(cursor_a <= childcells.size());
                if (cursor_a < childcells.size())
                {
                    r = childcells[cursor_a].cbox->move(movearg_First);
                    assert(r == moveret_OK);
                    cursor_b = childcells.size() + 1;
                    return moveret_OK;
                }
                else
                {
                    cursor_b = cursor_a = childcells.size();
                    return moveret_OK;
                }
            }
        }
        case movearg_ShiftRight:
        {
            if (cursor_b > childcells.size())
            {
                assert(cursor_a < childcells.size());
                r = childcells[cursor_a].cbox->move(movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return moveret_OK;
            }
            else
            {
                return moveret_OK;
            }
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

moveRet cellbox::move(moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = label.cbox->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = body.cbox->move(movearg_Last);
                    assert(r == moveret_OK);
                    cursor = 0;
                }
                return moveret_OK;
            }
        }
        case movearg_ShiftLeft:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = label.cbox->move(movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Right:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(movearg_Right);
            }
            else
            {
                assert(cursor == 1);
                r = label.cbox->move(movearg_Right);
            }
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_ShiftRight:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = label.cbox->move(movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Last:
        {
            r = body.cbox->move(movearg_Last);
            cursor = 0;
            assert(r == moveret_OK);
            return r;
        }
        case movearg_First:
        {
            r = body.cbox->move(movearg_First);
            assert(r == moveret_OK);
            cursor = 0;
            return r;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

moveRet rowbox::move(moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        {
            if (cursor_b >= child.size())
            {
                assert(cursor_a < child.size());
                r = ibox_to_ptr(child[cursor_a].cibox)->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                    cursor_b = cursor_a;
                return moveret_OK;
            }
            else
            {
                assert(cursor_a < child.size());
                if (cursor_a > 0)
                {
                    cursor_a--;
                    r = moveret_End;
                    if (ibox_is_ptr(child[cursor_a].cibox))
                        r = ibox_to_ptr(child[cursor_a].cibox)->move(movearg_Last);
                    assert(r == moveret_OK || r == moveret_End);
                    if (r == moveret_End)
                        cursor_b = cursor_a;
                    else
                        cursor_b = child.size();
                    return moveret_OK;
                }
                else
                {
                    cursor_a = cursor_b = 0;
                    return moveret_End;
                }
            }
        }
        case movearg_ShiftLeft:
        {
            if (cursor_b >= child.size())
            {
                assert(cursor_a < child.size());
                r = ibox_to_ptr(child[cursor_a].cibox)->move(movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                    cursor_b = std::min(cursor_a + 1, int32_t(child.size() - 1));
                return moveret_OK;
            }
            else
            {
                assert(cursor_a < child.size());
                if (cursor_a > 0)
                {
                    cursor_a--;
                    return moveret_OK;
                }
                else
                {
                    return moveret_End;
                }
            }
        }
        case movearg_Right:
        {
            if (cursor_b >= child.size())
            {
                assert(cursor_a < child.size());
                r = ibox_to_ptr(child[cursor_a].cibox)->move(movearg_Right);
            }
            else
            {
                assert(cursor_a < child.size());
                r = moveret_End;
                if (ibox_is_ptr(child[cursor_a].cibox))
                    r = ibox_to_ptr(child[cursor_a].cibox)->move(movearg_First);
            }
            assert(r == moveret_OK || r == moveret_End);
            if (r == moveret_End)
            {
                if (cursor_a + 1 < child.size())
                {
                    cursor_a++;
                    cursor_b = cursor_a;
                    return moveret_OK;
                }
                else
                {
                    cursor_b = cursor_a;
                    return moveret_End;
                }
            }
            else
            {
                cursor_b = child.size();
                return moveret_OK;
            }
        }
        case movearg_ShiftRight:
        {
            if (cursor_b >= child.size())
            {
                assert(cursor_a < child.size());
                r = ibox_to_ptr(child[cursor_a].cibox)->move(movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_OK)
                    return r;
                cursor_b = cursor_a;
            }
            if (cursor_a + 1 < child.size())
            {
                cursor_a++;
                return moveret_OK;
            }
            else
            {
                return moveret_End;
            }
        }
        case movearg_Last:
        {
            assert(child.size() > 0);
            cursor_b = cursor_a = child.size() - 1;
            select_placeholder_if_possible();
            return moveret_OK;
        }
        case movearg_First:
        {
            assert(child.size() > 0);
            cursor_b = cursor_a = 0;
            select_placeholder_if_possible();
            return moveret_OK;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

moveRet sqrtbox::move(moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        {
            r = inside.cbox->move(movearg_Left);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_ShiftLeft:
        {
            r = inside.cbox->move(movearg_ShiftLeft);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_Right:
        {
            r = inside.cbox->move(movearg_Right);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_ShiftRight:
        {
            r = inside.cbox->move(movearg_ShiftRight);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_Last:
        {
            r = inside.cbox->move(movearg_Last);
            assert(r == moveret_OK);
            return r;
        }
        case movearg_First:
        {
            r = inside.cbox->move(movearg_First);
            assert(r == moveret_OK);
            return r;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

moveRet fractionbox::move(moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        {
            if (cursor == 0)
            {
                r = num.cbox->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = den.cbox->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = num.cbox->move(movearg_Last);
                    assert(r == moveret_OK);
                    cursor = 0;
                }
                return moveret_OK;
            }
        }
        case movearg_ShiftLeft:
        {
            if (cursor == 0)
            {
                r = num.cbox->move(movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = den.cbox->move(movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Right:
        {
            if (cursor == 1)
            {
                r = den.cbox->move(movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 0);
                r = num.cbox->move(movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = den.cbox->move(movearg_First);
                    assert(r == moveret_OK);
                    cursor = 1;
                }
                return moveret_OK;
            }
        }
        case movearg_ShiftRight:
        {
            if (cursor == 0)
            {
                r = num.cbox->move(movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = den.cbox->move(movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Last:
        {
            r = den.cbox->move(movearg_Last);
            assert(r == moveret_OK);
            cursor = 1;
            return r;
        }
        case movearg_First:
        {
            r = num.cbox->move(movearg_First);
            assert(r == moveret_OK);
            cursor = 0;
            return r;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

moveRet subscriptbox::move(moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        {
            r = sub.cbox->move(movearg_Left);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_ShiftLeft:
        {
            r = sub.cbox->move(movearg_ShiftLeft);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_Right:
        {
            r = sub.cbox->move(movearg_Right);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_ShiftRight:
        {
            r = sub.cbox->move(movearg_ShiftRight);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_Last:
        {
            r = sub.cbox->move(movearg_Last);
            assert(r == moveret_OK);
            return r;
        }
        case movearg_First:
        {
            r = sub.cbox->move(movearg_First);
            assert(r == moveret_OK);
            return r;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

moveRet superscriptbox::move(moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        {
            r = super.cbox->move(movearg_Left);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_ShiftLeft:
        {
            r = super.cbox->move(movearg_ShiftLeft);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_Right:
        {
            r = super.cbox->move(movearg_Right);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_ShiftRight:
        {
            r = super.cbox->move(movearg_ShiftRight);
            assert(r == moveret_OK || r == moveret_End);
            return r;
        }
        case movearg_Last:
        {
            r = super.cbox->move(movearg_Last);
            assert(r == moveret_OK);
            return r;
        }
        case movearg_First:
        {
            r = super.cbox->move(movearg_First);
            assert(r == moveret_OK);
            return r;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

moveRet subsuperscriptbox::move(moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        {
            if (cursor == 0)
            {
                r = sub.cbox->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = super.cbox->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = sub.cbox->move(movearg_Last);
                    assert(r == moveret_OK);
                    cursor = 0;
                }
                return moveret_OK;
            }
        }
        case movearg_ShiftLeft:
        {
            if (cursor == 0)
            {
                r = sub.cbox->move(movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = super.cbox->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Right:
        {
            if (cursor == 1)
            {
                r = super.cbox->move(movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 0);
                r = sub.cbox->move(movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = super.cbox->move(movearg_First);
                    assert(r == moveret_OK);
                    cursor = 1;
                }
                return moveret_OK;
            }
        }
        case movearg_ShiftRight:
        {
            if (cursor == 0)
            {
                r = sub.cbox->move(movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = super.cbox->move(movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Last:
        {
            r = super.cbox->move(movearg_Last);
            assert(r == moveret_OK);
            cursor = 1;
            return r;
        }
        case movearg_First:
        {
            r = sub.cbox->move(movearg_First);
            assert(r == moveret_OK);
            cursor = 0;
            return r;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

moveRet underscriptbox::move(moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = under.cbox->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = body.cbox->move(movearg_Last);
                    assert(r == moveret_OK);
                    cursor = 0;
                }
                return moveret_OK;
            }
        }
        case movearg_ShiftLeft:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = under.cbox->move(movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Right:
        {
            if (cursor == 1)
            {
                r = under.cbox->move(movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 0);
                r = body.cbox->move(movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = under.cbox->move(movearg_First);
                    assert(r == moveret_OK);
                    cursor = 1;
                }
                return moveret_OK;
            }
        }
        case movearg_ShiftRight:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = under.cbox->move(movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Last:
        {
            r = under.cbox->move(movearg_Last);
            assert(r == moveret_OK);
            cursor = 1;
            return r;
        }
        case movearg_First:
        {
            r = body.cbox->move(movearg_First);
            assert(r == moveret_OK);
            cursor = 0;
            return r;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

moveRet overscriptbox::move(moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = over.cbox->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = body.cbox->move(movearg_Last);
                    assert(r == moveret_OK);
                    cursor = 0;
                }
                return moveret_OK;
            }
        }
        case movearg_ShiftLeft:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = over.cbox->move(movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Right:
        {
            if (cursor == 1)
            {
                r = over.cbox->move(movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 0);
                r = body.cbox->move(movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = over.cbox->move(movearg_First);
                    assert(r == moveret_OK);
                    cursor = 1;
                }
                return moveret_OK;
            }
        }
        case movearg_ShiftRight:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 1);
                r = over.cbox->move(movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Last:
        {
            r = over.cbox->move(movearg_Last);
            assert(r == moveret_OK);
            cursor = 1;
            return r;
        }
        case movearg_First:
        {
            r = body.cbox->move(movearg_First);
            assert(r == moveret_OK);
            cursor = 0;
            return r;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

moveRet underoverscriptbox::move(moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else if (cursor == 1)
            {
                r = under.cbox->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = body.cbox->move(movearg_Last);
                    assert(r == moveret_OK);
                    cursor = 0;
                }
                return moveret_OK;
            }
            else
            {
                assert(cursor == 2);
                r = over.cbox->move(movearg_Left);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = under.cbox->move(movearg_Last);
                    assert(r == moveret_OK);
                    cursor = 1;
                }
                return moveret_OK;
            }
        }
        case movearg_ShiftLeft:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else if (cursor == 1)
            {
                r = under.cbox->move(movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 2);
                r = over.cbox->move(movearg_ShiftLeft);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Right:
        {
            if (cursor == 2)
            {
                r = over.cbox->move(movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else if (cursor == 1)
            {
                r = under.cbox->move(movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = over.cbox->move(movearg_First);
                    assert(r == moveret_OK);
                    cursor = 2;
                }
                return moveret_OK;
            }
            else
            {
                assert(cursor == 0);
                r = body.cbox->move(movearg_Right);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    r = under.cbox->move(movearg_First);
                    assert(r == moveret_OK);
                    cursor = 1;
                }
                return moveret_OK;
            }
        }
        case movearg_ShiftRight:
        {
            if (cursor == 0)
            {
                r = body.cbox->move(movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else if (cursor == 1)
            {
                r = under.cbox->move(movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
            else
            {
                assert(cursor == 2);
                r = over.cbox->move(movearg_ShiftRight);
                assert(r == moveret_OK || r == moveret_End);
                return r;
            }
        }
        case movearg_Last:
        {
            r = over.cbox->move(movearg_Last);
            assert(r == moveret_OK);
            cursor = 2;
            return r;
        }
        case movearg_First:
        {
            r = body.cbox->move(movearg_First);
            assert(r == moveret_OK);
            cursor = 0;
            return r;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

/* insert ********************************************************************/

insertRet charbox::insert(insertArg m)
{
    insertRet r;
    switch (m)
    {
        default:
        {
            assert(false);
            return insertret_Done;
        }
    }
}

insertRet nullbox::insert(insertArg m)
{
    insertRet r;
    switch (m)
    {
        default:
        {
            assert(false);
            return insertret_Done;
        }
    }
}

insertRet graphics3dbox::insert(insertArg m)
{
    insertRet r;
    switch (m)
    {
        default:
        {
            return insertret_Done;
        }
    }
}

insertRet rootbox::insert(insertArg m)
{
    insertRet r;
    switch (m)
    {
        case insertarg_Fraction:
        {
            if (cursor_b > childcells.size())
            {
                assert(cursor_a < childcells.size());
                childcells[cursor_a].cbox->insert(m);
                return insertret_Done;
            }
            delete_selection();
            rowbox* newrow1 = new rowbox(2, 0,1);
            newrow1->child[0].cibox = iboximm_make(CHAR_Placeholder);
            newrow1->child[1].cibox.ptr = new nullbox();
            rowbox* newrow2 = new rowbox(2, 0,0);
            newrow2->child[0].cibox = iboximm_make(CHAR_Placeholder);
            newrow2->child[1].cibox.ptr = new nullbox();
            fractionbox* newfraction = new fractionbox(newrow1, newrow2, 0);
            rowbox* newrow = new rowbox(2, 0,2);
            newrow->child[0].cibox.ptr = newfraction;
            newrow->child[1].cibox.ptr = new nullbox();
            cellbox* newcell = new cellbox(newrow, cellt_INPUT);
            assert(cursor_a <= childcells.size());
            childcells.insert(childcells.begin() + cursor_a, cellboxarrayelem());
            childcells[cursor_a].cbox = newcell;
            cursor_b = childcells.size() + 1;
            return insertret_Done;
        }
        case insertarg_Sqrt:
        {
            if (cursor_b > childcells.size())
            {
                assert(cursor_a < childcells.size());
                childcells[cursor_a].cbox->insert(m);
                return insertret_Done;
            }
            delete_selection();
            rowbox* newrow1 = new rowbox(2, 0,1);
            newrow1->child[0].cibox = iboximm_make(CHAR_Placeholder);
            newrow1->child[1].cibox.ptr = new nullbox();
            sqrtbox* newsqrt = new sqrtbox(newrow1);
            rowbox* newrow = new rowbox(2, 0,2);
            newrow->child[0].cibox.ptr = newsqrt;
            newrow->child[1].cibox.ptr = new nullbox();
            cellbox* newcell = new cellbox(newrow, cellt_INPUT);
            assert(cursor_a <= childcells.size());
            childcells.insert(childcells.begin() + cursor_a, cellboxarrayelem());
            childcells[cursor_a].cbox = newcell;
            cursor_b = childcells.size() + 1;
            return insertret_Done;
        }
        case insertarg_Newline:
        {
            if (cursor_b > childcells.size())
            {
                assert(cursor_a < childcells.size());
                childcells[cursor_a].cbox->insert(m);
                return insertret_Done;
            }
            delete_selection();
            rowbox* newrow = new rowbox(2, 1,1);
            newrow->child[0].cibox.ptr = new nullbox();
            newrow->child[1].cibox.ptr = new nullbox();
            cellbox* newcell = new cellbox(newrow, cellt_INPUT);
            assert(cursor_a <= childcells.size());
            childcells.insert(childcells.begin() + cursor_a, cellboxarrayelem());
            childcells[cursor_a].cbox = newcell;
            cursor_b = childcells.size() + 1;
            return insertret_Done;
        }
        case insertarg_Subscript:
        case insertarg_Superscript:
        case insertarg_Underscript:
        case insertarg_Overscript:
        {
            if (cursor_b > childcells.size())
            {
                assert(cursor_a < childcells.size());
                childcells[cursor_a].cbox->insert(m);
                return insertret_Done;
            }
            delete_selection();
            rowbox* newrow = new rowbox(2, 0,0);
            newrow->child[0].cibox.ptr = new nullbox();
            cellbox* newcell = new cellbox(newrow, cellt_INPUT);
            assert(cursor_a <= childcells.size());
            childcells.insert(childcells.begin() + cursor_a, cellboxarrayelem());
            childcells[cursor_a].cbox = newcell;
            cursor_b = childcells.size() + 1;
            return insertret_Done;
        }
        case insertarg_Graphics3D:
        {
            if (cursor_b > childcells.size())
            {
                assert(cursor_a < childcells.size());
                childcells[cursor_a].cbox->insert(m);
                return insertret_Done;
            }
            delete_selection();
            rowbox* newrow = new rowbox(2, 1,1);
            newrow->child[0].cibox.ptr = new graphics3dbox();
            newrow->child[1].cibox.ptr = new nullbox();
            cellbox* newcell = new cellbox(newrow, cellt_INPUT);
            assert(cursor_a <= childcells.size());
            childcells.insert(childcells.begin() + cursor_a, cellboxarrayelem());
            childcells[cursor_a].cbox = newcell;
            cursor_b = childcells.size() + 1;
            return insertret_Done;
        }
        default:
        {
            assert(false);
            return insertret_Done;
        }
    }
}

insertRet cellbox::insert(insertArg m)
{
    if (cursor == 0)
    {
        return body.cbox->insert(m);
    }
    else
    {
        assert(cursor == 1);
        return label.cbox->insert(m);
    }
}


void rowbox::select_placeholder_if_possible()
{
    if (child.size() == 2 && ibox_is_char(child[0].cibox, CHAR_Placeholder))
    {
        cursor_b = cursor_a == 0 ? 1 : 0;
    }
}

void rowbox::select_prev_if_possible()
{
    int32_t npi, i;
    int32_t pi = cursor_a;

    npi = pi;
    while (npi > 0)
    {
        int32_t mtype, ptype;
        if (!ibox_is_char(child[npi - 1].cibox))
        {
            if (pi == npi && ibox_is_node(child[npi - 1].cibox))
                npi--;
            break;
        }
        ptype = ibox_type(child[npi - 1].cibox)&65535;
        if (ptype == ']' || ptype == '}' || ptype == ')' || ptype == CHAR_RightDoubleBracket)
        {
            std::stack<int32_t> estack;
            estack.push(ptype);
            while (--npi > 0 && estack.size() > 0)
            {
                if (!ibox_is_char(child[npi - 1].cibox)) {
                    continue;
                }
                ptype = ibox_type(child[npi - 1].cibox)&65535;
                if (ptype == ']' || ptype == '}' || ptype == ')' || ptype == CHAR_RightDoubleBracket) {
                    mtype = 0;
                    estack.push(ptype);
                } else if (ptype == '[') {
                    mtype = ']';
                } else if (ptype == CHAR_LeftDoubleBracket) {
                    mtype = CHAR_RightDoubleBracket;
                } else if (ptype == '(') {
                    mtype = ')';
                } else if (ptype == '{') {
                    mtype = '}';
                } else {
                    continue;
                }
                if (estack.top() == mtype) {
                    estack.pop();
                } else {
                    estack.push(ptype);
                }                
            }
        }
        else if (isletterchar(ptype) || ('0' <= ptype && ptype <= '9'))
        {
            --npi;
        }
        else if (pi == npi && (ptype == CHAR_Sum || ptype == CHAR_Product))
        {
            --npi;
            break;
        }
        else
        {
            break;
        }
    }
    if (npi < pi)
    {
        cursor_b = npi;
    }
}


insertRet rowbox::insert(insertArg m)
{
//printf("rowbox::insert called m = %d\n", m);

    flags &= ~(BNFLAG_MEASURED | BNFLAG_COLORED);

    insertRet r;
    switch (m)
    {
        case insertarg_Fraction:
        {
            if (cursor_b >= child.size())
            {
                return ibox_to_ptr(child[cursor_a].cibox)->insert(m);
            }
            else if (cursor_a == cursor_b)
            {
                select_prev_if_possible();
            }
            if (cursor_a == cursor_b)
            {
                rowbox* newrow1 = new rowbox(2, 0,1);
                newrow1->child[0].cibox = iboximm_make(CHAR_Placeholder);
                newrow1->child[1].cibox.ptr = new nullbox();
                rowbox* newrow2 = new rowbox(2, 0,0);
                newrow2->child[0].cibox = iboximm_make(CHAR_Placeholder);
                newrow2->child[1].cibox.ptr = new nullbox();
                fractionbox* newfraction = new fractionbox(newrow1, newrow2, 0);
                child.insert(child.begin() + cursor_a, iboxarrayelem(newfraction));
                cursor_b = child.size();
                return insertret_Done;
            }
            else
            {
                int32_t left = std::min(cursor_a, cursor_b);
                int32_t right = std::max(cursor_a, cursor_b);
                rowbox* newrow1 = new rowbox(right - left + 1, 0,0);
                for (int32_t i = left; i < right; i++)
                    newrow1->child[i - left].cibox = child[i].cibox;
                newrow1->child[right - left].cibox.ptr = new nullbox();
                child.erase(child.begin() + left, child.begin() + right);
                rowbox* newrow2 = new rowbox(2, 0,1);
                newrow2->child[0].cibox = iboximm_make(CHAR_Placeholder);
                newrow2->child[1].cibox.ptr = new nullbox();
                fractionbox* newfraction = new fractionbox(newrow1, newrow2, 1);                
                child.insert(child.begin() + left, iboxarrayelem(newfraction));
                cursor_a = left;
                cursor_b = child.size();
                return insertret_Done;
            }
        }
        case insertarg_Sqrt:
        {
            if (cursor_b >= child.size())
            {
                return ibox_to_ptr(child[cursor_a].cibox)->insert(m);
            }
            else if (cursor_a == cursor_b)
            {
                rowbox* newrow1 = new rowbox(2, 0,1);
                newrow1->child[0].cibox = iboximm_make(CHAR_Placeholder);
                newrow1->child[1].cibox.ptr = new nullbox();
                sqrtbox* newsqrt = new sqrtbox(newrow1);
                child.insert(child.begin() + cursor_a, iboxarrayelem(newsqrt));
                cursor_b = child.size();
                return insertret_Done;
            }
            else
            {
                int32_t left = std::min(cursor_a, cursor_b);
                int32_t right = std::max(cursor_a, cursor_b);
                rowbox* newrow1 = new rowbox(right - left + 1, 0, (right - left == 1 && ibox_is_char(child[0].cibox)) ? 1 : 0);
                for (int32_t i = left; i < right; i++)
                    newrow1->child[i - left].cibox = child[i].cibox;
                newrow1->child[right - left].cibox.ptr = new nullbox();
                child.erase(child.begin() + left, child.begin() + right);
                sqrtbox* newsqrt = new sqrtbox(newrow1);                
                child.insert(child.begin() + left, iboxarrayelem(newsqrt));
                cursor_a = left;
                cursor_b = child.size();
                return insertret_Done;
            }
        }
        case insertarg_Newline:
        {
            if (cursor_b >= child.size())
            {
                return ibox_to_ptr(child[cursor_a].cibox)->insert(m);
            }
            delete_selection();
            child.insert(child.begin() + cursor_a, iboxarrayelem(new nullbox()));
            cursor_b = cursor_a = cursor_a + 1;
            return insertret_Done;
        }
        case insertarg_Subscript:
        {
            if (cursor_b >= child.size())
            {
                return ibox_to_ptr(child[cursor_a].cibox)->insert(m);
            }
            else if (cursor_a == cursor_b)
            {
                rowbox* newrow1 = new rowbox(2, 0,1);
                newrow1->child[0].cibox = iboximm_make(CHAR_Placeholder);
                newrow1->child[1].cibox.ptr = new nullbox();
                subscriptbox* newsubscript = new subscriptbox(newrow1);
                child.insert(child.begin() + cursor_a, iboxarrayelem(newsubscript));
                cursor_b = child.size();
                return insertret_Done;
            }
            else
            {
                int32_t left = std::min(cursor_a, cursor_b);
                int32_t right = std::max(cursor_a, cursor_b);
                rowbox* newrow1 = new rowbox(right - left + 1, 0, (right - left == 1 && ibox_is_char(child[0].cibox)) ? 1 : 0);
                for (int32_t i = left; i < right; i++)
                    newrow1->child[i - left].cibox = child[i].cibox;
                newrow1->child[right - left].cibox.ptr = new nullbox();
                child.erase(child.begin() + left, child.begin() + right);
                subscriptbox* newsubscript = new subscriptbox(newrow1);
                child.insert(child.begin() + left, iboxarrayelem(newsubscript));
                cursor_a = left;
                cursor_b = child.size();
                return insertret_Done;
            }
        }
        case insertarg_Superscript:
        {
            if (cursor_b >= child.size())
            {
                return ibox_to_ptr(child[cursor_a].cibox)->insert(m);
            }
            else if (cursor_a == cursor_b)
            {
                rowbox* newrow1 = new rowbox(2, 0,1);
                newrow1->child[0].cibox = iboximm_make(CHAR_Placeholder);
                newrow1->child[1].cibox.ptr = new nullbox();
                superscriptbox* newsuperscript = new superscriptbox(newrow1);
                child.insert(child.begin() + cursor_a, iboxarrayelem(newsuperscript));
                cursor_b = child.size();
                return insertret_Done;
            }
            else
            {
                int32_t left = std::min(cursor_a, cursor_b);
                int32_t right = std::max(cursor_a, cursor_b);
                rowbox* newrow1 = new rowbox(right - left + 1, 0, (right - left == 1 && ibox_is_char(child[0].cibox)) ? 1 : 0);
                for (int32_t i = left; i < right; i++)
                    newrow1->child[i - left].cibox = child[i].cibox;
                newrow1->child[right - left].cibox.ptr = new nullbox();
                child.erase(child.begin() + left, child.begin() + right);
                superscriptbox* newsuperscript = new superscriptbox(newrow1);
                child.insert(child.begin() + left, iboxarrayelem(newsuperscript));
                cursor_a = left;
                cursor_b = child.size();
                return insertret_Done;
            }
        }
        case insertarg_Underscript:
        {
            if (cursor_b >= child.size())
            {
                return ibox_to_ptr(child[cursor_a].cibox)->insert(m);
            }
            else if (cursor_a == cursor_b)
            {
                select_prev_if_possible();
            }
            if (cursor_a == cursor_b)
            {
                rowbox* newrow1 = new rowbox(2, 0,1);
                newrow1->child[0].cibox = iboximm_make(CHAR_Placeholder);
                newrow1->child[1].cibox.ptr = new nullbox();
                rowbox* newrow2 = new rowbox(2, 0,0);
                newrow2->child[0].cibox = iboximm_make(CHAR_Placeholder);
                newrow2->child[1].cibox.ptr = new nullbox();
                underscriptbox* newunderscript = new underscriptbox(newrow1, newrow2, 0);
                child.insert(child.begin() + cursor_a, iboxarrayelem(newunderscript));
                cursor_b = child.size();
                return insertret_Done;
            }
            else
            {
                int32_t left = std::min(cursor_a, cursor_b);
                int32_t right = std::max(cursor_a, cursor_b);
                rowbox* newrow1 = new rowbox(right - left + 1, 0,0);
                for (int32_t i = left; i < right; i++)
                    newrow1->child[i - left].cibox = child[i].cibox;
                newrow1->child[right - left].cibox.ptr = new nullbox();
                child.erase(child.begin() + left, child.begin() + right);
                rowbox* newrow2 = new rowbox(2, 0,1);
                newrow2->child[0].cibox = iboximm_make(CHAR_Placeholder);
                newrow2->child[1].cibox.ptr = new nullbox();
                underscriptbox* newunderscript = new underscriptbox(newrow1, newrow2, 1);                
                child.insert(child.begin() + left, iboxarrayelem(newunderscript));
                cursor_a = left;
                cursor_b = child.size();
                return insertret_Done;
            }
        }
        case insertarg_Overscript:
        {
            if (cursor_b >= child.size())
            {
                return ibox_to_ptr(child[cursor_a].cibox)->insert(m);
            }
            else if (cursor_a == cursor_b)
            {
                select_prev_if_possible();
            }
            if (cursor_a == cursor_b)
            {
                rowbox* newrow1 = new rowbox(2, 0,1);
                newrow1->child[0].cibox = iboximm_make(CHAR_Placeholder);
                newrow1->child[1].cibox.ptr = new nullbox();
                rowbox* newrow2 = new rowbox(2, 0,0);
                newrow2->child[0].cibox = iboximm_make(CHAR_Placeholder);
                newrow2->child[1].cibox.ptr = new nullbox();
                overscriptbox* newoverscript = new overscriptbox(newrow1, newrow2, 0);
                child.insert(child.begin() + cursor_a, iboxarrayelem(newoverscript));
                cursor_b = child.size();
                return insertret_Done;
            }
            else
            {
                int32_t left = std::min(cursor_a, cursor_b);
                int32_t right = std::max(cursor_a, cursor_b);
                rowbox* newrow1 = new rowbox(right - left + 1, 0,0);
                for (int32_t i = left; i < right; i++)
                    newrow1->child[i - left].cibox = child[i].cibox;
                newrow1->child[right - left].cibox.ptr = new nullbox();
                child.erase(child.begin() + left, child.begin() + right);
                rowbox* newrow2 = new rowbox(2, 0,1);
                newrow2->child[0].cibox = iboximm_make(CHAR_Placeholder);
                newrow2->child[1].cibox.ptr = new nullbox();
                overscriptbox* newoverscript = new overscriptbox(newrow1, newrow2, 1);                
                child.insert(child.begin() + left, iboxarrayelem(newoverscript));
                cursor_a = left;
                cursor_b = child.size();
                return insertret_Done;
            }
        }
        case insertarg_Graphics3D:
        {
            if (cursor_b >= child.size())
            {
                return ibox_to_ptr(child[cursor_a].cibox)->insert(m);
            }
            delete_selection();
            child.insert(child.begin() + cursor_a, iboxarrayelem(new graphics3dbox()));
            cursor_b = cursor_a = cursor_a + 1;
            return insertret_Done;
        }
        default:
        {
            assert(false);
            return insertret_Done;
        }
    }
}

insertRet fractionbox::insert(insertArg m)
{
    if (cursor == 0)
    {
        return num.cbox->insert(m);
    }
    else
    {
        assert(cursor == 1);
        return den.cbox->insert(m);
    }
}

insertRet sqrtbox::insert(insertArg m)
{
    return inside.cbox->insert(m);
}

insertRet subscriptbox::insert(insertArg m)
{
    return sub.cbox->insert(m);
}

insertRet superscriptbox::insert(insertArg m)
{
    return super.cbox->insert(m);
}

insertRet subsuperscriptbox::insert(insertArg m)
{
    if (cursor == 0)
    {
        return sub.cbox->insert(m);
    }
    else
    {
        assert(cursor == 1);
        return super.cbox->insert(m);
    }
}

insertRet underscriptbox::insert(insertArg m)
{
    if (cursor == 0)
    {
        return body.cbox->insert(m);
    }
    else
    {
        assert(cursor == 1);
        return under.cbox->insert(m);
    }
}

insertRet overscriptbox::insert(insertArg m)
{
    if (cursor == 0)
    {
        return body.cbox->insert(m);
    }
    else
    {
        assert(cursor == 1);
        return over.cbox->insert(m);
    }
}

insertRet underoverscriptbox::insert(insertArg m)
{
    if (cursor == 0)
    {
        return body.cbox->insert(m);
    }
    else if (cursor == 1)
    {
        return under.cbox->insert(m);
    }
    else
    {
        assert(cursor == 2);
        return over.cbox->insert(m);
    }
}


/* remove ********************************************************************/

removeRet nullbox::remove(boxbase*&b, removeArg m)
{
    assert(b == nullptr);
    removeRet r;

    switch (m)
    {
        case removearg_Left:
        {
            assert(false);
            return removeret_Bad;
        }
        case removearg_Right:
        {
            assert(false);
            return removeret_Bad;
        }
        default:
        {
            assert(false);
            return removeret_Bad;
        }
    }
}

removeRet charbox::remove(boxbase*&b, removeArg m)
{
    assert(b == nullptr);
    removeRet r;

    switch (m)
    {
        case removearg_Left:
        {
            assert(false);
            return removeret_Bad;
        }
        case removearg_Right:
        {
            assert(false);
            return removeret_Bad;
        }
        default:
        {
            assert(false);
            return removeret_Bad;
        }
    }
}

removeRet graphics3dbox::remove(boxbase*&b, removeArg m)
{
    assert(b == nullptr);
    removeRet r;

    switch (m)
    {
        case removearg_Left:
        {
            return removeret_End;
        }
        case removearg_Right:
        {
            return removeret_End;
        }
        default:
        {
            assert(false);
            return removeret_Bad;
        }
    }
}


removeRet rootbox::remove(boxbase*&b, removeArg m)
{
//std::cout << "cellbox::remove called" << std::endl;
    assert(b == nullptr);
    removeRet r;

    if (cursor_b > childcells.size())
    {
        assert(cursor_a < childcells.size());
        r = childcells[cursor_a].cbox->remove(b, m);
        return removeret_OK;
    }
    else
    {
        delete_selection();
        return removeret_OK;
    }
}

removeRet cellbox::remove(boxbase*&b, removeArg m)
{
//std::cout << "cellbox::remove called" << std::endl;
    assert(b == nullptr);
    removeRet r;

    if (cursor == 0)
    {
        r = body.cbox->remove(b, m);
        assert(r == removeret_OK || r == removeret_End);
        return removeret_OK;
    }
    else
    {
        assert(cursor == 1);
        r = label.cbox->remove(b, m);
        assert(r == removeret_OK || r == removeret_End);
        if (r == removeret_End)
        {
            cursor == 0;
            body.cbox->cursor_b = body.cbox->cursor_a = 0;
        }
        return removeret_OK;
    }
}

removeRet rowbox::remove(boxbase*&b, removeArg m)
{
//std::cout << "rowbox::remove called" << std::endl;
    assert(b == nullptr);
    removeRet r;
    moveRet s;

    flags &= ~(BNFLAG_MEASURED | BNFLAG_COLORED);

    if (cursor_b >= child.size())
    {
        assert(cursor_a < child.size());
        r = ibox_to_ptr(child[cursor_a].cibox)->remove(b, m);
        assert(r == removeret_OK || r == removeret_Replace);
        if (r == removeret_Replace)
        {
            delete ibox_to_ptr(child[cursor_a].cibox);
            if (b == nullptr)
            {
                child.erase(child.begin() + cursor_a);
                cursor_b = cursor_a;
                return removeret_OK;
            }
            else if (b->get_type() == BNTYPE_ROW)
            {
                child.erase(child.begin() + cursor_a);
                cursor_b = cursor_a;
                rowbox* row = dynamic_cast<rowbox*>(b);
                child.insert(child.begin() + cursor_a, row->child.size() , iboxarrayelem(iboximm_make(0)));
                for (int32_t j = 0; j < row->child.size(); j++)
                {
                    child[cursor_a + j].cibox = row->child[j].cibox;
                    row->child[j].cibox = iboximm_make(0);
                }
                if (m == removearg_Left)
                    cursor_b = cursor_a = cursor_a + row->child.size();
                delete b;
                b = nullptr;
                return removeret_OK;
            }
            else
            {
                child[cursor_a].cibox.ptr = b;
                b = nullptr;
                return removeret_OK;
            }
        }
        else
        {
            return removeret_OK;
        }
    }
    else if (cursor_a == cursor_b)
    {
        assert(cursor_a < child.size());
        switch (m)
        {
            case removearg_Left:
            {
                if (cursor_a <= 0)
                {
                    return removeret_End;
                }
                else if (ibox_is_char(child[cursor_a - 1].cibox))
                {
                    if (ibox_is_ptr(child[cursor_a - 1].cibox))
                        delete ibox_to_ptr(child[cursor_a - 1].cibox);
                    child.erase(child.begin() + cursor_a - 1);
                    cursor_b = cursor_a = cursor_a - 1;
                    return removeret_OK;
                }
                else
                {
                    cursor_a--;
                    s = ibox_to_ptr(child[cursor_a].cibox)->move(movearg_Last);
                    assert(s == moveret_OK || s == moveret_End);
                    if (s == moveret_End)
                    {
                        delete ibox_to_ptr(child[cursor_a].cibox);
                        child.erase(child.begin() + cursor_a);
                        cursor_b = cursor_a;
                    }
                    else
                    {
                        cursor_b = child.size();
                    }
                    return removeret_OK;
                }
            }
            case removearg_Right:
            {
                if (cursor_a + 1 >= child.size())
                {
                    return removeret_End;
                }
                else if (ibox_is_char(child[cursor_a].cibox))
                {
                    if (ibox_is_ptr(child[cursor_a].cibox))
                        delete ibox_to_ptr(child[cursor_a].cibox);
                    child.erase(child.begin() + cursor_a);
                    return removeret_OK;
                }
                else
                {
                    s = ibox_to_ptr(child[cursor_a].cibox)->move(movearg_First);
                    assert(s == moveret_OK || s == moveret_End);
                    if (s == moveret_End)
                    {
                        delete ibox_to_ptr(child[cursor_a].cibox);
                        child.erase(child.begin() + cursor_a);
                        return removeret_OK;
                    }
                    else
                    {
                        cursor_b = child.size();
                        return removeret_OK;
                    }
                }
            }
            default:
            {
                assert(false);
                return removeret_Bad;
            }
        }
    }
    else
    {
        delete_selection();
        return removeret_OK;
    }
}

removeRet subscriptbox::remove(boxbase*&b, removeArg m)
{
    removeRet r;
    if (sub.cbox->is_selected_placeholder())
    {
        b = nullptr;
        return removeret_Replace;
    }
    else
    {
        r = sub.cbox->remove(b, m);
        assert(r == removeret_End || r == removeret_OK);
        if (r == removeret_End)
        {
            if (sub.cbox->child.size() > 1)
            {
                rowbox * row = new rowbox(sub.cbox->child.size() - 1, 0, 0);
                for (int32_t i = 0; i + 1 < sub.cbox->child.size(); i++)
                {
                    row->child[i].cibox = sub.cbox->child[i].cibox;
                    sub.cbox->child[i].cibox = iboximm_make(0);
                }
                b = row;
            }
            return removeret_Replace;
        }
        else
        {
            if (sub.cbox->child.size() == 1)
            {
                sub.cbox->child.insert(sub.cbox->child.begin(), iboximm_make(CHAR_Placeholder));
                sub.cbox->cursor_a = 0;
                sub.cbox->cursor_b = 1;
            }
            return removeret_OK;
        }
    }
}

removeRet superscriptbox::remove(boxbase*&b, removeArg m)
{
    removeRet r;
    if (super.cbox->is_selected_placeholder())
    {
        b = nullptr;
        return removeret_Replace;
    }
    else
    {
        r = super.cbox->remove(b, m);
        assert(r == removeret_End || r == removeret_OK);
        if (r == removeret_End)
        {
            if (super.cbox->child.size() > 1)
            {
                rowbox * row = new rowbox(super.cbox->child.size() - 1, 0, 0);
                for (int32_t i = 0; i + 1 < super.cbox->child.size(); i++)
                {
                    row->child[i].cibox = super.cbox->child[i].cibox;
                    super.cbox->child[i].cibox = iboximm_make(0);
                }
                b = row;
            }
            return removeret_Replace;
        }
        else
        {
            if (super.cbox->child.size() == 1)
            {
                super.cbox->child.insert(super.cbox->child.begin(), iboximm_make(CHAR_Placeholder));
                super.cbox->cursor_a = 0;
                super.cbox->cursor_b = 1;
            }
            return removeret_OK;
        }
    }
}

removeRet subsuperscriptbox::remove(boxbase*&b, removeArg m)
{
    assert(b == nullptr);
    removeRet r;

    if (cursor == 0)
    {
        if (sub.cbox->is_selected_placeholder())
        {
            rowbox * row = new rowbox(super.cbox->child.size(), super.cbox->child.size() - 1, super.cbox->child.size() - 1);
            for (int32_t i = 0; i < super.cbox->child.size(); i++)
            {
                row->child[i].cibox = super.cbox->child[i].cibox;
                super.cbox->child[i].cibox = iboximm_make(0);
            }
            b = new superscriptbox(row);
            return removeret_Replace;
        }
        else
        {
            r = sub.cbox->remove(b, m);
            assert(r == removeret_End || r == removeret_OK);
            if (r == removeret_End)
            {
                if (sub.cbox->child.size() > 1)
                {
                    rowbox * row = new rowbox(sub.cbox->child.size() - 1, 0, 0);
                    for (int32_t i = 0; i + 1 < sub.cbox->child.size(); i++)
                    {
                        row->child[i].cibox = sub.cbox->child[i].cibox;
                        sub.cbox->child[i].cibox = iboximm_make(0);
                    }
                    b = row;
                }
                return removeret_Replace;
            }
            else
            {
                if (sub.cbox->child.size() == 1)
                {
                    sub.cbox->child.insert(sub.cbox->child.begin(), iboximm_make(CHAR_Placeholder));
                    sub.cbox->cursor_a = 0;
                    sub.cbox->cursor_b = 1;
                }
                return removeret_OK;
            }
        }
    }
    else
    {
        assert(cursor == 1);
        if (super.cbox->is_selected_placeholder())
        {
            rowbox * row = new rowbox(sub.cbox->child.size(), sub.cbox->child.size() - 1, sub.cbox->child.size() - 1);
            for (int32_t i = 0; i < sub.cbox->child.size(); i++)
            {
                row->child[i].cibox = sub.cbox->child[i].cibox;
                sub.cbox->child[i].cibox = iboximm_make(0);
            }
            b = new subscriptbox(row);
            return removeret_Replace;
        }
        else
        {
            r = super.cbox->remove(b, m);
            assert(r == removeret_End || r == removeret_OK);
            if (r == removeret_End)
            {
                rowbox * row = new rowbox(sub.cbox->child.size() - 1, 0, 0);
                for (int32_t i = 0; i + 1 < sub.cbox->child.size(); i++)
                {
                    row->child[i].cibox = sub.cbox->child[i].cibox;
                    sub.cbox->child[i].cibox = iboximm_make(0);
                }
                b = row;
                return removeret_Replace;
            }
            else
            {
                if (super.cbox->child.size() == 1)
                {
                    super.cbox->child.insert(super.cbox->child.begin(), iboximm_make(CHAR_Placeholder));
                    super.cbox->cursor_a = 0;
                    super.cbox->cursor_b = 1;
                }
                return removeret_OK;
            }
        }
    }
}

removeRet underscriptbox::remove(boxbase*&b, removeArg m)
{
    removeRet r;
    switch (m)
    {
        case removearg_Left:
        {
            assert(false);
            return removeret_Bad;
        }
        case removearg_Right:
        {
            assert(false);
            return removeret_Bad;
        }
        default:
        {
            assert(false);
            return removeret_Bad;
        }
    }
}

removeRet overscriptbox::remove(boxbase*&b, removeArg m)
{
    removeRet r;
    switch (m)
    {
        case removearg_Left:
        {
            assert(false);
            return removeret_Bad;
        }
        case removearg_Right:
        {
            assert(false);
            return removeret_Bad;
        }
        default:
        {
            assert(false);
            return removeret_Bad;
        }
    }
}

removeRet underoverscriptbox::remove(boxbase*&b, removeArg m)
{
    removeRet r;
    switch (m)
    {
        case removearg_Left:
        {
            assert(false);
            return removeret_Bad;
        }
        case removearg_Right:
        {
            assert(false);
            return removeret_Bad;
        }
        default:
        {
            assert(false);
            return removeret_Bad;
        }
    }
}

removeRet fractionbox::remove(boxbase*&b, removeArg m)
{
    assert(b == nullptr);
    removeRet r;

    if (cursor == 0)
    {
        if (num.cbox->is_selected_placeholder())
        {
            b = nullptr;
            return removeret_Replace;
        }
        else
        {
            r = num.cbox->remove(b, m);
            assert(r == removeret_End || r == removeret_OK);
            if (r == removeret_End)
            {
                if (num.cbox->child.size() > 1)
                {
                    rowbox * row = new rowbox(num.cbox->child.size() - 1, 0, 0);
                    for (int32_t i = 0; i + 1 < num.cbox->child.size(); i++)
                    {
                        row->child[i].cibox = num.cbox->child[i].cibox;
                        num.cbox->child[i].cibox = iboximm_make(0);
                    }
                    b = row;
                }
                return removeret_Replace;
            }
            else
            {
                if (num.cbox->child.size() == 1)
                {
                    num.cbox->child.insert(num.cbox->child.begin(), iboximm_make(CHAR_Placeholder));
                    num.cbox->cursor_a = 0;
                    num.cbox->cursor_b = 1;
                }
                return removeret_OK;
            }
        }
    }
    else
    {
        assert(cursor == 1);
        if (den.cbox->is_selected_placeholder())
        {
printf("deleting denominator\n");
            rowbox * row = new rowbox(num.cbox->child.size() - 1, 0, 0);
            for (int32_t i = 0; i + 1 < num.cbox->child.size(); i++)
            {
                row->child[i].cibox = num.cbox->child[i].cibox;
                num.cbox->child[i].cibox = iboximm_make(0);
            }
            b = row;

std::cout << "b: " << std::endl;
b->print(0,0,0);

            return removeret_Replace;
        }
        else
        {
            r = den.cbox->remove(b, m);
            assert(r == removeret_End || r == removeret_OK);
            if (r == removeret_End)
            {
                rowbox * row = new rowbox(num.cbox->child.size() - 1, 0, 0);
                for (int32_t i = 0; i + 1 < num.cbox->child.size(); i++)
                {
                    row->child[i].cibox = num.cbox->child[i].cibox;
                    num.cbox->child[i].cibox = iboximm_make(0);
                }
                b = row;
                return removeret_Replace;
            }
            else
            {
                if (den.cbox->child.size() == 1)
                {
                    den.cbox->child.insert(den.cbox->child.begin(), iboximm_make(CHAR_Placeholder));
                    den.cbox->cursor_a = 0;
                    den.cbox->cursor_b = 1;
                }
                return removeret_OK;
            }
        }
    }
}

removeRet sqrtbox::remove(boxbase*&b, removeArg m)
{
    assert(b == nullptr);
    removeRet r;

    if (inside.cbox->is_selected_placeholder())
    {
        b = nullptr;
        return removeret_Replace;
    }
    else
    {
        r = inside.cbox->remove(b, m);
        assert(r == removeret_End || r == removeret_OK);
        if (r == removeret_End)
        {
            if (inside.cbox->child.size() > 1)
            {
                rowbox * row = new rowbox(inside.cbox->child.size() - 1, 0, 0);
                for (int32_t i = 0; i + 1 < inside.cbox->child.size(); i++)
                {
                    row->child[i].cibox = inside.cbox->child[i].cibox;
                    inside.cbox->child[i].cibox = iboximm_make(0);
                }
                b = row;
            }
            return removeret_Replace;
        }
        else
        {
            if (inside.cbox->child.size() == 1)
            {
                inside.cbox->child.insert(inside.cbox->child.begin(), iboximm_make(CHAR_Placeholder));
                inside.cbox->cursor_a = 0;
                inside.cbox->cursor_b = 1;
            }
            return removeret_OK;
        }
    }
}


/* get_ex ********************************************************************/

void rootbox::key_shiftenter(notebook* nb)
{
    if (cursor_b > childcells.size())
    {
        uex e(childcells[cursor_a].cbox->body.cbox->get_ex());
        swrite_byte(fp_in, CMD_EXPR);
        e.reset(emake_node(gs.sym_sEnterTextPacket.copy(), e.release()));
        swrite_ex(fp_in, e.get());
        fflush(fp_in);

        /* put line number on our cell */

        if (childcells[cursor_a].cbox->label.cbox != nullptr)
        {
            delete childcells[cursor_a].cbox->label.cbox;
            childcells[cursor_a].cbox->label.cbox = nullptr;
        }

        if (nb->in_name.get() != nullptr)
        {
            boxbase * label = boxbase_convert_from_ex(nb->in_name.get());
            if (label != nullptr)
            {
                if (label->get_type() != BNTYPE_ROW)
                {
                    delete label;
                }
                else
                {
                    childcells[cursor_a].cbox->label.cbox = dynamic_cast<rowbox*>(label);
                }
            }
        }

        nb->print_location = cursor_b = cursor_a = cursor_a + 1;

        /* delete supid stuff after our cell */
        int32_t left = cursor_a;
        int32_t right = cursor_a;
        while (right < childcells.size()
                && (  childcells[right].cbox->celltype == cellt_OUTPUT
                   || childcells[right].cbox->celltype == cellt_PRINT
                   || childcells[right].cbox->celltype == cellt_MESSAGE))
        {
            right++;
        }
        for (auto i = childcells.begin() + left; i != childcells.begin() + right; ++i)
            delete (*i).cbox;
        childcells.erase(childcells.begin() + left, childcells.begin() + right);
    }
    else
    {
        assert(false);
    }
}


ex charbox::get_ex()
{
    std::string s;
    stdstring_pushback_char16(s, get_type()&65535);
    return emake_str_move(s);
}

ex nullbox::get_ex()
{
    return gs.sym_sNull.copy();
}

ex graphics3dbox::get_ex()
{
    return emake_node(gs.sym_sGraphics3D.copy());
}

ex rootbox::get_ex()
{
    std::vector<wex> v;
    for (int32_t i = 0; i < childcells.size(); i++)
        v.push_back(wex(childcells[i].cbox->get_ex()));
    return emake_node(gs.sym_sNotebook.copy(), v);
}

ex cellbox::get_ex()
{
    std::vector<wex> v;
    v.push_back(wex(body.cbox->get_ex()));
    v.push_back(wex(emake_str(
        celltype == CELLTYPE_INPUT ? "Input" :
        celltype == CELLTYPE_OUTPUT ? "Output" :
        celltype == CELLTYPE_MESSAGE ? "Message" :
        celltype == CELLTYPE_PRINT ? "Print" :
        celltype == CELLTYPE_TEXT ? "Text" :
        celltype == CELLTYPE_BOLDTEXT ? "BoldText" :
        celltype == CELLTYPE_SUBSUBSECTION ? "Subsubsection" :
        celltype == CELLTYPE_SUBSECTION ? "Subsection" :
        celltype == CELLTYPE_SECTION ?  "Section" :
                                        "Title"      )));
    if (label.cbox != nullptr)
    {
        ex t = label.cbox->get_ex();
        v.push_back(wex(emake_node(gs.sym_sRule.copy(), gs.sym_sCellType.copy(), t)));
    }
    return emake_node(gs.sym_sCell.copy(), v);
}

ex rowbox::get_ex()
{
    std::vector<wex> v;
    for (int32_t i = 0; i + 1 < child.size(); i++)
    {
        if (ibox_is_char(child[i].cibox))
        {
            if (v.empty() || !eis_str(v.back().get()))
            {
                v.push_back(wex(emake_str()));
            }
            stdstring_pushback_char16(estr_string(v.back().get()), ibox_type(child[i].cibox)&65535);
        }
        else
        {
            v.push_back(wex(ibox_to_ptr(child[i].cibox)->get_ex()));
        }
    }
    if (v.empty())
    {
        return emake_str();
    }
    else if (v.size() == 1 && eis_str(v[0].get()))
    {
        return ecopy(v[0].get());
    }
    else
    {
        ex t = emake_node(gs.sym_sList.copy(), v);
        return emake_node(gs.sym_sRowBox.copy(), t);
    }
}

ex sqrtbox::get_ex()
{
    ex t = inside.cbox->get_ex();
    return emake_node(gs.sym_sSqrtBox.copy(), t);
}

ex subscriptbox::get_ex()
{
    ex t = sub.cbox->get_ex();
    return emake_node(gs.sym_sSubscriptBox.copy(), t);
}

ex superscriptbox::get_ex()
{
    ex t = super.cbox->get_ex();
    return emake_node(gs.sym_sSuperscriptBox.copy(), t);
}

ex subsuperscriptbox::get_ex()
{
    uex t1(sub.cbox->get_ex());
    ex t2 = super.cbox->get_ex();
    return emake_node(gs.sym_sSuperscriptBox.copy(), t1.release(), t2);
}

ex fractionbox::get_ex()
{
    uex t1(num.cbox->get_ex());
    ex t2 = den.cbox->get_ex();
    return emake_node(gs.sym_sFractionBox.copy(), t1.release(), t2);
}

ex overscriptbox::get_ex()
{
    uex t1(body.cbox->get_ex());
    ex t2 = over.cbox->get_ex();
    return emake_node(gs.sym_sFractionBox.copy(), t1.release(), t2);
}

ex underscriptbox::get_ex()
{
    uex t1(body.cbox->get_ex());
    ex t2 = under.cbox->get_ex();
    return emake_node(gs.sym_sFractionBox.copy(), t1.release(), t2);
}

ex underoverscriptbox::get_ex()
{
    uex t1(body.cbox->get_ex());
    uex t2(under.cbox->get_ex());
    ex t3 = over.cbox->get_ex();
    return emake_node(gs.sym_sFractionBox.copy(), t1.release(), t2.release(), t3);
}

/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/

int boxrow_child_sizey(boxnode* r, int i)
{
    int type, sizex, sizey, centery;
    int fs = boxnode_extra0(r);
    int default_sizey = fontsize_default_sizey(fs);
    int default_centery = fontsize_default_centery(fs);
    bget_header(type, sizex, sizey, centery, boxnode_child(r,i));
    return sizey;
}

int boxrow_child_centery(boxnode* r, int i)
{
    int type, sizex, sizey, centery;
    int fs = boxnode_extra0(r);
    int default_sizey = fontsize_default_sizey(fs);
    int default_centery = fontsize_default_centery(fs);
    bget_header(type, sizex, sizey, centery, boxnode_child(r,i));
    return centery;
}


void printindent(box cursorbox, box b, int indent)
{
    while (--indent >= 0)
        printf("       ");

    if (bare_same(b, cursorbox)) {
        printf("|");
    }
}


void boxnode_print(box cursorbox, box b, int depth) {

    printf("%016p ",b);

    if (!bis_node(b))
    {
        if (bis_ptr(b))
        {
            if (bis_special(b))
            {
                printf("SPECIAL(%d)\n", bptr_type(b));
            }
            else if (bis_char(b))
            {
                printf("CHAR(%d,%d,%d)  size (%d,%d)(%d)\n", (bptr_type(b)>>24)&255, (bptr_type(b)>>16)&255, bptr_type(b)&65535, bptr_sizex(b), bptr_sizey(b), bptr_centery(b));
            }
            else
            {
                assert(bptr_type(b) == BNTYPE_NULLER);
                printf("NULL()  size (%d,%d)(%d)\n", bptr_sizex(b), bptr_sizey(b), bptr_centery(b));
            }
        } else
        {
            printf("IMME(%d,%d,%d)  size (%d,%d)(%d)\n", (bui_type(b)>>24)&255, (bui_type(b)>>16)&255, (bui_type(b)>>0)&65535, bui_sizex(b), bui_sizey(b), bui_centery(b));
        }
    }
    else
    {
               if (bnode_type(b) == BNTYPE_COL)       {printf("COL(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_ROW)       {printf("ROW(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_ROOT)      {printf("ROOT(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_CELLGROUP) {printf("CELLGROUP(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_CELL)      {printf("CELL(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_FRAC)      {printf("FRAC(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_SUPER)     {printf("SUPER(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_SUB)       {printf("SUB(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_SUBSUPER)  {printf("SUBSUPER(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_UNDER)     {printf("UNDER(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_OVER)      {printf("OVER(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_UNDEROVER) {printf("UNDEROVER(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_SQRT)      {printf("SQRT(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_GRID)      {printf("GRID(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_STUB)      {printf("STUB(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_ROT)       {printf("ROT(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_BUTTON)    {printf("BUTTON(%d)", bnode_len(b));
        } else if (bnode_type(b) == BNTYPE_TABVIEW)   {printf("TABVIEW(%d)", bnode_len(b));
        } else {
            assert(false);
        }

        if (bnode_type(b) == BNTYPE_ROW)
        {
//            rasterfont * font = fontint_to_fontp(bnode_extra0(b));
//            int default_sizey = font->chars[99].sizey;
//            int default_centery = font->chars[99].centery;
            int fs = bnode_extra0(b);
            int default_sizey = fontsize_default_sizey(fs);
            int default_centery = fontsize_default_centery(fs);
            printf("  alloc %d extra (%d,%d) size (%d,%d) cen %d  default (,%d) %d",bnode_alloc(b),bnode_extra0(b),bnode_extra1(b),bnode_sizex(b),bnode_sizey(b),bnode_centery(b),default_sizey,default_centery);
        }
        else
        {
            printf("  alloc %d extra (%d,%d) size (%d,%d) cen %d",bnode_alloc(b),bnode_extra0(b),bnode_extra1(b),bnode_sizex(b),bnode_sizey(b),bnode_centery(b));
        }
        if (bnode_type(b) == BNTYPE_ROW || bnode_type(b) == BNTYPE_CELLGROUP)
        {
            if (!(bnode_extra1(b)&BNFLAG_MEASURED))
            {
                printf(" !!!! NOT MEASURED !!!!");
            }
        }
        printf("\n");
        for (int i = 0; i < bnode_len(b); i++)
        {
            printindent(cursorbox, bnode_child(b,i), depth);
            printf("(%d,%d): ", bnode_offx(b,i), bnode_offy(b,i));
            boxnode_print(cursorbox, bnode_child(b,i), depth + 1);
        }
    }
}


void box_invalidate_us(box B) {
    if (bis_node(B)) {
        boxnode * b = bto_node(B);
        if (b->header.type == BNTYPE_ROW || b->header.type == BNTYPE_CELLGROUP) {
            b->extra1 &= ~(BNFLAG_MEASURED | BNFLAG_COLORED);
        }
    }
}

void box_invalidate_all(box B) {
    if (bis_node(B)) {
        boxnode * b = bto_node(B);
        if (b->header.type == BNTYPE_ROW || b->header.type == BNTYPE_CELLGROUP) {
            b->extra1 &= ~(BNFLAG_MEASURED | BNFLAG_COLORED);
        }
        for (int i = 0; i < b->len; i++) {
            box_invalidate_all(b->array[i].child);
        }
    }
}


/*
box boxplot3d_create() {
    boxplot3d * a = (boxplot3d *) malloc(sizeof(boxplot3d));
    a->header.type = BNTYPE_PLOT3D;
    a->header.sizex = 0;
    a->header.sizey = 0;
    a->header.centery = 0;
    new (&a->plot) graphics3d;
    return bfrom_ptr(reinterpret_cast<boxheader*>(a));
}


box boxtext_create() {
    boxplot3d * a = (boxplot3d *) malloc(sizeof(boxtext));
    a->header.type = BNTYPE_TEXT;
    a->header.sizex = 0;
    a->header.sizey = 0;
    a->header.centery = 0;
    new (&a->plot) monotext;
    return bfrom_ptr(reinterpret_cast<boxheader*>(a));
}


void boxtext_insert_string(boxtext * us, const unsigned char * s, int sn)
{
    unsigned char * oldline = us->line_chars[us->cursory];
    unsigned char * newline = (unsigned char *) malloc((sn+us->line_lens[us->cursory])*sizeof(unsigned char));
    us->line_chars[us->cursory] = newline;
    for (int i = 0; i < us->cursorx; i++) {
        *newline++ = oldline[i];
    }
    for (int i = 0; i < sn; i++) {
        *newline++ = s[i];
    }
    for (int i = us->cursorx; i < us->line_lens[us->cursory]; i++) {
        *newline++ = oldline[i];
    }
    us->line_lens[us->cursory] += sn;
    us->cursorx += sn;
}
*/


int escape_seq_to_action(const char * s)
{
    int32_t c = esccode_to_char(s);
    if (c != 0)
    {
        return c;
    }
//  TODO: intt, sumt, ...
    return 0;
}




/*
bool boxtext_scan_escape_seq(boxtext * us) {
    unsigned char * row = us->line_chars[us->cursory];
    int row_len = us->line_lens[us->cursory];
    int r, l;

    // look for \[AliasDelimiter] to the right
    for (r = us->cursorx; r < row_len; r++) {
        if (row[r] == CHAR_AliasDelimiter)
            break;
    }
    if (r >= row_len)
        return false;

    // look for \[AliasDelimiter] to the left
    for (l = std::min(us->cursorx, r-1); l >= 0; l--) {
        if (row[l] == CHAR_AliasDelimiter)
            break;
    }
    if (l<0)
        return false;

    char * t = (char*)malloc((r-l)*sizeof(char));
    for (int i=l+1; i<r; i++)
        t[i-(l+1)] = row[i];
    t[r-(l+1)] = 0;
    int c = escape_seq_to_action(t);
    free(t);

    if (c == 0)
    {
        return false;
    }

    if (c > 0)
    {
        us->cursorx = l;
        row[l] = c;
        for (int i = r + 1; i < row_len; i++) {
            row[i-(r+1)+l+1] = row[i];
        }
        us->line_lens[us->cursory] = row_len - r + l;
        return true;
    }

    return false;
}


void boxtext_insert_char(boxtext * us, char16_t c)
{
    unsigned char * oldline = us->line_chars[us->cursory];
    unsigned char * newline = (unsigned char*) malloc((1+us->line_lens[us->cursory])*sizeof(unsigned char));
    us->line_chars[us->cursory] = newline;
    for (int i = 0; i < us->cursorx; i++) {
        *newline++ = oldline[i];
    }
    *newline++ = c;
    for (int i = us->cursorx; i < us->line_lens[us->cursory]; i++) {
        *newline++ = oldline[i];
    }
    free(oldline);
    us->line_lens[us->cursory] += 1;
    boxtext_scan_escape_seq(us);
    us->cursorx += 1;
}

void boxtext_insert_tab(boxtext * us)
{
    do {boxtext_insert_char(us, ' ');}
    while ((us->cursorx&3) != 0);
}

void boxtext_insert_newline(boxtext * us)
{
    int old_len = us->line_lens[us->cursory];
    int* old_line_lens = us->line_lens;
    unsigned char** old_line_chars = us->line_chars;

    unsigned char * oldline = old_line_chars[us->cursory]; old_line_chars[us->cursory] = NULL;

    unsigned char * newline1 = (unsigned char*) malloc((1 + us->cursorx)*sizeof(unsigned char));
    unsigned char * newline2 = (unsigned char*) malloc((1 + old_len - us->cursorx)*sizeof(unsigned char));

    for (int i = 0; i < us->cursorx; i++) {
        newline1[i] = oldline[i];
    }
    for (int i = us->cursorx; i < old_len; i++) {
        newline2[i-us->cursorx] = oldline[i];
    }
    free(oldline);

    int* new_line_lens = (int*) malloc((us->nlines+1)*sizeof(int));
    unsigned char** new_line_chars = (unsigned char**) malloc((us->nlines+1)*sizeof(unsigned char*));

    for (int i = 0; i < us->cursory; i++) {
        new_line_lens[i] = old_line_lens[i];
        new_line_chars[i] = old_line_chars[i];
        
    }
    new_line_lens[us->cursory]    = us->cursorx;
    new_line_chars[us->cursory]   = newline1;
    new_line_lens[us->cursory+1]  = old_len - us->cursorx;
    new_line_chars[us->cursory+1] = newline2;
    for (int i = us->cursory + 1; i < us->nlines; i++) {
        new_line_lens[i+1] = old_line_lens[i];
        new_line_chars[i+1] = old_line_chars[i];
    }

    us->line_lens = new_line_lens; free(old_line_lens);
    us->line_chars = new_line_chars; free(old_line_chars);

    us->cursory += 1;
    us->cursorx = 0;
    us->nlines += 1;
}

int boxtext_backspace(boxtext * us)
{
    int success = 1;
    if (us->cursorx > 0)
    {
        unsigned char * oldline = us->line_chars[us->cursory];
        unsigned char * newline = (unsigned char *) malloc((us->line_lens[us->cursory])*sizeof(unsigned char));
        us->line_chars[us->cursory] = newline;
        for (int i = 0; i+1 < us->cursorx; i++) {
            *newline++ = oldline[i];
        }
        for (int i = us->cursorx; i < us->line_lens[us->cursory]; i++) {
            *newline++ = oldline[i];
        }
        us->line_lens[us->cursory] -= 1;
        us->cursorx -= 1;
        free(oldline);
    }
    else if (us->cursory > 0)
    {
        int old_len = us->line_lens[us->cursory];
        int* old_line_lens = us->line_lens;
        unsigned char** old_line_chars = us->line_chars;

        unsigned char * oldline1 = old_line_chars[us->cursory-1]; old_line_chars[us->cursory-1] = NULL;
        unsigned char * oldline2 = old_line_chars[us->cursory  ]; old_line_chars[us->cursory  ] = NULL;
        int oldlen1 = old_line_lens[us->cursory-1];
        int oldlen2 = old_line_lens[us->cursory  ];

        unsigned char * newline = (unsigned char *) malloc((oldlen1 + oldlen2)*sizeof(unsigned char));

        for (int i = 0; i < oldlen1; i++) {
            newline[i] = oldline1[i];
        }
        free(oldline1);
        for (int i = 0; i < oldlen2; i++) {
            newline[i+oldlen1] = oldline2[i];
        }
        free(oldline2);

        int* new_line_lens = (int*) malloc((us->nlines-1)*sizeof(int));
        unsigned char** new_line_chars = (unsigned char**) malloc((us->nlines-1)*sizeof(unsigned char*));

        for (int i = 0; i < us->cursory-1; i++) {
            new_line_lens[i] = old_line_lens[i];
            new_line_chars[i] = old_line_chars[i];
        }
        new_line_lens[us->cursory-1]    = oldlen1 + oldlen2;
        new_line_chars[us->cursory-1]   = newline;
        for (int i = us->cursory+1; i < us->nlines; i++) {
            new_line_lens[i-1] = old_line_lens[i];
            new_line_chars[i-1] = old_line_chars[i];
        }

        us->line_lens = new_line_lens; free(old_line_lens);
        us->line_chars = new_line_chars; free(old_line_chars);

        us->cursory -= 1;
        us->cursorx = oldlen1;
        us->nlines -= 1;
    }
    else
    {
        success = 0;
    }
    return success;
}


void boxtext_key_end(boxtext * us)
{
    us->cursorx = us->line_lens[us->cursory];
}

void boxtext_key_home(boxtext * us)
{
    us->cursorx = 0;
}


void boxtext_end(boxtext * us)
{
    us->cursory = us->nlines - 1;
    us->cursorx = us->line_lens[us->cursory];
}
void boxtext_start(boxtext * us)
{
    us->cursorx = 0;
    us->cursory = 0;
}

int boxtext_left(boxtext * us)
{
    int success = 1;
    if (us->cursorx > 0)
    {
        us->cursorx--;
    }
    else if (us->cursory > 0)
    {
        us->cursory--;
        us->cursorx = us->line_lens[us->cursory];
    }
    else
    {
        success = 0;
    }
    return success;
}

int boxtext_right(boxtext * us)
{
    int success = 1;
    if (us->cursorx < us->line_lens[us->cursory])
    {
        us->cursorx++;
    }
    else if (us->cursory + 1 < us->nlines)
    {
        us->cursory++;
        us->cursorx = 0;
    }
    else
    {
        success = 0;
    }
    return success;
}

int boxtext_key_up(boxtext * us)
{
    int success = 0;
    if (us->cursory-1 >= 0)
    {
        us->cursory--;
        us->cursorx = std::min(us->cursorx, us->line_lens[us->cursory]);
        success = 1;
    }
    return success;
}

int boxtext_key_down(boxtext * us)
{
    int success = 0;
    if (us->cursory + 1 < us->nlines)
    {
        us->cursory++;
        us->cursorx = std::min(us->cursorx, us->line_lens[us->cursory]);
        success = 1;
    }
    return success;
}

int boxtext_deletekey(boxtext * us)
{
    int success = boxtext_right(us);
    if (success)
    {
        boxtext_backspace(us);
    }
    return success;
}
*/





box boxchar_set_sizes(box b, int32_t sizex, int32_t sizey, int32_t centery)
{
/*
std::cout << "sizex: " << sizex << std::endl;
std::cout << "sizey: " << sizey << std::endl;
std::cout << "centery: " << centery << std::endl;
*/
    assert(bis_char(b));
    assert(sizex >= 0);
    assert(sizey >= 0);
    assert(centery >= 0);

    if (bis_ui(b))
    {
        int32_t btype = bui_type(b);
        if ((sizex|centery) < (1 << 10) && (sizey) < (1 << 11))
        {
            box c;
            c.imm =  uint64_t(1)
                   + (uint64_t(btype) << 32)
                   + (uint64_t(sizey) << 1)
                   + (uint64_t(centery) << 12)
                   + (uint64_t(sizex) << 22);
            return c;
        }
        else
        {
//std::cout << "upgrading to big" << std::endl;
            return boxchar_createbig(btype, sizex, sizey, centery);
        }
    }
    else
    {
        int32_t btype = bptr_type(b);
        if ((sizex|centery) < (1 << 10) && (sizey) < (1 << 11))
        {
            free(bto_ptr(b));
            box c;
            c.imm =  uint64_t(1)
                   + (uint64_t(btype) << 32)
                   + (uint64_t(sizey) << 1)
                   + (uint64_t(centery) << 12)
                   + (uint64_t(sizex) << 22);
            return c;
        }
        else
        {
//std::cout << "keeping big" << std::endl;
            bto_ptr(b)->type = btype;
            bto_ptr(b)->sizex = sizex;
            bto_ptr(b)->sizey = sizey;
            bto_ptr(b)->centery = centery;
            return b;
        }
    }
}

box boxchar_create(int32_t type)
{
    uint64_t b = (((uint64_t)(type)) << 32) + 1;
    return bfrom_imm(b);
}

box boxchar_create(int32_t type, int32_t sizex)
{
    uint64_t b = uint64_t(((uint64_t)(type)) << 32) + uint64_t(((uint64_t)(sizex)) << 1) + uint64_t(1);
    return bfrom_imm(b);
}

box boxchar_createbig(int type, int sizex, int sizey, int centery) {
//printf("creating big char %c\n",type&255);
    boxheader * b = (boxheader *) malloc(sizeof(boxheader));
    b->type = type;
    b->sizex = sizex;
    b->sizey = sizey;
    b->centery = centery;
    return bfrom_ptr(b);
}


boxnode * boxnode_create(int type, int alloc) {
    boxnode * b = (boxnode *) malloc(sizeof(boxnode));
    b->header.type = type;
    b->extra0 = 0;
    b->extra1 = 0;
    b->len = 0;
    b->alloc = alloc;
    b->expr = nullptr;
    b->array = (boxarrayelem *) malloc(alloc*sizeof(boxarrayelem));
    return b;
}

box boxnode_make(int type, box b0) {
    boxnode * b = boxnode_create(type, 1);
    b->header.type = type;
    b->array[0].child = b0; b->array[0].offx = b->array[0].offy = 0;
    b->len = 1;
    return bfrom_node(b);
}
box boxnode_make(int type, box b0, box b1) {
    boxnode * b = boxnode_create(type, 2);
    b->header.type = type;
    b->array[0].child = b0; b->array[0].offx = b->array[0].offy = 0;
    b->array[1].child = b1; b->array[1].offx = b->array[1].offy = 0;
    b->len = 2;
    return bfrom_node(b);
}
box boxnode_make(int type, box b0, box b1, box b2) {
    boxnode * b = boxnode_create(type, 3);
    b->header.type = type;
    b->array[0].child = b0; b->array[0].offx = b->array[0].offy = 0;
    b->array[1].child = b1; b->array[1].offx = b->array[1].offy = 0;
    b->array[2].child = b2; b->array[2].offx = b->array[2].offy = 0;
    b->len = 3;
    return bfrom_node(b);
}


void box_node_delete(boxnode * b)
{
    for (int i = 0; i < b->len; i++)
    {
        box_delete(b->array[i].child);
    }
    if (b->expr != nullptr)
    {
        eclear(etox(b->expr));
    }
    free(b->array);
    free(b);
}

void boxptr_delete(boxheader * b)
{
    if (b->type < BNTYPE_NULLER)
    {
        box_node_delete(reinterpret_cast<boxnode *>(b));
    }
    else
    {
        if (b->type == BNTYPE_NULLER)
        {
            return;
        }
        else
        {
            free(b);
        }
/*
        if (b->type == BNTYPE_PLOT3D) {
            boxplot3d * a = (boxplot3d *) b;
            bitmap3d_clear(a->image);
            free(a->vertices);
            free(a->triangles);
        } else if (b->type == BNTYPE_PLOT2D) {
            boxplot2d * a = (boxplot2d *) b;
            bitmap2d_clear(a->image);
            free(a->vertices);
            free(a->lines);
        } else if (b->type == BNTYPE_TEXT) {
            boxtext * a = (boxtext *) b;
            for (int i = 0; i < a->nlines; i++) {
                free(a->line_chars[i]);
            }
            free(a->line_lens);
            free(a->line_chars);
        }
*/
    }
}

void box_delete(box b)
{
    if (bis_ptr(b))
        boxptr_delete(bto_ptr(b));
}

void boxnode_fit_length(boxnode * b, int len) {
    if (b->alloc >= len)
        return;
    if (b->alloc == 0) {
        b->alloc = len;
        b->array = (boxarrayelem *) malloc(len*sizeof(boxarrayelem));
        return;
    } else {
        b->alloc = std::max(len, b->alloc + b->alloc/8);
        b->array = (boxarrayelem *) realloc(b->array, b->alloc*sizeof(boxarrayelem));
    }
}

void boxnode_append(boxnode * parent, box child)
{
    boxnode_fit_length(parent, parent->len + 1);
    boxarrayelem * l = parent->array;
    l[parent->len].child = child; l[parent->len].offx = l[parent->len].offy = 0;
    parent->len++;
}

void boxnode_append_char(boxnode * parent, int c)
{
    assert(parent->header.type == BNTYPE_ROW);
    boxnode_fit_length(parent, parent->len + 1);
    boxarrayelem * l = parent->array;
    l[parent->len].child = boxchar_create(c); l[parent->len].offx = l[parent->len].offy = 0;
    parent->len++;
}
/*
void boxnode_append_str(boxnode * parent, expr s) {
    assert(parent->type == BNTYPE_ROW);
    assert(is_str(s));
    int n = str_getn(s);
    unsigned char* l = (unsigned char*)str_getl(s);
    for (int i = 0; i<n; i++) {
        boxnode_append_char(parent, l[i]);
    }
    clear(s);
}
*/
void boxnode_append_cstr(boxnode * parent, const char * cs)
{
    size_t i = 0;
    while(true)
    {
        char16_t c;
        i += readonechar16(c, (const unsigned char *)(cs) + i);
        if (c == 0)
            break;
        boxnode_append_char(parent, c);
    }
}

void boxnode_append_string(boxnode * parent, const std::string& a, bool esc)
{
    assert(parent->header.type == BNTYPE_ROW);

    for (size_t i = 0; i < a.size(); i++)
    {
        char16_t c = (unsigned char)(a[i]);
        if ((a[i] & 0x80) == 0)
        {
            c = a[i] & 0x7F;
        }
        else if ((a[i+0] & 0xE0) == 0xC0)
        {
            assert(i + 1 < a.size());
            assert((a[i+1] & 0xC0) == 0x80);
            c = ((a[i+0] & 0x1F) << 6) + ((a[i+1] & 0x3F) << 0);
            i+=1;
        }
        else
        {
            assert(i + 2 < a.size());
            assert((a[i+0] & 0xE0) ==0xE0);
            assert((a[i+1] & 0xC0) ==0x80);
            assert((a[i+2] & 0xC0) ==0x80);
            c = ((a[i+0] & 0x0F) << 12) + ((a[i+1] & 0x3F) << 6) + ((a[i+2] & 0x3F) << 0);
            i+=2;
        }
        if (esc && c == '"')
        {
            boxnode_append_char(parent, '\\');
        }
        boxnode_append_char(parent, c);
    }
}

box boxnode_make_from_string(const std::string& s)
{
    boxnode * b = boxnode_create(BNTYPE_ROW, s.length()+1);
    boxnode_append_string(b, s, false);
    boxnode_append(b, bfrom_ptr(&box_null));
    return bfrom_node(b);
}

box boxnode_make_from_cstr(const char* s)
{
    boxnode * b = boxnode_create(BNTYPE_ROW, strlen(s)+1);
    boxnode_append_cstr(b, s);
    boxnode_append(b, bfrom_ptr(&box_null));
    return bfrom_node(b);
}


void boxnode_insert(boxnode * parent, box child, int idx)
{
    assert(idx <= parent->len);
    boxnode_fit_length(parent, parent->len + 1);
    boxarrayelem * l = parent->array;
    int i;
    for (i = parent->len; i > idx; i--)
    {
        l[i] = l[i - 1];
    }
    l[i].child = child; l[i].offx = l[i].offy = 0;
    parent->len++;
}



/* return new row box with idx entries */
box boxnode_split_row(boxnode * org, int idx)
{
    assert(idx + 1 <= org->len);
    assert(org->header.type == BNTYPE_ROW);

    boxnode * newbox = boxnode_create(BNTYPE_ROW, idx + 1);
    int i;
    for (i = 0; i < idx; i++)
    {
        newbox->array[i] = org->array[i];
    }
    newbox->array[idx].child = bfrom_ptr(&box_null); newbox->array[idx].offx = newbox->array[idx].offy = 0;
    newbox->len = idx + 1;

    for (i = idx; i < org->len; i++) {
        org->array[i - idx] = org->array[i];
    }
    org->len -= idx;

    return bfrom_node(newbox);
}

/* b gets spliced into a, then deleted */

void boxnode_splice_row(boxnode * a, int idx, boxnode * b)
{
    int i;
    assert(b->header.type == BNTYPE_ROW);

    boxnode_fit_length(a, a->len + b->len - 1);

    /* first shift a */
    for (i = a->len-1; i >= idx; i--)
        a->array[i + b->len - 1] = a->array[i];

    /* then put b in */
    for (i = 0; i < b->len - 1; i++)
        a->array[idx + i] = b->array[i];

    box_delete(b->array[b->len - 1].child);

    a->len = a->len + b->len - 1;
    b->len = 0;
    box_node_delete(b);
}

void boxnode_splice(boxnode * a, int idx, boxnode * b)
{
    int i;
    boxnode_fit_length(a, a->len + b->len);

    /* first shift a */
    for (i = a->len-1; i >= idx; i--)
        a->array[i + b->len] = a->array[i];

    /* then put b in */
    for (i = 0; i < b->len; i++)
        a->array[idx + i] = b->array[i];

    a->len = a->len + b->len;
    b->len = 0;
    box_node_delete(b);
}


/* b gets appended to a, then deleted */
void boxnode_join_tworows(boxnode * a, boxnode * b)
{
    assert(a->header.type == BNTYPE_ROW);
    assert(b->header.type == BNTYPE_ROW);

    boxnode_fit_length(a, a->len + b->len - 1);

    box_delete(a->array[a->len - 1].child);
    int i;
    for (i = 0; i < b->len; i++) {
        a->array[a->len - 1 + i] = b->array[i];
    }
    a->len = a->len + b->len - 1;
    b->len = 0;
    box_node_delete(b);
}

void boxnode_append_row(boxnode * a, boxnode * b)
{
    assert(b->header.type == BNTYPE_ROW);

    boxnode_fit_length(a, a->len + b->len - 1);

    int i;
    for (i = 0; i + 1 < b->len; i++)
    {
        a->array[a->len + i] = b->array[i];
    }
    box_delete(b->array[b->len - 1].child);
    a->len = a->len + b->len - 1;
    b->len = 0;
    box_node_delete(b);
}


box boxnode_splitrange_row(boxnode * org, int start, int end)
{
    assert(0 <= start);
    assert(start < end);
    assert(end + 1 <= org->len);
    assert(org->header.type == BNTYPE_ROW);

    boxnode * newrow = boxnode_create(BNTYPE_ROW, end - start + 1);

    int i, j;

    for (i = start; i < end; i++) {
        newrow->array[i - start] = org->array[i];
    }
    newrow->array[i - start].child = bfrom_ptr(&box_null);
    newrow->array[i - start].offx = newrow->array[i - start].offy = 0;
    newrow->len = i - start + 1;

    for (i = start, j = end; j < org->len; i++, j++) {
        org->array[i] = org->array[j];
    }
    org->len = i;

    return bfrom_node(newrow);
}

void boxnode_deleterange(box Org, int start, int end)
{
    boxnode * org = bto_node(Org);
    assert(0 <= start);
    assert(start < end);
    assert(end + 1 <= org->len);
    int i, j;
    for (i = start; i < end; i++)
    {
        box_delete(org->array[i].child);
    }
    for (i = start, j = end; j < org->len; i++, j++) {
        org->array[i] = org->array[j];
    }
    org->len = i;
}

/* take out and return child of index idx */
box boxnode_remove(boxnode * org, int idx)
{
    box child = org->array[idx].child;
    int i;
    for (i = idx; i+1 < org->len; i++)
    {
        org->array[i] = org->array[i + 1];
    }
    org->len -= 1;
    return child;
}


box boxnode_removerange(boxnode * org, int start, int end)
{
    assert(0 <= start);
    assert(start <= end);
    assert(end <= org->len);
    int i, j;
    boxnode * newbox = boxnode_create(org->header.type, end - start + 1);
    for (i = start; i < end; i++)
    {
        newbox->array[i-start] = org->array[i];
    }
    newbox->len = i - start;
    for (i = start, j = end; j < org->len; i++, j++)
    {
        org->array[i] = org->array[j];
    }
    org->len = i;
    return bfrom_node(newbox);
}



box boxnode_replace(boxnode * parent, int idx, box newchild)
{
    assert(idx < parent->len);
    box child = parent->array[idx].child;
    parent->array[idx].child = newchild;
    parent->array[idx].offx = parent->array[idx].offy = 0;
    return child;
}

box boxnode_row_remove(boxnode * org, int idx)
{
    assert(idx + 1 < org->len);
    assert(org->header.type == BNTYPE_ROW);
    box child = org->array[idx].child;
    int i;
    for (i = idx; i+1 < org->len; i++)
    {
        org->array[i] = org->array[i + 1];
    }
    org->len -= 1;
    return child;
}




box boxnode_copy(box B)
{
    if (!bis_ptr(B))
    {
        return B;
    }
    else
    {
        if (bptr_isnode(B))
        {
            boxnode * b = bto_node(B);
            boxnode * a = boxnode_create(b->header.type, b->len);
            a->extra0 = b->extra0;
            a->extra1 = b->extra1;
            if (b->expr != nullptr)
            {
                a->expr = etor(ecopy(b->expr));
            }
            for (int i = 0; i < b->len; i++)
            {
                boxnode_append(a, boxnode_copy(b->array[i].child));
            }
            return bfrom_node(a);
        }
        else
        {
            boxheader * b = bto_ptr(B);
            if (b->type >= 0)
            {
                return boxchar_create(b->type);
            }
            else
            {
                assert(b->type == BNTYPE_NULLER);
                return bfrom_ptr(&box_null);
            }
        }
    }
}

box boxnode_copyrange(boxnode * org, int start, int end)
{
    assert(0 <= start);
    assert(start <= end);
    assert(end + 1 <= org->len);
    int i, j;
    boxnode * newbox = boxnode_create(org->header.type, end - start + 1);
    for (i = start; i < end; i++) {
        newbox->array[i-start].child = boxnode_copy(org->array[i].child);
        newbox->array[i-start].offx = newbox->array[i-start].offy = 0;
    }
    newbox->len = end - start;
    return bfrom_node(newbox);
}
