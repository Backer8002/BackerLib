#ifndef BackerString_h_
#define BackerString_h_


#include "Container.h"
#include "DynamicContainer.h"
#include <stddef.h>
#include <string.h>
#include <uchar.h>
#include <wchar.h>

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {



#endif // __cplusplus

    /**
     * @brief String should be treated as a DynamicContainer with element size of a char. There are no runtime checks that it is complied with and the trust is on the programmer. All library function adhere to this definition. String is a valid DynamicContainer.
     */
    typedef DynamicContainer String;
    typedef DynamicContainer StringW;
    /**
     * @brief Creates a String that contains a copy of string param.
     * If string is a CString it is not length checked with, strlen instead function will copy length bytes.
     * String will then be null termenated.
     * @param string Valid char array or CString
     * @param length Length of array or string
     * @return String that contains a copy of string up to length chars.
     * @return Invalid object if alloc failed.
     */
    extern String            stringCreate(const char* string, size_t length);

    /**
     * @brief Length of string.
     * @param string Pointer to valid String
     * @return Size of char array in bytes, excluding the end null terminator.
     */
    static inline size_t     stringLength(const String* string) {
        const size_t stringSize = containerSize((Container*) string);
        return stringSize ? stringSize - 1 : 0;
    }

    /**
     * @brief Wrapper of containerGet
     * @param string Pointer to valid String
     * @param index Index to get char from
     * @return Pointer to char in array. NULL if invalid index.
     */
    extern inline char*          stringGetChar(const String* string, size_t index);

    /**
     * @brief Appends stringToInsert on to destString.
     * @param destString Destination string object. Must be pointer to valid String/DynamicContainer
     * @param stringToInsert Source CString. Any char array is applicable
     * @param length Length of stringToInsert. This is the only length check
     * @return ContainerAllocFailure if allocation failed for the extra needed space.
     */
    extern inline ContainerError stringAppendCString(String* destString, const char* stringToInsert, size_t length);

    /**
     * @brief Inserts a CString into a String. Wrapper of containerDynamicInsert.
     * @param destString Pointer to valid String
     * @param other Pointer to valid CString
     * @param lenOfOther Length of the CString
     * @param firstIndex Index to insert at
     * @return ContainerInvalidIndex if index was larger than the length of destString.
     * @return ContainerAllocFailure if destString could not grow.
     */
    extern inline ContainerError stringInsertSubCString(String* destString, const char* other, size_t lenOfOther, size_t firstIndex);

    /**
     * @brief Appends stringToInsert on to destString
     * @param destString String to append to. Must be pointer to valid String/DynamicContainer
     * @param stringToInsert String used to append with. Must be pointer to valid String
     * @return ContainerAllocFailure if allocation failed for the extra needed space.
     */
    extern inline ContainerError stringAppendString(String* destString, const String* stringToInsert);

    /**
     * @brief Removes amountToStrip charToStrip chars in string. Result is returned as a copy.
     * @param string Pointer to valid String
     * @param charToStrip Char that should be removed
     * @param stripFromBack Should removal be from back to front
     * @param amountToStrip Max amount of charToStrip to remove. 0 means infinity
     * @return Result String. Check validity with isValidObject.
     */
    extern String                stringStrip(const String* string, char charToStrip, bool stripFromBack, uint64_t amountToStrip);

    /**
     * @brief Splits string on charToSplit up to amountOfCharsToSplitAt times. This is not mutating string
     * @param string Pointer to valid String
     * @param charToSplitOn Char that should be used as a splitting point
     * @param splitFromBack Should the splitting commence from back to front
     * @param amountOfCharsToSplitAt Max amount of times a split can happen. 0 means infinity
     * @return DynamicContainer containing Strings in order of how they were placed in string. Check validity of result with isValidObject. No cleanup needed for invalid result.
     */
    extern DynamicContainer      stringSplit(const String* string, char charToSplitOn, bool splitFromBack, uint64_t amountOfCharsToSplitAt);

    /**
     * @brief Replaces part of string with stringToReplaceWith starting from firstIndex. Unused indexes are considered valid to write to as long as it begins at maximum length of destString.
     * @param destString String which contents are to be replaced. Pointer to valid String
     * @param stringToReplaceWith Pointer to valid String
     * @param firstIndex Index to start replacing at
     * @return ContainerAllocFailure if string cannot grow in the cases were unused indexes are being written to.
     * @return ContainerInvalidIndex if index was larger than the length of destString or if replacement intrupted a UTF-8 char.
     */
    extern ContainerError        stringReplace(String* destString, const String* stringToReplaceWith, size_t firstIndex);
    /**
     * @brief Compares two Strings lexoconographicly
     * @param first Pointer to valid String
     * @param second Pointer to valid String
     * @return true if first is less than or equal to second, else false
     */
    extern bool                  stringCompareAcending(const void* first, const void* second);
    /**
     * @brief Compares two Strings lexoconographicly
     * @param first Pointer to valid String
     * @param second Pointer to valid String
     * @return true if first is greater than or equal to second, else false
     */
    extern bool                  stringCompareDecending(const void* first, const void* second);
    /**
     * @brief Compares the equality of two strings.
     * @param first Pointer to valid String
     * @param second Pointer to valid String
     * @return true if strings are equal, else false.
     */
    extern bool stringEqual(const void* first, const void* second);

    /**
     * @brief Creates a StringW that contains a copy of str param. If str is a CString it is not length checked with strlen, instead function will copy len indicies.
     * @param str Valid wchar_t array or Wide C-String
     * @param len Length of array or string
     * @return StringW that contains a copy of string up to length chars.
     * @return Invalid object if alloc failed.
     */
    extern StringW               stringWCreate(const wchar_t* str, size_t len);



    /**
     * A non owning constant string. Existing so that it can be appended to a string or have read operations that must know the length of it.
     */
    typedef const union StringView {
        struct {
            DataTypeFlags  header;
            const uint32_t byteSizeOfSingleElement;
            const size_t   amountOfIndexes;
            const char*    array;
        };

        Container container;
    } StringView;

    /**
     * A non owning constant string. Existing so that it can be appended to a string or have read operations that must know the length of it.
     */
    typedef const union StringViewW {
        struct {
            DataTypeFlags  header;
            const uint32_t byteSizeOfSingleElement;
            const size_t   amountOfIndexes;
            const wchar_t* array;
        };

        Container container;
    } StringViewW;

/**
 * @param cString Any valid constexpr char[] or literal
 */
#define stringViewInitConstExpr(cString)     {.amountOfIndexes = sizeof(cString), .array = cString, .byteSizeOfSingleElement = 1, .header = ObjectFlagIsValid | ObjectFlagIsContainer}
/**
 * @param wideString Any valid constexpr wchar_t[] or literal
 */
#define stringViewWInitConstExpr(wideString) {.amountOfIndexes = sizeof(wideString), .array = wideString, .byteSizeOfSingleElement = sizeof(wchar_t), .header = ObjectFlagIsValid | ObjectFlagIsContainer}

    /**
     *
     * @param str Any valid C-string
     * @return StringView Object with len counted.
     */
    static inline StringView stringViewInit(const char* str) {
        return (StringView) {
            .amountOfIndexes         = strlen(str),
            .array                   = str,
            .byteSizeOfSingleElement = 1,
            .header                  = ObjectFlagIsValid | ObjectFlagIsContainer};
    }

    /**
     *
     * @param str Any valid Wide C-string
     * @return StringViewW Object with len counted.
     */
    static inline StringViewW stringViewWInit(const wchar_t* str) {
        return (StringViewW) {
            .amountOfIndexes         = wcslen(str),
            .array                   = str,
            .byteSizeOfSingleElement = sizeof(wchar_t),
            .header                  = ObjectFlagIsValid | ObjectFlagIsContainer};
    }


#ifdef __cplusplus
    }
};
#endif // __cplusplus

#endif
