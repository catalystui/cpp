#ifndef CRYSTAL_HPP
#define CRYSTAL_HPP
#include "CATCRLIB.hpp"
#include "CRYSTAL.h"
namespace catalyst {
namespace modules {
namespace crystal {

typedef ::CRYSTALwindow window;
typedef ::CRYSTALnative native;

typedef ::CRYSTAL_PROPERTIES_STYLE PROPERTIES_STYLE;

static const CRYSTAL_PROPERTIES_STYLE PROPERTIES_STYLE_NONE = CRYSTAL_PROPERTIES_STYLE_NONE;
static const CRYSTAL_PROPERTIES_STYLE PROPERTIES_STYLE_DECORATED = CRYSTAL_PROPERTIES_STYLE_DECORATED;
static const CRYSTAL_PROPERTIES_STYLE PROPERTIES_STYLE_MINIMIZABLE = CRYSTAL_PROPERTIES_STYLE_MINIMIZABLE;
static const CRYSTAL_PROPERTIES_STYLE PROPERTIES_STYLE_MAXIMIZABLE = CRYSTAL_PROPERTIES_STYLE_MAXIMIZABLE;
static const CRYSTAL_PROPERTIES_STYLE PROPERTIES_STYLE_RESIZABLE = CRYSTAL_PROPERTIES_STYLE_RESIZABLE;

typedef ::CRYSTAL_PROPERTIES_STATE PROPERTIES_STATE;

static const CRYSTAL_PROPERTIES_STATE PROPERTIES_STATE_NONE = CRYSTAL_PROPERTIES_STATE_NONE;
static const CRYSTAL_PROPERTIES_STATE PROPERTIES_STATE_FOCUSED = CRYSTAL_PROPERTIES_STATE_FOCUSED;
static const CRYSTAL_PROPERTIES_STATE PROPERTIES_STATE_UNFOCUSED = CRYSTAL_PROPERTIES_STATE_UNFOCUSED;
static const CRYSTAL_PROPERTIES_STATE PROPERTIES_STATE_MINIMIZED = CRYSTAL_PROPERTIES_STATE_MINIMIZED;
static const CRYSTAL_PROPERTIES_STATE PROPERTIES_STATE_MAXIMIZED = CRYSTAL_PROPERTIES_STATE_MAXIMIZED;
static const CRYSTAL_PROPERTIES_STATE PROPERTIES_STATE_VISIBLE = CRYSTAL_PROPERTIES_STATE_VISIBLE;
static const CRYSTAL_PROPERTIES_STATE PROPERTIES_STATE_HIDDEN = CRYSTAL_PROPERTIES_STATE_HIDDEN;

typedef ::CRYSTALwindowClosingCallback windowClosingCallback;

inline window* createWindow(RESULT* result) {
    return ::crystalCreateWindow(result);
}

inline void pollEvents() {
    ::crystalPollEvents();
}

inline void waitEvents() {
    ::crystalWaitEvents();
}

inline void setWindowTitle(window* window, UTF8 title, RESULT* result) {
    ::crystalSetWindowTitle(window, title, result);
}

inline void getWindowTitle(window* window, UTF8W title, NUINT capacity, NUINT* length, RESULT* result) {
    ::crystalGetWindowTitle(window, title, capacity, length, result);
}

inline void setWindowPosition(window* window, NUINT x, NUINT y, RESULT* result) {
    ::crystalSetWindowPosition(window, x, y, result);
}

inline void getWindowPosition(window* window, NUINT* x, NUINT* y, RESULT* result) {
    ::crystalGetWindowPosition(window, x, y, result);
}

inline void setWindowSize(window* window, NUINT width, NUINT height, RESULT* result) {
    ::crystalSetWindowSize(window, width, height, result);
}

inline void getWindowSize(window* window, NUINT* width, NUINT* height, RESULT* result) {
    ::crystalGetWindowSize(window, width, height, result);
}

inline void setWindowSizeLimits(window* window, NUINT minWidth, NUINT minHeight, NUINT maxWidth, NUINT maxHeight, RESULT* result) {
    ::crystalSetWindowSizeLimits(window, minWidth, minHeight, maxWidth, maxHeight, result);
}

inline void getWindowSizeLimits(window* window, NUINT* minWidth, NUINT* minHeight, NUINT* maxWidth, NUINT* maxHeight, RESULT* result) {
    ::crystalGetWindowSizeLimits(window, minWidth, minHeight, maxWidth, maxHeight, result);
}

inline void setWindowStyle(window* window, CRYSTAL_PROPERTIES_STYLE style, RESULT* result) {
    ::crystalSetWindowStyle(window, style, result);
}

inline void getWindowStyle(window* window, CRYSTAL_PROPERTIES_STYLE* style, RESULT* result) {
    ::crystalGetWindowStyle(window, style, result);
}

inline void requestWindowFocus(window* window, RESULT* result) {
    ::crystalRequestWindowFocus(window, result);
}

inline void requestWindowAttention(window* window, RESULT* result) {
    ::crystalRequestWindowAttention(window, result);
}

inline void minimizeWindow(window* window, RESULT* result) {
    ::crystalMinimizeWindow(window, result);
}

inline void maximizeWindow(window* window, RESULT* result) {
    ::crystalMaximizeWindow(window, result);
}

inline void restoreWindow(window* window, RESULT* result) {
    ::crystalRestoreWindow(window, result);
}

inline void showWindow(window* window, RESULT* result) {
    ::crystalShowWindow(window, result);
}

inline void hideWindow(window* window, RESULT* result) {
    ::crystalHideWindow(window, result);
}

inline void getWindowState(window* window, CRYSTAL_PROPERTIES_STATE* state) {
    ::crystalGetWindowState(window, state);
}

inline void destroyWindow(window* window, RESULT* result) {
    ::crystalDestroyWindow(window, result);
}

inline void setWindowClosingCallback(window* window, windowClosingCallback callback) {
    ::crystalSetWindowClosingCallback(window, callback);
}

} /* namespace crystal */
} /* namespace modules */
} /* namespace catalyst */
#endif /* CRYSTAL_HPP */
