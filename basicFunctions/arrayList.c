#include"pch.h"
#include<arrayList.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<assert.h>

#include<stdio.h>
int arrayListSizeCheckAdd(ArrayList* arrayList) {
    if(arrayList->amountOfElements >= arrayList->totalAmountOfElements) {
        size_t indexesToAssign = arrayList->totalAmountOfElements*2+1;
        void* newPointer = realloc(arrayList->list,arrayList->elementSize*indexesToAssign);
        if(newPointer == NULL) {return -1;}
        arrayList->list = newPointer;
        arrayList->totalAmountOfElements = indexesToAssign;
    }
    return 1;
}
int arrayListSizeCheckRemove(ArrayList* arrayList) {
    if(arrayList->totalAmountOfElements>>1 > arrayList->amountOfElements) {
        void* newPointer = realloc(arrayList->list,arrayList->elementSize*(arrayList->totalAmountOfElements>>1));
        if (newPointer == NULL) return -1;
        arrayList->list = newPointer;
        arrayList->totalAmountOfElements >>= 1;
    }
    return 1;
}

void arrayListElementsClear(ArrayList* arrayList) {
    memset(arrayList->list,0,arrayList->totalAmountOfElements*arrayList->elementSize);
}

void* arrayListElementGetGeneric(ArrayList* arrayList,size_t index) {
    assert(index < arrayList->amountOfElements);
    return ((unsigned char*)arrayList->list+(index*arrayList->elementSize));
}

void* arrayListElementPopGeneric(ArrayList* arrayList) {
    void* element = arrayListElementGetGeneric(arrayList,arrayList->amountOfElements-1);
    arrayList->amountOfElements--;
    arrayListSizeCheckRemove(arrayList);
    return element;
}

void arrayListElementRemove(ArrayList* arrayList, size_t index,size_t lastIndex) {
    if (lastIndex != arrayList->amountOfElements-1){
        void* currentIndex = arrayListElementGetGeneric(arrayList,index);
        void* nextIndex = arrayListElementGetGeneric(arrayList,lastIndex+1);
        size_t amountOfBytesToMove = (arrayList->amountOfElements-lastIndex-1)*arrayList->elementSize;   
        memmove_s(currentIndex,amountOfBytesToMove+(index*arrayList->elementSize),nextIndex,amountOfBytesToMove);
    }
    arrayList->amountOfElements -= lastIndex-index+1;
    arrayListSizeCheckRemove(arrayList);
}

void arrayListDestroy(ArrayList* arraylist) {
    free(arraylist->list);
    free(arraylist);
}