#ifndef BL_TEXTPROCESSING_REGEX_H
#define BL_TEXTPROCESSING_REGEX_H

#include <BackerLibTextprocessing.h>
#include <BackerLibTypes.h>

#ifdef __cplusplus
#define BL_NOEXCEPT noexcept
extern "C" {
#else
#define BL_NOEXCEPT
#endif

typedef struct BL_Regex {
    BL_Container ast;
    BL_Container stack;
    size_t stackPoint;
} BL_Regex;

/**
    @note The function itself is thread-safe. However, the regex is not.
*/
BL_Regex bl_regex_create(BL_UnicodeView expr) BL_NOEXCEPT;
BL_UnicodeView bl_regex_match(const BL_Regex* regex, BL_UnicodeView str) BL_NOEXCEPT;
BL_DynamicContainer bl_regex_matches(const BL_Regex* regex, BL_UnicodeView str,bool lineMatching,bool matchOverlapping) BL_NOEXCEPT;
void bl_regex_destory(void* regex) BL_NOEXCEPT;

BL_UnicodeString bl_unicodestr_strip(BL_UnicodeView str, const BL_Regex* regex,bool lineMatching);
BL_DynamicContainer bl_unicodestr_split(BL_UnicodeView str, const BL_Regex* regex, bool lineMatching);

#ifdef __cplusplus
}
#endif
#undef BL_NOEXCEPT
#endif
