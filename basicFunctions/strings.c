#include"pch.h"
#include<backerStrings.h>
#include<arrayList.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<assert.h>
#include<stdio.h>


String stringCreate(char* string, size_t length) {
    if (length == 0) {
        String stringAlloced = { .list = NULL,.listType = ListNone};
        return stringAlloced;
    }
    String allocatedString = arrayListCreateStack(length,sizeof(char),ListInt8,false);
    if (allocatedString.list == NULL)
        return allocatedString;
    memcpy_s(allocatedString.list,length,string,length);
    allocatedString.amountOfElements = length;
    return allocatedString;
}

inline char stringGetChar(String* string, size_t index) {
    return *(char*)arrayListElementGetGeneric(string, index);
}

inline void stringRemoveSlice(String* string, size_t firstIndex, size_t lastIndex) {
    arrayListElementRemove(string, firstIndex, lastIndex);
}

String stringGetSlice(String* string, size_t firstIndex, size_t lastIndex) {
    if ((string->listType != ListInt8) | (string->amountOfElements <= lastIndex) | (firstIndex > lastIndex)) {
        String stringAlloced = { .list = NULL, .listType = ListNone };
        return stringAlloced;
    }

    if (firstIndex <= lastIndex) {
        String returnString = arrayListCreateStack(lastIndex - firstIndex + 1,sizeof(char),ListInt8,false); //of by one error
        if (returnString.list == NULL) return returnString;
        returnString.amountOfElements = lastIndex - firstIndex + 1;
        for (size_t iterator = 0; iterator < lastIndex-firstIndex+1; iterator++)
            *((char*)returnString.list + iterator) = stringGetChar(string, iterator+firstIndex);
        return returnString;
    }
    else
    {
        String returnString = arrayListCreateStack(firstIndex - lastIndex + 1,sizeof(char),ListInt8,false); //of by one error
        if (returnString.list == NULL) return returnString;
        returnString.amountOfElements = firstIndex - lastIndex + 1;
        size_t returnIterator = returnString.amountOfElements-1;
        for (size_t iterator = lastIndex; iterator <= firstIndex; iterator++) {
            *((char*)string->list + returnIterator) = stringGetChar(string, iterator);
            returnIterator--;
        }
        return returnString;
    }
}

inline String stringCopy(String* string) {
    return stringGetSlice(string, 0, string->amountOfElements - 1);
}

inline String stringReverse(String* string) {
    return stringGetSlice(string, string->amountOfElements - 1, 0);
}

int stringAdd(String* destString, char* string2,size_t length) {
    if((string2 == NULL)|(length==0)|(destString->listType!=ListInt8)) return -2;
    size_t endOfDestString = destString->amountOfElements;
    destString->amountOfElements += length;
    if (arrayListSizeCheckAdd(destString) == -1) {
        return -1;
    }
    for (size_t iterator = 0; iterator < length; iterator++)
        *((char*)destString->list + endOfDestString + iterator) = *(string2 + iterator);
    return 0;
}

int stringConcat(String* destString,String* secondString) {
    if ((destString->listType != ListInt8) | (secondString->listType != ListInt8)) return -2;
    size_t endOfDestString = destString->amountOfElements;
    destString->amountOfElements += secondString->amountOfElements;
    if (arrayListSizeCheckAdd(destString) == -1) {
        return -1;
    }
    for (size_t iterator = 0; iterator < secondString->amountOfElements; iterator++)
        *((char*)destString->list + endOfDestString + iterator) = *((char*)secondString->list + iterator);
    return 0;
}

String stringStrip(String* string, char charToStrip,bool enforceStripLimit ,int64_t amountToStrip) {
    if (string->listType != ListInt8) {
        String stringAlloced = { .list = NULL, .listType = ListNone };
        return stringAlloced;
    }
    size_t amountOfCharsThatCanBeStripped = 0;
    for (size_t iterator = 0; iterator < string->amountOfElements; iterator++)
        if (stringGetChar(string, iterator) == charToStrip)
            amountOfCharsThatCanBeStripped++;

    if (enforceStripLimit && amountOfCharsThatCanBeStripped > ((amountToStrip < 0) ? -amountToStrip : amountToStrip))
        amountOfCharsThatCanBeStripped = (amountToStrip < 0) ? -amountToStrip : amountToStrip;

    String returnString = arrayListCreateStack(string->amountOfElements-amountOfCharsThatCanBeStripped,sizeof(char),ListInt8,false);
    if (returnString.listType == ListNone)
        return returnString;


    size_t amountStripped = 0;
    if ((amountToStrip < 0) && enforceStripLimit) {
        for (size_t iterator = string->amountOfElements-1; iterator != SIZE_MAX; iterator--)
        {
            char currentChar = stringGetChar(string, iterator);
            if ((currentChar != charToStrip) || (amountStripped >= amountOfCharsThatCanBeStripped))
                *((char*)returnString.list + iterator - amountOfCharsThatCanBeStripped + amountStripped) = currentChar;
            else
                amountStripped++;
        }
    }
    else
    {
        for (size_t iterator = 0; iterator < string->amountOfElements; iterator++)
        {
            char currentChar = stringGetChar(string, iterator);
            if ((currentChar != charToStrip) || (enforceStripLimit && (amountStripped >= amountOfCharsThatCanBeStripped)))
                *((char*)returnString.list + iterator - amountStripped) = currentChar;
            else
                amountStripped++;
        }
    }
    returnString.amountOfElements = string->amountOfElements - amountStripped;
    return returnString;
}

ArrayList stringSplit(String* string, char charToSplitOn,bool enforceSplitLimit, int64_t amountOfCharsToSplitAt) {
    if (string->listType != ListInt8) {
        String stringAlloced = { .list = NULL, .listType = ListNone };
        return stringAlloced;
    }
    size_t amountOfStringsThatCanBeSplit = 0;
    if (enforceSplitLimit) {
        for (size_t iterator = 0; iterator < string->amountOfElements; iterator++)
            if (stringGetChar(string, iterator) == charToSplitOn)
                amountOfStringsThatCanBeSplit++;

        if (amountOfStringsThatCanBeSplit > ((amountOfCharsToSplitAt < 0) ? -amountOfCharsToSplitAt : amountOfCharsToSplitAt))
            amountOfStringsThatCanBeSplit = (amountOfCharsToSplitAt < 0) ? -amountOfCharsToSplitAt : amountOfCharsToSplitAt;
    }
    ArrayList stringArray = arrayListCreateStack(amountOfStringsThatCanBeSplit+1,sizeof(String),ListString,false);
    if (stringArray.list == NULL)
        return stringArray;

    size_t amountSplit = 0;
    if (amountOfCharsToSplitAt<0 && enforceSplitLimit) 
    {
        size_t firstIndexOfSubString = 0;
        size_t lastIndexOfSubString = string->amountOfElements-1;
        for (size_t iterator = string->amountOfElements-1; iterator != SIZE_MAX; iterator--)
        {
            char currentChar = stringGetChar(string, iterator);
            if (currentChar != charToSplitOn && iterator != 0)
                continue;

            if (amountOfStringsThatCanBeSplit <= amountSplit)
            {
                firstIndexOfSubString = 0;
                String subString = stringGetSlice(string, firstIndexOfSubString, lastIndexOfSubString);
                if (subString.list == NULL) goto stringSplitErrorExit;
                arrayListElementSetGeneric(&stringArray, stringArray.amountOfElements, &subString,sizeof(String));
                break;
            }

            if ((iterator == 0 && currentChar != charToSplitOn))
                firstIndexOfSubString = iterator;
            else
                firstIndexOfSubString = iterator + 1;
            amountSplit++;
            if (firstIndexOfSubString <= lastIndexOfSubString) {
                String subString = stringGetSlice(string, firstIndexOfSubString, lastIndexOfSubString);
                if (subString.list == NULL) goto stringSplitErrorExit;
                arrayListElementSetGeneric(&stringArray, stringArray.amountOfElements, &subString,sizeof(String));
            }
            lastIndexOfSubString = iterator - 1;
        }
    }
    else
    {
        size_t firstIndexOfSubString = 0;
        size_t lastIndexOfSubString = 0;
        for (size_t iterator = 0; iterator < string->amountOfElements; iterator++)
        {
            char currentChar = stringGetChar(string, iterator);
            if (currentChar != charToSplitOn && iterator != string->amountOfElements - 1)
                continue;
            if (amountOfStringsThatCanBeSplit <= amountSplit && enforceSplitLimit)
            {
                lastIndexOfSubString = string->amountOfElements-1;
                String subString = stringGetSlice(string, firstIndexOfSubString, lastIndexOfSubString);
                if (subString.list == NULL) goto stringSplitErrorExit;
                arrayListElementSetGeneric(&stringArray, stringArray.amountOfElements, &subString,sizeof(String));
                break;
            }


            if ((iterator == string->amountOfElements-1 && currentChar != charToSplitOn))
                lastIndexOfSubString = iterator;
            else
                lastIndexOfSubString = iterator - 1;
            amountSplit++;
            if (firstIndexOfSubString <= lastIndexOfSubString && iterator != 0) {
                String subString = stringGetSlice(string, firstIndexOfSubString, lastIndexOfSubString);
                if (subString.list == NULL)
                    goto stringSplitErrorExit;
                arrayListElementSetGeneric(&stringArray, stringArray.amountOfElements, &subString, sizeof(String));
            }
            firstIndexOfSubString = iterator + 1;
        }
    }

    return stringArray;
stringSplitErrorExit:

    arrayListDestroyWithElements(&stringArray,arrayListDestroy);
    return stringArray;
}