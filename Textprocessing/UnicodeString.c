#include "UnicodeString.h"
#include <BackerLibTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @returns true if operation was successful.
 */
static bool internal_unicodestr_check_and_resize_up(BL_UnicodeString* str, size_t expectedCapacity) {
    if (expectedCapacity > str->capacity >> 1) {
        size_t newCapacity = expectedCapacity * 2;
        if (newCapacity > SIZE_MAX >> 2)
            return false;

        BL_Unicodepoint* newPtr = realloc(str->data, (newCapacity+1) * sizeof *str->data);
        if (!newPtr)
            return false;

        str->capacity = newCapacity << 1;
        str->capacity |= 0x1;
    }
    return true;
}

BL_UnicodeString bl_unicodestr_create(void) {
    BL_UnicodeString str = {0};
    str.data             = malloc(sizeof *str.data);
    if (str.data) {
        str.capacity |= 0x1;
        str.data[0] = 0;
    }
    return str;
}

BL_Unicodepoint* bl_unicodestr_get(BL_UnicodeView str, size_t index) {
    if (index >= str.length)
        return NULL;
    return str.data + index;
}

BL_ContainerError bl_unicodestr_extend(BL_UnicodeString* str, BL_UnicodeView other) {
    if (!internal_unicodestr_check_and_resize_up(str, str->length + other.length))
        return BL_ContainerAllocFailure;
    memcpy(str->data + str->length, other.data, (other.length + 1) * sizeof *other.data);
    str->length += other.length;
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_unicodestr_append(BL_UnicodeString *str, BL_Unicodepoint codepoint) {
    if (!internal_unicodestr_check_and_resize_up(str, str->length+1))
        return  BL_ContainerAllocFailure;
    str->data[str->length] = codepoint;
    str->data[str->length + 1] = 0;
    str->length++;
    return BL_ContainerOPSuccessful;
}

BL_UnicodeView bl_unicodestr_substr(BL_UnicodeView str, size_t begin, size_t end) {
    if (begin >= end)
        return (BL_UnicodeView){0};
    if (end > str.length)
        return (BL_UnicodeView){0};
    return (BL_UnicodeView) {.data = str.data + begin, .length = end-begin};
}

BL_UnicodeString bl_unicodestr_copy(BL_UnicodeView str) {
    BL_UnicodeString newStr = {.length = str.length, .capacity = str.length << 1, .data = malloc((str.length + 1)* sizeof *str.data)};
    if (newStr.data) {
        newStr.capacity |= 0x1;
        memcpy(newStr.data, str.data, (str.length + 1) * sizeof *str.data);
    }
    return newStr;
}

BL_UnicodeView bl_unicodestr_view(BL_UnicodeString str) {
    return (BL_UnicodeView) {.data = str.data, .length = str.length};
}

BL_Unicodepoint* bl_unicodestr_front(BL_UnicodeView str) {
    if (str.length == 0)
        return NULL;
    return str.data;
}

BL_Unicodepoint* bl_unicodestr_back(BL_UnicodeView str) {
    if (str.length == 0)
        return NULL;
    return str.data + str.length - 1;
}

BL_Unicodepoint* bl_unicodestr_next(BL_UnicodeView str, BL_Unicodepoint *element) {
    if (element == bl_unicodestr_back(str))
        return NULL;
    return ++element;
}

BL_Unicodepoint* bl_unicodestr_prev(BL_UnicodeView str, BL_Unicodepoint* element) {
    if (element == bl_unicodestr_front(str))
        return NULL;
    return --element;
}

bool bl_unicodestr_equal(BL_UnicodeView first, BL_UnicodeView second) {
    if (first.length != second.length)
        return false;
    for (size_t i = 0; i < first.length; i++) {
        if (first.data[i] != second.data[i])
            return false;
    }
    return true;
}

bool bl_unicodestr_equal_ptr(const void *first, const void *second) {
    if (!first || !second)
        return false;
    return bl_unicodestr_equal(*(const BL_UnicodeView*) first, *(const BL_UnicodeView*) second);
}

bool bl_unicodestr_comp_ascending(BL_UnicodeView first, BL_UnicodeView second) {
    for (size_t i = 0; i < first.length; i++) {
        if (i >= second.length)
            return false;
        if (first.data[i] > second.data[i])
            return false;
        if (first.data[i] < second.data[i])
            return true;
    }
    return true;
}

bool bl_unicodestr_comp_ascending_ptr(const void *first, const void *second) {
    if (!first || !second)
        return false;
    return bl_unicodestr_comp_ascending(* (const BL_UnicodeView*) first, *(const BL_UnicodeView*) second);
}

bool bl_unicodestr_comp_decending(BL_UnicodeView first, BL_UnicodeView second) {
    for (size_t i = 0; i < second.length; i++) {
        if (i >= first.length)
            return true;
        if (first.data[i] > second.data[i])
            return true;
        if (first.data[i] < second.data[i])
            return false;
    }
    return false;
}

bool bl_unicodestr_comp_decending_ptr(const void *first, const void *second) {
    if (!first || !second)
        return false;
    return bl_unicodestr_comp_decending(*(const BL_UnicodeView*) first, *(const BL_UnicodeView*) second);
}

bool bl_unicodestr_is_valid(BL_UnicodeString str) {
    return str.capacity & 0x1;
}

size_t bl_unicodestr_length(BL_UnicodeView str) {
    return str.length;
}

void bl_unicodestr_destroy(void *str) {
    free(((BL_UnicodeString*)str)->data);
    ((BL_UnicodeString*)str)->capacity = 0;
}
