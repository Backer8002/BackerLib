#ifndef BL_TEXT_PROCESSING_UNICODE_STRING_H
#define BL_TEXT_PROCESSING_UNICODE_STRING_H

#include<BackerLibTypes.h>
#include<uchar.h>

#ifdef __cplusplus
#define BL_NOEXCEPT noexcept(true)
extern "C" {
#else
#define BL_NOEXCEPT
#endif

typedef BL_DynamicContainer BL_UnicodeString;
typedef BL_Container BL_UnicodeView;
typedef char32_t BL_UnicodePoint;

#ifdef __cplusplus
}
#endif
#undef BL_NOEXCEPT
#endif
