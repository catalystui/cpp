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

static int waitForSize(
    CRYSTALwindow* window,
    CATALYST_NUINT expectedWidth,
    CATALYST_NUINT expectedHeight,
    CATALYST_NUINT* actualWidth,
    CATALYST_NUINT* actualHeight,
    CATALYST_RESULT* result
) {
    int i;
    int stable;

    stable = 0;

    for (i = 0; i < WAIT_ATTEMPTS; i += 1) {
        crystalPollEvents();

        crystalGetWindowSize(window, actualWidth, actualHeight, result);

        if (result->status != CATALYST_STATUS_CODE_SUCCESS) {
            return 0;
        }

        if (*actualWidth == expectedWidth && *actualHeight == expectedHeight) {
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

    if (!waitForSize(window, expectedWidth, expectedHeight, &actualWidth, &actualHeight, &result)) {
        if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
            fail(name, result);
            return;
        }

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

static int readCurrentSize(
    CRYSTALwindow* window,
    CATALYST_NUINT* width,
    CATALYST_NUINT* height
) {
    CATALYST_RESULT result;

    *width = 0;
    *height = 0;

    drainEvents();

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
    CATALYST_NUINT threeQuarterWidth;
    CATALYST_NUINT threeQuarterHeight;
    CATALYST_NUINT fiveQuarterWidth;
    CATALYST_NUINT fiveQuarterHeight;
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

    threeQuarterWidth = threeQuarters(baseWidth);
    threeQuarterHeight = threeQuarters(baseHeight);

    fiveQuarterWidth = fiveQuarters(baseWidth);
    fiveQuarterHeight = fiveQuarters(baseHeight);

    largeWidth = threeHalves(baseWidth);
    largeHeight = threeHalves(baseHeight);

    printf(
        "Derived test sizes: half=%lux%lu threeQuarter=%lux%lu base=%lux%lu fiveQuarter=%lux%lu large=%lux%lu\n",
        (unsigned long) halfWidth,
        (unsigned long) halfHeight,
        (unsigned long) threeQuarterWidth,
        (unsigned long) threeQuarterHeight,
        (unsigned long) baseWidth,
        (unsigned long) baseHeight,
        (unsigned long) fiveQuarterWidth,
        (unsigned long) fiveQuarterHeight,
        (unsigned long) largeWidth,
        (unsigned long) largeHeight
    );

    waitForEnter("Press Enter to resize the window to half its initial size...");
    testSize(window, "crystalSet/GetWindowSize half initial size", halfWidth, halfHeight);

    waitForEnter("Inspect the smaller window, then press Enter to resize it to three-quarter size...");
    testSize(window, "crystalSet/GetWindowSize three-quarter initial size", threeQuarterWidth, threeQuarterHeight);

    waitForEnter("Inspect the window size, then press Enter to resize it back to initial size...");
    testSize(window, "crystalSet/GetWindowSize initial size", baseWidth, baseHeight);

    waitForEnter("Inspect the window size, then press Enter to resize it to 1.25x initial size...");
    testSize(window, "crystalSet/GetWindowSize 1.25x initial size", fiveQuarterWidth, fiveQuarterHeight);

    waitForEnter("Inspect the window size, then press Enter to resize it to 1.5x initial size...");
    testSize(window, "crystalSet/GetWindowSize 1.5x initial size", largeWidth, largeHeight);

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
