#include "Conversions.h"
#include <stddef.h>
#include <BackerLibTypes.h>

BL_UnicodeString bl_textprocessing_to_unicode(BL_Container data, BL_TextProcessing_Encoding encoding) {
    if (encoding != BL_TextProcessing_Encoding_UTF8 && bl_container_size(&data) % 2 != 0)
        return (BL_UnicodeString) {0};
    if ((encoding == BL_TextProcessing_Encoding_UTF32LE || encoding == BL_TextProcessing_Encoding_UTF32BE) && bl_container_size(&data) % 4 != 0)
        return (BL_UnicodeString) {0};

    BL_UnicodeString result = bl_container_dynamic_create_stack(0, sizeof(BL_Unicodepoint));
    switch (encoding) {
    case BL_TextProcessing_Encoding_UTF8: {
        size_t size = bl_container_size(&data);
        size_t i    = 0;
        while (i < size) {
            BL_Byte character    = *(BL_Byte*) bl_container_get(&data, i);
            size_t  requiredSize = 0;
            for (; requiredSize < 8 && ((character << requiredSize) & 0x80); requiredSize++) {}
            if (requiredSize > 4)
                requiredSize = 4; // Issue diagnostic?
            if (i + requiredSize > size)
                requiredSize = size - i;

            BL_TextProcessing_UTFCodepoint uftCodepoint = {.bytesUsed = requiredSize};
            for (size_t j = 0; j < requiredSize; j++)
                uftCodepoint.bytes[j] = *(BL_Byte*) bl_container_get(&data, i + j);
            BL_Unicodepoint codepoint = bl_textprocessing_to_unicodepoint(uftCodepoint, BL_TextProcessing_Encoding_UTF8);
            if (bl_container_dynamic_append(&result, sizeof codepoint, &codepoint) != BL_ContainerOPSuccessful)
                goto ErrorExit;
            i += requiredSize;
        }
    } break;
    case BL_TextProcessing_Encoding_UTF16BE:
    case BL_TextProcessing_Encoding_UTF16LE: {
        size_t maxIndex = bl_container_size(&data) / 2;
        for (size_t i = 0; i < maxIndex; i++) {
            BL_Byte firstByte  = *(BL_Byte*) bl_container_get(&data, i * 2);
            BL_Byte secondByte = *(BL_Byte*) bl_container_get(&data, i * 2 + 1);
            bool    grabMore   = false;
            if (encoding == BL_TextProcessing_Encoding_UTF16BE && (firstByte & 0xfc) == 0xd8)
                grabMore = true;
            else if (encoding == BL_TextProcessing_Encoding_UTF16LE &&(secondByte & 0xfc) == 0xd8)
                grabMore = true;
            BL_TextProcessing_UTFCodepoint utfCodepoint = {.bytesUsed = grabMore ? 4 : 2, .bytes = {firstByte, secondByte}};
            if (grabMore && i + 1 >= maxIndex) {
                utfCodepoint.bytes[3] = *(BL_Byte*) bl_container_get(&data, i * 2 + 2);
                utfCodepoint.bytes[4] = *(BL_Byte*) bl_container_get(&data, i * 2 + 3);
                i++;
            }
            BL_Unicodepoint codepoint = bl_textprocessing_to_unicodepoint(utfCodepoint, encoding);
            if (bl_container_dynamic_append(&result, sizeof codepoint, &codepoint) != BL_ContainerOPSuccessful)
                goto ErrorExit;
        }
    } break;
    case BL_TextProcessing_Encoding_UTF32BE:
    case BL_TextProcessing_Encoding_UTF32LE: {
        size_t maxIndex = bl_container_size(&data) / 4;
        for (size_t i = 0; i < maxIndex; i++) {
            BL_TextProcessing_UTFCodepoint utfCodepoint = {
                .bytesUsed = 4,
                .bytes     = {
                    *(BL_Byte*) bl_container_get(&data, i * 4),
                    *(BL_Byte*) bl_container_get(&data, i * 4 + 1),
                    *(BL_Byte*) bl_container_get(&data, i * 4 + 2),
                    *(BL_Byte*) bl_container_get(&data, i * 4 + 3)}};
            BL_Unicodepoint codepoint = bl_textprocessing_to_unicodepoint(utfCodepoint, encoding);
            if (bl_container_dynamic_append(&result, sizeof codepoint, &codepoint) != BL_ContainerOPSuccessful)
                goto ErrorExit;
        }
    } break;
    }
    if (bl_container_dynamic_append(&result, sizeof(BL_Unicodepoint), &(BL_Unicodepoint) {0}) != BL_ContainerOPSuccessful)
        goto ErrorExit;
    return result;

ErrorExit:
    bl_container_dynamic_destroy(&result);
    return result;
}

BL_Unicodepoint bl_textprocessing_to_unicodepoint(BL_TextProcessing_UTFCodepoint utf, BL_TextProcessing_Encoding encoding) {
    if (utf.bytesUsed > 4 || utf.bytesUsed <= 0) {
        return BL_TextProcessing_Unicode_Unknown;
    }
    switch (encoding) {
    case BL_TextProcessing_Encoding_UTF8: {
        for (size_t i = 0; i < utf.bytesUsed; i++) {
            if (utf.bytes[i] == 0xff || utf.bytes[i] == 0xfe || (utf.bytes[i] & 0xfe ) == 0xc0 )
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
    size_t elementSize = 0;

    switch (encoding) {
    case BL_TextProcessing_Encoding_UTF8:
        elementSize = 1;
        break;
    case BL_TextProcessing_Encoding_UTF16BE:
    case BL_TextProcessing_Encoding_UTF16LE:
        elementSize = 2;
        break;
    case BL_TextProcessing_Encoding_UTF32BE:
    case BL_TextProcessing_Encoding_UTF32LE:
        elementSize = 4;
        break;
    }
    BL_DynamicContainer result = bl_container_dynamic_create_stack(0, elementSize);

    for (BL_Unicodepoint* current = bl_container_front(&unicode); current; current = bl_container_next(&unicode, current)) {
        BL_TextProcessing_UTFCodepoint utfCodepoint = bl_textprocessing_from_unicodepoint(*current, encoding);
        if (bl_container_dynamic_insert(&result, bl_container_dynamic_size(&result), utfCodepoint.bytesUsed / elementSize, elementSize, utfCodepoint.bytes) != BL_ContainerOPSuccessful)
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
        return (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 4, .bytes = {codepoint >> 24,codepoint >> 16 &0xff,codepoint >> 8 & 0xff,codepoint & 0xff}};
    }

    case BL_TextProcessing_Encoding_UTF32LE: {
        return (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 4, .bytes =  {codepoint & 0xff,codepoint >> 8 & 0xff, codepoint >> 16 & 0xff, codepoint >> 24}};
    }

    }
    return (BL_TextProcessing_UTFCodepoint) {.bytesUsed = 0};
}

BL_TextProcessing_UTFCodepoint bl_textprocessing_transcode_utfcodepoint(
    BL_TextProcessing_UTFCodepoint utf,
    BL_TextProcessing_Encoding sourceEncoding,BL_TextProcessing_Encoding targetEncodeing) 
    {
        return bl_textprocessing_from_unicodepoint(bl_textprocessing_to_unicodepoint(utf,sourceEncoding),targetEncodeing);
    }

BL_DynamicContainer bl_textprocessing_transcode_utf(BL_Container utf,BL_TextProcessing_Encoding sourceEncoding,BL_TextProcessing_Encoding targetEncodeing) {
    size_t targetSize = 0;
    switch(targetEncodeing) {
        case BL_TextProcessing_Encoding_UTF8: targetSize = 1; break;
        case BL_TextProcessing_Encoding_UTF16BE:
        case BL_TextProcessing_Encoding_UTF16LE: targetSize = 2; break;
        case BL_TextProcessing_Encoding_UTF32LE:
        case BL_TextProcessing_Encoding_UTF32BE: targetSize = 4; break;
    }
    BL_DynamicContainer result = bl_container_dynamic_create_stack(0,targetSize);

    for ()
}
