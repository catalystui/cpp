#include "CMAKECFG.h"
#if TARGET_PLATFORM_LINUX

#include "CATCRLIB.hpp"
#include "CRYSTAL.internal.h"

CRYSTALwindow* crystalCreateWindowWayland(CATALYST_RESULT* result) {
    return 0;
}

void crystalProcessEventsWayland(CATALYST_BOOL wait) {

}

void crystalSetWindowTitleWayland(CRYSTALwindow* window, CATALYST_UTF8 title, CATALYST_RESULT* result) {

}

void crystalGetWindowTitleWayland(CRYSTALwindow* window, CATALYST_UTF8W title, CATALYST_NUINT capacity, CATALYST_NUINT* length, CATALYST_RESULT* result) {

}

void crystalSetWindowPositionWayland(CRYSTALwindow* window, CATALYST_NUINT x, CATALYST_NUINT y, CATALYST_RESULT* result) {

}

void crystalGetWindowPositionWayland(CRYSTALwindow* window, CATALYST_NUINT* x, CATALYST_NUINT* y, CATALYST_RESULT* result) {

}

void crystalSetWindowSizeWayland(CRYSTALwindow* window, CATALYST_NUINT width, CATALYST_NUINT height, CATALYST_RESULT* result) {

}

void crystalGetWindowSizeWayland(CRYSTALwindow* window, CATALYST_NUINT* width, CATALYST_NUINT* height, CATALYST_RESULT* result) {

}

void crystalSetWindowSizeLimitsWayland(CRYSTALwindow* window, CATALYST_NUINT minWidth, CATALYST_NUINT minHeight, CATALYST_NUINT maxWidth, CATALYST_NUINT maxHeight, CATALYST_RESULT* result) {

}

void crystalGetWindowSizeLimitsWayland(CRYSTALwindow* window, CATALYST_NUINT* minWidth, CATALYST_NUINT* minHeight, CATALYST_NUINT* maxWidth, CATALYST_NUINT* maxHeight, CATALYST_RESULT* result) {

}

void crystalSetWindowStyleWayland(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE style, CATALYST_RESULT* result) {

}

void crystalGetWindowStyleWayland(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE* style, CATALYST_RESULT* result) {

}

void crystalRequestWindowFocusWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalRequestWindowAttentionWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalMinimizeWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalMaximizeWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalRestoreWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalShowWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalHideWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalGetWindowStateWayland(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STATE* state) {

}

void crystalCloseWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalDestroyWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

#if TARGET_SHARED
extern "C" CRYSTALwindow* crystalCreateWindow(CATALYST_RESULT* result) {
    return crystalCreateWindowWayland(result);
}

extern "C" void crystalProcessEvents(CATALYST_BOOL wait) {
    crystalProcessEventsWayland(wait);
}

extern "C" void crystalSetWindowTitle(CRYSTALwindow* window, CATALYST_UTF8 title, CATALYST_RESULT* result) {
    crystalSetWindowTitleWayland(window, title, result);
}

extern "C" void crystalGetWindowTitle(CRYSTALwindow* window, CATALYST_UTF8W title, CATALYST_NUINT capacity, CATALYST_NUINT* length, CATALYST_RESULT* result) {
    crystalGetWindowTitleWayland(window, title, capacity, length, result);
}

extern "C" void crystalSetWindowPosition(CRYSTALwindow* window, CATALYST_NUINT x, CATALYST_NUINT y, CATALYST_RESULT* result) {
    crystalSetWindowPositionWayland(window, x, y, result);
}

extern "C" void crystalGetWindowPosition(CRYSTALwindow* window, CATALYST_NUINT* x, CATALYST_NUINT* y, CATALYST_RESULT* result) {
    crystalGetWindowPositionWayland(window, x, y, result);
}

extern "C" void crystalSetWindowSize(CRYSTALwindow* window, CATALYST_NUINT width, CATALYST_NUINT height, CATALYST_RESULT* result) {
    crystalSetWindowSizeWayland(window, width, height, result);
}

extern "C" void crystalGetWindowSize(CRYSTALwindow* window, CATALYST_NUINT* width, CATALYST_NUINT* height, CATALYST_RESULT* result) {
    crystalGetWindowSizeWayland(window, width, height, result);
}

extern "C" void crystalSetWindowSizeLimits(CRYSTALwindow* window, CATALYST_NUINT minWidth, CATALYST_NUINT minHeight, CATALYST_NUINT maxWidth, CATALYST_NUINT maxHeight, CATALYST_RESULT* result) {
    crystalSetWindowSizeLimitsWayland(window, minWidth, minHeight, maxWidth, maxHeight, result);
}

extern "C" void crystalGetWindowSizeLimits(CRYSTALwindow* window, CATALYST_NUINT* minWidth, CATALYST_NUINT* minHeight, CATALYST_NUINT* maxWidth, CATALYST_NUINT* maxHeight, CATALYST_RESULT* result) {
    crystalGetWindowSizeLimitsWayland(window, minWidth, minHeight, maxWidth, maxHeight, result);
}

extern "C" void crystalSetWindowStyle(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE style, CATALYST_RESULT* result) {
    crystalSetWindowStyleWayland(window, style, result);
}

extern "C" void crystalGetWindowStyle(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE* style, CATALYST_RESULT* result) {
    crystalGetWindowStyleWayland(window, style, result);
}

extern "C" void crystalRequestWindowFocus(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalRequestWindowFocusWayland(window, result);
}

extern "C" void crystalRequestWindowAttention(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalRequestWindowAttentionWayland(window, result);
}

extern "C" void crystalMinimizeWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalMinimizeWindowWayland(window, result);
}

extern "C" void crystalMaximizeWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalMaximizeWindowWayland(window, result);
}

extern "C" void crystalRestoreWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalRestoreWindowWayland(window, result);
}

extern "C" void crystalShowWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalShowWindowWayland(window, result);
}

extern "C" void crystalHideWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalHideWindowWayland(window, result);
}

extern "C" void crystalGetWindowState(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STATE* state) {
    crystalGetWindowStateWayland(window, state);
}

extern "C" void crystalCloseWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalCloseWindowWayland(window, result);
}

extern "C" void crystalDestroyWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalDestroyWindowWayland(window, result);
}
#endif /* TARGET_SHARED */

#endif /* TARGET_PLATFORM_LINUX */
