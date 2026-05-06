#ifndef CATCRLIB_C99_H
#define CATCRLIB_C99_H

#include <stdint.h>

/* -------------------------------------------------------------------------------------------------
 * C API :: NUMERICS
 * ------------------------------------------------------------------------------------------------- */

/* BYTE, SBYTE (8-bit types) */
#if defined(INT8_MAX) && defined(UINT8_MAX)
    typedef uint8_t CATALYST_BYTE;
    typedef int8_t CATALYST_SBYTE;

    #define CATALYST_BYTE_MIN  ((CATALYST_BYTE) 0)
    #define CATALYST_BYTE_MAX  ((CATALYST_BYTE) UINT8_MAX)
    #define CATALYST_SBYTE_MIN ((CATALYST_SBYTE) INT8_MIN)
    #define CATALYST_SBYTE_MAX ((CATALYST_SBYTE) INT8_MAX)
#else
    #error "CATCRLIB.h :: Unsupported Platform :: Platform must provide fixed-width 8-bit integer types for BYTE and SBYTE."
#endif

/* SHORT, USHORT (16-bit types) */
#if defined(INT16_MAX) && defined(UINT16_MAX)
    #define CATCRLIB_SUPPORTS_16BIT 1
    typedef int16_t CATALYST_SHORT;
    typedef uint16_t CATALYST_USHORT;

    #define CATALYST_SHORT_MIN  ((CATALYST_SHORT) INT16_MIN)
    #define CATALYST_SHORT_MAX  ((CATALYST_SHORT) INT16_MAX)
    #define CATALYST_USHORT_MIN ((CATALYST_USHORT) 0)
    #define CATALYST_USHORT_MAX ((CATALYST_USHORT) UINT16_MAX)
#else
    #define CATCRLIB_SUPPORTS_16BIT 0
#endif

/* INT, UINT (32-bit types) */
#if defined(INT32_MAX) && defined(UINT32_MAX)
    #define CATCRLIB_SUPPORTS_32BIT 1
    typedef int32_t CATALYST_INT;
    typedef uint32_t CATALYST_UINT;

    #define CATALYST_INT_MIN  ((CATALYST_INT) INT32_MIN)
    #define CATALYST_INT_MAX  ((CATALYST_INT) INT32_MAX)
    #define CATALYST_UINT_MIN ((CATALYST_UINT) 0)
    #define CATALYST_UINT_MAX ((CATALYST_UINT) UINT32_MAX)
#else
    #define CATCRLIB_SUPPORTS_32BIT 0
#endif

/* LONG, ULONG (64-bit types) */
#if defined(INT64_MAX) && defined(UINT64_MAX)
    #define CATCRLIB_SUPPORTS_64BIT 1
    typedef int64_t CATALYST_LONG;
    typedef uint64_t CATALYST_ULONG;

    #define CATALYST_LONG_MIN  ((CATALYST_LONG) INT64_MIN)
    #define CATALYST_LONG_MAX  ((CATALYST_LONG) INT64_MAX)
    #define CATALYST_ULONG_MIN ((CATALYST_ULONG) 0)
    #define CATALYST_ULONG_MAX ((CATALYST_ULONG) UINT64_MAX)
#else
    #define CATCRLIB_SUPPORTS_64BIT 0
#endif

#endif /* CATCRLIB_C99_H */
