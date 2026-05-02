#ifndef CRYSTAL_LINUX_HPP
#define CRYSTAL_LINUX_HPP
#include "CRYSTAL.hpp"
#include "CRYSTAL.linux.h"
namespace catalyst {
namespace modules {
namespace crystal {

    typedef ::CRYSTAL_DISPLAY_SERVER DISPLAY_SERVER;
    static const DISPLAY_SERVER DISPLAY_SERVER_NONE = CRYSTAL_DISPLAY_SERVER_NONE;
    static const DISPLAY_SERVER DISPLAY_SERVER_X11 = CRYSTAL_DISPLAY_SERVER_X11;
    static const DISPLAY_SERVER DISPLAY_SERVER_WAYLAND = CRYSTAL_DISPLAY_SERVER_WAYLAND;

    inline CRYSTAL_DISPLAY_SERVER getDisplayServer(CATALYST_RESULT* result) {
        return ::crystalGetDisplayServer(result);
    }

}
}
}
#endif /* CRYSTAL_LINUX_HPP */
