#ifndef CATCRLIB_H
#define CATCRLIB_H

#include "CMAKECFG.h"
#if defined(TARGET_STDC_VERSION) && (TARGET_STDC_VERSION >= 199901L)
#include "CATCRLIB.C99.h"
#else
#include "CATCRLIB.C90.h"
#endif

#include <float.h>

/* CONSTEXPR Support */
#if defined(__cplusplus)
    #if __cplusplus >= 201103L
        #define CATALYST_CONSTEXPR constexpr
    #else
        #define CATALYST_CONSTEXPR
    #endif
#endif

/* -------------------------------------------------------------------------------------------------
 * C API :: NUMERICS
 * ------------------------------------------------------------------------------------------------- */

#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MIN_EXP == -125 && FLT_MAX_EXP == 128)
    #define CATCRLIB_SUPPORTS_SINGLE 1
    typedef char CATCRLIB_requires_float_4_bytes[(sizeof(float) == 4) ? 1 : -1]; /* Compile-time check for 32-bit float */
    typedef float CATALYST_SINGLE;
#else
    #define CATCRLIB_SUPPORTS_SINGLE 0
#endif

#if (FLT_RADIX == 2 && DBL_MANT_DIG == 53 && DBL_MIN_EXP == -1021 && DBL_MAX_EXP == 1024)
    #define CATCRLIB_SUPPORTS_DOUBLE 1
    typedef char CATCRLIB_requires_double_8_bytes[(sizeof(double) == 8) ? 1 : -1]; /* Compile-time check for 64-bit double */
    typedef double CATALYST_DOUBLE;
#else
    #define CATCRLIB_SUPPORTS_DOUBLE 0
#endif

typedef CATALYST_BYTE CATALYST_BOOL;
#define CATALYST_TRUE    ((CATALYST_BOOL) 1)
#define CATALYST_FALSE   ((CATALYST_BOOL) 0)

#ifndef TARGET_SIZEOF_VOID_P
    #error "CATCRLIB.h :: Unsupported Platform :: Unable to determine pointer size, TARGET_SIZEOF_VOID_P was not defined."
#endif
#if TARGET_SIZEOF_VOID_P == 1
    typedef CATALYST_SBYTE  CATALYST_NINT;
    typedef CATALYST_BYTE   CATALYST_NUINT;
#elif TARGET_SIZEOF_VOID_P == 2
    #if CATCRLIB_SUPPORTS_16BIT
        typedef CATALYST_SHORT  CATALYST_NINT;
        typedef CATALYST_USHORT CATALYST_NUINT;
    #else
        #error "CATCRLIB.h :: Unsupported Platform :: Compiler specifies 16-bit pointers but no 16-bit integer type exists."
    #endif
#elif TARGET_SIZEOF_VOID_P == 4
    #if CATCRLIB_SUPPORTS_32BIT
        typedef CATALYST_INT    CATALYST_NINT;
        typedef CATALYST_UINT   CATALYST_NUINT;
    #else
        #error "CATCRLIB.h :: Unsupported Platform :: Compiler specifies 32-bit pointers but no 32-bit integer type exists."
    #endif
#elif TARGET_SIZEOF_VOID_P == 8
    #if CATCRLIB_SUPPORTS_64BIT
        typedef CATALYST_LONG   CATALYST_NINT;
        typedef CATALYST_ULONG  CATALYST_NUINT;
    #else
        #error "CATCRLIB.h :: Unsupported Platform :: Compiler specifies 64-bit pointers but no 64-bit integer type exists."
    #endif
#else
    #error "CATCRLIB.h :: Unsupported Platform :: Unsupported pointer size specified by compiler."
#endif

/* -------------------------------------------------------------------------------------------------
 * C API :: TEXT ENCODING
 * ------------------------------------------------------------------------------------------------- */

typedef CATALYST_BYTE CATALYST_TEXT_ENCODING;

enum {
    CATALYST_TEXT_ENCODING_UNKNOWN = 0x0,
    CATALYST_TEXT_ENCODING_ASCII   = 0x1,
    CATALYST_TEXT_ENCODING_CP1252  = 0x2,
    CATALYST_TEXT_ENCODING_UTF8    = 0x3,
    CATALYST_TEXT_ENCODING_UTF16LE = 0x4
};

typedef struct CATALYST_CODEPOINT {
    CATALYST_BYTE byte0; /* Most significant byte */
    CATALYST_BYTE byte1;
    CATALYST_BYTE byte2;
    CATALYST_BYTE byte3; /* Least significant byte */
} CATALYST_CODEPOINT;

typedef CATALYST_BYTE CATALYST_ASCII_CODE_UNIT;
typedef CATALYST_ASCII_CODE_UNIT* CATALYST_ASCIIW;
typedef const CATALYST_ASCII_CODE_UNIT* CATALYST_ASCII;

typedef CATALYST_BYTE CATALYST_CP1252_CODE_UNIT;
typedef CATALYST_CP1252_CODE_UNIT* CATALYST_CP1252W;
typedef const CATALYST_CP1252_CODE_UNIT* CATALYST_CP1252;

typedef CATALYST_BYTE CATALYST_UTF8_CODE_UNIT;
typedef CATALYST_UTF8_CODE_UNIT* CATALYST_UTF8W;
typedef const CATALYST_UTF8_CODE_UNIT* CATALYST_UTF8;

typedef struct CATALYST_UTF16LE_CODE_UNIT {
    CATALYST_BYTE lsb; /* Least significant byte of the codepoint. */
    CATALYST_BYTE msb; /* Most significant byte of the codepoint. */

    #if defined(__cplusplus)
        CATALYST_CONSTEXPR CATALYST_UTF16LE_CODE_UNIT() : msb(0), lsb(0) {}
        CATALYST_CONSTEXPR CATALYST_UTF16LE_CODE_UNIT(CATALYST_BYTE msb, CATALYST_BYTE lsb) : msb(msb), lsb(lsb) {}
        #if CATCRLIB_SUPPORTS_16BIT
            explicit CATALYST_CONSTEXPR CATALYST_UTF16LE_CODE_UNIT(CATALYST_USHORT value) : msb((CATALYST_BYTE) ((value >> 8) & (CATALYST_USHORT) 0x00FFu)), lsb((CATALYST_BYTE) (value & (CATALYST_USHORT) 0x00FFu)) {}
            CATALYST_CONSTEXPR operator CATALYST_USHORT() const {
                return ((CATALYST_USHORT) msb << 8) | (CATALYST_USHORT) lsb;
            }
        #endif
    #endif
} CATALYST_UTF16LE_CODE_UNIT;
typedef CATALYST_UTF16LE_CODE_UNIT* CATALYST_UTF16LEW;
typedef const CATALYST_UTF16LE_CODE_UNIT* CATALYST_UTF16LE;

/* -------------------------------------------------------------------------------------------------
 * C API :: OPERATION STATUS
 * ------------------------------------------------------------------------------------------------- */

typedef CATALYST_BYTE CATALYST_STATUS_CODE;

enum {
    CATALYST_STATUS_CODE_SUCCESS      = 0x00,
    CATALYST_STATUS_CODE_SUCCESS_NOOP = 0x01,

    CATALYST_STATUS_CODE_WARNING            = 0x40,
    CATALYST_STATUS_CODE_WARNING_PARTIAL    = 0x41,
    CATALYST_STATUS_CODE_WARNING_DEPRECATED = 0x42,

    CATALYST_STATUS_CODE_ERROR                      = 0x80,
    CATALYST_STATUS_CODE_ERROR_INVALID_ARGUMENT     = 0x81,
    CATALYST_STATUS_CODE_ERROR_INVALID_STATE        = 0x82,
    CATALYST_STATUS_CODE_ERROR_MALFORMED_INPUT      = 0x83,
    CATALYST_STATUS_CODE_ERROR_ACCESS_DENIED        = 0x84,
    CATALYST_STATUS_CODE_ERROR_NOT_IMPLEMENTED      = 0x85,
    CATALYST_STATUS_CODE_ERROR_SYSTEM_NOT_SUPPORTED = 0x86,
    CATALYST_STATUS_CODE_ERROR_TIMEOUT              = 0x87,
    CATALYST_STATUS_CODE_ERROR_NOT_FOUND            = 0x88,
    CATALYST_STATUS_CODE_ERROR_INTERRUPTED          = 0x89,
    CATALYST_STATUS_CODE_ERROR_DEPENDENCY_FAILURE   = 0x8A,
    CATALYST_STATUS_CODE_ERROR_BUFFER_OVERFLOW      = 0x90,
    CATALYST_STATUS_CODE_ERROR_ALLOCATION_FAILED    = 0x91,
    CATALYST_STATUS_CODE_ERROR_IO_ERROR             = 0xA0,

    CATALYST_STATUS_CODE_FATAL                      = 0xC0,
    CATALYST_STATUS_CODE_FATAL_INVARIANT_VIOLATION  = 0xC1
};

#define CATALYST_STATUS_CODE_LEVEL_MASK ((CATALYST_STATUS_CODE) 0xC0)

#define CATALYST_STATUS_CODE_IS_SUCCESS(status) ((((CATALYST_STATUS_CODE) (status)) & CATALYST_STATUS_CODE_LEVEL_MASK) == CATALYST_STATUS_CODE_SUCCESS)
#define CATALYST_STATUS_CODE_IS_WARNING(status) ((((CATALYST_STATUS_CODE) (status)) & CATALYST_STATUS_CODE_LEVEL_MASK) == CATALYST_STATUS_CODE_WARNING)
#define CATALYST_STATUS_CODE_IS_ERROR(status) ((((CATALYST_STATUS_CODE) (status)) & CATALYST_STATUS_CODE_LEVEL_MASK) == CATALYST_STATUS_CODE_ERROR)
#define CATALYST_STATUS_CODE_IS_FATAL(status) ((((CATALYST_STATUS_CODE) (status)) & CATALYST_STATUS_CODE_LEVEL_MASK) == CATALYST_STATUS_CODE_FATAL)

typedef CATALYST_BYTE CATALYST_CONTEXT_CODE;
typedef CATALYST_BYTE CATALYST_OPERATION_CODE;
typedef CATALYST_BYTE CATALYST_DETAIL_CODE;

typedef struct CATALYST_RESULT {
    CATALYST_STATUS_CODE status;
    CATALYST_CONTEXT_CODE context;
    CATALYST_OPERATION_CODE operation;
    CATALYST_DETAIL_CODE detail;

    #if defined(__cplusplus)
        CATALYST_CONSTEXPR CATALYST_RESULT() : status(CATALYST_STATUS_CODE_SUCCESS), context(0x00), operation(0x00), detail(0x00) {}
        CATALYST_CONSTEXPR CATALYST_RESULT(CATALYST_STATUS_CODE status, CATALYST_CONTEXT_CODE context, CATALYST_OPERATION_CODE operation, CATALYST_DETAIL_CODE detail) : status(status), context(context), operation(operation), detail(detail) {}
        #if CATCRLIB_SUPPORTS_32BIT
            explicit CATALYST_CONSTEXPR CATALYST_RESULT(CATALYST_UINT value) : status((CATALYST_STATUS_CODE) ((value >> 24) & (CATALYST_UINT) 0x000000FFu)), context((CATALYST_CONTEXT_CODE) ((value >> 16) & (CATALYST_UINT) 0x000000FFu)), operation((CATALYST_OPERATION_CODE) ((value >> 8) & (CATALYST_UINT) 0x000000FFu)), detail((CATALYST_DETAIL_CODE) (value & (CATALYST_UINT) 0x000000FFu)) {}
            CATALYST_CONSTEXPR operator CATALYST_UINT() const {
                return ((CATALYST_UINT) status << 24) | ((CATALYST_UINT) context << 16) | ((CATALYST_UINT) operation << 8) | (CATALYST_UINT) detail;
            }
        #endif
    #endif
} CATALYST_RESULT;

#endif /* CATCRLIB_H */
