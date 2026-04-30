#include "CMAKECFG.h"
#include "modules/crystal/CRYSTAL.h"

#include <stdio.h>

struct AppState {
    int closeRequests;
    int allowClose;
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
    appState.closeRequests += 1;

    if (appState.allowClose) {
        return CATALYST_TRUE;
    }

    return CATALYST_FALSE;
}

static void expectStatus(const char* name, CATALYST_RESULT result, CATALYST_BYTE expectedStatus) {
    if (result.status != expectedStatus) {
        fail(name, result);
        return;
    }

    pass(name);
}

static void expectInt(const char* name, int expected, int actual) {
    if (actual != expected) {
        printf("FAIL: %s\n", name);
        printf("  expected: %d\n", expected);
        printf("  actual:   %d\n", actual);
        failed += 1;
        return;
    }

    pass(name);
}

int main() {
    CATALYST_RESULT result;
    CRYSTALwindow* window;

    appState.closeRequests = 0;
    appState.allowClose = 0;

    window = crystalCreateWindow(&result);

    if (window == 0 || result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail("crystalCreateWindow", result);
        return 1;
    }

    pass("crystalCreateWindow");

    crystalSetWindowClosingCallback(window, onWindowClosing);

    drainEvents();

    crystalCloseWindow(window, &result);

    expectStatus(
        "crystalCloseWindow canceled by callback",
        result,
        CATALYST_STATUS_CODE_SUCCESS_NOOP
    );

    expectInt(
        "close callback count after canceled close",
        1,
        appState.closeRequests
    );

    drainEvents();

    appState.allowClose = 1;

    crystalCloseWindow(window, &result);

    expectStatus(
        "crystalCloseWindow allowed by callback",
        result,
        CATALYST_STATUS_CODE_SUCCESS
    );

    expectInt(
        "close callback count after allowed close",
        2,
        appState.closeRequests
    );

    if (failed != 0) {
        printf("\n%d CRYSTAL close window test(s) failed.\n", failed);
        return 1;
    }

    printf("\nAll CRYSTAL close window tests passed.\n");
    return 0;
}
