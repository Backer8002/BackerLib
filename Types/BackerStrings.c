#include "BackerStrings.h"
#include "TypesMain.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <wchar.h>

BL_String bl_string_create(const unsigned char* string, size_t length) {
    BL_String allocatedString = bl_container_dynamic_create_stack(length + 1, sizeof(char));

    if (!bl_container_dynamic_is_valid(&allocatedString))
        return allocatedString;

    memcpy(allocatedString.container.array, string, length);
    allocatedString.container.amountOfIndexes = length;
    bl_container_dynamic_append(&allocatedString, sizeof (char),&(char){'\0'});
    return allocatedString;
}

size_t     bl_string_length(const BL_StringView* string) {
    return string->amountOfIndexes - 1; // Strings always have atleast len 1.
}

inline unsigned char* bl_string_get_char(const BL_StringView* string, size_t index) {
    return bl_container_get(&string->container, index);
}

BL_ContainerError bl_string_append_cstring(BL_String* destString, const unsigned char* stringToInsert, size_t length) {
    return bl_container_dynamic_insert(destString, bl_string_length(bl_stringview_ptr_cast(destString)), length, sizeof(*stringToInsert), stringToInsert);
}

BL_ContainerError bl_string_insert_cstring(BL_String* destString, const unsigned char* other, const size_t lenOfOther, const size_t firstIndex) {
    return bl_container_dynamic_insert(destString, firstIndex, lenOfOther, sizeof(*other), other);
}

BL_ContainerError bl_string_append_string(BL_String* destString, const BL_StringView* stringToInsert) {
    BL_ContainerError errorCode = bl_container_dynamic_insert_container(destString, bl_string_length(bl_stringview_ptr_cast(destString)),&stringToInsert->container);
    if (errorCode != BL_ContainerOPSuccessful)
        return errorCode;
    bl_container_dynamic_pop(destString);
    return BL_ContainerOPSuccessful;
}

BL_String bl_string_strip(const BL_String* string, unsigned char charToStrip, bool stripFromBack, uint64_t amountToStrip) {
    if (string->container.byteSizeOfSingleElement != sizeof charToStrip) {
        BL_String returnString = {0};
        return returnString;
    }
    BL_String returnString = bl_container_dynamic_create_stack(string->container.amountOfIndexes, sizeof(char));
    if (!bl_container_dynamic_is_valid(&returnString))
        return returnString;

    size_t amountStripped = 0;
    for (size_t iterator = 0; iterator < bl_string_length((BL_StringView*)(BL_Container*)string); iterator++) {
        unsigned char* currentChar = bl_string_get_char((BL_StringView*)(BL_Container*)string, (stripFromBack ? bl_string_length((BL_StringView*)(BL_Container*)string) - 1 - iterator : iterator));
        if ((*currentChar != charToStrip) || (amountToStrip && (amountStripped >= amountToStrip)))
            bl_container_dynamic_append(&returnString, sizeof(*currentChar), currentChar);
        else
            amountStripped++;
    }
    if (stripFromBack)
        bl_container_reverse(&returnString.container);
    bl_container_dynamic_append(&returnString,sizeof(char),&(char){'\0'});
    return returnString;
}

BL_DynamicContainer bl_string_split(const BL_StringView* string, unsigned char charToSplitOn, bool splitFromBack, uint64_t amountOfCharsToSplitAt) {
    return bl_string_split_multi(string,&charToSplitOn,1,splitFromBack,amountOfCharsToSplitAt);
}

BL_DynamicContainer bl_string_split_multi(const BL_StringView* string, const unsigned char* charsToSplitOn, size_t amountOfChars, bool splitFromBack, uint64_t amountOfCharsToSplitAt) {
    BL_DynamicContainer stringArray = bl_container_dynamic_create_stack(0,sizeof(BL_String));
    bool charsToSplit[256] = {0};
    for (size_t i = 0; i < amountOfChars; i++)
        charsToSplit[charsToSplitOn[i]] = true;

    size_t amountSplit = 0, beginOfSubString = splitFromBack ? bl_string_length(string) - 1 : 0;

    for (size_t i = 0; i < bl_string_length(string); i++) {
        size_t currentIndexInString = splitFromBack ? bl_string_length(string) - 1 - i : i;
        unsigned char currentChar = *bl_string_get_char(string, currentIndexInString);
        if (!charsToSplit[currentChar])
            continue;

        if (beginOfSubString == currentIndexInString) {
            beginOfSubString += splitFromBack ? -1 : 1;
            continue;
        }

        BL_String subString = bl_string_create(
            bl_string_get_char(string,splitFromBack ? currentIndexInString + 1 : beginOfSubString),
            splitFromBack ? beginOfSubString - currentIndexInString : currentIndexInString - beginOfSubString);

        if (!bl_container_dynamic_is_valid(&subString)) {
            bl_container_dynamic_destroy_with_elements(&stringArray,bl_container_destroy);
            return stringArray;
        }

        if (bl_container_dynamic_append(&stringArray,sizeof subString, &subString) != BL_ContainerOPSuccessful) {
            bl_container_destroy(&subString);
            bl_container_dynamic_destroy_with_elements(&stringArray,bl_container_destroy);
            return stringArray;
        }

        beginOfSubString = currentIndexInString + (splitFromBack ? -1 : 1);

        if (amountOfCharsToSplitAt && amountSplit++ > amountOfCharsToSplitAt)
            break;
    }

    if ((splitFromBack && beginOfSubString != SIZE_MAX) || (!splitFromBack && beginOfSubString != bl_string_length(string))) {
        BL_String subString;
        if (splitFromBack)
            subString = bl_string_create(bl_string_get_char(string,0),beginOfSubString + 1);
        else
            subString = bl_string_create(bl_string_get_char(string,beginOfSubString),bl_string_length(string) - beginOfSubString);
        if (!bl_container_dynamic_is_valid(&subString)) {
            bl_container_dynamic_destroy_with_elements(&stringArray,bl_container_destroy);
            return stringArray;
        }

        if (bl_container_dynamic_append(&stringArray,sizeof subString, &subString) != BL_ContainerOPSuccessful) {
            bl_container_destroy(&subString);
            bl_container_dynamic_destroy_with_elements(&stringArray,bl_container_destroy);
            return stringArray;
        }
    }

    if (splitFromBack)
        bl_container_reverse((BL_Container*)&stringArray);
    return stringArray;
}

BL_ContainerError bl_string_replace(BL_String* destString, const BL_String* stringToReplaceWith, size_t firstIndex) {
    if (firstIndex >= bl_container_dynamic_size(destString))
        return BL_ContainerInvalidIndex;
    if (*bl_string_get_char(bl_stringview_ptr_cast(destString),firstIndex) > 127)
        return BL_ContainerInvalidIndex;

    if (stringToReplaceWith->container.amountOfIndexes > destString->container.amountOfIndexes - firstIndex - 1) {
        if (bl_container_dynamic_reserve(destString, stringToReplaceWith->container.amountOfIndexes - destString->container.amountOfIndexes + firstIndex + 1) == BL_ContainerAllocFailure)
            return BL_ContainerAllocFailure;
        destString->container.amountOfIndexes = stringToReplaceWith->container.amountOfIndexes + firstIndex + 1;
    }
    for (size_t i = 0; i < stringToReplaceWith->container.amountOfIndexes; i++)
        bl_container_set((BL_Container*) destString, i + firstIndex, sizeof(char), bl_container_get((const BL_Container*) stringToReplaceWith, i));
    return BL_ContainerOPSuccessful;
}

bool bl_string_compare_acending(const void* first, const void* second) {
    const BL_StringView* firstString = first;
    const BL_StringView* secondString = second;

    if(!first || !second)
        return false;

    for (size_t i = 0; i < bl_string_length(firstString); i++) {
        if (i>=bl_string_length(secondString))
            return false;
        bool firstIsUpper = false;
        bool secondIsUpper = false;
        unsigned char firstStringChar = *bl_string_get_char(firstString, i);
        unsigned char secondStringChar = *bl_string_get_char(secondString, i);
        if (isupper(firstStringChar))
            firstIsUpper = true;
        if (isupper(secondStringChar))
            secondIsUpper = true;
        firstStringChar = tolower(firstStringChar);
        secondStringChar = tolower(secondStringChar);
        if (firstStringChar > secondStringChar)
            return false;
        if (firstStringChar < secondStringChar)
            return true;
        if (firstIsUpper ^ secondIsUpper) {
            if (firstIsUpper)
                return true;
            return false;
        }
    }
    return true;
}

bool bl_string_compare_decending(const void* first, const void* second) {
    const BL_StringView* firstString = first;
    const BL_StringView* secondString = second;

    if(!first || !second)
        return false;

    for (size_t i = 0; i < bl_string_length(firstString); i++) {
        if (i>=bl_string_length(secondString))
            return true;
        bool firstIsUpper = false;
        bool secondIsUpper = false;
        unsigned char firstStringChar = *bl_string_get_char(firstString, i);
        unsigned char secondStringChar = *bl_string_get_char(secondString, i);
        if (isupper(firstStringChar))
            firstIsUpper = true;
        if (isupper(secondStringChar))
            secondIsUpper = true;
        firstStringChar = tolower(firstStringChar);
        secondStringChar = tolower(secondStringChar);
        if (firstStringChar > secondStringChar)
            return true;
        if (firstStringChar < secondStringChar)
            return false;
        if (firstIsUpper ^ secondIsUpper) {
            if (firstIsUpper)
                return false;
            return true;
        }
    }
    return true;
}

bool bl_string_equal(const void* first, const void* second) {
    const BL_StringView* firstString = first;
    const BL_StringView* secondString = second;

    if(!first || !second)
        return false;

    if (bl_string_length(firstString) != bl_string_length(secondString))
        return false;

    return memcmp(firstString->array,secondString->array,bl_string_length(firstString)) == 0;
}

BL_StringW bl_stringw_create(const wchar_t* str, size_t len) {
    BL_StringW allocatedString = bl_container_dynamic_create_stack(len+1, sizeof(wchar_t));

    if (!bl_container_dynamic_is_valid(&allocatedString))
        return allocatedString;

    memcpy(allocatedString.container.array, str, len * sizeof(wchar_t));
    allocatedString.container.amountOfIndexes = len;
    bl_container_dynamic_append(&allocatedString, sizeof(wchar_t),&(wchar_t){L'\0'});
    return allocatedString;
}

BL_StringView bl_stringview_init(const char* str) {
    return (BL_StringView) {
        .amountOfIndexes         = strlen(str),
        .array                   = (unsigned char*)str,
        .byteSizeOfSingleElement = 1,
        .header                  = ObjectFlagIsValid | ObjectFlagIsContainer};
}

BL_StringViewW bl_stringview_w_init(const wchar_t* str) {
    return (BL_StringViewW) {
        .amountOfIndexes         = wcslen(str),
        .array                   = str,
        .byteSizeOfSingleElement = sizeof(wchar_t),
        .header                  = ObjectFlagIsValid | ObjectFlagIsContainer};
}

BL_StringView bl_stringview_cast(BL_String string) {
    return (BL_StringView){.container = string.container};
}

BL_StringView* bl_stringview_ptr_cast(const BL_String* string) {
    return (BL_StringView*)(BL_Container*)string;
}