#ifndef CRYSTAL_H
#define CRYSTAL_H
#include "CATCRLIB.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Export API */
#if defined(_WIN32) || defined(__CYGWIN__)
    #if defined(CRYSTAL_EXPORTS)
        #define CRYSTAL_API __declspec(dllexport)
    #elif defined(CRYSTAL_USE_DLL)
        #define CRYSTAL_API __declspec(dllimport)
    #else
        #define CRYSTAL_API
    #endif
#else
    #define CRYSTAL_API
#endif

#define CRYSTAL_DEFAULT_WIDTH   800
#define CRYSTAL_DEFAULT_HEIGHT  450
#define CRYSTAL_DEFAULT_TITLE   "Crystal Window"

typedef struct CRYSTALwindow CRYSTALwindow;

typedef struct CRYSTALnative {
    CATALYST_NUINT primary; /* Primary native handle (HWND, NSWindow*, etc) */
    CATALYST_NUINT secondary; /* Secondary native handle (HINSTANCE, NSApplication*, etc) */
    CATALYST_NUINT tertiary; /* Tertiary native handle (HDC, NSView*, etc) */
} CRYSTALnative;

typedef CATALYST_BYTE CRYSTAL_PROPERTIES_STYLE;
enum {
    CRYSTAL_PROPERTIES_STYLE_NONE           = 0x00,
    CRYSTAL_PROPERTIES_STYLE_DECORATED      = 0x01,
    CRYSTAL_PROPERTIES_STYLE_MINIMIZABLE    = 0x02,
    CRYSTAL_PROPERTIES_STYLE_MAXIMIZABLE    = 0x04,
    CRYSTAL_PROPERTIES_STYLE_RESIZABLE      = 0x08
};

typedef CATALYST_BYTE CRYSTAL_PROPERTIES_STATE;
enum {
    CRYSTAL_PROPERTIES_STATE_NONE           = 0x00,
    CRYSTAL_PROPERTIES_STATE_FOCUSED        = 0x01,
    CRYSTAL_PROPERTIES_STATE_UNFOCUSED      = 0x02,
    CRYSTAL_PROPERTIES_STATE_MINIMIZED      = 0x04,
    CRYSTAL_PROPERTIES_STATE_MAXIMIZED      = 0x08,
    CRYSTAL_PROPERTIES_STATE_VISIBLE        = 0x10,
    CRYSTAL_PROPERTIES_STATE_HIDDEN         = 0x20
};

typedef CATALYST_NUINT (*CRYSTALwindowErroredCallback) (CRYSTALwindow* window);
typedef void (*CRYSTALwindowRepositionedCallback) (CRYSTALwindow* window, CATALYST_NUINT x, CATALYST_NUINT y);
typedef void (*CRYSTALwindowResizedCallback) (CRYSTALwindow* window, CATALYST_NUINT width, CATALYST_NUINT height);
typedef void (*CRYSTALwindowRefreshCallback) (CRYSTALwindow* window);
typedef void (*CRYSTALwindowRedrawCallback) (CRYSTALwindow* window);
typedef void (*CRYSTALwindowFocusedCallback) (CRYSTALwindow* window);
typedef void (*CRYSTALwindowUnfocusedCallback) (CRYSTALwindow* window);
typedef void (*CRYSTALwindowMinimizedCallback) (CRYSTALwindow* window);
typedef void (*CRYSTALwindowMaximizedCallback) (CRYSTALwindow* window);
typedef void (*CRYSTALwindowRestoredCallback) (CRYSTALwindow* window);
typedef void (*CRYSTALwindowShownCallback) (CRYSTALwindow* window);
typedef void (*CRYSTALwindowHiddenCallback) (CRYSTALwindow* window);
typedef CATALYST_BOOL (*CRYSTALwindowClosingCallback) (CRYSTALwindow* window);

CRYSTAL_API CRYSTALwindow* crystalCreateWindow(CATALYST_RESULT* result);
CRYSTAL_API void crystalPollEvents();
CRYSTAL_API void crystalWaitEvents();
CRYSTAL_API void crystalSetWindowTitle(CRYSTALwindow* window, CATALYST_UTF8 title, CATALYST_RESULT* result);
CRYSTAL_API void crystalGetWindowTitle(CRYSTALwindow* window, CATALYST_UTF8W title, CATALYST_NUINT capacity, CATALYST_NUINT* length, CATALYST_RESULT* result);
CRYSTAL_API void crystalSetWindowPosition(CRYSTALwindow* window, CATALYST_NUINT x, CATALYST_NUINT y, CATALYST_RESULT* result);
CRYSTAL_API void crystalGetWindowPosition(CRYSTALwindow* window, CATALYST_NUINT* x, CATALYST_NUINT* y, CATALYST_RESULT* result);
CRYSTAL_API void crystalSetWindowSize(CRYSTALwindow* window, CATALYST_NUINT width, CATALYST_NUINT height, CATALYST_RESULT* result);
CRYSTAL_API void crystalGetWindowSize(CRYSTALwindow* window, CATALYST_NUINT* width, CATALYST_NUINT* height, CATALYST_RESULT* result);
CRYSTAL_API void crystalSetWindowSizeLimits(CRYSTALwindow* window, CATALYST_NUINT minWidth, CATALYST_NUINT minHeight, CATALYST_NUINT maxWidth, CATALYST_NUINT maxHeight, CATALYST_RESULT* result);
CRYSTAL_API void crystalGetWindowSizeLimits(CRYSTALwindow* window, CATALYST_NUINT* minWidth, CATALYST_NUINT* minHeight, CATALYST_NUINT* maxWidth, CATALYST_NUINT* maxHeight, CATALYST_RESULT* result);
CRYSTAL_API void crystalSetWindowStyle(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE style, CATALYST_RESULT* result);
CRYSTAL_API void crystalGetWindowStyle(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE* style, CATALYST_RESULT* result);
CRYSTAL_API void crystalRequestWindowFocus(CRYSTALwindow* window, CATALYST_RESULT* result);
CRYSTAL_API void crystalRequestWindowAttention(CRYSTALwindow* window, CATALYST_RESULT* result);
CRYSTAL_API void crystalMinimizeWindow(CRYSTALwindow* window, CATALYST_RESULT* result);
CRYSTAL_API void crystalMaximizeWindow(CRYSTALwindow* window, CATALYST_RESULT* result);
CRYSTAL_API void crystalRestoreWindow(CRYSTALwindow* window, CATALYST_RESULT* result);
CRYSTAL_API void crystalShowWindow(CRYSTALwindow* window, CATALYST_RESULT* result);
CRYSTAL_API void crystalHideWindow(CRYSTALwindow* window, CATALYST_RESULT* result);
CRYSTAL_API void crystalGetWindowState(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STATE* state);
CRYSTAL_API void crystalCloseWindow(CRYSTALwindow* window, CATALYST_RESULT* result);
CRYSTAL_API void crystalDestroyWindow(CRYSTALwindow* window, CATALYST_RESULT* result);
CRYSTAL_API void crystalSetWindowErroredCallback(CRYSTALwindow* window, CRYSTALwindowErroredCallback callback);
CRYSTAL_API void crystalSetWindowRepositionedCallback(CRYSTALwindow* window, CRYSTALwindowRepositionedCallback callback);
CRYSTAL_API void crystalSetWindowResizedCallback(CRYSTALwindow* window, CRYSTALwindowResizedCallback callback);
CRYSTAL_API void crystalSetWindowRefreshCallback(CRYSTALwindow* window, CRYSTALwindowRefreshCallback callback);
CRYSTAL_API void crystalSetWindowRedrawCallback(CRYSTALwindow* window, CRYSTALwindowRedrawCallback callback);
CRYSTAL_API void crystalSetWindowFocusedCallback(CRYSTALwindow* window, CRYSTALwindowFocusedCallback callback);
CRYSTAL_API void crystalSetWindowUnfocusedCallback(CRYSTALwindow* window, CRYSTALwindowUnfocusedCallback callback);
CRYSTAL_API void crystalSetWindowMinimizedCallback(CRYSTALwindow* window, CRYSTALwindowMinimizedCallback callback);
CRYSTAL_API void crystalSetWindowMaximizedCallback(CRYSTALwindow* window, CRYSTALwindowMaximizedCallback callback);
CRYSTAL_API void crystalSetWindowRestoredCallback(CRYSTALwindow* window, CRYSTALwindowRestoredCallback callback);
CRYSTAL_API void crystalSetWindowShownCallback(CRYSTALwindow* window, CRYSTALwindowShownCallback callback);
CRYSTAL_API void crystalSetWindowHiddenCallback(CRYSTALwindow* window, CRYSTALwindowHiddenCallback callback);
CRYSTAL_API void crystalSetWindowClosingCallback(CRYSTALwindow* window, CRYSTALwindowClosingCallback callback);

#ifdef __cplusplus
} /* extern C */
#endif
#endif /* CRYSTAL_H */
