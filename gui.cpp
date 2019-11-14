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
#include "program.h"
#include "svg.h"

GdkPixbuf *pixbuf = NULL;
FILE * fp_in, * fp_out;
int fp_out_type;

std::vector<void*> exs_in_use;
globalstate gs;
notebook main_nb;

glimage glb_image;
svgGlyphSet glb_glyphs;
svgCharSet glb_chars;
std::vector<svgFont> glb_fonts;
svgFont glb_dingbat("Dingbat", 0.625, 2.0, 1.0);

void glyph_init()
{
    glb_fonts.emplace_back("SerifBold", 0.5, 25./16, 25./32);
    glb_fonts.emplace_back("SerifReg", 0.5, 25./16, 25./32);
    glb_fonts.emplace_back("SansReg", 0.5, 25./16, 25./32);
    glb_fonts.emplace_back("SansBold", 0.5, 25./16, 25./32);


    svgGlyph g("1,0,0,1,0,0 M197 575 h 829 l 36.5 -12 l 12.5 -35 v-125 l -15.5 -38 l -33.5 -17 h-829 l -31.5 15 l -11.5 40 v 125 l 12 34 l 31 13 z");
    std::cout << "g: " << std::endl;
    std::cout << g.tostring() << std::endl;

    return;
}


static void draw_pixbuf(GtkWidget* widget)
{
    if (pixbuf == NULL)
        return;

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
/*
    for (int32_t j = 0; j < glb_image.pixel_height; j++)
    {
        for (int32_t i = 0; i < glb_image.pixel_width; i++)
        {
            guchar* p = glb_image.pixels + glb_image.rowstride*(j) + 4*(i);
            p[0] = 255;
            p[1] = 255;
            p[2] = 255;
            p[3] = 255;
        }
    }
*/


std::memset(glb_image.pixels, 255, glb_image.pixel_height*glb_image.rowstride);
    

uint64_t time3 = GetMS();


    main_nb.draw_bitmap();
uint64_t time4 = GetMS();

printf("setup time: %d\n",time2-time1);
printf(" fill time: %d\n",time3-time2);
printf(" draw time: %d\n",time4-time3);
printf("total time: %d\n",time4-time1);
printf("characters draw: %d\n", chardrawcount);

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

/*
    glb_image.pixel_width = Bitmap_x;
    glb_image.pixel_height = Bitmap_y;
*/

//    bitmap_delete(image);
//    image = bitmap_create(Bitmap_x, Bitmap_y);
//    main_nb.draw_bitmap(image);

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

static gboolean button_press_event(GtkWidget* widget, GdkEventButton* event) {

    if (event->button == 1) {
        main_nb.handle_click(UNITS_PER_PIXEL*event->x - main_nb.offx, UNITS_PER_PIXEL*event->y - main_nb.offy);
        draw_pixbuf(widget);
    }

    return TRUE;
}

static gboolean motion_notify_event(GtkWidget* widget, GdkEventMotion* event) {
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
printf("key event: keyval = 0x%08x, state = 0x%08x\n",event->keyval,event->state);

    bool shift = event->state & GDK_SHIFT_MASK;
    bool ctrl = event->state & GDK_CONTROL_MASK;
    bool alt = event->state & GDK_MOD1_MASK;

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
        return FALSE; 
    }

    draw_pixbuf(widget);

    return FALSE; 
}







void quit() {
    exit(0);
}

int main(int argc, char * argv[])
{

//    image = bitmap_create(256, 256);
//    main_nb.draw_bitmap(image);

    fp_out = stdout;
    fp_out_type = FP_OUT_TEXT;
    glyph_init();

    GtkWidget * window;
    GtkWidget * drawing_area;
    GtkWidget * vbox;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_name(window, "Test Input");

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    gtk_widget_show(vbox);

    g_signal_connect(window, "destroy", G_CALLBACK(quit), NULL);
    g_signal_connect(G_OBJECT (window), "key_press_event", G_CALLBACK (on_key_press), NULL);

    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(GTK_WIDGET (drawing_area), 1200, 600);
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 0);
    gtk_widget_show(drawing_area);

    g_signal_connect(drawing_area, "expose_event", G_CALLBACK (expose_event), NULL);
    g_signal_connect(drawing_area, "configure_event", G_CALLBACK (configure_event), NULL);
    g_signal_connect(drawing_area, "motion_notify_event", G_CALLBACK(motion_notify_event), NULL);
    g_signal_connect(drawing_area, "button_press_event", G_CALLBACK(button_press_event), NULL);
    gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK | GDK_LEAVE_NOTIFY_MASK
			                                              | GDK_BUTTON_PRESS_MASK
			                                              | GDK_POINTER_MOTION_MASK
			                                              | GDK_POINTER_MOTION_HINT_MASK);

    if (argc > 1) {
        if (!main_nb.open(argv[1])) {
            std::cout << "could not load " << argv[1] << std::endl;
        }
    }

    gtk_widget_show(window);
    gtk_main();

    return 0;
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
