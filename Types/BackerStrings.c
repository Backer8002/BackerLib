#include "BackerStrings.h"
#include "TypesMain.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <wchar.h>

String stringCreate(const char* string, size_t length) {
    String allocatedString = containerDynamicCreateStack(length + 1, sizeof(char), false);

    if (!isValidObject(&allocatedString.header))
        return allocatedString;

    memcpy(allocatedString.container.array, string, length);
    allocatedString.container.amountOfIndexes = length;
    containerDynamicAppend(&allocatedString, sizeof (char),&(char){'\0'});
    return allocatedString;
}

inline char* stringGetChar(StringView* string, size_t index) {
    return containerGet((const Container*) string, index);
}

inline ContainerError stringAppendCString(String* destString, const char* stringToInsert, size_t length) {
    return containerDynamicInsert(destString, stringLength(destString), length, sizeof(*stringToInsert), stringToInsert);
}

ContainerError stringInsertSubCString(String* destString, const char* other, const size_t lenOfOther, const size_t firstIndex) {
    return containerDynamicInsert(destString, firstIndex, lenOfOther, sizeof(*other), other);
}

ContainerError stringAppendString(String* destString, const String* stringToInsert) {
    ContainerError errorCode = containerDynamicInsertContainer(destString, stringLength(destString), (Container*) stringToInsert);
    if (errorCode != ContainerOPSuccessful)
        return errorCode;
    containerDynamicPop(destString);
    return ContainerOPSuccessful;
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
    for (size_t iterator = 0; iterator < stringLength(string); iterator++) {
        char* currentChar = stringGetChar(string, (stripFromBack ? stringLength(string) - 1 - iterator : iterator));
        if ((*currentChar != charToStrip) || (amountToStrip && (amountStripped >= amountToStrip)))
            containerDynamicAppend(&returnString, sizeof(*currentChar), currentChar);
        else
            amountStripped++;
    }
    if (stripFromBack)
        containerReverse(&returnString.container);
    containerDynamicAppend(&returnString,sizeof(char),&(char){'\0'});
    return returnString;
}

DynamicContainer stringSplit(const String* string, char charToSplitOn, bool splitFromBack, uint64_t amountOfCharsToSplitAt) {
    DynamicContainer stringArray = containerDynamicCreateStack(0, sizeof(String), false);
    if (!isValidObject(&stringArray.header))
        return stringArray;

    size_t amountSplit           = 0;
    size_t firstIndexOfSubString = 0;
    size_t lastIndexOfSubString  = stringLength(string) - 1;

    for (size_t iterator = 0; iterator < stringLength(string); iterator++) {
        const char* currentChar = stringGetChar(string, splitFromBack ? stringLength(string) - 1 - iterator : iterator);
        if (*currentChar != charToSplitOn)
            continue;
        if (amountOfCharsToSplitAt && amountOfCharsToSplitAt <= amountSplit)
            break;

        amountSplit++;

        if (splitFromBack) {
            firstIndexOfSubString = stringLength(string) - iterator;
        } else {
            lastIndexOfSubString = iterator - 1;
            if (iterator == 0) {
                lastIndexOfSubString = 0;
                firstIndexOfSubString = 1;
            }
        }

        if (firstIndexOfSubString <= lastIndexOfSubString) {
            String subString = containerConvertToDynamicStack(containerGetSubArray((Container*) string, firstIndexOfSubString, lastIndexOfSubString, false));
            if (!isValidObject(&subString.header))
                goto stringSplitErrorExit;
            if (stringAppendCString(&subString, "\0",1) != ContainerOPSuccessful)
                goto stringSplitErrorExit;
            if (containerDynamicAppend(&stringArray, sizeof(String), &subString) == ContainerAllocFailure)
                goto stringSplitErrorExit;
        }
        if (stringLength(string) - iterator == 1)
            goto stringSplitExit;

        if (splitFromBack)
            lastIndexOfSubString = stringLength(string) - iterator - 2;
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
    if (*stringGetChar(destString,firstIndex) > 127)
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

bool stringCompareAcending(const void* first, const void* second) {
    StringView* firstString = first;
    StringView* secondString = second;

    for (size_t i = 0; i < stringLength(firstString); i++) {
        if (i>=stringLength(secondString))
            return false;
        bool firstIsUpper = false;
        bool secondIsUpper = false;
        char firstStringChar = *stringGetChar(firstString, i);
        char secondStringChar = *stringGetChar(secondString, i);
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

bool stringCompareDecending(const void* first, const void* second) {
    StringView* firstString = first;
    StringView* secondString = second;

    for (size_t i = 0; i < stringLength(firstString); i++) {
        if (i>=stringLength(secondString))
            return true;
        bool firstIsUpper = false;
        bool secondIsUpper = false;
        char firstStringChar = *stringGetChar(firstString, i);
        char secondStringChar = *stringGetChar(secondString, i);
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

bool stringEqual(const void* first, const void* second) {
    StringView* firstString = first;
    StringView* secondString = second;

    if (stringLength(firstString) != stringLength(secondString))
        return false;

    return memcmp(firstString->array,secondString->array,stringLength(firstString)) == 0;
}


StringW stringWCreate(const wchar_t* str, size_t len) {
    StringW allocatedString = containerDynamicCreateStack(len+1, sizeof(wchar_t), false);

    if (!isValidObject(&allocatedString.header))
        return allocatedString;

    memcpy(allocatedString.container.array, str, len * sizeof(wchar_t));
    allocatedString.container.amountOfIndexes = len;
    containerDynamicAppend(&allocatedString, sizeof(wchar_t),&(wchar_t){L'\0'});
    return allocatedString;
}
