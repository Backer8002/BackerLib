#ifndef BackerString_h_
#define BackerString_h_


#include "Container.h"
#include "DynamicContainer.h"
#include <stddef.h>

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#endif // __cplusplus

    /**
     * @brief String should be treated as a DynamicContainer with element size of a char. There are no runtime checks that it is complied with and the trust is on the programmer. All library function adhere to this definition. String is a valid DynamicContainer.
     */
    typedef DynamicContainer     String;
    /**
     * @brief Creates a String that contains a copy of string param. If string is a CString it is not length checked with strlen instead function will copy length bytes.
     * @param string Valid char array or CString
     * @param length Length of array or string
     * @return String that contains a copy of string up to length chars.
     * @return Invalid object if alloc failed.
     */
    extern String                stringCreate(const char* string, size_t length);
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
     * @return ContainerInvalidIndex if index was larger than the length of destString.
     */
    extern ContainerError        stringReplace(String* destString, const String* stringToReplaceWith, size_t firstIndex);


#ifdef __cplusplus
    }
};
#endif // __cplusplus

#endif