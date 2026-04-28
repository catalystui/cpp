#include "CATCRLIB.hpp"
#include "CATTELIB.h"

using namespace catalyst;

void catteAsciiEncode(const CODEPOINT codepoint, ASCIIW asciiw, RESULT* result) {
    // Validate, initialize parameters
    if (asciiw == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    BYTE codepoint0 = codepoint.byte0;
    BYTE codepoint1 = codepoint.byte1;
    BYTE codepoint2 = codepoint.byte2;
    BYTE codepoint3 = codepoint.byte3;
    if (codepoint0 != 0 || codepoint1 != 0 || codepoint2 != 0 || codepoint3 >= (BYTE) 0x80) {
        // Not a valid ASCII character
        asciiw[0] = (BYTE) '?';
        if (result != 0) *result = RESULT(STATUS_CODE_WARNING_PARTIAL, 0, 0, 0);
        return;
    } else if (codepoint3 == 0) {
        // Handle U+0000 (NUL)
        asciiw[0] = (BYTE) '\0';
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    } else {
        // Maps directly to ASCII
        asciiw[0] = codepoint3;
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }
}

void catteAsciiDecode(ASCII ascii, CODEPOINT* codepoint, RESULT* result) {
    // Validate, initialize parameters
    if (ascii == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (codepoint == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }

    // ASCII maps directly to the lsb of the codepoint
    BYTE codeunit0 = ascii[0];
    codepoint->byte0 = 0;
    codepoint->byte1 = 0;
    codepoint->byte2 = 0;
    codepoint->byte3 = codeunit0;
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
}

void catteAsciiLength(ASCII ascii, NUINT* length, RESULT* result) {
    if (ascii == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (length == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    NUINT count = 0;
    while (ascii[count] != (BYTE) '\0') {
        if (++count == (NUINT) -1) {
            // Overflow, no null terminator found
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }
    }
    *length = count;
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
}

void catteCp1252Encode(const CODEPOINT codepoint, CP1252W cp1252w, RESULT* result) {
    // Validate, initialize parameters
    if (cp1252w == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }

    BYTE codepoint0 = codepoint.byte0;
    BYTE codepoint1 = codepoint.byte1;
    BYTE codepoint2 = codepoint.byte2;
    BYTE codepoint3 = codepoint.byte3;

    if (codepoint0 == 0 && codepoint1 == 0 && codepoint2 == 0) {
        if (codepoint3 <= (BYTE) 0x7F || codepoint3 >= (BYTE) 0xA0) {
            // U+0000 through U+007F and U+00A0 through U+00FF map directly to CP1252
            cp1252w[0] = codepoint3;
            if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
            return;
        }
    }

    if (codepoint0 == 0 && codepoint1 == 0) {
        switch (codepoint2) {
            case (BYTE) 0x01:
                switch (codepoint3) {
                    case (BYTE) 0x52: cp1252w[0] = (BYTE) 0x8C; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+0152
                    case (BYTE) 0x53: cp1252w[0] = (BYTE) 0x9C; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+0153
                    case (BYTE) 0x60: cp1252w[0] = (BYTE) 0x8A; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+0160
                    case (BYTE) 0x61: cp1252w[0] = (BYTE) 0x9A; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+0161
                    case (BYTE) 0x78: cp1252w[0] = (BYTE) 0x9F; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+0178
                    case (BYTE) 0x7D: cp1252w[0] = (BYTE) 0x8E; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+017D
                    case (BYTE) 0x7E: cp1252w[0] = (BYTE) 0x9E; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+017E
                    case (BYTE) 0x92: cp1252w[0] = (BYTE) 0x83; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+0192
                }
                break;

            case (BYTE) 0x02:
                switch (codepoint3) {
                    case (BYTE) 0xC6: cp1252w[0] = (BYTE) 0x88; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+02C6
                    case (BYTE) 0xDC: cp1252w[0] = (BYTE) 0x98; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+02DC
                }
                break;

            case (BYTE) 0x20:
                switch (codepoint3) {
                    case (BYTE) 0x13: cp1252w[0] = (BYTE) 0x96; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2013
                    case (BYTE) 0x14: cp1252w[0] = (BYTE) 0x97; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2014
                    case (BYTE) 0x18: cp1252w[0] = (BYTE) 0x91; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2018
                    case (BYTE) 0x19: cp1252w[0] = (BYTE) 0x92; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2019
                    case (BYTE) 0x1A: cp1252w[0] = (BYTE) 0x82; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+201A
                    case (BYTE) 0x1C: cp1252w[0] = (BYTE) 0x93; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+201C
                    case (BYTE) 0x1D: cp1252w[0] = (BYTE) 0x94; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+201D
                    case (BYTE) 0x1E: cp1252w[0] = (BYTE) 0x84; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+201E
                    case (BYTE) 0x20: cp1252w[0] = (BYTE) 0x86; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2020
                    case (BYTE) 0x21: cp1252w[0] = (BYTE) 0x87; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2021
                    case (BYTE) 0x22: cp1252w[0] = (BYTE) 0x95; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2022
                    case (BYTE) 0x26: cp1252w[0] = (BYTE) 0x85; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2026
                    case (BYTE) 0x30: cp1252w[0] = (BYTE) 0x89; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2030
                    case (BYTE) 0x39: cp1252w[0] = (BYTE) 0x8B; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2039
                    case (BYTE) 0x3A: cp1252w[0] = (BYTE) 0x9B; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+203A
                    case (BYTE) 0xAC: cp1252w[0] = (BYTE) 0x80; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+20AC
                }
                break;

            case (BYTE) 0x21:
                switch (codepoint3) {
                    case (BYTE) 0x22: cp1252w[0] = (BYTE) 0x99; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2122
                }
                break;
        }
    }

    // Not a valid CP1252 character
    cp1252w[0] = (BYTE) '?';
    if (result != 0) *result = RESULT(STATUS_CODE_WARNING_PARTIAL, 0, 0, 0);
    return;
}

void catteCp1252Decode(CP1252 cp1252, CODEPOINT* codepoint, RESULT* result) {
    // Validate, initialize parameters
    if (cp1252 == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (codepoint == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }

    BYTE codeunit0 = cp1252[0];
    codepoint->byte0 = 0;
    codepoint->byte1 = 0;
    codepoint->byte2 = 0;
    codepoint->byte3 = codeunit0;

    if (codeunit0 <= (BYTE) 0x7F || codeunit0 >= (BYTE) 0xA0) {
        // U+0000 through U+007F and U+00A0 through U+00FF map directly from CP1252
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }

    switch (codeunit0) {
        case (BYTE) 0x80: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0xAC; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+20AC
        case (BYTE) 0x82: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0x1A; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+201A
        case (BYTE) 0x83: codepoint->byte2 = (BYTE) 0x01; codepoint->byte3 = (BYTE) 0x92; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+0192
        case (BYTE) 0x84: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0x1E; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+201E
        case (BYTE) 0x85: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0x26; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2026
        case (BYTE) 0x86: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0x20; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2020
        case (BYTE) 0x87: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0x21; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2021
        case (BYTE) 0x88: codepoint->byte2 = (BYTE) 0x02; codepoint->byte3 = (BYTE) 0xC6; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+02C6
        case (BYTE) 0x89: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0x30; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2030
        case (BYTE) 0x8A: codepoint->byte2 = (BYTE) 0x01; codepoint->byte3 = (BYTE) 0x60; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+0160
        case (BYTE) 0x8B: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0x39; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2039
        case (BYTE) 0x8C: codepoint->byte2 = (BYTE) 0x01; codepoint->byte3 = (BYTE) 0x52; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+0152
        case (BYTE) 0x8E: codepoint->byte2 = (BYTE) 0x01; codepoint->byte3 = (BYTE) 0x7D; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+017D
        case (BYTE) 0x91: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0x18; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2018
        case (BYTE) 0x92: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0x19; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2019
        case (BYTE) 0x93: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0x1C; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+201C
        case (BYTE) 0x94: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0x1D; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+201D
        case (BYTE) 0x95: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0x22; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2022
        case (BYTE) 0x96: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0x13; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2013
        case (BYTE) 0x97: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0x14; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2014
        case (BYTE) 0x98: codepoint->byte2 = (BYTE) 0x02; codepoint->byte3 = (BYTE) 0xDC; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+02DC
        case (BYTE) 0x99: codepoint->byte2 = (BYTE) 0x21; codepoint->byte3 = (BYTE) 0x22; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+2122
        case (BYTE) 0x9A: codepoint->byte2 = (BYTE) 0x01; codepoint->byte3 = (BYTE) 0x61; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+0161
        case (BYTE) 0x9B: codepoint->byte2 = (BYTE) 0x20; codepoint->byte3 = (BYTE) 0x3A; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+203A
        case (BYTE) 0x9C: codepoint->byte2 = (BYTE) 0x01; codepoint->byte3 = (BYTE) 0x53; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+0153
        case (BYTE) 0x9E: codepoint->byte2 = (BYTE) 0x01; codepoint->byte3 = (BYTE) 0x7E; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+017E
        case (BYTE) 0x9F: codepoint->byte2 = (BYTE) 0x01; codepoint->byte3 = (BYTE) 0x78; if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0); return; // U+0178
    }

    // Undefined CP1252 code unit
    codepoint->byte0 = 0;
    codepoint->byte1 = 0;
    codepoint->byte2 = 0;
    codepoint->byte3 = (BYTE) '?';
    if (result != 0) *result = RESULT(STATUS_CODE_WARNING_PARTIAL, 0, 0, 0);
    return;
}

void catteCp1252Length(CP1252 cp1252, NUINT* length, RESULT* result) {
    if (cp1252 == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (length == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }

    NUINT count = 0;
    while (cp1252[count] != (BYTE) '\0') {
        if (++count == (NUINT) -1) {
            // Overflow, no null terminator found
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }
    }

    *length = count;
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
}

// TODO: Document opcode 1 for overlong encoding
// TODO: Document opcode 2 for invalid surrogate codepoint
// TODO: Document opcode 3 for invalid codepoint
void catteUtf8Encode(const CODEPOINT codepoint, UTF8W utf8w, NUINT* produced, RESULT* result) {
    // Validate, initialize parameters
    if (utf8w == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (produced == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    *produced = 0;

    // Prepare code units
    BYTE codepoint0 = codepoint.byte0;
    BYTE codepoint1 = codepoint.byte1;
    BYTE codepoint2 = codepoint.byte2;
    BYTE codepoint3 = codepoint.byte3;

    // 1-byte ASCII
    if (codepoint0 == 0 && codepoint1 == 0 && codepoint2 == 0 && codepoint3 < (BYTE) 0x80u) {
        utf8w[0] = codepoint3;
        *produced = 1;
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }

    // 2-byte sequence
    if (codepoint0 == 0 && codepoint1 == 0 && codepoint2 <= (BYTE) 0x07u) {
        if (codepoint2 == 0 && codepoint3 < (BYTE) 0x80u) {
            // Overlong encoding
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 1, 0);
            return;
        }
        BYTE t0 = (BYTE) (((codepoint2 & (BYTE) 0x07u) << 2) | (codepoint3 >> 6));
        BYTE t1 = (BYTE) (codepoint3 & (BYTE) 0x3Fu);
        utf8w[0] = (BYTE) ((BYTE) 0xC0u | t0);
        utf8w[1] = (BYTE) ((BYTE) 0x80u | t1);
        *produced = 2;
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }

    // 3-byte sequence
    if (codepoint0 == 0 && codepoint1 == 0) {
        if (codepoint2 < (BYTE) 0x08u) {
            // Overlong encoding
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 1, 0);
            return;
        }
        if (codepoint2 >= (BYTE) 0xD8u && codepoint2 <= (BYTE) 0xDFu) {
            // Invalid surrogate codepoint
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 2, 0);
            return;
        }
        BYTE t0 = (BYTE) (codepoint2 >> 4);
        BYTE t1 = (BYTE) (((codepoint2 & (BYTE) 0x0Fu) << 2) | (codepoint3 >> 6));
        BYTE t2 = (BYTE) (codepoint3 & (BYTE) 0x3Fu);
        utf8w[0] = (BYTE) ((BYTE) 0xE0u | t0);
        utf8w[1] = (BYTE) ((BYTE) 0x80u | t1);
        utf8w[2] = (BYTE) ((BYTE) 0x80u | t2);
        *produced = 3;
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }

    // 4-byte sequence
    if (codepoint0 == 0 && codepoint1 != 0 && codepoint1 <= (BYTE) 0x10u) {
        BYTE t0 = (BYTE) ((codepoint1 & (BYTE) 0x1Cu) >> 2);
        BYTE t1 = (BYTE) (((codepoint1 & (BYTE) 0x03u) << 4) | (codepoint2 >> 4));
        BYTE t2 = (BYTE) (((codepoint2 & (BYTE) 0x0Fu) << 2) | (codepoint3 >> 6));
        BYTE t3 = (BYTE) (codepoint3 & (BYTE) 0x3Fu);
        utf8w[0] = (BYTE) ((BYTE) 0xF0u | t0);
        utf8w[1] = (BYTE) ((BYTE) 0x80u | t1);
        utf8w[2] = (BYTE) ((BYTE) 0x80u | t2);
        utf8w[3] = (BYTE) ((BYTE) 0x80u | t3);
        *produced = 4;
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }

    // Invalid codepoint
    if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 3, 0);
    return;
}

// TODO: Document opcode 1 for unexpected end of input
// TODO: Document opcode 2 for invalid continuation byte
// TODO: Document opcode 3 for overlong encoding
// TODO: Document opcode 4 for invalid surrogate codepoint
// TODO: Document opcode 5 for invalid codepoint
// TODO: Document opcode 6 for invalid leading code unit
void catteUtf8Decode(UTF8 utf8, CODEPOINT* codepoint, NUINT* consumed, RESULT* result) {
    // Validate, initialize parameters
    if (utf8 == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (codepoint == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (consumed == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 2);
        return;
    }
    *consumed = 0;

    // Zero out the codepoint
    BYTE codeunit0 = utf8[0];
    codepoint->byte0 = 0;
    codepoint->byte1 = 0;
    codepoint->byte2 = 0;
    codepoint->byte3 = 0;

    // NUL '\0' character
    if (codeunit0 == (BYTE) '\0') {
        *consumed = 1;
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }

    // 1-byte ASCII
    if (codeunit0 < (BYTE) 0x80) {
        codepoint->byte3 = codeunit0;
        *consumed = 1;
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }

    // 2-byte sequence
    if (codeunit0 >= (BYTE) 0xC2u && codeunit0 <= (BYTE) 0xDFu) {
        BYTE codeunit1 = utf8[1];
        if (codeunit1 == 0) {
            // Unexpected end of input
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 1, 0);
            return;
        }
        if (codeunit1 < (BYTE) 0x80u || codeunit1 > (BYTE) 0xBFu) {
            // Invalid continuation byte
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 2, 0);
            return;
        }
        BYTE t0 = (BYTE) (codeunit0 & (BYTE) 0x1Fu);
        BYTE t1 = (BYTE) (codeunit1 & (BYTE) 0x3Fu);
        codepoint->byte2 = (BYTE) (t0 >> 2);
        codepoint->byte3 = (BYTE) (((t0 & (BYTE) 0x03u) << 6) | t1);
        *consumed = 2;
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }

    // 3-byte sequence
    if (codeunit0 >= (BYTE) 0xE0u && codeunit0 <= (BYTE) 0xEFu) {
        BYTE codeunit1 = utf8[1];
        BYTE codeunit2 = utf8[2];
        if (codeunit1 == 0 || codeunit2 == 0) {
            // Unexpected end of input
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 1, 0);
            return;
        }
        if (codeunit1 < (BYTE) 0x80u || codeunit1 > (BYTE) 0xBFu) {
            // Invalid continuation byte
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 2, 0);
            return;
        }
        if (codeunit2 < (BYTE) 0x80u || codeunit2 > (BYTE) 0xBFu) {
            // Invalid continuation byte
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 2, 0);
            return;
        }
        if (codeunit0 == (BYTE) 0xE0u && codeunit1 < (BYTE) 0xA0u) {
            // Overlong encoding
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 3, 0);
            return;
        }
        if (codeunit0 == (BYTE) 0xEDu && codeunit1 >= (BYTE) 0xA0u) {
            // Invalid surrogate codepoint
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 4, 0);
            return;
        }
        BYTE t0 = (BYTE) (codeunit0 & 0x0Fu);
        BYTE t1 = (BYTE) (codeunit1 & 0x3Fu);
        BYTE t2 = (BYTE) (codeunit2 & 0x3Fu);
        codepoint->byte2 = (BYTE) ((t0 << 4) | (t1 >> 2));
        codepoint->byte3 = (BYTE) (((t1 & 0x03u) << 6) | t2);
        *consumed = 3;
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }

    // 4-byte sequence
    if (codeunit0 >= (BYTE) 0xF0u && codeunit0 <= (BYTE) 0xF4u) {
        BYTE codeunit1 = utf8[1];
        BYTE codeunit2 = utf8[2];
        BYTE codeunit3 = utf8[3];
        if (codeunit1 == 0 || codeunit2 == 0 || codeunit3 == 0) {
            // Unexpected end of input
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 1, 0);
            return;
        }
        if (codeunit1 < 0x80u || codeunit1 > 0xBFu) {
            // Invalid continuation byte
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 2, 0);
            return;
        }
        if (codeunit2 < 0x80u || codeunit2 > 0xBFu) {
            // Invalid continuation byte
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 2, 0);
            return;
        }
        if (codeunit3 < 0x80u || codeunit3 > 0xBFu) {
            // Invalid continuation byte
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 2, 0);
            return;
        }
        if (codeunit0 == 0xF0u && codeunit1 < 0x90u) {
            // Overlong encoding
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 3, 0);
            return;
        }
        if (codeunit0 == 0xF4u && codeunit1 > 0x8Fu) {
            // Codepoints above U+10FFFF
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 5, 0);
            return;
        }
        BYTE t0 = (BYTE) (codeunit0 & (BYTE) 0x07u);
        BYTE t1 = (BYTE) (codeunit1 & (BYTE) 0x3Fu);
        BYTE t2 = (BYTE) (codeunit2 & (BYTE) 0x3Fu);
        BYTE t3 = (BYTE) (codeunit3 & (BYTE) 0x3Fu);
        codepoint->byte0 = 0;
        codepoint->byte1 = (BYTE) ((t0 << 2) | (t1 >> 4));
        codepoint->byte2 = (BYTE) (((t1 & (BYTE) 0x0Fu) << 4) | (t2 >> 2));
        codepoint->byte3 = (BYTE) (((t2 & (BYTE) 0x03u) << 6) | t3);
        *consumed = 4;
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        return; // Valid 4-byte sequence
    }

    // Invalid leading code unit
    if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 6, 0);
    return;
}

// TODO: Document opcode 1 for invalid UTF-8 sequence
void catteUtf8Length(UTF8 utf8, NUINT* length, RESULT* result) {
    // Validate, initialize parameters
    if (utf8 == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (length == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    *length = 0;

    // Skip BOM if present
    if (utf8[0] == (BYTE) 0xEFu && utf8[1] == (BYTE) 0xBBu && utf8[2] == (BYTE) 0xBFu) {
        utf8 += 3;
    }

    NUINT count = 0;
    while (*utf8) {
        CODEPOINT codepoint;
        NUINT consumed;
        RESULT decodeResult;
        catteUtf8Decode(utf8, &codepoint, &consumed, &decodeResult);
        if (!statusCodeIsSuccess(decodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 1, 0);
            return;
        }
        if (++count == (NUINT) -1) {
            // Overflow, no null terminator found
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }
        utf8 += consumed;
    }
    *length = count;
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
}

// TODO: Document opcode 1 for invalid codepoint above U+10FFFF
// TODO: Document opcode 2 for invalid surrogate codepoint
void catteUtf16leEncode(const CODEPOINT codepoint, UTF16LEW utf16lew, NUINT* produced, RESULT* result) {
    // Validate, initialize parameters
    if (utf16lew == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (produced == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    *produced = 0;

    // Prepare code units
    BYTE codepoint0 = codepoint.byte0;
    BYTE codepoint1 = codepoint.byte1;
    BYTE codepoint2 = codepoint.byte2;
    BYTE codepoint3 = codepoint.byte3;

    // Validate codepoint is within Unicode range U+0000 to U+10FFFF
    if (codepoint0 != 0 || codepoint1 > (BYTE) 0x10u) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 1, 0); // Invalid codepoint above U+10FFFF
        return;
    }

    // Single UTF-16 code unit
    if (codepoint1 == 0) {
        if (codepoint2 >= (BYTE) 0xD8u && codepoint2 <= (BYTE) 0xDFu) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 2, 0); // Invalid surrogate codepoint
            return;
        }
        utf16lew[0].msb = codepoint2;
        utf16lew[0].lsb = codepoint3;
        *produced = 1;
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }

    // Surrogate pair
    BYTE x2 = (BYTE) (codepoint1 - (BYTE) 0x01u);
    BYTE x1 = codepoint2;
    BYTE x0 = codepoint3;

    // High surrogate: 0xD800 + ((codepoint - 0x10000) >> 10)
    BYTE hs_hi2 = (BYTE) (x2 >> 2);
    BYTE hs_lo8 = (BYTE) (((x2 & (BYTE) 0x03u) << 6) | (x1 >> 2));

    // Low surrogate: 0xDC00 + ((codepoint - 0x10000) & 0x03FF)
    BYTE ls_hi2 = (BYTE) (x1 & (BYTE) 0x03u);
    BYTE ls_lo8 = x0;

    utf16lew[0].msb = (BYTE) ((BYTE) 0xD8u | hs_hi2);
    utf16lew[0].lsb = hs_lo8;
    utf16lew[1].msb = (BYTE) ((BYTE) 0xDCu | ls_hi2);
    utf16lew[1].lsb = ls_lo8;
    *produced = 2;
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
    return;
}

// TODO: Document opcode 1 for low surrogate without preceding high surrogate
// TODO: Document opcode 2 for invalid low surrogate
void catteUtf16leDecode(UTF16LE utf16le, CODEPOINT* codepoint, NUINT* consumed, RESULT* result) {
    // Validate, initialize parameters
    if (utf16le == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (codepoint == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (consumed == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 2);
        return;
    }
    *consumed = 0;

    // Zero out the codepoint
    BYTE codeunit0 = utf16le[0].msb;
    BYTE codeunit1 = utf16le[0].lsb;
    codepoint->byte0 = 0;
    codepoint->byte1 = 0;
    codepoint->byte2 = 0;
    codepoint->byte3 = 0;

    // NUL '\0' character
    if (codeunit0 == 0 && codeunit1 == 0) {
        *consumed = 1;
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }

    // Low surrogate without preceding high surrogate
    if (codeunit0 >= (BYTE) 0xDCu && codeunit0 <= (BYTE) 0xDFu) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 1, 0);
        return;
    }

    // Not a surrogate pair, single code unit
    if (codeunit0 < (BYTE) 0xD8u || codeunit0 > (BYTE) 0xDBu) {
        codepoint->byte2 = codeunit0;
        codepoint->byte3 = codeunit1;
        *consumed = 1;
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
        return;
    }

    // Surrogate pair, must be followed by a low surrogate
    BYTE codeunit2 = utf16le[1].msb;
    BYTE codeunit3 = utf16le[1].lsb;

    if (!(codeunit2 >= (BYTE) 0xDCu && codeunit2 <= (BYTE) 0xDFu)) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 2, 0);
        return;
    }

    // Extract the 20-bit codepoint from the surrogate pair
    BYTE hs_hi = (BYTE) (codeunit0 & (BYTE) 0x03u);
    BYTE hs_lo = codeunit1;
    BYTE ls_hi = (BYTE) (codeunit2 & (BYTE) 0x03u);
    BYTE ls_lo = codeunit3;

    BYTE p0 = ls_lo;
    BYTE p1 = (BYTE) (((hs_lo & (BYTE) 0x3Fu) << 2) | ls_hi);
    BYTE p2 = (BYTE) (((hs_hi & (BYTE) 0x03u) << 2) | (hs_lo >> 6));

    codepoint->byte0 = 0;
    codepoint->byte1 = (BYTE) (p2 + (BYTE) 0x01u);
    codepoint->byte2 = p1;
    codepoint->byte3 = p0;
    *consumed = 2;
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
    return;
}

// TODO: Document opcode 1 for invalid UTF-16LE sequence
void catteUtf16leLength(UTF16LE utf16le, NUINT* length, RESULT* result) {
    // Validate, initialize parameters
    if (utf16le == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (length == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    *length = 0;

    // Skip BOM if present
    if (utf16le[0].msb == (BYTE) 0xFEu && utf16le[0].lsb == (BYTE) 0xFFu) {
        utf16le += 1;
    }

    NUINT count = 0;
    while (utf16le[0].msb != 0 || utf16le[0].lsb != 0) {
        CODEPOINT codepoint;
        NUINT consumed;
        RESULT decodeResult;
        catteUtf16leDecode(utf16le, &codepoint, &consumed, &decodeResult);
        if (!statusCodeIsSuccess(decodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR, 0, 1, 0); // Invalid UTF-16LE sequence
            return;
        }
        if (++count == (NUINT) -1) {
            // Overflow, no null terminator found
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }
        utf16le += consumed;
    }
    *length = count;
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
}

void catteAsciiToCp1252(ASCII ascii, CP1252W cp1252w, RESULT* result) {
    // Validate, initialize parameters
    if (ascii == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (cp1252w == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }

    STATUS_CODE status = STATUS_CODE_SUCCESS;
    NUINT index = 0;

    while (ascii[index] != (BYTE) '\0') {
        if (ascii[index] < (BYTE) 0x80u) {
            cp1252w[index] = ascii[index];
        } else {
            cp1252w[index] = (BYTE) '?';
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        if (++index == (NUINT) -1) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }
    }

    cp1252w[index] = (BYTE) '\0';
    if (result != 0) *result = RESULT(status, 0, 0, 0);
}

void catteAsciiToUtf8(ASCII ascii, UTF8W utf8w, NUINT capacity, RESULT* result) {
    // Validate, initialize parameters
    if (ascii == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (utf8w == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (capacity == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
        return;
    }

    STATUS_CODE status = STATUS_CODE_SUCCESS;
    NUINT read = 0;
    NUINT write = 0;

    while (ascii[read] != (BYTE) '\0') {
        if (write >= capacity || (capacity - write) <= 1) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }

        if (ascii[read] < (BYTE) 0x80u) {
            utf8w[write] = ascii[read];
        } else {
            utf8w[write] = (BYTE) '?';
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        ++read;
        ++write;
    }

    utf8w[write] = (BYTE) '\0';
    if (result != 0) *result = RESULT(status, 0, 0, 0);
}

void catteAsciiToUtf16le(ASCII ascii, UTF16LEW utf16lew, NUINT capacity, RESULT* result) {
    // Validate, initialize parameters
    if (ascii == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (utf16lew == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (capacity == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
        return;
    }

    STATUS_CODE status = STATUS_CODE_SUCCESS;
    NUINT read = 0;
    NUINT write = 0;

    while (ascii[read] != (BYTE) '\0') {
        if (write >= capacity || (capacity - write) <= 1) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }

        utf16lew[write].msb = 0;

        if (ascii[read] < (BYTE) 0x80u) {
            utf16lew[write].lsb = ascii[read];
        } else {
            utf16lew[write].lsb = (BYTE) '?';
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        ++read;
        ++write;
    }

    utf16lew[write].msb = 0;
    utf16lew[write].lsb = 0;
    if (result != 0) *result = RESULT(status, 0, 0, 0);
}

// TODO: Document opcode 1 for failed CP1252 decode
// TODO: Document opcode 2 for failed ASCII encode
void catteCp1252ToAscii(CP1252 cp1252, ASCIIW asciiw, RESULT* result) {
    // Validate, initialize parameters
    if (cp1252 == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (asciiw == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }

    STATUS_CODE status = STATUS_CODE_SUCCESS;
    NUINT index = 0;

    while (cp1252[index] != (BYTE) '\0') {
        CODEPOINT codepoint;
        BYTE output[1];
        RESULT decodeResult;
        RESULT encodeResult;

        catteCp1252Decode(cp1252 + index, &codepoint, &decodeResult);
        if (statusCodeIsError(decodeResult.status) || statusCodeIsFatal(decodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, (BYTE) decodeResult.status);
            return;
        }
        if (statusCodeIsWarning(decodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        catteAsciiEncode(codepoint, output, &encodeResult);
        if (statusCodeIsError(encodeResult.status) || statusCodeIsFatal(encodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, (BYTE) encodeResult.status);
            return;
        }
        if (statusCodeIsWarning(encodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        asciiw[index] = output[0];

        if (++index == (NUINT) -1) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }
    }

    asciiw[index] = (BYTE) '\0';
    if (result != 0) *result = RESULT(status, 0, 0, 0);
}

// TODO: Document opcode 1 for failed CP1252 decode
// TODO: Document opcode 2 for failed UTF-8 encode
void catteCp1252ToUtf8(CP1252 cp1252, UTF8W utf8w, NUINT capacity, RESULT* result) {
    // Validate, initialize parameters
    if (cp1252 == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (utf8w == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (capacity == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
        return;
    }

    STATUS_CODE status = STATUS_CODE_SUCCESS;
    NUINT read = 0;
    NUINT write = 0;

    while (cp1252[read] != (BYTE) '\0') {
        CODEPOINT codepoint;
        BYTE output[4];
        NUINT produced;
        RESULT decodeResult;
        RESULT encodeResult;
        NUINT i;

        output[0] = 0;
        output[1] = 0;
        output[2] = 0;
        output[3] = 0;

        catteCp1252Decode(cp1252 + read, &codepoint, &decodeResult);
        if (statusCodeIsError(decodeResult.status) || statusCodeIsFatal(decodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, (BYTE) decodeResult.status);
            return;
        }
        if (statusCodeIsWarning(decodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        catteUtf8Encode(codepoint, output, &produced, &encodeResult);
        if (statusCodeIsError(encodeResult.status) || statusCodeIsFatal(encodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, (BYTE) encodeResult.status);
            return;
        }
        if (statusCodeIsWarning(encodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        if (write >= capacity || produced >= (capacity - write)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }

        for (i = 0; i < produced; ++i) {
            utf8w[write + i] = output[i];
        }

        ++read;
        write += produced;
    }

    utf8w[write] = (BYTE) '\0';
    if (result != 0) *result = RESULT(status, 0, 0, 0);
}

// TODO: Document opcode 1 for failed CP1252 decode
// TODO: Document opcode 2 for failed UTF-16LE encode
void catteCp1252ToUtf16le(CP1252 cp1252, UTF16LEW utf16lew, NUINT capacity, RESULT* result) {
    // Validate, initialize parameters
    if (cp1252 == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (utf16lew == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (capacity == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
        return;
    }

    STATUS_CODE status = STATUS_CODE_SUCCESS;
    NUINT read = 0;
    NUINT write = 0;

    while (cp1252[read] != (BYTE) '\0') {
        CODEPOINT codepoint;
        UTF16LE_CODE_UNIT output[2];
        NUINT produced;
        RESULT decodeResult;
        RESULT encodeResult;
        NUINT i;

        output[0].msb = 0;
        output[0].lsb = 0;
        output[1].msb = 0;
        output[1].lsb = 0;

        catteCp1252Decode(cp1252 + read, &codepoint, &decodeResult);
        if (statusCodeIsError(decodeResult.status) || statusCodeIsFatal(decodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, (BYTE) decodeResult.status);
            return;
        }
        if (statusCodeIsWarning(decodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        catteUtf16leEncode(codepoint, output, &produced, &encodeResult);
        if (statusCodeIsError(encodeResult.status) || statusCodeIsFatal(encodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, (BYTE) encodeResult.status);
            return;
        }
        if (statusCodeIsWarning(encodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        if (write >= capacity || produced >= (capacity - write)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }

        for (i = 0; i < produced; ++i) {
            utf16lew[write + i] = output[i];
        }

        ++read;
        write += produced;
    }

    utf16lew[write].msb = 0;
    utf16lew[write].lsb = 0;
    if (result != 0) *result = RESULT(status, 0, 0, 0);
}

// TODO: Document opcode 1 for failed UTF-8 decode
// TODO: Document opcode 2 for failed ASCII encode
void catteUtf8ToAscii(UTF8 utf8, ASCIIW asciiw, NUINT capacity, RESULT* result) {
    // Validate, initialize parameters
    if (utf8 == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (asciiw == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (capacity == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
        return;
    }

    // Skip BOM if present
    if (utf8[0] == (BYTE) 0xEFu && utf8[1] == (BYTE) 0xBBu && utf8[2] == (BYTE) 0xBFu) {
        utf8 += 3;
    }

    STATUS_CODE status = STATUS_CODE_SUCCESS;
    NUINT write = 0;

    while (utf8[0] != (BYTE) '\0') {
        CODEPOINT codepoint;
        BYTE output[1];
        NUINT consumed;
        RESULT decodeResult;
        RESULT encodeResult;

        catteUtf8Decode(utf8, &codepoint, &consumed, &decodeResult);
        if (statusCodeIsError(decodeResult.status) || statusCodeIsFatal(decodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, (BYTE) decodeResult.status);
            return;
        }
        if (statusCodeIsWarning(decodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        catteAsciiEncode(codepoint, output, &encodeResult);
        if (statusCodeIsError(encodeResult.status) || statusCodeIsFatal(encodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, (BYTE) encodeResult.status);
            return;
        }
        if (statusCodeIsWarning(encodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        if (write >= capacity || (capacity - write) <= 1) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }

        asciiw[write] = output[0];
        utf8 += consumed;
        ++write;
    }

    asciiw[write] = (BYTE) '\0';
    if (result != 0) *result = RESULT(status, 0, 0, 0);
}

// TODO: Document opcode 1 for failed UTF-8 decode
// TODO: Document opcode 2 for failed CP1252 encode
void catteUtf8ToCp1252(UTF8 utf8, CP1252W cp1252w, NUINT capacity, RESULT* result) {
    // Validate, initialize parameters
    if (utf8 == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (cp1252w == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (capacity == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
        return;
    }

    // Skip BOM if present
    if (utf8[0] == (BYTE) 0xEFu && utf8[1] == (BYTE) 0xBBu && utf8[2] == (BYTE) 0xBFu) {
        utf8 += 3;
    }

    STATUS_CODE status = STATUS_CODE_SUCCESS;
    NUINT write = 0;

    while (utf8[0] != (BYTE) '\0') {
        CODEPOINT codepoint;
        BYTE output[1];
        NUINT consumed;
        RESULT decodeResult;
        RESULT encodeResult;

        catteUtf8Decode(utf8, &codepoint, &consumed, &decodeResult);
        if (statusCodeIsError(decodeResult.status) || statusCodeIsFatal(decodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, (BYTE) decodeResult.status);
            return;
        }
        if (statusCodeIsWarning(decodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        catteCp1252Encode(codepoint, output, &encodeResult);
        if (statusCodeIsError(encodeResult.status) || statusCodeIsFatal(encodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, (BYTE) encodeResult.status);
            return;
        }
        if (statusCodeIsWarning(encodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        if (write >= capacity || (capacity - write) <= 1) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }

        cp1252w[write] = output[0];
        utf8 += consumed;
        ++write;
    }

    cp1252w[write] = (BYTE) '\0';
    if (result != 0) *result = RESULT(status, 0, 0, 0);
}

// TODO: Document opcode 1 for failed UTF-8 decode
// TODO: Document opcode 2 for failed UTF-16LE encode
void catteUtf8ToUtf16le(UTF8 utf8, UTF16LEW utf16lew, NUINT capacity, RESULT* result) {
    // Validate, initialize parameters
    if (utf8 == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (utf16lew == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (capacity == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
        return;
    }

    // Skip BOM if present
    if (utf8[0] == (BYTE) 0xEFu && utf8[1] == (BYTE) 0xBBu && utf8[2] == (BYTE) 0xBFu) {
        utf8 += 3;
    }

    STATUS_CODE status = STATUS_CODE_SUCCESS;
    NUINT write = 0;

    while (utf8[0] != (BYTE) '\0') {
        CODEPOINT codepoint;
        UTF16LE_CODE_UNIT output[2];
        NUINT consumed;
        NUINT produced;
        RESULT decodeResult;
        RESULT encodeResult;
        NUINT i;

        output[0].msb = 0;
        output[0].lsb = 0;
        output[1].msb = 0;
        output[1].lsb = 0;

        catteUtf8Decode(utf8, &codepoint, &consumed, &decodeResult);
        if (statusCodeIsError(decodeResult.status) || statusCodeIsFatal(decodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, (BYTE) decodeResult.status);
            return;
        }
        if (statusCodeIsWarning(decodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        catteUtf16leEncode(codepoint, output, &produced, &encodeResult);
        if (statusCodeIsError(encodeResult.status) || statusCodeIsFatal(encodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, (BYTE) encodeResult.status);
            return;
        }
        if (statusCodeIsWarning(encodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        if (write >= capacity || produced >= (capacity - write)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }

        for (i = 0; i < produced; ++i) {
            utf16lew[write + i] = output[i];
        }

        utf8 += consumed;
        write += produced;
    }

    utf16lew[write].msb = 0;
    utf16lew[write].lsb = 0;
    if (result != 0) *result = RESULT(status, 0, 0, 0);
}

// TODO: Document opcode 1 for failed UTF-16LE decode
// TODO: Document opcode 2 for failed ASCII encode
void catteUtf16leToAscii(UTF16LE utf16le, ASCIIW asciiw, NUINT capacity, RESULT* result) {
    // Validate, initialize parameters
    if (utf16le == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (asciiw == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (capacity == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
        return;
    }

    // Skip BOM if present
    if (utf16le[0].msb == (BYTE) 0xFEu && utf16le[0].lsb == (BYTE) 0xFFu) {
        utf16le += 1;
    }

    STATUS_CODE status = STATUS_CODE_SUCCESS;
    NUINT write = 0;

    while (utf16le[0].msb != 0 || utf16le[0].lsb != 0) {
        CODEPOINT codepoint;
        BYTE output[1];
        NUINT consumed;
        RESULT decodeResult;
        RESULT encodeResult;

        catteUtf16leDecode(utf16le, &codepoint, &consumed, &decodeResult);
        if (statusCodeIsError(decodeResult.status) || statusCodeIsFatal(decodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, (BYTE) decodeResult.status);
            return;
        }
        if (statusCodeIsWarning(decodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        catteAsciiEncode(codepoint, output, &encodeResult);
        if (statusCodeIsError(encodeResult.status) || statusCodeIsFatal(encodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, (BYTE) encodeResult.status);
            return;
        }
        if (statusCodeIsWarning(encodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        if (write >= capacity || (capacity - write) <= 1) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }

        asciiw[write] = output[0];
        utf16le += consumed;
        ++write;
    }

    asciiw[write] = (BYTE) '\0';
    if (result != 0) *result = RESULT(status, 0, 0, 0);
}

// TODO: Document opcode 1 for failed UTF-16LE decode
// TODO: Document opcode 2 for failed CP1252 encode
void catteUtf16leToCp1252(UTF16LE utf16le, CP1252W cp1252w, NUINT capacity, RESULT* result) {
    // Validate, initialize parameters
    if (utf16le == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (cp1252w == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (capacity == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
        return;
    }

    // Skip BOM if present
    if (utf16le[0].msb == (BYTE) 0xFEu && utf16le[0].lsb == (BYTE) 0xFFu) {
        utf16le += 1;
    }

    STATUS_CODE status = STATUS_CODE_SUCCESS;
    NUINT write = 0;

    while (utf16le[0].msb != 0 || utf16le[0].lsb != 0) {
        CODEPOINT codepoint;
        BYTE output[1];
        NUINT consumed;
        RESULT decodeResult;
        RESULT encodeResult;

        catteUtf16leDecode(utf16le, &codepoint, &consumed, &decodeResult);
        if (statusCodeIsError(decodeResult.status) || statusCodeIsFatal(decodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, (BYTE) decodeResult.status);
            return;
        }
        if (statusCodeIsWarning(decodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        catteCp1252Encode(codepoint, output, &encodeResult);
        if (statusCodeIsError(encodeResult.status) || statusCodeIsFatal(encodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, (BYTE) encodeResult.status);
            return;
        }
        if (statusCodeIsWarning(encodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        if (write >= capacity || (capacity - write) <= 1) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }

        cp1252w[write] = output[0];
        utf16le += consumed;
        ++write;
    }

    cp1252w[write] = (BYTE) '\0';
    if (result != 0) *result = RESULT(status, 0, 0, 0);
}

// TODO: Document opcode 1 for failed UTF-16LE decode
// TODO: Document opcode 2 for failed UTF-8 encode
void catteUtf16leToUtf8(UTF16LE utf16le, UTF8W utf8w, NUINT capacity, RESULT* result) {
    // Validate, initialize parameters
    if (utf16le == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (utf8w == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }
    if (capacity == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
        return;
    }

    // Skip BOM if present
    if (utf16le[0].msb == (BYTE) 0xFEu && utf16le[0].lsb == (BYTE) 0xFFu) {
        utf16le += 1;
    }

    STATUS_CODE status = STATUS_CODE_SUCCESS;
    NUINT write = 0;

    while (utf16le[0].msb != 0 || utf16le[0].lsb != 0) {
        CODEPOINT codepoint;
        BYTE output[4];
        NUINT consumed;
        NUINT produced;
        RESULT decodeResult;
        RESULT encodeResult;
        NUINT i;

        output[0] = 0;
        output[1] = 0;
        output[2] = 0;
        output[3] = 0;

        catteUtf16leDecode(utf16le, &codepoint, &consumed, &decodeResult);
        if (statusCodeIsError(decodeResult.status) || statusCodeIsFatal(decodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, (BYTE) decodeResult.status);
            return;
        }
        if (statusCodeIsWarning(decodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        catteUtf8Encode(codepoint, output, &produced, &encodeResult);
        if (statusCodeIsError(encodeResult.status) || statusCodeIsFatal(encodeResult.status)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, (BYTE) encodeResult.status);
            return;
        }
        if (statusCodeIsWarning(encodeResult.status)) {
            status = STATUS_CODE_WARNING_PARTIAL;
        }

        if (write >= capacity || produced >= (capacity - write)) {
            if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
            return;
        }

        for (i = 0; i < produced; ++i) {
            utf8w[write + i] = output[i];
        }

        utf16le += consumed;
        write += produced;
    }

    utf8w[write] = (BYTE) '\0';
    if (result != 0) *result = RESULT(status, 0, 0, 0);
}
