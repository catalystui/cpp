#ifndef CATTELIB_H
#define CATTELIB_H
#include "CATCRLIB.h"
#ifdef __cplusplus
extern "C" {
#endif

// Export API
#if defined(_WIN32) || defined(__CYGWIN__)
    #if defined(CATTELIB_EXPORTS)
        #define CATTELIB_API __declspec(dllexport)
    #elif defined(CATTELIB_USE_DLL)
        #define CATTELIB_API __declspec(dllimport)
    #else
        #define CATTELIB_API
    #endif
#else
    #define CATTELIB_API
#endif

CATTELIB_API void catteAsciiEncode(const CATALYST_CODEPOINT codepoint, CATALYST_ASCIIW asciiw, CATALYST_RESULT* result);

CATTELIB_API void catteAsciiDecode(CATALYST_ASCII ascii, CATALYST_CODEPOINT* codepoint, CATALYST_RESULT* result);

CATTELIB_API void catteAsciiLength(CATALYST_ASCII ascii, CATALYST_NUINT* length, CATALYST_RESULT* result);

CATTELIB_API void catteCp1252Encode(const CATALYST_CODEPOINT codepoint, CATALYST_CP1252W cp1252w, CATALYST_RESULT* result);

CATTELIB_API void catteCp1252Decode(CATALYST_CP1252 cp1252, CATALYST_CODEPOINT* codepoint, CATALYST_RESULT* result);

CATTELIB_API void catteCp1252Length(CATALYST_CP1252 cp1252, CATALYST_NUINT* length, CATALYST_RESULT* result);

CATTELIB_API void catteUtf8Encode(const CATALYST_CODEPOINT codepoint, CATALYST_UTF8W utf8w, CATALYST_NUINT* produced, CATALYST_RESULT* result);

CATTELIB_API void catteUtf8Decode(CATALYST_UTF8 utf8, CATALYST_CODEPOINT* codepoint, CATALYST_NUINT* consumed, CATALYST_RESULT* result);

CATTELIB_API void catteUtf8Length(CATALYST_UTF8 utf8, CATALYST_NUINT* length, CATALYST_RESULT* result);

CATTELIB_API void catteUtf16leEncode(const CATALYST_CODEPOINT codepoint, CATALYST_UTF16LEW utf16lew, CATALYST_NUINT* produced, CATALYST_RESULT* result);

CATTELIB_API void catteUtf16leDecode(CATALYST_UTF16LE utf16le, CATALYST_CODEPOINT* codepoint, CATALYST_NUINT* consumed, CATALYST_RESULT* result);

CATTELIB_API void catteUtf16leLength(CATALYST_UTF16LE utf16le, CATALYST_NUINT* length, CATALYST_RESULT* result);

CATTELIB_API void catteAsciiToCp1252(CATALYST_ASCII ascii, CATALYST_CP1252W cp1252w, CATALYST_RESULT* result);

CATTELIB_API void catteAsciiToUtf8(CATALYST_ASCII ascii, CATALYST_UTF8W utf8w, CATALYST_NUINT capacity, CATALYST_RESULT* result);

CATTELIB_API void catteAsciiToUtf16le(CATALYST_ASCII ascii, CATALYST_UTF16LEW utf16lew, CATALYST_NUINT capacity, CATALYST_RESULT* result);

CATTELIB_API void catteCp1252ToAscii(CATALYST_CP1252 cp1252, CATALYST_ASCIIW ascii, CATALYST_RESULT* result);

CATTELIB_API void catteCp1252ToUtf8(CATALYST_CP1252 cp1252, CATALYST_UTF8W utf8w, CATALYST_NUINT capacity, CATALYST_RESULT* result);

CATTELIB_API void catteCp1252ToUtf16le(CATALYST_CP1252 cp1252, CATALYST_UTF16LEW utf16lew, CATALYST_NUINT capacity, CATALYST_RESULT* result);

CATTELIB_API void catteUtf8ToAscii(CATALYST_UTF8 utf8, CATALYST_ASCIIW ascii, CATALYST_NUINT capacity, CATALYST_RESULT* result);

CATTELIB_API void catteUtf8ToCp1252(CATALYST_UTF8 utf8, CATALYST_CP1252W cp1252w, CATALYST_NUINT capacity, CATALYST_RESULT* result);

CATTELIB_API void catteUtf8ToUtf16le(CATALYST_UTF8 utf8, CATALYST_UTF16LEW utf16lew, CATALYST_NUINT capacity, CATALYST_RESULT* result);

CATTELIB_API void catteUtf16leToAscii(CATALYST_UTF16LE utf16le, CATALYST_ASCIIW ascii, CATALYST_NUINT capacity, CATALYST_RESULT* result);

CATTELIB_API void catteUtf16leToCp1252(CATALYST_UTF16LE utf16le, CATALYST_CP1252W cp1252w, CATALYST_NUINT capacity, CATALYST_RESULT* result);

CATTELIB_API void catteUtf16leToUtf8(CATALYST_UTF16LE utf16le, CATALYST_UTF8W utf8w, CATALYST_NUINT capacity, CATALYST_RESULT* result);

#ifdef __cplusplus
} /* extern C */
#endif
#endif /* CATTELIB_H */
