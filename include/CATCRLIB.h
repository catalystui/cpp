#ifndef CATCRLIB_H
#define CATCRLIB_H
#ifdef __cplusplus
extern "C" {
namespace catalyst {
#endif

#include "CMAKECFG.h"
#if defined(CMAKECFG_STDC_VERSION) && (CMAKECFG_STDC_VERSION >= 199901L)
#include "CATCRLIB.C99.h"
#else
#include "CATCRLIB.C90.h"
#endif

#include <float.h>
#include <assert.h>

/* -------------------------------------------------------------------------------------------------
 * C API :: NUMERICS
 * ------------------------------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------------------------------
 * C API :: TEXT ENCODING
 * ------------------------------------------------------------------------------------------------- */

typedef BYTE TEXT_ENCODING;

enum {
    TEXT_ENCODING_UNKNOWN = 0x0,
    TEXT_ENCODING_ASCII   = 0x1,
    TEXT_ENCODING_CP1252  = 0x2,
    TEXT_ENCODING_UTF8    = 0x3,
    TEXT_ENCODING_UTF16LE = 0x4
};

typedef struct CODEPOINT {
    BYTE byte0; /* Most significant byte */
    BYTE byte1;
    BYTE byte2;
    BYTE byte3; /* Least significant byte */
} CODEPOINT;

typedef BYTE ASCII_CODE_UNIT;
typedef ASCII_CODE_UNIT* ASCIIW;
typedef const ASCII_CODE_UNIT* ASCII;

typedef BYTE CP1252_CODE_UNIT;
typedef CP1252_CODE_UNIT* CP1252W;
typedef const CP1252_CODE_UNIT* CP1252;

typedef BYTE UTF8_CODE_UNIT;
typedef UTF8_CODE_UNIT* UTF8W;
typedef const UTF8_CODE_UNIT* UTF8;

typedef struct UTF16LE_CODE_UNIT {
    BYTE msb; /* Most significant byte of the code unit. */
    BYTE lsb; /* Least significant byte of the code unit. */

    #if defined(__cplusplus)
        CONSTEXPR UTF16LE_CODE_UNIT() : msb(0), lsb(0) {}
        CONSTEXPR UTF16LE_CODE_UNIT(BYTE msb, BYTE lsb) : msb(msb), lsb(lsb) {}
        #if CATCRLIB_SUPPORTS_16BIT
            explicit CONSTEXPR UTF16LE_CODE_UNIT(USHORT value) : msb((BYTE) ((value >> 8) & (USHORT) 0x00FFu)), lsb((BYTE) (value & (USHORT) 0x00FFu)) {}
            CONSTEXPR operator USHORT() const {
                return ((USHORT) msb << 8) | (USHORT) lsb;
            }
        #endif
    #endif
} UTF16LE_CODE_UNIT;
typedef UTF16LE_CODE_UNIT* UTF16LEW;
typedef const UTF16LE_CODE_UNIT* UTF16LE;

/* -------------------------------------------------------------------------------------------------
 * C API :: OPERATION STATUS
 * ------------------------------------------------------------------------------------------------- */

typedef BYTE STATUS_CODE;

enum {
    STATUS_CODE_SUCCESS_NONE = 0x00,
    STATUS_CODE_SUCCESS_NOOP = 0x01,

    STATUS_CODE_WARNING_NONE       = 0x40,
    STATUS_CODE_WARNING_PARTIAL    = 0x41,
    STATUS_CODE_WARNING_DEPRECATED = 0x42,

    STATUS_CODE_ERROR_NONE                 = 0x80,
    STATUS_CODE_ERROR_INVALID_ARGUMENT     = 0x81,
    STATUS_CODE_ERROR_INVALID_STATE        = 0x82,
    STATUS_CODE_ERROR_MALFORMED_INPUT      = 0x83,
    STATUS_CODE_ERROR_ACCESS_DENIED        = 0x84,
    STATUS_CODE_ERROR_NOT_IMPLEMENTED      = 0x85,
    STATUS_CODE_ERROR_SYSTEM_NOT_SUPPORTED = 0x86,
    STATUS_CODE_ERROR_TIMEOUT              = 0x87,
    STATUS_CODE_ERROR_NOT_FOUND            = 0x88,
    STATUS_CODE_ERROR_INTERRUPTED          = 0x89,
    STATUS_CODE_ERROR_BUFFER_OVERFLOW      = 0x90,
    STATUS_CODE_ERROR_ALLOCATION_FAILED    = 0x91,
    STATUS_CODE_ERROR_IO_ERROR             = 0xA0,

    STATUS_CODE_FATAL_NONE                 = 0xC0,
    STATUS_CODE_FATAL_INVARIANT_VIOLATION  = 0xC1
};

#define STATUS_CODE_LEVEL_MASK ((STATUS_CODE) 0xC0)

#define STATUS_CODE_IS_SUCCESS(status) ((((STATUS_CODE) (status)) & STATUS_CODE_LEVEL_MASK) == STATUS_CODE_SUCCESS_NONE)
#define STATUS_CODE_IS_WARNING(status) ((((STATUS_CODE) (status)) & STATUS_CODE_LEVEL_MASK) == STATUS_CODE_WARNING_NONE)
#define STATUS_CODE_IS_ERROR(status) ((((STATUS_CODE) (status)) & STATUS_CODE_LEVEL_MASK) == STATUS_CODE_ERROR_NONE)
#define STATUS_CODE_IS_FATAL(status) ((((STATUS_CODE) (status)) & STATUS_CODE_LEVEL_MASK) == STATUS_CODE_FATAL_NONE)

typedef BYTE CONTEXT_CODE;

typedef BYTE OPERATION_CODE;

typedef BYTE DETAIL_CODE;

typedef BYTE CONTEXT_CODE;
typedef BYTE OPERATION_CODE;
typedef BYTE DETAIL_CODE;

#define CONTEXT_CODE_NONE   ((CONTEXT_CODE) 0x00)
#define OPERATION_CODE_NONE ((OPERATION_CODE) 0x00)
#define DETAIL_CODE_NONE    ((DETAIL_CODE) 0x00)

typedef struct RESULT {
    STATUS_CODE status;
    CONTEXT_CODE context;
    OPERATION_CODE operation;
    DETAIL_CODE detail;

    #if defined(__cplusplus)
        CONSTEXPR RESULT() : status(STATUS_CODE_SUCCESS_NONE), context(CONTEXT_CODE_NONE), operation(OPERATION_CODE_NONE), detail(DETAIL_CODE_NONE) {}
        CONSTEXPR RESULT(STATUS_CODE status, CONTEXT_CODE context, OPERATION_CODE operation, DETAIL_CODE detail) : status(status), context(context), operation(operation), detail(detail) {}
        #if CATCRLIB_SUPPORTS_32BIT
            explicit CONSTEXPR RESULT(UINT value) : status((STATUS_CODE) ((value >> 24) & (UINT) 0x000000FFu)), context((CONTEXT_CODE) ((value >> 16) & (UINT) 0x000000FFu)), operation((OPERATION_CODE) ((value >> 8) & (UINT) 0x000000FFu)), detail((DETAIL_CODE) (value & (UINT) 0x000000FFu)) {}
            CONSTEXPR operator UINT() const {
                return ((UINT) status << 24) | ((UINT) context << 16) | ((UINT) operation << 8) | (UINT) detail;
            }
        #endif
    #endif
} RESULT;

#ifdef __cplusplus
} /* namespace catalyst */
} /* extern C */
#endif
#endif /* CATCRLIB_H */
