#ifndef BL_TEXT_PROCESSING_CONVERSIONS_H
#define BL_TEXT_PROCESSING_CONVERSIONS_H

#include "UnicodeString.h"

#ifdef __cplusplus
extern "C" {
#else
#define noexcept
#endif

typedef enum BL_Textprocessing_Encoding {
    BL_Textprocessing_Encoding_UTF8 = 0,
    BL_Textprocessing_Encoding_UTF16LE = 1,
    BL_Textprocessing_Encoding_UTF16BE = 2,
    BL_Textprocessing_Encoding_UTF32LE = 3,
    BL_Textprocessing_Encoding_UTF32BE = 4,
} BL_Textprocessing_Encoding;

typedef struct BL_Textprocessing_UTFCodepoint{
    BL_Byte bytesUsed;
    BL_Byte bytes[4];
} BL_Textprocessing_UTFCodepoint;

static const BL_Unicodepoint BL_Textprocessing_Unicode_Unknown = 0xfffd;

extern BL_Textprocessing_UTFCodepoint bl_textprocessing_get_next_utfcodepoint(const BL_Container* container, const BL_Byte** currentElement, BL_Textprocessing_Encoding encoding) noexcept;
extern BL_UnicodeString bl_textprocessing_to_unicode(BL_Container data, BL_Textprocessing_Encoding encoding) noexcept;
extern BL_Unicodepoint bl_textprocessing_to_unicodepoint(BL_Textprocessing_UTFCodepoint utf,BL_Textprocessing_Encoding encoding) noexcept;
extern BL_DynamicContainer bl_textprocessing_from_unicode(BL_UnicodeView unicode, BL_Textprocessing_Encoding encoding) noexcept;
extern BL_Textprocessing_UTFCodepoint bl_textprocessing_from_unicodepoint(BL_Unicodepoint codepoint,BL_Textprocessing_Encoding encoding) noexcept;
extern BL_DynamicContainer bl_textprocessing_transcode_utf(BL_Container utf,BL_Textprocessing_Encoding sourceEncoding,BL_Textprocessing_Encoding targetEncodeing) noexcept;
extern BL_Textprocessing_UTFCodepoint bl_textprocessing_transcode_utfcodepoint(
    BL_Textprocessing_UTFCodepoint utf,
    BL_Textprocessing_Encoding sourceEncoding,BL_Textprocessing_Encoding targetEncodeing) noexcept;

#ifdef __cplusplus
}
#else
#undef noexcept
#endif
#endif
