#include "CMAKECFG.h"
#include "modules/crystal/CRYSTAL.h"

#include <stdio.h>
#include <string.h>

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

static unsigned long stringLength(const char* text) {
    unsigned long length = 0;

    while (text[length] != '\0') {
        length += 1;
    }

    return length;
}

CATALYST_BOOL onWindowClosing(CRYSTALwindow* window) {
    appState.running = 0;
    return CATALYST_FALSE;
}

static void failTitleMismatch(const char* name, const char* expected, const char* actual) {
    printf("FAIL: %s\n", name);
    printf("  expected: \"%s\"\n", expected);
    printf("  actual:   \"%s\"\n", actual);
    failed += 1;
}

static void failLengthMismatch(const char* name, unsigned long expected, unsigned long actual) {
    printf("FAIL: %s length\n", name);
    printf("  expected: %lu\n", expected);
    printf("  actual:   %lu\n", actual);
    failed += 1;
}

static void testTitle(CRYSTALwindow* window, const char* name, const char* title, const char* expected) {
    CATALYST_RESULT result;
    CATALYST_BYTE buffer[512];
    CATALYST_NUINT length;
    unsigned long expectedLength;

    buffer[0] = '\0';
    length = 0;

    crystalSetWindowTitle(window, (CATALYST_UTF8) title, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail(name, result);
        return;
    }

    drainEvents();

    crystalGetWindowTitle(
        window,
        buffer,
        (CATALYST_NUINT) sizeof(buffer),
        &length,
        &result
    );

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail(name, result);
        return;
    }

    if (strcmp((const char*) buffer, expected) != 0) {
        failTitleMismatch(name, expected, (const char*) buffer);
        return;
    }

    expectedLength = stringLength(expected);

    if (length != (CATALYST_NUINT) expectedLength) {
        failLengthMismatch(name, expectedLength, (unsigned long) length);
        return;
    }

    printf("PASS: %s -> \"%s\"\n", name, (const char*) buffer);
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

    waitForEnter("Press Enter to set plain text title...");
    testTitle(
        window,
        "crystalSet/GetWindowTitle plain text",
        "Plain Text Title",
        "Plain Text Title"
    );

    waitForEnter("Inspect the plain text title, then press Enter to set emoji title...");

#if TARGET_SUPPORTS_UNICODE
    testTitle(
        window,
        "crystalSet/GetWindowTitle emoji title",
        "Emoji Title 😀 🚀 ✨",
        "Emoji Title 😀 🚀 ✨"
    );
#else
    testTitle(
        window,
        "crystalSet/GetWindowTitle emoji title",
        "Emoji Title 😀 🚀 ✨",
        "Emoji Title ? ? ?"
    );
#endif

    waitForEnter("Inspect the emoji title, then press Enter to set foreign-character title...");

#if TARGET_SUPPORTS_UNICODE
    testTitle(
        window,
        "crystalSet/GetWindowTitle foreign-character title",
        "Foreign Title: Español — Русский — 中文 — 日本語",
        "Foreign Title: Español — Русский — 中文 — 日本語"
    );
#else
    testTitle(
        window,
        "crystalSet/GetWindowTitle foreign-character title",
        "Foreign Title: Español — Русский — 中文 — 日本語",
        "Foreign Title: Español — ??????? — ?? — ???"
    );
#endif

    waitForEnter("Inspect the foreign-character title, then press Enter to destroy the window...");

    crystalDestroyWindow(window, &result);

    if (result.status != CATALYST_STATUS_CODE_SUCCESS) {
        fail("crystalDestroyWindow", result);
        return 1;
    }

    pass("crystalDestroyWindow");

    if (failed != 0) {
        printf("\n%d CRYSTAL test(s) failed.\n", failed);
        return 1;
    }

    printf("\nAll CRYSTAL title tests passed.\n");
    return 0;
}
