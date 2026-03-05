#ifndef BL_TEXT_PROCESSING_UNICODE_STRING_H
#define BL_TEXT_PROCESSING_UNICODE_STRING_H

#include<BackerLibTypes.h>
#include<inttypes.h>
#include<stddef.h>

#ifdef __cplusplus
#define BL_NOEXCEPT noexcept(true)
extern "C" {
#else
#define BL_NOEXCEPT
#endif

typedef uint32_t BL_Unicodepoint;
struct BL_Internal_UnicodeString{
        BL_Unicodepoint* data;
        size_t length;
        size_t capacity;
};
typedef union BL_UnicodeString {
    struct BL_Internal_UnicodeString str;
    BL_Unicodepoint smallStr[sizeof(struct BL_Internal_UnicodeString) / 4 - 1];
} BL_UnicodeString;
typedef struct BL_UnicodeView {
    BL_Unicodepoint* data;
    size_t length;
} BL_UnicodeView;

BL_UnicodeString bl_unicodestr_create(void) BL_NOEXCEPT;
BL_Unicodepoint* bl_unicodestr_get(BL_UnicodeView str) BL_NOEXCEPT;
BL_ContainerError bl_unicodestr_extend(BL_UnicodeString str, BL_UnicodeView other) BL_NOEXCEPT;
BL_UnicodeView bl_unicodestr_substr(BL_UnicodeView str, size_t begin, size_t end) BL_NOEXCEPT;
BL_UnicodeString bl_unicodestr_copy(BL_UnicodeView str) BL_NOEXCEPT;
BL_UnicodeView bl_unicodestr_view(BL_UnicodeString str) BL_NOEXCEPT;
BL_Unicodepoint* bl_unicodestr_begin(BL_UnicodeView str) BL_NOEXCEPT;
BL_Unicodepoint* bl_unicodestr_back(BL_UnicodeView str) BL_NOEXCEPT;
BL_Unicodepoint* bl_unicodestr_next(BL_UnicodeView str, BL_Unicodepoint* element) BL_NOEXCEPT;
BL_Unicodepoint* bl_unicodestr_prev(BL_UnicodeView str, BL_Unicodepoint* element) BL_NOEXCEPT;
void bl_unicodestr_destroy(void* str) BL_NOEXCEPT;

#ifdef __cplusplus
}
#endif
#undef BL_NOEXCEPT
#endif
