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
#if (CHAR_MAX == 0x7F && UCHAR_MAX == 0xFFU)
    typedef unsigned char CATALYST_BYTE;
    typedef signed char CATALYST_SBYTE;
#else
    #error "CATCRLIB.h :: Unsupported Platform :: Platform must provide fixed-width 8-bit integer types for BYTE and SBYTE."
#endif

/* SHORT, USHORT (16-bit types) */
#if (SHRT_MAX == 0x7FFF && USHRT_MAX == 0xFFFFU)
    #define CATCRLIB_SUPPORTS_16BIT 1
    typedef signed short CATALYST_SHORT;
    typedef unsigned short CATALYST_USHORT;
#else
    #define CATCRLIB_SUPPORTS_16BIT 0
#endif

/* INT, UINT (32-bit types) */
#if (INT_MAX == 0x7FFFFFFFL && UINT_MAX == 0xFFFFFFFFUL)
    #define CATCRLIB_SUPPORTS_32BIT 1
    typedef signed int CATALYST_INT;
    typedef unsigned int CATALYST_UINT;
#else
    #define CATCRLIB_SUPPORTS_32BIT 0
#endif

/* LONG, ULONG (64-bit types) */
#define CATCRLIB_SUPPORTS_64BIT 0

#endif /* CATCRLIB_C90_H */
