#include "CATCRLIB.hpp"
#include "CATMMLIB.hpp"
#include "CRYSTAL.internal.hpp"

#define CRYSTAL_WIN32_WIDEN2(x) L##x
#define CRYSTAL_WIN32_WIDEN(x) CRYSTAL_WIN32_WIDEN2(x)

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <windows.h>

#ifdef UNICODE
#define CRYSTAL_WIN32_CLASS_NAME    L"CRYSTALWindowClass"
#define CRYSTAL_WIN32_DEFAULT_TITLE CRYSTAL_WIN32_WIDEN(CRYSTAL_DEFAULT_TITLE)
#else
#define CRYSTAL_WIN32_CLASS_NAME    "CRYSTALWindowClass"
#define CRYSTAL_WIN32_DEFAULT_TITLE CRYSTAL_DEFAULT_TITLE
#endif

static LRESULT CALLBACK crystalWin32WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_NCCREATE) {
        CREATESTRUCT* create = (CREATESTRUCT*) lParam;
        CRYSTALwindow* window = (CRYSTALwindow*) create->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) window);
        if (window != 0) {
            window->native.primary = (catalyst::NUINT) hwnd;
        }
        return TRUE;
    }
    CRYSTALwindow* window = (CRYSTALwindow*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (message) {
        case WM_CLOSE:
            if (window != 0 && window->closingCallback != 0) {
                if (!window->closingCallback(window, window->udata)) {
                    return 0;
                }
            }
            DestroyWindow(hwnd);
            return 0;
        case WM_NCDESTROY:
            if (window != 0) window->native.primary = 0;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
            return 0;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

// TODO: Document that dependency failure opcode 1 is for CATMMLIB allocation failure.
// TODO: Document that dependency failure opcode 2 is for Win32 window class registration failure.
// TODO: Document that dependency failure opcode 3 is for Win32 window creation failure.
CRYSTALwindow* crystalCreateWindow(catalyst::RESULT* result) {
    CRYSTALwindow* window = 0;
    catalyst::RESULT dependencyResult;
    catalyst::memory::alloc((void**) &window, (catalyst::NUINT) sizeof(CRYSTALwindow), &dependencyResult);
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, dependencyResult.status);
        return 0;
    }
    window->native.primary = 0;
    window->native.secondary = 0;
    window->native.tertiary = 0;
    window->udata = 0;
    window->closingCallback = 0;

    HINSTANCE hInstance = GetModuleHandle(0);
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = crystalWin32WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CRYSTAL_WIN32_CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    if (RegisterClass(&wc) == 0) {
        catalyst::memory::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, 0);
        return 0;
    }

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
                catalyst::memory::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 3, 0);
        return 0;
    }
    window->native.primary = (catalyst::NUINT) hwnd;
    window->native.secondary = (catalyst::NUINT) hInstance;
    window->native.tertiary = 0;

    SetWindowText(hwnd, CRYSTAL_WIN32_DEFAULT_TITLE);
    ShowWindow(hwnd, SW_NORMAL);
    UpdateWindow(hwnd);

    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
    return window;
}

static void crystalWin32ProcessEvents(catalyst::BOOL wait) {
    MSG message;
    if (wait && !PeekMessage(&message, 0, 0, 0, PM_NOREMOVE)) {
        WaitMessage();
    }
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

void crystalPollEvents() {
    crystalWin32ProcessEvents(CATALYST_FALSE);
}

void crystalWaitEvents() {
    crystalWin32ProcessEvents(CATALYST_TRUE);
}

// TODO: Document that dependency failure opcode 1 is for CATMMLIB deallocation failure.
void crystalDestroyWindow(CRYSTALwindow* window, catalyst::RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS_NOOP, 0, 0, 0);
        return;
    }

    if (window->native.primary != 0) {
        HWND hwnd = (HWND) window->native.primary;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        DestroyWindow(hwnd);
        window->native.primary = 0;
    }
    window->native.secondary = 0;
    window->native.tertiary = 0;
    window->udata = 0;
    window->closingCallback = 0;

    catalyst::RESULT dependencyResult;
    catalyst::memory::free(window, &dependencyResult);
    if (!catalyst::statusCodeIsSuccess(dependencyResult.status)) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, dependencyResult.status);
        return;
    }
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalSetWindowUserPointer(CRYSTALwindow* window, void* pointer) {
    if (window == 0) return;
    window->udata = pointer;
}

void* crystalGetWindowUserPointer(CRYSTALwindow* window) {
    if (window == 0) return 0;
    return window->udata;
}

void crystalSetWindowClosingCallback(CRYSTALwindow* window, CRYSTALwindowClosingCallback callback) {
    if (window == 0) return;
    window->closingCallback = callback;
}
