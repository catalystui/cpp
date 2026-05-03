#include "CMAKECFG.h"
#if TARGET_PLATFORM_LINUX

#include "CATCRLIB.hpp"
#include "modules/crystal/CRYSTAL.h"
#include "modules/crystal/CRYSTAL.linux.h"
#include "CRYSTAL.internal.h"
#if !TARGET_SHARED
#include "CRYSTAL.linux.internal.h"
#endif

#include <dlfcn.h>

using namespace catalyst;

static CRYSTAL_DISPLAY_SERVER displayServer = CRYSTAL_DISPLAY_SERVER_NONE;
static BOOL displayServerInitialized = FALSE;
static void* backendLibrary = 0;

typedef void* (*CRYSTAL_WL_DISPLAY_CONNECT) (const char* name);
typedef void  (*CRYSTAL_WL_DISPLAY_DISCONNECT) (void* display);

typedef void* (*CRYSTAL_X11_OPEN_DISPLAY) (const char* name);
typedef void  (*CRYSTAL_X11_CLOSE_DISPLAY) (void* display);

typedef CRYSTALwindow* (*CRYSTAL_BACKEND_CREATE_WINDOW) (CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_PROCESS_EVENTS) (CATALYST_BOOL wait);
typedef void (*CRYSTAL_BACKEND_SET_WINDOW_TITLE) (CRYSTALwindow* window, CATALYST_UTF8 title, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_GET_WINDOW_TITLE) (CRYSTALwindow* window, CATALYST_UTF8W title, CATALYST_NUINT capacity, CATALYST_NUINT* length, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_SET_WINDOW_POSITION) (CRYSTALwindow* window, CATALYST_NUINT x, CATALYST_NUINT y, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_GET_WINDOW_POSITION) (CRYSTALwindow* window, CATALYST_NUINT* x, CATALYST_NUINT* y, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_SET_WINDOW_SIZE) (CRYSTALwindow* window, CATALYST_NUINT width, CATALYST_NUINT height, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_GET_WINDOW_SIZE) (CRYSTALwindow* window, CATALYST_NUINT* width, CATALYST_NUINT* height, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_SET_WINDOW_SIZE_LIMITS) (CRYSTALwindow* window, CATALYST_NUINT minWidth, CATALYST_NUINT minHeight, CATALYST_NUINT maxWidth, CATALYST_NUINT maxHeight, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_GET_WINDOW_SIZE_LIMITS) (CRYSTALwindow* window, CATALYST_NUINT* minWidth, CATALYST_NUINT* minHeight, CATALYST_NUINT* maxWidth, CATALYST_NUINT* maxHeight, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_SET_WINDOW_STYLE) (CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE style, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_GET_WINDOW_STYLE) (CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE* style, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_REQUEST_WINDOW_FOCUS) (CRYSTALwindow* window, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_REQUEST_WINDOW_ATTENTION) (CRYSTALwindow* window, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_MINIMIZE_WINDOW) (CRYSTALwindow* window, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_MAXIMIZE_WINDOW) (CRYSTALwindow* window, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_RESTORE_WINDOW) (CRYSTALwindow* window, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_SHOW_WINDOW) (CRYSTALwindow* window, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_HIDE_WINDOW) (CRYSTALwindow* window, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_GET_WINDOW_STATE) (CRYSTALwindow* window, CRYSTAL_PROPERTIES_STATE* state);
typedef void (*CRYSTAL_BACKEND_CLOSE_WINDOW) (CRYSTALwindow* window, CATALYST_RESULT* result);
typedef void (*CRYSTAL_BACKEND_DESTROY_WINDOW) (CRYSTALwindow* window, CATALYST_RESULT* result);

static CRYSTAL_BACKEND_CREATE_WINDOW backendCreateWindow = 0;
static CRYSTAL_BACKEND_PROCESS_EVENTS backendProcessEvents = 0;
static CRYSTAL_BACKEND_SET_WINDOW_TITLE backendSetWindowTitle = 0;
static CRYSTAL_BACKEND_GET_WINDOW_TITLE backendGetWindowTitle = 0;
static CRYSTAL_BACKEND_SET_WINDOW_POSITION backendSetWindowPosition = 0;
static CRYSTAL_BACKEND_GET_WINDOW_POSITION backendGetWindowPosition = 0;
static CRYSTAL_BACKEND_SET_WINDOW_SIZE backendSetWindowSize = 0;
static CRYSTAL_BACKEND_GET_WINDOW_SIZE backendGetWindowSize = 0;
static CRYSTAL_BACKEND_SET_WINDOW_SIZE_LIMITS backendSetWindowSizeLimits = 0;
static CRYSTAL_BACKEND_GET_WINDOW_SIZE_LIMITS backendGetWindowSizeLimits = 0;
static CRYSTAL_BACKEND_SET_WINDOW_STYLE backendSetWindowStyle = 0;
static CRYSTAL_BACKEND_GET_WINDOW_STYLE backendGetWindowStyle = 0;
static CRYSTAL_BACKEND_REQUEST_WINDOW_FOCUS backendRequestWindowFocus = 0;
static CRYSTAL_BACKEND_REQUEST_WINDOW_ATTENTION backendRequestWindowAttention = 0;
static CRYSTAL_BACKEND_MINIMIZE_WINDOW backendMinimizeWindow = 0;
static CRYSTAL_BACKEND_MAXIMIZE_WINDOW backendMaximizeWindow = 0;
static CRYSTAL_BACKEND_RESTORE_WINDOW backendRestoreWindow = 0;
static CRYSTAL_BACKEND_SHOW_WINDOW backendShowWindow = 0;
static CRYSTAL_BACKEND_HIDE_WINDOW backendHideWindow = 0;
static CRYSTAL_BACKEND_GET_WINDOW_STATE backendGetWindowState = 0;
static CRYSTAL_BACKEND_CLOSE_WINDOW backendCloseWindow = 0;
static CRYSTAL_BACKEND_DESTROY_WINDOW backendDestroyWindow = 0;

// opcode1 = libwayland not found
// opcode2 = symbols missing or display failed to connect
static BOOL crystalInitializeWayland(RESULT* result) {
    void* wllib = dlopen("libwayland-client.so.0", RTLD_LAZY | RTLD_LOCAL);
    if (wllib != 0) {
        CRYSTAL_WL_DISPLAY_CONNECT wlDisplayConnect = (CRYSTAL_WL_DISPLAY_CONNECT) dlsym(wllib, "wl_display_connect");
        CRYSTAL_WL_DISPLAY_DISCONNECT wlDisplayDisconnect = (CRYSTAL_WL_DISPLAY_DISCONNECT) dlsym(wllib, "wl_display_disconnect");
        if (wlDisplayConnect != 0 && wlDisplayDisconnect != 0) {
            void* display = wlDisplayConnect(0);
            if (display != 0) {
                wlDisplayDisconnect(display);
                dlclose(wllib);
                return TRUE;
            } else {
                if (result != 0) *result = RESULT(STATUS_CODE_WARNING, 0, 2, 0);
            }
        } else {
            if (result != 0) *result = RESULT(STATUS_CODE_WARNING, 0, 2, 0);
        }
        dlclose(wllib);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_WARNING, 0, 1, 0);
    }
    return FALSE;
}

// opcode1 = libx11 not found
// opcode2 = symbols missing or display failed to connect
static BOOL crystalInitializeX11(RESULT* result) {
    void* x11lib = dlopen("libX11.so.6", RTLD_LAZY | RTLD_LOCAL);
    if (x11lib != 0) {
        CRYSTAL_X11_OPEN_DISPLAY x11OpenDisplay = (CRYSTAL_X11_OPEN_DISPLAY) dlsym(x11lib, "XOpenDisplay");
        CRYSTAL_X11_CLOSE_DISPLAY x11CloseDisplay = (CRYSTAL_X11_CLOSE_DISPLAY) dlsym(x11lib, "XCloseDisplay");
        if (x11OpenDisplay != 0 && x11CloseDisplay != 0) {
            void* display = x11OpenDisplay(0);
            if (display != 0) {
                x11CloseDisplay(display);
                dlclose(x11lib);
                return TRUE;
            } else {
                if (result != 0) *result = RESULT(STATUS_CODE_WARNING, 0, 2, 0);
            }
        } else {
            if (result != 0) *result = RESULT(STATUS_CODE_WARNING, 0, 2, 0);
        }
        dlclose(x11lib);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_WARNING, 0, 1, 0);
    }
    return FALSE;
}

// opcode1 = unable to find backend library for display server
// opcode2 = symbols missing from backend library
static BOOL crystalInitializeBackend(CRYSTAL_DISPLAY_SERVER displayServer, CATALYST_RESULT* result) {
    if (backendLibrary != 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS_NOOP, 0, 0, 0);
        return TRUE;
    }
    if (displayServer == CRYSTAL_DISPLAY_SERVER_NONE) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 1, 0);
        return FALSE;
    }

#if TARGET_SHARED
    // Determine library name and open it
    const char* libraryName = 0;
    if (displayServer == CRYSTAL_DISPLAY_SERVER_WAYLAND) {
        libraryName = "libcrystal-wayland.so";
    } else if (displayServer == CRYSTAL_DISPLAY_SERVER_X11) {
        libraryName = "libcrystal-x11.so";
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return FALSE;
    }
    backendLibrary = dlopen(libraryName, RTLD_LAZY | RTLD_LOCAL);
    if (backendLibrary == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
        return FALSE;
    }

    // Assign and validate function pointers
    backendCreateWindow = (CRYSTAL_BACKEND_CREATE_WINDOW) dlsym(backendLibrary, "crystalCreateWindow");
    backendProcessEvents = (CRYSTAL_BACKEND_PROCESS_EVENTS) dlsym(backendLibrary, "crystalProcessEvents");
    backendSetWindowTitle = (CRYSTAL_BACKEND_SET_WINDOW_TITLE) dlsym(backendLibrary, "crystalSetWindowTitle");
    backendGetWindowTitle = (CRYSTAL_BACKEND_GET_WINDOW_TITLE) dlsym(backendLibrary, "crystalGetWindowTitle");
    backendSetWindowPosition = (CRYSTAL_BACKEND_SET_WINDOW_POSITION) dlsym(backendLibrary, "crystalSetWindowPosition");
    backendGetWindowPosition = (CRYSTAL_BACKEND_GET_WINDOW_POSITION) dlsym(backendLibrary, "crystalGetWindowPosition");
    backendSetWindowSize = (CRYSTAL_BACKEND_SET_WINDOW_SIZE) dlsym(backendLibrary, "crystalSetWindowSize");
    backendGetWindowSize = (CRYSTAL_BACKEND_GET_WINDOW_SIZE) dlsym(backendLibrary, "crystalGetWindowSize");
    backendSetWindowSizeLimits = (CRYSTAL_BACKEND_SET_WINDOW_SIZE_LIMITS) dlsym(backendLibrary, "crystalSetWindowSizeLimits");
    backendGetWindowSizeLimits = (CRYSTAL_BACKEND_GET_WINDOW_SIZE_LIMITS) dlsym(backendLibrary, "crystalGetWindowSizeLimits");
    backendSetWindowStyle = (CRYSTAL_BACKEND_SET_WINDOW_STYLE) dlsym(backendLibrary, "crystalSetWindowStyle");
    backendGetWindowStyle = (CRYSTAL_BACKEND_GET_WINDOW_STYLE) dlsym(backendLibrary, "crystalGetWindowStyle");
    backendRequestWindowFocus = (CRYSTAL_BACKEND_REQUEST_WINDOW_FOCUS) dlsym(backendLibrary, "crystalRequestWindowFocus");
    backendRequestWindowAttention = (CRYSTAL_BACKEND_REQUEST_WINDOW_ATTENTION) dlsym(backendLibrary, "crystalRequestWindowAttention");
    backendMinimizeWindow = (CRYSTAL_BACKEND_MINIMIZE_WINDOW) dlsym(backendLibrary, "crystalMinimizeWindow");
    backendMaximizeWindow = (CRYSTAL_BACKEND_MAXIMIZE_WINDOW) dlsym(backendLibrary, "crystalMaximizeWindow");
    backendRestoreWindow = (CRYSTAL_BACKEND_RESTORE_WINDOW) dlsym(backendLibrary, "crystalRestoreWindow");
    backendShowWindow = (CRYSTAL_BACKEND_SHOW_WINDOW) dlsym(backendLibrary, "crystalShowWindow");
    backendHideWindow = (CRYSTAL_BACKEND_HIDE_WINDOW) dlsym(backendLibrary, "crystalHideWindow");
    backendGetWindowState = (CRYSTAL_BACKEND_GET_WINDOW_STATE) dlsym(backendLibrary, "crystalGetWindowState");
    backendCloseWindow = (CRYSTAL_BACKEND_CLOSE_WINDOW) dlsym(backendLibrary, "crystalCloseWindow");
    backendDestroyWindow = (CRYSTAL_BACKEND_DESTROY_WINDOW) dlsym(backendLibrary, "crystalDestroyWindow");
    if (
        backendCreateWindow == 0 ||
        backendProcessEvents == 0 ||
        backendSetWindowTitle == 0 ||
        backendGetWindowTitle == 0 ||
        backendSetWindowPosition == 0 ||
        backendGetWindowPosition == 0 ||
        backendSetWindowSize == 0 ||
        backendGetWindowSize == 0 ||
        backendSetWindowSizeLimits == 0 ||
        backendGetWindowSizeLimits == 0 ||
        backendSetWindowStyle == 0 ||
        backendGetWindowStyle == 0 ||
        backendRequestWindowFocus == 0 ||
        backendRequestWindowAttention == 0 ||
        backendMinimizeWindow == 0 ||
        backendMaximizeWindow == 0 ||
        backendRestoreWindow == 0 ||
        backendShowWindow == 0 ||
        backendHideWindow == 0 ||
        backendGetWindowState == 0 ||
        backendCloseWindow == 0 ||
        backendDestroyWindow == 0
    ) {
        dlclose(backendLibrary);
        backendLibrary = 0;
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, 0);
        return FALSE;
    }
#else
    if (displayServer == CRYSTAL_DISPLAY_SERVER_WAYLAND) {
        backendCreateWindow = crystalCreateWindowWayland;
        backendProcessEvents = crystalProcessEventsWayland;
        backendSetWindowTitle = crystalSetWindowTitleWayland;
        backendGetWindowTitle = crystalGetWindowTitleWayland;
        backendSetWindowPosition = crystalSetWindowPositionWayland;
        backendGetWindowPosition = crystalGetWindowPositionWayland;
        backendSetWindowSize = crystalSetWindowSizeWayland;
        backendGetWindowSize = crystalGetWindowSizeWayland;
        backendSetWindowSizeLimits = crystalSetWindowSizeLimitsWayland;
        backendGetWindowSizeLimits = crystalGetWindowSizeLimitsWayland;
        backendSetWindowStyle = crystalSetWindowStyleWayland;
        backendGetWindowStyle = crystalGetWindowStyleWayland;
        backendRequestWindowFocus = crystalRequestWindowFocusWayland;
        backendRequestWindowAttention = crystalRequestWindowAttentionWayland;
        backendMinimizeWindow = crystalMinimizeWindowWayland;
        backendMaximizeWindow = crystalMaximizeWindowWayland;
        backendRestoreWindow = crystalRestoreWindowWayland;
        backendShowWindow = crystalShowWindowWayland;
        backendHideWindow = crystalHideWindowWayland;
        backendGetWindowState = crystalGetWindowStateWayland;
        backendCloseWindow = crystalCloseWindowWayland;
        backendDestroyWindow = crystalDestroyWindowWayland;
    } else if (displayServer == CRYSTAL_DISPLAY_SERVER_X11) {
        backendCreateWindow = crystalCreateWindowX11;
        backendProcessEvents = crystalProcessEventsX11;
        backendSetWindowTitle = crystalSetWindowTitleX11;
        backendGetWindowTitle = crystalGetWindowTitleX11;
        backendSetWindowPosition = crystalSetWindowPositionX11;
        backendGetWindowPosition = crystalGetWindowPositionX11;
        backendSetWindowSize = crystalSetWindowSizeX11;
        backendGetWindowSize = crystalGetWindowSizeX11;
        backendSetWindowSizeLimits = crystalSetWindowSizeLimitsX11;
        backendGetWindowSizeLimits = crystalGetWindowSizeLimitsX11;
        backendSetWindowStyle = crystalSetWindowStyleX11;
        backendGetWindowStyle = crystalGetWindowStyleX11;
        backendRequestWindowFocus = crystalRequestWindowFocusX11;
        backendRequestWindowAttention = crystalRequestWindowAttentionX11;
        backendMinimizeWindow = crystalMinimizeWindowX11;
        backendMaximizeWindow = crystalMaximizeWindowX11;
        backendRestoreWindow = crystalRestoreWindowX11;
        backendShowWindow = crystalShowWindowX11;
        backendHideWindow = crystalHideWindowX11;
        backendGetWindowState = crystalGetWindowStateX11;
        backendCloseWindow = crystalCloseWindowX11;
        backendDestroyWindow = crystalDestroyWindowX11;
    }
#endif

    // Report success
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
    return TRUE;
}

// TODO: document result not assigned when already initialized
// TODO: document opcode (flag 0x01) for wayland server failed to initialize
// TODO: document opcode (flag 0x02) for wayland backend failed to initialize
// TODO: document opcode (flag 0x04) for x11 server failed to initialize
// TODO: document opcode (flag 0x08) for x11 backend failed to initialize
// TODO: document detail is backend-specific error code when most recent backend initialization fails
extern "C" CRYSTAL_DISPLAY_SERVER crystalGetDisplayServer(CATALYST_RESULT* result) {
    if (displayServerInitialized) return displayServer;
    RESULT dsResult;
    BYTE detail = 0;
    BYTE flags = 0x00;

    // Wayland initialization
    if (crystalInitializeWayland(&dsResult)) {
        dsResult = RESULT();
        if (crystalInitializeBackend(CRYSTAL_DISPLAY_SERVER_WAYLAND, &dsResult)) {
            displayServer = CRYSTAL_DISPLAY_SERVER_WAYLAND;
            displayServerInitialized = TRUE;
        } else {
            detail = dsResult.status;
            flags |= 0x02;
        }
    } else {
        // ignore opcode1, no server is not an error
        if (dsResult.operation == 2) {
            flags |= 0x01;
        }
    }

    // X11 initialization
    if (displayServer == CRYSTAL_DISPLAY_SERVER_NONE) {
        dsResult = RESULT();
        if (crystalInitializeX11(&dsResult)) {
            dsResult = RESULT();
            if (crystalInitializeBackend(CRYSTAL_DISPLAY_SERVER_X11, &dsResult)) {
                displayServer = CRYSTAL_DISPLAY_SERVER_X11;
                displayServerInitialized = TRUE;
            } else {
                detail = dsResult.status;
                flags |= 0x08;
            }
        } else {
            // ignore opcode1, no server is not an error
            if (dsResult.operation == 2) {
                flags |= 0x04;
            }
        }
    }

    // Report result
    if (displayServer == CRYSTAL_DISPLAY_SERVER_NONE) displayServerInitialized = TRUE;
    if (result != 0) {
        if (flags == 0x00) {
            *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        } else {
            *result = RESULT(STATUS_CODE_WARNING, 0, flags, detail);
        }
    }
    return displayServer;
}

// TODO: document context code is set to initialization opcode when display server fails to load
// TODO: document when context code is set, opcode is backend-specific detail (error) code for failure
extern "C" CRYSTALwindow* crystalCreateWindow(CATALYST_RESULT* result) {
    if (crystalGetDisplayServer(result) == CRYSTAL_DISPLAY_SERVER_NONE) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_SYSTEM_NOT_SUPPORTED, result->operation, result->detail, 0);
        return 0;
    }
    return backendCreateWindow(result);
}

void crystalProcessEvents(CATALYST_BOOL wait) {
    if (backendProcessEvents != 0) backendProcessEvents(wait);
}

extern "C" void crystalSetWindowTitle(CRYSTALwindow* window, CATALYST_UTF8 title, CATALYST_RESULT* result) {
    if (backendSetWindowTitle != 0) {
        backendSetWindowTitle(window, title, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalGetWindowTitle(CRYSTALwindow* window, CATALYST_UTF8W title, CATALYST_NUINT capacity, CATALYST_NUINT* length, CATALYST_RESULT* result) {
    if (backendGetWindowTitle != 0) {
        backendGetWindowTitle(window, title, capacity, length, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalSetWindowPosition(CRYSTALwindow* window, CATALYST_NUINT x, CATALYST_NUINT y, CATALYST_RESULT* result) {
    if (backendSetWindowPosition != 0) {
        backendSetWindowPosition(window, x, y, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalGetWindowPosition(CRYSTALwindow* window, CATALYST_NUINT* x, CATALYST_NUINT* y, CATALYST_RESULT* result) {
    if (backendGetWindowPosition != 0) {
        backendGetWindowPosition(window, x, y, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalSetWindowSize(CRYSTALwindow* window, CATALYST_NUINT width, CATALYST_NUINT height, CATALYST_RESULT* result) {
    if (backendSetWindowSize != 0) {
        backendSetWindowSize(window, width, height, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalGetWindowSize(CRYSTALwindow* window, CATALYST_NUINT* width, CATALYST_NUINT* height, CATALYST_RESULT* result) {
    if (backendGetWindowSize != 0) {
        backendGetWindowSize(window, width, height, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalSetWindowSizeLimits(CRYSTALwindow* window, CATALYST_NUINT minWidth, CATALYST_NUINT minHeight, CATALYST_NUINT maxWidth, CATALYST_NUINT maxHeight, CATALYST_RESULT* result) {
    if (backendSetWindowSizeLimits != 0) {
        backendSetWindowSizeLimits(window, minWidth, minHeight, maxWidth, maxHeight, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalGetWindowSizeLimits(CRYSTALwindow* window, CATALYST_NUINT* minWidth, CATALYST_NUINT* minHeight, CATALYST_NUINT* maxWidth, CATALYST_NUINT* maxHeight, CATALYST_RESULT* result) {
    if (backendGetWindowSizeLimits != 0) {
        backendGetWindowSizeLimits(window, minWidth, minHeight, maxWidth, maxHeight, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalSetWindowStyle(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE style, CATALYST_RESULT* result) {
    if (backendSetWindowStyle != 0) {
        backendSetWindowStyle(window, style, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalGetWindowStyle(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE* style, CATALYST_RESULT* result) {
    if (backendGetWindowStyle != 0) {
        backendGetWindowStyle(window, style, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalRequestWindowFocus(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (backendRequestWindowFocus != 0) {
        backendRequestWindowFocus(window, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalRequestWindowAttention(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (backendRequestWindowAttention != 0) {
        backendRequestWindowAttention(window, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalMinimizeWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (backendMinimizeWindow != 0) {
        backendMinimizeWindow(window, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalMaximizeWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (backendMaximizeWindow != 0) {
        backendMaximizeWindow(window, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalRestoreWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (backendRestoreWindow != 0) {
        backendRestoreWindow(window, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalShowWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (backendShowWindow != 0) {
        backendShowWindow(window, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalHideWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (backendHideWindow != 0) {
        backendHideWindow(window, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalGetWindowState(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STATE* state) {
    if (backendGetWindowState != 0) backendGetWindowState(window, state);
}

extern "C" void crystalCloseWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (backendCloseWindow != 0) {
        backendCloseWindow(window, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

extern "C" void crystalDestroyWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (backendDestroyWindow != 0) {
        backendDestroyWindow(window, result);
    } else {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
    }
}

#endif /* TARGET_PLATFORM_LINUX */
