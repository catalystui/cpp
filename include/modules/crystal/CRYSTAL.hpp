#ifndef CRYSTAL_HPP
#define CRYSTAL_HPP
#include "CATCRLIB.hpp"
#include "CRYSTAL.h"
namespace catalyst {
namespace modules {
namespace crystal {

typedef ::CRYSTALnative native;
typedef ::CRYSTALwindow window;

inline window* createWindow(RESULT* result) {
    return ::crystalCreateWindow(result);
}

inline void destroyWindow(window* window, RESULT* result) {
    ::crystalDestroyWindow(window, result);
}

} /* namespace crystal */
} /* namespace modules */
} /* namespace catalyst */
#endif /* CRYSTAL_HPP */
