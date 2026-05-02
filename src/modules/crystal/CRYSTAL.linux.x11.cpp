#include "CMAKECFG.h"
#if TARGET_PLATFORM_LINUX

#include "CATCRLIB.hpp"
#include "CATMMLIB.hpp"
#include "CRYSTAL.internal.h"
#include "CRYSTAL.linux.internal.h"

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

typedef struct CRYSTALx11Platform {
    Display* display;
    Window window;
    Atom wmDeleteWindow;
    catalyst::BOOL destroying;
} CRYSTALx11Platform;

static Display* crystalX11Display = 0;
static XContext crystalX11Context = 0;
static catalyst::NUINT crystalX11WindowCount = 0;

static CRYSTALwindow* crystalX11FindWindow(Display* display, Window xwindow) {
    XPointer pointer = 0;
    if (XFindContext(display, xwindow, crystalX11Context, &pointer) != 0) {
        return 0;
    }
    return (CRYSTALwindow*) pointer;
}

static void crystalX11HandleEvent(XEvent* event) {
    if (event == 0) return;
    CRYSTALwindow* window = crystalX11FindWindow(event->xany.display, event->xany.window);
    if (window == 0) return;
    CRYSTALx11Platform* platform = (CRYSTALx11Platform*) window->platform;
    if (platform == 0) return;
    switch (event->type) {
        case ClientMessage:
            if ((Atom) event->xclient.data.l[0] == platform->wmDeleteWindow) {
                if (window->closingCallback != 0) {
                    if (!window->closingCallback(window)) {
                        return;
                    }
                }
                crystalDestroyWindowX11(window, 0);
            }
            return;
        case ConfigureNotify:
            if (window->repositionedCallback != 0) {
                window->repositionedCallback(window, (catalyst::NUINT) event->xconfigure.x, (catalyst::NUINT) event->xconfigure.y);
            }
            if (window->resizedCallback != 0) {
                window->resizedCallback(window, (catalyst::NUINT) event->xconfigure.width, (catalyst::NUINT) event->xconfigure.height);
            }
            if (window->refreshCallback != 0) {
                window->refreshCallback(window);
            }
            return;
        case Expose:
            if (event->xexpose.count == 0) {
                if (window->refreshCallback != 0) {
                    window->refreshCallback(window);
                }
                if (window->redrawCallback != 0) {
                    window->redrawCallback(window);
                }
            }
            return;
        case FocusIn:
            if (window->focusedCallback != 0) {
                window->focusedCallback(window);
            }
            if (window->refreshCallback != 0) {
                window->refreshCallback(window);
            }
            return;
        case FocusOut:
            if (window->unfocusedCallback != 0) {
                window->unfocusedCallback(window);
            }
            if (window->refreshCallback != 0) {
                window->refreshCallback(window);
            }
            return;
    }
}

// TODO: document opcode1 = failed to open display
// TODO: document opcode2 = failed to create window
// TODO: document opcode3 = failed to allocate CRYSTALwindow structure
// TODO: document opcode4 = failed to allocate platform-specific data
CRYSTALwindow* crystalCreateWindowX11(CATALYST_RESULT* result) {
    // Open the X11 display if it hasn't been opened yet
    if (crystalX11Display == 0) {
        crystalX11Display = XOpenDisplay(0);
        if (crystalX11Display == 0) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
            return 0;
        }
        crystalX11Context = XUniqueContext();
    }

    // Allocate the window object
    CRYSTALwindow* window = 0;
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

    // Create platform storage
    CRYSTALx11Platform* platform = 0;
    catalyst::alloc((void**) &platform, (catalyst::NUINT) sizeof(CRYSTALx11Platform), &allocResult);
    if (platform == 0) {
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_ALLOCATION_FAILED, 0, 2, allocResult.status);
        return 0;
    }

    // Create the window
    int screen = DefaultScreen(crystalX11Display);
    Window root = RootWindow(crystalX11Display, screen);
    Window xwindow = XCreateSimpleWindow(
        crystalX11Display,
        root,
        0, 0,
        CRYSTAL_DEFAULT_WIDTH,
        CRYSTAL_DEFAULT_HEIGHT,
        1,
        BlackPixel(crystalX11Display, screen),
        WhitePixel(crystalX11Display, screen)
    );
    if (xwindow == 0) {
        catalyst::free(platform, 0);
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 3, 0);
        return 0;
    }

    // Configure the window
    Atom wmDeleteWindow = XInternAtom(crystalX11Display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(crystalX11Display, xwindow, &wmDeleteWindow, 1);
    XStoreName(crystalX11Display, xwindow, CRYSTAL_DEFAULT_TITLE);
    XSelectInput(
        crystalX11Display,
        xwindow,
        ExposureMask |
        StructureNotifyMask |
        FocusChangeMask
    );

    // Update platform storage
    platform->display = crystalX11Display;
    platform->window = xwindow;
    platform->wmDeleteWindow = wmDeleteWindow;
    platform->destroying = CATALYST_FALSE;

    // Assign native handles
    window->native.primary = (catalyst::NUINT) xwindow;
    window->native.secondary = (catalyst::NUINT) crystalX11Display;
    window->native.tertiary = 0;
    window->platform = platform;

    // Associate the window with the CRYSTALwindow structure
    XSaveContext(crystalX11Display, xwindow, crystalX11Context, (XPointer) window);
    crystalX11WindowCount += 1;

    // Show the window
    XMapWindow(crystalX11Display, xwindow);
    XFlush(crystalX11Display);

    // Return and report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
    return window;
}

void crystalProcessEventsX11(CATALYST_BOOL wait) {
    if (crystalX11Display == 0) return;
    if (wait && XPending(crystalX11Display) == 0) {
        XEvent event;
        XNextEvent(crystalX11Display, &event);
        crystalX11HandleEvent(&event);
    }
    while (crystalX11Display != 0 && XPending(crystalX11Display) > 0) {
        XEvent event;
        XNextEvent(crystalX11Display, &event);
        crystalX11HandleEvent(&event);
    }
}

void crystalSetWindowTitleX11(CRYSTALwindow* window, CATALYST_UTF8 title, CATALYST_RESULT* result) {

}

void crystalGetWindowTitleX11(CRYSTALwindow* window, CATALYST_UTF8W title, CATALYST_NUINT capacity, CATALYST_NUINT* length, CATALYST_RESULT* result) {

}

void crystalSetWindowPositionX11(CRYSTALwindow* window, CATALYST_NUINT x, CATALYST_NUINT y, CATALYST_RESULT* result) {

}

void crystalGetWindowPositionX11(CRYSTALwindow* window, CATALYST_NUINT* x, CATALYST_NUINT* y, CATALYST_RESULT* result) {

}

void crystalSetWindowSizeX11(CRYSTALwindow* window, CATALYST_NUINT width, CATALYST_NUINT height, CATALYST_RESULT* result) {

}

void crystalGetWindowSizeX11(CRYSTALwindow* window, CATALYST_NUINT* width, CATALYST_NUINT* height, CATALYST_RESULT* result) {

}

void crystalSetWindowSizeLimitsX11(CRYSTALwindow* window, CATALYST_NUINT minWidth, CATALYST_NUINT minHeight, CATALYST_NUINT maxWidth, CATALYST_NUINT maxHeight, CATALYST_RESULT* result) {

}

void crystalGetWindowSizeLimitsX11(CRYSTALwindow* window, CATALYST_NUINT* minWidth, CATALYST_NUINT* minHeight, CATALYST_NUINT* maxWidth, CATALYST_NUINT* maxHeight, CATALYST_RESULT* result) {

}

void crystalSetWindowStyleX11(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE style, CATALYST_RESULT* result) {

}

void crystalGetWindowStyleX11(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE* style, CATALYST_RESULT* result) {

}

void crystalRequestWindowFocusX11(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalRequestWindowAttentionX11(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalMinimizeWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalMaximizeWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalRestoreWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalShowWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalHideWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalGetWindowStateX11(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STATE* state) {

}

void crystalCloseWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalDestroyWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }

    // Get the platform storage, mark as destroying
    CRYSTALx11Platform* platform = (CRYSTALx11Platform*) window->platform;
    if (platform != 0) {
        if (!platform->destroying) {
            platform->destroying = CATALYST_TRUE;

            // Destroy the native window if it exists
            if (platform->display != 0 && platform->window != 0) {
                XDeleteContext(platform->display, platform->window, crystalX11Context);
                XDestroyWindow(platform->display, platform->window);
                XFlush(platform->display);
            }
        }

        // Release the platform storage
        catalyst::free(platform, 0);
        window->platform = 0;
    }

    // Relaese native handles
    window->native.primary = 0;
    window->native.secondary = 0;
    window->native.tertiary = 0;

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
    if (crystalX11WindowCount != 0) {
        crystalX11WindowCount -= 1;
    }
    if (crystalX11WindowCount == 0 && crystalX11Display != 0) {
        XCloseDisplay(crystalX11Display);
        crystalX11Display = 0;
    }
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

#if TARGET_SHARED
extern "C" CRYSTALwindow* crystalCreateWindow(CATALYST_RESULT* result) {
    return crystalCreateWindowX11(result);
}

extern "C" void crystalProcessEvents(CATALYST_BOOL wait) {
    crystalProcessEventsX11(wait);
}

extern "C" void crystalSetWindowTitle(CRYSTALwindow* window, CATALYST_UTF8 title, CATALYST_RESULT* result) {
    crystalSetWindowTitleX11(window, title, result);
}

extern "C" void crystalGetWindowTitle(CRYSTALwindow* window, CATALYST_UTF8W title, CATALYST_NUINT capacity, CATALYST_NUINT* length, CATALYST_RESULT* result) {
    crystalGetWindowTitleX11(window, title, capacity, length, result);
}

extern "C" void crystalSetWindowPosition(CRYSTALwindow* window, CATALYST_NUINT x, CATALYST_NUINT y, CATALYST_RESULT* result) {
    crystalSetWindowPositionX11(window, x, y, result);
}

extern "C" void crystalGetWindowPosition(CRYSTALwindow* window, CATALYST_NUINT* x, CATALYST_NUINT* y, CATALYST_RESULT* result) {
    crystalGetWindowPositionX11(window, x, y, result);
}

extern "C" void crystalSetWindowSize(CRYSTALwindow* window, CATALYST_NUINT width, CATALYST_NUINT height, CATALYST_RESULT* result) {
    crystalSetWindowSizeX11(window, width, height, result);
}

extern "C" void crystalGetWindowSize(CRYSTALwindow* window, CATALYST_NUINT* width, CATALYST_NUINT* height, CATALYST_RESULT* result) {
    crystalGetWindowSizeX11(window, width, height, result);
}

extern "C" void crystalSetWindowSizeLimits(CRYSTALwindow* window, CATALYST_NUINT minWidth, CATALYST_NUINT minHeight, CATALYST_NUINT maxWidth, CATALYST_NUINT maxHeight, CATALYST_RESULT* result) {
    crystalSetWindowSizeLimitsX11(window, minWidth, minHeight, maxWidth, maxHeight, result);
}

extern "C" void crystalGetWindowSizeLimits(CRYSTALwindow* window, CATALYST_NUINT* minWidth, CATALYST_NUINT* minHeight, CATALYST_NUINT* maxWidth, CATALYST_NUINT* maxHeight, CATALYST_RESULT* result) {
    crystalGetWindowSizeLimitsX11(window, minWidth, minHeight, maxWidth, maxHeight, result);
}

extern "C" void crystalSetWindowStyle(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE style, CATALYST_RESULT* result) {
    crystalSetWindowStyleX11(window, style, result);
}

extern "C" void crystalGetWindowStyle(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE* style, CATALYST_RESULT* result) {
    crystalGetWindowStyleX11(window, style, result);
}

extern "C" void crystalRequestWindowFocus(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalRequestWindowFocusX11(window, result);
}

extern "C" void crystalRequestWindowAttention(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalRequestWindowAttentionX11(window, result);
}

extern "C" void crystalMinimizeWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalMinimizeWindowX11(window, result);
}

extern "C" void crystalMaximizeWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalMaximizeWindowX11(window, result);
}

extern "C" void crystalRestoreWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalRestoreWindowX11(window, result);
}

extern "C" void crystalShowWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalShowWindowX11(window, result);
}

extern "C" void crystalHideWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalHideWindowX11(window, result);
}

extern "C" void crystalGetWindowState(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STATE* state) {
    crystalGetWindowStateX11(window, state);
}

extern "C" void crystalCloseWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalCloseWindowX11(window, result);
}

extern "C" void crystalDestroyWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalDestroyWindowX11(window, result);
}
#endif /* TARGET_SHARED */

#endif /* TARGET_PLATFORM_LINUX */
