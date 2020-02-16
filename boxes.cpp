#include "boxes.h"
#include "ex_parse.h"
#include <stack>
#include "serialize.h"
#include "notebook.h"
#include "box_convert.h"


const char* cell_type_names[cellt_max] = {
    "Input",
    "Output",
    "Message",
    "Print",
    "Text",
    "BoldText",
    "Subsubsection",
    "Subsection",
    "Section",
    "Title",
};


std::string stdvector_tostring(std::vector<int32_t> v)
{
    std::string s = "<";
    for (size_t i = 0; i < v.size(); i++)
    {
        s.append(stdstring_to_string(v[i]));
        if (i + 1 < v.size())
            s.push_back(',');
    }
    s.push_back('>');
    return s;
}


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

rowbox * steal_rowbox(rowbox * row, int32_t a, int32_t b)
{
    size_t n = row->child.size();
    rowbox * newrow = new rowbox(n, a, b);
    for (int32_t i = 0; i < n; i++)
    {
        newrow->child[i].cibox = row->child[i].cibox;
        row->child[i].cibox = iboximm_make(0);
    }
    return newrow;
}

bool made_into_placeholder(rowbox * r)
{
    if (r->child.size() == 1)
    {
        r->child.insert(r->child.begin(), iboximm_make(CHAR_Placeholder));
        r->cursor_a = 0;
        r->cursor_b = 1;
        return true;
    }
    else
    {
        return false;
    }
}

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


/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/

#if 0

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
#endif