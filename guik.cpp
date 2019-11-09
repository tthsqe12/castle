#include <GL/glew.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<string>
#include<memory>
#include<vector>
#include<stack>
#include<list>
#include<map>
#include<set>
#include<assert.h>
#include <cstring>

#include "ex.h"
#include "timing.h"
#include "graphics.h"
#include "notebook.h"
#include "font.h"
#include "svg.h"
#include "serialize.h"
#include "ex_print.h"
#include "box_convert.h"

FILE * fp_in, * fp_out;
int fp_out_type;
//pthread_t gui_read_thread;
GThread * gui_read_thread;

GtkWidget * palette_window;
GtkWidget * palette_drawing_area;
GdkPixbuf * palette_pixbuf = NULL;
glimage palette_image;

GtkWidget * window;
GtkWidget * drawing_area;
GdkPixbuf * pixbuf = NULL;
glimage glb_image;


std::vector<void*> exs_in_use;
globalstate gs;
notebook main_nb;

svgGlyphSet glb_glyphs;
svgCharSet glb_chars;
std::vector<svgFont> glb_fonts;
svgFont glb_dingbat("Dingbat", 0.625, 2.0, 1.0,  0.01,0.1,0.2,0.1);

void glyph_init()
{
    glb_fonts.emplace_back("fCourierBold", 0.5, 25./16, 25./32,  0.14,0.3125,0.375,0.3);
    glb_fonts.emplace_back("fCourierReg",  0.5, 25./16, 25./32,  0.07,0.3125,0.375,0.3);
    glb_fonts.emplace_back("fTahomaReg",   0.5, 25./16, 25./32,  0.13,0.3,0.3,0.1);
    glb_fonts.emplace_back("fTahomaBold",  0.5, 25./16, 25./32,  0.10,0.3,0.3,0.1);
    glb_fonts.emplace_back("fTimesReg",    0.5, 25./16, 25./32,  0.10,0.3,0.3,0.1);
    glb_fonts.emplace_back("fTimesBold",   0.5, 25./16, 25./32,  0.10,0.3,0.3,0.1);
/*
    svgGlyph b1("1,0,0,1,0,0 M 14.664063 -18.445313 C 14.660156 -18.246094 14.617188 -18.046875 14.53125 -17.847656 C 14.445313 -17.648438 14.277344 -17.546875 14.027344 -17.550781 C 13.722656 -17.546875 13.398438 -17.683594 13.054688 -17.953125 C 12.710938 -18.222656 12.539063 -18.519531 12.542969 -18.851563 C 12.539063 -19.0625 12.496094 -19.269531 12.417969 -19.472656 C 12.332031 -19.667969 12.15625 -19.769531 11.882813 -19.773438 C 11.25 -19.769531 10.707031 -19.367188 10.261719 -18.566406 C 9.808594 -17.757813 9.464844 -16.617188 9.230469 -15.144531 C 9.023438 -13.898438 8.894531 -12.664063 8.839844 -11.4375 C 8.78125 -10.210938 8.738281 -9.375 8.710938 -8.929688 C 8.628906 -7.742188 8.539063 -6.34375 8.4375 -4.734375 C 8.335938 -3.121094 8.1875 -1.445313 8 0.296875 C 7.9375 0.847656 7.828125 1.664063 7.667969 2.75 C 7.503906 3.828125 7.292969 4.71875 7.03125 5.421875 C 6.71875 6.257813 6.277344 6.972656 5.710938 7.5625 C 5.144531 8.152344 4.386719 8.449219 3.433594 8.449219 C 2.714844 8.449219 2.09375 8.269531 1.574219 7.910156 C 1.050781 7.550781 0.789063 7.097656 0.792969 6.550781 C 0.789063 6.351563 0.832031 6.132813 0.921875 5.898438 C 1.007813 5.660156 1.1875 5.542969 1.453125 5.546875 C 1.78125 5.542969 2.125 5.695313 2.488281 6 C 2.84375 6.304688 3.023438 6.628906 3.027344 6.976563 C 3.023438 7.144531 3.0625 7.351563 3.148438 7.597656 C 3.226563 7.839844 3.386719 7.960938 3.625 7.964844 C 4.273438 7.960938 4.785156 7.613281 5.15625 6.914063 C 5.523438 6.214844 5.789063 5.355469 5.953125 4.34375 C 6.105469 3.367188 6.191406 2.429688 6.210938 1.523438 C 6.226563 0.617188 6.242188 -0.121094 6.265625 -0.691406 C 6.3125 -2.570313 6.382813 -4.179688 6.46875 -5.519531 C 6.554688 -6.855469 6.65625 -8.464844 6.773438 -10.351563 C 6.820313 -11.09375 6.925781 -12.066406 7.097656 -13.273438 C 7.261719 -14.476563 7.515625 -15.558594 7.851563 -16.511719 C 8.257813 -17.664063 8.8125 -18.574219 9.523438 -19.242188 C 10.226563 -19.90625 11.078125 -20.238281 12.078125 -20.242188 C 12.675781 -20.238281 13.257813 -20.089844 13.820313 -19.800781 C 14.378906 -19.503906 14.660156 -19.054688 14.664063 -18.445313 Z");
    std::cout << "int: " << std::endl;
std::cout << "x range: (" << b1.minx <<", "<< b1.maxx << ")" <<std::endl;
std::cout << "y range: (" << b1.miny <<", "<< b1.maxy << ")" <<std::endl;

    svgGlyph b2("1,0,0,1,0,0 M 16.625 5.695313 L 12.480469 5.695313 L 12.480469 5.433594 C 12.816406 5.429688 13.078125 5.378906 13.265625 5.277344 C 13.453125 5.171875 13.546875 5.03125 13.546875 4.851563 L 13.546875 -13.261719 C 13.546875 -13.332031 13.515625 -13.398438 13.460938 -13.453125 C 13.398438 -13.503906 13.304688 -13.527344 13.171875 -13.53125 L 4.488281 -13.53125 C 4.371094 -13.527344 4.28125 -13.503906 4.21875 -13.457031 C 4.152344 -13.40625 4.117188 -13.347656 4.121094 -13.285156 L 4.121094 4.851563 C 4.117188 5.082031 4.253906 5.234375 4.523438 5.316406 C 4.789063 5.390625 5.019531 5.429688 5.222656 5.433594 L 5.222656 5.695313 L 1.066406 5.695313 L 1.066406 5.433594 C 1.339844 5.429688 1.585938 5.382813 1.804688 5.289063 C 2.023438 5.191406 2.132813 5.050781 2.132813 4.863281 L 2.132813 -13.484375 C 2.132813 -13.710938 1.992188 -13.851563 1.710938 -13.898438 C 1.429688 -13.941406 1.214844 -13.960938 1.066406 -13.964844 L 1.066406 -14.328125 L 16.625 -14.328125 L 16.625 -13.964844 C 16.453125 -13.960938 16.230469 -13.933594 15.953125 -13.882813 C 15.667969 -13.828125 15.527344 -13.6875 15.53125 -13.457031 L 15.53125 4.886719 C 15.527344 5.101563 15.648438 5.246094 15.894531 5.320313 C 16.136719 5.394531 16.378906 5.429688 16.625 5.433594 Z");
    std::cout << "sum: " << std::endl;
std::cout << "x range: (" << b2.minx <<", "<< b2.maxx << ")" <<std::endl;
std::cout << "y range: (" << b2.miny <<", "<< b2.maxy << ")" <<std::endl;

    svgGlyph b3("1,0,0,1,0,0 M 16.496094 -0.363281 L 16.210938 5.695313 L 0.558594 5.695313 L 8.511719 -4.242188 L 1.300781 -14.328125 L 15.519531 -14.328125 L 15.679688 -9.425781 L 15.296875 -9.425781 C 15.054688 -10.863281 14.683594 -11.890625 14.1875 -12.511719 C 13.730469 -13.066406 13.121094 -13.347656 12.351563 -13.347656 L 4.429688 -13.347656 L 10.320313 -4.773438 L 3.226563 4.105469 L 13.011719 4.105469 C 13.71875 4.101563 14.339844 3.730469 14.871094 2.996094 C 15.4375 2.207031 15.839844 1.089844 16.078125 -0.363281 Z");
    std::cout << "prod: " << std::endl;
std::cout << "x range: (" << b3.minx <<", "<< b3.maxx << ")" <<std::endl;
std::cout << "y range: (" << b3.miny <<", "<< b3.maxy << ")" <<std::endl;
*/
    return;
}


static void draw_pixbuf(GtkWidget* widget)
{
    if (pixbuf == NULL)
        return;


    main_nb.io_mutex.lock();
chardrawcount = 0;
uint64_t time1 = GetMS();

    glb_image.pixel_width = gdk_pixbuf_get_width(pixbuf);
    glb_image.pixel_height = gdk_pixbuf_get_height(pixbuf);
    glb_image.rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    glb_image.pixels = gdk_pixbuf_get_pixels(pixbuf);
    glb_image.units_per_pixel = UNITS_PER_PIXEL;
    glb_image.sizex = glb_image.units_per_pixel * glb_image.pixel_width;
    glb_image.sizey = glb_image.units_per_pixel * glb_image.pixel_height;

uint64_t time2 = GetMS();

    std::memset(glb_image.pixels, 255, glb_image.pixel_height*glb_image.rowstride);
    

uint64_t time3 = GetMS();


    main_nb.draw_bitmap();
uint64_t time4 = GetMS();

    main_nb.io_mutex.unlock();

//printf("setup time: %d\n",time2-time1);
//printf(" fill time: %d\n",time3-time2);
//printf(" draw time: %d\n",time4-time3);
//printf("total time: %d\n",time4-time1);
//printf("characters draw: %d\n", chardrawcount);

    gtk_widget_queue_draw(widget);
    gdk_window_process_all_updates();   // draw window before processing key event
}





static void draw_pixbuf(GtkWidget* widget);

static gboolean configure_event(GtkWidget* widget, GdkEventConfigure* event)
{

    if (pixbuf)
        g_object_unref(pixbuf);

    int Bitmap_x = std::max(widget->allocation.width, 100);
    int Bitmap_y = std::max(widget->allocation.height, 100);
//    main_nb.resize(Bitmap_x, Bitmap_y);

    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 1, 8, Bitmap_x, Bitmap_y);
    draw_pixbuf(widget);
    gtk_widget_queue_draw_area(widget, 0, 0, Bitmap_x, Bitmap_y);

    return TRUE;
}

static gboolean expose_event(GtkWidget* widget, GdkEventExpose* event)
{
    cairo_t * cr = gdk_cairo_create(widget->window);
    gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
    return FALSE;
}

static gboolean button_press_event(GtkWidget* widget, GdkEventButton* event)
{
    if (event->button == 1)
    {
        if (event->type == GDK_2BUTTON_PRESS)
        {
            main_nb.handle_doubleclick(
                                UNITS_PER_PIXEL*event->x - main_nb.offx,
                                UNITS_PER_PIXEL*event->y - main_nb.offy);
        }
        else
        {
            main_nb.handle_click(
                                UNITS_PER_PIXEL*event->x - main_nb.offx,
                                UNITS_PER_PIXEL*event->y - main_nb.offy);
        }
        draw_pixbuf(widget);
    }

    return TRUE;
}

static gboolean motion_notify_event(GtkWidget* widget, GdkEventMotion* event)
{
    int x, y;
    GdkModifierType state;

    if (event->is_hint) {
        gdk_window_get_pointer(event->window, &x, &y, &state);
    } else {
        x = event->x;
        y = event->y;
        state = (GdkModifierType) event->state;
    }
    
    if (state & GDK_BUTTON1_MASK) {
        /* mouse movement here */
        
    }
  
    return TRUE;
}



void _handle_open(notebook&nb)
{
    GtkWidget * dialog = gtk_file_chooser_dialog_new(
                                        "Open File",
                                        NULL,
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        "_Cancel",
                                        GTK_RESPONSE_CANCEL,
                                        "_Open",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);

    if (gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char * filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        if (!nb.open(filename))
        {
            std::cout << "could not load " << filename << std::endl;
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}


void _handle_saveas(notebook&nb)
{
    GtkWidget * dialog = gtk_file_chooser_dialog_new(
                                    "Save File",
                                    NULL,
                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                        "_Cancel",
                                    GTK_RESPONSE_CANCEL,
                                        "_Save",
                                    GTK_RESPONSE_ACCEPT,
                                    NULL);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    if (nb.filestring.empty())
    {
        gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "Untitled notebook");
    }
    else
    {
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), nb.filestring.c_str());
    }
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char * filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        nb.save(filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void _handle_save(notebook&nb)
{
//std::cout << "file string : " << nb.filestring << std::endl;
    if (nb.filestring.empty())
    {
        _handle_saveas(nb);
    }
    else
    {
        nb.save(nb.filestring.c_str());
    }
}


gboolean on_key_press(GtkWidget * widget, GdkEventKey * event, gpointer user_data)
{
//printf("key event: keyval = 0x%08x, state = 0x%08x\n",event->keyval,event->state);

    bool shift = event->state & GDK_SHIFT_MASK;
    bool ctrl = event->state & GDK_CONTROL_MASK;
    bool alt = event->state & GDK_MOD1_MASK;

    main_nb.io_mutex.lock();

    switch (event->keyval)
    {

    case GDK_KEY_KP_Add:
        ctrl ? main_nb.zoom_in() : main_nb.insert_char('+');
        break;
    case GDK_KEY_KP_Subtract:
        ctrl ? main_nb.zoom_out() : main_nb.insert_char('-');
        break;

    case GDK_KEY_Left:
           ctrl ? main_nb.key_ctrlleft()
        :   alt ? main_nb.key_altleft()
        : shift ? main_nb.key_shiftleft()
        :         main_nb.key_left();
        break;
    case GDK_KEY_Right:
           ctrl ? main_nb.key_ctrlright()
        :   alt ? main_nb.key_altright()
        : shift ? main_nb.key_shiftright()
        :         main_nb.key_right();
        break;
    case GDK_KEY_Up:
           ctrl ? main_nb.key_ctrlup()
        :   alt ? main_nb.key_altup()
        : shift ? main_nb.key_shiftup()
        :         main_nb.key_up();
        break;
    case GDK_KEY_Down:
           ctrl ? main_nb.key_ctrldown()
        :   alt ? main_nb.key_altdown()
        : shift ? main_nb.key_shiftdown()
        :         main_nb.key_down();
        break;
    case GDK_KEY_Page_Up:
        main_nb.key_pageup();
        break;
    case GDK_KEY_Page_Down:
        main_nb.key_pagedown();
        break;
    case GDK_KEY_period:
        main_nb.insert_char('.');
        break;
    case GDK_KEY_underscore:
        main_nb.insert_char('_');
        break;
    case GDK_KEY_question:
        main_nb.insert_char('?');
        break;
    case GDK_KEY_quoteleft:
        main_nb.insert_char('`');
        break;
    case GDK_KEY_exclam:
        main_nb.insert_char('!');
        break;
    case GDK_KEY_at:
        main_nb.insert_char('@');
        break;
    case GDK_KEY_numbersign:
        main_nb.insert_char('#');
        break;
    case GDK_KEY_dollar:
        main_nb.insert_char('$');
        break;
    case GDK_KEY_percent:
        main_nb.insert_char('%');
        break;
    case GDK_KEY_asciicircum:
        main_nb.insert_char('^');
        break;
    case GDK_KEY_ampersand:
        main_nb.insert_char('&');
        break;
    case GDK_KEY_asterisk:
        main_nb.insert_char('*');
        break;
    case GDK_KEY_asciitilde:
        main_nb.insert_char('~');
        break;
    case GDK_KEY_plus:
        main_nb.insert_char('+');
        break;
    case GDK_KEY_slash:
        ctrl ? main_nb.insert_fraction() : main_nb.insert_char('/');
        break;
    case GDK_KEY_minus:
        ctrl ? main_nb.insert_subscript() : main_nb.insert_char('-');
        break;
    case GDK_KEY_Escape:
        main_nb.insert_char(CHAR_AliasDelimiter);
        break;
    case GDK_KEY_backslash:
        main_nb.insert_char('\\');
        break;
    case GDK_KEY_equal:
        main_nb.insert_char('=');
        break;
    case GDK_KEY_bracketleft:
        main_nb.insert_char('[');
        break;
    case GDK_KEY_bracketright:
        main_nb.insert_char(']');
        break;
    case GDK_KEY_braceleft:
        main_nb.insert_char('{');
        break;
    case GDK_KEY_braceright:
        main_nb.insert_char('}');
        break;
    case GDK_KEY_parenleft:
        main_nb.insert_char('(');
        break;
    case GDK_KEY_parenright:
        main_nb.insert_char(')');
        break;
    case GDK_KEY_quotedbl:
        main_nb.insert_char('"');
        break;
    case GDK_KEY_apostrophe:
        main_nb.insert_char('\'');
        break;
    case GDK_KEY_bar:
        main_nb.insert_char('|');
        break;
    case GDK_KEY_Delete:
        main_nb.key_delete();
        break;
    case GDK_KEY_BackSpace:
        main_nb.key_backspace();
        break;
    case GDK_KEY_Return:
        ctrl ? main_nb.insert_gridrow() : shift ? main_nb.key_shiftenter() : main_nb.insert_newline();
        break;
    case GDK_KEY_comma:
        ctrl ? main_nb.insert_gridcol() : main_nb.insert_char(',');
        break;
    case GDK_KEY_colon:
        main_nb.insert_char(':');
        break;
    case GDK_KEY_Home:
        main_nb.key_home();
        break;
    case GDK_KEY_End:
        main_nb.key_end();
        break;
    case GDK_KEY_semicolon:
        main_nb.insert_char(';');
        break;
    case GDK_KEY_greater:
        main_nb.insert_char('>');
        break;
    case GDK_KEY_less:
        main_nb.insert_char('<');
        break;
    case GDK_KEY_0:
        alt ? main_nb.key_makecell(CELLTYPE_BOLDTEXT)
            : main_nb.insert_char('0');
        break;
    case GDK_KEY_1:
        alt ? main_nb.key_makecell(CELLTYPE_TITLE)
            : main_nb.insert_char('1');
        break;
    case GDK_KEY_2:
        alt ? main_nb.key_makecell(CELLTYPE_SECTION)
            : ctrl ? main_nb.insert_sqrt() : main_nb.insert_char('2');
        break;
    case GDK_KEY_3:
        alt ? main_nb.key_makecell(CELLTYPE_SUBSECTION)
            : main_nb.insert_char('3');
        break;
    case GDK_KEY_4:
        alt ? main_nb.key_makecell(CELLTYPE_SUBSUBSECTION)
            : (ctrl ? main_nb.insert_underscript() : main_nb.insert_char('4'));
        break;
    case GDK_KEY_5:
        alt ? main_nb.key_makecell(CELLTYPE_TEXT)
            : ctrl ? main_nb.key_switch() : main_nb.insert_char('5');
        break;
    case GDK_KEY_6:
        alt ? main_nb.key_makecell(CELLTYPE_MESSAGE)
            : (ctrl ? main_nb.insert_superscript() : main_nb.insert_char('6'));
        break;
    case GDK_KEY_7:
        alt ? main_nb.key_makecell(CELLTYPE_PRINT)
            : (ctrl ? main_nb.insert_overscript() : main_nb.insert_char('7'));
        break;
    case GDK_KEY_8:
        alt ? main_nb.key_makecell(CELLTYPE_OUTPUT)
            : main_nb.insert_char('8');
        break;
    case GDK_KEY_9:
        alt ? main_nb.key_makecell(CELLTYPE_INPUT)
            : main_nb.insert_char('9');
        break;
    case GDK_KEY_space:
        if (ctrl)
        {
            boxnode * newrow = boxnode_create(BNTYPE_ROW, 12);
            boxnode_append_cstr(newrow, "test");
            boxnode_append(newrow, bfrom_ptr(&box_null));
            box newcell = boxnode_make(BNTYPE_CELL, bfrom_node(newrow));
            bto_node(newcell)->extra0 = CELLTYPE_OUTPUT;
            main_nb.print_cell(newcell);
        }
        else
        {
            main_nb.insert_char(' ');
        }
        break;

    /* capture save and open specially */
    case GDK_KEY_S: case GDK_KEY_s:
        ctrl ? (shift ? _handle_saveas(main_nb) : _handle_save(main_nb)) :  main_nb.insert_char(shift ? 'S' : 's');
        break;
    case GDK_KEY_O: case GDK_KEY_o:
        ctrl ? _handle_open(main_nb) : main_nb.insert_char(shift ? 'O' : 'o');
        break;

    default:
        if (GDK_KEY_a <= event->keyval && event->keyval <= GDK_KEY_z)
        {
            ctrl ? main_nb.key_ctrl_char(event->keyval - GDK_KEY_a + 'a')
                 : main_nb.insert_char(event->keyval - GDK_KEY_a + 'a');
            break;
        }
        else if (GDK_KEY_A <= event->keyval && event->keyval <= GDK_KEY_Z)
        {
            ctrl ? main_nb.key_ctrl_char(event->keyval - GDK_KEY_A + 'A')
                 : main_nb.insert_char(event->keyval - GDK_KEY_A + 'A');
            break;
        }
    }

    main_nb.io_mutex.unlock();

    draw_pixbuf(widget);

    return FALSE; 
}







void quit()
{
    fclose(fp_in);
    fclose(fp_out);
//    pthread_join(gui_read_thread, NULL);
    exit(0);
}





gint myfunc(void * unused)
{
        draw_pixbuf(drawing_area);
        return FALSE; // call only once
}


gpointer gui_read_loop(gpointer data)
{
    std::string s;
    while (1)
    {
        uint8_t t;
        if (0 != sread_byte(fp_out, t))
        {
std::cout << "<gui reader>: could not read" << std::endl;
            break;
        }

        if (t == CMD_EXPR)
        {
            uex e;
            int err = sread_ex(fp_out, e);
std::cout << "<gui reader>: CMD_EXPR err: " << err << "  e: " << ex_tostring_full(e.get()) << std::endl;
            if (err != 0)
            {
                std::cerr << "ERROR gui: could not read CMD_EXPR - error " << err << std::endl;
            }
            else if (ehas_head_sym_length(e.get(), gs.symsOutputNamePacket.get(), 1))
            {
                main_nb.io_mutex.lock();
                main_nb.out_name.reset(e.copychild(1));
                main_nb.io_mutex.unlock();
            }
            else if (ehas_head_sym_length(e.get(), gs.symsInputNamePacket.get(), 1))
            {
                main_nb.io_mutex.lock();
                main_nb.in_name.reset(e.copychild(1));
                main_nb.io_mutex.unlock();
            }
            else if (ehas_head_sym_length(e.get(), gs.symsReturnTextPacket.get(), 1))
            {
                boxnode * row = boxnode_convert_from_ex(e.child(1));
                if (row == nullptr || boxnode_type(row) != BNTYPE_ROW)
                {
                    std::cerr << "ERROR gui: could not convert " << ex_tostring_full(e.get()) << std::endl;
                    if (row != nullptr)
                    {
                        box_node_delete(row);
                    }
                }
                else
                {
                    box cell = boxnode_make(BNTYPE_CELL, bfrom_node(row));
                    if (main_nb.out_name.get() != nullptr)
                    {
                        boxnode * label = boxnode_convert_from_ex(main_nb.out_name.get());
                        if (label == nullptr || boxnode_type(label) != BNTYPE_ROW)
                        {
                            std::cerr << "ERROR gui: could not convert " << ex_tostring_full(main_nb.out_name.get()) << std::endl;
                            if (label != nullptr)
                            {
                                box_node_delete(label);
                            }
                        }
                        else
                        {
                            boxnode_append(cell, bfrom_node(label));
                        }
                    }
                    bto_node(cell)->extra0 = CELLTYPE_OUTPUT;
                    main_nb.io_mutex.lock();
                    main_nb.print_cell(cell);
                    main_nb.io_mutex.unlock();
                    g_idle_add(myfunc, NULL);
                }
            }
            else if (ehas_head_sym_length(e.get(), gs.symsMessagePacket.get(), 2))
            {
                e.replacechild(0, gs.symsList.copy());
                e.reset(emake_node(gs.symsRowBox.copy(), e.release()));
                boxnode * row = boxnode_convert_from_ex(e.get());
                if (row == nullptr || boxnode_type(row) != BNTYPE_ROW)
                {
                    std::cerr << "ERROR gui: could not convert " << ex_tostring_full(e.get()) << std::endl;
                    if (row != nullptr)
                    {
                        box_node_delete(row);
                    }
                }
                else
                {
                    box cell = boxnode_make(BNTYPE_CELL, bfrom_node(row));
                    bto_node(cell)->extra0 = CELLTYPE_MESSAGE;
                    main_nb.io_mutex.lock();
                    main_nb.print_cell(cell);
                    main_nb.io_mutex.unlock();
                    g_idle_add(myfunc, NULL);
                }
            }
            else if (ehas_head_sym_length(e.get(), gs.symsTextPacket.get(), 1))
            {
                e.replacechild(0, gs.symsList.copy());
                e.reset(emake_node(gs.symsRowBox.copy(), e.release()));
                boxnode * row = boxnode_convert_from_ex(e.get());
                if (row == nullptr || boxnode_type(row) != BNTYPE_ROW)
                {
                    std::cerr << "ERROR gui: could not convert " << ex_tostring_full(e.get()) << std::endl;
                    if (row != nullptr)
                    {
                        box_node_delete(row);
                    }
                }
                else
                {
                    box cell = boxnode_make(BNTYPE_CELL, bfrom_node(row));
                    bto_node(cell)->extra0 = CELLTYPE_PRINT;
                    main_nb.io_mutex.lock();
                    main_nb.print_cell(cell);
                    main_nb.io_mutex.unlock();
                    g_idle_add(myfunc, NULL);
                }
            }
        }
        else if (t == CMD_SYNTAX)
        {
            if (0 != sread_byte(fp_out, t))
                break;

            sread_string(fp_out, s);
            if (t != 0)
            {
                gs.char_set.insert(s);
            }
            else
            {
                gs.char_set.erase(s);
            }
std::cout << "<gui reader>: CMD_SYNTAX state: " << int(t) << "  s: " << s << std::endl;
        }
        else
        {
            std::cerr << "<!gui reader>: received unknown cmd " << (int)(t) << std::endl;
        }
    }

    std::cout << "gui: read thread exiting" << std::endl;
    return NULL;
}







void draw_button_rect(int32_t minx, int32_t maxx, int32_t miny, int32_t maxy, uint32_t inside_color, uint32_t border_color)
{
    for (int32_t j = miny; j <= maxy; j++)
    for (int32_t i = minx; i <= maxx; i++)
    {
        if (0 <= i && i < palette_image.pixel_width
            && 0 <= j && j < palette_image.pixel_height)
        {
            uint32_t color = inside_color;
            if (i == minx || i == maxx || j == miny || j == maxy)
                color = border_color;
            uint8_t * p = palette_image.pixels + palette_image.rowstride*(j) + 4*(i);
            p[0] = color&255;
            p[1] = (color>>8)&255;
            p[2] = (color>>16)&255;
            p[3] = 255;
        }
    }
}

char16_t palette_start_char=0;

static void palette_draw_pixbuf(GtkWidget* widget)
{
    if (palette_pixbuf == NULL)
        return;

    palette_image.pixel_width = gdk_pixbuf_get_width(palette_pixbuf);
    palette_image.pixel_height = gdk_pixbuf_get_height(palette_pixbuf);
    palette_image.rowstride = gdk_pixbuf_get_rowstride(palette_pixbuf);
    palette_image.pixels = gdk_pixbuf_get_pixels(palette_pixbuf);
    palette_image.units_per_pixel = 1;
    palette_image.sizex = palette_image.units_per_pixel * palette_image.pixel_width;
    palette_image.sizey = palette_image.units_per_pixel * palette_image.pixel_height;

    std::memset(palette_image.pixels, 255, palette_image.pixel_height*palette_image.rowstride);

    affTran T(1,0,0,1,10,10);
    for (int i = 0; i < 256; i++)
    {
        int32_t minx = 100 + 20*(i%32) + 20;
        int32_t maxx = 100 + 20*(i%32) + 38;
        int32_t miny = 24*(i/32) + 10;
        int32_t maxy = 24*(i/32) + 32;
        T.e = 1+(minx*7+maxx*1)/8;
        T.f = 2+(miny*7+maxy*1)/8;
        draw_button_rect(minx, maxx, miny, maxy, RGB_COLOR(0xf0, 0xf0, 0xf0), RGB_COLOR(0x40, 0x30, 0x00));
        glb_fonts[0].draw_char(&palette_image, palette_start_char+i, RGB_COLOR(0x00, 0x00, 0x00),
                                             T, 5*(maxx-minx)/8, 5*(maxy-miny)/8);
    }

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            int32_t c = 15 & ((palette_start_char + 32*i) >> (12-4*j));
            c += (c < 10) ? '0' : 'a' - 10;
            T.e = 760 + 6*j;
            T.f = 6+24*i+10;
            glb_fonts[2].draw_char(&palette_image, c, RGB_COLOR(0x30, 0x40, 0x80),
                                                 T, 6, 8);
            
        }
    }

    T.e = 5;
    T.f = 25;
    glb_fonts[0].draw_char(&palette_image, palette_start_char, RGB_COLOR(0x00, 0x00, 0x00),
                                           T, 80, 160);


    gtk_widget_queue_draw(widget);
    gdk_window_process_all_updates();   // draw window before processing key event
}


static gboolean palette_configure_event(GtkWidget* widget, GdkEventConfigure* event)
{
    if (palette_pixbuf)
        g_object_unref(palette_pixbuf);

    int Bitmap_x = std::max(widget->allocation.width, 100);
    int Bitmap_y = std::max(widget->allocation.height, 100);

    palette_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 1, 8, Bitmap_x, Bitmap_y);
    palette_draw_pixbuf(widget);
    gtk_widget_queue_draw_area(widget, 0, 0, Bitmap_x, Bitmap_y);

    return TRUE;
}

static gboolean palette_expose_event(GtkWidget* widget, GdkEventExpose* event)
{
    cairo_t * cr = gdk_cairo_create(widget->window);
    gdk_cairo_set_source_pixbuf(cr, palette_pixbuf, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
    return FALSE;
}

static gboolean palette_button_press_event(GtkWidget* widget, GdkEventButton* event)
{
    if (event->button == 1)
    {
std::cout << "pallet clicked at " << event->x << ", " << event->y << std::endl;
    }

    return TRUE;
}

static gboolean palette_motion_notify_event(GtkWidget* widget, GdkEventMotion* event)
{
    return TRUE;
}

static gboolean palette_on_key_press(GtkWidget * widget, GdkEventKey * event, gpointer user_data)
{
//printf("key event: keyval = 0x%08x, state = 0x%08x\n",event->keyval,event->state);

    bool shift = event->state & GDK_SHIFT_MASK;
    bool ctrl = event->state & GDK_CONTROL_MASK;
    bool alt = event->state & GDK_MOD1_MASK;

    switch (event->keyval)
    {

    case GDK_KEY_KP_Add:
        break;
    case GDK_KEY_KP_Subtract:
        break;

    case GDK_KEY_Left:
        palette_start_char-=1;
        break;
    case GDK_KEY_Right:
        palette_start_char+=1;
        break;
    case GDK_KEY_Up:
        palette_start_char-=32;
        break;
    case GDK_KEY_Down:
        palette_start_char+=32;
        break;
    case GDK_KEY_Page_Up:
        palette_start_char-=128;
        break;
    case GDK_KEY_Page_Down:
        palette_start_char+=128;
        break;

    default:
        break;
    }


    palette_draw_pixbuf(widget);

    return FALSE; 
}



void makepalette()
{
        palette_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW (palette_window), "All Characters");
//        GtkWidget *button = gtk_button_new_from_stock (GTK_STOCK_ADD);
//        gchar *title;

        GtkWidget * vbox = gtk_vbox_new(FALSE, 0);
        gtk_container_add (GTK_CONTAINER(palette_window), vbox);
        gtk_widget_show(vbox);


        palette_drawing_area = gtk_drawing_area_new();
        gtk_widget_set_size_request(GTK_WIDGET (palette_drawing_area), 790, 220);
        gtk_box_pack_start(GTK_BOX(vbox), palette_drawing_area, TRUE, TRUE, 0);
        gtk_widget_show(palette_drawing_area);
        gtk_widget_show(vbox);

        g_signal_connect(G_OBJECT(palette_window), "key_press_event", G_CALLBACK(palette_on_key_press), NULL);


        g_signal_connect(palette_drawing_area, "expose_event", G_CALLBACK(palette_expose_event), NULL);
        g_signal_connect(palette_drawing_area, "configure_event", G_CALLBACK (palette_configure_event), NULL);
        g_signal_connect(palette_drawing_area, "motion_notify_event", G_CALLBACK(palette_motion_notify_event), NULL);
        g_signal_connect(palette_drawing_area, "button_press_event", G_CALLBACK(palette_button_press_event), NULL);
        gtk_widget_set_events(palette_drawing_area, GDK_EXPOSURE_MASK | GDK_LEAVE_NOTIFY_MASK
			                                                  | GDK_BUTTON_PRESS_MASK
			                                                  | GDK_POINTER_MOTION_MASK
			                                                  | GDK_POINTER_MOTION_HINT_MASK);

        gtk_widget_show_all(palette_window);

}


int main(int argc, char * argv[])
{
#define READ_END 0
#define WRITE_END 1
    int finput[2];
    int foutput[2];

    if (pipe(finput)==-1) {fprintf(stderr, "Pipe Failed" ); return 1;} 
    if (pipe(foutput)==-1) {fprintf(stderr, "Pipe Failed" ); return 1;} 
    pid_t p = fork(); if (p < 0) {fprintf(stderr, "fork Failed"); return 1;} 
    if (p == 0) 
    {
        // Lets start a kernel
        // TODO: move this somewhere else so we can restart it if it dies
        // TODO: more robust way of finding kernel path
        std::string kernel_path(argv[0]);
        kernel_path.pop_back();
        kernel_path.pop_back();
        kernel_path.pop_back();
        kernel_path.pop_back();
        kernel_path.append("kernel");
        close(finput[WRITE_END]);
        close(foutput[READ_END]); 
        char outstr[20], instr[20];
        sprintf(outstr, "%d", foutput[WRITE_END]);
        sprintf(instr, "%d", finput[READ_END]);
std::cout << "kernel path: " << kernel_path << std::endl;
        execlp(kernel_path.c_str(), kernel_path.c_str(), outstr, instr, NULL);
    }
    else
    {
        close(finput[READ_END]);
        close(foutput[WRITE_END]);
        fp_in = fdopen(finput[WRITE_END], "wb");
        fp_out = fdopen(foutput[READ_END], "rb");
        fp_out_type = FP_OUT_NONE;

        gui_read_thread = g_thread_new("asdf", gui_read_loop, NULL);

        glyph_init();

        gtk_init(&argc, &argv);

        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "castle");

        GtkWidget * vbox = gtk_vbox_new(FALSE, 0);
        gtk_container_add (GTK_CONTAINER (window), vbox);
        gtk_widget_show(vbox);

        g_signal_connect(window, "destroy", G_CALLBACK(quit), NULL);
        g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK (on_key_press), NULL);

GtkWidget *menubar;
GtkWidget *filemenu;
GtkWidget *file;
GtkWidget *file_new; 
GtkWidget *file_open;
GtkWidget *file_save; 
GtkWidget *file_quit;
GtkWidget *palettesmenu;
GtkWidget *palettes;
GtkWidget *palettes_all; 

menubar = gtk_menu_bar_new();

filemenu = gtk_menu_new();
file = gtk_menu_item_new_with_label("File");
file_new = gtk_menu_item_new_with_label("New"); 
file_open = gtk_menu_item_new_with_label("Open");
file_save = gtk_menu_item_new_with_label("Save");
file_quit = gtk_menu_item_new_with_label("Quit"); 
gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), file_new);
gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), file_open);
gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), file_save); 
gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), file_quit);
gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);


palettesmenu = gtk_menu_new();
palettes = gtk_menu_item_new_with_label("Palettes");
palettes_all = gtk_menu_item_new_with_label("All"); 
gtk_menu_item_set_submenu(GTK_MENU_ITEM(palettes), palettesmenu);
gtk_menu_shell_append(GTK_MENU_SHELL(palettesmenu), palettes_all);
gtk_menu_shell_append(GTK_MENU_SHELL(menubar), palettes);




        drawing_area = gtk_drawing_area_new();
        gtk_widget_set_size_request(GTK_WIDGET(drawing_area), 700, 300);
    	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 0);
        gtk_widget_show(menubar);
        gtk_widget_show(drawing_area);
        gtk_widget_show(vbox);

        g_signal_connect(drawing_area, "expose_event", G_CALLBACK (expose_event), NULL);
        g_signal_connect(drawing_area, "configure_event", G_CALLBACK (configure_event), NULL);
        g_signal_connect(drawing_area, "motion_notify_event", G_CALLBACK(motion_notify_event), NULL);
        g_signal_connect(drawing_area, "button_press_event", G_CALLBACK(button_press_event), NULL);
        gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK | GDK_LEAVE_NOTIFY_MASK
			                                                  | GDK_BUTTON_PRESS_MASK
			                                                  | GDK_POINTER_MOTION_MASK
			                                                  | GDK_POINTER_MOTION_HINT_MASK);

    	gtk_signal_connect_object(GTK_OBJECT(palettes_all),"activate",GTK_SIGNAL_FUNC(makepalette),NULL);

        if (argc > 1)
        {
            if (!main_nb.open(argv[1]))
            {
                std::cout << "could not load " << argv[1] << std::endl;
            }
        }

        gtk_widget_show_all(window);
        gtk_main();
    }
}



void clipboard_get_data_append(std::string&s)
{
    gchar *t = gtk_clipboard_wait_for_text (gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
    if (t)
    {
        printf("on clipboard: %s\n", t);
        s.append(t);
        g_free(t);
    }
}

void clipboard_set_data(const char*s, size_t len)
{
    gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), s, len);
    gtk_clipboard_store(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
}
