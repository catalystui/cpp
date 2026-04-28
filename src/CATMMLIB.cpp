#if TARGET_PLATFORM_WIN32
    #error "CATMMLIB is not supported on Windows."
#elif TARGET_PLATFORM_DARWIN
    #include "CATMMLIB.darwin.cpp"
#else
    #error "CATMMLIB unknown platform. Cannot define natives."
#endif
