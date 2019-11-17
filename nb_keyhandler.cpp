#include <stack>

#include "types.h"
#include "graphics.h"
#include "font.h"
#include "asm_inlines.h"
#include "boxes.h"
#include "notebook.h"
#include "os_utilities.h"
#include "timing.h"

#include "ex.h"
#include "ex_print.h"
#include "ex_parse_boxes.h"
#include "ex_print_boxes.h"
#include "eval.h"
#include "serialize.h"
#include "box_convert.h"


//void boxplot3d_render(boxplot3d * us);


int notebook::stub_key_up()
{
    if (bptr_type(_us()) == BNTYPE_TEXT)
    {
//        return boxtext_key_up(_us());
    }
    return 0;
}

int notebook::stub_key_down()
{
    if (bptr_type(_us()) == BNTYPE_TEXT)
    {
//        return boxtext_key_down(_us());
    }
    return 0;
}

void notebook::stub_key_end()
{
    if (bptr_type(_us()) == BNTYPE_TEXT)
    {
//        boxtext_key_end(_us());
        _invalidate_downto(1);
    }
    return;
}

void notebook::stub_key_home()
{
    if (bptr_type(_us()) == BNTYPE_TEXT)
    {
//        boxtext_key_home(_us());
        _invalidate_downto(1);
    }
    return;
}


void notebook::stub_key_tab()
{
    if (bptr_type(_us()) == BNTYPE_TEXT)
    {
        boxtext * us = bto_text(_us());
//        boxtext_insert_tab(us);
        _invalidate_downto(1);
    }
    return;
}

void notebook::stub_insert_newline()
{
    if (bptr_type(_us()) == BNTYPE_TEXT)
    {
        boxtext * us = bto_text(_us());
//        boxtext_insert_newline(us);
        _invalidate_downto(1);
    }
    return;
}

void notebook::stub_key_backspace()
{
    if (bptr_type(_us()) == BNTYPE_TEXT)
    {
        boxtext * us = bto_text(_us());
//        boxtext_backspace(us);
        _invalidate_downto(1);
    }
    return;
}

void notebook::stub_insert_char(char16_t c)
{
    if (bptr_type(_us()) == BNTYPE_TEXT)
    {
        boxtext * us = bto_text(_us());
//        boxtext_insert_char(us, c);
        _invalidate_downto(1);
    }
    else if (bptr_type(_us()) == BNTYPE_PLOT3D)
    {
/*
        boxplot3d * us = (boxplot3d *) uus;
        quaterion_t q;

        if (c == 'w') {
            q->w = cos(-0.03); q->x = sin(-0.03); q->y = 0; q->z = 0;
            QuatMul(us->qangle, q, us->qangle);
            boxplot3d_render(us);
        } else if (c == 's') {
            q->w = cos(+0.03); q->x = sin(+0.03); q->y = 0; q->z = 0;
            QuatMul(us->qangle, q, us->qangle);
            boxplot3d_render(us);
        } else if (c == 'a') {
            q->w = cos(-0.03); q->x = 0; q->y = sin(-0.03); q->z = 0;
            QuatMul(us->qangle, q, us->qangle);
            boxplot3d_render(us);
        } else if (c == 'd') {
            q->w = cos(+0.03); q->x = 0; q->y = sin(+0.03); q->z = 0;
            QuatMul(us->qangle, q, us->qangle);
            boxplot3d_render(us);
        } else if (c == 'e') {
            q->w = cos(-0.03); q->x = 0; q->y = 0; q->z = sin(-0.03);
            QuatMul(us->qangle, q, us->qangle);
            boxplot3d_render(us);
        } else if (c == 'q') {
            q->w = cos(+0.03); q->x = 0; q->y = 0; q->z = sin(+0.03);
            QuatMul(us->qangle, q, us->qangle);
            boxplot3d_render(us);
        } else if (c == 'x') {
            us->fov = us->fov * 1.05;
            boxplot3d_render(us);
        } else if (c == 'c') {
            us->fov = us->fov / 1.05;
            boxplot3d_render(us);
        }
*/
    }
}






bool notebook::placeholder_selected()
{
    if (_depth() < 1 || selection.type != SELTYPE_ROW)
        return false;

    if (bnode_len(_parent()) != 2)
        return false;

    if (!bis_char(bnode_child(_parent(),0), CHAR_Placeholder))
        return false;

    return true;
}


void notebook::insert_placholder_if_needed()
{
    assert(we_are_legal());

//printf("insert_placholder_if_needed\n");

    if (bnode_type(_parent()) != BNTYPE_ROW)
    {
        return;
    }
    if (bnode_len(_parent()) == 1)
    {
        int t = bnode_type(_gparent());
        if (    t == BNTYPE_SQRT || t == BNTYPE_GRID || t == BNTYPE_FRAC
             || t == BNTYPE_SUB || t == BNTYPE_SUBSUPER || t == BNTYPE_SUPER
             || t == BNTYPE_UNDER || t == BNTYPE_OVER || t == BNTYPE_UNDEROVER)
        {
            _insert(boxchar_create(CHAR_Placeholder));
            _right1();
            selection.type = SELTYPE_ROW;
            selection.data.resize(1);
            selection.data[0] = 0;
        }

    }
}

void notebook::scroll_up()
{
    offy += glb_image.sizey/10;
}

void notebook::scroll_down()
{
    offy -= glb_image.sizey/10;
}

void notebook::select_side_placeholder_if_needed(int side)
{
    assert(we_are_legal());

    if (bnode_type(_parent()) != BNTYPE_ROW)
    {
        return;
    }
    if (bnode_len(_parent()) == 2 && bis_char(bnode_child(_parent(),0), CHAR_Placeholder))
    {
        selection.type = SELTYPE_ROW;
        selection.data.resize(1);
        assert(_pi() < 2);
        if (side == 0)
        {
            selection.data[0] = 1;
            if (_pi() == 1)
            {
                _left1();
            }
        }
        else
        {
            selection.data[0] = 0;
            if (_pi() == 0)
            {
                _right1();
            }
        }
    }
}


void notebook::select_placeholder_if_needed()
{
    assert(we_are_legal());

    if (bnode_type(_parent()) != BNTYPE_ROW)
    {
        return;
    }
    if (   bnode_len(_parent()) == 2
        && _pi() == 1 
        && bis_char(bnode_child(_parent(),0), CHAR_Placeholder))
    {
        selection.type = SELTYPE_ROW;
        selection.data.resize(1);
        selection.data[0] = 0;
    }
}



void notebook::select_prev_if_possible()
{
    int npi, i;
    box topnode = _us();
    boxnode * parent = bto_node(_parent());
    int pi = _pi();

    if (selection.type == SELTYPE_NONE && boxnode_type(parent) == BNTYPE_ROW)
    {
        npi = pi;
        while (npi > 0)
        {
            int mtype, ptype;
            if (!bis_char(parent->array[npi - 1].child))
            {
                if (pi==npi && bis_node(parent->array[npi - 1].child))
                {
                    npi--;
                    break;
                }
                else
                {
                    break;
                }
            }
            else
            {
                ptype = btype(parent->array[npi - 1].child)&65535;
                if (ptype == ']' || ptype == '}' || ptype == ')' || ptype == CHAR_RightDoubleBracket)
                {
                    std::stack<int> estack;
                    estack.push(ptype);
                    while (--npi > 0 && estack.size() > 0) {
                        if (!bis_char(parent->array[npi - 1].child)) {
                            continue;
                        }
                        ptype = btype(parent->array[npi - 1].child)&65535;
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
                else if (pi==npi && (ptype == CHAR_Sum || ptype == CHAR_Product))
                {
                    --npi;
                    break;
                } else {
                    break;
                }
            }
        }
        if (npi < pi)
        {
            selection.type = SELTYPE_ROW;
            selection.data.resize(1);
            selection.data[0] = npi;
        }
    }
}









/**************** navigation **************/

bool _notebook_click_helper(boxnode * us, std::vector<int>&istack, int x, int y) {

//printf("Us type: %d\n",btype(Us));

    x = clamp(x, 0, boxnode_sizex(us) - 1);
    y = clamp(y, 0, boxnode_sizey(us) - 1);

    if (us->header.type == BNTYPE_ROW)
    {
//        rasterfont * f = fontint_to_fontp(bnode_extra0(us));
//        int default_sizey = f->chars[99].sizey;
//        int default_centery = f->chars[99].centery;
        uint32_t fs = boxnode_extra0(us);
        int default_sizey = fontsize_default_sizey(fs);
        int default_centery = fontsize_default_centery(fs);

        istack.push_back(0);
        bool back_is_char = true;
        uint32_t record = -1;
        x+=2; y+=0;
        for (int i=0; i<us->len; i++)
        {
            int child_type, child_sizex, child_sizey, child_centery;
            int child_offx = us->array[i].offx;
            int child_offy = us->array[i].offy;
            bget_header(child_type, child_sizex, child_sizey, child_centery, us->array[i].child);
            int distx = 0;
            distx = std::max(distx, child_offx - x);
            distx = std::max(distx, 1 + x - child_offx - child_sizex);
            int disty = std::abs(child_offy + child_centery - y);
            if (record > uint32_t(distx) + uint32_t(disty))
            {
                record = uint32_t(distx) + uint32_t(disty);
                istack.back() = i;
                back_is_char = child_type >= BNTYPE_NULLER;
            }
        }

//printf("istack back: %d   is char %d\n",istack.back(),back_is_char);

        if (!back_is_char)
        {
            _notebook_click_helper(bto_node(us->array[istack.back()].child), istack, x - us->array[istack.back()].offx,
                                                                           y - us->array[istack.back()].offy);
        }
        return true;

    }
    else if (us->header.type == BNTYPE_COL || us->header.type == BNTYPE_UNDER
                                           || us->header.type == BNTYPE_OVER
                                           || us->header.type == BNTYPE_UNDEROVER)
    {
        for (int i=0; i<us->len; i++)
        {
            boxnode * child = bto_node(us->array[i].child);
            if (/*0 <= y-us->array[i].offy &&*/ y - us->array[i].offy < boxnode_sizey(child))
            {
                istack.push_back(i);
                _notebook_click_helper(child, istack, x-us->array[i].offx, y-us->array[i].offy);
                return true;
            }
        }
        assert(false);
        return false;
    }
    else if (us->header.type == BNTYPE_SUB)
    {
        assert(us->len == 1);
        boxnode * child0 = bto_node(us->array[0].child);
        if (x <= 1 || y < us->array[0].offy)
        {
            return true;
        }
        istack.push_back(0);
        _notebook_click_helper(child0, istack, x - us->array[0].offx, y - us->array[0].offy);
        return true;

    }
    else if (us->header.type == BNTYPE_SUPER)
    {
        assert(us->len == 1);
        boxnode * child0 = bto_node(us->array[0].child);
        if (x <= 1 || y >= us->array[0].offy + boxnode_sizey(child0))
        {
            return true;
        }
        istack.push_back(0);
        _notebook_click_helper(child0, istack, x - us->array[0].offx, y - us->array[0].offy);
        return true;
    }
    else if (us->header.type == BNTYPE_SQRT)
    {
        assert(us->len == 1);
        boxnode * child0 = bto_node(us->array[0].child);
        if (x < us->array[0].offx)
        {
            return true;
        }
        istack.push_back(0);
        _notebook_click_helper(child0, istack, x - us->array[0].offx, y - us->array[0].offy);
        return true;
    }
    else if (us->header.type == BNTYPE_ROT)
    {
        assert(us->len == 1);
        double xx = x - us->array[0].offx;
        double yy = y - us->array[0].offy;
        double c, s;
        cossinth(c, s, us->extra0);
        int32_t nx = c*xx + s*yy;
        int32_t ny = -s*xx + c*yy;
        boxnode * child0 = bto_node(us->array[0].child);
        if (nx < 0 || nx > boxnode_sizex(child0) || ny < 0 || ny > boxnode_sizey(child0))
        {
            return true;
        }
        istack.push_back(0);
        _notebook_click_helper(child0, istack, nx, ny);
        return true;
    }
    else if (us->header.type == BNTYPE_GRID || us->header.type == BNTYPE_FRAC || us->header.type == BNTYPE_SUBSUPER)
    {
        if (x < 1)
        {
            return true;
        }
        int best = 0;
        int score = 2000000000;
        for (int i=0; i<us->len && score > 0; i++)
        {
            boxnode * child = bto_node(us->array[i].child);
            int newscore = std::max(0, us->array[i].offx - x)
                         + std::max(0, x - (us->array[i].offx + boxnode_sizex(child) - 1))
                         + std::max(0, us->array[i].offy - y)
                         + std::max(0, y - (us->array[i].offy + boxnode_sizey(child) - 1));
            if (newscore < score)
            {
                best = i;
                score = newscore;
            }
        }
        istack.push_back(best);
        _notebook_click_helper(bto_node(us->array[best].child), istack, x-us->array[best].offx, y-us->array[best].offy);
        return true;
/*
    } else if (us->type == BNTYPE_STUB) {
        assert(us->len == 1);
        child0 = us->array[0].child;
        istack.push_back(0);
        _notebook_click_helper(child0, istack, x-us->array[0].offx, y-us->array[0].offy);
        return true;
*/
    } else if (us->header.type == BNTYPE_CELL) {
        assert(us->len >= 1);
        boxnode * child0 = bto_node(us->array[0].child);
        if (y < us->array[0].offy) {
            return true;
        } else if (y >= us->array[0].offy + boxnode_sizey(child0)) {
            return false;
        } else {
//printf("trying cell with x = %d,,xoffset %d\n",x,us->array[0].offx);
            istack.push_back(0);
            _notebook_click_helper(child0, istack, x - us->array[0].offx, y - us->array[0].offy);
            return true;
        }

    } else if (us->header.type == BNTYPE_CELLGROUP) {
        for (int i=0; i<us->len; i++) {
            boxnode * child = bto_node(us->array[i].child);
            if (/*0 <= y-child->offy &&*/ y - us->array[i].offy < boxnode_sizey(child)) {
                istack.push_back(i);
                if (_notebook_click_helper(child, istack, x, y-us->array[i].offy)) {
                    return true;
                } else {
                    //if (i + 1 < us->len) {
                    //    istack.back() = i + 1;
                    //    return true;
                    //} else {
                        istack.pop_back();
                        //return false;
                    //}
                }
            }
        }
        return false;
    }
    else if (us->header.type == BNTYPE_ROOT)
    {
        for (int i=0; i+1<us->len; i++)
        {
            boxnode * child = bto_node(us->array[i].child);
            if (/*0 <= y-child->offy &&*/ y - us->array[i].offy < boxnode_sizey(child))
            {
                istack.push_back(i);
                if (_notebook_click_helper(child, istack, x, y - us->array[i].offy))
                {
                    return true;
                }
                else
                {
                    istack.pop_back();
                    //istack.back() = i + 1;
                    //return true;
                }
            }
        }
        istack.push_back(us->len-1);
        return true;
    }
    else
    {
        assert(false);
        return false;
    }
}

void notebook::handle_click(int x, int y)
{
//std::cout << "handle_click: " << x << "," << y << std::endl;
    if (handle_cellclick(x,y,false))
    {
        return;
    }
    cursor_needs_fitting = true;
    std::vector<int> istack;
    _notebook_click_helper(root, istack, x, y);
    selection.type = SELTYPE_NONE;
    cursor1.resize(1);
    for (size_t i = 0; i < istack.size(); i++)
    {
        _down1(istack[i]);
    }
    return;
}

void notebook::handle_doubleclick(int x, int y)
{
//std::cout << "handle_click: " << x << "," << y << std::endl;
    if (handle_cellclick(x,y,true))
    {
        return;
    }
    cursor_needs_fitting = true;
    std::vector<int> istack;
    _notebook_click_helper(root, istack, x, y);
    selection.type = SELTYPE_NONE;
    cursor1.resize(1);
    for (size_t i = 0; i < istack.size(); i++)
    {
        _down1(istack[i]);
    }
    return;
}

void notebook::zoom_in() {
    set_zoom(zoomint + 1);
}

void notebook::zoom_out() {
    set_zoom(zoomint - 1);
}


int bnode_len_visible(box b) {
    return bnode_type(b) == BNTYPE_CELL ? 1 : bnode_len(b);
}

void notebook::preorder_next() {
    if (bis_node(_us())) {
        _down1(0);
    } else {
        while (_pi() + 1 >= bnode_len_visible(_parent())) {
            _up1();
            if (_depth() < 1)
                return;
        }
        _right1();
    }
}

void notebook::preorder_prev() {
    if (_depth() > 0 && _pi() == 0) {
        _up1();       
    } else {
        /* goto previous child */
        if (_depth() > 0) {
            _left1();
        }
        while (bis_node(_us())) {
            _down1(bnode_len_visible(_us()) - 1);
        }
    }
}



void notebook::select_grid_dir(int dx, int dy)
{
    assert(bnode_type(_parent()) == BNTYPE_ROW);
    assert(bnode_type(_gparent()) == BNTYPE_GRID);

    int gpi = _gpi();
    if (selection.type != SELTYPE_GRID)
    {
        selection.type = SELTYPE_GRID;
        selection.data.resize(1);
        selection.data[0] = gpi;
    }
    _up2();
    int width = bnode_extra0(_us());
    int height = bnode_extra1(_us());
    int x = gpi % width;
    int y = gpi / width;
    x = std::max(std::min(x+dx,width-1),0);
    y = std::max(std::min(y+dy,height-1),0);
    _down2(y*width+x, 0);
    assert(bnode_type(_parent()) == BNTYPE_ROW);
}



bool notebook::we_first_cellbreak()
{
    if (bnode_type(_parent()) == BNTYPE_ROOT && bnode_len(_parent()) == 1)
    {
        // empty notebook
        return true;
    }
    if (!bis_ptr(_us()) || bptr_type(_us()) != BNTYPE_CELL)
    {
        return false;
    }
    for (size_t i=0; i+2<cursor1.size(); i++)
    {
        assert(bnode_type(cursor1[i+1].node) == BNTYPE_CELLGROUP);
        if (cursor1[i].idx != 0)
        {
            return false;
        }
    }
    return _pi() == 0;
}

bool notebook::we_last_cellbreak()
{
    return bnode_type(_parent()) == BNTYPE_ROOT && _pi() + 1 == bnode_len(_parent());
}

bool notebook::we_in_row()
{
    return bnode_type(_parent()) == BNTYPE_ROW;
}

bool notebook::we_at_cellbreak()
{
    return bnode_type(_parent()) == BNTYPE_ROOT || bnode_type(_parent()) == BNTYPE_CELLGROUP;
}

void notebook::goto_last_cellbreak()
{
    cursor1.clear();
    int i = boxnode_len(root) - 1;
    cursor1.push_back(cursorentry(root, i));
    cursor1.push_back(cursorentry(boxnode_child(root,i)));
}

void notebook::goto_first_cellbreak()
{
    cursor1.clear();
    box us = bfrom_node(root);
    while (bptr_type(us) == BNTYPE_ROOT || bptr_type(us) == BNTYPE_CELLGROUP)
    {
        cursor1.push_back(cursorentry(us, 0));
        us = bnode_child(us,0);
    }
    assert(bptr_type(us) == BNTYPE_CELL || bptr_type(us) == BNTYPE_NULLER);
    cursor1.push_back(cursorentry(us));
}

void notebook::goto_next_cellbreak()
{
//std::cout << "goto_next_cellbreak()" << std::endl;
    assert(we_at_cellbreak());
    do
    {
        if (!bis_node(_us()))
        {
//std::cout << "goto_next_cellbreak() returning" << std::endl;
            return;
        }
        else if (bnode_type(_us()) == BNTYPE_CELL)
        {
            while (_pi() + 1 >= bnode_len(_parent()))
            {
                _up1();
                assert(bnode_type(_us()) == BNTYPE_CELLGROUP);
            }
            _right1();        
        }
        else
        {
            assert(bnode_type(_us()) == BNTYPE_CELLGROUP);
            _down1(0);
        }
    } while (bptr_type(_us()) == BNTYPE_CELLGROUP);

//std::cout << "goto_next_cellbreak() returning" << std::endl;
}

void notebook::goto_prev_cellbreak()
{
    assert(we_at_cellbreak());
//std::cout << "goto_prev_cellbreak()" << std::endl;
    do
    {
        if (we_first_cellbreak())
        {
//std::cout << "goto_prev_cellbreak returning" << std::endl;
            return;
        }
        else if (_pi() == 0)
        {
//std::cout << "up1" << std::endl;
            _up1();
            assert(bnode_type(_us()) == BNTYPE_CELLGROUP);
        }
        else
        {
//std::cout << "left1" << std::endl;
            _left1();
            while (bnode_type(_us()) == BNTYPE_CELLGROUP)
            {
//std::cout << "down1" << std::endl;
                _down1(-1);
            }
        }
    } while (bptr_type(_us()) == BNTYPE_CELLGROUP);

//std::cout << "goto_prev_cellbreak returning" << std::endl;
}
/*
void notebook::preorder_prev() {

    if (_depth() > 0 && _pi() == 0) {
        _up1();       
    } else {
        // goto previous child
        if (_depth() > 0) {
            _left1();
        }
        // while there is a child, go to rightmost
        while (_us()->type < 0) {
            _down1(-1);
        }
    }
}
*/


/*
void notebook::get_cursor_coor(int&globx, int&globy)
{
    int usx = globx, usy = globy;
    box us = cursor1[0].node;
    for (size_t i = 1; i <= _depth(); i++) {
        usx += bnode_offx(us, cursor1[i-1].idx);
        usy += bnode_offy(us, cursor1[i-1].idx);
        us = cursor1[i].node;
    }
    if (bis_char(us))
    {
        if (bis_ptr(us))
        {
            usy += bptr_centery(us);
        }
        else
        {
            uint32_t fs = bnode_extra0(_parent());
            int default_centery = fontsize_default_centery(fs);
            usy += default_centery;
        }

    } else {
        if (bptr_type(us) == BNTYPE_CELL || bptr_type(us) == BNTYPE_CELLGROUP) {
            usy += 0;
        } else {
            usy += bptr_centery(us);
        }
    }
    globx = usx; globy = usy;
}
*/

void notebook::get_cursor_xytheta(int32_t&globx, int32_t&globy, uint32_t&theta)
{
    double usx = globx;
    double usy = globy;
    aftransform T;
    T.orig_x = 0.0;
    T.orig_y = 0.0;
    T.theta = 0;
    T.cos_theta = 1.0;
    T.sin_theta = 0.0;
    box us = cursor1[0].node;
    for (size_t i = 1; i <= _depth(); i++)
    {
        if (bnode_type(us) != BNTYPE_ROT)
        {
            usx += bnode_offx(us, cursor1[i - 1].idx);
            usy += bnode_offy(us, cursor1[i - 1].idx);
        }
        else
        {
            assert(cursor1[i - 1].idx == 0);
            double neworx = T.orig_x + T.cos_theta*(usx + bto_node(us)->array[0].offx) - T.sin_theta*(usy + bto_node(us)->array[0].offy);
            double newory = T.orig_y + T.sin_theta*(usx + bto_node(us)->array[0].offx) + T.cos_theta*(usy + bto_node(us)->array[0].offy);
            T.orig_x = neworx;
            T.orig_y = newory;
            T.theta += bto_node(us)->extra0;
            cossinth(T.cos_theta, T.sin_theta, T.theta);
            usx = 0;
            usy = 0;
        }
        us = cursor1[i].node;
    }
    globx = T.orig_x + T.cos_theta*(usx) - T.sin_theta*(usy);
    globy = T.orig_y + T.sin_theta*(usx) + T.cos_theta*(usy);
    theta = T.theta;
}


void notebook::get_cursor_parentT(aftransform &T, int32_t goffx, int32_t goffy)
{
    double usx = goffx;
    double usy = goffy;
    T.orig_x = 0.0;
    T.orig_y = 0.0;
    T.theta = 0;
    T.cos_theta = 1.0;
    T.sin_theta = 0.0;
    box us = cursor1[0].node;
    for (size_t i = 1; i < _depth(); i++)
    {
        if (bnode_type(us) != BNTYPE_ROT)
        {
            usx += bnode_offx(us, cursor1[i - 1].idx);
            usy += bnode_offy(us, cursor1[i - 1].idx);
        }
        else
        {
            assert(cursor1[i - 1].idx == 0);
            double neworx = T.orig_x + T.cos_theta*(usx + bto_node(us)->array[0].offx) - T.sin_theta*(usy + bto_node(us)->array[0].offy);
            double newory = T.orig_y + T.sin_theta*(usx + bto_node(us)->array[0].offx) + T.cos_theta*(usy + bto_node(us)->array[0].offy);
            T.orig_x = neworx;
            T.orig_y = newory;
            T.theta += bto_node(us)->extra0;
            cossinth(T.cos_theta, T.sin_theta, T.theta);
            usx = 0;
            usy = 0;
        }
        us = cursor1[i].node;
    }

    T.orig_x += T.cos_theta*(usx) - T.sin_theta*(usy);
    T.orig_y += T.sin_theta*(usx) + T.cos_theta*(usy);

}


bool notebook::_move_from_cell_sel(int dir)
{
    assert(selection.type == SELTYPE_CELL);

    selection.type = SELTYPE_NONE;

    int diff = 0;
    for (size_t i = 0; i < _depth(); i++)
    {
        assert(i < selection.data.size());
        diff = cursor1[i].idx - selection.data[i];
        if (diff != 0)
        {
            break;
        }
    }

    if (diff == 0)
    {
        return false;
    }

    if ((diff > 0) != (dir > 0))
    {
        // move to selection cursor
        cursor1.clear();
        cursor1.push_back(cursorentry(root));
        for (size_t j = 0; j < selection.data.size(); j++) {
            _down1(selection.data[j]);
        }

    }
    return true;
}


void notebook::move_up()
{
    int32_t x = 0, y = 0;
    uint32_t theta;
    get_cursor_xytheta(x, y, theta);
    int32_t orgx = x, orgy = y;
    int32_t newx = x, newy = y;
    std::vector<int> istack;
    while (y > 0 && newy >= orgy)
    {
        bool diff = false;
        while (!diff && y > 0)
        {
            y -= (zoomint+4)/4;
            istack.clear();
            _notebook_click_helper(root, istack, x, y);
            if (istack.size() != _depth())
            {
                diff = true;
            }
            else
            {
                for (size_t i = 0; i < istack.size(); i++)
                {
                    if (istack[i] != cursor1[i].idx)
                    {
                        diff = true;
                        break;
                    }
                }
            }
        }
        cursor1.resize(1);
        if (y > 0)
        {
            for (size_t i = 0; i < istack.size(); i++)
            {
                _down1(istack[i]);
            }
            newx = 0; newy = 0;
            get_cursor_xytheta(newx, newy, theta);
        }
        else
        {
            _down1(-1);
            break;
        }
    }
}

void notebook::move_down()
{
    int32_t x = 0, y = 0;
    uint32_t theta;
    get_cursor_xytheta(x, y, theta);
    int32_t orgx = x, orgy = y;
    int32_t newx = x, newy = y;
    std::vector<int> istack;
    while (y < boxnode_sizey(root) && newy <= orgy)
    {
        bool diff = false;
        while (!diff && y < boxnode_sizey(root))
        {
            y += (zoomint+4)/4;
            istack.clear();
            _notebook_click_helper(root, istack, x, y);
            if (istack.size() != _depth())
            {
                diff = true;
            }
            else
            {
                for (size_t i = 0; i < istack.size(); i++)
                {
                    if (istack[i] != cursor1[i].idx)
                    {
                        diff = true;
                        break;
                    }
                }
            }
        }
        cursor1.resize(1);
        if (y < boxnode_sizey(root))
        {
            for (size_t i = 0; i < istack.size(); i++)
            {
                _down1(istack[i]);
            }
            newx = 0; newy = 0;
            get_cursor_xytheta(newx, newy, theta);
        }
        else
        {
            _down1(-1);
            break;
        }
    }
}


void notebook::key_up()
{
    assert(we_are_legal());
    cursor_needs_fitting = true;
    if (selection.type == SELTYPE_CELL)
    {
        if (_move_from_cell_sel(-1))
        {
            return;
        }
    }
    selection.type = SELTYPE_NONE;
    if (we_in_stub())
    {
        if (stub_key_up())
        {
            return;
        }
    }
    if (we_in_row() || we_at_cellbreak())
    {
        move_up();
    }
    select_side_placeholder_if_needed(0);
}


void notebook::key_down()
{
    assert(we_are_legal());
    cursor_needs_fitting = true;
    if (selection.type == SELTYPE_CELL)
    {
        if (_move_from_cell_sel(1))
        {
            return;
        }
    }
    selection.type = SELTYPE_NONE;
    if (we_in_stub())
    {
        if (stub_key_down())
        {
            return;
        }
    }
    if (we_in_row() || we_at_cellbreak())
    {
        move_down();
    }
    select_side_placeholder_if_needed(1);
}

void notebook::fixselection()
{
    selection.type = SELTYPE_NONE;
    for (size_t i = 0; i < _depth() && i < selection.data.size(); i++)
    {
        if (selection.data[i] != cursor1[i].idx)
        {
            int t = bnode_type(cursor1[i].node);
            if (   t == BNTYPE_SQRT
                || t == BNTYPE_ROT
                || t == BNTYPE_FRAC
                || t == BNTYPE_SUB
                || t == BNTYPE_SUPER
                || t == BNTYPE_SUBSUPER
                || t == BNTYPE_UNDER
                || t == BNTYPE_OVER
                || t == BNTYPE_UNDEROVER)
            {
                cursor1.resize(i + 1);
                selection.type = SELTYPE_ROW;
                selection.data.resize(1);
                selection.data[0] = _pi() + 1;
                return;
            }

            if (t == BNTYPE_GRID)
            {
                int cursoridx = cursor1[i].idx;
                int selectionidx = selection.data[i];
                cursor1.resize(i + 1);
                _down2(cursoridx, 0);
                selection.type = SELTYPE_GRID;
                selection.data.resize(1);
                selection.data[0] = selectionidx;                
                return;
            }

            /* go up until we hit a row or grid or cell */
            while (i > 0 && t != BNTYPE_ROW
                         && t != BNTYPE_CELL
                         && t != BNTYPE_CELLGROUP)
            {
                i--;
                t = bnode_type(cursor1[i].node);
            }
            if (t == BNTYPE_ROW)
            {
                int idx = selection.data[i];
                cursor1.resize(i + 2);
                selection.type = SELTYPE_ROW;
                selection.data.resize(1);
                selection.data[0] = idx;
            }
            else
            {
                while (!we_at_cellbreak())
                {
                    _up1();
                }
                selection.type = SELTYPE_CELL;
                box u = bfrom_node(root);
                for (size_t j = 0; j < selection.data.size(); j++)
                {
                    box c = bnode_child(u, selection.data[j]);
                    if (   btype(c) == BNTYPE_CELL
                        || btype(c) == BNTYPE_NULLER)
                    {
                        selection.data.resize(j + 1);
                        return;
                    }
                    u = c;
                }
                assert(false);
            }
            return;
        }
    }
}

void notebook::key_shiftdown()
{
    assert(we_are_legal());
    cursor_needs_fitting = true;

    if (selection.type == 0)
    {
        if (we_at_cellbreak())
        {
            if (!we_last_cellbreak())
            {
                selection.type = SELTYPE_CELL;
                selection.data.resize(_depth());
                for (size_t i = 0; i < _depth(); i++)
                {
                    selection.data[i] = cursor1[i].idx;
                }
                goto_next_cellbreak();
            }
            return;
        }
        else
        {
            selection.data.clear();
            for (size_t i = 0; i < _depth(); i++)
            {
                selection.data.push_back(cursor1[i].idx);
            }
            move_down();
//printf("after move down:\n"); print();
            fixselection();
//printf("after fix selection:\n"); print();
            return;
        }
    }
    else if (selection.type == SELTYPE_ROW)
    {
        int idx = selection.data[0];
        selection.data.clear();
        for (size_t i = 0; i + 1 < _depth(); i++)
        {
            selection.data.push_back(cursor1[i].idx);
        }
        selection.data.push_back(idx);
        move_down();
        fixselection();
        return;
    }
    else if (selection.type == SELTYPE_GRID)
    {
        select_grid_dir(0, 1);
        return;
    }
    else if (selection.type == SELTYPE_CELL)
    {
        assert(we_at_cellbreak());
        goto_next_cellbreak();
        return;
    }
    else
    {
        assert(false);
    }

}


void notebook::key_shiftup()
{
    assert(we_are_legal());
    cursor_needs_fitting = true;

    if (selection.type == SELTYPE_NONE)
    {
        if (we_at_cellbreak())
        {
            if (!we_first_cellbreak())
            {
                selection.type = SELTYPE_CELL;
                selection.data.resize(_depth());
                for (size_t i = 0; i < _depth(); i++) {
                    selection.data[i] = cursor1[i].idx;
                }
                goto_prev_cellbreak();
            }
            return;
        }
        else
        {
            selection.data.clear();
            for (size_t i = 0; i < _depth(); i++)
            {
                selection.data.push_back(cursor1[i].idx);
            }
            move_up();
            fixselection();
            return;
        }
    }
    else if (selection.type == SELTYPE_ROW)
    {
        int idx = selection.data[0];
        selection.data.clear();
        for (size_t i = 0; i + 1 < _depth(); i++)
        {
            selection.data.push_back(cursor1[i].idx);
        }
        selection.data.push_back(idx);
        move_up();
        fixselection();
        return;
    }
    else if (selection.type == SELTYPE_GRID)
    {
        select_grid_dir(0, -1);
        return;

    } else if (selection.type == SELTYPE_CELL) {
        assert(we_at_cellbreak());
        goto_prev_cellbreak();
        return;
    }
    else
    {
        assert(false);
    }
}



void notebook::key_shiftleft()
{
    cursor_needs_fitting = true;
    boxbase* b = nullptr;
    myroot->move(b, movearg_ShiftLeft);
    assert(b == nullptr);
    return;
}

void notebook::key_shiftright()
{
    cursor_needs_fitting = true;
    boxbase* b = nullptr;
    myroot->move(b, movearg_ShiftRight);
    assert(b == nullptr);
    return;
}

void notebook::key_altup()
{
    cursor_needs_fitting = true;
    myroot->insert_char(-11);
    return;
}

void notebook::key_altdown()
{
    cursor_needs_fitting = true;
    myroot->insert_char(-12);
    return;
}

void notebook::key_altleft()
{
}

void notebook::key_altright()
{
}


void notebook::key_ctrlup()
{
}
void notebook::key_ctrldown()
{
}
void notebook::key_ctrlleft()
{
}
void notebook::key_ctrlright()
{
}


void notebook::key_left()
{
    cursor_needs_fitting = true;
    boxbase* b = nullptr;
    myroot->move(b, movearg_Left);
    assert(b == nullptr);
    return;
}






void notebook::key_home()
{
    assert(we_are_legal());
    cursor_needs_fitting = true;
    if (we_in_stub())
    {
        stub_key_home();
        return;
    }
    if (_depth() > 0 && bnode_type(_parent()) == BNTYPE_ROW)
    {
        selection.type = SELTYPE_NONE;
        if (_pi() > 0)
        {
            _up1();
            _down1(0);
            return;
        }
        else
        {
            if (bnode_type(_ggparent()) == BNTYPE_ROW)
            {
                _up2();
                _up1();
                _down1(0);
                return;
            }
            else
            {
                return;
            }
        }
    }
}




void notebook::key_right()
{
    cursor_needs_fitting = true;
    boxbase* b = nullptr;
    myroot->move(b, movearg_Right);
    assert(b == nullptr);
    return;
}


void notebook::key_end()
{
    assert(we_are_legal());
    cursor_needs_fitting = true;
    if (we_in_stub())
    {
        stub_key_end();
        return;
    }
    if (_depth() > 0 && bnode_type(_parent()) == BNTYPE_ROW)
    {
        selection.type = SELTYPE_NONE;
        if (_pi() + 1 < bnode_len(_parent()))
        {
            _up1();
            _down1(-1);
            return;
        }
        else
        {
            if (bnode_type(_ggparent()) == BNTYPE_ROW)
            {
                _up2();
                _up1();
                _down1(-1);
                return;
            }
            else
            {
                return;
            }
        }
    }
}

void notebook::insert_char(char16_t c)
{
    cursor_needs_fitting = true;
    myroot->insert_char(c);
    return;
}

void notebook::key_ctrl_char(char16_t c)
{
    boxbase* b = nullptr;
    if (c == 'v')
    {
        key_paste();
    }
    else if (c == 'c')
    {
        key_copy();
    }
    else if (c == 'r')
    {
        myroot->insert(b, insertarg_Rotation);
    }
    else if (c == 't')
    {
        myroot->insert(b, insertarg_Text);
    }
    else if (c == 'g')
    {
        myroot->insert(b, insertarg_Graphics3D);
    }
    assert(b == nullptr);
}


void notebook::key_makecell(cellType c)
{
    cursor_needs_fitting = true;

    if (myroot->cursor_b > myroot->childcells.size())
    {
        myroot->childcells[myroot->cursor_a].cbox->celltype = c;
    }
    else
    {
        myroot->delete_selection();

        rowbox * row = new rowbox(c == cellt_TITLE         ? "Title" :
                                  c == cellt_SECTION       ? "Section" :
                                  c == cellt_SUBSECTION    ? "Subsection" :
                                  c == cellt_SUBSUBSECTION ? "Subsubsection" :
                                  c == cellt_BOLDTEXT      ? "BoldText" :
                                  c == cellt_TEXT          ? "Text" :
                                  c == cellt_MESSAGE       ? "Message" :
                                  c == cellt_PRINT         ? "Print" :
                                  c == cellt_OUTPUT        ? "Output" :
                                                             "Input");

        cellbox * newcell = new cellbox(row, c);

        assert(myroot->cursor_a <= myroot->childcells.size());
        myroot->childcells.insert(myroot->childcells.begin() + myroot->cursor_a, cellboxarrayelem());
        myroot->childcells[myroot->cursor_a].cbox = newcell;
        myroot->cursor_b = myroot->childcells.size() + 1;
        return;
    }
}


void notebook::insert_newline()
{
    cursor_needs_fitting = true;
    boxbase* b = nullptr;
    myroot->insert(b, insertarg_Newline);
    assert(b == nullptr);
    return;
}

void notebook::key_tab()
{
    cursor_needs_fitting = true;
    myroot->insert_char('\t');
    return;
}



void notebook::insert_sqrt()
{
    cursor_needs_fitting = true;
    boxbase* b = nullptr;
    myroot->insert(b, insertarg_Sqrt);
    assert(b == nullptr);
    return;
}



void notebook::insert_subscript()
{
    cursor_needs_fitting = true;
    boxbase* b = nullptr;
    myroot->insert(b, insertarg_Subscript);
    assert(b == nullptr);
    return;
}



void notebook::insert_superscript()
{
    cursor_needs_fitting = true;
    boxbase* b = nullptr;
    myroot->insert(b, insertarg_Superscript);
    assert(b == nullptr);
    return;
}


bool notebook::we_are_legal() {return !(btype(_us()) == BNTYPE_ROW || btype(_us()) == BNTYPE_CELLGROUP || btype(_us()) == BNTYPE_ROOT);}

void notebook::key_switch()
{
    cursor_needs_fitting = true;
    boxbase* b = nullptr;
    myroot->move(b, movearg_Switch);
    assert(b == nullptr);
    return;
}



void notebook::insert_fraction()
{
    cursor_needs_fitting = true;
    boxbase* b = nullptr;
    myroot->insert(b, insertarg_Fraction);
    assert(b == nullptr);
    return;
}

void notebook::insert_underscript()
{
    cursor_needs_fitting = true;
    boxbase* b = nullptr;
    myroot->insert(b, insertarg_Underscript);
    assert(b == nullptr);
    return;
}


void notebook::insert_overscript()
{
    cursor_needs_fitting = true;
    boxbase* b = nullptr;
    myroot->insert(b, insertarg_Overscript);
    assert(b == nullptr);
    return;
}



void notebook::insert_gridrow()
{
    assert(we_are_legal());
    cursor_needs_fitting = true;

    if (we_at_cellbreak())
    {
        box newcell = boxnode_make(BNTYPE_CELL, boxnode_make(BNTYPE_ROW, boxchar_create(CHAR_Placeholder), bfrom_ptr(&box_null)));
        bto_node(newcell)->extra0 = CELLTYPE_INPUT;
        cell_insert(newcell);
        _down2(0, 1);
        selection.type = SELTYPE_ROW;
        selection.data.resize(1);
        selection.data[0] = 0;
    }

    /* insert into ROW */
    assert(bnode_type(_parent()) == BNTYPE_ROW);

    /* if we are in a grid with (no selection or selected placeholder), add a row below */
    if ((selection.type == SELTYPE_NONE || placeholder_selected()) && has_ancester(BNTYPE_GRID))
    {
        while (bnode_type(_parent()) != BNTYPE_GRID)
        {
            _up1();
        }
        selection.type = SELTYPE_NONE;
        boxnode * parent = bto_node(_parent());
        int pi = _pi();
        int posx = pi % parent->extra0;
        int posy = pi / parent->extra0;
        _up1();
        _insert_gridrow(posy+1);
        _down2((posy+1)*parent->extra0 + 0, 1);
        select_placeholder_if_needed();
    }
    else
    {
        /* attempt to get a selection if none exists */
        select_prev_if_possible();

        if (selection.type == SELTYPE_NONE)
        {
            box newgrid = boxnode_make(BNTYPE_GRID, 
                                        boxnode_make(BNTYPE_ROW, boxchar_create(CHAR_Placeholder), bfrom_ptr(&box_null)),
                                        boxnode_make(BNTYPE_ROW, boxchar_create(CHAR_Placeholder), bfrom_ptr(&box_null)));
            bto_node(newgrid)->extra0 = 1;
            bto_node(newgrid)->extra1 = 2;
            _insert(newgrid);
            _down2(1, 1);
            select_placeholder_if_needed();
        }
        else if (selection.type == SELTYPE_ROW)
        {
            box newrow = _splitrange_row(selection.data[0]);
            box newgrid = boxnode_make(BNTYPE_GRID, newrow, boxnode_make(BNTYPE_ROW, boxchar_create(CHAR_Placeholder), bfrom_ptr(&box_null)));
            bto_node(newgrid)->extra0 = 1;
            bto_node(newgrid)->extra1 = 2;
            _insert(newgrid);
            _down2(1, 1);
            select_placeholder_if_needed();
        }
        else
        {
            /* do nothing */
        }
        selection.type = SELTYPE_NONE;
    }
    return;
}




void notebook::insert_gridcol()
{
    assert(we_are_legal());
    cursor_needs_fitting = true;

    if (we_at_cellbreak())
    {
        box newcell = boxnode_make(BNTYPE_CELL, boxnode_make(BNTYPE_ROW, boxchar_create(CHAR_Placeholder), bfrom_ptr(&box_null)));
        bto_node(newcell)->extra0 = CELLTYPE_INPUT;
        cell_insert(newcell);
        _down2(0, 1);
        selection.type = SELTYPE_ROW;
        selection.data.resize(1);
        selection.data[0] = 0;
    }

    /* insert into ROW */
    assert(bnode_type(_parent()) == BNTYPE_ROW);

    /* if we are in a grid with (no selection or selected placeholder), add a row below */
    if ((selection.type == SELTYPE_NONE || placeholder_selected()) && has_ancester(BNTYPE_GRID))
    {
        while (bnode_type(_parent()) != BNTYPE_GRID)
        {
            _up1();
        }
        selection.type = SELTYPE_NONE;
        boxnode * parent = bto_node(_parent());
        int pi = _pi();
        int posx = pi % parent->extra0;
        int posy = pi / parent->extra0;
        _up1();
        _insert_gridcol(posx + 1);
        _down2((0)*parent->extra0 + posx + 1, 1);
        select_placeholder_if_needed();
    }
    else
    {
        /* attempt to get a selection if none exists */
        select_prev_if_possible();
        if (selection.type == SELTYPE_NONE)
        {
            box newgrid = boxnode_make(BNTYPE_GRID,
                                    boxnode_make(BNTYPE_ROW, boxchar_create(CHAR_Placeholder), bfrom_ptr(&box_null)),
                                    boxnode_make(BNTYPE_ROW, boxchar_create(CHAR_Placeholder), bfrom_ptr(&box_null)));
            bto_node(newgrid)->extra0 = 2;
            bto_node(newgrid)->extra1 = 1;
            _insert(newgrid);
            _down2(1, 1);
            select_placeholder_if_needed();
        }
        else if (selection.type == SELTYPE_ROW)
        {
            selection.type = SELTYPE_NONE;
            box newrow = _splitrange_row(selection.data[0]);
            box newgrid = boxnode_make(BNTYPE_GRID, newrow, boxnode_make(BNTYPE_ROW, boxchar_create(CHAR_Placeholder), bfrom_ptr(&box_null)));
            bto_node(newgrid)->extra0 = 2;
            bto_node(newgrid)->extra1 = 1;
            _insert(newgrid);
            _down2(1, 1);
            select_placeholder_if_needed();
        }
        else
        {
            selection.type = SELTYPE_NONE;
        }
    }
    return;
}

/* remove all children from us after and including s */
void notebook::_cell_remove_sels(boxnode * r, std::vector<int>&s, size_t i)
{
    _invalidate_downto(0);
    if (i + 1 >= s.size())
    {
        assert(s[i] > 0);
        boxnode_splice(r, boxnode_len(r), bto_node(boxnode_removerange(_us(), s[i], bnode_len(_us()))));
    }
    else
    {
        _down1(s[i]);
        _cell_remove_sels(r, s, i+1);
        _up1();
        boxnode_splice(r, boxnode_len(r), bto_node(boxnode_removerange(_us(), s[i]+1, bnode_len(_us()))));
    }
    assert(bnode_type(_us()) == BNTYPE_CELLGROUP);
    if (bnode_len(_us()) == 1)
    {
        _down1(0);
        _deleteup1();
    }
}

/*
    put all children strictly before t into "before"
    put all children after and including t into "after"
*/
void notebook::_cell_remove_selt(boxnode * before, boxnode * after, std::vector<int>&t, size_t i)
{
    _invalidate_downto(0);
    box r, f;
    if (i + 1 >= t.size())
    {
        boxnode_splice(before, boxnode_len(before), bto_node(boxnode_removerange(_us(), 0, t[i])));
        boxnode_splice(after, boxnode_len(after), bto_node(boxnode_removerange(_us(), 0, bnode_len(_us()))));
        return;
    }
    else
    {
        boxnode_splice(before, boxnode_len(before), bto_node(boxnode_removerange(_us(), 0, t[i])));
        t[i] = 0;
        _down1(t[i]);
        _cell_remove_selt(before, after, t, i+1);
        _up1();
        box_node_delete(_remove(0));
        boxnode_splice(after, boxnode_len(after), bto_node(boxnode_removerange(_us(), 0, bnode_len(_us()))));
        return;
    }
}

box notebook::_cell_remove_selection(std::vector<int>&s, std::vector<int>&t, size_t i)
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

        boxnode* r = boxnode_create(BNTYPE_ROOT, 2);
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
            boxnode_append(r, boxnode_remove(_us(), s[i]));
            if (bnode_type(_us()) == BNTYPE_CELLGROUP && bnode_len(_us()) == 1)
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

//printf("0 notebook:\n"); print();


//printf("\n\n\nremove s:\n"); boxnode_print(NULL, r, 0); printf("\n");


//printf("s[i]: %d  t[i]: %d\n",s[i],t[i]);

        if (s[i] <= t[i])
        {
//printf("_cell_remove_selection 6\n");

            boxnode_splice(r, boxnode_len(r), bto_node(boxnode_removerange(_us(), s[i], t[i])));
//printf("remove middle:\n"); boxnode_print(NULL, r, 0); printf("\n");
            t[i] = s[i];
        }

//printf("s[i]: %d  t[i]: %d\n",s[i],t[i]);

        assert(t[i] == s[i]);

//printf("1 notebook:\n"); print();

        assert(t[i] < bnode_len(_us()));

//printf("_cell_remove_selection 7\n");
        if (bnode_type(_us()) == BNTYPE_CELLGROUP && bnode_len(_us()) == 1)
        {
            assert(t[i] == 0);
            t[i] = _pi();
            _down1(0);
            _deleteup1();
            _up1();           
        }

//printf("_cell_remove_selection 8\n");

        boxnode* v = boxnode_create(BNTYPE_ROOT, 2);
        if (bnode_type(_us()) == BNTYPE_ROOT && t[i] == bnode_len(_us()) - 1
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
//printf("remove r:\n"); boxnode_print(NULL, r, 0); printf("\n");
//printf("v:\n"); boxnode_print(NULL, v, 0); printf("\n");
//printf("notebook:\n"); print();

            cell_delete();
        }
//printf("2 notebook:\n"); print();
//printf("_cell_remove_selection 11\n");
        while (bptr_type(_us()) == BNTYPE_CELLGROUP)
        {
            _down1(0);
        }
//printf("_cell_remove_selection 12\n");

        while (boxnode_len(v) > 0)
        {
            cell_insert(boxnode_remove(v, boxnode_len(v) - 1));
        }
//printf("_cell_remove_selection 13\n");

        box_node_delete(v);
        return bfrom_node(r);
    }
}

void notebook::delete_selection()
{
    assert(selection.type != SELTYPE_NONE);

    if (selection.type == SELTYPE_ROW)
    {
        int min = selection.data[0];
        int max = _pi();
        if (min > max) {
            std::swap(min, max);
        }
        _up1();
        _deleterange(min, max);
        _down1(min);
        selection.type = SELTYPE_NONE;
    }
    else if (selection.type == SELTYPE_GRID)
    {
        assert(bnode_type(_gparent()) == BNTYPE_GRID);

        boxnode * grid = bto_node(_gparent());
        int gridi = _gpi();

        _up2();

        int min_x = selection.data[0] % grid->extra0;
        int min_y = selection.data[0] / grid->extra0;
        int max_x = gridi % grid->extra0;
        int max_y = gridi / grid->extra0;

        if (min_x > max_x) {std::swap(min_x, max_x);}
        if (min_y > max_y) {std::swap(min_y, max_y);}

        for (int j=min_y; j<=max_y; j++) {
            for (int i=min_x; i <= max_x; i++) {
                _replace(j*grid->extra0+i, boxnode_make(BNTYPE_ROW, boxchar_create(CHAR_Placeholder), bfrom_ptr(&box_null)));
            }
        }
        selection.type = SELTYPE_NONE;

        for (int i = max_x; i >= min_x; i--) {
            if (_grid_col_is_empty(i)) {
                _delete_col_from_grid(i);
            }
        }
        if (bnode_len(_us()) == 0) {
            _delete();
            return;
        }
        for (int i = max_y; i >= min_y; i--) {
            if (_grid_row_is_empty(i)) {
                _delete_row_from_grid(i);
            }
        }
        if (bnode_len(_us()) == 0) {
            _delete();
            return;
        }
        min_x = std::min(min_x, bnode_extra0(_us()) - 1);
        min_y = std::min(min_y, bnode_extra1(_us()) - 1);
        _down2(min_y*bnode_extra0(_us()) + min_x, 0);
        assert(bnode_type(_parent()) == BNTYPE_ROW);
        return;
    }
    else
    {
        assert(selection.type == SELTYPE_CELL);

        selection.type = SELTYPE_NONE;

        std::vector<int> sstack, tstack;

        tstack.resize(_depth());
        for (size_t i = 0; i < _depth(); i++)
        {
            tstack[i] = cursor1[i].idx;
//std::cout << "tstack[" << i <<"] = " << tstack[i] << std::endl;
        }

        sstack.resize(selection.data.size());
        for (size_t i = 0; i < selection.data.size(); i++)
        {
            sstack[i] = selection.data[i];
//std::cout << "sstack[" << i <<"] = " << sstack[i] << std::endl;
        }

        for (size_t i = 0; i < std::min(sstack.size(), tstack.size()); i++)
        {
            if (sstack[i] != tstack[i])
            {
                cursor1.clear();
                cursor1.push_back(cursorentry(root));

                if (sstack[i] < tstack[i])
                {
                    box_delete(_cell_remove_selection(sstack, tstack, 0));
                }
                else
                {
                    box_delete(_cell_remove_selection(tstack, sstack, 0));
                }
                break;
            }
        }
    }
    return;
}





void notebook::handle_empty_row(int dir)
{
    assert(bnode_type(_parent()) == BNTYPE_ROW);
//printf("handling empty row\n");

    /* if we leave an empty row, take care of it */
    if (bnode_len(_parent()) == 1)
    {
        if (   bnode_type(_gparent()) == BNTYPE_SUPER
            || bnode_type(_gparent()) == BNTYPE_SUB
            || bnode_type(_gparent()) == BNTYPE_SQRT)
        {
            _up2();
            _delete();
            insert_placholder_if_needed();
            return;
        }
        else if (bnode_type(_gparent()) == BNTYPE_FRAC)
        {
            if (_gpi() == 0)
            {
                _up2();
                box newrow = _remove(1);
                _delete();
                _splice_row(newrow);
            }
            else if (_gpi() == 1)
            {
                _up2();
                box newrow = _remove(0);
                _delete();
                _splice_row_before(newrow);
            }
            else
            {
                assert(false && "fraction box index out of range");
            }
            insert_placholder_if_needed();
            return;
        }
        else if (bnode_type(_gparent()) == BNTYPE_SUBSUPER)
        {
            if (_gpi() == 0)
            {
                _up2();
                box newrow = _remove(1);
                _delete();
                _insert(boxnode_make(BNTYPE_SUPER, newrow));
                _down2(0, 0);
            }
            else if (_gpi() == 1)
            {
                _up2();
                box newrow = _remove(0);
                _delete();
                _insert(boxnode_make(BNTYPE_SUB, newrow));
                _down2(0, -1);
            }
            else
            {
                assert(false && "subsuper box index out of range");
            }
            return;
        }
        else if (   bnode_type(_gparent()) == BNTYPE_UNDER
                 || bnode_type(_gparent()) == BNTYPE_OVER)
        {
            if (_gpi() == 0)
            {
                _up2();
                _delete();
                insert_placholder_if_needed();
                return;
            }
            else if (_gpi() == 1)
            {
                _up2();
                box newrow = _remove(0);
                _delete();
                _splice_row_before(newrow);
            }
            else
            {
                assert(false && "under/over box index out of range");
            }
            insert_placholder_if_needed();
            return;
        }
        else if (   bnode_type(_gparent()) == BNTYPE_UNDEROVER)
        {
            if (_gpi() == 0)
            {
                _up2();
                _delete();
                insert_placholder_if_needed();
                return;
            }
            else if (_gpi() == 1)
            {
                _up2();
                box newmain = _remove(0);
                box newover = _remove(2);
                _delete();
                _insert(boxnode_make(BNTYPE_OVER, newmain, newover));
                _down2(1, -1);
            }
            else if (_gpi() == 2)
            {
                _up2();
                box newmain = _remove(0);
                box newunder = _remove(1);
                _delete();
                _insert(boxnode_make(BNTYPE_UNDER, newmain, newunder));
                _down2(1, -1);
            }
            else
            {
                assert(false && "underover box index out of range");
            }
            insert_placholder_if_needed();
            return;            
        }
        else if (bnode_type(_gparent()) == BNTYPE_GRID)
        {
            /* if all entries in row are empty or only have place holders, delete row */
            /* if all entires in col are empty or only have place holders, delete row */
            /* else back up */

            boxnode * gparent = bto_node(_gparent());
            int gpi     = _gpi();
            int posx = gpi % gparent->extra0;
            int posy = gpi / gparent->extra0;

            bool rows_empty = true;
            for (int i = 0; i < gparent->extra0; i++) {
                boxnode * entry = bto_node(gparent->array[posy*gparent->extra0+i].child);
                assert(entry->header.type == BNTYPE_ROW);
                if (!(entry->len == 1 || (entry->len == 2 && bis_char(entry->array[0].child, CHAR_Placeholder)))) {
                    rows_empty = false;
                    break;
                }
            }

            bool cols_empty = true;
            for (int i = 0; i < gparent->extra1; i++) {
                boxnode * entry = bto_node(gparent->array[i*gparent->extra0+posx].child);
                assert(entry->header.type == BNTYPE_ROW);
                if (!(entry->len == 1 || (entry->len == 2 && bis_char(entry->array[0].child, CHAR_Placeholder)))) {
                    cols_empty = false;
                    break;
                }
            }

            if (rows_empty && (   (posx == 0 && dir < 0)
                               || (posx == gparent->extra0 - 1 && dir >= 0)))
            {
                if (dir < 0) {
                    _up2();
                    _delete_row_from_grid(posy);
                    if (bnode_len(_us()) == 0) {
                        _delete();
                        insert_placholder_if_needed();
                    } else if (posy > 0) {
                        _down2( (posy - 1)*bnode_extra0(_us()) + (bnode_extra0(_us()) - 1), -1);
                        //assert(bnode_type(_us()) == BNTYPE_ROW);
                        select_placeholder_if_needed();
                    }
                } else if (dir >= 0) {
                    _up2();
                    _delete_row_from_grid(posy);
                    if (bnode_len(_us()) == 0) {
                        _delete();
                        insert_placholder_if_needed();
                    } else if (posy < gparent->extra1) {
                        _down2( (posy)*bnode_extra0(_us()) + 0, 0);
                        //assert(bnode_type(_us()) == BNTYPE_ROW);
                        select_placeholder_if_needed();
                    }
                }
            }
            else if (posy == 0 && cols_empty)
            {
                _up2();
                _delete_col_from_grid(posx);
                if (bnode_len(_us()) == 0) {
                    _delete();
                    insert_placholder_if_needed();
                } else if (posx > 0) {
                    _down2( (0)*bnode_extra0(_us()) + (posx - 1), -1);
                    //assert(bnode_type(_us()) == BNTYPE_ROW);
                    select_placeholder_if_needed();
                }
            }
            else
            {
                _insert(boxchar_create(CHAR_Placeholder));
                if (dir < 0) {
                    key_left();
                } else {
                    _right(1);
                    key_right();
                }
            }
            return;

        }
        else if (bnode_type(_gparent()) == BNTYPE_CELL)
        {
            /* do nothing */
        }
        else
        {
            assert(false);
        }

    } else {
        /* do nothing */
    }
}


void notebook::key_backspace()
{
    cursor_needs_fitting = true;
    boxbase * b = nullptr;
    myroot->remove(b, removearg_Left);
    assert(b == nullptr);
    return;
}


void notebook::key_delete()
{
    cursor_needs_fitting = true;
    boxbase* b = nullptr;
    myroot->remove(b, removearg_Right);
    assert(b == nullptr);
    return;
}




/* copy all children after and including s */
void _cell_copy_sels(box buf, box us, std::vector<int>&s, size_t i)
{
    if (i + 1 >= s.size())
    {
        assert(s[i] > 0);
        for (int j = s[i]; j < bnode_len(us); j++)
        {
            boxnode_append(buf, boxnode_copy(bnode_child(us, j)));
        }
    }
    else
    {
        _cell_copy_sels(buf, bnode_child(us, s[i]), s, i+1);
        for (int j = s[i]+1; j < bnode_len(us); j++)
        {
            boxnode_append(buf, boxnode_copy(bnode_child(us, j)));
        }
    }
}

/* copy all children strictly before t */
void _cell_copy_selt(box buf, box us, std::vector<int>&t, size_t i)
{
    if (i + 1 >= t.size())
    {
        for (int j = 0; j < t[i]; j++)
        {
            boxnode_append(buf, boxnode_copy(bnode_child(us, j)));
        }
        return;
    }
    else
    {
        for (int j = 0; j < t[i]; j++)
        {
            boxnode_append(buf, boxnode_copy(bnode_child(us, j)));
        }
        _cell_copy_selt(buf, bnode_child(us, t[i]), t, i+1);
        return;
    }
}

void _cell_copy_selection(box buf, box us, std::vector<int>&s, std::vector<int>&t, size_t i) {

    assert(i < s.size());
    assert(i < t.size());

    if (s[i] == t[i])
    {
        return _cell_copy_selection(buf, bnode_child(us, s[i]), s, t, i+1);
    }
    else
    {
        while (s.size() > i+1 && s.back() == 0) {s.pop_back();}
        while (t.size() > i+1 && t.back() == 0) {t.pop_back();}
        if (s.size() > i+1)
        {
            _cell_copy_sels(buf, bnode_child(us, s[i]), s, i+1);
        }
        else
        {
            boxnode_append(buf, boxnode_copy(bnode_child(us, s[i])));
        }
        for (int j = s[i]+1; j < t[i]; j++)
        {
            boxnode_append(buf, boxnode_copy(bnode_child(us, j)));
        }
        if (t.size() > i+1)
        {
            _cell_copy_selt(buf, bnode_child(us, t[i]), t, i+1);
        }
    }
}


void notebook::key_copy() {

    if (selection.type == SELTYPE_NONE)
    {
        return;
    }
    else if (selection.type == SELTYPE_ROW)
    {
        int previ = selection.data[0];
        int pi = _pi();
        int min = previ;
        int max = pi;
        if (min > max) {
            min = pi;
            max = previ;
        }
        if (min < max) {
            if (copy_buffer.ptr != nullptr) {box_delete(copy_buffer);}
            copy_buffer = boxnode_copyrange(_parent(), min, max);
        }
        return;
    }
    else if (selection.type == SELTYPE_GRID)
    {
        assert(false && "copy grid unimplemented");
        return;

    }
    else if (selection.type == SELTYPE_CELL)
    {
        std::vector<int> sstack, tstack;

        tstack.resize(_depth());
        for (size_t i = 0; i < _depth(); i++)
        {
            tstack[i] = cursor1[i].idx;
        }

        sstack.resize(selection.data.size());
        for (size_t i = 0; i < selection.data.size(); i++)
        {
            sstack[i] = selection.data[i];
        }

        box copybuffer = bfrom_node(boxnode_create(BNTYPE_ROOT, 3));

        for (size_t i = 0; i < std::min(sstack.size(), tstack.size()); i++)
        {
            if (sstack[i] != tstack[i])
            {
                if (sstack[i] < tstack[i])
                {
                    _cell_copy_selection(copybuffer, bfrom_node(root), sstack, tstack, 0);
                }
                else
                {
                    _cell_copy_selection(copybuffer, bfrom_node(root), tstack, sstack, 0);
                }
                break;
            }
        }

//printf("copybuffer:\n"); boxnode_print(NULL,copybuffer,0);

        if (bnode_len(copybuffer) > 0)
        {
            if (copy_buffer.ptr != nullptr) {box_delete(copy_buffer);}
            copy_buffer = copybuffer;
        }
        else
        {
            box_delete(copybuffer);
        }
    }
}



void notebook::key_paste()
{
    cursor_needs_fitting = true;
    boxbase* b = nullptr;

    std::string s;
    clipboard_get_data_append(s);
    for (size_t j = 0; j < s.length(); j++)
    {
        if (s[j] >= ' ')
        {
            myroot->insert_char(s[j]);
        }
        else if (s[j] == '\n')
        {
            myroot->insert(b, insertarg_Newline);
        }
    }

    assert(b == nullptr);

    return;



    assert(we_are_legal());
/*
    if (_us()->type >= BNTYPE_SPECIAL && _us()->type != BNTYPE_TEXT) {
        return;
    }
*/

/*
    if (selection.type != SELTYPE_NONE)
    {
        delete_selection();
    }

    if (copy_buffer.ptr == nullptr)
    {
        std::string s;
        clipboard_get_data_append(s);
        for (size_t j = 0; j < s.length(); j++) {
            if (s[j] >= ' ') {
                insert_char(s[j]);
            } else if (s[j] == '\n') {
                insert_newline();
            } else if (s[j] == '\t') {
                key_tab();
            }
        }
        return;
    }
    else if (bnode_type(copy_buffer) == BNTYPE_ROW)
    {
        if (we_at_cellbreak())
        {
            box newcell = boxnode_make(BNTYPE_CELL, boxnode_make(BNTYPE_ROW, bfrom_ptr(&box_null)));
            cell_insert(newcell);
            _down2(0, 0);
            _splice(boxnode_copy(copy_buffer));
            _right(bnode_len(copy_buffer));
            return;
        }
        else if (bnode_type(_parent()) == BNTYPE_ROW)
        {
            _splice(boxnode_copy(copy_buffer));
            _right(bnode_len(copy_buffer));
            return;
        }
        else
        {
            assert(false && "paste row unimplemented");
            return;
        }
    }
    else if (bnode_type(copy_buffer) == BNTYPE_GRID)
    {
        assert(false && "paste grid unimplemented");
        return;
    }
    else if (bnode_type(copy_buffer) == BNTYPE_ROOT)
    {
        if (we_at_cellbreak())
        {
            for (int j = bnode_len(copy_buffer) - 1; j >= 0; j--)
            {
                cell_insert(boxnode_copy(bnode_child(copy_buffer,j)));
            }
        }
    }
*/
}

void notebook::goto_our_cellbreak() {

    assert(we_are_legal());
    assert(!we_at_cellbreak());

    while (btype(_us()) != BNTYPE_CELL) {
        _up1();
    }    
    assert(we_are_legal());
}


void notebook::cell_delete()
{
    int pi = _pi();
    bool gotonext;

    _up1();
    assert(bnode_len(_us())>1);
    box_delete(_remove(pi));
    if (pi < bnode_len(_us())) {
        gotonext = false;
        _down1(pi);
    } else {
        gotonext = true;
        _down1(-1);
    }
    while (   bnode_type(_parent()) == BNTYPE_CELLGROUP
            && bnode_len(_parent()) == 1) {
        _deleteup1();
    }

    if (gotonext) {
        goto_next_cellbreak();
    }
}


box notebook::cell_remove()
{
    int pi = _pi();
    if (bnode_type(_parent()) != BNTYPE_CELLGROUP || pi > 0)
    {
        bool gotonext;
        _up1();
        assert(bnode_len(_us())>1);
        box us = _remove(pi);
        if (pi < bnode_len(_us()))
        {
            gotonext = false;
            _down1(pi);
        }
        else
        {
            gotonext = true;
            _down1(-1);
        }
        if (   bnode_type(_parent()) == BNTYPE_CELLGROUP
                && bnode_len(_parent()) == 1)
        {
            _deleteup1();
        }
        if (gotonext)
        {
            while (bnode_type(_us()) == BNTYPE_CELLGROUP)
            {
                _down1(-1);
            }
            goto_next_cellbreak();
        }
        else
        {
            while (bptr_type(_us()) == BNTYPE_CELLGROUP)
            {
                _down1(0);
            }
        }
        return us;
    }
    else
    {
        int gpi = _gpi();
        _up2();
        box r = _remove(gpi);
        box us = boxnode_remove(r, 0);
        int i = bnode_len(r) - 1;
        boxnode_insert(_us(), boxnode_remove(r,i), gpi);
        _down1(gpi);
        _fix_cell_insert();
        i--;
        while (i >= 0)
        {
            _insert(boxnode_remove(r,i));
            _fix_cell_insert();
            i--;
        }
        assert(bnode_len(r) == 0);
        box_delete(r);
        return us;
    }
}


bool ok_parent_child(box p, box c)
{

//printf("ok_parent_child %p %p\n",p,c);

    if (p.ptr == &box_null || c.ptr == &box_null)
        return false;

    while (bnode_type(p) == BNTYPE_CELLGROUP) {p = bnode_child(p,0);}
    while (bnode_type(c) == BNTYPE_CELLGROUP) {c = bnode_child(c,0);}

assert(bnode_type(p) == BNTYPE_CELL);
assert(bnode_type(c) == BNTYPE_CELL);

    int pstyle = bnode_extra0(p);
    int cstyle = bnode_extra0(c);

//printf(" pstyle %d  cstyle %d\n", pstyle, cstyle);

    switch (pstyle) {
        case CELLTYPE_INPUT:
            return cstyle == CELLTYPE_OUTPUT || cstyle == CELLTYPE_MESSAGE || cstyle == CELLTYPE_PRINT;
        case CELLTYPE_MESSAGE:
        case CELLTYPE_PRINT:
            return cstyle == pstyle;
        case CELLTYPE_OUTPUT:
        case CELLTYPE_BOLDTEXT:
        case CELLTYPE_TEXT:
            return false;
        case CELLTYPE_SUBSUBSECTION:
        case CELLTYPE_SUBSECTION:
        case CELLTYPE_SECTION:
        case CELLTYPE_TITLE:
            return cstyle < pstyle;
        default:
            assert(false);
            return false;
    }
}

bool ok_parent_child_splice(box p, box c)
{

//printf("ok_parent_child %p %p\n",p,c);

    if (p.ptr == &box_null || c.ptr == &box_null)
        return false;

    while (bnode_type(p) == BNTYPE_CELLGROUP) {p = bnode_child(p,0);}
    while (bnode_type(c) == BNTYPE_CELLGROUP) {c = bnode_child(c,0);}

assert(bnode_type(p) == BNTYPE_CELL);
assert(bnode_type(c) == BNTYPE_CELL);

    int pstyle = bnode_extra0(p);
    int cstyle = bnode_extra0(c);

//printf(" pstyle %d  cstyle %d\n", pstyle, cstyle);

    switch (pstyle) {
        case CELLTYPE_INPUT:
            return false;
        case CELLTYPE_MESSAGE:
        case CELLTYPE_PRINT:
            return cstyle == pstyle;
        case CELLTYPE_OUTPUT:
        case CELLTYPE_BOLDTEXT:
        case CELLTYPE_TEXT:
            return false;
        case CELLTYPE_SUBSUBSECTION:
        case CELLTYPE_SUBSECTION:
        case CELLTYPE_SECTION:
        case CELLTYPE_TITLE:
            return false;
        default:
            assert(false);
            return false;
    }
}

bool ok_parent_child_master(box p, box c, box m) {

//printf("ok_parent_child %p %p\n",p,c);

    if (p.ptr == &box_null || c.ptr == &box_null)
        return false;

    if (bnode_type(m) == BNTYPE_ROOT) {
        return ok_parent_child(p,c);
    }

    while (bnode_type(p) == BNTYPE_CELLGROUP) {p = bnode_child(p,0);}
    while (bnode_type(c) == BNTYPE_CELLGROUP) {c = bnode_child(c,0);}
    while (bnode_type(m) == BNTYPE_CELLGROUP) {m = bnode_child(m,0);}

assert(bnode_type(p) == BNTYPE_CELL);
assert(bnode_type(c) == BNTYPE_CELL);

    int pstyle = bnode_extra0(p);
    int cstyle = bnode_extra0(c);
    int mstyle = bnode_extra0(m);

//printf(" pstyle %d  cstyle %d  mstyle %d\n", pstyle, cstyle, mstyle);

    switch (pstyle) {
        case CELLTYPE_INPUT:
            return cstyle == CELLTYPE_OUTPUT || cstyle == CELLTYPE_MESSAGE || cstyle == CELLTYPE_PRINT;
        case CELLTYPE_MESSAGE:
        case CELLTYPE_PRINT:
            return cstyle == pstyle && cstyle != mstyle;
        case CELLTYPE_OUTPUT:
        case CELLTYPE_BOLDTEXT:
        case CELLTYPE_TEXT:
            return false;
        case CELLTYPE_SUBSUBSECTION:
        case CELLTYPE_SUBSECTION:
        case CELLTYPE_SECTION:
        case CELLTYPE_TITLE:
            return cstyle < pstyle;
        default:
            assert(false);
            return false;
    }
}

void notebook::cell_insert(box c)
{
    assert(we_at_cellbreak());
    while (_pi() == 0 && bnode_type(_parent()) == BNTYPE_CELLGROUP)
    {
        _up1();
    }
    _insert(c);
    _fix_cell_insert();
}

void notebook::_fix_cell_insert()
{
    assert(we_at_cellbreak());

//printf("\n\n");

try_again:

//printf("**** before correction\n");print();SleepUS(10000);


    int i, pi = _pi();

    if (pi == 0 && bnode_type(_parent()) == BNTYPE_CELLGROUP)
    {
        _up1();
        goto try_again;
    }

    /* if there are siblings to the right that would rather be our children, make them our children */
    if (pi > 0 || bnode_type(_parent()) == BNTYPE_ROOT)
    {
        int len = bnode_len(_parent());
        if (bnode_type(_parent()) == BNTYPE_ROOT)
            len--;

        if (pi + 1 < len && ok_parent_child_master(_us(), bnode_child(_parent(), pi + 1), _parent()))
        {
            _up1();
            box sib = _remove(pi + 1);
            _down1(pi);
            if (bnode_type(_us()) != BNTYPE_CELLGROUP)
            {
                box g = boxnode_make(BNTYPE_CELLGROUP, bfrom_ptr(nullptr));
                bto_node(g)->extra1 |= BNFLAG_OPEN;
                _insertup1(g,0);
                _up1();
            }
            if (bnode_type(sib) == BNTYPE_CELLGROUP && ok_parent_child_splice(_us(), sib))
            {
                boxnode_splice(_us(), bnode_len(_us()), sib);                
            }
            else
            {
                boxnode_append(_us(), sib);
            }
//printf("did 1!!!!!!!!!!!!!!!!!!!\n");
            goto try_again;
        }
    }

    /* if we cannot be a child of our parent, move us up */
    if (bnode_type(_parent()) == BNTYPE_CELLGROUP)
    {
        if (pi > 0 && !ok_parent_child(_parent(), _us()))
        {
            _up1();
            box r = _removerange(pi, bnode_len(_us()));
            if (pi == 1)
            {
                _down1(0);
                _deleteup1();
            }
            pi = _pi();
            _up1();
            boxnode_splice(_us(), pi + 1, r);
            _down1(pi + 1);
//printf("did 2!!!!!!!!!!!!!!!!!!\n");
            goto try_again;
        }        
    }

    /* if the sibling to the left accepts us as a child over our current parent, make us their child */
    if ( bnode_type(_parent()) == BNTYPE_ROOT ? pi > 0 : pi > 1)
    {
        if (ok_parent_child_master(_prev(), _us(), _parent()))
        {
            _up1();
            box u = _remove(pi);
            _down1(pi - 1);
            if (bnode_type(_us()) != BNTYPE_CELLGROUP)
            {
                box g = boxnode_make(BNTYPE_CELLGROUP, bfrom_ptr(nullptr));
                bto_node(g)->extra1 |= BNFLAG_OPEN;
                _insertup1(g,0);
                _up1();
            }
            boxnode_append(_us(), u);
            _down1(-1);

//printf("did 3!!!!!!!!!!!!!!!!!!!!!!!!\n");
            goto try_again;
        }
    }

//printf("**** after correction\n");print();

    /* good to go */
    while (bnode_type(_us()) == BNTYPE_CELLGROUP)
    {
        _down1(0);
    }
    _invalidate_downto(0);
}


//extern HANDLE hPipeG2K;

void notebook::key_shiftenter()
{
    cursor_needs_fitting = true;
    myroot->key_shiftenter(this);
    return;
}



boxnode * _find_cell_mark(boxnode * b, std::vector<int32_t>& v)
{
    int32_t len = boxnode_len(b);
    if (boxnode_type(b) == BNTYPE_ROOT)
    {
        len--;
    }
//std::cout << "len : " << len << std::endl;

    for (int32_t i = 0; i < len; i++)
    {
        boxnode * c = bto_node(boxnode_child(b,i));
        v.push_back(i);
        if (boxnode_type(c) == BNTYPE_CELL)
        {
            if (boxnode_extra1(c) & BNFLAG_MARKED)
            {
                return c;
            }
        }
        else
        {
            assert(boxnode_type(c) == BNTYPE_CELLGROUP);
            boxnode * r = _find_cell_mark(c, v);
            if (r != nullptr)
            {
                return r;
            }
        }
        v.pop_back();
    }
    return nullptr;
}

void notebook::erase_cell_mark()
{
    std::vector<int32_t> v;
    boxnode * r = _find_cell_mark(root, v);
    if (r != nullptr)
    {
        r->extra1 &= ~BNFLAG_MARKED;
    }
}


void notebook::print_cell(cellbox* c)
{
    print_location = std::min(print_location, int32_t(myroot->childcells.size()));

    if (myroot->cursor_b > myroot->childcells.size())
    {
        myroot->childcells.insert(myroot->childcells.begin() + print_location, cellboxarrayelem());
        myroot->childcells[print_location].cbox = c;
        myroot->cursor_b = myroot->childcells.size() + 1;
    }
    else
    {
        myroot->childcells.insert(myroot->childcells.begin() + print_location, cellboxarrayelem());
        myroot->childcells[print_location].cbox = c;
        myroot->cursor_b += print_location <= myroot->cursor_b;
    }
    myroot->cursor_a += print_location <= myroot->cursor_a;
    print_location++;
    return;

}

void notebook::print_cell(box c)
{
    assert(bnode_type(c) == BNTYPE_CELL);
    bto_node(c)->extra1 &= ~BNFLAG_MARKED;

//std::cout << "********print_cell called******" << std::endl;

    std::vector<int32_t> marked_cell_position;
    boxnode * r = _find_cell_mark(root, marked_cell_position);

    if (r == nullptr)
    {
//std::cout << "case 0" << std::endl;
        boxnode_insert(root, c, boxnode_len(root) - 1);
//std::cout << "********print_cell returning******" << std::endl;
        return;
    }
    else
    {
        r->extra1 &= ~BNFLAG_MARKED;
        if (bis_node(_us()) && bto_node(_us()) == r)
        {
//std::cout << "case 1" << std::endl;
            if (!we_at_cellbreak())
            {
                goto_our_cellbreak();
            }
            goto_next_cellbreak();
            cell_insert(c);
            _invalidate_downto(0);
            goto_next_cellbreak();
            bto_node(c)->extra1 |= BNFLAG_MARKED;
//std::cout << "********print_cell returning******" << std::endl;
            return;
        }
        else if (we_last_cellbreak())
        {
//std::cout << "case 2" << std::endl;
            /* go down to originally marked cell and put c after */
            cursor1.resize(1);
            for (size_t i = 0; i < marked_cell_position.size(); i++)
            {
                _down1(marked_cell_position[i]);
            }
            goto_next_cellbreak();
            cell_insert(c);
            _invalidate_downto(0);
            goto_last_cellbreak();
            bto_node(c)->extra1 |= BNFLAG_MARKED;
//std::cout << "********print_cell returning******" << std::endl;
            return;
        }
        else
        {
//std::cout << "case 3" << std::endl;
            /* get our position in current cell */
            std::stack<int32_t> position_in_cur_cell;
//std::cout << "depth: " << _depth() << std::endl;
            for (size_t i = 1; i < _depth(); i++)
            {
//std::cout << "i = " << i << std::endl;
                if (bnode_type(cursor1[i].node) == BNTYPE_CELL)
                {
                    std::stack<int32_t> s;
                    while (_depth() > i)
                    {
//std::cout << "pushed " << _pi() << std::endl;

                        position_in_cur_cell.push(_pi());
                        _up1();
                    }
                    break;
                }
            }
            /* mark cursor cell */
            assert(bnode_type(_us()) == BNTYPE_CELL);
            bto_node(_us())->extra1 |= BNFLAG_MARKED;

//std::cout << "case 3 here" << std::endl;
            /* go down to originally marked cell and put c after */
            cursor1.resize(1);
            for (size_t i = 0; i < marked_cell_position.size(); i++)
            {
//std::cout << "going down " << marked_cell_position[i] << std::endl;
                _down1(marked_cell_position[i]);
            }
            goto_next_cellbreak();
            cell_insert(c);
            _invalidate_downto(0);

            /* go down to original cursor cell */
            marked_cell_position.clear();
            r = _find_cell_mark(root, marked_cell_position);
            assert(r != nullptr); // r should be the cell the cursor was in
            r->extra1 &= ~BNFLAG_MARKED;

            cursor1.resize(1);
            for (size_t i = 0; i < marked_cell_position.size(); i++)
            {
                _down1(marked_cell_position[i]);
            }
            while (!position_in_cur_cell.empty())
            {
                _down1(position_in_cur_cell.top());
                position_in_cur_cell.pop();
            }

            bto_node(c)->extra1 |= BNFLAG_MARKED;
//std::cout << "********print_cell returning******" << std::endl;
            return;
        }
    }
}
