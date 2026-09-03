/*
 * Phase WIN1 — Native Win32 window backend (GDI, no extra dependencies).
 *
 * Implements the same platform/host_window.h interface as host_window.c
 * (X11). Selected by CMake when WIN32 is true. Uses only Win32 API calls
 * present since Windows XP: RegisterClassA, CreateWindowExA, GetDC,
 * StretchDIBits, PeekMessageA, SetWindowTextA. No SDL, no DirectX, no
 * additional libraries — links against stock gdi32/user32.
 *
 * Pad mapping mirrors the X11 backend exactly:
 *   Cross  = Return / Space / Z / X  (raw 0x4000)
 *   Up/Right/Down/Left               (raw 0x10/0x20/0x40/0x80)
 *   Escape or window-close           (close requested)
 */

#include "host_window.h"

/* GetTickCount64 needs Vista+ declarations on older MinGW defaults. */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

/* Framebuffer dimensions shared with the X11 backend. */
#ifndef PE_PORT_FB_WIDTH
#define PE_PORT_FB_WIDTH 320
#endif
#ifndef PE_PORT_FB_HEIGHT
#define PE_PORT_FB_HEIGHT 240
#endif

static HWND     g_hwnd = NULL;
static uint8_t *g_imgbuf = NULL;   /* BGRA back buffer, win_w * win_h * 4 */
static int      g_win_w = 0, g_win_h = 0;
static int      g_fb_w = 0, g_fb_h = 0;
static BITMAPINFO g_bmi;
static int      g_close_requested = 0;

int g_host_window_open = 0;
static uint16_t g_sony_held = 0;

/* Translate a virtual-key code to a Sony pad bit (0 = unmapped). */
static uint16_t vk_to_pad_bit(UINT vk)
{
    switch (vk) {
    case VK_RETURN:
    case VK_SPACE:
    case 'Z':
    case 'X':
        return 0x4000u;
    case VK_UP:    return 0x0010u;
    case VK_RIGHT: return 0x0020u;
    case VK_DOWN:  return 0x0040u;
    case VK_LEFT:  return 0x0080u;
    default:       return 0u;
    }
}

static void paint_window(HDC hdc)
{
    if (!g_imgbuf || g_win_w <= 0 || g_win_h <= 0) return;
    StretchDIBits(hdc, 0, 0, g_win_w, g_win_h,
                  0, 0, g_win_w, g_win_h,
                  g_imgbuf, &g_bmi, DIB_RGB_COLORS, SRCCOPY);
}

static LRESULT CALLBACK PE_WndProc(HWND hwnd, UINT msg,
                                   WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        paint_window(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (wparam == VK_ESCAPE) { g_close_requested = 1; return 0; }
        g_sony_held |= vk_to_pad_bit((UINT)wparam);
        return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        g_sony_held &= (uint16_t)~vk_to_pad_bit((UINT)wparam);
        return 0;
    case WM_CLOSE:
        g_close_requested = 1;
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        g_close_requested = 1;
        g_hwnd = NULL;
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

int HostWindow_Open(const char *display, int width, int height,
                    const char *title, int scale)
{
    WNDCLASSA wc;
    RECT rc;
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    (void)display; /* no display string on Windows */

    if (scale < 1) scale = 1;
    g_fb_w = PE_PORT_FB_WIDTH;
    g_fb_h = PE_PORT_FB_HEIGHT;
    g_win_w = width > 0 ? width : g_fb_w * scale;
    g_win_h = height > 0 ? height : g_fb_h * scale;

    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc   = PE_WndProc;
    wc.hInstance     = GetModuleHandleA(NULL);
    wc.lpszClassName = "PEPortWindow";
    wc.hCursor       = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    if (!RegisterClassA(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        fprintf(stderr, "[WINDOW] RegisterClassA failed (%lu)\n",
                (unsigned long)GetLastError());
        return -1;
    }

    /* Size the *client* area to the requested dimensions. */
    rc.left = 0; rc.top = 0; rc.right = g_win_w; rc.bottom = g_win_h;
    AdjustWindowRect(&rc, style, FALSE);

    g_hwnd = CreateWindowExA(0, "PEPortWindow", title ? title : "PE",
                             style, CW_USEDEFAULT, CW_USEDEFAULT,
                             rc.right - rc.left, rc.bottom - rc.top,
                             NULL, NULL, wc.hInstance, NULL);
    if (!g_hwnd) {
        fprintf(stderr, "[WINDOW] CreateWindowExA failed (%lu)\n",
                (unsigned long)GetLastError());
        return -1;
    }

    g_imgbuf = (uint8_t *)calloc((size_t)g_win_w * (size_t)g_win_h, 4);
    if (!g_imgbuf) {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
        return -1;
    }

    ZeroMemory(&g_bmi, sizeof(g_bmi));
    g_bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    g_bmi.bmiHeader.biWidth       = g_win_w;
    g_bmi.bmiHeader.biHeight      = -g_win_h; /* top-down */
    g_bmi.bmiHeader.biPlanes      = 1;
    g_bmi.bmiHeader.biBitCount    = 32;
    g_bmi.bmiHeader.biCompression = BI_RGB;

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    g_close_requested = 0;
    g_host_window_open = 1;
    fprintf(stderr, "[WINDOW] %dx%d (fb %dx%d) title='%s'\n",
            g_win_w, g_win_h, g_fb_w, g_fb_h, title ? title : "PE");
    return 0;
}

void HostWindow_Blit(const uint8_t *rgb, int fb_w, int fb_h)
{
    int scale_x, scale_y, ox, oy;
    HDC hdc;

    if (!g_hwnd || !g_imgbuf || !rgb) return;
    if (fb_w <= 0 || fb_h <= 0) return;
    scale_x = g_win_w / fb_w;
    scale_y = g_win_h / fb_h;
    if (scale_x < 1) scale_x = 1;
    if (scale_y < 1) scale_y = 1;
    for (oy = 0; oy < g_win_h; oy++) {
        int sy = oy / scale_y;
        uint8_t *dst;
        if (sy >= fb_h) sy = fb_h - 1;
        dst = g_imgbuf + (size_t)oy * (size_t)g_win_w * 4u;
        for (ox = 0; ox < g_win_w; ox++) {
            int sx = ox / scale_x;
            const uint8_t *src;
            if (sx >= fb_w) sx = fb_w - 1;
            src = rgb + ((size_t)sy * (size_t)fb_w + (size_t)sx) * 3u;
            *dst++ = src[2]; *dst++ = src[1]; *dst++ = src[0]; *dst++ = 0;
        }
    }
    hdc = GetDC(g_hwnd);
    if (hdc) {
        paint_window(hdc);
        ReleaseDC(g_hwnd, hdc);
    }
}

void HostWindow_SetTitle(const char *title)
{
    if (g_hwnd && title) SetWindowTextA(g_hwnd, title);
}

int HostWindow_Poll(void)
{
    MSG msg;

    if (!g_hwnd && !g_close_requested) return 0;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) { g_close_requested = 1; break; }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
        if (g_close_requested) return 1;
    }
    return g_close_requested ? 1 : 0;
}

uint16_t HostWindow_PadRaw(void)
{
    return (uint16_t)(~g_sony_held);
}

int HostWindow_Run(int milliseconds)
{
    ULONGLONG start = GetTickCount64();

    if (!g_hwnd) return 0;
    for (;;) {
        int rc = HostWindow_Poll();
        ULONGLONG elapsed;
        if (rc) return rc;
        elapsed = GetTickCount64() - start;
        if (milliseconds > 0 && elapsed >= (ULONGLONG)milliseconds) return 0;
        Sleep(16); /* ~60 Hz poll */
    }
}

void HostWindow_Close(void)
{
    if (g_hwnd) {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
    }
    free(g_imgbuf);
    g_imgbuf = NULL;
    g_host_window_open = 0;
}
