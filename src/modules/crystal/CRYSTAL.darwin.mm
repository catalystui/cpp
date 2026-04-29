#include "CMAKECFG.h"
#if TARGET_PLATFORM_DARWIN

#include "CATCRLIB.hpp"
#include "CATMMLIB.hpp"
#include "CATTELIB.hpp"
#include "CRYSTAL.internal.hpp"

#import <Cocoa/Cocoa.h>

#ifndef __has_feature
    #define __has_feature(x) 0
#endif

@interface CRYSTALWindowDelegate : NSObject <NSWindowDelegate> {
@public
    CRYSTALwindow* window;
}
@end

@implementation CRYSTALWindowDelegate

- (BOOL)windowShouldClose:(id)sender {
    if (window != 0 && window->closingCallback != 0) {
        return window->closingCallback(window, window->udata) ? YES : NO;
    }

    return YES;
}

@end

// TODO: Document that dependency failure opcode 1 is for CATMMLIB allocation failure.
// TODO: Document that dependency failure opcode 2 is for NSWindow allocation failure.
// TODO: Document that dependency failure opcode 3 is for CRYSTALWindowDelegate allocation failure.
extern "C" CRYSTALwindow* crystalCreateWindow(catalyst::RESULT* result) {
    @autoreleasepool {
        CRYSTALwindow* window = 0;
        catalyst::RESULT dependencyResult;
        catalyst::alloc((void**) &window, (catalyst::NUINT) sizeof(CRYSTALwindow), &dependencyResult);
        if (window == 0) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, dependencyResult.status);
            return 0;
        }
        window->native.primary = 0;
        window->native.secondary = 0;
        window->native.tertiary = 0;
        window->udata = 0;
        window->closingCallback = 0;

        NSApplication* app = [NSApplication sharedApplication];

        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
        [app finishLaunching];

        NSRect rect = NSMakeRect(0, 0, CRYSTAL_DEFAULT_WIDTH, CRYSTAL_DEFAULT_HEIGHT);

        NSWindow* nsWindow = [[NSWindow alloc]
            initWithContentRect:rect
            styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable)
            backing:NSBackingStoreBuffered
            defer:NO];

        if (nsWindow == nil) {
            catalyst::free(window, 0);
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_ALLOCATION_FAILED, 0, 2, 0);
            return 0;
        }

        CRYSTALWindowDelegate* delegate = [[CRYSTALWindowDelegate alloc] init];

        if (delegate == nil) {
#if !__has_feature(objc_arc)
            [nsWindow release];
#endif
            catalyst::free(window, 0);
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_ALLOCATION_FAILED, 0, 3, 0);
            return 0;
        }

        delegate->window = window;

        [nsWindow setReleasedWhenClosed:NO];
        [nsWindow setDelegate:delegate];
        [nsWindow setTitle:@CRYSTAL_DEFAULT_TITLE];
        [nsWindow makeKeyAndOrderFront:nil];

        [app activateIgnoringOtherApps:YES];

#if __has_feature(objc_arc)
        window->native.primary = (catalyst::NUINT) (__bridge_retained void*) nsWindow;
        window->native.secondary = (catalyst::NUINT) (__bridge void*) app;
        window->native.tertiary = (catalyst::NUINT) (__bridge_retained void*) delegate;
#else
        window->native.primary = (catalyst::NUINT) (void*) nsWindow;
        window->native.secondary = (catalyst::NUINT) (void*) app;
        window->native.tertiary = (catalyst::NUINT) (void*) delegate;
#endif

        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
        return window;
    }
}

static void crystalDarwinProcessEvents(NSDate* untilDate) {
    NSApplication* app = [NSApplication sharedApplication];

    NSEvent* event = [app
        nextEventMatchingMask:NSEventMaskAny
        untilDate:untilDate
        inMode:NSDefaultRunLoopMode
        dequeue:YES];

    if (event != nil) {
        [app sendEvent:event];
    }

    for (;;) {
        event = [app
            nextEventMatchingMask:NSEventMaskAny
            untilDate:[NSDate distantPast]
            inMode:NSDefaultRunLoopMode
            dequeue:YES];

        if (event == nil) break;

        [app sendEvent:event];
    }

    [app updateWindows];
}

extern "C" void crystalPollEvents() {
    @autoreleasepool {
        crystalDarwinProcessEvents([NSDate distantPast]);
    }
}

extern "C" void crystalWaitEvents() {
    @autoreleasepool {
        crystalDarwinProcessEvents([NSDate distantFuture]);
    }
}

// TODO: Document that dependency failure opcode 1 is for CATMMLIB deallocation failure.
extern "C" void crystalDestroyWindow(CRYSTALwindow* window, catalyst::RESULT* result) {
    @autoreleasepool {
        if (window == 0) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS_NOOP, 0, 0, 0);
            return;
        }
        if (window->native.primary != 0) {
#if __has_feature(objc_arc)
            NSWindow* nsWindow = (__bridge_transfer NSWindow*) (void*) window->native.primary;
#else
            NSWindow* nsWindow = (NSWindow*) (void*) window->native.primary;
#endif

            [nsWindow setDelegate:nil];
            [nsWindow close];

#if !__has_feature(objc_arc)
            [nsWindow release];
#endif

            window->native.primary = 0;
        }
        window->native.secondary = 0;
        if (window->native.tertiary != 0) {
#if __has_feature(objc_arc)
            CRYSTALWindowDelegate* delegate = (__bridge_transfer CRYSTALWindowDelegate*) (void*) window->native.tertiary;
#else
            CRYSTALWindowDelegate* delegate = (CRYSTALWindowDelegate*) (void*) window->native.tertiary;
#endif

            delegate->window = 0;

#if !__has_feature(objc_arc)
            [delegate release];
#endif

            window->native.tertiary = 0;
        }
        window->udata = 0;
        window->closingCallback = 0;

        catalyst::RESULT dependencyResult;
        catalyst::free(window, &dependencyResult);
        if (!catalyst::statusCodeIsSuccess(dependencyResult.status)) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, dependencyResult.status);
            return;
        }
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
    }
}

extern "C" void crystalSetWindowUserPointer(CRYSTALwindow* window, void* pointer) {
    if (window == 0) return;
    window->udata = pointer;
}

extern "C" void* crystalGetWindowUserPointer(CRYSTALwindow* window) {
    if (window == 0) return 0;
    return window->udata;
}

extern "C" void crystalSetWindowTitle(CRYSTALwindow* window, catalyst::UTF8 title) {
    @autoreleasepool {
        if (window == 0) return;
        if (window->native.primary == 0) return;
        if (title == 0) return;

#if __has_feature(objc_arc)
        NSWindow* nsWindow = (__bridge NSWindow*) (void*) window->native.primary;
#else
        NSWindow* nsWindow = (NSWindow*) (void*) window->native.primary;
#endif

        if (nsWindow == nil) return;

        NSString* nsTitle = [NSString stringWithUTF8String:(const char*) title];

        if (nsTitle == nil) {
            return;
        }

        [nsWindow setTitle:nsTitle];
    }
}

extern "C" void crystalGetWindowTitle(
    CRYSTALwindow* window,
    catalyst::UTF8W title,
    catalyst::NUINT capacity,
    catalyst::NUINT* length
) {
    @autoreleasepool {
        if (length != 0) *length = 0;
        if (window == 0) return;
        if (window->native.primary == 0) return;
#if __has_feature(objc_arc)
        NSWindow* nsWindow = (__bridge NSWindow*) (void*) window->native.primary;
#else
        NSWindow* nsWindow = (NSWindow*) (void*) window->native.primary;
#endif
        if (nsWindow == nil) return;
        NSString* nsTitle = [nsWindow title];
        if (nsTitle == nil) return;
        const char* utf8 = [nsTitle UTF8String];
        if (utf8 == 0) return;
        catalyst::NUINT required = 0;
        while (utf8[required] != '\0') {
            if (++required == (catalyst::NUINT) -1) {
                return;
            }
        }
        if (length != 0) *length = required;
        if (title == 0) return;
        if (capacity == 0) return;
        catalyst::NUINT index = 0;
        catalyst::NUINT writable = capacity - 1;
        while (index < writable && index < required) {
            title[index] = (catalyst::BYTE) utf8[index];
            ++index;
        }
        title[index] = (catalyst::BYTE) '\0';
    }
}

extern "C" void crystalSetWindowClosingCallback(CRYSTALwindow* window, CRYSTALwindowClosingCallback callback) {
    if (window == 0) return;
    window->closingCallback = callback;
}

#endif /* TARGET_PLATFORM_DARWIN */
