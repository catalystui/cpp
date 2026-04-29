#ifndef CRYSTAL_H
#define CRYSTAL_H
#include "CATCRLIB.h"
#ifdef __cplusplus
extern "C" {
#endif

// Export API
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

typedef CATALYST_BOOL (*CRYSTALwindowClosingCallback) (CRYSTALwindow* window);

CRYSTAL_API CRYSTALwindow* crystalCreateWindow(CATALYST_RESULT* result);
CRYSTAL_API void crystalPollEvents();
CRYSTAL_API void crystalWaitEvents();
CRYSTAL_API void crystalDestroyWindow(CRYSTALwindow* window, CATALYST_RESULT* result);
CRYSTAL_API void crystalSetWindowClosingCallback(CRYSTALwindow* window, CRYSTALwindowClosingCallback callback);

#ifdef __cplusplus
} /* extern C */
#endif
#endif /* CRYSTAL_H */
