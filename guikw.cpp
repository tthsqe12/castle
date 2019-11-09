#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600

#include <windows.h>
#include <Commctrl.h>
#include <algorithm>
#include "intrin.h"

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<iostream>
#include<string>
#include<memory>
#include<vector>
#include<stack>
#include<list>
#include<map>
#include<set>
#include<cmath>

#include "ex.h"
#include "graphics.h"
#include "notebook.h"
#include "serialize.h"
#include "box_convert.h"

FILE * fp_in, * fp_out;
int fp_out_type;

std::vector<void*> exs_in_use;
globalstate gs;

PAINTSTRUCT ps;
HDC hdc, hMemDC;
HANDLE hBitmap;
//HWND  hStatusWindow;
HWND  hMainWindow;
RECT    rect;
RECT    rc;
HINSTANCE  hInst;
BITMAPINFO bmi;
//int statusWindowID = 123;
void * ppvBits;

int Bitmap_x, Bitmap_y;
int Cursor_x, Cursor_y;
int Mouse_x, Mouse_y;

//DWORD StatusWindowHeight;

notebook main_nb;
glimage glb_image;

int Zoom = 1;
char OpenFileString[1000];






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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    DWORD x, *pb;
    const char shiftnum[10] = {')','!','@','#','$','%','^','&','*','('};
    bool ShiftPressed, CtrlPressed, AltPressed;

    switch(msg) {

    case WM_MOUSEMOVE:
        Mouse_x = LOWORD(lParam);
        Mouse_y = HIWORD(lParam);
        break;

    case WM_LBUTTONDOWN:
        main_nb.handle_click(Mouse_x/Zoom - main_nb.offx, Mouse_y/Zoom - main_nb.offy);
        goto wm_paint;
        
    case WM_KEYDOWN:
        AltPressed = GetKeyState(VK_MENU) & 0x0100;
        ShiftPressed = GetKeyState(VK_SHIFT) & 0x0100;
        CtrlPressed = GetKeyState(VK_CONTROL) & 0x0100;

        main_nb.io_mutex.lock();

        x = LOWORD(wParam);
//printf("x: %d\n",x);
        if ('A' <= x && x <= 'Z') {
            if (CtrlPressed) {
                if (x == 'T') {
                    main_nb.insert_textbox();
                } else if (x == 'C') {
                    main_nb.key_copy();
                } else if (x == 'V') {
                    main_nb.key_paste();

                } else if (x == 'S') {
                    if (ShiftPressed || main_nb.filestring.empty()) {
                        OPENFILENAME of;
                        memset(&of, 0, sizeof(OPENFILENAME));
                        of.lStructSize = sizeof(OPENFILENAME);
                        of.lpstrFile = OpenFileString;
                        of.nMaxFile = 1000;
                        if (GetSaveFileNameA(&of)) {
                            main_nb.save(OpenFileString);
                        }
                    } else {
                        main_nb.save(main_nb.filestring.c_str());
                    }

                } else if (x == 'O') {
                    OPENFILENAME of;
                    memset(&of, 0, sizeof(OPENFILENAME));
                    of.lStructSize = sizeof(OPENFILENAME);
                    of.lpstrFile = OpenFileString;
                    of.nMaxFile = 1000;
                    if (GetOpenFileNameA(&of)) {
                        if (!main_nb.open(OpenFileString)) {
                            MessageBoxA(hMainWindow, "file could not be loaded", "error", MB_OK);
                        }
                    }
                }
            } else {
                main_nb.insert_char(ShiftPressed ? x : x - 'A' + 'a');
            }
        } else if (x == VK_SPACE) {
            main_nb.insert_char(' ');
        } else if (x == VK_TAB) {
            main_nb.key_tab();
        } else if (x == 27) {
            main_nb.insert_char(CHAR_AliasDelimiter);
        } else if (x == 107) {
            CtrlPressed ? main_nb.zoom_in() : main_nb.insert_char('+');
        } else if (x == 109) {
            CtrlPressed ? main_nb.zoom_out() : main_nb.insert_char('-');
        } else if (x == 186) {
            CtrlPressed ? void(x) : main_nb.insert_char(ShiftPressed ? ':' : ';');
        } else if (x == 187) {
            CtrlPressed ? void(x) : main_nb.insert_char(ShiftPressed ? '+' : '=');
        } else if (x == 188) {
            CtrlPressed ? main_nb.insert_gridcol() : main_nb.insert_char(ShiftPressed ? '<' : ',');
        } else if (x == 189) {
            CtrlPressed ?main_nb.insert_subscript() : main_nb.insert_char(ShiftPressed ? '_' : '-');
        } else if (x == 190) {
            CtrlPressed ? void(x) : main_nb.insert_char(ShiftPressed ? '>' : '.');
        } else if (x == 191) {
            CtrlPressed ? main_nb.insert_fraction() : main_nb.insert_char(ShiftPressed ? '?' : '/');
        } else if (x == 192) {
            CtrlPressed ? void(x) : main_nb.insert_char(ShiftPressed ? '~' : '`');
        } else if (x == 219) {
            CtrlPressed ? void(x) : main_nb.insert_char(ShiftPressed ? '{' : '[');
        } else if (x == 220) {
            CtrlPressed ? void(x) : main_nb.insert_char(ShiftPressed ? '|' : '\\');
        } else if (x == 221) {
            CtrlPressed ? void(x) : main_nb.insert_char(ShiftPressed ? '}' : ']');
        } else if (x == 222) {
            CtrlPressed ? void(x) : main_nb.insert_char(ShiftPressed ? '"' : '\'');
        } else if ('0' <= x && x <= '9') {
            if (CtrlPressed) {
                if (ShiftPressed) {
                    if (x == '1') {
                        main_nb.key_makecell(CELLTYPE_TITLE);
                    } else if (x == '2') {
                        main_nb.key_makecell(CELLTYPE_SECTION);
                    } else if (x == '3') {
                        main_nb.key_makecell(CELLTYPE_SUBSECTION);
                    } else if (x == '4') {
                        main_nb.key_makecell(CELLTYPE_SUBSUBSECTION);
                    } else if (x == '5') {
                        main_nb.key_makecell(CELLTYPE_TEXT);
                    } else if (x == '6') {
                        main_nb.key_makecell(CELLTYPE_MESSAGE);
                    } else if (x == '7') {
                        main_nb.key_makecell(CELLTYPE_PRINT);
                    } else if (x == '8') {
                        main_nb.key_makecell(CELLTYPE_OUTPUT);
                    } else if (x == '9') {
                        main_nb.key_makecell(CELLTYPE_INPUT);
                    }
                } else {
                    if (x == '2') {
                        main_nb.insert_sqrt();
                    } else if (x == '4') {
                        main_nb.insert_underscript();
                    } else if (x == '5') {
                        main_nb.key_switch();
                    } else if (x == '6') {
                        main_nb.insert_superscript();
                    } else if (x == '7') {
                        main_nb.insert_overscript();
                    }
                }
            } else {
                main_nb.insert_char(ShiftPressed ? shiftnum[x - '0'] : x);
            }
        } else if (x == VK_LEFT) {
            ShiftPressed ? main_nb.key_shiftleft() : main_nb.key_left();
        } else if (x == VK_RIGHT) {
            ShiftPressed ? main_nb.key_shiftright() : main_nb.key_right();
        } else if (x == VK_UP) {
            ShiftPressed ? main_nb.key_shiftup() : main_nb.key_up();
        } else if (x == VK_DOWN) {
            ShiftPressed ? main_nb.key_shiftdown() : main_nb.key_down();
        } else if (x == VK_BACK) {
            main_nb.key_backspace();
        } else if (x == VK_RETURN) {
            CtrlPressed ? main_nb.insert_gridrow() : ShiftPressed ? main_nb.key_shiftenter() : main_nb.insert_newline();
        } else if (x == 33) {
            main_nb.key_pageup();
        } else if (x == 34) {
            main_nb.key_pagedown();
        } else if (x == 35) {
            main_nb.key_end();
        } else if (x == 36) {
            main_nb.key_home();
        } else if (x == 46) {
            main_nb.key_delete();
        } else if (x == VK_F2) {
            Zoom = (Zoom & 3) + 1;
            main_nb.io_mutex.unlock();
            goto wm_size;
        }
        main_nb.io_mutex.unlock();
        goto wm_paint;

    case WM_CREATE:
//        hStatusWindow = CreateWindowEx(0, STATUSCLASSNAME, "hello", WS_CHILD+WS_VISIBLE+SBS_SIZEGRIP, 0,0,0,0, hwnd, NULL, hInst, NULL);
//        GetWindowRect(hStatusWindow, &rc);
//        StatusWindowHeight = rc.bottom - rc.top;
//        book = notebook_create();

    case WM_SIZE: wm_size:
        GetClientRect(hwnd, &rect);
        if (rect.right == 0) {
            break;
        }
        Bitmap_x = std::max(int(rect.right/Zoom), 100);
        Bitmap_y = std::max(int(rect.bottom/Zoom), 100);
main_nb.io_mutex.lock();
        main_nb.resize(Bitmap_x, Bitmap_y);
main_nb.io_mutex.unlock();

//std::cout << "x: " << Bitmap_x << "  y: " << Bitmap_y << std::endl;

//        MoveWindow(hStatusWindow,0,0,0,0,TRUE);
        if (hBitmap != 0) {
            DeleteObject(hBitmap);
            hBitmap = 0;
        }
    case WM_PAINT: wm_paint:
        GetClientRect(hwnd, &rect);
        if (rect.right == 0) {
            break;
        }
        if (hBitmap == 0) {
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = Bitmap_x;
            bmi.bmiHeader.biHeight = -Bitmap_y;
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression, BI_RGB;
            bmi.bmiHeader.biSizeImage = 4*Bitmap_x*Bitmap_y + 16;
            bmi.bmiHeader.biXPelsPerMeter = 0;
            bmi.bmiHeader.biYPelsPerMeter = 0;
            bmi.bmiHeader.biClrUsed = 0;
            bmi.bmiHeader.biClrImportant = 0;
            hBitmap = CreateDIBSection(0, &bmi, DIB_RGB_COLORS, &ppvBits, 0, 0);
            //image->parray = (u32*)ppvBits;
            //image->sizex = Bitmap_x;
            //image->sizey = Bitmap_y;
            glb_image.pixel_width = Bitmap_x;
            glb_image.pixel_height = Bitmap_y;
            glb_image.rowstride = 4*Bitmap_x;
            glb_image.pixels = (uint8_t*)(ppvBits);
            glb_image.units_per_pixel = UNITS_PER_PIXEL;
            glb_image.sizex = glb_image.units_per_pixel * glb_image.pixel_width;
            glb_image.sizey = glb_image.units_per_pixel * glb_image.pixel_height;
        }

        if (hBitmap == 0) {
            break;
        }

        GetClientRect(hMainWindow, &rect);
//        rect.bottom = rect.bottom - StatusWindowHeight;
        InvalidateRect(hMainWindow, &rect, FALSE);
        hdc = BeginPaint(hMainWindow, &ps);
        hMemDC = CreateCompatibleDC(hdc);

//        notebook_draw_bitmap(image, book);

        memset(glb_image.pixels, 255, glb_image.pixel_height*glb_image.rowstride);
main_nb.io_mutex.lock();
        main_nb.draw_bitmap();
main_nb.io_mutex.unlock();



        SelectObject(hMemDC, hBitmap);
        if (Zoom > 1) {
            StretchBlt(hdc, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, rect.right/Zoom, rect.bottom/Zoom, SRCCOPY);
        } else {
            BitBlt(hdc, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, SRCCOPY);
        }
        DeleteDC(hMemDC);
        EndPaint(hMainWindow, &ps);
        break;
    case WM_CLOSE:
        //notebook_delete(book);
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}





#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <cstdio>

#define BUFSIZE 512






HANDLE hPipeG2K = INVALID_HANDLE_VALUE, hPipeK2G = INVALID_HANDLE_VALUE, hThread = NULL;
DWORD  dwThreadId = 0;
LPTSTR lpszPipenameG2K = TEXT("\\\\.\\pipe\\G2Knamedpipe"); 
LPTSTR lpszPipenameK2G = TEXT("\\\\.\\pipe\\K2Gnamedpipe"); 



DWORD WINAPI GuiReadThread(LPVOID lpvParam)
{
    Sleep(500);

    std::string s;
    while (1)
    {
        uint8_t t;
        unsigned char buffer[8];
        if (1 != fread(buffer, 1, 1, fp_out))
        {
std::cout << "<gui reader>: could not read" << std::endl;
            break;
        }
//std::cout << "<gui reader>: got " << int(buffer[0]) << std::endl;
        if (buffer[0] == CMD_EXPR)
        {
            uex e;
            int err = sread_ex(fp_out, e);
std::cout << "<gui reader>: CMD_EXPR err: " << err << "  e: " << ex_tostring_full(e.get()) << std::endl;
            if (err != 0)
            {
                std::cerr << "<!gui reader>: could not read CMD_EXPR - error " << err << std::endl;
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
                    std::cerr << "<!gui reader>: could not convert " << ex_tostring_full(e.get()) << std::endl;
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
                            std::cerr << "<!gui reader>: could not convert " << ex_tostring_full(main_nb.out_name.get()) << std::endl;
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
                    PostMessage(hMainWindow, WM_PAINT, 0, 0);
                }
            }
            else if (ehas_head_sym_length(e.get(), gs.symsMessagePacket.get(), 2))
            {
                e.replacechild(0, gs.symsList.copy());
                e.reset(emake_node(gs.symsRowBox.copy(), e.release()));
                boxnode * row = boxnode_convert_from_ex(e.get());
                if (row == nullptr || boxnode_type(row) != BNTYPE_ROW)
                {
                    std::cerr << "<!gui reader>: could not convert " << ex_tostring_full(e.get()) << std::endl;
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
                    PostMessage(hMainWindow, WM_PAINT, 0, 0);
                }
            }
            else if (ehas_head_sym_length(e.get(), gs.symsTextPacket.get(), 1))
            {
                e.replacechild(0, gs.symsList.copy());
                e.reset(emake_node(gs.symsRowBox.copy(), e.release()));
                boxnode * row = boxnode_convert_from_ex(e.get());
                if (row == nullptr || boxnode_type(row) != BNTYPE_ROW)
                {
                    std::cerr << "<!gui reader>: could not convert " << ex_tostring_full(e.get()) << std::endl;
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
                    PostMessage(hMainWindow, WM_PAINT, 0, 0);
                }
            }
        }
        else if (buffer[0] == CMD_SYNTAX)
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
            std::cerr << "<!gui reader>: received unknown cmd " << (int)(buffer[0]) << std::endl;
        }
    }

std::cout << "<gui reader>: exiting" << std::endl;

    fclose(fp_in);
    fclose(fp_out);
    return 0;
}


int start_kernel()
{
    hPipeG2K = CreateNamedPipe(lpszPipenameG2K, PIPE_ACCESS_OUTBOUND, 0, PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE, 0, NULL);
    if (hPipeG2K == INVALID_HANDLE_VALUE)
    {
        printf("<!gui>: CreateNamedPipe failed, GLE = %d\n", GetLastError()); 
        return 1;
    }

    hPipeK2G = CreateNamedPipe(lpszPipenameK2G, PIPE_ACCESS_INBOUND, 0, PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE, 0, NULL);
    if (hPipeK2G == INVALID_HANDLE_VALUE)
    {
        printf("<!gui>: CreateNamedPipe failed, GLE = %d\n", GetLastError()); 
        CloseHandle(hPipeG2K);
        return 1;
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
    if (!CreateProcess(NULL, "wkernel.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) 
    {
        printf( "<gui> CreateProcess failed, GLE = %d\n", GetLastError());
        CloseHandle(hPipeG2K);
        CloseHandle(hPipeK2G);
        return 1;
    }
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );


    printf("<gui>: awaiting client connection on %s\n", lpszPipenameG2K);
    if (!(ConnectNamedPipe(hPipeG2K, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED)))
    {
        printf("<!gui>: kernel could not connect to G2K\n");
        CloseHandle(hPipeG2K);
        CloseHandle(hPipeK2G);
        return 1;
    }

    printf("<gui>: awaiting client connection on %s\n", lpszPipenameK2G);
    if (!(ConnectNamedPipe(hPipeK2G, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED)))
    {
        printf("<!gui>: kernel could not connect to G2K\n");
        CloseHandle(hPipeG2K);
        CloseHandle(hPipeK2G);
        return 1;
    }

    int fd;
    fd = _open_osfhandle((intptr_t)hPipeG2K, _O_APPEND);
    if (fd == -1)
    {
        printf("_open_osfhandle failed, GLE = %d\n", GetLastError()); 
        CloseHandle(hPipeG2K);
        CloseHandle(hPipeK2G);
        return 1;
    }
    fp_in = _fdopen(fd,"a");
    if (fp_in == 0)
    {
        printf("_fdopen failed, GLE = %d\n", GetLastError());
        _close(fd);
        CloseHandle(hPipeK2G);
        return 1;        
    }

    fd = _open_osfhandle((intptr_t)hPipeK2G, _O_APPEND);
    if (fd == -1)
    {
        printf("_open_osfhandle failed, GLE = %d\n", GetLastError()); 
        fclose(fp_in);
        CloseHandle(hPipeK2G);
        return 1;
    }
    fp_out = _fdopen(fd,"r");
    if (fp_out == 0)
    {
        printf("_fdopen failed, GLE = %d\n", GetLastError());
        fclose(fp_in);
        _close(fd);
        return 1;        
    }

    hThread = CreateThread(NULL, 0, GuiReadThread, (LPVOID) 0, 0, &dwThreadId);
    if (hThread == NULL) 
    {
        printf("CreateThread failed, GLE = %d\n", GetLastError()); 
        fclose(fp_in);
        fclose(fp_out);
        return 1;
    }
    else
    {
        CloseHandle(hThread);
    }

    printf("<gui>: started kernel good\n");
    return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    hInst = hInstance;

    start_kernel();
    glyph_init();


    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "casWindowClass";
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
    if(!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "casWindowClass", "cas", WS_VISIBLE+WS_OVERLAPPEDWINDOW, 900, 20, 700, 1000, NULL, NULL, hInstance, NULL);
    hMainWindow = hwnd;
    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    return Msg.wParam;
}


void clipboard_get_data_append(std::string&s) {
    if (IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(hMainWindow)) {
        HANDLE hglbCopy = GetClipboardData(CF_TEXT);
        if (hglbCopy) {
            char* t = (char*) GlobalLock(hglbCopy);
            if (t) {
                printf("on clipboard: %s\n", t);
                s.append(t);
                GlobalUnlock(hglbCopy);
            }
        }
        CloseClipboard();
    }
}


void clipboard_set_data(const char*s, size_t len) {
    if (OpenClipboard(hMainWindow)) {
        HANDLE hglbCopy = GlobalAlloc(GHND|GMEM_SHARE, len+1);
        if (hglbCopy) {
            char* t = (char*) GlobalLock(hglbCopy);
            if (t) {
                memcpy(t, s, len); t[len] = 0;
            }
            GlobalUnlock(hglbCopy);
            EmptyClipboard();
            SetClipboardData(CF_TEXT, hglbCopy);
            CloseClipboard();
        }
    }
}
