#ifndef CATMMLIB_H
#define CATMMLIB_H
#include "CATCRLIB.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Export API */
#if defined(_WIN32) || defined(__CYGWIN__)
    #if defined(CATMMLIB_EXPORTS)
        #define CATMMLIB_API __declspec(dllexport)
    #elif defined(CATMMLIB_USE_DLL)
        #define CATMMLIB_API __declspec(dllimport)
    #else
        #define CATMMLIB_API
    #endif
#else
    #define CATMMLIB_API
#endif

typedef struct CATALYST_MEMORY_HEADER {
    CATALYST_NUINT requested_size;
    CATALYST_NUINT allocated_size;
} CATALYST_MEMORY_HEADER;

CATMMLIB_API void catmmAlloc(void** memory, CATALYST_NUINT size, CATALYST_RESULT* result);

CATMMLIB_API void catmmRealloc(void** memory, CATALYST_NUINT size, CATALYST_RESULT* result);

CATMMLIB_API void catmmFree(void* memory, CATALYST_RESULT* result);

#ifdef __cplusplus
} /* extern C */
#endif
#endif /* CATMMLIB_H */
