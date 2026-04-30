#include "CMAKECFG.h"
#include "modules/crystal/CRYSTAL.h"

#include <stdio.h>

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

CATALYST_BOOL onWindowClosing(CRYSTALwindow* window) {
    appState.running = 0;
    return CATALYST_FALSE;
}

static void failSizeMismatch(
    const char* name,
    CATALYST_NUINT expectedWidth,
    CATALYST_NUINT expectedHeight,
    CATALYST_NUINT actualWidth,
    CATALYST_NUINT actualHeight
) {
    printf("FAIL: %s\n", name);
    printf(
        "  expected: width=%lu height=%lu\n",
        (unsigned long) expectedWidth,
        (unsigned long) expectedHeight
    );
    printf(
        "  actual:   width=%lu height=%lu\n",
        (unsigned long) actualWidth,
        (unsigned long) actualHeight
    );
    failed += 1;
}

static void testSize(
    CRYSTALwindow* window,
    const char* name,
    CATALYST_NUINT expectedWidth,
    CATALYST_NUINT expectedHeight
) {
    CATALYST_RESULT result;
    CATALYST_NUINT actualWidth;
    CATALYST_NUINT actualHeight;

    actualWidth = 0;
    actualHeight = 0;

    crystalSetWindowSize(window, expectedWidth, expectedHeight, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail(name, result);
        return;
    }

    drainEvents();

    crystalGetWindowSize(window, &actualWidth, &actualHeight, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail(name, result);
        return;
    }

    if (actualWidth != expectedWidth || actualHeight != expectedHeight) {
        failSizeMismatch(name, expectedWidth, expectedHeight, actualWidth, actualHeight);
        return;
    }

    printf(
        "PASS: %s -> width=%lu height=%lu\n",
        name,
        (unsigned long) actualWidth,
        (unsigned long) actualHeight
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

    waitForEnter("Press Enter to resize the window to width=640 height=480...");
    testSize(
        window,
        "crystalSet/GetWindowSize width=640 height=480",
        640,
        480
    );

    waitForEnter("Inspect the window size, then press Enter to resize it to width=800 height=600...");
    testSize(
        window,
        "crystalSet/GetWindowSize width=800 height=600",
        800,
        600
    );

    waitForEnter("Inspect the window size, then press Enter to resize it to width=1024 height=768...");
    testSize(
        window,
        "crystalSet/GetWindowSize width=1024 height=768",
        1024,
        768
    );

    waitForEnter("Inspect the final window size, then press Enter to destroy the window...");

    crystalDestroyWindow(window, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail("crystalDestroyWindow", result);
        return 1;
    }

    pass("crystalDestroyWindow");

    if (failed != 0) {
        printf("\n%d CRYSTAL size test(s) failed.\n", failed);
        return 1;
    }

    printf("\nAll CRYSTAL size tests passed.\n");
    return 0;
}
