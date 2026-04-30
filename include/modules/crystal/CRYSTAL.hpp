#ifndef CRYSTAL_HPP
#define CRYSTAL_HPP
#include "CATCRLIB.hpp"
#include "CRYSTAL.h"
namespace catalyst {
namespace modules {
namespace crystal {

typedef ::CRYSTALnative native;
typedef ::CRYSTALwindow window;

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

inline void closeWindow(window* window, RESULT* result) {
    ::crystalCloseWindow(window, result);
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
