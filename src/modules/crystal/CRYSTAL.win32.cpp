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

#if TARGET_PLATFORM_WIN32 && (!defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0600)
    #define CRYSTAL_WIN32_DEFAULT_WIDTH   (CRYSTAL_DEFAULT_WIDTH / 2)
    #define CRYSTAL_WIN32_DEFAULT_HEIGHT  (CRYSTAL_DEFAULT_HEIGHT / 2)
#else
    #define CRYSTAL_WIN32_DEFAULT_WIDTH   CRYSTAL_DEFAULT_WIDTH
    #define CRYSTAL_WIN32_DEFAULT_HEIGHT  CRYSTAL_DEFAULT_HEIGHT
#endif

typedef struct CRYSTALwin32Platform {
    catalyst::NUINT minWidth;
    catalyst::NUINT minHeight;
    catalyst::NUINT maxWidth;
    catalyst::NUINT maxHeight;
    catalyst::BOOL wasMinimized;
    catalyst::BOOL wasMaximized;
    catalyst::BOOL destroying;
    catalyst::BOOL suppressShowWindowCallback;
} CRYSTALwin32Platform;

static void crystalWin32ClientSizeToFrameSize(HWND hwnd, catalyst::NUINT clientWidth, catalyst::NUINT clientHeight, LONG* frameWidth, LONG* frameHeight) {
    DWORD style = (DWORD) GetWindowLongPtr(hwnd, GWL_STYLE);
    DWORD exStyle = (DWORD) GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = (LONG) clientWidth;
    rect.bottom = (LONG) clientHeight;
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);
    *frameWidth = rect.right - rect.left;
    *frameHeight = rect.bottom - rect.top;
}

static DWORD crystalWin32StyleFromCrystalStyle(CRYSTAL_PROPERTIES_STYLE style) {
    DWORD nativeStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    if ((style & CRYSTAL_PROPERTIES_STYLE_DECORATED) != 0) {
        nativeStyle |= WS_CAPTION;
        nativeStyle |= WS_SYSMENU;
    }
    if ((style & CRYSTAL_PROPERTIES_STYLE_MINIMIZABLE) != 0) {
        nativeStyle |= WS_SYSMENU;
        nativeStyle |= WS_MINIMIZEBOX;
    }
    if ((style & CRYSTAL_PROPERTIES_STYLE_MAXIMIZABLE) != 0) {
        nativeStyle |= WS_SYSMENU;
        nativeStyle |= WS_MAXIMIZEBOX;
    }
    if ((style & CRYSTAL_PROPERTIES_STYLE_RESIZABLE) != 0) {
        nativeStyle |= WS_THICKFRAME;
    }
    return nativeStyle;
}

static CRYSTAL_PROPERTIES_STYLE crystalWin32StyleToCrystalStyle(DWORD nativeStyle) {
    CRYSTAL_PROPERTIES_STYLE style;
    style = CRYSTAL_PROPERTIES_STYLE_NONE;
    if ((nativeStyle & WS_CAPTION) == WS_CAPTION && (nativeStyle & WS_SYSMENU) == WS_SYSMENU) {
        style = (CRYSTAL_PROPERTIES_STYLE) (style | CRYSTAL_PROPERTIES_STYLE_DECORATED);
    }
    if ((nativeStyle & WS_MINIMIZEBOX) != 0 && (nativeStyle & WS_SYSMENU) != 0) {
        style = (CRYSTAL_PROPERTIES_STYLE) (style | CRYSTAL_PROPERTIES_STYLE_MINIMIZABLE);
    }
    if ((nativeStyle & WS_MAXIMIZEBOX) != 0 && (nativeStyle & WS_SYSMENU) != 0) {
        style = (CRYSTAL_PROPERTIES_STYLE) (style | CRYSTAL_PROPERTIES_STYLE_MAXIMIZABLE);
    }
    if ((nativeStyle & WS_THICKFRAME) != 0) {
        style = (CRYSTAL_PROPERTIES_STYLE) (style | CRYSTAL_PROPERTIES_STYLE_RESIZABLE);
    }
    return style;
}

static void crystalWin32Refresh(CRYSTALwindow* window) {
    if (window != 0 && window->refreshCallback != 0) {
        window->refreshCallback(window);
    }
}

static void crystalWin32Shown(CRYSTALwindow* window) {
    if (window != 0 && window->shownCallback != 0) {
        window->shownCallback(window);
    }
    crystalWin32Refresh(window);
}

static void crystalWin32Hidden(CRYSTALwindow* window) {
    if (window != 0 && window->hiddenCallback != 0) {
        window->hiddenCallback(window);
    }
    crystalWin32Refresh(window);
}

static catalyst::BOOL crystalWin32IsSameOrChildWindow(HWND root, HWND candidate) {
    if (root == 0 || candidate == 0) {
        return 0;
    }
    if (candidate == root) {
        return 1;
    }
    if (IsChild(root, candidate)) {
        return 1;
    }
    return 0;
}

static catalyst::BOOL crystalWin32IsWindowFocused(HWND hWnd) {
    HWND foreground = GetForegroundWindow();
    if (crystalWin32IsSameOrChildWindow(hWnd, foreground)) {
        return 1;
    }
    HWND focus = GetFocus();
    if (crystalWin32IsSameOrChildWindow(hWnd, focus)) {
        return 1;
    }
    return 0;
}

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
    CRYSTALwin32Platform* platform = 0;
    if (window != 0) platform = (CRYSTALwin32Platform*) window->platform;
    switch (uMsg) {
        case WM_MOVE:
            if (window != 0) {
                RECT rect;
                if (GetWindowRect(hWnd, &rect)) {
                    if (window->repositionedCallback != 0) {
                        window->repositionedCallback(window, (catalyst::NUINT) rect.left, (catalyst::NUINT) rect.top);
                    }
                    crystalWin32Refresh(window);
                }
            }
            return 0;
        case WM_SIZE:
            if (window != 0) {
                if (wParam != SIZE_MINIMIZED && window->resizedCallback != 0) {
                    RECT clientRect;
                    if (GetClientRect(hWnd, &clientRect)) {
                        window->resizedCallback(
                            window,
                            (catalyst::NUINT) (clientRect.right - clientRect.left),
                            (catalyst::NUINT) (clientRect.bottom - clientRect.top)
                        );
                    }
                }
                if (platform != 0) {
                    if (wParam == SIZE_MINIMIZED) {
                        if (!platform->wasMinimized && window->minimizedCallback != 0) {
                            window->minimizedCallback(window);
                        }
                        platform->wasMinimized = 1;
                    }
                    else if (wParam == SIZE_MAXIMIZED) {
                        if (!platform->wasMaximized && window->maximizedCallback != 0) {
                            window->maximizedCallback(window);
                        }
                        platform->wasMinimized = 0;
                        platform->wasMaximized = 1;
                    }
                    else if (wParam == SIZE_RESTORED) {
                        if ((platform->wasMinimized || platform->wasMaximized) && window->restoredCallback != 0) {
                            window->restoredCallback(window);
                        }
                        platform->wasMinimized = 0;
                        platform->wasMaximized = 0;
                    }
                }
                crystalWin32Refresh(window);
                if (window->redrawCallback != 0) {
                    window->redrawCallback(window);
                }
            }
            return 0;
        case WM_GETMINMAXINFO:
            if (platform == 0) {
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }
            {
                MINMAXINFO* info = (MINMAXINFO*) lParam;
                LONG width;
                LONG height;
                if (platform->minWidth != 0) {
                    crystalWin32ClientSizeToFrameSize(hWnd, platform->minWidth, platform->minHeight, &width, &height);
                    info->ptMinTrackSize.x = width;
                }
                if (platform->minHeight != 0) {
                    crystalWin32ClientSizeToFrameSize(hWnd, platform->minWidth, platform->minHeight, &width, &height);
                    info->ptMinTrackSize.y = height;
                }
                if (platform->maxWidth != 0) {
                    catalyst::NUINT nativeHeight = platform->maxHeight == 0 ? (catalyst::NUINT) info->ptMaxTrackSize.y : platform->maxHeight;
                    crystalWin32ClientSizeToFrameSize(hWnd, platform->maxWidth, nativeHeight, &width, &height);
                    info->ptMaxTrackSize.x = width;
                }
                if (platform->maxHeight != 0) {
                    catalyst::NUINT nativeWidth = platform->maxWidth == 0 ? (catalyst::NUINT) info->ptMaxTrackSize.x : platform->maxWidth;
                    crystalWin32ClientSizeToFrameSize(hWnd, nativeWidth, platform->maxHeight, &width, &height);
                    info->ptMaxTrackSize.y = height;
                }
            }
            return 0;
        case WM_ACTIVATE:
            if (window != 0) {
                if (LOWORD(wParam) == WA_INACTIVE) {
                    if (window->unfocusedCallback != 0) {
                        window->unfocusedCallback(window);
                    }
                } else {
                    if (window->focusedCallback != 0) {
                        window->focusedCallback(window);
                    }
                }
                crystalWin32Refresh(window);
            }
            return 0;
        case WM_SHOWWINDOW:
            if (window != 0) {
                if (platform != 0 && platform->suppressShowWindowCallback) {
                    return 0;
                }
                if (wParam) {
                    crystalWin32Shown(window);
                } else {
                    crystalWin32Hidden(window);
                }
            }
            return 0;
        case WM_PAINT:
            {
                PAINTSTRUCT paint;
                HDC hdc = BeginPaint(hWnd, &paint);
                FillRect(hdc, &paint.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));
                if (window != 0 && window->redrawCallback != 0) {
                    window->redrawCallback(window);
                }
                EndPaint(hWnd, &paint);
            }
            return 0;
        case WM_DISPLAYCHANGE:
            crystalWin32Refresh(window);
            return 0;
        case WM_CLOSE:
            if (window != 0 && window->closingCallback != 0) {
                if (!window->closingCallback(window)) {
                    return 0;
                }
            }
            if (window != 0 && platform != 0 && !platform->destroying) {
                platform->destroying = 1;
                crystalDestroyWindow(window, 0);
                return 0;
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
// TODO: Document opcode 2 for failed to create platform storage
// TODO: Document opcode 3 for failed to create window
// TODO: Document opcode 4 for failed to create device context
extern "C" CRYSTALwindow* crystalCreateWindow(CATALYST_RESULT* result) {
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
    window->erroredCallback = 0;
    window->repositionedCallback = 0;
    window->resizedCallback = 0;
    window->refreshCallback = 0;
    window->redrawCallback = 0;
    window->focusedCallback = 0;
    window->unfocusedCallback = 0;
    window->minimizedCallback = 0;
    window->maximizedCallback = 0;
    window->restoredCallback = 0;
    window->shownCallback = 0;
    window->hiddenCallback = 0;
    window->closingCallback = 0;

    // Create the window class
    HINSTANCE hInstance = GetModuleHandle(0);
    WNDCLASS wc = { 0 };
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = crystalWin32WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CRYSTAL_WIN32_CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    if (RegisterClass(&wc) == 0) {
        DWORD error = GetLastError();
        if (error != ERROR_CLASS_ALREADY_EXISTS) {
            catalyst::free(window, 0);
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, (CATALYST_NUINT) error);
            return 0;
        }
    }

    // Create platform storage
    CRYSTALwin32Platform* platform;
    catalyst::alloc((void**) &platform, (catalyst::NUINT) sizeof(CRYSTALwin32Platform), &allocResult);
    if (platform == 0) {
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, 0);
        return 0;
    }
    platform->minWidth = 0;
    platform->minHeight = 0;
    platform->maxWidth = 0;
    platform->maxHeight = 0;
    platform->wasMinimized = 0;
    platform->wasMaximized = 0;
    platform->destroying = 0;
    platform->suppressShowWindowCallback = 0;
    window->platform = platform;

    // Adjust the window rectangle to get the correct client-side area
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD exStyle = 0;
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = CRYSTAL_WIN32_DEFAULT_WIDTH;
    rect.bottom = CRYSTAL_WIN32_DEFAULT_HEIGHT;
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);

    // Create the window
    HWND hwnd = CreateWindowEx(
        exStyle,
        CRYSTAL_WIN32_CLASS_NAME,
        CRYSTAL_WIN32_DEFAULT_TITLE,
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        hInstance,
        window
    );
    if (hwnd == NULL) {
        catalyst::free(platform, 0);
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 3, 0);
        return 0;
    }

    // Create the device context
    HDC hdc = GetDC(hwnd);
    if (hdc == NULL) {
        DestroyWindow(hwnd);
        catalyst::free(platform, 0);
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 4, 0);
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

void crystalProcessEvents(CATALYST_BOOL wait) {
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
extern "C" void crystalSetWindowTitle(CRYSTALwindow* window, CATALYST_UTF8 title, CATALYST_RESULT* result) {
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
extern "C" void crystalGetWindowTitle(CRYSTALwindow* window, CATALYST_UTF8W title, CATALYST_NUINT capacity, CATALYST_NUINT* length, CATALYST_RESULT* result) {
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

extern "C" void crystalSetWindowPosition(CRYSTALwindow* window, CATALYST_NUINT x, CATALYST_NUINT y, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the HWND reference
    HWND hWnd = (HWND) window->native.primary;

    // Set the window position
    if (!SetWindowPos(hWnd, NULL, (int) x, (int) y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE)) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
        return;
    }

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalGetWindowPosition(CRYSTALwindow* window, CATALYST_NUINT* x, CATALYST_NUINT* y, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (x == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    *x = 0;
    if (y == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 2);
        return;
    }
    *y = 0;
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the HWND reference
    HWND hWnd = (HWND) window->native.primary;

    // Get the window rectangle
    RECT rect;
    if (!GetWindowRect(hWnd, &rect)) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
        return;
    }
    *x = (catalyst::NUINT) rect.left;
    *y = (catalyst::NUINT) rect.top;

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalSetWindowSize(CRYSTALwindow* window, CATALYST_NUINT width, CATALYST_NUINT height, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (width == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (height == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 2);
        return;
    }
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Apply size constraints from the platform storage, if available
    CRYSTALwin32Platform* platform = (CRYSTALwin32Platform*) window->platform;
    if (platform != 0) {
        if (platform->minWidth != 0 && width < platform->minWidth) width = platform->minWidth;
        if (platform->minHeight != 0 && height < platform->minHeight) height = platform->minHeight;
        if (platform->maxWidth != 0 && width > platform->maxWidth) width = platform->maxWidth;
        if (platform->maxHeight != 0 && height > platform->maxHeight) height = platform->maxHeight;
    }

    // Get the HWND reference
    HWND hWnd = (HWND) window->native.primary;

    // Convert client size to frame size, then set the window size
    LONG frameWidth;
    LONG frameHeight;
    crystalWin32ClientSizeToFrameSize(hWnd, width, height, &frameWidth, &frameHeight);
    if (!SetWindowPos(hWnd, NULL, 0, 0, frameWidth, frameHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE)) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
        return;
    }

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalGetWindowSize(CRYSTALwindow* window, CATALYST_NUINT* width, CATALYST_NUINT* height, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (width == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    *width = 0;
    if (height == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 2);
        return;
    }
    *height = 0;
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the HWND reference
    HWND hWnd = (HWND) window->native.primary;

    // Get the client rectangle
    RECT rect;
    if (!GetClientRect(hWnd, &rect)) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
        return;
    }
    *width = (catalyst::NUINT) (rect.right - rect.left);
    *height = (catalyst::NUINT) (rect.bottom - rect.top);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalSetWindowSizeLimits(CRYSTALwindow* window, CATALYST_NUINT minWidth, CATALYST_NUINT minHeight, CATALYST_NUINT maxWidth, CATALYST_NUINT maxHeight, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (maxWidth != 0 && minWidth != 0 && maxWidth < minWidth) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 3);
        return;
    }
    if (maxHeight != 0 && minHeight != 0 && maxHeight < minHeight) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 4);
        return;
    }
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Apply size limits to the platform storage, if available
    CRYSTALwin32Platform* platform = (CRYSTALwin32Platform*) window->platform;
    if (platform == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 1);
        return;
    }
    platform->minWidth = minWidth;
    platform->minHeight = minHeight;
    platform->maxWidth = maxWidth;
    platform->maxHeight = maxHeight;

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalGetWindowSizeLimits(CRYSTALwindow* window, CATALYST_NUINT* minWidth, CATALYST_NUINT* minHeight, CATALYST_NUINT* maxWidth, CATALYST_NUINT* maxHeight, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (minWidth == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    *minWidth = 0;
    if (minHeight == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 2);
        return;
    }
    *minHeight = 0;
    if (maxWidth == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 3);
        return;
    }
    *maxWidth = 0;
    if (maxHeight == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 4);
        return;
    }
    *maxHeight = 0;
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Read size limits from the platform storage, if available
    CRYSTALwin32Platform* platform = (CRYSTALwin32Platform*) window->platform;
    if (platform == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 1);
        return;
    }
    *minWidth = platform->minWidth;
    *minHeight = platform->minHeight;
    *maxWidth = platform->maxWidth;
    *maxHeight = platform->maxHeight;

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalSetWindowStyle(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE style, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if ((style & ~(
        CRYSTAL_PROPERTIES_STYLE_DECORATED |
        CRYSTAL_PROPERTIES_STYLE_MINIMIZABLE |
        CRYSTAL_PROPERTIES_STYLE_MAXIMIZABLE |
        CRYSTAL_PROPERTIES_STYLE_RESIZABLE
    )) != 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the HWND reference
    HWND hWnd = (HWND) window->native.primary;

    // Determine the new window style
    DWORD oldStyle = (DWORD) GetWindowLongPtr(hWnd, GWL_STYLE);
    DWORD exStyle = (DWORD) GetWindowLongPtr(hWnd, GWL_EXSTYLE);
    DWORD nativeStyle = crystalWin32StyleFromCrystalStyle(style);
    nativeStyle |= (oldStyle & (WS_VISIBLE | WS_MINIMIZE | WS_MAXIMIZE));

    // Adjust the window rectangle
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    POINT clientTopLeft;
    clientTopLeft.x = 0;
    clientTopLeft.y = 0;
    ClientToScreen(hWnd, &clientTopLeft);

    // Determine new frame, client size
    RECT newFrame;
    newFrame.left = 0;
    newFrame.top = 0;
    newFrame.right = clientRect.right - clientRect.left;
    newFrame.bottom = clientRect.bottom - clientRect.top;
    AdjustWindowRectEx(&newFrame, nativeStyle, FALSE, exStyle);
    SetWindowLongPtr(hWnd, GWL_STYLE, (LONG_PTR) nativeStyle);

    // Set the new window position and style
    if (!SetWindowPos(
        hWnd,
        NULL,
        clientTopLeft.x + newFrame.left,
        clientTopLeft.y + newFrame.top,
        newFrame.right - newFrame.left,
        newFrame.bottom - newFrame.top,
        SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
    )) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
        return;
    }

    // Refresh the window
    crystalWin32Refresh(window);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalGetWindowStyle(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE* style, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (style == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    *style = CRYSTAL_PROPERTIES_STYLE_NONE;
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the HWND reference
    HWND hWnd = (HWND) window->native.primary;

    // Read the native style, convert, and assign
    DWORD nativeStyle = (DWORD) GetWindowLongPtr(hWnd, GWL_STYLE);
    *style = crystalWin32StyleToCrystalStyle(nativeStyle);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalRequestWindowFocus(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the HWND reference
    HWND hWnd = (HWND) window->native.primary;

    // Attempt to bring the window to the foreground and focus it
    BringWindowToTop(hWnd);
    SetForegroundWindow(hWnd);
    SetActiveWindow(hWnd);
    SetFocus(hWnd);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalRequestWindowAttention(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the HWND reference
    HWND hWnd = (HWND) window->native.primary;

    // Flash the taskbar entry
#if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0501
    FLASHWINFO info;
    info.cbSize = sizeof(FLASHWINFO);
    info.hwnd = hWnd;
    info.dwFlags = FLASHW_TRAY | FLASHW_TIMERNOFG;
    info.uCount = 0;
    info.dwTimeout = 0;
    FlashWindowEx(&info);
#else
    FlashWindow(hWnd, TRUE);
#endif

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalMinimizeWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the HWND reference and minimize the window
    ShowWindow((HWND) window->native.primary, SW_MINIMIZE);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalMaximizeWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the HWND reference and maximize the window
    ShowWindow((HWND) window->native.primary, SW_MAXIMIZE);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalRestoreWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the HWND reference and restore the window
    ShowWindow((HWND) window->native.primary, SW_RESTORE);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalShowWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the HWND reference
    HWND hWnd = (HWND) window->native.primary;
    CRYSTALwin32Platform* platform = (CRYSTALwin32Platform*) window->platform;

    // Show the window
    if (platform != 0) {
        platform->suppressShowWindowCallback = 1;
    }
    BOOL succeeded = SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    if (platform != 0) {
        platform->suppressShowWindowCallback = 0;
    }
    if (!succeeded) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
        return;
    }

    // Manually invoke the shown callback
    crystalWin32Shown(window);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalHideWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the HWND reference
    HWND hWnd = (HWND) window->native.primary;
    CRYSTALwin32Platform* platform = (CRYSTALwin32Platform*) window->platform;

    // Hide the window
    if (platform != 0) {
        platform->suppressShowWindowCallback = 1;
    }
    ShowWindow(hWnd, SW_HIDE);
    if (platform != 0) {
        platform->suppressShowWindowCallback = 0;
    }

    // Manually invoke the hidden callback
    crystalWin32Hidden(window);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void crystalGetWindowState(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STATE* state) {
    if (state == 0) {
        return;
    }
    *state = CRYSTAL_PROPERTIES_STATE_NONE;
    if (window == 0) {
        return;
    }
    if (window->native.primary == 0) {
        return;
    }

    // Get the HWND reference
    HWND hWnd = (HWND) window->native.primary;

    // Determine window state
    if (crystalWin32IsWindowFocused(hWnd)) {
        *state = (CRYSTAL_PROPERTIES_STATE) (*state | CRYSTAL_PROPERTIES_STATE_FOCUSED);
    } else {
        *state = (CRYSTAL_PROPERTIES_STATE) (*state | CRYSTAL_PROPERTIES_STATE_UNFOCUSED);
    }
    if (IsIconic(hWnd)) {
        *state = (CRYSTAL_PROPERTIES_STATE) (*state | CRYSTAL_PROPERTIES_STATE_MINIMIZED);
    }
    if (IsZoomed(hWnd)) {
        *state = (CRYSTAL_PROPERTIES_STATE) (*state | CRYSTAL_PROPERTIES_STATE_MAXIMIZED);
    }
    if (IsWindowVisible(hWnd) && !IsIconic(hWnd)) {
        *state = (CRYSTAL_PROPERTIES_STATE) (*state | CRYSTAL_PROPERTIES_STATE_VISIBLE);
    } else {
        *state = (CRYSTAL_PROPERTIES_STATE) (*state | CRYSTAL_PROPERTIES_STATE_HIDDEN);
    }
}

extern "C" void crystalCloseWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (window->native.primary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Invoke the closing callback, if configured
    if (window->closingCallback != 0) {
        if (!window->closingCallback(window)) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS_NOOP, 0, 0, 0);
            return;
        }
    }

    // Destroy the window
    crystalDestroyWindow(window, result);
}

extern "C" void crystalDestroyWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }

    // Get the platform storage, mark as destroying
    CRYSTALwin32Platform* platform = (CRYSTALwin32Platform*) window->platform;
    if (platform != 0) {
        platform->destroying = 1;
    }

    // Release HDC
    if (window->native.tertiary != 0 && window->native.primary != 0) {
        HWND hwnd = (HWND) window->native.primary;
        HDC hdc = (HDC) window->native.tertiary;
        ReleaseDC(hwnd, hdc);
        window->native.tertiary = 0;
    }

    // Release hInstance reference
    window->native.secondary = 0;

    // Release window
    if (window->native.primary != 0) {
        HWND hwnd = (HWND) window->native.primary;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        DestroyWindow(hwnd);
        window->native.primary = 0;
    }

    // Release platform storage
    if (window->platform != 0) {
        catalyst::free(window->platform, 0);
        window->platform = 0;
    }

    // Deconfigure callbacks
    window->erroredCallback = 0;
    window->repositionedCallback = 0;
    window->resizedCallback = 0;
    window->refreshCallback = 0;
    window->redrawCallback = 0;
    window->focusedCallback = 0;
    window->unfocusedCallback = 0;
    window->minimizedCallback = 0;
    window->maximizedCallback = 0;
    window->restoredCallback = 0;
    window->shownCallback = 0;
    window->hiddenCallback = 0;
    window->closingCallback = 0;

    // Free the window object
    catalyst::free(window, 0);
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

#endif /* TARGET_PLATFORM_WIN32 */
