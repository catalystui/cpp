#ifndef CATCRLIB_HPP
#define CATCRLIB_HPP
#include "CATCRLIB.h"
namespace catalyst {

/* -------------------------------------------------------------------------------------------------
 * C++ API :: NUMERICS
 * ------------------------------------------------------------------------------------------------- */

typedef ::CATALYST_BYTE BYTE;
typedef ::CATALYST_SBYTE SBYTE;

#if CATCRLIB_SUPPORTS_16BIT
    typedef ::CATALYST_SHORT SHORT;
    typedef ::CATALYST_USHORT USHORT;
#endif

#if CATCRLIB_SUPPORTS_32BIT
    typedef ::CATALYST_INT INT;
    typedef ::CATALYST_UINT UINT;
#endif

#if CATCRLIB_SUPPORTS_64BIT
    typedef ::CATALYST_LONG LONG;
    typedef ::CATALYST_ULONG ULONG;
#endif

#if CATCRLIB_SUPPORTS_SINGLE
    typedef ::CATALYST_SINGLE SINGLE;
#endif

#if CATCRLIB_SUPPORTS_DOUBLE
    typedef ::CATALYST_DOUBLE DOUBLE;
#endif

typedef ::CATALYST_BOOL BOOL;
typedef ::CATALYST_NINT NINT;
typedef ::CATALYST_NUINT NUINT;

static const BOOL TRUE = CATALYST_TRUE;
static const BOOL FALSE = CATALYST_FALSE;

/* -------------------------------------------------------------------------------------------------
 * C++ API :: TEXT ENCODING
 * ------------------------------------------------------------------------------------------------- */

typedef ::CATALYST_TEXT_ENCODING TEXT_ENCODING;

static const TEXT_ENCODING TEXT_ENCODING_UNKNOWN = ::CATALYST_TEXT_ENCODING_UNKNOWN;
static const TEXT_ENCODING TEXT_ENCODING_ASCII = ::CATALYST_TEXT_ENCODING_ASCII;
static const TEXT_ENCODING TEXT_ENCODING_CP1252 = ::CATALYST_TEXT_ENCODING_CP1252;
static const TEXT_ENCODING TEXT_ENCODING_UTF8 = ::CATALYST_TEXT_ENCODING_UTF8;
static const TEXT_ENCODING TEXT_ENCODING_UTF16LE = ::CATALYST_TEXT_ENCODING_UTF16LE;

struct CODEPOINT : public ::CATALYST_CODEPOINT {
    CODEPOINT() {
        byte0 = (BYTE) 0;
        byte1 = (BYTE) 0;
        byte2 = (BYTE) 0;
        byte3 = (BYTE) 0;
    }
    CODEPOINT(BYTE byte0, BYTE byte1, BYTE byte2, BYTE byte3) {
        this->byte0 = byte0;
        this->byte1 = byte1;
        this->byte2 = byte2;
        this->byte3 = byte3;
    }
    CODEPOINT(const ::CATALYST_CODEPOINT& codepoint) {
        byte0 = codepoint.byte0;
        byte1 = codepoint.byte1;
        byte2 = codepoint.byte2;
        byte3 = codepoint.byte3;
    }
    CODEPOINT& operator=(const ::CATALYST_CODEPOINT& codepoint) {
        byte0 = codepoint.byte0;
        byte1 = codepoint.byte1;
        byte2 = codepoint.byte2;
        byte3 = codepoint.byte3;
        return *this;
    }

#if CATCRLIB_SUPPORTS_32BIT
    explicit CODEPOINT(UINT value) {
        byte0 = (BYTE) ((value >> 24) & (UINT) 0x000000FFu);
        byte1 = (BYTE) ((value >> 16) & (UINT) 0x000000FFu);
        byte2 = (BYTE) ((value >> 8)  & (UINT) 0x000000FFu);
        byte3 = (BYTE) ( value        & (UINT) 0x000000FFu);
    }
    CODEPOINT& operator=(UINT value) {
        byte0 = (BYTE) ((value >> 24) & (UINT) 0x000000FFu);
        byte1 = (BYTE) ((value >> 16) & (UINT) 0x000000FFu);
        byte2 = (BYTE) ((value >> 8)  & (UINT) 0x000000FFu);
        byte3 = (BYTE) ( value        & (UINT) 0x000000FFu);
        return *this;
    }
    operator UINT() const {
        return ((UINT) byte0 << 24) | ((UINT) byte1 << 16) | ((UINT) byte2 << 8) |  (UINT) byte3;
    }
#endif
};

typedef ::CATALYST_ASCII_CODE_UNIT ASCII_CODE_UNIT;
typedef ::CATALYST_ASCIIW ASCIIW;
typedef ::CATALYST_ASCII ASCII;

typedef ::CATALYST_CP1252_CODE_UNIT CP1252_CODE_UNIT;
typedef ::CATALYST_CP1252W CP1252W;
typedef ::CATALYST_CP1252 CP1252;

typedef ::CATALYST_UTF8_CODE_UNIT UTF8_CODE_UNIT;
typedef ::CATALYST_UTF8W UTF8W;
typedef ::CATALYST_UTF8 UTF8;

struct UTF16LE_CODE_UNIT : public ::CATALYST_UTF16LE_CODE_UNIT {
    UTF16LE_CODE_UNIT() {
        lsb = (BYTE) 0;
        msb = (BYTE) 0;
    }
    UTF16LE_CODE_UNIT(BYTE msb, BYTE lsb) {
        this->msb = msb;
        this->lsb = lsb;
    }
    UTF16LE_CODE_UNIT(const ::CATALYST_UTF16LE_CODE_UNIT& codeunit) {
        lsb = codeunit.lsb;
        msb = codeunit.msb;
    }
    UTF16LE_CODE_UNIT& operator=(const ::CATALYST_UTF16LE_CODE_UNIT& codeunit) {
        lsb = codeunit.lsb;
        msb = codeunit.msb;
        return *this;
    }

#if CATCRLIB_SUPPORTS_16BIT
    explicit UTF16LE_CODE_UNIT(USHORT value) {
        msb = (BYTE) ((value >> 8) & (USHORT) 0x00FFu);
        lsb = (BYTE) (value & (USHORT) 0x00FFu);
    }
    UTF16LE_CODE_UNIT& operator=(USHORT value) {
        msb = (BYTE) ((value >> 8) & (USHORT) 0x00FFu);
        lsb = (BYTE) (value & (USHORT) 0x00FFu);
        return *this;
    }
    operator USHORT() const {
        return ((USHORT) msb << 8) | (USHORT)lsb;
    }
#endif
};
typedef ::CATALYST_UTF16LEW UTF16LEW;
typedef ::CATALYST_UTF16LE UTF16LE;

/* -------------------------------------------------------------------------------------------------
 * C++ API :: OPERATION STATUS
 * ------------------------------------------------------------------------------------------------- */

typedef ::CATALYST_STATUS_CODE STATUS_CODE;

static const STATUS_CODE STATUS_CODE_SUCCESS = ::CATALYST_STATUS_CODE_SUCCESS;
static const STATUS_CODE STATUS_CODE_SUCCESS_NOOP = ::CATALYST_STATUS_CODE_SUCCESS_NOOP;

static const STATUS_CODE STATUS_CODE_WARNING = ::CATALYST_STATUS_CODE_WARNING;
static const STATUS_CODE STATUS_CODE_WARNING_PARTIAL = ::CATALYST_STATUS_CODE_WARNING_PARTIAL;
static const STATUS_CODE STATUS_CODE_WARNING_DEPRECATED = ::CATALYST_STATUS_CODE_WARNING_DEPRECATED;

static const STATUS_CODE STATUS_CODE_ERROR = ::CATALYST_STATUS_CODE_ERROR;
static const STATUS_CODE STATUS_CODE_ERROR_INVALID_ARGUMENT = ::CATALYST_STATUS_CODE_ERROR_INVALID_ARGUMENT;
static const STATUS_CODE STATUS_CODE_ERROR_INVALID_STATE = ::CATALYST_STATUS_CODE_ERROR_INVALID_STATE;
static const STATUS_CODE STATUS_CODE_ERROR_MALFORMED_INPUT = ::CATALYST_STATUS_CODE_ERROR_MALFORMED_INPUT;
static const STATUS_CODE STATUS_CODE_ERROR_ACCESS_DENIED = ::CATALYST_STATUS_CODE_ERROR_ACCESS_DENIED;
static const STATUS_CODE STATUS_CODE_ERROR_NOT_IMPLEMENTED = ::CATALYST_STATUS_CODE_ERROR_NOT_IMPLEMENTED;
static const STATUS_CODE STATUS_CODE_ERROR_SYSTEM_NOT_SUPPORTED = ::CATALYST_STATUS_CODE_ERROR_SYSTEM_NOT_SUPPORTED;
static const STATUS_CODE STATUS_CODE_ERROR_TIMEOUT = ::CATALYST_STATUS_CODE_ERROR_TIMEOUT;
static const STATUS_CODE STATUS_CODE_ERROR_NOT_FOUND = ::CATALYST_STATUS_CODE_ERROR_NOT_FOUND;
static const STATUS_CODE STATUS_CODE_ERROR_INTERRUPTED = ::CATALYST_STATUS_CODE_ERROR_INTERRUPTED;
static const STATUS_CODE STATUS_CODE_ERROR_DEPENDENCY_FAILURE = ::CATALYST_STATUS_CODE_ERROR_DEPENDENCY_FAILURE;
static const STATUS_CODE STATUS_CODE_ERROR_BUFFER_OVERFLOW = ::CATALYST_STATUS_CODE_ERROR_BUFFER_OVERFLOW;
static const STATUS_CODE STATUS_CODE_ERROR_ALLOCATION_FAILED = ::CATALYST_STATUS_CODE_ERROR_ALLOCATION_FAILED;
static const STATUS_CODE STATUS_CODE_ERROR_IO_ERROR = ::CATALYST_STATUS_CODE_ERROR_IO_ERROR;

static const STATUS_CODE STATUS_CODE_FATAL = ::CATALYST_STATUS_CODE_FATAL;
static const STATUS_CODE STATUS_CODE_FATAL_INVARIANT_VIOLATION = ::CATALYST_STATUS_CODE_FATAL_INVARIANT_VIOLATION;

static const STATUS_CODE STATUS_CODE_LEVEL_MASK = CATALYST_STATUS_CODE_LEVEL_MASK;

inline BOOL statusCodeIsSuccess(STATUS_CODE status) {
    return CATALYST_STATUS_CODE_IS_SUCCESS(status) ? TRUE : FALSE;
}

inline BOOL statusCodeIsWarning(STATUS_CODE status) {
    return CATALYST_STATUS_CODE_IS_WARNING(status) ? TRUE : FALSE;
}

inline BOOL statusCodeIsError(STATUS_CODE status) {
    return CATALYST_STATUS_CODE_IS_ERROR(status) ? TRUE : FALSE;
}

inline BOOL statusCodeIsFatal(STATUS_CODE status) {
    return CATALYST_STATUS_CODE_IS_FATAL(status) ? TRUE : FALSE;
}

typedef ::CATALYST_CONTEXT_CODE CONTEXT_CODE;
typedef ::CATALYST_OPERATION_CODE OPERATION_CODE;
typedef ::CATALYST_DETAIL_CODE DETAIL_CODE;

static const CONTEXT_CODE CONTEXT_CODE_NONE = (CONTEXT_CODE) 0x00;
static const OPERATION_CODE OPERATION_CODE_NONE = (OPERATION_CODE) 0x00;
static const DETAIL_CODE DETAIL_CODE_NONE = (DETAIL_CODE) 0x00;

struct RESULT : public ::CATALYST_RESULT {
    RESULT() {
        status = STATUS_CODE_SUCCESS;
        context = CONTEXT_CODE_NONE;
        operation = OPERATION_CODE_NONE;
        detail = DETAIL_CODE_NONE;
    }
    RESULT(STATUS_CODE status, CONTEXT_CODE context, OPERATION_CODE operation, DETAIL_CODE detail) {
        this->status = status;
        this->context = context;
        this->operation = operation;
        this->detail = detail;
    }
    RESULT(const ::CATALYST_RESULT& result) {
        status = result.status;
        context = result.context;
        operation = result.operation;
        detail = result.detail;
    }
    RESULT& operator=(const ::CATALYST_RESULT& result) {
        status = result.status;
        context = result.context;
        operation = result.operation;
        detail = result.detail;
        return *this;
    }

#if CATCRLIB_SUPPORTS_32BIT
    explicit RESULT(UINT value) {
        status = (STATUS_CODE) ((value >> 24) & (UINT) 0x000000FFu);
        context = (CONTEXT_CODE) ((value >> 16) & (UINT) 0x000000FFu);
        operation = (OPERATION_CODE) ((value >> 8) & (UINT) 0x000000FFu);
        detail = (DETAIL_CODE) (value & (UINT) 0x000000FFu);
    }
    RESULT& operator=(UINT value) {
        status = (STATUS_CODE) ((value >> 24) & (UINT) 0x000000FFu);
        context = (CONTEXT_CODE) ((value >> 16) & (UINT) 0x000000FFu);
        operation = (OPERATION_CODE) ((value >> 8) & (UINT) 0x000000FFu);
        detail = (DETAIL_CODE) (value & (UINT) 0x000000FFu);
        return *this;
    }
    operator UINT() const {
        return ((UINT) status << 24) | ((UINT) context << 16) | ((UINT) operation << 8) |  (UINT) detail;
    }
#endif
};

} /* namespace catalyst */
#endif /* CATCRLIB_HPP */
