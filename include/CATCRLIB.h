#ifndef CATCRLIB_H
#define CATCRLIB_H
#ifdef __cplusplus
extern "C" {
namespace cat {
#endif

#include "CMAKECFG.h"
#if defined(CMAKECFG_STDC_VERSION) && (CMAKECFG_STDC_VERSION >= 199901L)
#include "CATCRLIB.C99.h"
#else
#include "CATCRLIB.C90.h"
#endif

#include <float.h>
#include <assert.h>

#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MIN_EXP == -125 && FLT_MAX_EXP == 128)
    #define CATCRLIB_SUPPORTS_SINGLE 1
    typedef char CATCRLIB_requires_float_4_bytes[(sizeof(float) == 4) ? 1 : -1]; // Compile-time check for 32-bit float
    typedef float SINGLE;
#else
    #define CATCRLIB_SUPPORTS_SINGLE 0
#endif

#if (FLT_RADIX == 2 && DBL_MANT_DIG == 53 && DBL_MIN_EXP == -1021 && DBL_MAX_EXP == 1024)
    #define CATCRLIB_SUPPORTS_DOUBLE 1
    typedef char CATCRLIB_requires_double_8_bytes[(sizeof(double) == 8) ? 1 : -1]; // Compile-time check for 64-bit double
    typedef double DOUBLE;
#else
    #define CATCRLIB_SUPPORTS_DOUBLE 0
#endif

typedef BYTE BOOL;
#define TRUE    (BOOL) 1
#define FALSE   (BOOL) 0

#ifndef __SIZEOF_VOID_P__
    #error "CATCRLIB.h :: Unsupported Platform :: Unable to determine pointer size, __SIZEOF_VOID_P__ was not defined."
#endif
#if __SIZEOF_VOID_P__ == 1
    typedef SBYTE NINT;
    typedef BYTE NUINT;
#elif __SIZEOF_VOID_P__ == 2
    #if CATCRLIB_SUPPORTS_16BIT
        typedef SHORT NINT;
        typedef USHORT NUINT;
    #else
        #error "CATCRLIB.h :: Unsupported Platform :: Compiler specifies 16-bit pointers but no 16-bit integer type exists."
    #endif
#elif __SIZEOF_VOID_P__ == 4
    #if CATCRLIB_SUPPORTS_32BIT
        typedef INT NINT;
        typedef UINT NUINT;
    #else
        #error "CATCRLIB.h :: Unsupported Platform :: Compiler specifies 32-bit pointers but no 32-bit integer type exists."
    #endif
#elif __SIZEOF_VOID_P__ == 8
    #if CATCRLIB_SUPPORTS_64BIT
        typedef LONG NINT;
        typedef ULONG NUINT;
    #else
        #error "CATCRLIB.h :: Unsupported Platform :: Compiler specifies 64-bit pointers but no 64-bit integer type exists."
    #endif
#else
    #error "CATCRLIB.h :: Unsupported Platform :: Unsupported pointer size specified by compiler."
#endif

typedef void VOID;

#ifdef __cplusplus
} /* namespace cat */
} /* extern C */
#endif
#endif /* CATCRLIB_H */
