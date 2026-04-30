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

static void failLimitsMismatch(
    const char* name,
    CATALYST_NUINT expectedMinWidth,
    CATALYST_NUINT expectedMinHeight,
    CATALYST_NUINT expectedMaxWidth,
    CATALYST_NUINT expectedMaxHeight,
    CATALYST_NUINT actualMinWidth,
    CATALYST_NUINT actualMinHeight,
    CATALYST_NUINT actualMaxWidth,
    CATALYST_NUINT actualMaxHeight
) {
    printf("FAIL: %s\n", name);
    printf(
        "  expected limits: minWidth=%lu minHeight=%lu maxWidth=%lu maxHeight=%lu\n",
        (unsigned long) expectedMinWidth,
        (unsigned long) expectedMinHeight,
        (unsigned long) expectedMaxWidth,
        (unsigned long) expectedMaxHeight
    );
    printf(
        "  actual limits:   minWidth=%lu minHeight=%lu maxWidth=%lu maxHeight=%lu\n",
        (unsigned long) actualMinWidth,
        (unsigned long) actualMinHeight,
        (unsigned long) actualMaxWidth,
        (unsigned long) actualMaxHeight
    );
    failed += 1;
}

static void failSizeMismatch(
    const char* name,
    CATALYST_NUINT requestedWidth,
    CATALYST_NUINT requestedHeight,
    CATALYST_NUINT expectedWidth,
    CATALYST_NUINT expectedHeight,
    CATALYST_NUINT actualWidth,
    CATALYST_NUINT actualHeight
) {
    printf("FAIL: %s\n", name);
    printf(
        "  requested size: width=%lu height=%lu\n",
        (unsigned long) requestedWidth,
        (unsigned long) requestedHeight
    );
    printf(
        "  expected size:  width=%lu height=%lu\n",
        (unsigned long) expectedWidth,
        (unsigned long) expectedHeight
    );
    printf(
        "  actual size:    width=%lu height=%lu\n",
        (unsigned long) actualWidth,
        (unsigned long) actualHeight
    );
    failed += 1;
}

static void testSizeLimits(
    CRYSTALwindow* window,
    const char* name,
    CATALYST_NUINT expectedMinWidth,
    CATALYST_NUINT expectedMinHeight,
    CATALYST_NUINT expectedMaxWidth,
    CATALYST_NUINT expectedMaxHeight
) {
    CATALYST_RESULT result;
    CATALYST_NUINT actualMinWidth;
    CATALYST_NUINT actualMinHeight;
    CATALYST_NUINT actualMaxWidth;
    CATALYST_NUINT actualMaxHeight;

    actualMinWidth = 0;
    actualMinHeight = 0;
    actualMaxWidth = 0;
    actualMaxHeight = 0;

    crystalSetWindowSizeLimits(
        window,
        expectedMinWidth,
        expectedMinHeight,
        expectedMaxWidth,
        expectedMaxHeight,
        &result
    );

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail(name, result);
        return;
    }

    drainEvents();

    crystalGetWindowSizeLimits(
        window,
        &actualMinWidth,
        &actualMinHeight,
        &actualMaxWidth,
        &actualMaxHeight,
        &result
    );

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail(name, result);
        return;
    }

    if (
        actualMinWidth != expectedMinWidth ||
        actualMinHeight != expectedMinHeight ||
        actualMaxWidth != expectedMaxWidth ||
        actualMaxHeight != expectedMaxHeight
    ) {
        failLimitsMismatch(
            name,
            expectedMinWidth,
            expectedMinHeight,
            expectedMaxWidth,
            expectedMaxHeight,
            actualMinWidth,
            actualMinHeight,
            actualMaxWidth,
            actualMaxHeight
        );
        return;
    }

    printf(
        "PASS: %s -> minWidth=%lu minHeight=%lu maxWidth=%lu maxHeight=%lu\n",
        name,
        (unsigned long) actualMinWidth,
        (unsigned long) actualMinHeight,
        (unsigned long) actualMaxWidth,
        (unsigned long) actualMaxHeight
    );
}

static void testSizeAfterLimits(
    CRYSTALwindow* window,
    const char* name,
    CATALYST_NUINT requestedWidth,
    CATALYST_NUINT requestedHeight,
    CATALYST_NUINT expectedWidth,
    CATALYST_NUINT expectedHeight
) {
    CATALYST_RESULT result;
    CATALYST_NUINT actualWidth;
    CATALYST_NUINT actualHeight;

    actualWidth = 0;
    actualHeight = 0;

    crystalSetWindowSize(window, requestedWidth, requestedHeight, &result);

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
        failSizeMismatch(
            name,
            requestedWidth,
            requestedHeight,
            expectedWidth,
            expectedHeight,
            actualWidth,
            actualHeight
        );
        return;
    }

    printf(
        "PASS: %s -> requested=%lux%lu actual=%lux%lu\n",
        name,
        (unsigned long) requestedWidth,
        (unsigned long) requestedHeight,
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

    testSizeLimits(
        window,
        "crystalSet/GetWindowSizeLimits min=320x240 max=1024x768",
        320,
        240,
        1024,
        768
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize clamps below finite minimum",
        100,
        100,
        320,
        240
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize allows size inside finite limits",
        640,
        480,
        640,
        480
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize clamps above finite maximum",
        1200,
        900,
        1024,
        768
    );

    testSizeLimits(
        window,
        "crystalSet/GetWindowSizeLimits min=400x300 max=none",
        400,
        300,
        0,
        0
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize clamps below min-only limits",
        200,
        100,
        400,
        300
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize allows large size with no maximum",
        900,
        700,
        900,
        700
    );

    testSizeLimits(
        window,
        "crystalSet/GetWindowSizeLimits min=none max=800x600",
        0,
        0,
        800,
        600
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize allows small size with no minimum",
        200,
        150,
        200,
        150
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize clamps above max-only limits",
        1000,
        700,
        800,
        600
    );

    testSizeLimits(
        window,
        "crystalSet/GetWindowSizeLimits min=none max=none",
        0,
        0,
        0,
        0
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize allows normal size with no limits",
        640,
        480,
        640,
        480
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize allows larger size with no limits",
        900,
        700,
        900,
        700
    );

    crystalDestroyWindow(window, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail("crystalDestroyWindow", result);
        return 1;
    }

    pass("crystalDestroyWindow");

    if (failed != 0) {
        printf("\n%d CRYSTAL size limit test(s) failed.\n", failed);
        return 1;
    }

    printf("\nAll CRYSTAL size limit tests passed.\n");
    return 0;
}
