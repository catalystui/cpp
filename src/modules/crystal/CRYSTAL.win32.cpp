#include "CMAKECFG.h"
#if TARGET_PLATFORM_WIN32

#include "CATCRLIB.hpp"
#include "CATMMLIB.hpp"
#include "CATTELIB.hpp"
#include "CRYSTAL.internal.h"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <windows.h>

#define CRYSTAL_WIN32_WIDEN2(x) L##x
#define CRYSTAL_WIN32_WIDEN(x) CRYSTAL_WIN32_WIDEN2(x)

#ifdef UNICODE
#define CRYSTAL_WIN32_CLASS_NAME    L"CRYSTALWindowClass"
#define CRYSTAL_WIN32_DEFAULT_TITLE CRYSTAL_WIN32_WIDEN(CRYSTAL_DEFAULT_TITLE)
#else
#define CRYSTAL_WIN32_CLASS_NAME    "CRYSTALWindowClass"
#define CRYSTAL_WIN32_DEFAULT_TITLE CRYSTAL_DEFAULT_TITLE
#endif

static LRESULT CALLBACK crystalWin32WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* create = (CREATESTRUCT*) lParam;
        CRYSTALwindow* window = (CRYSTALwindow*) create->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) window);
        if (window != 0) {
            window->native.primary = (catalyst::NUINT) hWnd;
        }
        return TRUE;
    }
    CRYSTALwindow* window = (CRYSTALwindow*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    switch (uMsg) {
        case WM_CLOSE:
            if (window != 0 && window->closingCallback != 0) {
                if (!window->closingCallback(window)) {
                    return 0;
                }
            }
            DestroyWindow(hWnd);
            return 0;
        case WM_NCDESTROY:
            if (window != 0) window->native.primary = 0;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
            return 0;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

// TODO: Document opcode 1 for failed to register window class
// TODO: Document opcode 2 for failed to create window
// TODO: Document opcode 3 for failed to create device context
extern "C" CRYSTALwindow* crystalCreateWindow(catalyst::RESULT* result) {
    // Allocate the window object
    CRYSTALwindow* window;
    catalyst::RESULT allocResult;
    catalyst::alloc((void**) &window, (catalyst::NUINT) sizeof(CRYSTALwindow), &allocResult);
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_ALLOCATION_FAILED, 0, 0, 0);
        return 0;
    }
    window->native.primary = 0;
    window->native.secondary = 0;
    window->native.tertiary = 0;
    window->platform = 0;
    window->closingCallback = 0;

    // Create the window class
    HINSTANCE hInstance = GetModuleHandle(0);
    WNDCLASS wc = { 0 };
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = crystalWin32WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CRYSTAL_WIN32_CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    if (RegisterClass(&wc) == 0) {
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
        return 0;
    }

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,
        CRYSTAL_WIN32_CLASS_NAME,
        CRYSTAL_WIN32_DEFAULT_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CRYSTAL_DEFAULT_WIDTH, CRYSTAL_DEFAULT_HEIGHT,
        NULL,
        NULL,
        hInstance,
        window
    );
    if (hwnd == NULL) {
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, 0);
        return 0;
    }

    // Create the device context
    HDC hdc = GetDC(hwnd);
    if (hdc == NULL) {
        DestroyWindow(hwnd);
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 3, 0);
        return 0;
    }

    // Assign native handles
    window->native.primary = (catalyst::NUINT) hwnd;
    window->native.secondary = (catalyst::NUINT) hInstance;
    window->native.tertiary = (catalyst::NUINT) hdc;

    // Configure and show the window
    SetWindowText(hwnd, CRYSTAL_WIN32_DEFAULT_TITLE);
    ShowWindow(hwnd, SW_NORMAL);
    UpdateWindow(hwnd);

    // Return and report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
    return window;
}

void crystalProcessEvents(catalyst::BOOL wait) {
    MSG message;
    if (wait && !PeekMessage(&message, 0, 0, 0, PM_NOREMOVE)) {
        WaitMessage();
    }
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

void crystalDestroyWindow(CRYSTALwindow* window, catalyst::RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }

    // Release HDC
    if (window->native.tertiary != 0 && window->native.primary != 0) {
        HWND hwnd = (HWND) window->native.primary;
        HDC hdc = (HDC) window->native.tertiary;
        ReleaseDC(hwnd, hdc);
        window->native.tertiary = 0;
    }

    // Release window
    if (window->native.primary != 0) {
        HWND hwnd = (HWND) window->native.primary;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        DestroyWindow(hwnd);
        window->native.primary = 0;
    }

    // Release hInstance reference
    window->native.secondary = 0;

    // Deconfigure callbacks
    window->closingCallback = 0;

    // Free the window object
    catalyst::free(window, 0);
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

#endif /* TARGET_PLATFORM_WIN32 */
