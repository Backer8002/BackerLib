#include "BackerStrings.h"
#include "TypesMain.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>
#include <uchar.h>
#include <string.h>

String stringCreate(const char* string, size_t length) {
    String allocatedString = containerDynamicCreateStack(length, sizeof(char), false);

    if (!isValidObject(&allocatedString.header))
        return allocatedString;

    memcpy(allocatedString.container.array, string, length);
    allocatedString.container.amountOfIndexes = length;
    return allocatedString;
}

inline char* stringGetChar(const String* string, size_t index) {
    return containerGet((const Container*) string, index);
}

inline ContainerError stringAppendCString(String* destString, const char* stringToInsert, size_t length) {
    return containerDynamicInsert(destString, destString->container.amountOfIndexes, length, sizeof(*stringToInsert), stringToInsert);
}

ContainerError stringInsertSubCString(String* destString, const char* other, size_t lenOfOther, size_t firstIndex) {
    return containerDynamicInsert(destString, firstIndex, lenOfOther, sizeof(*other), other);
}

inline ContainerError stringAppendString(String* destString, const String* stringToInsert) {
    return containerDynamicInsertContainer(destString, destString->container.amountOfIndexes, (Container*) stringToInsert);
}

String stringStrip(const String* string, char charToStrip, bool stripFromBack, uint64_t amountToStrip) {
    if (string->container.byteSizeOfSingleElement != sizeof charToStrip) {
        String returnString = {0};
        return returnString;
    }
    String returnString = containerDynamicCreateStack(string->container.amountOfIndexes, sizeof(char), false);
    if (!isValidObject(&returnString.header))
        return returnString;

    size_t amountStripped = 0;
    for (size_t iterator = 0; iterator < string->container.amountOfIndexes; iterator++) {
        char* currentChar = stringGetChar(string, (stripFromBack ? string->container.amountOfIndexes - 1 - iterator : iterator));
        if ((*currentChar != charToStrip) || (amountToStrip && (amountStripped >= amountToStrip)))
            containerDynamicAppend(&returnString, sizeof(*currentChar), currentChar);
        else
            amountStripped++;
    }
    if (stripFromBack)
        containerReverse(&returnString.container);
    return returnString;
}

DynamicContainer stringSplit(const String* string, char charToSplitOn, bool splitFromBack, uint64_t amountOfCharsToSplitAt) {
    DynamicContainer stringArray = containerDynamicCreateStack(0, sizeof(String), false);
    if (!isValidObject(&stringArray.header))
        return stringArray;

    size_t amountSplit           = 0;
    size_t firstIndexOfSubString = 0;
    size_t lastIndexOfSubString  = string->container.amountOfIndexes - 1;

    for (size_t iterator = 0; iterator < string->container.amountOfIndexes; iterator++) {
        char* currentChar = stringGetChar(string, (splitFromBack ? string->container.amountOfIndexes - 1 - iterator : iterator));
        if (*currentChar != charToSplitOn)
            continue;
        if (amountOfCharsToSplitAt && amountOfCharsToSplitAt <= amountSplit)
            break;

        amountSplit++;

        if (splitFromBack) {
            firstIndexOfSubString = string->container.amountOfIndexes - iterator;
        } else {
            lastIndexOfSubString = iterator - 1;
            if (iterator == 0) {
                lastIndexOfSubString = 0;
                firstIndexOfSubString = 1;
            }
        }

        if (firstIndexOfSubString <= lastIndexOfSubString) {
            DynamicContainer subString = containerConvertToDynamicStack(containerGetSubArray((Container*) string, firstIndexOfSubString, lastIndexOfSubString, false));
            if (containerDynamicAppend(&stringArray, sizeof(String), &subString) == ContainerAllocFailure)
                goto stringSplitErrorExit;
        }
        if (string->container.amountOfIndexes - iterator == 1)
            goto stringSplitExit;

        if (splitFromBack)
            lastIndexOfSubString = string->container.amountOfIndexes - iterator - 2;
        else
            firstIndexOfSubString = iterator + 1;
    }

    if (splitFromBack)
        firstIndexOfSubString = 0;
    else
        lastIndexOfSubString = string->container.amountOfIndexes - 1;

    DynamicContainer subString = containerConvertToDynamicStack(containerGetSubArray((Container*) string, firstIndexOfSubString, lastIndexOfSubString, false));
    if (containerDynamicAppend(&stringArray, sizeof(String), &subString) == ContainerAllocFailure)
        goto stringSplitErrorExit;

stringSplitExit:
    if (splitFromBack)
        containerReverse(&stringArray.container);
    return stringArray;

stringSplitErrorExit:
    containerDynamicDestroyWithElements(&stringArray, containerDestroy);
    return stringArray;
}

ContainerError stringReplace(String* destString, const String* stringToReplaceWith, size_t firstIndex) {
    if (firstIndex > destString->container.amountOfIndexes)
        return ContainerInvalidIndex;
    if (stringToReplaceWith->container.amountOfIndexes > destString->container.amountOfIndexes - firstIndex - 1) {
        if (containerDynamicReserve(destString, stringToReplaceWith->container.amountOfIndexes - destString->container.amountOfIndexes + firstIndex + 1) == ContainerAllocFailure)
            return ContainerAllocFailure;
        destString->container.amountOfIndexes = stringToReplaceWith->container.amountOfIndexes + firstIndex + 1;
    }
    for (size_t i = 0; i < stringToReplaceWith->container.amountOfIndexes; i++)
        containerSet((Container*) destString, i + firstIndex, sizeof(char), containerGet((const Container*) stringToReplaceWith, i));
    return ContainerOPSuccessful;
}

StringW stringWCreate(const wchar_t* str, size_t len) {
    StringW allocatedString = containerDynamicCreateStack(len, sizeof(wchar_t), false);

    if (!isValidObject(&allocatedString.header))
        return allocatedString;

    memcpy(allocatedString.container.array, str, len * sizeof(wchar_t));
    allocatedString.container.amountOfIndexes = len;
    return allocatedString;
}

StringUTF8 stringUTF8Create(const char32_t* str, size_t len) {
    StringUTF8 allocatedString = containerDynamicCreateStack(len, sizeof *str, false);

    if (!isValidObject(&allocatedString.header))
        return allocatedString;

    memcpy(allocatedString.container.array, str, len * sizeof *str);
    allocatedString.container.amountOfIndexes = len;
    return allocatedString;
}