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

static CATALYST_NUINT half(CATALYST_NUINT value) {
    CATALYST_NUINT result;

    result = value / 2;

    if (result == 0) {
        result = 1;
    }

    return result;
}

static CATALYST_NUINT threeQuarters(CATALYST_NUINT value) {
    CATALYST_NUINT result;

    result = value - (value / 4);

    if (result == 0) {
        result = 1;
    }

    return result;
}

static CATALYST_NUINT fiveQuarters(CATALYST_NUINT value) {
    CATALYST_NUINT result;

    result = value + (value / 4);

    if (result <= value) {
        result = value + 1;
    }

    return result;
}

static CATALYST_NUINT threeHalves(CATALYST_NUINT value) {
    CATALYST_NUINT result;

    result = value + (value / 2);

    if (result <= value) {
        result = value + 1;
    }

    return result;
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

static int readCurrentSize(
    CRYSTALwindow* window,
    CATALYST_NUINT* width,
    CATALYST_NUINT* height
) {
    CATALYST_RESULT result;

    *width = 0;
    *height = 0;

    crystalGetWindowSize(window, width, height, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail("crystalGetWindowSize initial", result);
        return 0;
    }

    if (*width == 0 || *height == 0) {
        printf("FAIL: crystalGetWindowSize initial returned zero size\n");
        printf("  width=%lu height=%lu\n", (unsigned long) *width, (unsigned long) *height);
        failed += 1;
        return 0;
    }

    printf(
        "Initial window size: width=%lu height=%lu\n",
        (unsigned long) *width,
        (unsigned long) *height
    );

    return 1;
}

int main() {
    CATALYST_RESULT result;
    CRYSTALwindow* window;

    CATALYST_NUINT baseWidth;
    CATALYST_NUINT baseHeight;

    CATALYST_NUINT halfWidth;
    CATALYST_NUINT halfHeight;
    CATALYST_NUINT minWidth;
    CATALYST_NUINT minHeight;
    CATALYST_NUINT maxWidth;
    CATALYST_NUINT maxHeight;
    CATALYST_NUINT largeWidth;
    CATALYST_NUINT largeHeight;

    appState.running = 1;

    window = crystalCreateWindow(&result);

    if (window == 0 || result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail("crystalCreateWindow", result);
        return 1;
    }

    pass("crystalCreateWindow");

    crystalSetWindowClosingCallback(window, onWindowClosing);

    drainEvents();

    if (!readCurrentSize(window, &baseWidth, &baseHeight)) {
        crystalDestroyWindow(window, &result);
        return 1;
    }

    halfWidth = half(baseWidth);
    halfHeight = half(baseHeight);

    minWidth = threeQuarters(baseWidth);
    minHeight = threeQuarters(baseHeight);

    maxWidth = fiveQuarters(baseWidth);
    maxHeight = fiveQuarters(baseHeight);

    largeWidth = threeHalves(baseWidth);
    largeHeight = threeHalves(baseHeight);

    printf(
        "Derived test sizes: half=%lux%lu min=%lux%lu base=%lux%lu max=%lux%lu large=%lux%lu\n",
        (unsigned long) halfWidth,
        (unsigned long) halfHeight,
        (unsigned long) minWidth,
        (unsigned long) minHeight,
        (unsigned long) baseWidth,
        (unsigned long) baseHeight,
        (unsigned long) maxWidth,
        (unsigned long) maxHeight,
        (unsigned long) largeWidth,
        (unsigned long) largeHeight
    );

    testSizeLimits(
        window,
        "crystalSet/GetWindowSizeLimits finite min/max based on current size",
        minWidth,
        minHeight,
        maxWidth,
        maxHeight
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize clamps below finite minimum",
        halfWidth,
        halfHeight,
        minWidth,
        minHeight
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize allows original size inside finite limits",
        baseWidth,
        baseHeight,
        baseWidth,
        baseHeight
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize clamps above finite maximum",
        largeWidth,
        largeHeight,
        maxWidth,
        maxHeight
    );

    testSizeLimits(
        window,
        "crystalSet/GetWindowSizeLimits min-only based on current size",
        minWidth,
        minHeight,
        0,
        0
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize clamps below min-only limits",
        halfWidth,
        halfHeight,
        minWidth,
        minHeight
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize allows larger size with no maximum",
        maxWidth,
        maxHeight,
        maxWidth,
        maxHeight
    );

    testSizeLimits(
        window,
        "crystalSet/GetWindowSizeLimits max-only based on current size",
        0,
        0,
        maxWidth,
        maxHeight
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize allows smaller size with no minimum",
        halfWidth,
        halfHeight,
        halfWidth,
        halfHeight
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize clamps above max-only limits",
        largeWidth,
        largeHeight,
        maxWidth,
        maxHeight
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
        "crystalSetWindowSize allows original size with no limits",
        baseWidth,
        baseHeight,
        baseWidth,
        baseHeight
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize allows smaller size with no limits",
        halfWidth,
        halfHeight,
        halfWidth,
        halfHeight
    );

    testSizeAfterLimits(
        window,
        "crystalSetWindowSize allows larger size with no limits",
        maxWidth,
        maxHeight,
        maxWidth,
        maxHeight
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
