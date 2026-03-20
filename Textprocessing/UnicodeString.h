#ifndef BL_TEXT_PROCESSING_UNICODE_STRING_H
#define BL_TEXT_PROCESSING_UNICODE_STRING_H

#include <BackerLibTypes.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#ifdef __cplusplus
#define BL_NOEXCEPT noexcept(true)
extern "C" {
#else
#define BL_NOEXCEPT
#endif

typedef uint32_t BL_Unicodepoint;
typedef struct BL_UnicodeString {
    BL_Unicodepoint* data;
    size_t           length;
    size_t           capacity; // First byte is is valid flag
} BL_UnicodeString;

typedef struct BL_UnicodeView {
    BL_Unicodepoint* data;
    size_t           length;
} BL_UnicodeView;

BL_UnicodeString  bl_unicodestr_create(void) BL_NOEXCEPT;
BL_Unicodepoint*  bl_unicodestr_get(BL_UnicodeView str, size_t index) BL_NOEXCEPT;
BL_ContainerError bl_unicodestr_extend(BL_UnicodeString* str, BL_UnicodeView other) BL_NOEXCEPT;
BL_ContainerError bl_unicodestr_append(BL_UnicodeString* str, BL_Unicodepoint codepoint) BL_NOEXCEPT;
BL_UnicodeView    bl_unicodestr_substr(BL_UnicodeView str, size_t begin, size_t end) BL_NOEXCEPT;
BL_UnicodeView    bl_unicodestr_split_front(BL_UnicodeView* str, size_t secondBegin) BL_NOEXCEPT;
BL_UnicodeView    bl_unicodestr_split_back(BL_UnicodeView* str, size_t secondBegin) BL_NOEXCEPT;
BL_UnicodeView    bl_unicodestr_splice_front(BL_UnicodeView* str, BL_Unicodepoint character) BL_NOEXCEPT;
BL_UnicodeView    bl_unicodestr_splice_front_excluding(BL_UnicodeView* str, BL_Unicodepoint character) BL_NOEXCEPT;
BL_UnicodeView    bl_unicodestr_splice_back(BL_UnicodeView* str, BL_Unicodepoint character) BL_NOEXCEPT;
BL_UnicodeView    bl_unicodestr_splice_back_excluding(BL_UnicodeView* str, BL_Unicodepoint character) BL_NOEXCEPT;
BL_UnicodeString  bl_unicodestr_copy(BL_UnicodeView str) BL_NOEXCEPT;
BL_UnicodeView    bl_unicodestr_view(BL_UnicodeString str) BL_NOEXCEPT;
BL_Unicodepoint*  bl_unicodestr_front(BL_UnicodeView str) BL_NOEXCEPT;
BL_Unicodepoint*  bl_unicodestr_back(BL_UnicodeView str) BL_NOEXCEPT;
BL_Unicodepoint*  bl_unicodestr_next(BL_UnicodeView str, const BL_Unicodepoint* element) BL_NOEXCEPT;
BL_Unicodepoint*  bl_unicodestr_prev(BL_UnicodeView str, const BL_Unicodepoint* element) BL_NOEXCEPT;
bool              bl_unicodestr_equal(BL_UnicodeView first, BL_UnicodeView second) BL_NOEXCEPT;
bool              bl_unicodestr_equal_ptr(const void* first, const void* second) BL_NOEXCEPT;
bool              bl_unicodestr_comp_ascending(BL_UnicodeView first, BL_UnicodeView second) BL_NOEXCEPT;
bool              bl_unicodestr_comp_ascending_ptr(const void* first, const void* second) BL_NOEXCEPT;
bool              bl_unicodestr_comp_decending(BL_UnicodeView first, BL_UnicodeView second) BL_NOEXCEPT;
bool              bl_unicodestr_comp_decending_ptr(const void* first, const void* second) BL_NOEXCEPT;
bool              bl_unicodestr_is_valid(BL_UnicodeString str) BL_NOEXCEPT;
size_t            bl_unicodestr_length(BL_UnicodeView str) BL_NOEXCEPT;
void              bl_unicodestr_destroy(void* str) BL_NOEXCEPT;

#ifdef __cplusplus
}
#endif
#undef BL_NOEXCEPT
#endif
