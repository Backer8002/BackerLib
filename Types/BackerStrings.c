#include "BackerStrings.h"
#include "TypesMain.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <wchar.h>

String stringCreate(const unsigned char* string, size_t length) {
    String allocatedString = containerDynamicCreateStack(length + 1, sizeof(char), false);

    if (!isValidObject(&allocatedString.header))
        return allocatedString;

    memcpy(allocatedString.container.array, string, length);
    allocatedString.container.amountOfIndexes = length;
    containerDynamicAppend(&allocatedString, sizeof (char),&(char){'\0'});
    return allocatedString;
}

inline unsigned char* stringGetChar(StringView* string, size_t index) {
    return containerGet((const Container*) string, index);
}

ContainerError stringAppendCString(String* destString, const unsigned char* stringToInsert, size_t length) {
    return containerDynamicInsert(destString, stringLength((StringView*)(Container*)destString), length, sizeof(*stringToInsert), stringToInsert);
}

ContainerError stringInsertSubCString(String* destString, const unsigned char* other, const size_t lenOfOther, const size_t firstIndex) {
    return containerDynamicInsert(destString, firstIndex, lenOfOther, sizeof(*other), other);
}

ContainerError stringAppendString(String* destString, const String* stringToInsert) {
    ContainerError errorCode = containerDynamicInsertContainer(destString, stringLength((StringView*)(Container*)destString), (Container*) stringToInsert);
    if (errorCode != ContainerOPSuccessful)
        return errorCode;
    containerDynamicPop(destString);
    return ContainerOPSuccessful;
}

String stringStrip(const String* string, unsigned char charToStrip, bool stripFromBack, uint64_t amountToStrip) {
    if (string->container.byteSizeOfSingleElement != sizeof charToStrip) {
        String returnString = {0};
        return returnString;
    }
    String returnString = containerDynamicCreateStack(string->container.amountOfIndexes, sizeof(char), false);
    if (!isValidObject(&returnString.header))
        return returnString;

    size_t amountStripped = 0;
    for (size_t iterator = 0; iterator < stringLength((StringView*)(Container*)string); iterator++) {
        unsigned char* currentChar = stringGetChar((StringView*)(Container*)string, (stripFromBack ? stringLength((StringView*)(Container*)string) - 1 - iterator : iterator));
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

DynamicContainer stringSplit(const StringView* string, unsigned char charToSplitOn, bool splitFromBack, uint64_t amountOfCharsToSplitAt) {
    return stringSplitMulti(string,&charToSplitOn,1,splitFromBack,amountOfCharsToSplitAt);
}

DynamicContainer stringSplitMulti(const StringView* string, const unsigned char* charsToSplitOn, size_t amountOfChars, bool splitFromBack, uint64_t amountOfCharsToSplitAt) {
    DynamicContainer stringArray = containerDynamicCreateStack(0,sizeof(String),false);
    bool charsToSplit[256] = {0};
    for (size_t i = 0; i < amountOfChars; i++)
        charsToSplit[charsToSplitOn[i]] = true;

    size_t amountSplit = 0, beginOfSubString = splitFromBack ? stringLength(string) - 1 : 0;

    for (size_t i = 0; i < stringLength(string); i++) {
        size_t currentIndexInString = splitFromBack ? stringLength(string) - 1 - i : i;
        unsigned char currentChar = *stringGetChar(string, currentIndexInString);
        if (!charsToSplit[currentChar])
            continue;

        if (beginOfSubString == currentIndexInString) {
            beginOfSubString += splitFromBack ? -1 : 1;
            continue;
        }

        String subString = stringCreate(
            stringGetChar(string,splitFromBack ? currentIndexInString + 1 : beginOfSubString),
            splitFromBack ? beginOfSubString - currentIndexInString : currentIndexInString - beginOfSubString);

        if (!isValidObject((DataTypeFlags*)&subString)) {
            containerDynamicDestroyWithElements(&stringArray,containerDestroy);
            return stringArray;
        }

        if (containerDynamicAppend(&stringArray,sizeof subString, &subString) != ContainerOPSuccessful) {
            containerDestroy(&subString);
            containerDynamicDestroyWithElements(&stringArray,containerDestroy);
            return stringArray;
        }

        beginOfSubString = currentIndexInString + (splitFromBack ? -1 : 1);

        if (amountOfCharsToSplitAt && amountSplit++ > amountOfCharsToSplitAt)
            break;
    }

    if ((splitFromBack && beginOfSubString != SIZE_MAX) || (!splitFromBack && beginOfSubString != stringLength(string))) {
        String subString;
        if (splitFromBack)
            subString = stringCreate(stringGetChar(string,0),beginOfSubString + 1);
        else
            subString = stringCreate(stringGetChar(string,beginOfSubString),stringLength(string) - beginOfSubString);
        if (!isValidObject((DataTypeFlags*)&subString)) {
            containerDynamicDestroyWithElements(&stringArray,containerDestroy);
            return stringArray;
        }

        if (containerDynamicAppend(&stringArray,sizeof subString, &subString) != ContainerOPSuccessful) {
            containerDestroy(&subString);
            containerDynamicDestroyWithElements(&stringArray,containerDestroy);
            return stringArray;
        }
    }

    if (splitFromBack)
        containerReverse((Container*)&stringArray);
    return stringArray;
}

ContainerError stringReplace(String* destString, const String* stringToReplaceWith, size_t firstIndex) {
    if (firstIndex > destString->container.amountOfIndexes)
        return ContainerInvalidIndex;
    if (*stringGetChar((StringView*)(Container*)destString,firstIndex) < 0)
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
        unsigned char firstStringChar = *stringGetChar(firstString, i);
        unsigned char secondStringChar = *stringGetChar(secondString, i);
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
        unsigned char firstStringChar = *stringGetChar(firstString, i);
        unsigned char secondStringChar = *stringGetChar(secondString, i);
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
