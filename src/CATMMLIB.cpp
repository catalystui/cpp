#if TARGET_PLATFORM_WIN32
    #include "CATMMLIB.win32.cpp"
#elif TARGET_PLATFORM_DARWIN
    #include "CATMMLIB.darwin.cpp"
#else
    #error "CATMMLIB unknown platform. Cannot define natives."
#endif
