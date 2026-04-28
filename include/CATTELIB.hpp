#ifndef CATTELIB_HPP
#define CATTELIB_HPP
#include "CATCRLIB.hpp"
#include "CATTELIB.h"
namespace catalyst {

inline void asciiEncode(const CODEPOINT codepoint, ASCIIW asciiw, RESULT* result) {
    ::catteAsciiEncode(codepoint, asciiw, result);
}

inline void asciiDecode(ASCII ascii, CODEPOINT* codepoint, RESULT* result) {
    ::catteAsciiDecode(ascii, codepoint, result);
}

inline void asciiLength(ASCII ascii, NUINT* length, RESULT* result) {
    ::catteAsciiLength(ascii, length, result);
}

inline void cp1252Encode(const CODEPOINT codepoint, CP1252W cp1252w, RESULT* result) {
    ::catteCp1252Encode(codepoint, cp1252w, result);
}

inline void cp1252Decode(CP1252 cp1252, CODEPOINT* codepoint, RESULT* result) {
    ::catteCp1252Decode(cp1252, codepoint, result);
}

inline void cp1252Length(CP1252 cp1252, NUINT* length, RESULT* result) {
    ::catteCp1252Length(cp1252, length, result);
}

inline void utf8Encode(const CODEPOINT codepoint, UTF8W utf8w, NUINT* produced, RESULT* result) {
    ::catteUtf8Encode(codepoint, utf8w, produced, result);
}

inline void utf8Decode(UTF8 utf8, CODEPOINT* codepoint, NUINT* consumed, RESULT* result) {
    ::catteUtf8Decode(utf8, codepoint, consumed, result);
}

inline void utf8Length(UTF8 utf8, NUINT* length, RESULT* result) {
    ::catteUtf8Length(utf8, length, result);
}

inline void catteUtf16leEncode(const CODEPOINT codepoint, UTF16LEW utf16lew, NUINT* produced, RESULT* result) {
    ::catteUtf16leEncode(codepoint, utf16lew, produced, result);
}

inline void catteUtf16leDecode(UTF16LE utf16le, CODEPOINT* codepoint, NUINT* consumed, RESULT* result) {
    ::catteUtf16leDecode(utf16le, codepoint, consumed, result);
}

inline void catteUtf16leLength(UTF16LE utf16le, NUINT* length, RESULT* result) {
    ::catteUtf16leLength(utf16le, length, result);
}

inline void asciiToCp1252(ASCII ascii, CP1252W cp1252w, RESULT* result) {
    ::catteAsciiToCp1252(ascii, cp1252w, result);
}

inline void asciiToUtf8(ASCII ascii, UTF8W utf8w, NUINT capacity, RESULT* result) {
    ::catteAsciiToUtf8(ascii, utf8w, capacity, result);
}

inline void asciiToUtf16le(ASCII ascii, UTF16LEW utf16lew, NUINT capacity, RESULT* result) {
    ::catteAsciiToUtf16le(ascii, utf16lew, capacity, result);
}

inline void cp1252ToAscii(CP1252 cp1252, ASCIIW ascii, RESULT* result) {
    ::catteCp1252ToAscii(cp1252, ascii, result);
}

inline void cp1252ToUtf8(CP1252 cp1252, UTF8W utf8w, NUINT capacity, RESULT* result) {
    ::catteCp1252ToUtf8(cp1252, utf8w, capacity, result);
}

inline void cp1252ToUtf16le(CP1252 cp1252, UTF16LEW utf16lew, NUINT capacity, RESULT* result) {
    ::catteCp1252ToUtf16le(cp1252, utf16lew, capacity, result);
}

inline void utf8ToAscii(UTF8 utf8, ASCIIW ascii, NUINT capacity, RESULT* result) {
    ::catteUtf8ToAscii(utf8, ascii, capacity, result);
}

inline void utf8ToCp1252(UTF8 utf8, CP1252W cp1252w, NUINT capacity, RESULT* result) {
    ::catteUtf8ToCp1252(utf8, cp1252w, capacity, result);
}

inline void utf8ToUtf16le(UTF8 utf8, UTF16LEW utf16lew, NUINT capacity, RESULT* result) {
    ::catteUtf8ToUtf16le(utf8, utf16lew, capacity, result);
}

inline void utf16leToAscii(UTF16LE utf16le, ASCIIW ascii, NUINT capacity, RESULT* result) {
    ::catteUtf16leToAscii(utf16le, ascii, capacity, result);
}

inline void utf16leToCp1252(UTF16LE utf16le, CP1252W cp1252w, NUINT capacity, RESULT* result) {
    ::catteUtf16leToCp1252(utf16le, cp1252w, capacity, result);
}

inline void utf16leToUtf8(UTF16LE utf16le, UTF8W utf8w, NUINT capacity, RESULT* result) {
    ::catteUtf16leToUtf8(utf16le, utf8w, capacity, result);
}

inline void catteUtf16leToAscii(UTF16LE utf16le, ASCIIW ascii, NUINT capacity, RESULT* result) {
    ::catteUtf16leToAscii(utf16le, ascii, capacity, result);
}

inline void catteUtf16leToCp1252(UTF16LE utf16le, CP1252W cp1252w, NUINT capacity, RESULT* result) {
    ::catteUtf16leToCp1252(utf16le, cp1252w, capacity, result);
}

inline void catteUtf16leToUtf8(UTF16LE utf16le, UTF8W utf8w, NUINT capacity, RESULT* result) {
    ::catteUtf16leToUtf8(utf16le, utf8w, capacity, result);
}

} /* namespace catalyst */
#endif /* CATTELIB_HPP */
