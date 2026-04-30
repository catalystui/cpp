#include "CMAKECFG.h"
#include "modules/crystal/CRYSTAL.h"

#include <stdio.h>
#include <time.h>

struct AppState {
    int repositioned;
    int resized;
    int refresh;
    int redraw;
    int focused;
    int unfocused;
    int minimized;
    int maximized;
    int restored;
    int shown;
    int hidden;

    CATALYST_NUINT x;
    CATALYST_NUINT y;
    CATALYST_NUINT width;
    CATALYST_NUINT height;
};

static AppState appState;
static int failed = 0;

enum {
    EXPECT_REPOSITIONED = 0x0001,
    EXPECT_RESIZED      = 0x0002,
    EXPECT_REFRESH      = 0x0004,
    EXPECT_REDRAW       = 0x0008,
    EXPECT_FOCUSED      = 0x0010,
    EXPECT_UNFOCUSED    = 0x0020,
    EXPECT_MINIMIZED    = 0x0040,
    EXPECT_MAXIMIZED    = 0x0080,
    EXPECT_RESTORED     = 0x0100,
    EXPECT_SHOWN        = 0x0200,
    EXPECT_HIDDEN       = 0x0400
};

static void printResult(const char* name, CATALYST_RESULT result) {
    printf(
        "%s -> status=0x%02X context=0x%02X operation=0x%02X detail=0x%02X\n",
        name,
        result.status,
        result.context,
        result.operation,
        result.detail
    );
}

static void pass(const char* name) {
    printf("PASS: %s\n", name);
}

static void fail(const char* name, CATALYST_RESULT result) {
    printf("FAIL: %s\n", name);
    printResult("  result", result);
    failed += 1;
}

static void sleepMillis(int milliseconds) {
    clock_t start;
    clock_t elapsed;
    clock_t wait;

    if (milliseconds <= 0) {
        return;
    }

    wait = (clock_t) (((double) milliseconds * (double) CLOCKS_PER_SEC) / 1000.0);

    if (wait <= 0) {
        wait = 1;
    }

    start = clock();

    if (start == (clock_t) -1) {
        return;
    }

    do {
        elapsed = clock() - start;
    } while (elapsed < wait);
}

static void waitForEnter(const char* message) {
    int ch;

    printf("%s", message);
    fflush(stdout);

    do {
        ch = getchar();
    } while (ch != '\n' && ch != EOF);
}

static void drainEvents() {
    int i;

    for (i = 0; i < 10; i += 1) {
        crystalPollEvents();
    }
}

static void pumpEventsFor(int milliseconds) {
    int elapsed;

    elapsed = 0;

    while (elapsed < milliseconds) {
        drainEvents();
        sleepMillis(16);
        elapsed += 16;
    }
}

static void countdownInspect(int seconds) {
    int remaining;
    int elapsed;

    for (remaining = seconds; remaining > 0; remaining -= 1) {
        printf("\rInspecting for %d....", remaining);
        fflush(stdout);

        elapsed = 0;

        while (elapsed < 1000) {
            drainEvents();
            sleepMillis(16);
            elapsed += 16;
        }
    }

    printf("\r                    \r");
    fflush(stdout);
}

static void resetCounts() {
    appState.repositioned = 0;
    appState.resized = 0;
    appState.refresh = 0;
    appState.redraw = 0;
    appState.focused = 0;
    appState.unfocused = 0;
    appState.minimized = 0;
    appState.maximized = 0;
    appState.restored = 0;
    appState.shown = 0;
    appState.hidden = 0;

    appState.x = 0;
    appState.y = 0;
    appState.width = 0;
    appState.height = 0;
}

static void printCounts() {
    printf(
        "  callbacks: repositioned=%d resized=%d refresh=%d redraw=%d focused=%d unfocused=%d minimized=%d maximized=%d restored=%d shown=%d hidden=%d\n",
        appState.repositioned,
        appState.resized,
        appState.refresh,
        appState.redraw,
        appState.focused,
        appState.unfocused,
        appState.minimized,
        appState.maximized,
        appState.restored,
        appState.shown,
        appState.hidden
    );

    if (appState.repositioned != 0) {
        printf("  last position: x=%lu y=%lu\n", (unsigned long) appState.x, (unsigned long) appState.y);
    }

    if (appState.resized != 0) {
        printf("  last size: width=%lu height=%lu\n", (unsigned long) appState.width, (unsigned long) appState.height);
    }
}

static int hasExpectedCallbacks(int expected) {
    if ((expected & EXPECT_REPOSITIONED) != 0 && appState.repositioned == 0) return 0;
    if ((expected & EXPECT_RESIZED) != 0 && appState.resized == 0) return 0;
    if ((expected & EXPECT_REFRESH) != 0 && appState.refresh == 0) return 0;
    if ((expected & EXPECT_REDRAW) != 0 && appState.redraw == 0) return 0;
    if ((expected & EXPECT_FOCUSED) != 0 && appState.focused == 0) return 0;
    if ((expected & EXPECT_UNFOCUSED) != 0 && appState.unfocused == 0) return 0;
    if ((expected & EXPECT_MINIMIZED) != 0 && appState.minimized == 0) return 0;
    if ((expected & EXPECT_MAXIMIZED) != 0 && appState.maximized == 0) return 0;
    if ((expected & EXPECT_RESTORED) != 0 && appState.restored == 0) return 0;
    if ((expected & EXPECT_SHOWN) != 0 && appState.shown == 0) return 0;
    if ((expected & EXPECT_HIDDEN) != 0 && appState.hidden == 0) return 0;

    return 1;
}

static void waitForCallbacks(int expected, int milliseconds) {
    int elapsed;

    elapsed = 0;

    while (elapsed < milliseconds) {
        drainEvents();

        if (hasExpectedCallbacks(expected)) {
            return;
        }

        sleepMillis(16);
        elapsed += 16;
    }
}

static void failMissingCallbacks(const char* name, int expected) {
    printf("FAIL: %s\n", name);
    printf("  missing expected callback(s):");

    if ((expected & EXPECT_REPOSITIONED) != 0 && appState.repositioned == 0) printf(" repositioned");
    if ((expected & EXPECT_RESIZED) != 0 && appState.resized == 0) printf(" resized");
    if ((expected & EXPECT_REFRESH) != 0 && appState.refresh == 0) printf(" refresh");
    if ((expected & EXPECT_REDRAW) != 0 && appState.redraw == 0) printf(" redraw");
    if ((expected & EXPECT_FOCUSED) != 0 && appState.focused == 0) printf(" focused");
    if ((expected & EXPECT_UNFOCUSED) != 0 && appState.unfocused == 0) printf(" unfocused");
    if ((expected & EXPECT_MINIMIZED) != 0 && appState.minimized == 0) printf(" minimized");
    if ((expected & EXPECT_MAXIMIZED) != 0 && appState.maximized == 0) printf(" maximized");
    if ((expected & EXPECT_RESTORED) != 0 && appState.restored == 0) printf(" restored");
    if ((expected & EXPECT_SHOWN) != 0 && appState.shown == 0) printf(" shown");
    if ((expected & EXPECT_HIDDEN) != 0 && appState.hidden == 0) printf(" hidden");

    printf("\n");
    printCounts();

    failed += 1;
}

void onRepositioned(CRYSTALwindow* window, CATALYST_NUINT x, CATALYST_NUINT y) {
    appState.repositioned += 1;
    appState.x = x;
    appState.y = y;
}

void onResized(CRYSTALwindow* window, CATALYST_NUINT width, CATALYST_NUINT height) {
    appState.resized += 1;
    appState.width = width;
    appState.height = height;
}

void onRefresh(CRYSTALwindow* window) {
    appState.refresh += 1;
}

void onRedraw(CRYSTALwindow* window) {
    appState.redraw += 1;
}

void onFocused(CRYSTALwindow* window) {
    appState.focused += 1;
}

void onUnfocused(CRYSTALwindow* window) {
    appState.unfocused += 1;
}

void onMinimized(CRYSTALwindow* window) {
    appState.minimized += 1;
}

void onMaximized(CRYSTALwindow* window) {
    appState.maximized += 1;
}

void onRestored(CRYSTALwindow* window) {
    appState.restored += 1;
}

void onShown(CRYSTALwindow* window) {
    appState.shown += 1;
}

void onHidden(CRYSTALwindow* window) {
    appState.hidden += 1;
}

static void setCallbacks(CRYSTALwindow* window) {
    crystalSetWindowRepositionedCallback(window, onRepositioned);
    crystalSetWindowResizedCallback(window, onResized);
    crystalSetWindowRefreshCallback(window, onRefresh);
    crystalSetWindowRedrawCallback(window, onRedraw);
    crystalSetWindowFocusedCallback(window, onFocused);
    crystalSetWindowUnfocusedCallback(window, onUnfocused);
    crystalSetWindowMinimizedCallback(window, onMinimized);
    crystalSetWindowMaximizedCallback(window, onMaximized);
    crystalSetWindowRestoredCallback(window, onRestored);
    crystalSetWindowShownCallback(window, onShown);
    crystalSetWindowHiddenCallback(window, onHidden);
}

static void actionFocus(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalRequestWindowFocus(window, result);
}

static void actionMove(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalSetWindowPosition(window, 120, 120, result);
}

static void actionResize(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalSetWindowSize(window, 640, 480, result);
}

static void actionHide(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalHideWindow(window, result);
}

static void actionShow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalShowWindow(window, result);
}

static void actionMinimize(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalMinimizeWindow(window, result);
}

static void actionMaximize(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalMaximizeWindow(window, result);
}

static void actionRestore(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalRestoreWindow(window, result);
}

static void runActionTest(
    CRYSTALwindow* window,
    const char* name,
    void (*action)(CRYSTALwindow* window, CATALYST_RESULT* result),
    int expected
) {
    CATALYST_RESULT result;

    resetCounts();

    action(window, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS && result.status != CATALYST_STATUS_CODE_SUCCESS_NOOP) {
        fail(name, result);
        return;
    }

    waitForCallbacks(expected, 1500);

    if (!hasExpectedCallbacks(expected)) {
        failMissingCallbacks(name, expected);
        countdownInspect(3);
        return;
    }

    pass(name);
    printCounts();

    countdownInspect(3);
}

int main() {
    CATALYST_RESULT result;
    CRYSTALwindow* window;

    window = crystalCreateWindow(&result);

    if (window == 0 || result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail("crystalCreateWindow", result);
        return 1;
    }

    pass("crystalCreateWindow");

    setCallbacks(window);

    pumpEventsFor(500);

    printf("\nManual focus test:\n");
    waitForEnter("Click another app/window so CRYSTAL loses focus, then press Enter to request focus...");
    resetCounts();

    actionFocus(window, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS && result.status != CATALYST_STATUS_CODE_SUCCESS_NOOP) {
        fail("crystalRequestWindowFocus", result);
    } else {
        pass("crystalRequestWindowFocus returned success");
    }

    pumpEventsFor(500);
    printCounts();
    countdownInspect(3);

    waitForEnter("Press Enter to reposition the window...");
    runActionTest(
        window,
        "crystalSetWindowPosition fires repositioned/refresh",
        actionMove,
        EXPECT_REPOSITIONED | EXPECT_REFRESH
    );

    waitForEnter("Press Enter to resize the window...");
    runActionTest(
        window,
        "crystalSetWindowSize fires resized/refresh/redraw",
        actionResize,
        EXPECT_RESIZED | EXPECT_REFRESH | EXPECT_REDRAW
    );

    waitForEnter("Press Enter to hide the window...");
    runActionTest(
        window,
        "crystalHideWindow fires hidden/refresh",
        actionHide,
        EXPECT_HIDDEN | EXPECT_REFRESH
    );

    waitForEnter("Press Enter to show the window...");
    runActionTest(
        window,
        "crystalShowWindow fires shown/refresh",
        actionShow,
        EXPECT_SHOWN | EXPECT_REFRESH
    );

    waitForEnter("Press Enter to minimize the window...");
    runActionTest(
        window,
        "crystalMinimizeWindow fires minimized/refresh",
        actionMinimize,
        EXPECT_MINIMIZED | EXPECT_REFRESH
    );

    waitForEnter("Press Enter to restore the window from minimized...");
    runActionTest(
        window,
        "crystalRestoreWindow fires restored/refresh",
        actionRestore,
        EXPECT_RESTORED | EXPECT_REFRESH
    );

    waitForEnter("Press Enter to maximize the window...");
    runActionTest(
        window,
        "crystalMaximizeWindow fires maximized/refresh",
        actionMaximize,
        EXPECT_MAXIMIZED | EXPECT_REFRESH
    );

    waitForEnter("Press Enter to restore the window from maximized...");
    runActionTest(
        window,
        "crystalRestoreWindow fires restored/refresh",
        actionRestore,
        EXPECT_RESTORED | EXPECT_REFRESH
    );

    printf("\nManual unfocus test:\n");
    waitForEnter("Click another app/window so CRYSTAL loses focus, then press Enter...");
    resetCounts();
    pumpEventsFor(500);

    if (appState.unfocused == 0) {
        printf("FAIL: unfocused callback did not fire\n");
        printCounts();
        failed += 1;
    } else {
        pass("unfocused callback fired");
        printCounts();
    }

    countdownInspect(3);

    waitForEnter("Press Enter to destroy the window...");

    crystalDestroyWindow(window, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail("crystalDestroyWindow", result);
        return 1;
    }

    pass("crystalDestroyWindow");

    if (failed != 0) {
        printf("\n%d CRYSTAL callback test(s) failed.\n", failed);
        return 1;
    }

    printf("\nAll CRYSTAL callback tests passed.\n");
    return 0;
}
