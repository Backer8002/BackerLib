#ifndef BackerString_h_
#define BackerString_h_


#include "BL_Container.h"
#include "BL_DynamicContainer.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <uchar.h>
#include <wchar.h>

#ifdef __cplusplus
    extern "C" {
#else
#define noexcept
#endif // __cplusplus

    /**
     * @brief String should be treated as a DynamicContainer with element size of a char. There are no runtime checks that it is complied with and the trust is on the programmer. All library function adhere to this definition. String is a valid DynamicContainer.
     */
    typedef BL_DynamicContainer BL_String;
    typedef BL_DynamicContainer BL_StringW;

        /**
         * A non owning constant string. Existing so that it can be appended to a string or have read operations that must know the length of it.
         */
        typedef union BL_StringView {
            struct {
                const BL_DataTypeFlags  header;
                const uint32_t byteSizeOfSingleElement;
                const size_t   amountOfIndexes;
                const unsigned char*    array;
            };
            const BL_Container container;
        } BL_StringView;

        /**
         * A non owning constant string. Existing so that it can be appended to a string or have read operations that must know the length of it.
         */
        typedef union BL_StringViewW {
            struct {
                const BL_DataTypeFlags  header;
                const uint32_t byteSizeOfSingleElement;
                const size_t   amountOfIndexes;
                const wchar_t* array;
            };
            const BL_Container container;
        } BL_StringViewW;

    /**
     * @brief Creates a String that contains a copy of string param.
     * If string is a CString it is not length checked with, strlen instead function will copy length bytes.
     * String will then be null termenated.
     * @param string Valid char array or CString
     * @param length Length of array or string
     * @return String that contains a copy of string up to length chars.
     * @return Invalid object if alloc failed.
     */
    extern BL_String            bl_string_create(const unsigned char* string, size_t length) noexcept;
    /**
     * @brief Length of string.
     * @param string Pointer to valid String
     * @return Size of char array in bytes, excluding the end null terminator.
     */
    extern size_t     bl_string_length(const BL_StringView* string) noexcept;
    /**
     * @brief Wrapper of bl_container_get
     * @param string Pointer to valid String
     * @param index Index to get char from
     * @return Pointer to char in array. NULL if invalid index.
     */
    extern unsigned char*          bl_string_get_char(const BL_StringView* string, size_t index) noexcept;
    /**
     * @brief Appends stringToInsert on to destString.
     * @param destString Destination string object. Must be pointer to valid String/DynamicContainer
     * @param stringToInsert Source CString. Any char array is applicable
     * @param length Length of stringToInsert. This is the only length check
     * @return ContainerAllocFailure if allocation failed for the extra needed space.
     */
    extern BL_ContainerError bl_string_append_cstring(BL_String* destString, const unsigned char* stringToInsert, size_t length) noexcept;
    /**
     * @brief Inserts a CString into a String. Wrapper of containerDynamicInsert.
     * @param destString Pointer to valid String
     * @param other Pointer to valid CString
     * @param lenOfOther Length of the CString
     * @param firstIndex Index to insert at
     * @return ContainerInvalidIndex if index was larger than the length of destString.
     * @return ContainerAllocFailure if destString could not grow.
     */
    extern BL_ContainerError bl_string_insert_cstring(BL_String* destString, const unsigned char* other, size_t lenOfOther, size_t firstIndex) noexcept;
    /**
     * @brief Appends stringToInsert on to destString
     * @param destString String to append to. Must be pointer to valid String/DynamicContainer
     * @param stringToInsert String used to append with. Must be pointer to valid String
     * @return ContainerAllocFailure if allocation failed for the extra needed space.
     */
    extern BL_ContainerError bl_string_append_string(BL_String* destString, const BL_StringView* stringToInsert) noexcept;
    /**
     * @brief Removes amountToStrip charToStrip chars in string. Result is returned as a copy.
     * @param string Pointer to valid String
     * @param charToStrip Char that should be removed
     * @param stripFromBack Should removal be from back to front
     * @param amountToStrip Max amount of charToStrip to remove. 0 means infinity
     * @return Result String. Check validity with isValidObject.
     */
    extern BL_String                bl_string_strip(const BL_String* string, unsigned char charToStrip, bool stripFromBack, uint64_t amountToStrip) noexcept;
    /**
     * @brief Splits string on charToSplit up to amountOfCharsToSplitAt times. This is not mutating string
     * @param string Pointer to valid StringView
     * @param charToSplitOn Char that should be used as a splitting point
     * @param splitFromBack Should the splitting commence from back to front
     * @param amountOfCharsToSplitAt Max amount of times a split can happen. 0 means infinity
     * @return DynamicContainer containing Strings in order of how they were placed in string. Check validity of result with isValidObject. No cleanup needed for invalid result.
     */
    extern BL_DynamicContainer      bl_string_split(const BL_StringView* string, unsigned char charToSplitOn, bool splitFromBack, uint64_t amountOfCharsToSplitAt) noexcept;
    /**
     * @brief Splits string on specified chars. Use isValidObject to check validity of return result.
     * @param string Pointer to valid StringView
     * @param charsToSplitOn List of characters to split on
     * @param amountOfChars Length of charsToSplitOn
     * @param splitFromBack Should splitting commence from back to front
     * @param amountOfCharsToSplitAt Max amount of characters to split on. 0, means no limit.
     * @return DynamicContainer containing split Strings in the order they occured in string.
     */
    extern BL_DynamicContainer bl_string_split_multi(const BL_StringView* string, const unsigned char* charsToSplitOn, size_t amountOfChars, bool splitFromBack, uint64_t amountOfCharsToSplitAt) noexcept;
    /**
     * @brief Replaces part of string with stringToReplaceWith starting from firstIndex. Unused indexes are considered valid to write to as long as it begins at maximum length of destString.
     * @param destString String which contents are to be replaced. Pointer to valid String
     * @param stringToReplaceWith Pointer to valid String
     * @param firstIndex Index to start replacing at
     * @return ContainerAllocFailure if string cannot grow in the cases were unused indexes are being written to.
     * @return ContainerInvalidIndex if index was larger than the length of destString or if replacement intrupted a UTF-8 char.
     */
    extern BL_ContainerError        bl_string_replace(BL_String* destString, const BL_String* stringToReplaceWith, size_t firstIndex) noexcept;
    /**
     * @brief Compares two Strings lexoconographicly
     * @param first Pointer to valid String
     * @param second Pointer to valid String
     * @return true if first is less than or equal to second, else false
     */
    extern bool                  bl_string_compare_acending(const void* first, const void* second) noexcept;
    /**
     * @brief Compares two Strings lexoconographicly
     * @param first Pointer to valid String
     * @param second Pointer to valid String
     * @return true if first is greater than or equal to second, else false
     */
    extern bool                  bl_string_compare_decending(const void* first, const void* second) noexcept;
    /**
     * @brief Compares the equality of two strings.
     * @param first Pointer to valid String
     * @param second Pointer to valid String
     * @return true if strings are equal, else false.
     */
    extern bool bl_string_equal(const void* first, const void* second) noexcept;
    /**
     * @brief Reads input from file until EOF or \\n.
     * @returns Invalid string if allocation of string fails.
     */
    extern BL_String bl_getline(FILE* file) noexcept;
    /**
     * @brief Creates a StringW that contains a copy of str param. If str is a CString it is not length checked with strlen, instead function will copy len indicies.
     * @param str Valid wchar_t array or Wide C-String
     * @param len Length of array or string
     * @return StringW that contains a copy of string up to length chars.
     * @return Invalid object if alloc failed.
     */
    extern BL_StringW               bl_string_w_create(const wchar_t* str, size_t len) noexcept;

/**
 * @param cString Any valid constexpr char[] or literal
 */
#define bl_stringview_init_constexpr(cString)     {.amountOfIndexes = sizeof(cString), .array = (const unsigned char*)(cString), .byteSizeOfSingleElement = 1, .header = ObjectFlagIsValid | ObjectFlagIsContainer}
/**
 * @param wideString Any valid constexpr wchar_t[] or literal
 */
#define bl_stringviewW_init_constexpr(wideString) {.amountOfIndexes = sizeof(wideString), .array = (wideString), .byteSizeOfSingleElement = sizeof(wchar_t), .header = ObjectFlagIsValid | ObjectFlagIsContainer}

    /**
     *
     * @param str Any valid C-string
     * @return StringView Object with len counted.
     */
    extern BL_StringView bl_stringview_init(const char* str) noexcept;
    /**
     *
     * @param str Any valid Wide C-string
     * @return StringViewW Object with len counted.
     */
    extern BL_StringViewW bl_stringview_w_init(const wchar_t* str) noexcept;
    extern BL_StringView bl_stringview_cast(BL_String string) noexcept;
    extern BL_StringView* bl_stringview_ptr_cast(const BL_String* string) noexcept;

#ifdef __cplusplus
    }
#else
#undef noexcept
#endif // __cplusplus
#endif
