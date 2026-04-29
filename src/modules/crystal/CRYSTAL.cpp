#include "CMAKECFG.h"

#if TARGET_PLATFORM_WIN32
    #include "CRYSTAL.win32.cpp"
#elif TARGET_PLATFORM_DARWIN
    /* Defined in Objective-C++, linked via CMake. */
#else
    #error "CRYSTAL unknown platform. Cannot define natives."
#endif
