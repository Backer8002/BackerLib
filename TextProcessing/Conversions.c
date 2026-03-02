#include "Conversions.h"

BL_UnicodeString bl_textprocessing_from_unicode(BL_Container data, BL_TextProcessing_Encoding encoding) {
    if (encoding != BL_TextProcessing_Encoding_UTF8 && bl_container_size(&data) % 2 != 0)
        return (BL_UnicodeString) {0};
    if ((encoding == BL_TextProcessing_Encoding_UTF32LE || encoding == BL_TextProcessing_Encoding_UTF32BE) 
        && bl_container_size(&data) % 4 != 0)
        return (BL_UnicodeString) {0};

    BL_UnicodeString result = bl_container_dynamic_create_stack(0,sizeof(BL_UnicodePoint));
    switch (encoding) {
        case BL_TextProcessing_Encoding_UTF8: {
            size_t size = bl_container_size(&data);
            for (size_t i = 0; i < size; i++) {
                BL_Byte character = *(BL_Byte*)bl_container_get(&data,i);
            }
        } break;
        case BL_TextProcessing_Encoding_UTF16BE:
        case BL_TextProcessing_Encoding_UTF16LE: {
            size_t maxIndex = bl_container_size(&data)/2;
            for (size_t i = 0; i < maxIndex; i++) {
                BL_Byte firstByte = *(BL_Byte*)bl_container_get(&data,i*2);
                BL_Byte secondByte = *(BL_Byte*)bl_container_get(&data,i*2 + 1);
                bool grabMore = false;
                if (encoding == BL_TextProcessing_Encoding_UTF16BE && (firstByte & 0xfc) == 0xd8)
                    grabMore = true;
                else if ((secondByte & 0xfc) == 0xd8)
                    grabMore = true;
                BL_TextProcessing_UTFCodepoint utfCodepoint = {.bytesUsed = grabMore ? 4 : 2, .bytes = {firstByte,secondByte}};
                if (grabMore && i + 1 >= maxIndex) {
                    utfCodepoint.bytes[3] = *(BL_Byte*)bl_container_get(&data,i*2+2);
                    utfCodepoint.bytes[4] = *(BL_Byte*)bl_container_get(&data,i*2+3);
                    i++;
                }
                BL_UnicodePoint codepoint = bl_textprocessing_to_unicodepoint(utfCodepoint,encoding);
                if (bl_container_dynamic_append(&result,sizeof codepoint,&codepoint) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            }
        } break;
        case BL_TextProcessing_Encoding_UTF32BE:
        case BL_TextProcessing_Encoding_UTF32LE: {
            size_t maxIndex = bl_container_size(&data)/4;
            for (size_t i = 0; i < maxIndex;i++) {
                BL_TextProcessing_UTFCodepoint utfCodepoint = {
                    .bytesUsed = 4,
                    .bytes = {
                        *(BL_Byte*)bl_container_get(&data,i*4),
                        *(BL_Byte*)bl_container_get(&data,i*4+1),
                        *(BL_Byte*)bl_container_get(&data,i*4+2),
                        *(BL_Byte*)bl_container_get(&data,i*4+3)
                    }
                };
                BL_UnicodePoint codepoint = bl_textprocessing_to_unicodepoint(utfCodepoint,encoding);
                if (bl_container_dynamic_append(&result,sizeof codepoint,&codepoint) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            }
        } break;
    }
    if (bl_container_dynamic_append(&result,sizeof (BL_UnicodePoint),&(BL_UnicodePoint){0}) != BL_ContainerOPSuccessful)
        goto ErrorExit;
    return result;

    ErrorExit:
    bl_container_dynamic_destroy(&result);
    return result;
}
