#include "timing.h"
#include "font.h"
#include "graphics.h"
#include "notebook.h"
#include "box_lex.h"
#include "graphics.h"
#include "svg.h"
#include "eval.h"
#include "ex_print.h"

void cpcolorp(double*a, double *b) {a[0]=b[0]; a[1]=b[1]; a[2]=b[2];}



void drawchar(uint32_t fs, char16_t c, int sizey, int offx, int offy, double * color)
{
    uint32_t size = fs&65535;
    uint32_t family = fs >> 16;
//printf("drawing char: family = %d size = %d   offx = %d offy = %d\n", family, size, offx, offy);
/*
    if (family == FONTFAMILY_SERIFREG)
    {
        glf_serifreg.drawchar(&glb_image, glb_image.serifreg_todraw, c, size, sizey, offx, offy, color);
    }
    else if (family == FONTFAMILY_SERIFBOLD)
    {
        glf_serifbold.drawchar(&glb_image, glb_image.serifbold_todraw, c, size, sizey, offx, offy, color);
    }
    else if (family == FONTFAMILY_SANSREG)
    {
        glf_sansreg.drawchar(&glb_image, glb_image.sansreg_todraw, c, size, sizey, offx, offy, color);
    }
    else if (family == FONTFAMILY_SANSBOLD)
    {
        glf_sansbold.drawchar(&glb_image, glb_image.sansbold_todraw, c, size, sizey, offx, offy, color);
    }
*/
}

int32_t chardrawcount;

void drawtchar(uint32_t fs, char16_t c, int32_t sizex, int32_t sizey, int32_t offx, int32_t offy, uint32_t color, aftransform * T)
{
    int32_t size = fontsize_size(fs);
    uint32_t family = fontsize_family(fs);
    int32_t ci = glb_fonts[family].get_characteridx(c);
    if (ci >= 0)
    {

chardrawcount++;
        affTran S(T->cos_theta, -T->sin_theta,
                  T->sin_theta, T->cos_theta,
                  T->orig_x + T->cos_theta*offx - T->sin_theta*offy,
                  T->orig_y + T->sin_theta*offx + T->cos_theta*offy);
        renderChar(&glb_image, color, glb_chars.chars[ci], S, sizex, sizey);
    }
    else
    {
printf("<gui>: could not print character 0x%04lx\n", (unsigned long)(c));
    }
}


void drawbtchar(double blend, uint32_t fs, char16_t c, int32_t sizex, int32_t sizey, int32_t offx, int32_t offy, uint32_t color, aftransform * T)
{
    int32_t size = fontsize_size(fs);
    uint32_t family = fontsize_family(fs);
    int32_t ci = glb_fonts[family].get_characteridx(c);
    if (ci >= 0)
    {

chardrawcount++;
        affTran S(T->cos_theta, -T->sin_theta,
                  T->sin_theta, T->cos_theta,
                  T->orig_x + T->cos_theta*offx - T->sin_theta*offy,
                  T->orig_y + T->sin_theta*offx + T->cos_theta*offy);
        renderBlendChar(&glb_image, blend, color, glb_chars.chars[ci], S, sizex, sizey);
    }
    else
    {
std::cout << "<gui>: could not print character " << c << std::endl;
    }
}


void drawtline(double X1, double Y1, double X2, double Y2, double e, uint32_t color, aftransform*T)
{
    renderLine(&glb_image,
                T->orig_x + T->cos_theta*(X1) - T->sin_theta*(Y1),
                T->orig_y + T->sin_theta*(X1) + T->cos_theta*(Y1),
                T->orig_x + T->cos_theta*(X2) - T->sin_theta*(Y2),
                T->orig_y + T->sin_theta*(X2) + T->cos_theta*(Y2),
            e, color);
}


void drawtlines(double * coords, size_t nlines, double e, uint32_t color, aftransform*T)
{
    for (size_t i = 0; i < nlines; i++)
    {
        drawtline(coords[2*i+0], coords[2*i+1], coords[2*i+2], coords[2*i+3], e, color, T);
    }
}

void drawtrect(double minx, double maxx, double miny, double maxy, uint32_t color, aftransform*T)
{
    affTran S(T->cos_theta, -T->sin_theta,
              T->sin_theta, T->cos_theta,
              T->orig_x,
              T->orig_y);
    renderRect(&glb_image, color, S, minx, maxx, miny, maxy);
}


void drawbtrect(double blend, double minx, double maxx, double miny, double maxy, uint32_t color, aftransform*T)
{
    affTran S(T->cos_theta, -T->sin_theta,
              T->sin_theta, T->cos_theta,
              T->orig_x,
              T->orig_y);
    renderBlendRect(&glb_image, blend, color, S, minx, maxx, miny, maxy);
}


void _draw_cellgroup_bracket(boxbase * us, boxdrawarg da, bool selected)
{
//std::cout << "_draw_cellgroup_bracket called" << std::endl;

    assert(us->get_type() == BNTYPE_CELL || us->get_type() == BNTYPE_CELLGROUP);

    /* find starting corrodinate y and height for bracket */
    int32_t y     = da.globy;
    int32_t sizey = us->sizey;
    int32_t firstoffy = 0;
    int32_t lastoffy = 0;
    boxbase * first = us;

    while (first->get_type() == BNTYPE_CELLGROUP)
    {
        firstoffy += offyat(first, 0);
        first = childat(first, 0);
    }
    boxbase* last = us;

    while (last->get_type() == BNTYPE_CELLGROUP)
    {
/*
        if (last->extra1 & BNFLAG_OPEN)
        {
*/
            lastoffy += offyat(last, childlen(last) - 1);
            last = childat(last, childlen(last) - 1);
/*
        }
        else
        {
//std::cout << " reading from first !!!!!!!!!!!!!!!!!" << std::endl;
            lastoffy +=  last->array[0].offy;
            last = bto_node(last->array[0].child);
        }
*/
    }

    cellbox* ffirst = dynamic_cast<cellbox*>(first);
    cellbox* llast = dynamic_cast<cellbox*>(last);

    
    firstoffy += ffirst->bracket_offy;
    int32_t firstsizey = ffirst->bracket_sizey;
    lastoffy += llast->bracket_offy;
    int32_t lastsizey = llast->bracket_sizey;

    int32_t cellbracket_w = glb_dingbat.get_char_sizex(DINGBAT_CELLGEN, da.nb->zoomint);
    int32_t cellbracket_h = glb_dingbat.get_char_sizey(DINGBAT_CELLGEN, da.nb->zoomint);

    int32_t delta = 16;
    firstoffy -= delta; firstsizey += 2*delta;
    lastoffy -= delta; lastsizey += 2*delta;

    /* make sure first/last sizey is big enough for dingbat */
    if (firstsizey < cellbracket_h)
    {
        delta = cellbracket_h - firstsizey;
        firstoffy -= delta/2; firstsizey += delta;
    }
    if (lastsizey < cellbracket_h)
    {
        delta = cellbracket_h - lastsizey;
        lastoffy -= delta/2; lastsizey += delta;
    }

    /* we have the location and size of the bracket now */
    y = da.globy + firstoffy;
    sizey = lastsizey + lastoffy - firstoffy;

    int x = glb_image.sizex - cellbracket_w*(2*da.level+1)/2;
    uint32_t color = da.nb->cCellBracket;
    if (selected)
    {        
        color = da.nb->cSelectionForeground;
        glb_dingbat.draw_char(&glb_image, DINGBAT_CELLHIGHLIGHT, da.nb->cSelectionBackground, affTran(1,0,0,1,x,y), cellbracket_w, sizey);
    }

    char16_t c;
    if (us->get_type() == BNTYPE_CELLGROUP)
    {
        c = (true) ? DINGBAT_CELLGEN : DINGBAT_CELLCLOSED;
    }
    else
    {
        cellType ct = dynamic_cast<cellbox*>(us)->celltype;
        c = ct == cellt_INPUT   ? DINGBAT_CELLINPUT :
            ct == cellt_PRINT   ? DINGBAT_CELLOUTPUT :
            ct == cellt_OUTPUT  ? DINGBAT_CELLOUTPUT :
            ct == cellt_MESSAGE ? DINGBAT_CELLMESSAGE :
                                     DINGBAT_CELLTEXT;
    }
    glb_dingbat.draw_char(&glb_image, c, color, affTran(1,0,0,1,x,y), cellbracket_w, sizey);
}



/*
    else if (us->header.type == BNTYPE_BUTTON)
    {
        assert(us->len == 1);
        drawtrect(globx, globx + us->header.sizex, globy, globy + us->header.sizey, RGB_COLOR(220, 220, 220), T);
        for (int i = 0; i < 1; i++)
        {
            _draw_bitmap(bto_node(us->array[i].child), level+1, globx + us->array[i].offx, globy + us->array[i].offy, dflags, T);
        }
    }
    else if (us->header.type == BNTYPE_GRID)
    {
        assert(us->len == us->extra0*us->extra1);
        if (selection.type == SELTYPE_GRID && us == bto_node(_gparent()))
        {
            assert(selection.data.size() == 1);
            int gridi = _gpi();

            int a_x = selection.data[0] % us->extra0;
            int a_y = selection.data[0] / us->extra0;        
            int b_x = gridi % us->extra0;
            int b_y = gridi / us->extra0;
            if (a_x > b_x) {std::swap(a_x, b_x);}
            if (a_y > b_y) {std::swap(a_y, b_y);}

            int * minx = (int *) malloc(us->extra0*sizeof(int));
            int * maxx = (int *) malloc(us->extra0*sizeof(int));
            int * miny = (int *) malloc(us->extra1*sizeof(int));
            int * maxy = (int *) malloc(us->extra1*sizeof(int));
            for (int i = 0; i < us->extra0; i++)
            {
                minx[i] = 2000000000;
                maxx[i] = 0;
            }
            for (int j = 0; j < us->extra1; j++)
            {
                miny[j] = 2000000000;
                maxy[j] = 0;
            }
            for (int j = 0; j < us->extra1; j++) {
                for (int i = 0; i < us->extra0; i++) {
                    boxheader * child = bto_ptr(us->array[us->extra0*j + i].child);
                    minx[i] = std::min(minx[i], us->array[us->extra0*j + i].offx);
                    maxx[i] = std::max(maxx[i], us->array[us->extra0*j + i].offx + child->sizex);
                    miny[j] = std::min(miny[j], us->array[us->extra0*j + i].offy);
                    maxy[j] = std::max(maxy[j], us->array[us->extra0*j + i].offy + child->sizey);
                }
            }

            int mx = globx + (a_x > 0 ? (maxx[a_x-1] + minx[a_x])/2 : 0);
            int my = globy + (a_y > 0 ? (maxy[a_y-1] + miny[a_y])/2 : 0);
            int dx = (b_x + 1 < us->extra0 ? (maxx[b_x] + minx[b_x+1])/2 : us->header.sizex) - (a_x > 0 ? (maxx[a_x-1] + minx[a_x])/2 : 0);
            int dy = (b_y + 1 < us->extra1 ? (maxy[b_y] + miny[b_y+1])/2 : us->header.sizey) - (a_y > 0 ? (maxy[a_y-1] + miny[a_y])/2 : 0);

            drawtrect(mx, mx + dx, my, my + dy, cSelectionBackground, T);

            free(maxy);
            free(miny);
            free(maxx);
            free(minx);

            for (int j = 0; j < us->extra1; j++)
            {
                for (int i = 0; i < us->extra0; i++)
                {
                    _draw_bitmap(bto_node(us->array[us->extra0*j + i].child), level+1,
                                     globx + us->array[us->extra0*j + i].offx,
                                     globy + us->array[us->extra0*j + i].offy,
                                     dflags | ((a_x <= i && i <= b_x && a_y <= j && j <= b_y) ? DFLAG_SELECTION : 0), T);
                }
            }
        }
        else
        {
            for (int j = 0; j < us->extra1; j++)
            {
                for (int i = 0; i < us->extra0; i++)
                {
                    _draw_bitmap(bto_node(us->array[us->extra0*j + i].child), level+1,
                                    globx + us->array[us->extra0*j + i].offx,
                                    globy + us->array[us->extra0*j + i].offy,
                                    dflags, T);
                }
            }
        }
    }
    else if (us->header.type == BNTYPE_TABVIEW)
    {
        drawtrect(globx, globx + us->header.sizex, globy, globy + us->header.sizey, RGB_COLOR(240, 240, 240), T);
        for (int32_t j = 0; j < us->len; j += 2)
        {
            _draw_bitmap(bto_node(us->array[j].child), level+1,
                             globx + us->array[j].offx,
                             globy + us->array[j].offy,
                             dflags, T);
        }

        int32_t j = us->extra0;
        assert(j < us->len/2);
            _draw_bitmap(bto_node(us->array[2*j+1].child), level+1,
                             globx + us->array[2*j+1].offx,
                             globy + us->array[2*j+1].offy,
                             dflags, T);
    }
*/
