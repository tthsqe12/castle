#include <stack>
#include "boxes.h"
#include "ex_parse.h"
#include "notebook.h"
#include "box_convert.h"
#include "serialize.h"


int32_t childlen(boxbase* b)
{
    if (b->get_type() == BNTYPE_ROOT)
    {
        rootbox* B = dynamic_cast<rootbox*>(b);
        return B->child.size();
    }
    else if (b->get_type() == BNTYPE_CELLGROUP)
    {
        cellgroupbox* B = dynamic_cast<cellgroupbox*>(b);
        return B->child.size();
    }
    else
    {
        assert(false);
        return -1;
    }
}

boxbase* childat(boxbase* b, size_t i)
{
    if (b->get_type() == BNTYPE_ROOT)
    {
        rootbox* B = dynamic_cast<rootbox*>(b);
        return B->child[i].cbox;
    }
    else if (b->get_type() == BNTYPE_CELLGROUP)
    {
        cellgroupbox* B = dynamic_cast<cellgroupbox*>(b);
        return B->child[i].cbox;
    }
    else
    {
        assert(false);
        return nullptr;
    }
}

int32_t offyat(boxbase* b, size_t i)
{
    if (b->get_type() == BNTYPE_ROOT)
    {
        rootbox* B = dynamic_cast<rootbox*>(b);
        return B->child[i].offy;
    }
    else if (b->get_type() == BNTYPE_CELLGROUP)
    {
        cellgroupbox* B = dynamic_cast<cellgroupbox*>(b);
        return B->child[i].offy;
    }
    else
    {
        assert(false);
        return 0;
    }
}


int32_t offxat(boxbase* b, size_t i)
{
    if (b->get_type() == BNTYPE_ROOT)
    {
        rootbox* B = dynamic_cast<rootbox*>(b);
        return B->child[i].offx;
    }
    else if (b->get_type() == BNTYPE_CELLGROUP)
    {
        cellgroupbox* B = dynamic_cast<cellgroupbox*>(b);
        return B->child[i].offx;
    }
    else
    {
        assert(false);
        return 0;
    }
}


void childinsert(boxbase* b, boxbase* c, size_t i)
{
    if (b->get_type() == BNTYPE_ROOT)
    {
        rootbox* B = dynamic_cast<rootbox*>(b);
        B->child.insert(B->child.begin() + i, cellarrayelem(c));
    }
    else if (b->get_type() == BNTYPE_CELLGROUP)
    {
        cellgroupbox* B = dynamic_cast<cellgroupbox*>(b);
        B->child.insert(B->child.begin() + i, cellarrayelem(c));
    }
    else
    {
        assert(false);
    }    
}

void childreplace(boxbase* b, boxbase* c, size_t i)
{
    if (b->get_type() == BNTYPE_ROOT)
    {
        rootbox* B = dynamic_cast<rootbox*>(b);
        B->child[i] = cellarrayelem(c);
    }
    else if (b->get_type() == BNTYPE_CELLGROUP)
    {
        cellgroupbox* B = dynamic_cast<cellgroupbox*>(b);
        B->child[i] = cellarrayelem(c);
    }
    else
    {
        assert(false);
    }    
}


boxbase* childremove(boxbase* b, size_t i)
{
    if (b->get_type() == BNTYPE_ROOT)
    {
        rootbox* B = dynamic_cast<rootbox*>(b);
        boxbase* r = B->child[i].cbox;
        B->child.erase(B->child.begin() + i);
        return r;
    }
    else if (b->get_type() == BNTYPE_CELLGROUP)
    {
        cellgroupbox* B = dynamic_cast<cellgroupbox*>(b);
        boxbase* r = B->child[i].cbox;
        B->child.erase(B->child.begin() + i);
        return r;
    }
    else
    {
        assert(false);
        return nullptr;
    }    
}


boxbase* childremoverange(boxbase* org, int32_t start, int32_t end)
{
    assert(0 <= start);
    assert(start <= end);
    assert(end <= childlen(org));
    int32_t i, j;
    cellgroupbox* newbox = new cellgroupbox(end - start);
    if (org->get_type() == BNTYPE_ROOT)
    {
        rootbox* B = dynamic_cast<rootbox*>(org);
        for (i = start; i < end; i++)
            newbox->child[i-start].cbox = B->child[i].cbox;
        B->child.erase(B->child.begin() + start, B->child.begin() + end);
    }
    else
    {
        cellgroupbox* B = dynamic_cast<cellgroupbox*>(org);
        for (i = start; i < end; i++)
            newbox->child[i-start].cbox = B->child[i].cbox;
        B->child.erase(B->child.begin() + start, B->child.begin() + end);
    }
    return newbox;
}


void childsplice(boxbase* a, int32_t idx, boxbase* b)
{
    int32_t i;

    if (b->get_type() == BNTYPE_ROOT)
    {
        rootbox* B = dynamic_cast<rootbox*>(b);
        if (a->get_type() == BNTYPE_ROOT)
        {
            rootbox* A = dynamic_cast<rootbox*>(a);
            for (i = B->child.size() - 1; i >= 0; i--)
                A->child.insert(A->child.begin() + idx, B->child[i]);
            B->child.clear();
            delete B;
        }
        else
        {
            cellgroupbox* A = dynamic_cast<cellgroupbox*>(a);
            for (i = B->child.size() - 1; i >= 0; i--)
                A->child.insert(A->child.begin() + idx, B->child[i]);
            B->child.clear();
            delete B;
        }
    }
    else
    {
        cellgroupbox* B = dynamic_cast<cellgroupbox*>(b);
        if (a->get_type() == BNTYPE_ROOT)
        {
            rootbox* A = dynamic_cast<rootbox*>(a);
            for (i = B->child.size() - 1; i >= 0; i--)
                A->child.insert(A->child.begin() + idx, B->child[i]);
            B->child.clear();
            delete B;
        }
        else
        {
            cellgroupbox* A = dynamic_cast<cellgroupbox*>(a);
            for (i = B->child.size() - 1; i >= 0; i--)
                A->child.insert(A->child.begin() + idx, B->child[i]);
            B->child.clear();
            delete B;
        }
    }
}

void childappend(boxbase* a, boxbase* b)
{
    int32_t i;

    if (a->get_type() == BNTYPE_ROOT)
    {
        rootbox* A = dynamic_cast<rootbox*>(a);
        A->child.push_back(cellarrayelem(b));
    }
    else
    {
        cellgroupbox* A = dynamic_cast<cellgroupbox*>(a);
        A->child.push_back(cellarrayelem(b));
    }
}

bool _find_mark(boxbase * b, std::vector<int32_t>& v, uint32_t mask)
{
    for (int32_t i = 0; i < childlen(b); i++)
    {
        boxbase* c = childat(b, i);
        v.push_back(i);
        if (c->get_type() == BNTYPE_CELL)
        {
            if (dynamic_cast<cellbox*>(c)->flags & mask)
                return true;
        }
        else
        {
            if (_find_mark(c, v, mask))
                return true;
        }
        v.pop_back();
    }
    return false;
}


void rootbox::erase_cell_mark(uint32_t mask)
{
    cursor_t.clear();
    if (_find_mark(this, cursor_t, mask))
    {
        dynamic_cast<cellbox*>(_us())->flags &= ~mask;
    }
}


void rootbox::print_cell(cellbox* c)
{
    bool find_a = false;
    bool find_b = false;

    if (cursor_a[0] < child.size())
    {
        cursor_t = cursor_a;
        dynamic_cast<cellbox*>(_us())->flags |= BNFLAG_CURSORAMARK;
        find_a = true;
    }

    if (!cursor_b.empty() && cursor_b[0] < child.size())
    {
        cursor_t = cursor_b;
        dynamic_cast<cellbox*>(_us())->flags |= BNFLAG_CURSORBMARK;
        find_b = true;
    }

    cursor_t.clear();
    bool found = _find_mark(this, cursor_t, BNFLAG_PRINTMARK);

    if (!found)
    {
        cursor_t.clear();
        cursor_t.push_back(child.size());
    }
    else
    {
        dynamic_cast<cellbox*>(_us())->flags &= ~BNFLAG_PRINTMARK;
        goto_next_cellbreak();
    }

    c->flags |= BNFLAG_PRINTMARK;
    cell_insert(c);

    if (find_a)
    {
        cursor_t.clear();
        found = _find_mark(this, cursor_t, BNFLAG_CURSORAMARK);
        assert(found);
        dynamic_cast<cellbox*>(_us())->flags &= ~BNFLAG_CURSORAMARK;
        cursor_a = cursor_t;
    }
    else
    {
        cursor_a.clear();
        cursor_a.push_back(child.size());
    }

    if (find_b)
    {
        cursor_t.clear();
        found = _find_mark(this, cursor_t, BNFLAG_CURSORBMARK);
        assert(found);
        dynamic_cast<cellbox*>(_us())->flags &= ~BNFLAG_CURSORBMARK;
        cursor_b = cursor_t;
    }
    else if (!cursor_b.empty())
    {
        cursor_b.clear();
        cursor_b.push_back(child.size());
    }
}

/* remove all children from us after and including s */
void rootbox::_cell_remove_sels(boxbase * r, std::vector<int32_t>&s, size_t i)
{
    _invalidate_downto(0);
    if (i + 1 >= s.size())
    {
        assert(s[i] > 0);
        childsplice(r, childlen(r), childremoverange(_us(), s[i], childlen(_us())));
    }
    else
    {
        _down1(s[i]);
        _cell_remove_sels(r, s, i+1);
        _up1();
        childsplice(r, childlen(r), childremoverange(_us(), s[i]+1, childlen(_us())));
    }
    assert(_us()->get_type() == BNTYPE_CELLGROUP);
    if (childlen(_us()) == 1)
    {
        _down1(0);
        _deleteup1();
    }
}

/*
    put all children strictly before t into "before"
    put all children after and including t into "after"
*/
void rootbox::_cell_remove_selt(boxbase * before, boxbase * after, std::vector<int32_t>&t, size_t i)
{
    _invalidate_downto(0);
//    box r, f;
    if (i + 1 >= t.size())
    {
        childsplice(before, childlen(before), childremoverange(_us(), 0, t[i]));
        childsplice(after, childlen(after), childremoverange(_us(), 0, childlen(_us())));
        return;
    }
    else
    {
        childsplice(before, childlen(before), childremoverange(_us(), 0, t[i]));
        t[i] = 0;
        _down1(t[i]);
        _cell_remove_selt(before, after, t, i+1);
        _up1();
        delete _remove(0);
        childsplice(after, childlen(after), childremoverange(_us(), 0, childlen(_us())));
        return;
    }
}

boxbase* rootbox::_cell_remove_selection(std::vector<int32_t>&s, std::vector<int32_t>&t, size_t i)
{
    assert(i < s.size());
    assert(i < t.size());

//printf("_cell_remove_selection 1\n");

    if (s[i] == t[i])
    {
        _down1(s[i]);
//printf("_cell_remove_selection 2\n");
        return _cell_remove_selection(s, t, i+1);
    }
    else
    {
//printf("_cell_remove_selection 3\n");

        boxbase* r = new cellgroupbox(0);
        assert(s[i] < t[i]);
        while (s.size() > i+1 && s.back() == 0) {s.pop_back();}
        while (t.size() > i+1 && t.back() == 0) {t.pop_back();}
        if (s.size() > i+1)
        {
//printf("_cell_remove_selection 4\n");
            _down1(s[i]);
            _cell_remove_sels(r, s, i+1);
            _up1();
            s[i]++;
        }
        else
        {
//printf("_cell_remove_selection 5\n");
            _invalidate_downto(0);
            childappend(r, childremove(_us(), s[i]));
            if (_us()->get_type() == BNTYPE_CELLGROUP && childlen(_us()) == 1)
            {
                assert(t[i] == 1);
                s[i] = t[i] = _pi();
                _down1(0);
                _deleteup1();
                _up1();           
            }
            else
            {
                t[i]--;
            }
        }

//printf("0 notebook:\n"); print(0,0,0);


//printf("\n\n\nremove s:\n"); boxnode_print(NULL, r, 0); printf("\n");


//printf("s[i]: %d  t[i]: %d\n",s[i],t[i]);

        if (s[i] <= t[i])
        {
//printf("_cell_remove_selection 6\n");

            childsplice(r, childlen(r), childremoverange(_us(), s[i], t[i]));
//printf("remove middle:\n"); r->print(0,0,0); printf("\n");
            t[i] = s[i];
        }

//printf("s[i]: %d  t[i]: %d\n",s[i],t[i]);

        assert(t[i] == s[i]);

//printf("1 notebook:\n"); print(0,0,0);

//printf("_cell_remove_selection 7\n");
        if (_us()->get_type() == BNTYPE_CELLGROUP && childlen(_us()) == 1)
        {
            assert(t[i] == 0);
            t[i] = _pi();
            _down1(0);
            _deleteup1();
            _up1();           
        }

//printf("_cell_remove_selection 8\n");

        boxbase* v = new cellgroupbox(0);
        if (_us()->get_type() == BNTYPE_ROOT && t[i] == childlen(_us()) - 1
                || t.size() <= i+1)
        {
//printf("_cell_remove_selection 9\n");

            _down1(t[i]);
        }
        else
        {
//printf("_cell_remove_selection 10\n");
//printf("going to child %d %p\n",t[i],bnode_child(_us(), t[i]));
            _down1(t[i]);
            _cell_remove_selt(r, v, t, i+1);
//printf("remove r:\n"); r->print(0,0,0); printf("\n");
//printf("v:\n"); v->print(0,0,0); printf("\n");
//printf("notebook:\n"); print(0,0,0);

            cell_delete();
        }
//printf("2 notebook:\n"); print(0,0,0);
//printf("_cell_remove_selection 11\n");
        while (_us()->get_type() == BNTYPE_CELLGROUP)
        {
            _down1(0);
        }
//printf("_cell_remove_selection 12\n");

        while (childlen(v) > 0)
        {
            cell_insert(childremove(v, childlen(v) - 1));
        }
//printf("_cell_remove_selection 13\n");

        delete v;

        return r;
    }
}

bool stdvector_equal(std::vector<int32_t>&a, std::vector<int32_t>&b)
{
    size_t n = a.size();

    if (n != b.size())
        return false;

    for (size_t i = 0; i < n; i++)
    {
        if (a[i] != b[i])
            return false;
    }

    return true;
}

void rootbox::makecell(cellType t)
{
    if (cursor_b.empty())
    {
        // make new cell
        assert(cursor_a[0] < child.size());
        cursor_t = cursor_a;
        cellbox* u = dynamic_cast<cellbox*>(_us());
        u->flags |= BNFLAG_CURSORAMARK;
        u->celltype = t;
        _fix_cell_insert();
        cursor_t.clear();
        bool found = _find_mark(this, cursor_t, BNFLAG_CURSORAMARK);
        assert(found);
        dynamic_cast<cellbox*>(_us())->flags &= ~BNFLAG_CURSORAMARK;
        cursor_a = cursor_t;
    }
    else if (stdvector_equal(cursor_a, cursor_b))
    {
        // change our cell
        cellbox* newcell = new cellbox(new rowbox(cell_type_names[t]), t);
        newcell->flags |= BNFLAG_CURSORAMARK;
        cursor_t = cursor_a;
        cell_insert(newcell);
        cursor_t.clear();
        bool found = _find_mark(this, cursor_t, BNFLAG_CURSORAMARK);
        assert(found);
        dynamic_cast<cellbox*>(_us())->flags &= ~BNFLAG_CURSORAMARK;
        cursor_a = cursor_t;        
        cursor_b.clear();
    }
    else
    {
        // TODO: cell selection
    }
}



void rootbox::cell_delete()
{
    int32_t pi = _pi();
    bool gotonext;

    _up1();
    assert(childlen(_us())>1);
    delete _remove(pi);
    if (pi < childlen(_us()))
    {
        gotonext = false;
        _down1(pi);
    } else {
        gotonext = true;
        _down1(-1);
    }
    while (_parent()->get_type() == BNTYPE_CELLGROUP
            && childlen(_parent()) == 1)
    {
        _deleteup1();
    }

    if (gotonext)
        goto_next_cellbreak();
}


boxbase* rootbox::cell_remove()
{
    int32_t pi = _pi();
    if (_parent()->get_type() != BNTYPE_CELLGROUP || pi > 0)
    {
        bool gotonext;
        _up1();
        assert(childlen(_us())>1);
        boxbase* us = _remove(pi);
        if (pi < childlen(_us()))
        {
            gotonext = false;
            _down1(pi);
        }
        else
        {
            gotonext = true;
            _down1(-1);
        }
        if (_parent()->get_type() == BNTYPE_CELLGROUP
                && childlen(_parent()) == 1)
        {
            _deleteup1();
        }
        if (gotonext)
        {
            while (_us()->get_type() == BNTYPE_CELLGROUP)
            {
                _down1(-1);
            }
            goto_next_cellbreak();
        }
        else
        {
            while (_us()->get_type() == BNTYPE_CELLGROUP)
            {
                _down1(0);
            }
        }
        return us;
    }
    else
    {
        int32_t gpi = _gpi();
        _up1();
        _up1();
        boxbase* r = _remove(gpi);
        boxbase* us = childremove(r, 0);
        int32_t i = childlen(r) - 1;
        childinsert(_us(), childremove(r,i), gpi);
        _down1(gpi);
        _fix_cell_insert();
        i--;
        while (i >= 0)
        {
            _insert(childremove(r,i));
            _fix_cell_insert();
            i--;
        }
        assert(childlen(r) == 0);
        delete r;
        return us;
    }
}

bool rootbox::goto_first_cellbreak()
{
    cursor_t.clear();
    cursor_t.push_back(0);
    if (cursor_t[0] >= child.size())
        return false;
    while (_us()->get_type() != BNTYPE_CELL)
        _down1(0);
    return true;
}

bool rootbox::goto_next_cellbreak()
{
//std::cout << "goto_next_cellbreak" << std::endl;print(0,0,0);
    if (cursor_t.empty())
    {
        cursor_t.push_back(0);
//std::cout << "goto_next_cellbreak return true" << std::endl;print(0,0,0);
        return true;
    }

    if (cursor_t[0] >= child.size())
    {
//std::cout << "goto_next_cellbreak return false" << std::endl;print(0,0,0);
        return false;
    }

    if (_us()->get_type() == BNTYPE_CELL)
    {
        if (_pi() + 1 < childlen(_parent()))
        {
            cursor_t[cursor_t.size() - 1]++;
            goto down_first;
        }
        _up1();
    }

    while (!cursor_t.empty())
    {
        if (_pi() + 1 < childlen(_parent()))
        {
            cursor_t[cursor_t.size() - 1]++;
            goto down_first;
        }
        _up1();
    }

    cursor_t.clear();
    cursor_t.push_back(child.size());
//std::cout << "goto_next_cellbreak return true" << std::endl;print(0,0,0);
    return true;

down_first:

    while (_us()->get_type() != BNTYPE_CELL)
        _down1(0);

//std::cout << "goto_next_cellbreak return true" << std::endl;print(0,0,0);
    return true;
}

bool rootbox::goto_prev_cellbreak()
{
//std::cout << "goto_prev_cellbreak" << std::endl;print(0,0,0);
    if (cursor_t.empty())
    {
//std::cout << "goto_prev_cellbreak return false" << std::endl;print(0,0,0);
        return false;
    }

    if (cursor_t[0] >= child.size())
    {
        if (child.size() < 1)
        {
//std::cout << "goto_prev_cellbreak return false" << std::endl;print(0,0,0);
            return false;
        }
        cursor_t.clear();
        cursor_t.push_back(child.size() - 1);
        goto down_last;
    }

    cursor_s = cursor_t;

    if (_us()->get_type() == BNTYPE_CELL)
    {
        if (_pi() > 0)
        {
            cursor_t[cursor_t.size() - 1]--;
            goto down_last;
        }
        _up1();
    }

    while (!cursor_t.empty())
    {
        if (_pi() > 0)
        {
            cursor_t[cursor_t.size() - 1]--;
            goto down_last;        
        }
        _up1();
    }

    cursor_t = cursor_s;
//std::cout << "goto_prev_cellbreak return false" << std::endl;print(0,0,0);
    return false;

down_last:

    while (_us()->get_type() != BNTYPE_CELL)
        _down1(-1);

//std::cout << "goto_prev_cellbreak return true" << std::endl;print(0,0,0);
    return true;
}


void rootbox::delete_selection()
{
    for (size_t i = 0; i < cursor_a.size() && i < cursor_b.size(); i++)
    {
        if (cursor_a[i] == cursor_b[i])
            continue;
        cursor_t.clear();
        if (cursor_a[i] < cursor_b[i])
            delete _cell_remove_selection(cursor_a, cursor_b, 0);
        else
            delete _cell_remove_selection(cursor_b, cursor_a, 0);
        cursor_a = cursor_t;
        cursor_b = cursor_t;
        break;
    }
}


boxbase* rootbox::_us()
{
    boxbase* b = this;
    for (size_t i = 0; i < cursor_t.size(); i++)
    {
        b = childat(b, cursor_t[i]);
    }
    return b;
}

int32_t rootbox::_pi()
{
    assert(cursor_t.size() >= 1);
    return cursor_t[cursor_t.size() - 1];
}

boxbase* rootbox::_parent()
{
    assert(cursor_t.size() >= 1);
    boxbase* b = this;
    for (size_t i = 0; i + 1 < cursor_t.size(); i++)
        b = childat(b, cursor_t[i]);
    return b;
}

int32_t rootbox::_gpi()
{
    assert(cursor_t.size() >= 2);
    return cursor_t[cursor_t.size() - 2];
}

boxbase* rootbox::_gparent()
{
    assert(cursor_t.size() >= 2);
    boxbase* b = this;
    for (size_t i = 0; i + 2 < cursor_t.size(); i++)
        b = childat(b, cursor_t[i]);
    return b;
}

void rootbox::_down1(int32_t a)
{
    if (a < 0) {a += childlen(_us());}
    cursor_t.push_back(a);
}


void rootbox::_up1()
{
    assert(!cursor_t.empty());
    cursor_t.pop_back();
}

void rootbox::_insert(boxbase* newchild)
{
    newchild->visit(visitarg_InvalidateAll);
    childinsert(_parent(), newchild, _pi());
    _invalidate_downto(1);
}


void rootbox::cell_insert(boxbase* c)
{
    while (_pi() == 0 && _parent()->get_type() == BNTYPE_CELLGROUP)
    {
        _up1();
    }
    _insert(c);
    _fix_cell_insert();
}




bool ok_parent_child(boxbase* p, boxbase* c)
{
/*
    if (p.ptr == &box_null || c.ptr == &box_null)
        return false;
*/
    while (p->get_type() == BNTYPE_CELLGROUP) {p = childat(p,0);}
    while (c->get_type() == BNTYPE_CELLGROUP) {c = childat(c,0);}
    cellType pstyle = dynamic_cast<cellbox*>(p)->celltype;
    cellType cstyle = dynamic_cast<cellbox*>(c)->celltype;

    switch (pstyle) {
        case cellt_INPUT:
            return cstyle == cellt_OUTPUT || cstyle == cellt_MESSAGE || cstyle == cellt_PRINT;
        case cellt_MESSAGE:
        case cellt_PRINT:
            return cstyle == pstyle;
        case cellt_OUTPUT:
        case cellt_BOLDTEXT:
        case cellt_TEXT:
            return false;
        case cellt_SUBSUBSECTION:
        case cellt_SUBSECTION:
        case cellt_SECTION:
        case cellt_TITLE:
            return cstyle < pstyle;
        default:
            assert(false);
            return false;
    }
}

bool ok_parent_child_splice(boxbase* p, boxbase* c)
{
/*
    if (p.ptr == &box_null || c.ptr == &box_null)
        return false;
*/

    while (p->get_type() == BNTYPE_CELLGROUP) {p = childat(p,0);}
    while (c->get_type() == BNTYPE_CELLGROUP) {c = childat(c,0);}
    cellType pstyle = dynamic_cast<cellbox*>(p)->celltype;
    cellType cstyle = dynamic_cast<cellbox*>(c)->celltype;

    switch (pstyle) {
        case cellt_INPUT:
            return false;
        case cellt_MESSAGE:
        case cellt_PRINT:
            return cstyle == pstyle;
        case cellt_OUTPUT:
        case cellt_BOLDTEXT:
        case cellt_TEXT:
            return false;
        case cellt_SUBSUBSECTION:
        case cellt_SUBSECTION:
        case cellt_SECTION:
        case cellt_TITLE:
            return false;
        default:
            assert(false);
            return false;
    }
}

bool ok_parent_child_master(boxbase* p, boxbase* c, boxbase* m)
{
    if (m->get_type() == BNTYPE_ROOT)
        return ok_parent_child(p, c);

    while (p->get_type() == BNTYPE_CELLGROUP) {p = childat(p,0);}
    while (c->get_type() == BNTYPE_CELLGROUP) {c = childat(c,0);}
    while (m->get_type() == BNTYPE_CELLGROUP) {m = childat(m,0);}

    cellType pstyle = dynamic_cast<cellbox*>(p)->celltype;
    cellType cstyle = dynamic_cast<cellbox*>(c)->celltype;
    cellType mstyle = dynamic_cast<cellbox*>(m)->celltype;

    switch (pstyle)
    {
        case cellt_INPUT:
            return cstyle == cellt_OUTPUT || cstyle == cellt_MESSAGE || cstyle == cellt_PRINT;
        case cellt_MESSAGE:
        case cellt_PRINT:
            return cstyle == pstyle && cstyle != mstyle;
        case cellt_OUTPUT:
        case cellt_BOLDTEXT:
        case cellt_TEXT:
            return false;
        case cellt_SUBSUBSECTION:
        case cellt_SUBSECTION:
        case cellt_SECTION:
        case cellt_TITLE:
            return cstyle < pstyle;
        default:
            assert(false);
            return false;
    }
}

void rootbox::_deleteup1()
{
    boxbase* us = _us();
    boxbase* parent = _parent();
    int32_t pi = _pi();
    boxbase* gparent = _gparent();
    int32_t gpi = _gpi();

    assert(childlen(parent) == 1);
    childreplace(gparent, childremove(parent, 0), gpi);
    delete parent;
    cursor_t.pop_back();
    _invalidate_downto(1);
}

void rootbox::_insertup1(boxbase* newparent, int32_t idx)
{
    boxbase* us = _us();
    boxbase* parent = _parent();
    int32_t pi = _pi();

//    bto_node(parent)->array[pi].child = newparent;
    if (parent->get_type() == BNTYPE_ROOT)
    {
        rootbox* B = dynamic_cast<rootbox*>(parent);
        B->child[pi].cbox = newparent;
    }
    else
    {
        cellgroupbox* B = dynamic_cast<cellgroupbox*>(parent);
        B->child[pi].cbox = newparent;
    }

//    bto_node(newparent)->array[idx].child = us;
    if (newparent->get_type() == BNTYPE_ROOT)
    {
        rootbox* B = dynamic_cast<rootbox*>(newparent);
        B->child.insert(B->child.begin() + idx, cellarrayelem(us));
    }
    else
    {
        cellgroupbox* B = dynamic_cast<cellgroupbox*>(newparent);
        B->child.insert(B->child.begin() + idx, cellarrayelem(us));
    }

    cursor_t.push_back(idx);
}


boxbase* rootbox::_remove(int32_t idx)
{
    _invalidate_downto(0);
    return childremove(_us(), idx);
}


boxbase* rootbox::_removerange(int32_t start, int32_t end)
{
    boxbase* range = childremoverange(_us(), start, end);
    _invalidate_downto(1);
    return range;
}


void rootbox::_fix_cell_insert()
{

//printf("\n\n");

try_again:

//printf("**** before correction\n");print(0,0,0);SleepUS(10000);


    int32_t i, pi = _pi();

//std::cout << "pi : " << pi << std::endl;

    if (pi == 0 && _parent()->get_type() == BNTYPE_CELLGROUP)
    {
        _up1();
//printf("did 0!!!!!!!!!!!!!!!!!!!!!!!!\n");
        goto try_again;
    }

    /* if there are siblings to the right that would rather be our children, make them our children */
    if (pi > 0 || _parent()->get_type() == BNTYPE_ROOT)
    {
//std::cout << "trying 1" << std::endl;
        int32_t len = childlen(_parent());

        if (pi + 1 < len && ok_parent_child_master(_us(), childat(_parent(), pi + 1), _parent()))
        {
//std::cout << "doing 1" << std::endl;
            _up1();
            boxbase* sib = _remove(pi + 1);
            _down1(pi);
            if (_us()->get_type() != BNTYPE_CELLGROUP)
            {
                boxbase* g = new cellgroupbox(0);
//                bto_node(g)->extra1 |= BNFLAG_OPEN;
                _insertup1(g,0);
                _up1();
            }
            if (sib->get_type() == BNTYPE_CELLGROUP && ok_parent_child_splice(_us(), sib))
            {
                childsplice(_us(), childlen(_us()), sib);   
            }
            else
            {
                childappend(_us(), sib);
            }
//printf("did 1!!!!!!!!!!!!!!!!!!!!!!!!\n");
            goto try_again;
        }
    }

    /* if we cannot be a child of our parent, move us up */
    if (_parent()->get_type() == BNTYPE_CELLGROUP)
    {
//std::cout << "trying 2" << std::endl;
        if (pi > 0 && !ok_parent_child(_parent(), _us()))
        {
//std::cout << "doing 2" << std::endl;
            _up1();
            boxbase* r = _removerange(pi, childlen(_us()));
            if (pi == 1)
            {
                _down1(0);
                _deleteup1();
            }
            pi = _pi();
            _up1();
            childsplice(_us(), pi + 1, r);
            _down1(pi + 1);
//printf("did 2!!!!!!!!!!!!!!!!!!!!!!!!\n");
            goto try_again;
        }        
    }

    /* if the sibling to the left accepts us as a child over our current parent, make us their child */
    if (_parent()->get_type() == BNTYPE_ROOT ? pi > 0 : pi > 1)
    {
//std::cout << "trying 3" << std::endl;
        if (ok_parent_child_master(childat(_parent(), _pi() - 1), _us(), _parent()))
        {
//std::cout << "doing 3" << std::endl;
            _up1();
            boxbase* u = _remove(pi);
            _down1(pi - 1);
            if (_us()->get_type() != BNTYPE_CELLGROUP)
            {
                boxbase* g = new cellgroupbox(0);
//                bto_node(g)->extra1 |= BNFLAG_OPEN;
                _insertup1(g,0);
                _up1();
            }
            childappend(_us(), u);
            _down1(-1);

//printf("did 3!!!!!!!!!!!!!!!!!!!!!!!!\n");
            goto try_again;
        }
    }

//printf("**** after correction\n");print(0,0,0);

    /* good to go */
    while (_us()->get_type() == BNTYPE_CELLGROUP)
    {
        _down1(0);
    }
    _invalidate_downto(0);
}



/*******************************************************************/

void rootbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");
    printf("(%d,%d) root: size (%d,%d:%d) ", offx, offy, sizex, sizey, centery);
    std::cout << "cursor_a : " << stdvector_tostring(cursor_a);
    std::cout << ", cursor_b: " << stdvector_tostring(cursor_b);
    std::cout << ", cursor_t: " << stdvector_tostring(cursor_t) << std::endl;

    for (auto& c: child)
    {
        c.cbox->print(indent + 1, c.offx, c.offy);
    }
}

visitRet rootbox::visit(visitArg m)
{
    switch (m)
    {
        case visitarg_InvalidateAll:
        {
//            flags &= ~(BNFLAG_MEASURED | BNFLAG_COLORED);
            for (auto i = child.begin(); i != child.end(); ++i)
            {
                i->cbox->visit(m);
            }
            return visitret_OK;
        }
        default:
        {
            assert(false);
            return visitret_OK;
        }
    }

}

boxbase * rootbox::copy()
{
    assert(false);
    return nullptr;
}



static void _key_copy_helper_left(rootbox* r, boxbase* U, const std::vector<int32_t>* A, size_t j)
{
    if (U->get_type() == BNTYPE_CELL)
    {
        cellbox* u = dynamic_cast<cellbox*>(U);
        r->child.push_back(cellarrayelem(u->copy()));
        return;
    }

    assert(j < A->size());
    assert(U->get_type() == BNTYPE_CELLGROUP);
    cellgroupbox* u = dynamic_cast<cellgroupbox*>(U);

    int32_t i = (*A)[j];
    if (i == 0)
    {
        for (size_t k = j + 1; k < A->size(); k++)
        {
            if ((*A)[k] != 0)
                goto doit;
        }
        r->child.push_back(cellarrayelem(u->copy()));
        return;
    }

doit:
    _key_copy_helper_left(r, u->child[i].cbox, A, j + 1);    
    for (i++; i < u->child.size(); i++)
        r->child.push_back(u->child[i].cbox->copy());
}


static void _key_copy_helper_right(rootbox* r, boxbase* U, const std::vector<int32_t> * B, size_t j)
{
    if (U->get_type() == BNTYPE_CELL)
    {
        return;
    }

    assert(j < B->size());
    assert(U->get_type() == BNTYPE_CELLGROUP);
    cellgroupbox* u = dynamic_cast<cellgroupbox*>(U);

    int32_t i = (*B)[j];
    _key_copy_helper_right(r, u->child[i].cbox, B, j + 1);
    for (i--; i >= 0; i--)
        r->child.push_back(u->child[i].cbox->copy());
}

static void _key_copy_helper(rootbox* r, boxbase* U, const std::vector<int32_t> * A, const std::vector<int32_t> * B, size_t j)
{
    if (U->get_type() == BNTYPE_CELL)
    {
        return;
    }

    assert(U->get_type() == BNTYPE_CELLGROUP);
    cellgroupbox* u = dynamic_cast<cellgroupbox*>(U);

    int32_t i = (*A)[j];
    if (i == (*B)[j])
    {
        _key_copy_helper(r, u->child[i].cbox, A, B, j + 1);
    }
    else
    {
        _key_copy_helper_left(r, u->child[i].cbox, A, j + 1);
        for (++i; i < (*B)[j]; ++i)
            r->child.push_back(cellarrayelem(u->child[i].cbox->copy()));
        _key_copy_helper_right(r, u->child[i].cbox, B, j + 1);
    }
}

void rootbox::key_copy(boxbase*&b)
{
    if (cursor_b.empty())
    {
        cursor_t = cursor_a;
        _us()->key_copy(b);
    }
    else
    {
        std::vector<int32_t> * A = &cursor_a, * B = &cursor_b;

        for (size_t i = 0; i < cursor_a.size(); i++)
        {
            if (i >= cursor_b.size()) // should not happen
                break;

            if (cursor_a[i] < cursor_b[i])
            {
                break;
            }
            else if (cursor_a[i] > cursor_b[i])
            {
                std::swap(A, B);
                break;
            }
        }

        rootbox* r = new rootbox();
        assert(b == nullptr);
        b = r;
        int32_t i = (*A)[0];
        if (i == (*B)[0])
        {
            if (i < child.size())
                _key_copy_helper(r, child[i].cbox, A, B, 1);
        }
        else
        {
            _key_copy_helper_left(r, child[i].cbox, A, 1);
            for (++i; i < (*B)[0]; ++i)
                r->child.push_back(cellarrayelem(child[i].cbox->copy()));
            if ((*B)[0] < child.size())
                _key_copy_helper_right(r, child[i].cbox, B, 1);
        }
    }
}

void rootbox::key_paste(boxbase*&b)
{
    if (cursor_b.empty())
    {
        _us()->key_paste(b);
    }
    else if (b != nullptr && b->get_type() == BNTYPE_ROOT)
    {
        delete_selection();
        rootbox* r = dynamic_cast<rootbox*>(b);
        cursor_t = cursor_a;
        if (cursor_a[0] < child.size())
        {
            dynamic_cast<cellbox*>(_us())->flags |= BNFLAG_CURSORAMARK;
            for (size_t i = r->child.size(); i > 0; i--)
                cell_insert(r->child[i - 1].cbox->copy());
            cursor_t.clear();
            bool found = _find_mark(this, cursor_t, BNFLAG_CURSORAMARK);
            assert(found);
            dynamic_cast<cellbox*>(_us())->flags &= ~BNFLAG_CURSORAMARK;
            cursor_a = cursor_t;        
        }
        else
        {
            for (size_t i = r->child.size(); i > 0; i--)
                cell_insert(r->child[i - 1].cbox->copy());
            cursor_a.clear();
            cursor_a.push_back(child.size());
        }
        cursor_b = cursor_a;
    }
    else if (b != nullptr && b->get_type() == BNTYPE_ROW)
    {
        delete_selection();
        rowbox* r = dynamic_cast<rowbox*>(b->copy());
        if (r->child.empty() || ibox_type(r->child.back().cibox) != BNTYPE_NULLER)
            r->child.push_back(iboxarrayelem(new nullbox()));
        r->cursor_b = r->cursor_a = r->child.size() - 1;
        cursor_t = cursor_a;
        if (cursor_a[0] < child.size())
        {
            dynamic_cast<cellbox*>(_us())->flags |= BNFLAG_CURSORAMARK;
            cell_insert(new cellbox(r, cellt_INPUT));
            cursor_t.clear();
            bool found = _find_mark(this, cursor_t, BNFLAG_CURSORAMARK);
            assert(found);
            dynamic_cast<cellbox*>(_us())->flags &= ~BNFLAG_CURSORAMARK;
            cursor_a = cursor_t;        
        }
        else
        {
            cell_insert(new cellbox(r, cellt_INPUT));
            cursor_a.clear();
            cursor_a.push_back(child.size());
        }
        cursor_b = cursor_a;
    }
    else if (b != nullptr && b->get_type() == BNTYPE_GRID)
    {
        delete_selection();
    }
}

void rootbox::toggle_cell_expr()
{
    if (cursor_b.empty())
    {
        dynamic_cast<cellbox*>(_us())->toggle_cell_expr();
        return;
    }

    assert(false && "selection toggle not implemented");
}

void rootbox::insert_char(int32_t c)
{
    if (cursor_b.empty())
    {
        _us()->insert_char(c);
        return;
    }

    delete_selection();

    rowbox* newrow = new rowbox(2, 1,1);
    newrow->child[0].cibox = iboximm_make(c);
    newrow->child[1].cibox.ptr = new nullbox();
    cellbox* newcell = new cellbox(newrow, cellt_INPUT);
    cursor_t = cursor_a;
    cell_insert(newcell);
    cursor_a = cursor_t;
    cursor_b.clear();
}


moveRet rootbox::move(boxbase*&b, moveArg m)
{
    moveRet r;
    cursor_t = cursor_a;
    switch (m)
    {
        case movearg_Up:
        case movearg_Left:
        {
            if (cursor_b.empty())
            {
                r = _us()->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                    cursor_b = cursor_a;
                return moveret_OK;
            }
            else
            {
                if (goto_prev_cellbreak())
                {
                    r = _us()->move(b, movearg_Last);
                    assert(r == moveret_OK || r == moveret_End);
                    cursor_a = cursor_t;
                    cursor_b.clear();
                    return moveret_OK;
                }
                else
                {
                    cursor_b = cursor_a;
                    return moveret_OK;
                }
            }
        }
        case movearg_ShiftUp:
        {
            if (cursor_b.empty())
            {
                r = _us()->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return moveret_OK;
            }
            else
            {
                cursor_t = cursor_a;
                goto_prev_cellbreak();
                cursor_a = cursor_t;
                return moveret_OK;
            }
        }
        case movearg_ShiftLeft:
        {
            if (cursor_b.empty())
            {
                r = _us()->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return moveret_OK;
            }
            else
            {
                return moveret_OK;
            }
        }
        case movearg_Down:
        case movearg_Right:
        {
            if (cursor_b.empty())
            {
                r = _us()->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                if (r == moveret_End)
                {
                    goto_next_cellbreak();
                    cursor_b = cursor_t;
                    cursor_a = cursor_t;
                }
                return moveret_OK;
            }
            else
            {
                if (cursor_t[0] < child.size())
                {
                    r = _us()->move(b, movearg_First);
                    assert(r == moveret_OK || r == moveret_End);
                    cursor_b.clear();
                    return moveret_OK;
                }
                else
                {
                    cursor_b = cursor_a;
                    return moveret_OK;
                }
            }
        }
        case movearg_ShiftRight:
        {
            if (cursor_b.empty())
            {
                r = _us()->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return moveret_OK;
            }
            else
            {
                return moveret_OK;
            }
        }
        case movearg_ShiftDown:
        {
            if (cursor_b.empty())
            {
                r = _us()->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return moveret_OK;
            }
            else
            {
                cursor_t = cursor_a;
                goto_next_cellbreak();
                cursor_a = cursor_t;
                return moveret_OK;
            }            
        }
        case movearg_Home:
        case movearg_End:
        case movearg_Tab:
        {
            if (cursor_b.empty())
            {
                r = _us()->move(b, m);
                assert(r == moveret_OK || r == moveret_End);
                return moveret_OK;
            }
            else
            {
                return moveret_OK;
            }
        }
        case movearg_Switch:
        {
            if (cursor_b.empty())
            {
                r = _us()->move(b, m);
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

insertRet rootbox::insert(boxbase*&b, insertArg m)
{
    if (cursor_b.empty())
        return _us()->insert(b, m);

    switch (m)
    {
        case insertarg_Fraction:
        case insertarg_Sqrt:
        case insertarg_Newline:
        case insertarg_Subscript:
        case insertarg_Superscript:
        case insertarg_Underscript:
        case insertarg_Overscript:
        case insertarg_Text:
        case insertarg_Graphics3D:
        {
            delete_selection();
            cursor_t = cursor_a;
            rowbox* newrow = new rowbox(1, 0,0);
            newrow->child[0].cibox.ptr = new nullbox();
            cell_insert(new cellbox(newrow, cellt_INPUT));
            cursor_a = cursor_t;
            cursor_b.clear();
            return _us()->insert(b, m);
        }
        default:
        {
            assert(false);
            return insertret_Done;
        }
    }
}

removeRet rootbox::remove(boxbase*&b, removeArg m)
{
//std::cout << "cellbox::remove called" << std::endl;
    assert(b == nullptr);
    removeRet r;

    if (cursor_b.empty())
    {
        r = _us()->remove(b, m);
        return removeret_OK;
    }
    else
    {
        delete_selection();
        return removeret_OK;
    }
}

void rootbox::key_shiftenter(notebook* nb)
{
    erase_cell_mark(BNFLAG_PRINTMARK);

    // nothing to do at cell break
    if (!cursor_b.empty())
        return;

    cursor_t = cursor_a;
    cellbox* U = dynamic_cast<cellbox*>(_us());
    U->flags |= BNFLAG_PRINTMARK;
    uex e(U->body.cbox->get_ex());
    swrite_byte(fp_in, CMD_EXPR);
    e.set(emake_node(gs.sym_sEnterTextPacket.copy(), e.release()));
    swrite_ex(fp_in, e.get());
    fflush(fp_in);

    /* put line number on our cell */
    if (U->label.cbox != nullptr)
    {
        delete U->label.cbox;
        U->label.cbox = nullptr;
    }
    if (nb->in_name.get() != nullptr)
    {
        boxbase * label = boxbase_from_ex(nb->in_name.get());
        if (label != nullptr)
        {
            if (label->get_type() != BNTYPE_ROW)
            {
                delete label;
            }
            else
            {
                U->label.cbox = dynamic_cast<rowbox*>(label);
            }
        }
    }

    /* delete supid stuff after our cell */
    goto_next_cellbreak();
    while (cursor_t[0] < child.size() &&
           _us()->get_type() == BNTYPE_CELL && (
                U = dynamic_cast<cellbox*>(_us()),
                   U->celltype == cellt_OUTPUT
                || U->celltype == cellt_PRINT
                || U->celltype == cellt_MESSAGE))
    {
        delete cell_remove();
    }

    cursor_a = cursor_t;
    cursor_b = cursor_t;
}

ex rootbox::get_ex()
{
    uex v(gs.sym_sList.get(), child.size());
    for (int32_t i = 0; i < child.size(); i++)
        v.push_back(child[i].cbox->get_ex());
    return emake_node(gs.sym_sNotebook.copy(), v.release());
}

void rootbox::get_cursor(aftransform * T)
{
    if (cursor_b.empty())
    {
        cursor_t = cursor_a;
        _us()->get_cursor(T);
    }
    else
    {
        T->orig_x = 20;
        T->orig_y = 0;
        T->theta = 0;
        T->cos_theta = 1.0;
        T->sin_theta = 0.0;
    }

    int32_t y;
    if (cursor_a[0] < child.size())
    {
        y = child[cursor_a[0]].offy;
        boxbase* u = child[cursor_a[0]].cbox;
        for (size_t i = 1; i < cursor_a.size(); i++)
        {
            y += offyat(u, cursor_a[i]);
            u = childat(u, cursor_a[i]);
        }
    }
    else
    {
        y = sizey;
    }
    T->orig_y += y;
}

void rootbox::measure(boxmeasurearg ma)
{
    int32_t accheight = 0;
    int32_t maxwidth = 0;
    for (auto& i : child)
    {
        i.cbox->measure(ma);
        i.offx = 0;
        i.offy = accheight;
        maxwidth = std::max(maxwidth, i.cbox->sizex);
        accheight += i.cbox->sizey;
    }

    sizex = maxwidth;
    sizey = accheight;
    centery = accheight/2;
}

void rootbox::draw_pre(boxdrawarg da)
{
    if (cursor_b.empty())
    {
        int32_t y = child[cursor_a[0]].offy;
        int32_t x = child[cursor_a[0]].offx;
        boxbase* u = child[cursor_a[0]].cbox;
        for (size_t i = 1; i < cursor_a.size(); i++)
        {
            y += offyat(u, cursor_a[i]);
            x += offxat(u, cursor_a[i]);
            u = childat(u, cursor_a[i]);
        }
        u->draw_pre(boxdrawarg(da.nb, da.globx + x, da.globy + y, 0, da.T));
        return;
    }

    size_t n = cursor_a.size();
    if (n != cursor_b.size())
        return;

    int32_t y = sizey;
    if (cursor_a[0] < child.size())
    {
        if (cursor_a[0] != cursor_b[0])
            return;

        y = child[cursor_a[0]].offy;
        boxbase* u = child[cursor_a[0]].cbox;
        for (size_t i = 1; i < n; i++)
        {
            if (cursor_a[i] != cursor_b[i])
                return;
            y += offyat(u, cursor_a[i]);
            u = childat(u, cursor_a[i]);
        }
    }

    y += da.globy;
    drawtline(0, y, glb_image.sizex, y, da.nb->zoomint*0.1, da.nb->cCursor, da.T);
}


void rootbox::draw_main(boxdrawarg da)
{
    if (cursor_b.empty())
    {
        for (int32_t i = 0; i < child.size(); i++)
        {
            child[i].cbox->draw_main(boxdrawarg(da, child[i].offx, child[i].offy, cursor_a[0] == i ? 0 : DFLAG_IGNORESEL));
        }        
    }
    else
    {
        for (auto&i : child)
        {
            i.cbox->draw_main(boxdrawarg(da, i.offx, i.offy, DFLAG_IGNORESEL));
        }
    }
}

static void _draw_post_helper_left(boxbase* U, boxdrawarg da, const std::vector<int32_t> * A, size_t j)
{
    if (U->get_type() == BNTYPE_CELL)
    {
        cellbox* u = dynamic_cast<cellbox*>(U);
        _draw_cellgroup_bracket(u, da, true);
        return;
    }

    assert(j < A->size());
    assert(U->get_type() == BNTYPE_CELLGROUP);
    cellgroupbox* u = dynamic_cast<cellgroupbox*>(U);

    int32_t i = (*A)[j];

    if (i == 0)
    {
        for (size_t k = j + 1; k < A->size(); k++)
        {
            if ((*A)[k] != 0)
                goto doit;
        }
        _draw_cellgroup_bracket(u, da, true);
        return;
    }

doit:
    _draw_post_helper_left(u->child[i].cbox, boxdrawarg(da, u->child[i].offx, u->child[i].offy), A, j + 1);    
    for (i++; i < u->child.size(); i++)
        _draw_cellgroup_bracket(u->child[i].cbox, boxdrawarg(da, u->child[i].offx, u->child[i].offy), true);
}


static void _draw_post_helper_right(boxbase* U, boxdrawarg da, const std::vector<int32_t> * B, size_t j)
{
    if (U->get_type() == BNTYPE_CELL)
    {
        return;
    }

    assert(j < B->size());
    assert(U->get_type() == BNTYPE_CELLGROUP);
    cellgroupbox* u = dynamic_cast<cellgroupbox*>(U);

    int32_t i = (*B)[j];
    _draw_post_helper_right(u->child[i].cbox, boxdrawarg(da, u->child[i].offx, u->child[i].offy), B, j + 1);
    for (i--; i >= 0; i--)
        _draw_cellgroup_bracket(u->child[i].cbox, boxdrawarg(da, u->child[i].offx, u->child[i].offy), true);
}

static void _draw_post_helper(boxbase* U, boxdrawarg da, const std::vector<int32_t> * A, const std::vector<int32_t> * B, size_t j)
{
    if (U->get_type() == BNTYPE_CELL)
    {
        return;
    }

    assert(U->get_type() == BNTYPE_CELLGROUP);
    cellgroupbox* u = dynamic_cast<cellgroupbox*>(U);

    int32_t i = (*A)[j];
    if (i == (*B)[j])
    {
        _draw_post_helper(u->child[i].cbox, boxdrawarg(da, u->child[i].offx, u->child[i].offy), A, B, j + 1);
    }
    else
    {
        _draw_post_helper_left(u->child[i].cbox, boxdrawarg(da, u->child[i].offx, u->child[i].offy), A, j + 1);
        for (++i; i < (*B)[j]; ++i)
            _draw_cellgroup_bracket(u->child[i].cbox, boxdrawarg(da, u->child[i].offx, u->child[i].offy), true);
        _draw_post_helper_right(u->child[i].cbox, boxdrawarg(da, u->child[i].offx, u->child[i].offy), B, j + 1);
    }
}


void rootbox::draw_post(boxdrawarg da)
{

    if (cursor_b.empty())
    {
        int32_t x = 0, y = 0;
        boxbase* U = this;
        for (int32_t i = 0; i < cursor_a.size(); i++)
        {
            x += offxat(U, cursor_a[i]);
            y += offyat(U, cursor_a[i]);
            U = childat(U, cursor_a[i]);
        }
        U->draw_post(boxdrawarg(da, x, y));
        return;
    }
    else
    {
        std::vector<int32_t> * A = &cursor_a, * B = &cursor_b;

        for (size_t i = 0; i < cursor_a.size(); i++)
        {
            if (i >= cursor_b.size()) // should not happen
                break;

            if (cursor_a[i] < cursor_b[i])
            {
                break;
            }
            else if (cursor_a[i] > cursor_b[i])
            {
                std::swap(A, B);
                break;
            }
        }

        int32_t i = (*A)[0];
        if (i == (*B)[0])
        {
            if (i < child.size())
                _draw_post_helper(child[i].cbox, boxdrawarg(da, child[i].offx, child[i].offy), A, B, 1);
        }
        else
        {
            _draw_post_helper_left(child[i].cbox, boxdrawarg(da, child[i].offx, child[i].offy), A, 1);
            for (++i; i < (*B)[0]; ++i)
                _draw_cellgroup_bracket(child[i].cbox, boxdrawarg(da, child[i].offx, child[i].offy), true);
            if ((*B)[0] < child.size())
                _draw_post_helper_right(child[i].cbox, boxdrawarg(da, child[i].offx, child[i].offy), B, 1);
        }
    }
}

