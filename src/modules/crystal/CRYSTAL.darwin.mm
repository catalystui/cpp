#include "CATCRLIB.hpp"
#include "CATMMLIB.hpp"
#include "CRYSTAL.internal.hpp"

#import <Cocoa/Cocoa.h>

// TODO: Document that dependency failure opcode 1 is for NSWindow allocation failure
extern "C" CRYSTALwindow* crystalCreateWindow(catalyst::RESULT* result) {
    @autoreleasepool {
        CRYSTALwindow* window = 0;
        catalyst::RESULT dependencyResult;
        catalyst::memory::alloc((void**) &window, (catalyst::NUINT) sizeof(CRYSTALwindow), &dependencyResult);
        if (window == 0) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, dependencyResult.status);
            return 0;
        }
        window->native.primary = 0;
        window->native.secondary = 0;
        window->native.tertiary = 0;

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
            catalyst::memory::free(window, 0);
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_ALLOCATION_FAILED, 0, 0, 0);
            return 0;
        }
        window->native.primary = (catalyst::NUINT) nsWindow;

        [nsWindow setTitle:@CRYSTAL_DEFAULT_TITLE];
        [nsWindow makeKeyAndOrderFront:nil];

        [app activateIgnoringOtherApps:YES];

        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
        return window;
    }
}

// TODO: Document that dependency failure opcode 1 is for NSWindow deallocation failure
extern "C" void crystalDestroyWindow(CRYSTALwindow* window, catalyst::RESULT* result) {
    @autoreleasepool {
        if (window == 0) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS_NOOP, 0, 0, 0);
            return;
        }
        if (window->native.primary != 0) {
            NSWindow* nsWindow = (NSWindow*) window->native.primary;
            [nsWindow close];
            window->native.primary = 0;
            window->native.secondary = 0;
            window->native.tertiary = 0;
        }
        catalyst::RESULT dependencyResult;
        catalyst::memory::free(window, &dependencyResult);
        if (!catalyst::statusCodeIsSuccess(dependencyResult.status)) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, dependencyResult.status);
            return;
        }
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
    }
}
