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

inline void destroyWindow(window* window, RESULT* result) {
    ::crystalDestroyWindow(window, result);
}

inline void pollEvents() {
    ::crystalPollEvents();
}

inline void waitEvents() {
    ::crystalWaitEvents();
}

inline void setUserPointer(window* window, void* pointer) {
    ::crystalSetUserPointer(window, pointer);
}

inline void* getUserPointer(window* window) {
    return ::crystalGetUserPointer(window);
}

inline void setWindowClosingCallback(window* window, windowClosingCallback callback) {
    ::crystalSetWindowClosingCallback(window, callback);
}

} /* namespace crystal */
} /* namespace modules */
} /* namespace catalyst */
#endif /* CRYSTAL_HPP */
