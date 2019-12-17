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

GtkWidget* palette_window;
GtkWidget* palette_drawing_area;
GdkPixbuf* palette_pixbuf = NULL;
glimage palette_image;

GtkIMContext* glb_im_context;
GtkWidget* glb_window;
GtkWidget* drawing_area;
GdkPixbuf* pixbuf = NULL;
glimage glb_image;
GdkRectangle glb_area = {20, 20, 20, 20};

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
    glb_image.pixel_width = gdk_pixbuf_get_width(pixbuf);
    glb_image.pixel_height = gdk_pixbuf_get_height(pixbuf);
    glb_image.rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    glb_image.pixels = gdk_pixbuf_get_pixels(pixbuf);
    glb_image.units_per_pixel = UNITS_PER_PIXEL;
    glb_image.sizex = glb_image.units_per_pixel * glb_image.pixel_width;
    glb_image.sizey = glb_image.units_per_pixel * glb_image.pixel_height;
    main_nb.draw_bitmap();

    aftransform T;
    main_nb.root->get_cursor(&T);
    glb_area.x = (T.orig_x + main_nb.offx)/UNITS_PER_PIXEL;
    glb_area.y = (T.orig_y + main_nb.offy)/UNITS_PER_PIXEL - 10;

//std::cout << "glb_area: " << glb_area.x << ", " << glb_area.y << std::endl;

    main_nb.io_mutex.unlock();
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
            main_nb.mouse_doubleclick(UNITS_PER_PIXEL*event->x - main_nb.offx,
                                      UNITS_PER_PIXEL*event->y - main_nb.offy);
        }
        else
        {
            main_nb.mouse_click(UNITS_PER_PIXEL*event->x - main_nb.offx,
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

static gboolean focus_callback(GtkWidget* gtk_window, GdkEvent* event, GtkIMContext* im_context)
{
  gtk_widget_grab_focus(gtk_window);
  gtk_im_context_focus_in(im_context);
  gtk_im_context_reset(im_context);


    GdkRectangle rect;
    rect.x = 50;
    rect.y = 50;
    rect.width = 10;
    rect.height = 10;
//    gtk_im_context_set_cursor_location(im_context, &rect);
  return TRUE;
}

static gboolean blur_callback(GtkWidget* widget, GdkEvent* event, GtkIMContext* im_context)
{
  gtk_im_context_focus_out(im_context);
  return TRUE;
}

static gboolean commit_callback(GtkWidget * widget, gchar* str, gpointer user_data)
{
    char16_t c;
    const unsigned char* s = (const unsigned char*) str;
    main_nb.io_mutex.lock();
    while (*s)
    {
        s += readonechar16(c, s);
        main_nb.key_insert_char(c);
    }
    main_nb.io_mutex.unlock();
    draw_pixbuf(glb_window);
    return FALSE;
}

static gboolean preedit_changed_callback(GtkWidget * widget, GdkEventKey * event, gpointer user_data)
{
printf("preedit_changed_callback called\n");

    GdkRectangle rect;
    rect.x = 50;
    rect.y = 50;
    rect.width = 10;
    rect.height = 10;
//    gtk_im_context_set_cursor_location(glb_im_context, &rect);


    return FALSE;
}

static gboolean retrieve_surrounding_callback(GtkWidget * widget, GdkEventKey * event, gpointer user_data)
{
printf("retrieve_surrounding_callback called\n");
    GdkRectangle rect;
    rect.x = 50;
    rect.y = 50;
    rect.width = 10;
    rect.height = 10;
//    gtk_im_context_set_cursor_location(glb_im_context, &rect);

    return FALSE;
}

static gboolean delete_surrounding_callback(GtkWidget * widget, GdkEventKey * event, gpointer user_data)
{
printf("delete_surrounding_callback called\n");
    return FALSE;
}



static gboolean signal_key_release(GtkWidget *widget, GdkEventKey *event, GtkIMContext *im_context)
{
//    printf("signal_key_release called\n");

//    printf("calling gtk_im_context_filter_keypress(release)\n");
    if (gtk_im_context_filter_keypress(im_context, event))
    {
//        printf("gtk_im_context_filter_keypress(release) returned TRUE\n");
        return TRUE;
    }
//    g_print ("key_release:keyval=%02X\n", event->keyval);
    return TRUE;
}


static gboolean signal_key_press(GtkWidget *widget, GdkEventKey *event, GtkIMContext *im_context)
{
    gint n;
    GdkRectangle* area;

    if (gtk_im_context_filter_keypress(im_context, event))
        return TRUE;
    area = (GdkRectangle*) g_object_get_data(G_OBJECT(widget), "user_data");
    n = (event->state & GDK_SHIFT_MASK) == 0 ? 1 : 10;
    switch (event->keyval)
    {
        case GDK_Home:      area->x = area->y = 0; area->height = 10; break;
        case GDK_Left:      area->x -= n; break;
        case GDK_Right:     area->x += n; break;
        case GDK_Up:        area->y -= n; break;
        case GDK_Down:      area->y += n; break;
        case GDK_Page_Up:   area->height += n; break;
        case GDK_Page_Down: area->height = MAX (area->height - n, 1); break;
        default:
            g_print ("key_press:keyval=%02X\n", event->keyval);
            return TRUE;
    }
    gtk_im_context_set_cursor_location(im_context, area);
    g_print ("key_press:keyval=%02X,cursor=%d,%d,%d\n", event->keyval, area->x ,area->y, area->height);
    return TRUE;
}

gboolean on_key_press(GtkWidget * widget, GdkEventKey * event, GtkIMContext *im_context)
{
//printf("key event: keyval = 0x%08x, state = 0x%08x\n",event->keyval,event->state);

//printf("calling gtk_im_context_filter_keypress\n");
    if (gtk_im_context_filter_keypress(im_context, event))
    {
//        printf("gtk_im_context_filter_keypress(press) returned TRUE\n");
        return TRUE;
    }

    bool shift = event->state & GDK_SHIFT_MASK;
    bool ctrl = event->state & GDK_CONTROL_MASK;
    bool alt = event->state & GDK_MOD1_MASK;

    main_nb.io_mutex.lock();

    switch (event->keyval)
    {

    case GDK_KEY_KP_Add:
        ctrl ? main_nb.zoom_in()
             : main_nb.key_insert_char('+');
        break;
    case GDK_KEY_KP_Subtract:
        ctrl ? main_nb.zoom_out()
             : main_nb.key_insert_char('-');
        break;
    case GDK_KEY_Left:
           ctrl ? main_nb.key_ctrlleft()
                : alt ? main_nb.key_altleft()
                      : main_nb.key_move(shift ? movearg_ShiftLeft : movearg_Left);
        break;
    case GDK_KEY_Right:
           ctrl ? main_nb.key_ctrlright()
                : alt ? main_nb.key_altright()
                      : main_nb.key_move(shift ? movearg_ShiftRight : movearg_Right);
        break;
    case GDK_KEY_Up:
           ctrl ? main_nb.key_ctrlup()
                : alt ? main_nb.key_altup()
                      : main_nb.key_move(shift ? movearg_ShiftUp : movearg_Up);
        break;
    case GDK_KEY_Down:
           ctrl ? main_nb.key_ctrldown()
                : alt ? main_nb.key_altdown()
                      : main_nb.key_move(shift ? movearg_ShiftDown : movearg_Down);
        break;
    case GDK_KEY_Page_Up:
        main_nb.key_pageup();
        break;
    case GDK_KEY_Page_Down:
        main_nb.key_pagedown();
        break;
    case GDK_KEY_period:
        main_nb.key_insert_char('.');
        break;
    case GDK_KEY_underscore:
        main_nb.key_insert_char('_');
        break;
    case GDK_KEY_question:
        main_nb.key_insert_char('?');
        break;
    case GDK_KEY_quoteleft:
        main_nb.key_insert_char('`');
        break;
    case GDK_KEY_exclam:
        main_nb.key_insert_char('!');
        break;
    case GDK_KEY_at:
        main_nb.key_insert_char('@');
        break;
    case GDK_KEY_numbersign:
        main_nb.key_insert_char('#');
        break;
    case GDK_KEY_dollar:
        main_nb.key_insert_char('$');
        break;
    case GDK_KEY_percent:
        main_nb.key_insert_char('%');
        break;
    case GDK_KEY_asciicircum:
        main_nb.key_insert_char('^');
        break;
    case GDK_KEY_ampersand:
        main_nb.key_insert_char('&');
        break;
    case GDK_KEY_asterisk:
        main_nb.key_insert_char('*');
        break;
    case GDK_KEY_asciitilde:
        main_nb.key_insert_char('~');
        break;
    case GDK_KEY_plus:
        main_nb.key_insert_char('+');
        break;
    case GDK_KEY_slash:
        ctrl ? main_nb.key_insert(insertarg_Fraction)
             : main_nb.key_insert_char('/');
        break;
    case GDK_KEY_minus:
        ctrl ? main_nb.key_insert(insertarg_Subscript)
             : main_nb.key_insert_char('-');
        break;
    case GDK_KEY_Escape:
        main_nb.key_insert_char(CHAR_AliasDelimiter);
        break;
    case GDK_KEY_backslash:
        main_nb.key_insert_char('\\');
        break;
    case GDK_KEY_equal:
        main_nb.key_insert_char('=');
        break;
    case GDK_KEY_bracketleft:
        main_nb.key_insert_char('[');
        break;
    case GDK_KEY_bracketright:
        main_nb.key_insert_char(']');
        break;
    case GDK_KEY_braceleft:
        main_nb.key_insert_char('{');
        break;
    case GDK_KEY_braceright:
        main_nb.key_insert_char('}');
        break;
    case GDK_KEY_parenleft:
        main_nb.key_insert_char('(');
        break;
    case GDK_KEY_parenright:
        main_nb.key_insert_char(')');
        break;
    case GDK_KEY_quotedbl:
        main_nb.key_insert_char('"');
        break;
    case GDK_KEY_apostrophe:
        main_nb.key_insert_char('\'');
        break;
    case GDK_KEY_bar:
        main_nb.key_insert_char('|');
        break;
    case GDK_KEY_Delete:
        main_nb.key_remove(removearg_Right);
        break;
    case GDK_KEY_BackSpace:
        main_nb.key_remove(removearg_Left);
        break;
    case GDK_KEY_Return:
        ctrl ? main_nb.key_insert(insertarg_)
             : shift ? main_nb.key_shiftenter()
                     : main_nb.key_insert(insertarg_Newline);
        break;
    case GDK_KEY_comma:
        ctrl ? main_nb.key_insert(insertarg_)
             : main_nb.key_insert_char(',');
        break;
    case GDK_KEY_colon:
        main_nb.key_insert_char(':');
        break;
    case GDK_KEY_Home:
        main_nb.key_move(movearg_Home);
        break;
    case GDK_KEY_End:
        main_nb.key_move(movearg_End);
        break;
    case GDK_KEY_semicolon:
        main_nb.key_insert_char(';');
        break;
    case GDK_KEY_greater:
        main_nb.key_insert_char('>');
        break;
    case GDK_KEY_less:
        main_nb.key_insert_char('<');
        break;
    case GDK_KEY_0:
        alt ? main_nb.key_makecell(cellt_BOLDTEXT)
            : main_nb.key_insert_char('0');
        break;
    case GDK_KEY_1:
        alt ? main_nb.key_makecell(cellt_TITLE)
            : main_nb.key_insert_char('1');
        break;
    case GDK_KEY_2:
        alt ? main_nb.key_makecell(cellt_SECTION)
            : ctrl ? main_nb.key_insert(insertarg_Sqrt)
                   : main_nb.key_insert_char('2');
        break;
    case GDK_KEY_3:
        alt ? main_nb.key_makecell(cellt_SUBSECTION)
            : main_nb.key_insert_char('3');
        break;
    case GDK_KEY_4:
        alt ? main_nb.key_makecell(cellt_SUBSUBSECTION)
            : ctrl ? main_nb.key_insert(insertarg_Underscript)
                   : main_nb.key_insert_char('4');
        break;
    case GDK_KEY_5:
        alt ? main_nb.key_makecell(cellt_TEXT)
            : ctrl ? main_nb.key_move(movearg_Switch)
                   : main_nb.key_insert_char('5');
        break;
    case GDK_KEY_6:
        alt ? main_nb.key_makecell(cellt_MESSAGE)
            : ctrl ? main_nb.key_insert(insertarg_Superscript)
                   : main_nb.key_insert_char('6');
        break;
    case GDK_KEY_7:
        alt ? main_nb.key_makecell(cellt_PRINT)
            : ctrl ? main_nb.key_insert(insertarg_Overscript)
                   : main_nb.key_insert_char('7');
        break;
    case GDK_KEY_8:
        alt ? main_nb.key_makecell(cellt_OUTPUT)
            : main_nb.key_insert_char('8');
        break;
    case GDK_KEY_9:
        alt ? main_nb.key_makecell(cellt_INPUT)
            : main_nb.key_insert_char('9');
        break;
    case GDK_KEY_space:
        if (ctrl)
        {
        }
        else
        {
            main_nb.key_insert_char(' ');
        }
        break;

    /* capture save and open specially */
    case GDK_KEY_S: case GDK_KEY_s:
        ctrl ? (shift ? _handle_saveas(main_nb) : _handle_save(main_nb)) :  main_nb.key_insert_char(shift ? 'S' : 's');
        break;
    case GDK_KEY_O: case GDK_KEY_o:
        ctrl ? _handle_open(main_nb) : main_nb.key_insert_char(shift ? 'O' : 'o');
        break;

    default:
        if (GDK_KEY_a <= event->keyval && event->keyval <= GDK_KEY_z)
        {
            ctrl ? main_nb.key_ctrl_char(event->keyval - GDK_KEY_a + 'a')
                 : main_nb.key_insert_char(event->keyval - GDK_KEY_a + 'a');
            break;
        }
        else if (GDK_KEY_A <= event->keyval && event->keyval <= GDK_KEY_Z)
        {
            ctrl ? main_nb.key_ctrl_char(event->keyval - GDK_KEY_A + 'A')
                 : main_nb.key_insert_char(event->keyval - GDK_KEY_A + 'A');
            break;
        }
    }

    main_nb.io_mutex.unlock();
    draw_pixbuf(widget);
    gtk_im_context_set_cursor_location(im_context, &glb_area);

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
            else if (ehas_head_sym_length(e.get(), gs.sym_sOutputNamePacket.get(), 1))
            {
                main_nb.io_mutex.lock();
                main_nb.out_name.reset(e.copychild(1));
                main_nb.io_mutex.unlock();
            }
            else if (ehas_head_sym_length(e.get(), gs.sym_sInputNamePacket.get(), 1))
            {
                main_nb.io_mutex.lock();
                main_nb.in_name.reset(e.copychild(1));
                main_nb.io_mutex.unlock();
            }
            else if (ehas_head_sym_length(e.get(), gs.sym_sReturnTextPacket.get(), 1))
            {
                boxbase * row = boxbase_convert_from_ex(e.child(1));
                if (row == nullptr || row->get_type() != BNTYPE_ROW)
                {
                    std::cerr << "ERROR gui: could not convert " << ex_tostring_full(e.get()) << std::endl;
                    if (row != nullptr)
                        delete row;
                }
                else
                {
                    cellbox* cell = new cellbox(dynamic_cast<rowbox*>(row), cellt_OUTPUT);
                    if (main_nb.out_name.get() != nullptr)
                    {
                        boxbase * label = boxbase_convert_from_ex(main_nb.out_name.get());
                        if (label == nullptr || label->get_type() != BNTYPE_ROW)
                        {
                            std::cerr << "ERROR gui: could not convert " << ex_tostring_full(main_nb.out_name.get()) << std::endl;
                            if (label != nullptr)
                                delete label;
                        }
                        else
                        {
                            cell->label.cbox = dynamic_cast<rowbox*>(label);
                        }
                    }
                    main_nb.io_mutex.lock();
                    main_nb.print_cell(cell);
                    main_nb.io_mutex.unlock();
                    g_idle_add(myfunc, NULL);
                }
            }
            else if (ehas_head_sym_length(e.get(), gs.sym_sMessagePacket.get(), 2))
            {
                e.replacechild(0, gs.sym_sList.copy());
                e.reset(emake_node(gs.sym_sRowBox.copy(), e.release()));
                boxbase * row = boxbase_convert_from_ex(e.get());
                if (row == nullptr || row->get_type() != BNTYPE_ROW)
                {
                    std::cerr << "ERROR gui: could not convert " << ex_tostring_full(e.get()) << std::endl;
                    if (row != nullptr)
                        delete row;
                }
                else
                {
                    cellbox* cell = new cellbox(dynamic_cast<rowbox*>(row), cellt_MESSAGE);
                    main_nb.io_mutex.lock();
                    main_nb.print_cell(cell);
                    main_nb.io_mutex.unlock();
                    g_idle_add(myfunc, NULL);
                }
            }
            else if (ehas_head_sym_length(e.get(), gs.sym_sTextPacket.get(), 1))
            {
                e.replacechild(0, gs.sym_sList.copy());
                e.reset(emake_node(gs.sym_sRowBox.copy(), e.release()));
                boxbase * row = boxbase_convert_from_ex(e.get());
                if (row == nullptr || row->get_type() != BNTYPE_ROW)
                {
                    std::cerr << "ERROR gui: could not convert " << ex_tostring_full(e.get()) << std::endl;
                    if (row != nullptr)
                        delete row;
                }
                else
                {
                    cellbox* cell = new cellbox(dynamic_cast<rowbox*>(row), cellt_PRINT);
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
//std::cout << "<gui reader>: CMD_SYNTAX state: " << int(t) << "  s: " << s << std::endl;
        }
        else
        {
            std::cerr << "<!gui reader>: received unknown cmd " << (int)(t) << std::endl;
        }
    }

    std::cout << "gui: read thread exiting" << std::endl;
    return NULL;
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


/*******************************************************


*********************************************************/











static void signal_realize(GtkWidget *widget, GtkIMContext *im_context)
{
    gtk_im_context_set_client_window (im_context, widget->window);
}

static void signal_unrealize(GtkWidget *widget, GtkIMContext *im_context)
{
    gtk_im_context_set_client_window (im_context, NULL);
}


static gboolean signal_focus_in(GtkWidget *widget, GdkEventFocus *event,
                                                    GtkIMContext *im_context)
{
    GTK_WIDGET_SET_FLAGS(widget, GTK_HAS_FOCUS);
    gtk_im_context_focus_in(im_context);
    return FALSE;
}


static gboolean signal_focus_out(GtkWidget *widget, GdkEventFocus *event,
                                                    GtkIMContext *im_context)
{
    GTK_WIDGET_UNSET_FLAGS(widget, GTK_HAS_FOCUS);
    gtk_im_context_focus_out(im_context);
    return FALSE;
}


gboolean signal_expose(GtkWidget *widget, GdkEventExpose *event, GtkIMContext *im_context)
{
    gchar *str;
    gint cursor_pos;
    GdkGC *gc;
    PangoAttrList *attrs;
    PangoLayout *layout;
    GdkColor color[2] = {{0, 0x0000, 0x0000, 0x0000},
                         {0, 0xffff, 0xffff, 0xffff}};

    gtk_im_context_get_preedit_string(im_context, &str, &attrs, &cursor_pos);
    layout = gtk_widget_create_pango_layout((GtkWidget*)g_object_get_data((GObject*)G_OBJECT (gtk_widget_get_parent (widget)), "user_data"), str);
    pango_layout_set_attributes(layout, attrs);

    gc = gdk_gc_new(widget->window);
    gdk_color_alloc(gdk_colormap_get_system (), color);
    gdk_color_alloc(gdk_colormap_get_system (), color + 1);

    gdk_gc_set_foreground(gc, color + 1);
    gdk_draw_rectangle(widget->window, gc, TRUE, event->area.x, event->area.y,
                                        event->area.width, event->area.height);

    gdk_gc_set_foreground(gc, color);
    gdk_gc_set_background(gc, color + 1);
    gdk_draw_layout(widget->window, gc, 0, 0, layout);

    gdk_gc_unref(gc);
    g_free(str);
    pango_attr_list_unref(attrs);
    g_object_unref(layout);
    return TRUE;
}

static void signal_commit(GtkIMContext *im_context, gchar* str, gpointer user_data)
{
    char16_t c;
    const unsigned char* s = (const unsigned char*) str;
    main_nb.io_mutex.lock();
    while (*s)
    {
        s += readonechar16(c, s);
        main_nb.key_insert_char(c);
    }
    main_nb.io_mutex.unlock();
    draw_pixbuf(glb_window);
}

static void signal_preedit_changed(GtkIMContext *im_context, GtkWidget *drawing)
{
    int width, height;
    gchar *str;
    gint x, y;
    PangoAttrList *attrs;
    PangoLayout *layout;
    GtkWidget *preedit,*window;
    GdkRectangle rc;

    preedit = gtk_widget_get_parent (drawing);
    window = (GtkWidget*) g_object_get_data (G_OBJECT (preedit), "user_data");
    gtk_im_context_get_preedit_string (im_context, &str, &attrs, NULL);
    if (strlen(str) > 0)
    {
        layout = gtk_widget_create_pango_layout(window, str);
        pango_layout_set_attributes(layout, attrs);
        pango_layout_get_pixel_size(layout, &width, &height);
        g_object_unref(layout);
        gdk_window_get_origin(window->window, &x, &y);
        gtk_window_move(GTK_WINDOW(preedit), x + glb_area.x, y + glb_area.y);
        gtk_window_resize(GTK_WINDOW(preedit), width, height);
        gtk_widget_show(preedit);
        gtk_widget_queue_draw_area(preedit, 0, 0, width, height);
    }
    else
    {
        gtk_widget_hide(preedit);
    }
    g_free(str);
    pango_attr_list_unref(attrs);
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
        kernel_path.append("kernel");
        close(finput[WRITE_END]);
        close(foutput[READ_END]); 
        char outstr[20], instr[20];
        sprintf(outstr, "%d", foutput[WRITE_END]);
        sprintf(instr, "%d", finput[READ_END]);
//std::cout << "kernel_path: " << kernel_path << std::endl;
        execlp(kernel_path.c_str(), kernel_path.c_str(), outstr, instr, NULL);
        return 0;
    }

    close(finput[READ_END]);
    close(foutput[WRITE_END]);
    fp_in = fdopen(finput[WRITE_END], "wb");
    fp_out = fdopen(foutput[READ_END], "rb");
    fp_out_type = FP_OUT_NONE;

    gui_read_thread = g_thread_new("asdf", gui_read_loop, NULL);

    glyph_init();

    GtkIMContext* im_context;
    GtkWidget* preedit, * drawing;

    setlocale(LC_ALL, "");
    gtk_set_locale();
    gtk_init(&argc, &argv);

    im_context = gtk_im_multicontext_new();
    preedit = gtk_window_new(GTK_WINDOW_POPUP);
    drawing = gtk_drawing_area_new();
    glb_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(glb_window), "castle");

    gtk_window_move(GTK_WINDOW(preedit), 0, 0);
    gtk_container_add(GTK_CONTAINER(preedit), drawing);
    g_object_set_data(G_OBJECT(preedit), "user_data", glb_window);

    g_signal_connect(G_OBJECT(drawing), "expose-event", G_CALLBACK(signal_expose), im_context);
    gtk_widget_show(drawing);

    gtk_im_context_set_cursor_location(im_context, &glb_area);
    g_signal_connect(im_context, "commit", G_CALLBACK(signal_commit), NULL);
    g_signal_connect(im_context, "preedit_changed", G_CALLBACK(signal_preedit_changed), drawing);

    GtkWidget * vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(glb_window), vbox);
    gtk_widget_show(vbox);

    g_signal_connect(G_OBJECT(glb_window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(glb_window), "key-press-event", G_CALLBACK(on_key_press), im_context);
    g_signal_connect(G_OBJECT(glb_window), "key-release-event", G_CALLBACK(signal_key_release), im_context);
    g_signal_connect(G_OBJECT(glb_window), "realize", G_CALLBACK(signal_realize), im_context);
    g_signal_connect(G_OBJECT(glb_window), "unrealize", G_CALLBACK(signal_unrealize), im_context);
    g_signal_connect(G_OBJECT(glb_window), "focus-in-event", G_CALLBACK(signal_focus_in), im_context);
    g_signal_connect(G_OBJECT(glb_window), "focus-out-event", G_CALLBACK(signal_focus_out), im_context);


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
    gtk_widget_set_size_request(GTK_WIDGET(drawing_area), 600, 500);
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 0);
    gtk_widget_show(menubar);
    gtk_widget_show(drawing_area);
    gtk_widget_show(vbox);

    g_signal_connect(drawing_area, "expose-event", G_CALLBACK (expose_event), NULL);
    g_signal_connect(drawing_area, "configure-event", G_CALLBACK (configure_event), NULL);
    g_signal_connect(drawing_area, "motion-notify-event", G_CALLBACK(motion_notify_event), NULL);
    g_signal_connect(drawing_area, "button-press-event", G_CALLBACK(button_press_event), NULL);
    gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK | GDK_LEAVE_NOTIFY_MASK
		                                                  | GDK_BUTTON_PRESS_MASK
		                                                  | GDK_POINTER_MOTION_MASK
		                                                  | GDK_POINTER_MOTION_HINT_MASK);

    if (argc > 1)
    {
        if (!main_nb.open(argv[1]))
        {
            std::cout << "could not load " << argv[1] << std::endl;
        }
    }

    gtk_widget_show_all(glb_window);
    gtk_window_set_policy(GTK_WINDOW(glb_window), TRUE, TRUE, TRUE);

    gtk_main();

    g_object_unref(im_context);

    return 0;
}
