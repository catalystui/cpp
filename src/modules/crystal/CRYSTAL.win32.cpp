#include "CMAKECFG.h"
#if TARGET_PLATFORM_WIN32

#include "CATCRLIB.hpp"
#include "CATMMLIB.hpp"
#include "CATTELIB.hpp"
#include "CRYSTAL.internal.h"

#if TARGET_SUPPORTS_UNICODE
    #ifndef UNICODE
        #define UNICODE
    #endif
    #ifndef _UNICODE
        #define _UNICODE
    #endif
#endif
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

// TODO: Document opcode 1 for failed UTF-8 to UTF-16 length calculation
// TODO: Document opcode 2 for failed UTF-16 buffer allocation
// TODO: Document opcode 3 for failed UTF-8 to UTF-16 conversion
// TODO: Document opcode 4 for failed Win32 title assignment
// TODO: Document opcode 5 for failed UTF-8 length calculation
// TODO: Document opcode 6 for failed CP1252 buffer allocation
// TODO: Document opcode 7 for failed UTF-8 to CP1252 conversion
// TODO: Document opcode 8 for failed Win32 ANSI title assignment
extern "C" void crystalSetWindowTitle(CRYSTALwindow* window, catalyst::UTF8 title, catalyst::RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (title == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    HWND hWnd = (HWND) window->native.primary;

#ifdef UNICODE
    // Determine the UTF16-LE buffer length, then allocate it
    int utf16leLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (LPCCH) title, -1, NULL, 0);
    if (utf16leLength == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
        return;
    }
    WCHAR* utf16le = 0;
    catalyst::RESULT allocResult;
    catalyst::alloc((void**) &utf16le, (catalyst::NUINT) utf16leLength * (catalyst::NUINT) sizeof(WCHAR), &allocResult);
    if (utf16le == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, allocResult.status);
        return;
    }

    // Convert the UTF-8 string to UTF16-LE
    int converted = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (LPCCH) title, -1, utf16le, utf16leLength);
    if (converted == 0) {
        catalyst::free(utf16le, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 3, 0);
        return;
    }

    // Apply the title and report success
    if (!SetWindowTextW(hWnd, utf16le)) {
        catalyst::free(utf16le, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 4, 0);
        return;
    }
    catalyst::free(utf16le, 0);
#else
    // Determine the CP1252 buffer length, then allocate it
    catalyst::NUINT titleLength = 0;
    catalyst::RESULT lengthResult;
    catalyst::utf8Length(title, &titleLength, &lengthResult);
    if (!catalyst::statusCodeIsSuccess(lengthResult.status) && !catalyst::statusCodeIsWarning(lengthResult.status)) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 5, lengthResult.status);
        return;
    }
    catalyst::NUINT cp1252Capacity = titleLength + 1;
    catalyst::BYTE* cp1252 = 0;
    catalyst::RESULT allocResult;
    catalyst::alloc((void**) &cp1252, cp1252Capacity, &allocResult);
    if (cp1252 == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 6, allocResult.status);
        return;
    }

    // Convert the UTF-8 string to CP1252
    catalyst::RESULT conversionResult;
    catalyst::utf8ToCp1252(title, cp1252, cp1252Capacity, &conversionResult);
    if (!catalyst::statusCodeIsSuccess(conversionResult.status) && !catalyst::statusCodeIsWarning(conversionResult.status)) {
        catalyst::free(cp1252, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 7, conversionResult.status);
        return;
    }

    // Apply the title and report success
    if (!SetWindowTextA(hWnd, (LPCSTR) cp1252)) {
        catalyst::free(cp1252, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 8, 0);
        return;
    }
    catalyst::free(cp1252, 0);
#endif
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

// TODO: Document opcode 1 for failed native title buffer allocation
// TODO: Document opcode 2 for failed native title read
// TODO: Document opcode 3 for failed UTF-16 to UTF-8 length calculation
// TODO: Document opcode 4 for failed UTF-8 buffer allocation
// TODO: Document opcode 5 for failed UTF-16 to UTF-8 conversion
// TODO: Document opcode 6 for failed CP1252 to UTF-8 conversion
extern "C" void crystalGetWindowTitle(CRYSTALwindow* window, catalyst::UTF8W title, catalyst::NUINT capacity, catalyst::NUINT* length, catalyst::RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (title == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (capacity == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 2);
        return;
    }
    title[0] = (catalyst::BYTE) '\0';
    if (length == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 3);
        return;
    }
    *length = 0;
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the HWND reference
    HWND hWnd = (HWND) window->native.primary;

#ifdef UNICODE
    // Determine the UTF-16LE title length, then allocate a buffer for it
    int nativeLength = GetWindowTextLengthW(hWnd);
    if (nativeLength == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }
    WCHAR* utf16le = 0;
    catalyst::RESULT allocResult;
    catalyst::NUINT utf16leCapacity = (catalyst::NUINT) nativeLength + 1;
    catalyst::alloc((void**) &utf16le, utf16leCapacity * (catalyst::NUINT) sizeof(WCHAR), &allocResult);
    if (utf16le == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, allocResult.status);
        return;
    }

    // Read the UTF-16LE title
    int copied = GetWindowTextW(hWnd, utf16le, nativeLength + 1);
    if (copied == 0) {
        catalyst::free(utf16le, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, 0);
        return;
    }

    // Convert UTF-16 to UTF-8
    int utf8Capacity = WideCharToMultiByte(CP_UTF8, 0, utf16le, -1, NULL, 0, NULL, NULL);
    if (utf8Capacity == 0) {
        catalyst::free(utf16le, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 3, 0);
        return;
    }

    // Allocate UTF-8 buffer
    catalyst::BYTE* utf8 = 0;
    catalyst::alloc((void**) &utf8, (catalyst::NUINT) utf8Capacity, &allocResult);
    if (utf8 == 0) {
        catalyst::free(utf16le, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 4, allocResult.status);
        return;
    }

    // Convert UTF-16LE to UTF-8
    int converted = WideCharToMultiByte(CP_UTF8, 0, utf16le, -1, (LPSTR) utf8, utf8Capacity, NULL, NULL);
    if (converted == 0) {
        catalyst::free(utf8, 0);
        catalyst::free(utf16le, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 5, 0);
        return;
    }

    // Determine the UTF-8 length
    catalyst::NUINT required = (catalyst::NUINT) utf8Capacity - 1;
#else
    // Determine the native title length, then allocate a buffer for it
    int nativeLength = GetWindowTextLengthA(hWnd);
    if (nativeLength == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }
    catalyst::BYTE* cp1252 = 0;
    catalyst::RESULT allocResult;
    catalyst::NUINT cp1252Capacity = (catalyst::NUINT) nativeLength + 1;
    catalyst::alloc((void**) &cp1252, cp1252Capacity, &allocResult);
    if (cp1252 == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, allocResult.status);
        return;
    }

    // Read the native title in CP1252 encoding
    int copied = GetWindowTextA(hWnd, (LPSTR) cp1252, nativeLength + 1);
    if (copied == 0) {
        catalyst::free(cp1252, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, 0);
        return;
    }
    cp1252[copied] = (catalyst::BYTE) '\0';

    // Convert CP1252 to UTF-8
    catalyst::NUINT utf8Capacity = ((catalyst::NUINT) copied * 3) + 1;
    catalyst::BYTE* utf8 = 0;
    catalyst::alloc((void**) &utf8, utf8Capacity, &allocResult);
    if (utf8 == 0) {
        catalyst::free(cp1252, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 4, allocResult.status);
        return;
    }

    // Convert CP1252 to UTF-8
    catalyst::RESULT conversionResult;
    catalyst::cp1252ToUtf8(cp1252, utf8, utf8Capacity, &conversionResult);
    if (!catalyst::statusCodeIsSuccess(conversionResult.status) && !catalyst::statusCodeIsWarning(conversionResult.status)) {
        catalyst::free(utf8, 0);
        catalyst::free(cp1252, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 6, conversionResult.status);
        return;
    }

    // Determine the UTF-8 length
    catalyst::NUINT required = 0;
    while (utf8[required] != (catalyst::BYTE) '\0') {
        if (++required == (catalyst::NUINT) -1) {
            catalyst::free(utf8, 0);
            catalyst::free(cp1252, 0);
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }
    }
#endif

    // Copy output
    *length = required;
    catalyst::NUINT writable = capacity - 1;
    catalyst::NUINT i = 0;
    while (i < writable && i < required) {
        title[i] = utf8[i];
        i += 1;
    }
    title[i] = (catalyst::BYTE) '\0';

#ifdef UNICODE
    catalyst::free(utf8, 0);
    catalyst::free(utf16le, 0);
#else
    catalyst::free(utf8, 0);
    catalyst::free(cp1252, 0);
#endif

    // Report result
    if (i < required) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_WARNING_PARTIAL, 0, 0, 0);
        return;
    }
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalDestroyWindow(CRYSTALwindow* window, catalyst::RESULT* result) {
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
