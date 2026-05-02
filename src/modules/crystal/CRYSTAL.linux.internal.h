#ifndef CRYSTAL_LINUX_INTERNAL_H
#define CRYSTAL_LINUX_INTERNAL_H
#include "CATCRLIB.hpp"
#include "modules/crystal/CRYSTAL.h"
#include "modules/crystal/CRYSTAL.linux.h"
#include "CRYSTAL.internal.h"

/* X11 */
CRYSTALwindow* crystalCreateWindowX11(CATALYST_RESULT* result);
void crystalProcessEventsX11(CATALYST_BOOL wait);
void crystalSetWindowTitleX11(CRYSTALwindow* window, CATALYST_UTF8 title, CATALYST_RESULT* result);
void crystalGetWindowTitleX11(CRYSTALwindow* window, CATALYST_UTF8W title, CATALYST_NUINT capacity, CATALYST_NUINT* length, CATALYST_RESULT* result);
void crystalSetWindowPositionX11(CRYSTALwindow* window, CATALYST_NUINT x, CATALYST_NUINT y, CATALYST_RESULT* result);
void crystalGetWindowPositionX11(CRYSTALwindow* window, CATALYST_NUINT* x, CATALYST_NUINT* y, CATALYST_RESULT* result);
void crystalSetWindowSizeX11(CRYSTALwindow* window, CATALYST_NUINT width, CATALYST_NUINT height, CATALYST_RESULT* result);
void crystalGetWindowSizeX11(CRYSTALwindow* window, CATALYST_NUINT* width, CATALYST_NUINT* height, CATALYST_RESULT* result);
void crystalSetWindowSizeLimitsX11(CRYSTALwindow* window, CATALYST_NUINT minWidth, CATALYST_NUINT minHeight, CATALYST_NUINT maxWidth, CATALYST_NUINT maxHeight, CATALYST_RESULT* result);
void crystalGetWindowSizeLimitsX11(CRYSTALwindow* window, CATALYST_NUINT* minWidth, CATALYST_NUINT* minHeight, CATALYST_NUINT* maxWidth, CATALYST_NUINT* maxHeight, CATALYST_RESULT* result);
void crystalSetWindowStyleX11(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE style, CATALYST_RESULT* result);
void crystalGetWindowStyleX11(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE* style, CATALYST_RESULT* result);
void crystalRequestWindowFocusX11(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalRequestWindowAttentionX11(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalMinimizeWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalMaximizeWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalRestoreWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalShowWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalHideWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalGetWindowStateX11(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STATE* state);
void crystalCloseWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalDestroyWindowX11(CRYSTALwindow* window, CATALYST_RESULT* result);

/* Wayland */
CRYSTALwindow* crystalCreateWindowWayland(CATALYST_RESULT* result);
void crystalProcessEventsWayland(CATALYST_BOOL wait);
void crystalSetWindowTitleWayland(CRYSTALwindow* window, CATALYST_UTF8 title, CATALYST_RESULT* result);
void crystalGetWindowTitleWayland(CRYSTALwindow* window, CATALYST_UTF8W title, CATALYST_NUINT capacity, CATALYST_NUINT* length, CATALYST_RESULT* result);
void crystalSetWindowPositionWayland(CRYSTALwindow* window, CATALYST_NUINT x, CATALYST_NUINT y, CATALYST_RESULT* result);
void crystalGetWindowPositionWayland(CRYSTALwindow* window, CATALYST_NUINT* x, CATALYST_NUINT* y, CATALYST_RESULT* result);
void crystalSetWindowSizeWayland(CRYSTALwindow* window, CATALYST_NUINT width, CATALYST_NUINT height, CATALYST_RESULT* result);
void crystalGetWindowSizeWayland(CRYSTALwindow* window, CATALYST_NUINT* width, CATALYST_NUINT* height, CATALYST_RESULT* result);
void crystalSetWindowSizeLimitsWayland(CRYSTALwindow* window, CATALYST_NUINT minWidth, CATALYST_NUINT minHeight, CATALYST_NUINT maxWidth, CATALYST_NUINT maxHeight, CATALYST_RESULT* result);
void crystalGetWindowSizeLimitsWayland(CRYSTALwindow* window, CATALYST_NUINT* minWidth, CATALYST_NUINT* minHeight, CATALYST_NUINT* maxWidth, CATALYST_NUINT* maxHeight, CATALYST_RESULT* result);
void crystalSetWindowStyleWayland(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE style, CATALYST_RESULT* result);
void crystalGetWindowStyleWayland(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE* style, CATALYST_RESULT* result);
void crystalRequestWindowFocusWayland(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalRequestWindowAttentionWayland(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalMinimizeWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalMaximizeWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalRestoreWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalShowWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalHideWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalGetWindowStateWayland(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STATE* state);
void crystalCloseWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result);
void crystalDestroyWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result);

#endif /* CRYSTAL_LINUX_INTERNAL_H */
