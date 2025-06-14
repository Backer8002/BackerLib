#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<assert.h>
#include<stdio.h>
#include<backerStrings.h>

String stringCreate(const char* string, size_t length) {
    if (length == 0) {
        String stringAlloced = {{.list = NULL, .header.dataArrayVarType = ListNone}};
        return stringAlloced;
    }
    String allocatedString            = {arrayListCreateStack(length, sizeof(char), ListInt8, false)};
    allocatedString.arrayList.header.objectType = ListString;
    if (allocatedString.arrayList.list == NULL)
        return allocatedString;
    memcpy(allocatedString.arrayList.list,string,length);
    allocatedString.arrayList.amountOfElements = length;
    return allocatedString;
}

inline char stringGetChar(const String* string, size_t index) {
    char* element = arrayListElementGet((const ArrayList*)string, index);
    if (element != NULL)
        return *element;
    assert(false);
    return 0;
}

inline void stringRemoveSlice(String* string, size_t firstIndex, size_t lastIndex) {
    arrayListElementRemove((ArrayList*)string, firstIndex, lastIndex);
}

String stringGetSlice(const String* string, size_t firstIndex, size_t lastIndex) {
    if ((string->arrayList.header.dataArrayVarType != ListInt8) | (string->arrayList.amountOfElements <= lastIndex) | (firstIndex > lastIndex)) {
        String stringAlloced = {{.list = NULL, .header.dataArrayVarType = ListNone}};
        return stringAlloced;
    }

    if (firstIndex <= lastIndex) {
        String returnString = {arrayListCreateStack(lastIndex - firstIndex + 1, sizeof(char), ListInt8, false)}; // of by one error
        if (returnString.arrayList.list == NULL) return returnString;
        returnString.arrayList.amountOfElements = lastIndex - firstIndex + 1;
        for (size_t iterator = 0; iterator < lastIndex-firstIndex+1; iterator++)
            *((char*)returnString.arrayList.list + iterator) = stringGetChar(string, iterator+firstIndex);
        return returnString;
    }
    else
    {
        String returnString = {arrayListCreateStack(firstIndex - lastIndex + 1, sizeof(char), ListInt8, false)}; // of by one error
        if (returnString.arrayList.list == NULL) return returnString;
        returnString.arrayList.amountOfElements = firstIndex - lastIndex + 1;
        size_t returnIterator = returnString.arrayList.amountOfElements-1;
        for (size_t iterator = lastIndex; iterator <= firstIndex; iterator++) {
            *((char*)string->arrayList.list + returnIterator) = stringGetChar(string, iterator);
            returnIterator--;
        }
        return returnString;
    }
}

inline String stringCopy(const String* string) {
    return stringGetSlice(string, 0, ((const ArrayList*)string)->amountOfElements - 1);
}

inline String stringReverse(const String* string) {
    return stringGetSlice(string, ((const ArrayList*)string)->amountOfElements - 1, 0);
}

int stringAdd(String* destString, const char* string2,size_t length) {
    if((string2 == NULL)|(length==0)|(destString->arrayList.header.dataArrayVarType!=ListInt8)) return -2;
    size_t endOfDestString = destString->arrayList.amountOfElements;
    destString->arrayList.amountOfElements += length;
    if (arrayListSizeCheckAdd((ArrayList*)destString) == -1) {
        return -1;
    }
    for (size_t iterator = 0; iterator < length; iterator++)
        *((char*)destString->arrayList.list + endOfDestString + iterator) = *(string2 + iterator);
    return 0;
}

int stringConcat(String* destString,const String* secondString) {
    if ((destString->arrayList.header.dataArrayVarType != ListInt8) | (secondString->arrayList.header.dataArrayVarType != ListInt8)) return -2;
    size_t endOfDestString = destString->arrayList.amountOfElements;
    destString->arrayList.amountOfElements += secondString->arrayList.amountOfElements;
    if (arrayListSizeCheckAdd((ArrayList*)destString) == -1) {
        return -1;
    }
    for (size_t iterator = 0; iterator < secondString->arrayList.amountOfElements; iterator++)
        *((char*)destString->arrayList.list + endOfDestString + iterator) = *((char*)secondString->arrayList.list + iterator);
    return 0;
}

String stringStrip(const String* string, char charToStrip,bool enforceStripLimit ,int64_t amountToStrip) {
    if (string->arrayList.header.dataArrayVarType != ListInt8) {
        String stringAlloced = {{.list = NULL, .header.dataArrayVarType = ListNone}};
        return stringAlloced;
    }
    size_t amountOfCharsThatCanBeStripped = 0;
    for (size_t iterator = 0; iterator < string->arrayList.amountOfElements; iterator++)
        if (stringGetChar(string, iterator) == charToStrip)
            amountOfCharsThatCanBeStripped++;

    if (enforceStripLimit && amountOfCharsThatCanBeStripped > (size_t)((amountToStrip < 0) ? -amountToStrip : amountToStrip))
        amountOfCharsThatCanBeStripped = (amountToStrip < 0) ? -amountToStrip : amountToStrip;

    String returnString = {arrayListCreateStack(string->arrayList.amountOfElements - amountOfCharsThatCanBeStripped, sizeof(char), ListInt8, false)};
    if (returnString.arrayList.header.dataArrayVarType == ListNone)
        return returnString;


    size_t amountStripped = 0;
    if ((amountToStrip < 0) && enforceStripLimit) {
        for (size_t iterator = string->arrayList.amountOfElements-1; iterator != SIZE_MAX; iterator--)
        {
            char currentChar = stringGetChar(string, iterator);
            if ((currentChar != charToStrip) || (amountStripped >= amountOfCharsThatCanBeStripped))
                *((char*)returnString.arrayList.list + iterator - amountOfCharsThatCanBeStripped + amountStripped) = currentChar;
            else
                amountStripped++;
        }
    }
    else
    {
        for (size_t iterator = 0; iterator < string->arrayList.amountOfElements; iterator++)
        {
            char currentChar = stringGetChar(string, iterator);
            if ((currentChar != charToStrip) || (enforceStripLimit && (amountStripped >= amountOfCharsThatCanBeStripped)))
                *((char*)returnString.arrayList.list + iterator - amountStripped) = currentChar;
            else
                amountStripped++;
        }
    }
    returnString.arrayList.amountOfElements = string->arrayList.amountOfElements - amountStripped;
    return returnString;
}

ArrayList stringSplit(const String* string, char charToSplitOn,bool enforceSplitLimit, int64_t amountOfCharsToSplitAt) {
    if (string->arrayList.header.dataArrayVarType != ListInt8) {
        ArrayList stringAlloced = {.list = NULL, .header.dataArrayVarType = ListNone};
        return stringAlloced;
    }
    size_t amountOfStringsThatCanBeSplit = 0;
    if (enforceSplitLimit) {
        for (size_t iterator = 0; iterator < string->arrayList.amountOfElements; iterator++)
            if (stringGetChar(string, iterator) == charToSplitOn)
                amountOfStringsThatCanBeSplit++;

        if (amountOfStringsThatCanBeSplit > (size_t)((amountOfCharsToSplitAt < 0) ? -amountOfCharsToSplitAt : amountOfCharsToSplitAt))
            amountOfStringsThatCanBeSplit = (amountOfCharsToSplitAt < 0) ? -amountOfCharsToSplitAt : amountOfCharsToSplitAt;
    }
    ArrayList stringArray = arrayListCreateStack(amountOfStringsThatCanBeSplit+1,sizeof(String),ListString,false);
    if (stringArray.list == NULL)
        return stringArray;

    size_t amountSplit = 0;
    if (amountOfCharsToSplitAt<0 && enforceSplitLimit) 
    {
        size_t firstIndexOfSubString = 0;
        size_t lastIndexOfSubString = string->arrayList.amountOfElements-1;
        for (size_t iterator = string->arrayList.amountOfElements-1; iterator != SIZE_MAX; iterator--)
        {
            char currentChar = stringGetChar(string, iterator);
            if (currentChar != charToSplitOn && iterator != 0)
                continue;

            if (amountOfStringsThatCanBeSplit <= amountSplit)
            {
                firstIndexOfSubString = 0;
                String subString = stringGetSlice(string, firstIndexOfSubString, lastIndexOfSubString);
                if (subString.arrayList.list == NULL) goto stringSplitErrorExit;
                arrayListElementSet(&stringArray, stringArray.amountOfElements, &subString,ListInt8);
                break;
            }

            if ((iterator == 0 && currentChar != charToSplitOn))
                firstIndexOfSubString = iterator;
            else
                firstIndexOfSubString = iterator + 1;
            amountSplit++;
            if (firstIndexOfSubString <= lastIndexOfSubString) {
                String subString = stringGetSlice(string, firstIndexOfSubString, lastIndexOfSubString);
                if (subString.arrayList.list == NULL) goto stringSplitErrorExit;
                arrayListElementSet(&stringArray, stringArray.amountOfElements, &subString,ListInt8);
            }
            lastIndexOfSubString = iterator - 1;
        }
    }
    else
    {
        size_t firstIndexOfSubString = 0;
        size_t lastIndexOfSubString = 0;
        for (size_t iterator = 0; iterator < string->arrayList.amountOfElements; iterator++)
        {
            char currentChar = stringGetChar(string, iterator);
            if (currentChar != charToSplitOn && iterator != string->arrayList.amountOfElements - 1)
                continue;
            if (amountOfStringsThatCanBeSplit <= amountSplit && enforceSplitLimit)
            {
                lastIndexOfSubString = string->arrayList.amountOfElements-1;
                String subString = stringGetSlice(string, firstIndexOfSubString, lastIndexOfSubString);
                if (subString.arrayList.list == NULL) goto stringSplitErrorExit;
                arrayListElementSet(&stringArray, stringArray.amountOfElements, &subString,ListInt8);
                break;
            }


            if ((iterator == string->arrayList.amountOfElements-1 && currentChar != charToSplitOn))
                lastIndexOfSubString = iterator;
            else
                lastIndexOfSubString = iterator - 1;
            amountSplit++;
            if (firstIndexOfSubString <= lastIndexOfSubString && iterator != 0) {
                String subString = stringGetSlice(string, firstIndexOfSubString, lastIndexOfSubString);
                if (subString.arrayList.list == NULL)
                    goto stringSplitErrorExit;
                arrayListElementSet(&stringArray, stringArray.amountOfElements, &subString,ListInt8);
            }
            firstIndexOfSubString = iterator + 1;
        }
    }

    return stringArray;
stringSplitErrorExit:

    arrayListDestroyWithElements(&stringArray,arrayListDestroy);
    return stringArray;
}