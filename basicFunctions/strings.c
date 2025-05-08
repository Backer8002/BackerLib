#include"pch.h"
#include<backerStrings.h>
#include<arrayList.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<assert.h>
#include<stdio.h>

ArrayList* arrayListCreateString(size_t initialSize);
void stringCleanUpHelper(String* string, size_t* currentIndex,String* destString);
int setStringElement(ArrayList* arrayList,size_t index, void* value);
void* getStringElement(ArrayList* arrayList,size_t index);
void stringDestroyHelper(String* string);

String** stringCreate(char* string, size_t length) {
    if (length == 0) return NULL;
    String* allocatedString = malloc(sizeof(String));
    if (allocatedString == NULL) return NULL;
    char* rawString = calloc(length,sizeof(char));
    
    if (rawString == NULL) {
        free(allocatedString);
        return NULL;
    }
    memcpy_s(rawString,length,string,length);

    allocatedString->length = length;
    allocatedString->type = stringRaw;
    allocatedString->data = rawString;
    String** stringPtr = malloc(sizeof(String*));
    if (stringPtr == NULL) {
        free(allocatedString);
        free(rawString);
        return NULL;
    }
    *stringPtr = allocatedString;
    return stringPtr;
}

String** stringCreateWithCharPointer(char* string, size_t length) {
    String* allocatedString = malloc(sizeof(String));
    if (allocatedString == NULL) return NULL;

    allocatedString->length = length;
    allocatedString->type = stringRaw;
    allocatedString->data = string;
    String** stringPtr = malloc(sizeof(String*));
    if (stringPtr == NULL) {
        free(allocatedString);
        return NULL;
    }
    *stringPtr = allocatedString;
    return stringPtr;
}

void stringDestroy(String** stringPtr) {
    stringDestroyHelper(*stringPtr);
    free(stringPtr);
}

void stringDestroyHelper(String* string) {
    if (string->type == stringNode) {
        for(size_t iterator = 0;iterator<string->node->amountOfElements;iterator++) stringDestroyHelper(string->node->get(string->node,iterator));
        arrayListDestroy(string->node);
        free(string);
    } else {
        free(string->data);
        free(string);
    }
}

void stringCleanUp(String** stringPtr) {
    String* string = *stringPtr;
    if (string->type == stringRaw) return;

    char* rawDestString = calloc(string->length,sizeof(char));
    if(rawDestString == NULL) return;
    String** destString = stringCreateWithCharPointer(rawDestString,string->length);
    if(destString == NULL) {
        free(rawDestString);
        return;
    }
    size_t cleanerIndex = 0;
    stringCleanUpHelper(string,&cleanerIndex,*destString);
    stringDestroyHelper(string);
    *stringPtr = *destString;
    free(destString);
}

void stringCleanUpHelper(String* string, size_t* currentIndex,String* destString) {
    if(string->type == stringRaw) {
        for (size_t iterator = 0; iterator<string->length;iterator++) {
            *stringCharGet(&destString,*currentIndex) = *stringCharGet(&string,iterator);
            (*currentIndex)++;
        }
    } else {
        for(size_t iterator = 0; iterator<string->node->amountOfElements;iterator++) {
            String* subString = string->node->get(string->node,iterator);
            if (subString == NULL) return;
            stringCleanUpHelper(subString,currentIndex,destString);
        }
    }
}

char* stringCharGet(String** stringPtr,size_t index) {
    String* string = *stringPtr;
    if(string->type == stringRaw) {
        assert(string->length > index);
        return string->data+index;
    }
    assert(string->length > index);
    size_t indexIterator = 0;
    size_t iterator;
    String* subString = NULL;
    for(iterator = 0; iterator < string->node->amountOfElements; iterator++) {
        subString = string->node->get(string->node,iterator);
        indexIterator += subString->length;
        if(indexIterator > index) break;
    }
    if (subString == NULL) return NULL;
    return stringCharGet(&subString,subString->length-(indexIterator-index));
}

int stringAdd(String** destString, char* string2,size_t length) {
    if(string2 == NULL) return -2;
    String** allocedString2 = stringCreate(string2,length);
    if(allocedString2 == NULL) {return -1;}
    if (stringConcat(destString,allocedString2)!=0) {
        stringDestroy(allocedString2);
        return -1;
    }
    free(allocedString2);
    return 0;
}

int stringConcat(String** destStringPtr,String** secondStringPtr) {
    if((*destStringPtr)->type == stringRaw) {
        String** destStringRealloced = stringCreateWithCharPointer((*destStringPtr)->data,(*destStringPtr)->length);
        if(destStringRealloced == NULL) return -1;
        ArrayList* stringArray = arrayListCreateString(2);
        if (stringArray == NULL) {
            stringDestroy(destStringRealloced);
            return -1;
        }

        stringArray->set(stringArray,0,*destStringRealloced);
        stringArray->set(stringArray,1,*secondStringPtr);
        (*destStringPtr)->node = stringArray;
        (*destStringPtr)->type = stringNode;
        (*destStringPtr)->length = (*secondStringPtr)->length + (*destStringRealloced)->length;
        free(destStringRealloced);
    } else {
        int code = (*destStringPtr)->node->add((*destStringPtr)->node,*secondStringPtr);
        if(code!=0) return code;
        (*destStringPtr)->length = (*secondStringPtr)->length + (*destStringPtr)->length;
    }
    *secondStringPtr = *destStringPtr;
    return 0;
}

char* stringGetArray(String** stringPtr, size_t* arraySize) {
    stringCleanUp(stringPtr);
    if (*stringPtr == stringNode) return NULL;
    *arraySize = (*stringPtr)->length;
    return (*stringPtr)->data;
}


//ArrayListStuff

int setStringElement(ArrayList* arrayList,size_t index, void* value) {
    assert(index <= arrayList->amountOfElements);
    if(index == arrayList->amountOfElements) { 
        if (arrayListSizeCheckAdd(arrayList) == -1) return -1;
    }
    *((String**)arrayList->list+index) = (String*)value;
    if (index == arrayList->amountOfElements) arrayList->amountOfElements++;
    return 0;
}

void* getStringElement(ArrayList* arrayList,size_t index) {
    assert(index < arrayList->amountOfElements);
    return *((String**)arrayList->list+index);
}

ArrayList* arrayListCreateString(size_t initialSize) {
    ArrayList* arrayList = malloc(sizeof(ArrayList));
    if(arrayList == NULL) return NULL;
    String** stringList = calloc(initialSize,sizeof(String*));
    if(stringList == NULL) {
        free(arrayList);
        return NULL;
    }
    
    arrayList->listType = STRING;
    arrayList->add = arrayListGenericAddElement;
    arrayList->get = getStringElement;
    arrayList->pop = arrayListGenericPopElement;
    arrayList->set = setStringElement;
    arrayList->remove = arrayListGenericRemoveElement;
    arrayList->clear = arrayListGenericClearElements;
    arrayList->elementSize = sizeof(String);
    arrayList->amountOfElements = initialSize;
    arrayList->totalAmountOfElements = initialSize;
    arrayList->list = stringList;
    return arrayList;
}