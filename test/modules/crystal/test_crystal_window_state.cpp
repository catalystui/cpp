#include "CMAKECFG.h"
#include "modules/crystal/CRYSTAL.h"

#include <stdio.h>
#include <time.h>

struct AppState {
    int running;
};

static AppState appState;
static int failed = 0;

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
        printf("\rWaiting %d....", remaining);
        fflush(stdout);

        elapsed = 0;

        while (elapsed < 1000) {
            drainEvents();
            sleepMillis(16);
            elapsed += 16;
        }
    }

    printf("\r             \r");
    fflush(stdout);
}

CATALYST_BOOL onWindowClosing(CRYSTALwindow* window) {
    appState.running = 0;
    return CATALYST_FALSE;
}

static void printState(const char* label, CRYSTAL_PROPERTIES_STATE state) {
    printf("%s: 0x%02X [", label, state);

    if (state == CRYSTAL_PROPERTIES_STATE_NONE) {
        printf(" none");
    }
    if ((state & CRYSTAL_PROPERTIES_STATE_FOCUSED) != 0) {
        printf(" focused");
    }
    if ((state & CRYSTAL_PROPERTIES_STATE_UNFOCUSED) != 0) {
        printf(" unfocused");
    }
    if ((state & CRYSTAL_PROPERTIES_STATE_MINIMIZED) != 0) {
        printf(" minimized");
    }
    if ((state & CRYSTAL_PROPERTIES_STATE_MAXIMIZED) != 0) {
        printf(" maximized");
    }
    if ((state & CRYSTAL_PROPERTIES_STATE_VISIBLE) != 0) {
        printf(" visible");
    }
    if ((state & CRYSTAL_PROPERTIES_STATE_HIDDEN) != 0) {
        printf(" hidden");
    }

    printf(" ]\n");
}

static void failStateMismatch(
    const char* name,
    CRYSTAL_PROPERTIES_STATE expectedIncludes,
    CRYSTAL_PROPERTIES_STATE expectedExcludes,
    CRYSTAL_PROPERTIES_STATE actual
) {
    printf("FAIL: %s\n", name);
    printState("  expected includes", expectedIncludes);
    printState("  expected excludes", expectedExcludes);
    printState("  actual          ", actual);
    failed += 1;
}

static void testWindowActionState(
    CRYSTALwindow* window,
    const char* name,
    void (*action)(CRYSTALwindow* window, CATALYST_RESULT* result),
    CRYSTAL_PROPERTIES_STATE expectedIncludes,
    CRYSTAL_PROPERTIES_STATE expectedExcludes
) {
    CATALYST_RESULT result;
    CRYSTAL_PROPERTIES_STATE actual;

    actual = CRYSTAL_PROPERTIES_STATE_NONE;

    action(window, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail(name, result);
        return;
    }

    pumpEventsFor(500);

    crystalGetWindowState(window, &actual);

    if ((actual & expectedIncludes) != expectedIncludes) {
        failStateMismatch(name, expectedIncludes, expectedExcludes, actual);
        return;
    }

    if ((actual & expectedExcludes) != 0) {
        failStateMismatch(name, expectedIncludes, expectedExcludes, actual);
        return;
    }

    printf("PASS: %s\n", name);
    printState("  actual", actual);

    countdownInspect(3);
}

static void testWindowActionOnly(
    CRYSTALwindow* window,
    const char* name,
    void (*action)(CRYSTALwindow* window, CATALYST_RESULT* result)
) {
    CATALYST_RESULT result;
    CRYSTAL_PROPERTIES_STATE actual;

    actual = CRYSTAL_PROPERTIES_STATE_NONE;

    action(window, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail(name, result);
        return;
    }

    pumpEventsFor(500);

    crystalGetWindowState(window, &actual);

    printf("PASS: %s\n", name);
    printState("  actual", actual);

    countdownInspect(5);
}

int main() {
    CATALYST_RESULT result;
    CRYSTALwindow* window;

    appState.running = 1;

    window = crystalCreateWindow(&result);

    if (window == 0 || result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail("crystalCreateWindow", result);
        return 1;
    }

    pass("crystalCreateWindow");

    crystalSetWindowClosingCallback(window, onWindowClosing);

    pumpEventsFor(500);

    waitForEnter("Press Enter to request window focus...");
    testWindowActionState(
        window,
        "crystalRequestWindowFocus",
        crystalRequestWindowFocus,
        (CRYSTAL_PROPERTIES_STATE) (
            CRYSTAL_PROPERTIES_STATE_FOCUSED |
            CRYSTAL_PROPERTIES_STATE_VISIBLE
        ),
        (CRYSTAL_PROPERTIES_STATE) (
            CRYSTAL_PROPERTIES_STATE_HIDDEN |
            CRYSTAL_PROPERTIES_STATE_MINIMIZED
        )
    );

    waitForEnter("Press Enter to request window attention...");
    testWindowActionOnly(
        window,
        "crystalRequestWindowAttention",
        crystalRequestWindowAttention
    );

    waitForEnter("Press Enter to hide the window...");
    testWindowActionState(
        window,
        "crystalHideWindow",
        crystalHideWindow,
        CRYSTAL_PROPERTIES_STATE_HIDDEN,
        (CRYSTAL_PROPERTIES_STATE) (
            CRYSTAL_PROPERTIES_STATE_VISIBLE |
            CRYSTAL_PROPERTIES_STATE_MINIMIZED
        )
    );

    waitForEnter("Press Enter to show the window...");
    testWindowActionState(
        window,
        "crystalShowWindow",
        crystalShowWindow,
        CRYSTAL_PROPERTIES_STATE_VISIBLE,
        CRYSTAL_PROPERTIES_STATE_HIDDEN
    );

    waitForEnter("Press Enter to minimize the window...");
    testWindowActionState(
        window,
        "crystalMinimizeWindow",
        crystalMinimizeWindow,
        (CRYSTAL_PROPERTIES_STATE) (
            CRYSTAL_PROPERTIES_STATE_MINIMIZED |
            CRYSTAL_PROPERTIES_STATE_HIDDEN
        ),
        CRYSTAL_PROPERTIES_STATE_VISIBLE
    );

    waitForEnter("Press Enter to restore the window from minimized...");
    testWindowActionState(
        window,
        "crystalRestoreWindow from minimized",
        crystalRestoreWindow,
        CRYSTAL_PROPERTIES_STATE_VISIBLE,
        (CRYSTAL_PROPERTIES_STATE) (
            CRYSTAL_PROPERTIES_STATE_HIDDEN |
            CRYSTAL_PROPERTIES_STATE_MINIMIZED |
            CRYSTAL_PROPERTIES_STATE_MAXIMIZED
        )
    );

    waitForEnter("Press Enter to maximize the window...");
    testWindowActionState(
        window,
        "crystalMaximizeWindow",
        crystalMaximizeWindow,
        (CRYSTAL_PROPERTIES_STATE) (
            CRYSTAL_PROPERTIES_STATE_MAXIMIZED |
            CRYSTAL_PROPERTIES_STATE_VISIBLE
        ),
        (CRYSTAL_PROPERTIES_STATE) (
            CRYSTAL_PROPERTIES_STATE_HIDDEN |
            CRYSTAL_PROPERTIES_STATE_MINIMIZED
        )
    );

    waitForEnter("Press Enter to restore the window from maximized...");
    testWindowActionState(
        window,
        "crystalRestoreWindow from maximized",
        crystalRestoreWindow,
        CRYSTAL_PROPERTIES_STATE_VISIBLE,
        (CRYSTAL_PROPERTIES_STATE) (
            CRYSTAL_PROPERTIES_STATE_HIDDEN |
            CRYSTAL_PROPERTIES_STATE_MINIMIZED |
            CRYSTAL_PROPERTIES_STATE_MAXIMIZED
        )
    );

    waitForEnter("Press Enter to destroy the window...");

    crystalDestroyWindow(window, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail("crystalDestroyWindow", result);
        return 1;
    }

    pass("crystalDestroyWindow");

    if (failed != 0) {
        printf("\n%d CRYSTAL window action/state test(s) failed.\n", failed);
        return 1;
    }

    printf("\nAll CRYSTAL window action/state tests passed.\n");
    return 0;
}
