#ifndef CATCRLIB_C90_H
#define CATCRLIB_C90_H

#include <limits.h>

/* -------------------------------------------------------------------------------------------------
 * C API :: NUMERICS
 * ------------------------------------------------------------------------------------------------- */

/* CHAR_BIT */
#if (CHAR_BIT != 8)
    #error "CATCRLIB.h :: Unsupported Platform :: Platform must contain 8-bit bytes (CHAR_BIT == 8)."
#endif

/* BYTE, SBYTE (8-bit types) */
#if (SCHAR_MAX == 0x7F && UCHAR_MAX == 0xFFU)
    typedef unsigned char CATALYST_BYTE;
    typedef signed char CATALYST_SBYTE;

    #define CATALYST_BYTE_MIN  ((CATALYST_BYTE) 0)
    #define CATALYST_BYTE_MAX  ((CATALYST_BYTE) UCHAR_MAX)
    #define CATALYST_SBYTE_MIN ((CATALYST_SBYTE) SCHAR_MIN)
    #define CATALYST_SBYTE_MAX ((CATALYST_SBYTE) SCHAR_MAX)
#else
    #error "CATCRLIB.h :: Unsupported Platform :: Platform must provide fixed-width 8-bit integer types for BYTE and SBYTE."
#endif

/* SHORT, USHORT (16-bit types) */
#if (SHRT_MAX == 0x7FFF && USHRT_MAX == 0xFFFFU)
    #define CATCRLIB_SUPPORTS_16BIT 1
    typedef signed short CATALYST_SHORT;
    typedef unsigned short CATALYST_USHORT;

    #define CATALYST_SHORT_MIN  ((CATALYST_SHORT) SHRT_MIN)
    #define CATALYST_SHORT_MAX  ((CATALYST_SHORT) SHRT_MAX)
    #define CATALYST_USHORT_MIN ((CATALYST_USHORT) 0)
    #define CATALYST_USHORT_MAX ((CATALYST_USHORT) USHRT_MAX)
#else
    #define CATCRLIB_SUPPORTS_16BIT 0
#endif

/* INT, UINT (32-bit types) */
#if (INT_MAX == 0x7FFFFFFFL && UINT_MAX == 0xFFFFFFFFUL)
    #define CATCRLIB_SUPPORTS_32BIT 1
    typedef signed int CATALYST_INT;
    typedef unsigned int CATALYST_UINT;

    #define CATALYST_INT_MIN  ((CATALYST_INT) INT_MIN)
    #define CATALYST_INT_MAX  ((CATALYST_INT) INT_MAX)
    #define CATALYST_UINT_MIN ((CATALYST_UINT) 0)
    #define CATALYST_UINT_MAX ((CATALYST_UINT) UINT_MAX)
#else
    #define CATCRLIB_SUPPORTS_32BIT 0
#endif

/* LONG, ULONG (64-bit types) */
#define CATCRLIB_SUPPORTS_64BIT 0

#endif /* CATCRLIB_C90_H */
