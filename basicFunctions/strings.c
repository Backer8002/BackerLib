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
    if (length == 0) return NULL;
    String allocatedString = arrayListCreateChar(length);
    if (allocatedString == NULL) return NULL;

    memcpy_s(allocatedString->list,length,string,length);
    allocatedString->amountOfElements = length;
    return allocatedString;
}

inline void stringDestroy(String string) {
    arrayListDestroy(string);
}

inline char stringGetChar(String string, size_t index) {
    return *(char*)arrayListElementGetGeneric(string, index);
}

inline void stringRemoveSlice(String string, size_t firstIndex, size_t lastIndex) {
    arrayListElementRemove(string, firstIndex, lastIndex);
}

String stringGetSlice(String string, size_t firstIndex, size_t lastIndex) {
    if ((string->listType != Char)|(string->amountOfElements<= lastIndex)|(firstIndex>lastIndex)) 
        return NULL;
    if (firstIndex <= lastIndex) {
        String returnString = arrayListCreateChar(lastIndex - firstIndex + 1); //of by one error
        if (returnString == NULL) return NULL;
        returnString->amountOfElements = lastIndex - firstIndex + 1;
        for (size_t iterator = 0; iterator < lastIndex-firstIndex+1; iterator++)
            *((char*)returnString->list + iterator) = stringGetChar(string, iterator+firstIndex);
        fwrite(returnString->list,1,returnString->amountOfElements,stderr);
        printf("\n");
        return returnString;
    }
    else
    {
        String returnString = arrayListCreateChar(firstIndex - lastIndex + 1); //of by one error
        if (returnString == NULL) return NULL;
        returnString->amountOfElements = firstIndex - lastIndex + 1;
        size_t returnIterator = returnString->amountOfElements-1;
        for (size_t iterator = lastIndex; iterator <= firstIndex; iterator++) {
            *((char*)string->list + returnIterator) = stringGetChar(string, iterator);
            returnIterator--;
        }
        return returnString;
    }
}

int stringAdd(String destString, char* string2,size_t length) {
    if((string2 == NULL)|(length==0)|(destString->listType!=Char)) return -2;
    size_t endOfDestString = destString->amountOfElements;
    destString->amountOfElements += length;
    if(arrayListSizeCheckAdd(destString)==-1)
        return -1;
    for (size_t iterator = 0; iterator < length; iterator++)
        *((char*)destString->list + endOfDestString + iterator) = *(string2 + iterator);
    return 0;
}

int stringConcat(String destString,String secondString) {
    if ((destString->listType != Char) | (secondString->listType != Char)) return -2;
    size_t endOfDestString = destString->amountOfElements;
    destString->amountOfElements += secondString->amountOfElements;
    if (arrayListSizeCheckAdd(destString) == -1)
        return -1;
    for (size_t iterator = 0; iterator < secondString->amountOfElements; iterator++)
        *((char*)destString->list + endOfDestString + iterator) = *((char*)secondString->list + iterator);
    return 0;
}

String stringStrip(String string, char charToStrip,bool enforceStripLimit ,int64_t amountToStrip) {
    if (string->listType != Char) 
        return NULL;
    size_t amountOfCharsThatCanBeStripped = 0;
    for (size_t iterator = 0; iterator < string->amountOfElements; iterator++)
        if (stringGetChar(string, iterator) == charToStrip)
            amountOfCharsThatCanBeStripped++;

    if ((amountOfCharsThatCanBeStripped > (amountToStrip<0) ? amountToStrip*-1 : amountToStrip) & (enforceStripLimit))
        amountOfCharsThatCanBeStripped = (amountToStrip < 0) ? amountToStrip*-1 : amountToStrip;

    String returnString = arrayListCreateChar(string->amountOfElements-amountOfCharsThatCanBeStripped);
    if (returnString == NULL)
        return NULL;


    size_t amountStripped = 0;
    if ((amountToStrip < 0) & enforceStripLimit) {
        for (size_t iterator = string->amountOfElements; iterator != SIZE_MAX; iterator--)
        {
            char currentChar = stringGetChar(string, iterator);
            if ((currentChar != charToStrip) | ((amountStripped >= amountOfCharsThatCanBeStripped) & !enforceStripLimit))
                *((char*)returnString->list + iterator - amountOfCharsThatCanBeStripped + amountStripped) = currentChar;
            else
                amountStripped++;
        }
    }
    else
    {
        for (size_t iterator = 0; iterator < string->amountOfElements; iterator++)
        {
            char currentChar = stringGetChar(string, iterator);
            if ((currentChar != charToStrip) | (amountStripped >= amountOfCharsThatCanBeStripped))
                *((char*)returnString->list + iterator - amountStripped) = currentChar;
            else
                amountStripped++;
        }
    }
    return returnString;
}

ArrayList* stringSplit(String string, char charToSplitOn,bool enforceSplitLimit, int64_t amountOfCharsToSplitAt) {
    if (string->listType != Char)
        return NULL;
    size_t amountOfStringsThatCanBeSplit = 0;
    for (size_t iterator = 0; iterator < string->amountOfElements; iterator++)
        if (stringGetChar(string, iterator) == charToSplitOn)
            amountOfStringsThatCanBeSplit++;

    if ((amountOfStringsThatCanBeSplit > (amountOfCharsToSplitAt < 0) ? amountOfCharsToSplitAt * -1 : amountOfCharsToSplitAt) & (enforceSplitLimit))
        amountOfStringsThatCanBeSplit = (amountOfCharsToSplitAt < 0) ? amountOfCharsToSplitAt * -1 : amountOfCharsToSplitAt;

    ArrayList* stringArray = arrayListCreateString(amountOfStringsThatCanBeSplit+1);
    if (stringArray == NULL)
        return NULL;

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

            if (amountOfStringsThatCanBeSplit <= amountSplit && enforceSplitLimit)
            {
                firstIndexOfSubString = 0;
                String subString = stringGetSlice(string, firstIndexOfSubString, lastIndexOfSubString);
                if (subString == NULL) goto stringSplitErrorExit;
                arrayListElementSetString(stringArray, stringArray->amountOfElements, subString);
                break;
            }

            if (iterator == 0 && currentChar != charToSplitOn)
                firstIndexOfSubString = 0;
            else
                firstIndexOfSubString = iterator + 1;
            amountSplit++;
            if (firstIndexOfSubString <= lastIndexOfSubString) {
                String subString = stringGetSlice(string, firstIndexOfSubString, lastIndexOfSubString);
                if (subString == NULL) goto stringSplitErrorExit;
                arrayListElementSetString(stringArray, stringArray->amountOfElements, subString);
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
            if (amountOfStringsThatCanBeSplit <= amountSplit && enforceSplitLimit)
            {
                lastIndexOfSubString = string->amountOfElements-1;
                String subString = stringGetSlice(string, firstIndexOfSubString, lastIndexOfSubString);
                if (subString == NULL) goto stringSplitErrorExit;
                arrayListElementSetString(stringArray, stringArray->amountOfElements, subString);
                break;
            }
            if (currentChar != charToSplitOn && iterator != string->amountOfElements-1)
                continue;


            if (iterator == string->amountOfElements-1 && currentChar != charToSplitOn)
                lastIndexOfSubString = iterator;
            else
                lastIndexOfSubString = iterator - 1;
            amountSplit++;
            if (firstIndexOfSubString <= lastIndexOfSubString) {
                String subString = stringGetSlice(string, firstIndexOfSubString, lastIndexOfSubString);
                if (subString == NULL) goto stringSplitErrorExit;
                arrayListElementSetString(stringArray, stringArray->amountOfElements, subString);
            }
            firstIndexOfSubString = iterator + 1;
        }
    }

    return stringArray;

stringSplitErrorExit:
    for (size_t iterator = 0; iterator < stringArray->amountOfElements; iterator++)
        stringDestroy(arrayListElementGetString(stringArray, iterator));
    arrayListDestroy(stringArray);
    return NULL;
}

//ArrayListStuff

int arrayListElementSetString(ArrayList* arrayList,size_t index, String value) {
    assert(index <= arrayList->amountOfElements);
    if(index == arrayList->amountOfElements) { 
        if (arrayListSizeCheckAdd(arrayList) == -1) 
            return -1;
    }
    *((String*)arrayList->list+index) = value;
    if (index == arrayList->amountOfElements) arrayList->amountOfElements++;
    return 0;
}

String arrayListElementGetString(ArrayList* arrayList,size_t index) {
    assert(index < arrayList->amountOfElements);
    return *((String*)arrayList->list+index);
}

ArrayList* arrayListCreateString(size_t initialSize) {
    ArrayList* arrayList = malloc(sizeof(ArrayList));
    if(arrayList == NULL) return NULL;
    String* stringList = calloc(initialSize,sizeof(String));
    if(stringList == NULL) {
        free(arrayList);
        return NULL;
    }
    
    arrayList->listType = STRING;
    arrayList->elementSize = sizeof(String);
    arrayList->amountOfElements = 0;
    arrayList->totalAmountOfElements = initialSize;
    arrayList->list = stringList;
    return arrayList;
}

ArrayList* arrayListCreateChar(size_t initialSize) {
    ArrayList* arrayList = malloc(sizeof(ArrayList));
    if (arrayList == NULL) return NULL;
    char* string = calloc(initialSize, sizeof(char));
    if (string == NULL) {
        free(arrayList);
        return NULL;
    }

    arrayList->listType = Char;
    arrayList->elementSize = sizeof(char);
    arrayList->amountOfElements = 0;
    arrayList->totalAmountOfElements = initialSize;
    arrayList->list = string;
    return arrayList;
}