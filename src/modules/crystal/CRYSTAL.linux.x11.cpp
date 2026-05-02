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
    catalyst::NUINT minWidth;
    catalyst::NUINT minHeight;
    catalyst::NUINT maxWidth;
    catalyst::NUINT maxHeight;
    catalyst::BOOL destroying;
} CRYSTALx11Platform;

typedef struct CRYSTALx11WaitConfigureData {
    Window window;
} CRYSTALx11WaitConfigureData;

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

static void crystalX11GetFrameExtents(Display* display, Window xwindow, int* left, int* top) {
    *left = 0;
    *top = 0;
    Atom property = XInternAtom(display, "_NET_FRAME_EXTENTS", True);
    if (property == None) return;
    Atom actualType;
    int actualFormat;
    unsigned long itemCount;
    unsigned long bytesAfter;
    unsigned char* data = 0;
    if (XGetWindowProperty(display, xwindow, property, 0, 4, False, XA_CARDINAL, &actualType, &actualFormat, &itemCount, &bytesAfter, &data) != Success) {
        return;
    }
    if (data != 0) {
        if (actualType == XA_CARDINAL && actualFormat == 32 && itemCount >= 4) {
            long* extents = (long*) data;
            *left = (int) extents[0];
            *top = (int) extents[2];
        }
        XFree(data);
    }
}

static Bool crystalX11ConfigurePredicate(Display* display, XEvent* event, XPointer arg) {
    CRYSTALx11WaitConfigureData* data = (CRYSTALx11WaitConfigureData*) arg;
    return event != 0 &&
        event->type == ConfigureNotify &&
        event->xconfigure.window == data->window;
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

static void crystalX11WaitForConfigure(Display* display, Window xwindow) {
    XEvent event;
    CRYSTALx11WaitConfigureData data;
    data.window = xwindow;
    XSync(display, False);
    if (XCheckTypedWindowEvent(display, xwindow, ConfigureNotify, &event)) {
        crystalX11HandleEvent(&event);
        return;
    }
    XIfEvent(display, &event, crystalX11ConfigurePredicate, (XPointer) &data);
    crystalX11HandleEvent(&event);
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
    platform->minWidth = 0;
    platform->minHeight = 0;
    platform->maxWidth = 0;
    platform->maxHeight = 0;
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
    // Validate arguments
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (title == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (window->native.primary == 0 || window->native.secondary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the xwindow reference
    Display* display = (Display*) window->native.secondary;
    Window xwindow = (Window) window->native.primary;

    // Calculate the title byte length
    catalyst::NUINT length = 0;
    while (title[length] != 0) {
        length += 1;
    }
    Atom utf8String = XInternAtom(display, "UTF8_STRING", False);
    Atom netWmName = XInternAtom(display, "_NET_WM_NAME", False);

    // Set the window title
    XChangeProperty(
        display,
        xwindow,
        netWmName,
        utf8String,
        8,
        PropModeReplace,
        (const unsigned char*) title,
        (int) length
    );
    XStoreName(display, xwindow, (const char*) title);
    crystalX11WaitForConfigure(display, xwindow);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalGetWindowTitleX11(CRYSTALwindow* window, CATALYST_UTF8W title, CATALYST_NUINT capacity, CATALYST_NUINT* length, CATALYST_RESULT* result) {
    // Validate arguments
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
    title[0] = 0;
    if (length == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 3);
        return;
    }
    *length = 0;
    if (window->native.primary == 0 || window->native.secondary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the xwindow reference
    Display* display = (Display*) window->native.secondary;
    Window xwindow = (Window) window->native.primary;
    XSync(display, False);

    // Fetch the native title
    char* nativeTitle = 0;
    if (XFetchName(display, xwindow, &nativeTitle) == 0 || nativeTitle == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS_NOOP, 0, 0, 0);
        return;
    }

    // Calculate the title byte length
    catalyst::NUINT required = 0;
    while (nativeTitle[required] != '\0') {
        required += 1;
    }
    *length = required;

    // Copy the title to the output buffer, truncating if necessary
    catalyst::NUINT writable = capacity - 1;
    catalyst::NUINT i = 0;
    while (i < writable && i < required) {
        title[i] = (catalyst::BYTE) nativeTitle[i];
        i += 1;
    }
    title[i] = 0;

    // Free the native title, check for overflow, and report success
    XFree(nativeTitle);
    if (i < required) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_WARNING_PARTIAL, 0, 0, 0);
        return;
    }
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalSetWindowPositionX11(CRYSTALwindow* window, CATALYST_NUINT x, CATALYST_NUINT y, CATALYST_RESULT* result) {
    // Validate arguments
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (window->native.primary == 0 || window->native.secondary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the xwindow reference
    Display* display = (Display*) window->native.secondary;
    Window xwindow = (Window) window->native.primary;

    // Set the window position
    XMoveWindow(display, xwindow, (int) x, (int) y);
    crystalX11WaitForConfigure(display, xwindow);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalGetWindowPositionX11(CRYSTALwindow* window, CATALYST_NUINT* x, CATALYST_NUINT* y, CATALYST_RESULT* result) {
    // Validate arguments
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
    if (window->native.primary == 0 || window->native.secondary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the xwindow reference
    Display* display = (Display*) window->native.secondary;
    Window xwindow = (Window) window->native.primary;
    XSync(display, False);

    // Translate the window coordinates to root coordinates
    int nativeX = 0;
    int nativeY = 0;
    Window child = 0;
    XTranslateCoordinates(
        display,
        xwindow,
        RootWindow(display, DefaultScreen(display)),
        0,
        0,
        &nativeX,
        &nativeY,
        &child
    );
    int frameLeft = 0;
    int frameTop = 0;
    crystalX11GetFrameExtents(display, xwindow, &frameLeft, &frameTop);
    nativeX -= frameLeft;
    nativeY -= frameTop;
    *x = nativeX < 0 ? 0 : (catalyst::NUINT) nativeX;
    *y = nativeY < 0 ? 0 : (catalyst::NUINT) nativeY;

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalSetWindowSizeX11(CRYSTALwindow* window, CATALYST_NUINT width, CATALYST_NUINT height, CATALYST_RESULT* result) {
    // Validate arguments
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
    if (window->native.primary == 0 || window->native.secondary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the platform storage and apply the size limits
    CRYSTALx11Platform* platform = (CRYSTALx11Platform*) window->platform;
    if (platform != 0) {
        if (platform->minWidth != 0 && width < platform->minWidth) width = platform->minWidth;
        if (platform->minHeight != 0 && height < platform->minHeight) height = platform->minHeight;
        if (platform->maxWidth != 0 && width > platform->maxWidth) width = platform->maxWidth;
        if (platform->maxHeight != 0 && height > platform->maxHeight) height = platform->maxHeight;
    }

    // Get the xwindow reference
    Display* display = (Display*) window->native.secondary;
    Window xwindow = (Window) window->native.primary;

    // Set the window size
    XResizeWindow(display, xwindow, (unsigned int) width, (unsigned int) height);
    crystalX11WaitForConfigure(display, xwindow);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalGetWindowSizeX11(CRYSTALwindow* window, CATALYST_NUINT* width, CATALYST_NUINT* height, CATALYST_RESULT* result) {
    // Validate arguments
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
    if (window->native.primary == 0 || window->native.secondary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the xwindow reference
    Display* display = (Display*) window->native.secondary;
    Window xwindow = (Window) window->native.primary;
    XSync(display, False);

    // Query the window attributes for the size
    XWindowAttributes attributes;
    if (XGetWindowAttributes(display, xwindow, &attributes) == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
        return;
    }
    *width = (catalyst::NUINT) attributes.width;
    *height = (catalyst::NUINT) attributes.height;

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalSetWindowSizeLimitsX11(CRYSTALwindow* window, CATALYST_NUINT minWidth, CATALYST_NUINT minHeight, CATALYST_NUINT maxWidth, CATALYST_NUINT maxHeight, CATALYST_RESULT* result) {
    // Validate arguments
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
    if (window->native.primary == 0 || window->native.secondary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the platform storage
    CRYSTALx11Platform* platform = (CRYSTALx11Platform*) window->platform;
    if (platform == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 1);
        return;
    }

    // Get the xwindow reference
    Display* display = (Display*) window->native.secondary;
    Window xwindow = (Window) window->native.primary;

    // Update the size limits in platform storage
    platform->minWidth = minWidth;
    platform->minHeight = minHeight;
    platform->maxWidth = maxWidth;
    platform->maxHeight = maxHeight;

    // Update the size hints for the window manager
    XSizeHints hints;
    hints.flags = 0;
    if (minWidth != 0 || minHeight != 0) {
        hints.flags |= PMinSize;
        hints.min_width = (int) minWidth;
        hints.min_height = (int) minHeight;
    }
    if (maxWidth != 0 || maxHeight != 0) {
        hints.flags |= PMaxSize;
        hints.max_width = (int) maxWidth;
        hints.max_height = (int) maxHeight;
    }
    XSetWMNormalHints(display, xwindow, &hints);
    XFlush(display);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalGetWindowSizeLimitsX11(CRYSTALwindow* window, CATALYST_NUINT* minWidth, CATALYST_NUINT* minHeight, CATALYST_NUINT* maxWidth, CATALYST_NUINT* maxHeight, CATALYST_RESULT* result) {
    // Validate arguments
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

    // Get the platform storage
    CRYSTALx11Platform* platform = (CRYSTALx11Platform*) window->platform;
    if (platform == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 1);
        return;
    }

    // Return the size limits and report success
    *minWidth = platform->minWidth;
    *minHeight = platform->minHeight;
    *maxWidth = platform->maxWidth;
    *maxHeight = platform->maxHeight;
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
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
