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

static void printStyle(const char* label, CRYSTAL_PROPERTIES_STYLE style) {
    printf("%s: 0x%02X [", label, style);

    if (style == CRYSTAL_PROPERTIES_STYLE_NONE) {
        printf(" none");
    }
    if ((style & CRYSTAL_PROPERTIES_STYLE_DECORATED) != 0) {
        printf(" decorated");
    }
    if ((style & CRYSTAL_PROPERTIES_STYLE_MINIMIZABLE) != 0) {
        printf(" minimizable");
    }
    if ((style & CRYSTAL_PROPERTIES_STYLE_MAXIMIZABLE) != 0) {
        printf(" maximizable");
    }
    if ((style & CRYSTAL_PROPERTIES_STYLE_RESIZABLE) != 0) {
        printf(" resizable");
    }

    printf(" ]\n");
}

static void failStyleMissingFlags(
    const char* name,
    CRYSTAL_PROPERTIES_STYLE requested,
    CRYSTAL_PROPERTIES_STYLE actual
) {
    printf("FAIL: %s\n", name);
    printStyle("  requested", requested);
    printStyle("  actual   ", actual);
    failed += 1;
}

static void testStyleIncludes(
    CRYSTALwindow* window,
    const char* name,
    CRYSTAL_PROPERTIES_STYLE requested
) {
    CATALYST_RESULT result;
    CRYSTAL_PROPERTIES_STYLE actual;

    actual = 0xFF;

    crystalSetWindowStyle(window, requested, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail(name, result);
        return;
    }

    drainEvents();

    crystalGetWindowStyle(window, &actual, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail(name, result);
        return;
    }

    if ((actual & requested) != requested) {
        failStyleMissingFlags(name, requested, actual);
        return;
    }

    printf("PASS: %s\n", name);
    printStyle("  requested", requested);
    printStyle("  actual   ", actual);

    countdownInspect(3);
}

static void testStyleExact(
    CRYSTALwindow* window,
    const char* name,
    CRYSTAL_PROPERTIES_STYLE expected
) {
    CATALYST_RESULT result;
    CRYSTAL_PROPERTIES_STYLE actual;

    actual = 0xFF;

    crystalSetWindowStyle(window, expected, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail(name, result);
        return;
    }

    drainEvents();

    crystalGetWindowStyle(window, &actual, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail(name, result);
        return;
    }

    if (actual != expected) {
        failStyleMissingFlags(name, expected, actual);
        return;
    }

    printf("PASS: %s\n", name);
    printStyle("  expected", expected);
    printStyle("  actual  ", actual);

    countdownInspect(5);
}

static void testInvalidStyle(CRYSTALwindow* window) {
    CATALYST_RESULT result;

    crystalSetWindowStyle(window, (CRYSTAL_PROPERTIES_STYLE) 0x80, &result);

    if (result.status != CATALYST_STATUS_CODE_ERROR_INVALID_ARGUMENT) {
        fail("crystalSetWindowStyle invalid flag", result);
        return;
    }

    pass("crystalSetWindowStyle invalid flag");
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

    waitForEnter("Press Enter to remove all style flags...");
    testStyleExact(
        window,
        "crystalSet/GetWindowStyle none",
        CRYSTAL_PROPERTIES_STYLE_NONE
    );

    waitForEnter("Press Enter to set decorated only...");
    testStyleIncludes(
        window,
        "crystalSet/GetWindowStyle decorated only",
        CRYSTAL_PROPERTIES_STYLE_DECORATED
    );

    waitForEnter("Press Enter to enable minimize...");
    testStyleIncludes(
        window,
        "crystalSet/GetWindowStyle decorated minimizable",
        (CRYSTAL_PROPERTIES_STYLE) (
            CRYSTAL_PROPERTIES_STYLE_DECORATED |
            CRYSTAL_PROPERTIES_STYLE_MINIMIZABLE
        )
    );

    waitForEnter("Press Enter to enable maximize...");
    testStyleIncludes(
        window,
        "crystalSet/GetWindowStyle decorated maximizable",
        (CRYSTAL_PROPERTIES_STYLE) (
            CRYSTAL_PROPERTIES_STYLE_DECORATED |
            CRYSTAL_PROPERTIES_STYLE_MAXIMIZABLE
        )
    );

    waitForEnter("Press Enter to enable resize...");
    testStyleIncludes(
        window,
        "crystalSet/GetWindowStyle decorated resizable",
        (CRYSTAL_PROPERTIES_STYLE) (
            CRYSTAL_PROPERTIES_STYLE_DECORATED |
            CRYSTAL_PROPERTIES_STYLE_RESIZABLE
        )
    );

    waitForEnter("Press Enter to enable all style flags...");
    testStyleIncludes(
        window,
        "crystalSet/GetWindowStyle all",
        (CRYSTAL_PROPERTIES_STYLE) (
            CRYSTAL_PROPERTIES_STYLE_DECORATED |
            CRYSTAL_PROPERTIES_STYLE_MINIMIZABLE |
            CRYSTAL_PROPERTIES_STYLE_MAXIMIZABLE |
            CRYSTAL_PROPERTIES_STYLE_RESIZABLE
        )
    );

    waitForEnter("Press Enter to test invalid style flag...");
    testInvalidStyle(window);

    waitForEnter("Press Enter to destroy the window...");

    crystalDestroyWindow(window, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail("crystalDestroyWindow", result);
        return 1;
    }

    pass("crystalDestroyWindow");

    if (failed != 0) {
        printf("\n%d CRYSTAL window style test(s) failed.\n", failed);
        return 1;
    }

    printf("\nAll CRYSTAL window style tests passed.\n");
    return 0;
}
