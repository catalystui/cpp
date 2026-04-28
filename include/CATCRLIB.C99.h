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
#else
    #error "CATCRLIB.h :: Unsupported Platform :: Platform must provide fixed-width 8-bit integer types for BYTE and SBYTE."
#endif

/* SHORT, USHORT (16-bit types) */
#if defined(INT16_MAX) && defined(UINT16_MAX)
    #define CATCRLIB_SUPPORTS_16BIT 1
    typedef int16_t CATALYST_SHORT;
    typedef uint16_t CATALYST_USHORT;
#else
    #define CATCRLIB_SUPPORTS_16BIT 0
#endif

/* INT, UINT (32-bit types) */
#if defined(INT32_MAX) && defined(UINT32_MAX)
    #define CATCRLIB_SUPPORTS_32BIT 1
    typedef int32_t CATALYST_INT;
    typedef uint32_t CATALYST_UINT;
#else
    #define CATCRLIB_SUPPORTS_32BIT 0
#endif

/* LONG, ULONG (64-bit types) */
#if defined(INT64_MAX) && defined(UINT64_MAX)
    #define CATCRLIB_SUPPORTS_64BIT 1
    typedef int64_t CATALYST_LONG;
    typedef uint64_t CATALYST_ULONG;
#else
    #define CATCRLIB_SUPPORTS_64BIT 0
#endif

#endif /* CATCRLIB_C99_H */
