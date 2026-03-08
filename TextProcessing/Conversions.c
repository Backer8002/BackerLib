#include "Conversions.h"
#include "UnicodeString.h"
#include <BackerLibTypes.h>
#include <stdbool.h>
#include <stddef.h>

BL_TextProcessing_UTFCodepoint bl_textprocessing_get_next_utfcodepoint(const BL_Container* container, const BL_Byte** currentElement, BL_TextProcessing_Encoding encoding) {
    BL_TextProcessing_UTFCodepoint result = {0};
    switch (encoding) {

    case BL_TextProcessing_Encoding_UTF8: {
        const BL_Byte* current = *currentElement;
        while (current && (*current & 0xc0) == 0x80) // We will ignore characters in the middle of a word, because they likely come from an invalid codepoint
            current = bl_container_next(container, current);
        if (!current) {
            *currentElement = NULL;
            return result;
        }
        BL_Byte amountOfBytes = 1;
        for (; amountOfBytes < 4 && (0x80 & (*current << (amountOfBytes - 1))); amountOfBytes++) {}
        result.bytesUsed = amountOfBytes;
        for (size_t i = 0; i < amountOfBytes; i++) {
            result.bytes[i] = *current;
            current         = bl_container_next(container, current);
            if (!current) {
                *currentElement = NULL;
                return (BL_TextProcessing_UTFCodepoint) {0};
            }
        }
        *currentElement = current;
    } break;
    case BL_TextProcessing_Encoding_UTF16LE:
    case BL_TextProcessing_Encoding_UTF16BE: {
        const BL_Byte* first  = *currentElement;
        const BL_Byte* second = bl_container_next(container, first);
        if (!first || !second) {
            *currentElement = NULL;
            return result;
        }
        bool grabMore = false;

        if (encoding == BL_TextProcessing_Encoding_UTF16BE && (*first & 0xfc) == 0xd8)
            grabMore = true;
        else if (encoding == BL_TextProcessing_Encoding_UTF16LE && (*second & 0xfc) == 0xd8)
            grabMore = true;

        if (grabMore) {
            const BL_Byte* third  = bl_container_next(container, second);
            const BL_Byte* fourth = bl_container_next(container, third);
            if (!third || !fourth) {
                *currentElement = NULL;
                return result;
            }
            *currentElement = bl_container_next(container, fourth);
            result          = (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 4, .bytes = {*first, *second, *third, *fourth}};
        } else {
            *currentElement = bl_container_next(container, second);
            result          = (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 2, .bytes = {*first, *second}};
        }
    } break;
    case BL_TextProcessing_Encoding_UTF32BE:
    case BL_TextProcessing_Encoding_UTF32LE: {
        const BL_Byte* first  = *currentElement;
        const BL_Byte* second = bl_container_next(container, first);
        const BL_Byte* third  = bl_container_next(container, second);
        const BL_Byte* fourth = bl_container_next(container, third);
        if (!first || !second || !third || !fourth) {
            *currentElement = NULL;
            return result;
        }
        *currentElement = bl_container_next(container, fourth);
        result          = (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 4, .bytes = {*first, *second, *third, *fourth}};
    } break;
    }
    return result;
}

BL_UnicodeString bl_textprocessing_to_unicode(BL_Container data, BL_TextProcessing_Encoding encoding) {
    if (encoding != BL_TextProcessing_Encoding_UTF8 && bl_container_size(&data) % 2 != 0)
        return (BL_UnicodeString) {0};
    if ((encoding == BL_TextProcessing_Encoding_UTF32LE || encoding == BL_TextProcessing_Encoding_UTF32BE) && bl_container_size(&data) % 4 != 0)
        return (BL_UnicodeString) {0};

    BL_UnicodeString result = bl_unicodestr_create();
    if (!bl_unicodestr_is_valid(result))
        return result;

    const BL_Byte*   begin  = bl_container_front(&data);
    while (begin) {
        BL_TextProcessing_UTFCodepoint utfCodepoint = bl_textprocessing_get_next_utfcodepoint(&data, &begin, encoding);

        if (utfCodepoint.bytesUsed == 0)
            goto ErrorExit;

        if (bl_unicodestr_append(&result, bl_textprocessing_to_unicodepoint(utfCodepoint, encoding)) != BL_ContainerOPSuccessful)
            goto ErrorExit;
    }
    return result;

ErrorExit:
    bl_unicodestr_destroy(&result);
    return result;
}

BL_Unicodepoint bl_textprocessing_to_unicodepoint(BL_TextProcessing_UTFCodepoint utf, BL_TextProcessing_Encoding encoding) {
    if (utf.bytesUsed > 4 || utf.bytesUsed <= 0) {
        return BL_TextProcessing_Unicode_Unknown;
    }
    switch (encoding) {
    case BL_TextProcessing_Encoding_UTF8: {
        for (size_t i = 0; i < utf.bytesUsed; i++) {
            if (utf.bytes[i] == 0xff || utf.bytes[i] == 0xfe || (utf.bytes[i] & 0xfe) == 0xc0)
                return BL_TextProcessing_Unicode_Unknown; // Invalid utf-8
        }
        if (utf.bytesUsed == 1) {
            if (utf.bytes[0] & 0x80)
                return BL_TextProcessing_Unicode_Unknown;
            return utf.bytes[0];
        }

        if ((BL_Unicodepoint) utf.bytes[0] - 0xc2 > 0xf7 - 0xc2 || ((0xf1 << (4 - utf.bytesUsed)) & utf.bytes[0]) != (0xf0 << (4 - utf.bytesUsed)))
            return BL_TextProcessing_Unicode_Unknown;
        for (size_t i = 1; i < utf.bytesUsed; i++) {
            if (((BL_Unicodepoint) utf.bytes[i] & 0xc0) != 0x80)
                return BL_TextProcessing_Unicode_Unknown;
        }
        BL_Unicodepoint result = utf.bytes[0] & (0xff >> utf.bytesUsed);
        for (size_t i = 1; i < utf.bytesUsed; i++) {
            result <<= 6;
            result |= (BL_Unicodepoint) utf.bytes[i] & 0x3f;
        }
        return result;
    } break;

    case BL_TextProcessing_Encoding_UTF16BE:
    case BL_TextProcessing_Encoding_UTF16LE: {
        if (utf.bytesUsed % 2 != 0)
            return BL_TextProcessing_Unicode_Unknown;
        BL_Byte firstChar, secondChar, thirdChar, fourthChar;
        if (encoding == BL_TextProcessing_Encoding_UTF16BE)
            firstChar = utf.bytes[0], secondChar = utf.bytes[1], thirdChar = utf.bytes[2], fourthChar = utf.bytes[3];
        else
            firstChar = utf.bytes[1], secondChar = utf.bytes[0], thirdChar = utf.bytes[3], fourthChar = utf.bytes[2];
        if (utf.bytesUsed == 2) {
            if ((firstChar & 0xf8) == 0xd8)
                return BL_TextProcessing_Unicode_Unknown;
            return firstChar << 8 | secondChar;
        }
        if ((firstChar & 0xfc) != 0xd8 && (thirdChar & 0xfc) != 0xdc)
            return BL_TextProcessing_Unicode_Unknown;
        return 0x10000 + ((firstChar & 0x03) << 18 | secondChar << 10 | (thirdChar & 0x03) << 8 | fourthChar);
    }

    case BL_TextProcessing_Encoding_UTF32BE: {
        if (utf.bytesUsed != 4)
            return BL_TextProcessing_Unicode_Unknown;
        return (BL_Unicodepoint) utf.bytes[0] << 24 | (BL_Unicodepoint) utf.bytes[1] << 16 | (BL_Unicodepoint) utf.bytes[2] << 8 | (BL_Unicodepoint) utf.bytes[3];
    }
    case BL_TextProcessing_Encoding_UTF32LE: {
        if (utf.bytesUsed != 4)
            return BL_TextProcessing_Unicode_Unknown;
        return (BL_Unicodepoint) utf.bytes[0] | (BL_Unicodepoint) utf.bytes[1] << 8 | (BL_Unicodepoint) utf.bytes[2] << 16 | (BL_Unicodepoint) utf.bytes[3] << 24;
    }
    }
    return BL_TextProcessing_Unicode_Unknown;
}

BL_DynamicContainer bl_textprocessing_from_unicode(BL_UnicodeView unicode, BL_TextProcessing_Encoding encoding) {
    BL_DynamicContainer result = bl_container_dynamic_create_stack(0, 1);

    for (BL_Unicodepoint* current = bl_unicodestr_front(unicode); current; current = bl_unicodestr_next(unicode, current)) {
        BL_TextProcessing_UTFCodepoint utfCodepoint = bl_textprocessing_from_unicodepoint(*current, encoding);
        if (bl_container_dynamic_insert(&result, bl_container_dynamic_size(&result), utfCodepoint.bytesUsed, 1, utfCodepoint.bytes) != BL_ContainerOPSuccessful)
            goto ErrorExit;
    }

    return result;

ErrorExit:
    bl_container_dynamic_destroy(&result);
    return result;
}

BL_TextProcessing_UTFCodepoint bl_textprocessing_from_unicodepoint(BL_Unicodepoint codepoint, BL_TextProcessing_Encoding encoding) {

    switch (encoding) {
    case BL_TextProcessing_Encoding_UTF8: {
        if (codepoint < 0x80)
            return (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 1, .bytes = {codepoint & 0xff}};
        else if (codepoint < 0x800)
            return (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 2, .bytes = {codepoint >> 6 | 0xc0, (codepoint & 0x3f) | 0x80}};
        else if (codepoint < 0x10000)
            return (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 3, .bytes = {codepoint >> 12 | 0xe0, ((codepoint >> 6) & 0x3f) | 0x80, (codepoint & 0x3f) | 0x80}};
        return (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 4, .bytes = {codepoint >> 18 | 0xf0, ((codepoint >> 12) & 0x3f) | 0x80, ((codepoint >> 6) & 0x3f) | 0x80, (codepoint & 0x3f) | 0x80}};
    } break;

    case BL_TextProcessing_Encoding_UTF16BE: {
        if (codepoint < 0x10000)
            return (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 2, .bytes = {codepoint >> 8, codepoint & 0xff}};
        codepoint -= 0x10000;
        return (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 4, .bytes = {(codepoint >> 18) & 0x03 | 0xd8, (codepoint >> 10) & 0xff, ((codepoint >> 8) & 0x03) | 0xdc, codepoint & 0xff}};
    }

    case BL_TextProcessing_Encoding_UTF16LE: {
        if (codepoint < 0x10000)
            return (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 2, .bytes = {codepoint & 0xff, codepoint >> 8}};
        codepoint -= 0x10000;
        return (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 4, .bytes = {(codepoint >> 10) & 0xff, ((codepoint >> 18) & 0x03) | 0xd8, codepoint & 0xff, ((codepoint >> 8) & 0x03) | 0xdc}};
    }

    case BL_TextProcessing_Encoding_UTF32BE: {
        return (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 4, .bytes = {codepoint >> 24, codepoint >> 16 & 0xff, codepoint >> 8 & 0xff, codepoint & 0xff}};
    }

    case BL_TextProcessing_Encoding_UTF32LE: {
        return (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 4, .bytes = {codepoint & 0xff, codepoint >> 8 & 0xff, codepoint >> 16 & 0xff, codepoint >> 24}};
    }
    }
    return (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 0};
}

BL_TextProcessing_UTFCodepoint bl_textprocessing_transcode_utfcodepoint(
    BL_TextProcessing_UTFCodepoint utf,
    BL_TextProcessing_Encoding sourceEncoding, BL_TextProcessing_Encoding targetEncodeing) {
    return bl_textprocessing_from_unicodepoint(bl_textprocessing_to_unicodepoint(utf, sourceEncoding), targetEncodeing);
}

BL_DynamicContainer bl_textprocessing_transcode_utf(BL_Container utf, BL_TextProcessing_Encoding sourceEncoding, BL_TextProcessing_Encoding targetEncodeing) {
    BL_DynamicContainer result = bl_container_dynamic_create_stack(0, 1);

    const BL_Byte* currentPtr = bl_container_front(&utf);
    while (currentPtr) {
        BL_TextProcessing_UTFCodepoint sourceUTFPoint = bl_textprocessing_get_next_utfcodepoint(&utf, &currentPtr, sourceEncoding);
        if (sourceUTFPoint.bytesUsed == 0)
            goto ErrorExit;

        BL_TextProcessing_UTFCodepoint targetUTFPoint = bl_textprocessing_transcode_utfcodepoint(sourceUTFPoint, sourceEncoding, targetEncodeing);
        if (bl_container_dynamic_append(&result, targetUTFPoint.bytesUsed,targetUTFPoint.bytes) != BL_ContainerOPSuccessful)
            goto ErrorExit;
    }

    return result;
    ErrorExit:
    bl_container_dynamic_destroy(&result);
    return result;
}
