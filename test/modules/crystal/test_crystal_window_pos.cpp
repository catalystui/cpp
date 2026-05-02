#include "CMAKECFG.h"
#include "modules/crystal/CRYSTAL.h"

#include <stdio.h>

struct AppState {
    int running;
};

static AppState appState;
static int failed = 0;

enum {
    WAIT_ATTEMPTS = 500,
    STABLE_READS = 2
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

    for (i = 0; i < 25; i += 1) {
        crystalPollEvents();
    }
}

CATALYST_BOOL onWindowClosing(CRYSTALwindow* window) {
    appState.running = 0;
    return CATALYST_FALSE;
}

static void failPositionMismatch(
    const char* name,
    CATALYST_NUINT expectedX,
    CATALYST_NUINT expectedY,
    CATALYST_NUINT actualX,
    CATALYST_NUINT actualY
) {
    printf("FAIL: %s\n", name);
    printf("  expected: x=%lu y=%lu\n", (unsigned long) expectedX, (unsigned long) expectedY);
    printf("  actual:   x=%lu y=%lu\n", (unsigned long) actualX, (unsigned long) actualY);
    failed += 1;
}

static int waitForPosition(
    CRYSTALwindow* window,
    CATALYST_NUINT expectedX,
    CATALYST_NUINT expectedY,
    CATALYST_NUINT* actualX,
    CATALYST_NUINT* actualY,
    CATALYST_RESULT* result
) {
    int i;
    int stable;

    stable = 0;

    for (i = 0; i < WAIT_ATTEMPTS; i += 1) {
        crystalPollEvents();

        crystalGetWindowPosition(window, actualX, actualY, result);

        if (result->status != CATALYST_STATUS_CODE_SUCCESS) {
            return 0;
        }

        if (*actualX == expectedX && *actualY == expectedY) {
            stable += 1;

            if (stable >= STABLE_READS) {
                return 1;
            }
        } else {
            stable = 0;
        }
    }

    return 0;
}

static void testPosition(
    CRYSTALwindow* window,
    const char* name,
    CATALYST_NUINT expectedX,
    CATALYST_NUINT expectedY
) {
    CATALYST_RESULT result;
    CATALYST_NUINT actualX;
    CATALYST_NUINT actualY;

    actualX = 0;
    actualY = 0;

    crystalSetWindowPosition(window, expectedX, expectedY, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail(name, result);
        return;
    }

    if (!waitForPosition(window, expectedX, expectedY, &actualX, &actualY, &result)) {
        if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
            fail(name, result);
            return;
        }

        failPositionMismatch(name, expectedX, expectedY, actualX, actualY);
        return;
    }

    printf(
        "PASS: %s -> x=%lu y=%lu\n",
        name,
        (unsigned long) actualX,
        (unsigned long) actualY
    );
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

    drainEvents();

    waitForEnter("Press Enter to move the window to x=100 y=100...");
    testPosition(window, "crystalSet/GetWindowPosition x=100 y=100", 100, 100);

    waitForEnter("Inspect the window position, then press Enter to move it to x=300 y=200...");
    testPosition(window, "crystalSet/GetWindowPosition x=300 y=200", 300, 200);

    waitForEnter("Inspect the window position, then press Enter to move it to x=50 y=50...");
    testPosition(window, "crystalSet/GetWindowPosition x=50 y=50", 50, 50);

    waitForEnter("Inspect the final window position, then press Enter to destroy the window...");

    crystalDestroyWindow(window, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail("crystalDestroyWindow", result);
        return 1;
    }

    pass("crystalDestroyWindow");

    if (failed != 0) {
        printf("\n%d CRYSTAL position test(s) failed.\n", failed);
        return 1;
    }

    printf("\nAll CRYSTAL position tests passed.\n");
    return 0;
}
