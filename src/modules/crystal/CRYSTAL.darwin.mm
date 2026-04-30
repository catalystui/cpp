#include "CMAKECFG.h"
#if TARGET_PLATFORM_DARWIN

#include "CATCRLIB.hpp"
#include "CATMMLIB.hpp"
#include "CATTELIB.hpp"
#include "CRYSTAL.internal.h"

#import <Cocoa/Cocoa.h>

#ifndef __has_feature
    #define __has_feature(x) 0
#endif

@interface CrystalWindowDelegate : NSObject<NSWindowDelegate> {
@public
    CRYSTALwindow* window;
}
@end

@implementation CrystalWindowDelegate

- (BOOL) windowShouldClose: (id) sender {
    if (window != 0 && window->closingCallback != 0) {
        return window->closingCallback(window) ? YES : NO;
    }
    return YES;
}

@end

// TODO: Document opcode 1 for failed to create NSWindow
// TODO: Document opcode 2 for failed to create NSView
// TODO: Document opcode 3 for failed to create delegate
extern "C" CRYSTALwindow* crystalCreateWindow(catalyst::RESULT* result) {
    @autoreleasepool {
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

        // Create the application
        NSApplication* nsApplication = [NSApplication sharedApplication];
        [nsApplication setActivationPolicy:NSApplicationActivationPolicyRegular];
        [nsApplication finishLaunching];

        // Create the window
        NSRect nsRect = NSMakeRect(0, 0, CRYSTAL_DEFAULT_WIDTH, CRYSTAL_DEFAULT_HEIGHT);
        NSWindow* nsWindow = [[NSWindow alloc]
            initWithContentRect:nsRect
            styleMask:(
                NSWindowStyleMaskTitled |
                NSWindowStyleMaskClosable |
                NSWindowStyleMaskResizable |
                NSWindowStyleMaskMiniaturizable)
            backing:NSBackingStoreBuffered
            defer:NO];
        if (nsWindow == nil) {
            catalyst::free(window, 0);
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
            return 0;
        }

        // Create the content view
        NSView* nsView = [[NSView alloc] initWithFrame:nsRect];
        if (nsView == nil) {
#if !__has_feature(objc_arc)
            [nsWindow release];
#endif
            catalyst::free(window, 0);
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, 0);
            return 0;
        }
        [nsWindow setContentView:nsView];
#if !__has_feature(objc_arc)
        [nsView release];
#endif

        // Create the delegate
        CrystalWindowDelegate* delegate = [[CrystalWindowDelegate alloc] init];
        if (delegate == nil) {
            [nsWindow setContentView:nil];
#if !__has_feature(objc_arc)
            [nsWindow release];
#endif
            catalyst::free(window, 0);
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 3, 0);
            return 0;
        }
        delegate->window = window;

        // Configure and show the window
        [nsWindow setReleasedWhenClosed:NO];
        [nsWindow setTitle:@CRYSTAL_DEFAULT_TITLE];
        [nsWindow setDelegate:delegate];
        [nsWindow center];
        [nsWindow makeKeyAndOrderFront:nil];
        [nsApplication activateIgnoringOtherApps:YES];

        // Assign native handles
#if __has_feature(objc_arc)
        window->native.primary = (catalyst::NUINT) (__bridge_retained void*) nsWindow;
        window->native.secondary = (catalyst::NUINT) (__bridge void*) nsApplication;
        window->native.tertiary = (catalyst::NUINT) (__bridge void*) nsView;
        window->platform = (__bridge_retained void*) delegate;
#else
        window->native.primary = (catalyst::NUINT) (void*) nsWindow;
        window->native.secondary = (catalyst::NUINT) (void*) nsApplication;
        window->native.tertiary = (catalyst::NUINT) (void*) nsView;
        window->platform = (void*) delegate;
#endif

        // Return and report success
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
        return window;
    }
}

void crystalProcessEvents(catalyst::BOOL wait) {
    @autoreleasepool {
        NSDate* untilDate = wait ? [NSDate distantFuture] : [NSDate distantPast];
        NSApplication* nsApplication = [NSApplication sharedApplication];
        NSEvent* event = [nsApplication
            nextEventMatchingMask:NSEventMaskAny
            untilDate:untilDate
            inMode:NSDefaultRunLoopMode
            dequeue:YES];
        if (event != nil) {
            [nsApplication sendEvent:event];
        }
        for (;;) {
            event = [nsApplication
                nextEventMatchingMask:NSEventMaskAny
                untilDate:[NSDate distantPast]
                inMode:NSDefaultRunLoopMode
                dequeue:YES];
            if (event == nil) break;
            [nsApplication sendEvent:event];
        }
        [nsApplication updateWindows];
    }
}

extern "C" void crystalSetWindowTitle(CRYSTALwindow* window, catalyst::UTF8 title, catalyst::RESULT* result) {
    @autoreleasepool {
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

        // Get the NSWindow reference
#if __has_feature(objc_arc)
        NSWindow* nsWindow = (__bridge NSWindow*) (void*) window->native.primary;
#else
        NSWindow* nsWindow = (NSWindow*) (void*) window->native.primary;
#endif

        // Convert UTF-8 title to NSString
        NSString* nsTitle = [NSString stringWithUTF8String:(const char*) title];
        if (nsTitle == nil) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
            return;
        }

        // Apply the title and report success
        [nsWindow setTitle:nsTitle];
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
    }
}

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

    // Get the NSWindow reference
#if __has_feature(objc_arc)
    NSWindow* nsWindow = (__bridge NSWindow*) (void*) window->native.primary;
#else
    NSWindow* nsWindow = (NSWindow*) (void*) window->native.primary;
#endif

    // Convert the title to UTF-8
    NSString* nsTitle = [nsWindow title];
    if (nsTitle == nil) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS_NOOP, 0, 0, 0);
        return;
    }
    const char* utf8 = [nsTitle UTF8String];
    if (utf8 == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
        return;
    }

    // Determine the required length
    catalyst::NUINT required = 0;
    while (utf8[required] != '\0') {
        if (++required == (catalyst::NUINT) -1) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }
    }
    *length = required;

    // Copy the title and report success
    catalyst::NUINT writable = capacity - 1;
    catalyst::NUINT i = 0;
    while (i < writable && i < required) {
        title[i] = (catalyst::BYTE) utf8[i];
        i += 1;
    }
    title[i] = (catalyst::BYTE) '\0';
    if (i < required) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_WARNING_PARTIAL, 0, 0, 0);
        return;
    }
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

// TODO: Document opcode 1 for failed to get NSScreen
extern "C" void crystalSetWindowPosition(CRYSTALwindow* window, catalyst::NUINT x, catalyst::NUINT y, catalyst::RESULT* result) {
    @autoreleasepool {
        if (window == 0) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
            return;
        }
        if (window->native.primary == 0) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_STATE, 0, 0, 0);
            return;
        }

        // Get the NSWindow reference
#if __has_feature(objc_arc)
        NSWindow* nsWindow = (__bridge NSWindow*) (void*) window->native.primary;
#else
        NSWindow* nsWindow = (NSWindow*) (void*) window->native.primary;
#endif

        // Get the NSScreen reference
        NSScreen* nsScreen = [nsWindow screen];
        if (nsScreen == nil) {
            nsScreen = [NSScreen mainScreen];
        }
        if (nsScreen == nil) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
            return;
        }

        // Convert top-left coordinates into Cocoa screen coordinates
        NSRect nsScreenFrame = [nsScreen frame];
        CGFloat nsTop = NSMaxY(nsScreenFrame);
        NSPoint nsTopLeft = NSMakePoint((CGFloat) x, nsTop - (CGFloat) y);

        // Apply position and report success
        [nsWindow setFrameTopLeftPoint:nsTopLeft];
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
    }
}

extern "C" void crystalGetWindowPosition(CRYSTALwindow* window, catalyst::NUINT* x, catalyst::NUINT* y, catalyst::RESULT* result) {
    @autoreleasepool {
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

        // Get the NSWindow reference
#if __has_feature(objc_arc)
        NSWindow* nsWindow = (__bridge NSWindow*) (void*) window->native.primary;
#else
        NSWindow* nsWindow = (NSWindow*) (void*) window->native.primary;
#endif

        // Get the NSScreen reference
        NSScreen* nsScreen = [nsWindow screen];
        if (nsScreen == nil) {
            nsScreen = [NSScreen mainScreen];
        }
        if (nsScreen == nil) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
            return;
        }

        // Convert Cocoa screen coordinates into top-left coordinates
        NSRect nsWindowFrame = [nsWindow frame];
        NSRect nsScreenFrame = [nsScreen frame];
        CGFloat nsTop = NSMaxY(nsScreenFrame);
        *x = (catalyst::NUINT) nsWindowFrame.origin.x;
        *y = (catalyst::NUINT) (nsTop - NSMaxY(nsWindowFrame));

        // Report success
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
    }
}

extern "C" void crystalSetWindowSize(CRYSTALwindow* window, catalyst::NUINT width, catalyst::NUINT height, catalyst::RESULT* result) {
    @autoreleasepool {
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

        // Get the NSWindow reference
#if __has_feature(objc_arc)
        NSWindow* nsWindow = (__bridge NSWindow*) (void*) window->native.primary;
#else
        NSWindow* nsWindow = (NSWindow*) (void*) window->native.primary;
#endif

        // Clamp the size to the window's content size limits
        NSSize nsMinSize = [nsWindow contentMinSize];
        NSSize nsMaxSize = [nsWindow contentMaxSize];
        CGFloat nsWidth = (CGFloat) width;
        CGFloat nsHeight = (CGFloat) height;

        if (nsWidth < nsMinSize.width) {
            nsWidth = nsMinSize.width;
        }
        if (nsHeight < nsMinSize.height) {
            nsHeight = nsMinSize.height;
        }
        if (nsWidth > nsMaxSize.width) {
            nsWidth = nsMaxSize.width;
        }
        if (nsHeight > nsMaxSize.height) {
            nsHeight = nsMaxSize.height;
        }

        // Preserve the top-left point while changing the content size
        NSRect nsFrame = [nsWindow frame];
        CGFloat nsTop = NSMaxY(nsFrame);

        NSRect nsContent = [nsWindow contentRectForFrameRect:nsFrame];
        nsContent.size = NSMakeSize(nsWidth, nsHeight);

        NSRect nsNewFrame = [nsWindow frameRectForContentRect:nsContent];
        nsNewFrame.origin.x = nsFrame.origin.x;
        nsNewFrame.origin.y = nsTop - nsNewFrame.size.height;

        // Apply the size and report success
        [nsWindow setFrame:nsNewFrame display:YES];
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
    }
}

extern "C" void crystalGetWindowSize(CRYSTALwindow* window, catalyst::NUINT* width, catalyst::NUINT* height, catalyst::RESULT* result) {
    @autoreleasepool {
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

        // Get the NSWindow reference
#if __has_feature(objc_arc)
        NSWindow* nsWindow = (__bridge NSWindow*) (void*) window->native.primary;
#else
        NSWindow* nsWindow = (NSWindow*) (void*) window->native.primary;
#endif

        // Get the content size
        NSRect nsContent = [nsWindow contentRectForFrameRect:[nsWindow frame]];
        *width = (catalyst::NUINT) nsContent.size.width;
        *height = (catalyst::NUINT) nsContent.size.height;

        // Report success
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
    }
}

extern "C" void crystalSetWindowSizeLimits(CRYSTALwindow* window, catalyst::NUINT minWidth, catalyst::NUINT minHeight, catalyst::NUINT maxWidth, catalyst::NUINT maxHeight, catalyst::RESULT* result) {
    @autoreleasepool {
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

        // Get the NSWindow reference
#if __has_feature(objc_arc)
        NSWindow* nsWindow = (__bridge NSWindow*) (void*) window->native.primary;
#else
        NSWindow* nsWindow = (NSWindow*) (void*) window->native.primary;
#endif

        // Convert Catalyst size-limit semantics into Cocoa size-limit semantics
        CGFloat nsMinWidth = minWidth == 0 ? 0.0 : (CGFloat) minWidth;
        CGFloat nsMinHeight = minHeight == 0 ? 0.0 : (CGFloat) minHeight;
        CGFloat nsMaxWidth = maxWidth == 0 || maxWidth == (catalyst::NUINT) -1 ? CGFLOAT_MAX : (CGFloat) maxWidth;
        CGFloat nsMaxHeight = maxHeight == 0 || maxHeight == (catalyst::NUINT) -1 ? CGFLOAT_MAX : (CGFloat) maxHeight;

        // Apply content size limits
        NSSize nsMinSize = NSMakeSize(nsMinWidth, nsMinHeight);
        NSSize nsMaxSize = NSMakeSize(nsMaxWidth, nsMaxHeight);

        [nsWindow setContentMinSize:nsMinSize];
        [nsWindow setContentMaxSize:nsMaxSize];

        // Report success
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
    }
}

extern "C" void crystalGetWindowSizeLimits(CRYSTALwindow* window, catalyst::NUINT* minWidth, catalyst::NUINT* minHeight, catalyst::NUINT* maxWidth, catalyst::NUINT* maxHeight, catalyst::RESULT* result) {
    @autoreleasepool {
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

        // Get the NSWindow reference
#if __has_feature(objc_arc)
        NSWindow* nsWindow = (__bridge NSWindow*) (void*) window->native.primary;
#else
        NSWindow* nsWindow = (NSWindow*) (void*) window->native.primary;
#endif

        // Get content size limits
        NSSize nsMinSize = [nsWindow contentMinSize];
        NSSize nsMaxSize = [nsWindow contentMaxSize];

        // Convert Cocoa size-limit semantics into Catalyst size-limit semantics
        *minWidth = nsMinSize.width <= 0.0 ? 0 : (catalyst::NUINT) nsMinSize.width;
        *minHeight = nsMinSize.height <= 0.0 ? 0 : (catalyst::NUINT) nsMinSize.height;
        *maxWidth = nsMaxSize.width >= (CGFloat) ((catalyst::NUINT) -1) ? 0 : (catalyst::NUINT) nsMaxSize.width;
        *maxHeight = nsMaxSize.height >= (CGFloat) ((catalyst::NUINT) -1) ? 0 : (catalyst::NUINT) nsMaxSize.height;

        // Report success
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
    }
}

extern "C" void crystalDestroyWindow(CRYSTALwindow* window, catalyst::RESULT* result) {
    @autoreleasepool {
        if (window == 0) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
            return;
        }

        // Get the NSWindow reference
        NSWindow* nsWindow = 0;
        if (window->native.primary != 0) {
#if __has_feature(objc_arc)
            nsWindow = (__bridge NSWindow*) (void*) window->native.primary;
#else
            nsWindow = (NSWindow*) (void*) window->native.primary;
#endif
        }

        // Detach and release delegate
        if (window->platform != 0) {
            if (nsWindow != 0) {
                [nsWindow setDelegate:nil];
            }

#if __has_feature(objc_arc)
            CrystalWindowDelegate* delegate = (__bridge_transfer CrystalWindowDelegate*) (void*) window->platform;
#else
            CrystalWindowDelegate* delegate = (CrystalWindowDelegate*) window->platform;
#endif

            delegate->window = 0;

#if !__has_feature(objc_arc)
            [delegate release];
#endif

            window->platform = 0;
        }

        // Release view reference
        window->native.tertiary = 0;

        // Release application reference
        window->native.secondary = 0;

        // Release window
        if (window->native.primary != 0) {
#if __has_feature(objc_arc)
            nsWindow = (__bridge_transfer NSWindow*) (void*) window->native.primary;
#else
            nsWindow = (NSWindow*) (void*) window->native.primary;
#endif

            [nsWindow setContentView:nil];
            [nsWindow close];

#if !__has_feature(objc_arc)
            [nsWindow release];
#endif

            window->native.primary = 0;
        }

        // Deconfigure callbacks
        window->closingCallback = 0;

        // Free the window object
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
    }
}

#endif /* TARGET_PLATFORM_DARWIN */
