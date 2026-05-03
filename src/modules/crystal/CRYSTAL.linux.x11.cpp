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
    Atom wmDeleteWindow;
    catalyst::BOOL destroying;
} CRYSTALx11Platform;

typedef struct CRYSTALx11MotifHints {
    catalyst::BYTE flags;
    catalyst::BYTE functions;
    catalyst::BYTE decorations;
    catalyst::BYTE inputMode;
    catalyst::BYTE status;
} CRYSTALx11MotifHints;

static const catalyst::BYTE CRYSTAL_X11_MWM_HINTS_FUNCTIONS   = (catalyst::BYTE) 0x01;
static const catalyst::BYTE CRYSTAL_X11_MWM_HINTS_DECORATIONS = (catalyst::BYTE) 0x02;
static const catalyst::BYTE CRYSTAL_X11_MWM_FUNC_RESIZE       = (catalyst::BYTE) 0x02;
static const catalyst::BYTE CRYSTAL_X11_MWM_FUNC_MOVE         = (catalyst::BYTE) 0x04;
static const catalyst::BYTE CRYSTAL_X11_MWM_FUNC_MINIMIZE     = (catalyst::BYTE) 0x08;
static const catalyst::BYTE CRYSTAL_X11_MWM_FUNC_MAXIMIZE     = (catalyst::BYTE) 0x10;
static const catalyst::BYTE CRYSTAL_X11_MWM_FUNC_CLOSE        = (catalyst::BYTE) 0x20;
static const catalyst::BYTE CRYSTAL_X11_MWM_DECOR_BORDER      = (catalyst::BYTE) 0x02;
static const catalyst::BYTE CRYSTAL_X11_MWM_DECOR_RESIZEH     = (catalyst::BYTE) 0x04;
static const catalyst::BYTE CRYSTAL_X11_MWM_DECOR_TITLE       = (catalyst::BYTE) 0x08;
static const catalyst::BYTE CRYSTAL_X11_MWM_DECOR_MENU        = (catalyst::BYTE) 0x10;
static const catalyst::BYTE CRYSTAL_X11_MWM_DECOR_MINIMIZE    = (catalyst::BYTE) 0x20;
static const catalyst::BYTE CRYSTAL_X11_MWM_DECOR_MAXIMIZE    = (catalyst::BYTE) 0x40;
static const catalyst::BYTE CRYSTAL_X11_NET_WM_STATE_REMOVE   = (catalyst::BYTE) 0x00;
static const catalyst::BYTE CRYSTAL_X11_NET_WM_STATE_ADD      = (catalyst::BYTE) 0x01;

typedef struct CRYSTALx11WaitConfigureData {
    Window window;
} CRYSTALx11WaitConfigureData;

static Display* crystalX11Display = 0;
static XContext crystalX11Context = 0;
static catalyst::NUINT crystalX11WindowCount = 0;

static Bool crystalX11ConfigurePredicate(Display* display, XEvent* event, XPointer arg) {
    CRYSTALx11WaitConfigureData* data = (CRYSTALx11WaitConfigureData*) arg;
    return event != 0 && event->type == ConfigureNotify && event->xconfigure.window == data->window;
}

#include <stdio.h>

static catalyst::BOOL crystalX11WindowHasAtom(Display* display, Window xwindow, Atom property, Atom atom) {
    if (property == None || atom == None) {
        return CATALYST_FALSE;
    }

    // Check if the atom exists for the window
    Atom actualType;
    int actualFormat;
    unsigned long itemCount;
    unsigned long bytesAfter;
    unsigned char* data = 0;
    if (XGetWindowProperty(display, xwindow, property, 0, 1024, False, XA_ATOM, &actualType, &actualFormat, &itemCount, &bytesAfter, &data) != Success) {
        return CATALYST_FALSE;
    }

    // Iterate through the atoms to find a match
    catalyst::BOOL found = CATALYST_FALSE;
    if (data != 0) {
        if (actualType == XA_ATOM && actualFormat == 32) {
            Atom* atoms = (Atom*) data;
            catalyst::NUINT i = 0;
            while (i < (catalyst::NUINT) itemCount) {
                if (atoms[i] == atom) {
                    found = CATALYST_TRUE;
                    break;
                }
                i += 1;
            }
        }
        XFree(data);
    }
    return found;
}

static void crystalX11ApplyMotifStyle(Display* display, Window xwindow, CRYSTAL_PROPERTIES_STYLE style) {
    Atom property = XInternAtom(display, "_MOTIF_WM_HINTS", False);
    if (property == None) {
        return;
    }

    // Determine requested hints and construct the hints structure
    CRYSTALx11MotifHints hints;
    hints.flags = (catalyst::BYTE) (CRYSTAL_X11_MWM_HINTS_FUNCTIONS | CRYSTAL_X11_MWM_HINTS_DECORATIONS);
    hints.functions = (catalyst::BYTE) (CRYSTAL_X11_MWM_FUNC_MOVE | CRYSTAL_X11_MWM_FUNC_CLOSE);
    hints.decorations = (catalyst::BYTE) 0x00;
    hints.inputMode = (catalyst::BYTE) 0x00;
    hints.status = (catalyst::BYTE) 0x00;
    if ((style & CRYSTAL_PROPERTIES_STYLE_RESIZABLE) != 0) {
        hints.functions = (catalyst::BYTE) (hints.functions | CRYSTAL_X11_MWM_FUNC_RESIZE);
    }
    if ((style & CRYSTAL_PROPERTIES_STYLE_MINIMIZABLE) != 0) {
        hints.functions = (catalyst::BYTE) (hints.functions | CRYSTAL_X11_MWM_FUNC_MINIMIZE);
    }
    if ((style & CRYSTAL_PROPERTIES_STYLE_MAXIMIZABLE) != 0) {
        hints.functions = (catalyst::BYTE) (hints.functions | CRYSTAL_X11_MWM_FUNC_MAXIMIZE);
    }
    if ((style & CRYSTAL_PROPERTIES_STYLE_DECORATED) != 0) {
        hints.decorations = (catalyst::BYTE) (
            CRYSTAL_X11_MWM_DECOR_BORDER |
            CRYSTAL_X11_MWM_DECOR_TITLE |
            CRYSTAL_X11_MWM_DECOR_MENU
        );
        if ((style & CRYSTAL_PROPERTIES_STYLE_RESIZABLE) != 0) {
            hints.decorations = (catalyst::BYTE) (hints.decorations | CRYSTAL_X11_MWM_DECOR_RESIZEH);
        }
        if ((style & CRYSTAL_PROPERTIES_STYLE_MINIMIZABLE) != 0) {
            hints.decorations = (catalyst::BYTE) (hints.decorations | CRYSTAL_X11_MWM_DECOR_MINIMIZE);
        }
        if ((style & CRYSTAL_PROPERTIES_STYLE_MAXIMIZABLE) != 0) {
            hints.decorations = (catalyst::BYTE) (hints.decorations | CRYSTAL_X11_MWM_DECOR_MAXIMIZE);
        }
    }

    // Write hints and apply property change
    long nativeHints[5];
    nativeHints[0] = (long) hints.flags;
    nativeHints[1] = (long) hints.functions;
    nativeHints[2] = (long) hints.decorations;
    nativeHints[3] = (long) hints.inputMode;
    nativeHints[4] = (long) hints.status;
    XChangeProperty(
        display,
        xwindow,
        property,
        property,
        32,
        PropModeReplace,
        (const unsigned char*) nativeHints,
        5
    );
}

static void crystalX11ApplyAllowedActions(Display* display, Window xwindow, CRYSTAL_PROPERTIES_STYLE style) {
    Atom property = XInternAtom(display, "_NET_WM_ALLOWED_ACTIONS", False);
    if (property == None) {
        return;
    }

    // Determine allowed actions based on style
    long nativeActions[8];
    catalyst::BYTE count = 0;
    nativeActions[count] = (long) XInternAtom(display, "_NET_WM_ACTION_MOVE", False);
    count += 1;
    nativeActions[count] = (long) XInternAtom(display, "_NET_WM_ACTION_CLOSE", False);
    count += 1;
    if ((style & CRYSTAL_PROPERTIES_STYLE_RESIZABLE) != 0) {
        nativeActions[count] = (long) XInternAtom(display, "_NET_WM_ACTION_RESIZE", False);
        count += 1;
    }
    if ((style & CRYSTAL_PROPERTIES_STYLE_MINIMIZABLE) != 0) {
        nativeActions[count] = (long) XInternAtom(display, "_NET_WM_ACTION_MINIMIZE", False);
        count += 1;
    }
    if ((style & CRYSTAL_PROPERTIES_STYLE_MAXIMIZABLE) != 0) {
        nativeActions[count] = (long) XInternAtom(display, "_NET_WM_ACTION_MAXIMIZE_HORZ", False);
        count += 1;
        nativeActions[count] = (long) XInternAtom(display, "_NET_WM_ACTION_MAXIMIZE_VERT", False);
        count += 1;
    }

    // Apply the allowed actions property change
    XChangeProperty(
        display,
        xwindow,
        property,
        XA_ATOM,
        32,
        PropModeReplace,
        (const unsigned char*) nativeActions,
        (int) count
    );
}

static catalyst::BOOL crystalX11SendNetWmState(Display* display, Window xwindow, catalyst::BYTE action, Atom first, Atom second) {
    if (display == 0 || xwindow == 0 || first == None) {
        return CATALYST_FALSE;
    }

    // Send a _NET_WM_STATE client message to the root window to change the state of the window
    XClientMessageEvent client;
    client.type = ClientMessage;
    client.serial = 0;
    client.send_event = True;
    client.display = display;
    client.window = xwindow;
    client.message_type = XInternAtom(display, "_NET_WM_STATE", False);
    client.format = 32;
    client.data.l[0] = (long) action;
    client.data.l[1] = (long) first;
    client.data.l[2] = (long) second;
    client.data.l[3] = 1;
    client.data.l[4] = 0;
    XEvent event;
    event.xclient = client;
    Window root = RootWindow(display, DefaultScreen(display));
    return XSendEvent(display, root, False, SubstructureRedirectMask | SubstructureNotifyMask, &event) != 0 ? CATALYST_TRUE : CATALYST_FALSE;
}

static void crystalX11HandleEvent(XEvent* event) {
    if (event == 0) return;
    XPointer pointer = 0;
    if (XFindContext(event->xany.display, event->xany.window, crystalX11Context, &pointer) != 0) return;
    CRYSTALwindow* window = (CRYSTALwindow*) pointer;
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
    Atom windowType = XInternAtom(crystalX11Display, "_NET_WM_WINDOW_TYPE", False);
    Atom normalType = XInternAtom(crystalX11Display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
    long nativeWindowType = (long) normalType;
    XChangeProperty(
        crystalX11Display,
        xwindow,
        windowType,
        XA_ATOM,
        32,
        PropModeReplace,
        (const unsigned char*) &nativeWindowType,
        1
    );

    // Update platform storage
    platform->wmDeleteWindow = wmDeleteWindow;
    platform->destroying = CATALYST_FALSE;

    // Assign native handles
    window->native.primary = (catalyst::NUINT) xwindow;
    window->native.secondary = (catalyst::NUINT) crystalX11Display;
    window->native.tertiary = 0; // TODO: graphical thing???
    window->platform = platform;

    // Apply initial window-manager hints
    crystalX11ApplyMotifStyle(
        crystalX11Display,
        xwindow,
        (CRYSTAL_PROPERTIES_STYLE) (
            CRYSTAL_PROPERTIES_STYLE_DECORATED |
            CRYSTAL_PROPERTIES_STYLE_MINIMIZABLE |
            CRYSTAL_PROPERTIES_STYLE_MAXIMIZABLE |
            CRYSTAL_PROPERTIES_STYLE_RESIZABLE
        )
    );
    crystalX11ApplyAllowedActions(
        crystalX11Display,
        xwindow,
        (CRYSTAL_PROPERTIES_STYLE) (
            CRYSTAL_PROPERTIES_STYLE_DECORATED |
            CRYSTAL_PROPERTIES_STYLE_MINIMIZABLE |
            CRYSTAL_PROPERTIES_STYLE_MAXIMIZABLE |
            CRYSTAL_PROPERTIES_STYLE_RESIZABLE
        )
    );

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

    // Adjust for the window frame extents if the window manager provides them
    catalyst::NINT frameLeft = 0;
    catalyst::NINT frameTop = 0;
    Atom frameExtents = XInternAtom(display, "_NET_FRAME_EXTENTS", True);
    if (frameExtents != None) {
        Atom actualType;
        int actualFormat;
        unsigned long itemCount;
        unsigned long bytesAfter;
        unsigned char* data = 0;
        if (XGetWindowProperty(display, xwindow, frameExtents, 0, 4, False, XA_CARDINAL, &actualType, &actualFormat, &itemCount, &bytesAfter, &data) == Success) {
            if (data != 0) {
                if (actualType == XA_CARDINAL && actualFormat == 32 && itemCount >= 4) {
                    long* nativeExtents = (long*) data;
                    frameLeft = (catalyst::NINT) nativeExtents[0];
                    frameTop = (catalyst::NINT) nativeExtents[2];
                }
                XFree(data);
            }
        }
    }
    nativeX -= (int) frameLeft;
    nativeY -= (int) frameTop;
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

    // Get the xwindow reference
    Display* display = (Display*) window->native.secondary;
    Window xwindow = (Window) window->native.primary;

    // Query the window manager size limits and apply them before resizing
    XSizeHints hints;
    long supplied = 0;
    if (XGetWMNormalHints(display, xwindow, &hints, &supplied) != 0) {
        if ((hints.flags & PMinSize) != 0) {
            if (hints.min_width > 0 && width < (catalyst::NUINT) hints.min_width) width = (catalyst::NUINT) hints.min_width;
            if (hints.min_height > 0 && height < (catalyst::NUINT) hints.min_height) height = (catalyst::NUINT) hints.min_height;
        }
        if ((hints.flags & PMaxSize) != 0) {
            if (hints.max_width > 0 && width > (catalyst::NUINT) hints.max_width) width = (catalyst::NUINT) hints.max_width;
            if (hints.max_height > 0 && height > (catalyst::NUINT) hints.max_height) height = (catalyst::NUINT) hints.max_height;
        }
    }

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

    // Get the xwindow reference
    Display* display = (Display*) window->native.secondary;
    Window xwindow = (Window) window->native.primary;

    // Query the window manager size hints
    XSizeHints hints;
    long supplied = 0;
    if (XGetWMNormalHints(display, xwindow, &hints, &supplied) == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }
    if ((hints.flags & PMinSize) != 0) {
        *minWidth = hints.min_width < 0 ? 0 : (catalyst::NUINT) hints.min_width;
        *minHeight = hints.min_height < 0 ? 0 : (catalyst::NUINT) hints.min_height;
    }
    if ((hints.flags & PMaxSize) != 0) {
        *maxWidth = hints.max_width < 0 ? 0 : (catalyst::NUINT) hints.max_width;
        *maxHeight = hints.max_height < 0 ? 0 : (catalyst::NUINT) hints.max_height;
    }

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalSetWindowStyleX11(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE style, CATALYST_RESULT* result) {
    // Validate arguments
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
    if (window->native.primary == 0 || window->native.secondary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the xwindow reference and apply the requested window-manager style
    Display* display = (Display*) window->native.secondary;
    Window xwindow = (Window) window->native.primary;
    crystalX11ApplyMotifStyle(display, xwindow, style);
    crystalX11ApplyAllowedActions(display, xwindow, style);
    XFlush(display);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalGetWindowStyleX11(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE* style, CATALYST_RESULT* result) {
    // Validate arguments
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (style == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    *style = (CRYSTAL_PROPERTIES_STYLE) (
        CRYSTAL_PROPERTIES_STYLE_DECORATED |
        CRYSTAL_PROPERTIES_STYLE_MINIMIZABLE |
        CRYSTAL_PROPERTIES_STYLE_MAXIMIZABLE |
        CRYSTAL_PROPERTIES_STYLE_RESIZABLE
    );
    if (window->native.primary == 0 || window->native.secondary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Query the Motif window-manager hints if they exist
    Display* display = (Display*) window->native.secondary;
    Window xwindow = (Window) window->native.primary;
    Atom property = XInternAtom(display, "_MOTIF_WM_HINTS", True);
    if (property == None) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }

    // Fetch the Motif hints property data
    Atom actualType;
    int actualFormat;
    unsigned long itemCount;
    unsigned long bytesAfter;
    unsigned char* data = 0;
    if (XGetWindowProperty(display, xwindow, property, 0, 5, False, property, &actualType, &actualFormat, &itemCount, &bytesAfter, &data) != Success) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
        return;
    }

    // Parse the Motif hints and update the style output accordingly, then free the property data
    if (data != 0) {
        if (actualType == property && actualFormat == 32 && itemCount >= 3) {
            long* nativeHints = (long*) data;
            catalyst::BYTE flags = (catalyst::BYTE) nativeHints[0];
            catalyst::BYTE functions = (catalyst::BYTE) nativeHints[1];
            catalyst::BYTE decorations = (catalyst::BYTE) nativeHints[2];
            *style = CRYSTAL_PROPERTIES_STYLE_NONE;
            if ((flags & CRYSTAL_X11_MWM_HINTS_FUNCTIONS) != 0) {
                if ((functions & CRYSTAL_X11_MWM_FUNC_MINIMIZE) != 0) {
                    *style = (CRYSTAL_PROPERTIES_STYLE) (*style | CRYSTAL_PROPERTIES_STYLE_MINIMIZABLE);
                }
                if ((functions & CRYSTAL_X11_MWM_FUNC_MAXIMIZE) != 0) {
                    *style = (CRYSTAL_PROPERTIES_STYLE) (*style | CRYSTAL_PROPERTIES_STYLE_MAXIMIZABLE);
                }
                if ((functions & CRYSTAL_X11_MWM_FUNC_RESIZE) != 0) {
                    *style = (CRYSTAL_PROPERTIES_STYLE) (*style | CRYSTAL_PROPERTIES_STYLE_RESIZABLE);
                }
            }
            if ((flags & CRYSTAL_X11_MWM_HINTS_DECORATIONS) != 0) {
                if ((decorations & (
                    CRYSTAL_X11_MWM_DECOR_BORDER |
                    CRYSTAL_X11_MWM_DECOR_TITLE |
                    CRYSTAL_X11_MWM_DECOR_MENU
                )) != 0) {
                    *style = (CRYSTAL_PROPERTIES_STYLE) (*style | CRYSTAL_PROPERTIES_STYLE_DECORATED);
                }
            }
        }
        XFree(data);
    }

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalRequestWindowFocusX11(CRYSTALwindow* window, CATALYST_RESULT* result) {
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

    // Ask the window manager to raise/focus the window
    XMapRaised(display, xwindow);
    XRaiseWindow(display, xwindow);
    XSetInputFocus(display, xwindow, RevertToParent, CurrentTime);
    XFlush(display);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalRequestWindowAttentionX11(CRYSTALwindow* window, CATALYST_RESULT* result) {
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

    // Request urgency/attention using both the traditional WM hint and EWMH
    XWMHints* hints = XGetWMHints(display, xwindow);
    if (hints == 0) {
        hints = XAllocWMHints();
    }
    if (hints != 0) {
        hints->flags |= XUrgencyHint;
        XSetWMHints(display, xwindow, hints);
        XFree(hints);
    }
    Atom demandsAttention = XInternAtom(display, "_NET_WM_STATE_DEMANDS_ATTENTION", False);
    crystalX11SendNetWmState(display, xwindow, CRYSTAL_X11_NET_WM_STATE_ADD, demandsAttention, None);
    XFlush(display);

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalMinimizeWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result) {
    // Validate arguments
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (window->native.primary == 0 || window->native.secondary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the xwindow reference and iconify/minimize the window
    Display* display = (Display*) window->native.secondary;
    Window xwindow = (Window) window->native.primary;
    if (XIconifyWindow(display, xwindow, DefaultScreen(display)) == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
        return;
    }
    XFlush(display);
    if (window->minimizedCallback != 0) {
        window->minimizedCallback(window);
    }
    if (window->refreshCallback != 0) {
        window->refreshCallback(window);
    }

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalMaximizeWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result) {
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

    // Restore from minimized/hidden state first, then request maximized state
    XMapRaised(display, xwindow);
    Atom maximizedHorz = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    Atom maximizedVert = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    if (!crystalX11SendNetWmState(display, xwindow, CRYSTAL_X11_NET_WM_STATE_ADD, maximizedHorz, maximizedVert)) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
        return;
    }
    XFlush(display);
    if (window->maximizedCallback != 0) {
        window->maximizedCallback(window);
    }
    if (window->refreshCallback != 0) {
        window->refreshCallback(window);
    }

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalRestoreWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result) {
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

    // Deiconify/show and request removal of maximized state
    XMapRaised(display, xwindow);
    Atom maximizedHorz = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    Atom maximizedVert = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    crystalX11SendNetWmState(display, xwindow, CRYSTAL_X11_NET_WM_STATE_REMOVE, maximizedHorz, maximizedVert);
    XFlush(display);
    if (window->restoredCallback != 0) {
        window->restoredCallback(window);
    }
    if (window->refreshCallback != 0) {
        window->refreshCallback(window);
    }

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalShowWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result) {
    // Validate arguments
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (window->native.primary == 0 || window->native.secondary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the xwindow reference and map/show it
    Display* display = (Display*) window->native.secondary;
    Window xwindow = (Window) window->native.primary;
    XMapRaised(display, xwindow);
    XFlush(display);
    if (window->shownCallback != 0) {
        window->shownCallback(window);
    }
    if (window->refreshCallback != 0) {
        window->refreshCallback(window);
    }

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalHideWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result) {
    // Validate arguments
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (window->native.primary == 0 || window->native.secondary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Get the xwindow reference and unmap/hide it
    Display* display = (Display*) window->native.secondary;
    Window xwindow = (Window) window->native.primary;
    XUnmapWindow(display, xwindow);
    XFlush(display);
    if (window->hiddenCallback != 0) {
        window->hiddenCallback(window);
    }
    if (window->refreshCallback != 0) {
        window->refreshCallback(window);
    }

    // Report success
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

void crystalGetWindowStateX11(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STATE* state) {
    if (state == 0) {
        return;
    }
    *state = CRYSTAL_PROPERTIES_STATE_NONE;
    if (window == 0) {
        return;
    }
    if (window->native.primary == 0 || window->native.secondary == 0) {
        return;
    }

    // Get the xwindow reference
    Display* display = (Display*) window->native.secondary;
    Window xwindow = (Window) window->native.primary;
    XSync(display, False);

    // Determine whether this window, or one of its child windows, owns focus
    catalyst::BOOL focusedState = CATALYST_FALSE;
    Window focused = 0;
    int revert = 0;
    XGetInputFocus(display, &focused, &revert);
    if (focused != 0 && focused != PointerRoot) {
        if (focused == xwindow) {
            focusedState = CATALYST_TRUE;
        }
        else {
            Window current = focused;
            while (current != 0) {
                Window treeRoot = 0;
                Window parent = 0;
                Window* children = 0;
                unsigned int childCount = 0;
                if (XQueryTree(display, current, &treeRoot, &parent, &children, &childCount) == 0) {
                    if (children != 0) XFree(children);
                    break;
                }
                if (children != 0) XFree(children);
                if (parent == xwindow) {
                    focusedState = CATALYST_TRUE;
                    break;
                }
                if (parent == 0 || parent == treeRoot || parent == current) {
                    break;
                }
                current = parent;
            }
        }
    }
    if (focusedState) {
        *state = (CRYSTAL_PROPERTIES_STATE) (*state | CRYSTAL_PROPERTIES_STATE_FOCUSED);
    }
    else {
        *state = (CRYSTAL_PROPERTIES_STATE) (*state | CRYSTAL_PROPERTIES_STATE_UNFOCUSED);
    }

    // Query whether the window manager currently considers the window minimized
    catalyst::BOOL minimized = CATALYST_FALSE;
    Atom wmState = XInternAtom(display, "WM_STATE", True);
    if (wmState != None) {
        Atom actualType;
        int actualFormat;
        unsigned long itemCount;
        unsigned long bytesAfter;
        unsigned char* data = 0;
        if (XGetWindowProperty(display, xwindow, wmState, 0, 2, False, wmState, &actualType, &actualFormat, &itemCount, &bytesAfter, &data) == Success) {
            if (data != 0) {
                if (actualType == wmState && actualFormat == 32 && itemCount >= 1) {
                    long* nativeValues = (long*) data;
                    if (nativeValues[0] == IconicState) {
                        minimized = CATALYST_TRUE;
                    }
                }
                XFree(data);
            }
        }
    }
    if (minimized) {
        *state = (CRYSTAL_PROPERTIES_STATE) (*state | CRYSTAL_PROPERTIES_STATE_MINIMIZED);
    }

    // Query maximized state directly from the EWMH state property
    Atom netWmState = XInternAtom(display, "_NET_WM_STATE", True);
    Atom maximizedHorz = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", True);
    Atom maximizedVert = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", True);
    if (crystalX11WindowHasAtom(display, xwindow, netWmState, maximizedHorz) && crystalX11WindowHasAtom(display, xwindow, netWmState, maximizedVert)) {
        *state = (CRYSTAL_PROPERTIES_STATE) (*state | CRYSTAL_PROPERTIES_STATE_MAXIMIZED);
    }

    XWindowAttributes attributes;
    if (XGetWindowAttributes(display, xwindow, &attributes) != 0 && attributes.map_state == IsViewable && !minimized) {
        *state = (CRYSTAL_PROPERTIES_STATE) (*state | CRYSTAL_PROPERTIES_STATE_VISIBLE);
    } else {
        *state = (CRYSTAL_PROPERTIES_STATE) (*state | CRYSTAL_PROPERTIES_STATE_HIDDEN);
    }
}

void crystalCloseWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result) {
    // Validate arguments
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (window->native.primary == 0 || window->native.secondary == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Allow the close request to be canceled
    if (window->closingCallback != 0) {
        if (!window->closingCallback(window)) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS_NOOP, 0, 0, 0);
            return;
        }
    }

    // Close accepted, so destroy the window
    crystalDestroyWindowX11(window, result);
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
            if (window->native.primary != 0 && window->native.secondary != 0) {
                Display* display = (Display*) window->native.secondary;
                Window xwindow = (Window) window->native.primary;
                XDeleteContext(display, xwindow, crystalX11Context);
                XDestroyWindow(display, xwindow);
                XFlush(display);
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
