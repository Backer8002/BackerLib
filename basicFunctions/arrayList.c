#include"pch.h"
#include<arrayList.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<threads.h>
#include<assert.h>
#undef arrayListElementSet
#undef arrayListElementInsert

int arrayListSizeCheckAdd(ArrayList* arrayList) {
    if(arrayList->amountOfElements >= arrayList->totalAmountOfElements) {
        size_t indexesToAssign = arrayList->totalAmountOfElements * 2 + 1;
        void*  newPointer      = realloc(arrayList->list, arrayList->elementSize * indexesToAssign);
        if(newPointer == NULL) 
            return -1;
        arrayList->list                  = newPointer;
        arrayList->totalAmountOfElements = indexesToAssign;
    }
    return 1;
}
int arrayListSizeCheckRemove(ArrayList* arrayList) {
    if(arrayList->totalAmountOfElements>>1 > arrayList->amountOfElements) {
        void* newPointer = realloc(arrayList->list,arrayList->elementSize*(arrayList->totalAmountOfElements>>1));
        if (newPointer == NULL)
            return -1;
        arrayList->list = newPointer;
        arrayList->totalAmountOfElements >>= 1;
    }
    return 1;
}

inline void arrayListElementsClear(ArrayList* arrayList) {
    mtx_lock(&arrayList->mutex);
    memset(arrayList->list,0,arrayList->totalAmountOfElements*arrayList->elementSize);
    mtx_unlock(&arrayList->mutex);
}

void* arrayListElementGet(ArrayList* arrayList,size_t index) {
    assert(index < arrayList->amountOfElements);
    unsigned char* returnVal = (arrayList->header.flags & ObjectFlagContentsIsPointers) ? *((unsigned char**) arrayList->list + (index * arrayList->elementSize)) : ((unsigned char*) arrayList->list + (index * arrayList->elementSize));
    return returnVal;
}

void arrayListElementPop(ArrayList* arrayList) {
    if (arrayList->amountOfElements == 0) {
        return;
    }
    arrayList->amountOfElements--;
    arrayListSizeCheckRemove(arrayList);
}

void arrayListElementRemove(ArrayList* arrayList, size_t index,size_t lastIndex) {
    assert(index <= lastIndex);
    if (lastIndex != arrayList->amountOfElements-1){
        void*  currentIndex        = arrayListElementGet(arrayList, index);
        void*  nextIndex           = arrayListElementGet(arrayList, lastIndex + 1);
        size_t amountOfBytesToMove = (arrayList->amountOfElements - lastIndex - 1) * arrayList->elementSize;   
        memmove_s(currentIndex,amountOfBytesToMove+((lastIndex-index+1)*arrayList->elementSize),nextIndex,amountOfBytesToMove);
    }
    arrayList->amountOfElements -= lastIndex-index+1;
    arrayListSizeCheckRemove(arrayList);
}

int arrayListElementInsert(ArrayList* arrayList, size_t index, void* element,size_t elementSize) {
    assert(index <= arrayList->amountOfElements);
    assert(elementSize == arrayList->elementSize);
    if (arrayListSizeCheckAdd(arrayList) == -1) {
        return -1;
    }
    memmove_s((unsigned char*) arrayList->list + (index + 1) * elementSize,
        (arrayList->totalAmountOfElements - index + 2) * elementSize,
        (unsigned char*) arrayList->list + index * elementSize,
        (arrayList->totalAmountOfElements - index + 1) * elementSize); //+2 and +1 for off by one

    for (size_t i = 0; i < elementSize; i++)
        *((unsigned char*) arrayList->list + arrayList->elementSize * index + i) = *((unsigned char*) element + i);
    arrayList->amountOfElements++;
    return 0;
}

int arrayListElementSet(ArrayList* arrayList, size_t index,void* element,size_t elementSize) {
    assert(index <= arrayList->amountOfElements);
    assert(elementSize == arrayList->elementSize);
    if (index == arrayList->amountOfElements) {
        if (arrayListSizeCheckAdd(arrayList) == -1) {
            return -1;
        }
        arrayList->amountOfElements++;
    }
    for (size_t i = 0; i < elementSize; i++)
        *((unsigned char*) arrayList->list + arrayList->elementSize * index + i) = *((unsigned char*) element + i);
    return 0;
}


ArrayList arrayListCreateStack(size_t intialSize, size_t elementSize, ListTypes_t elementType,bool elementsArePointers) {
    ArrayList arrayList;
    arrayList.amountOfElements = 0;
    arrayList.elementSize = elementSize;
    arrayList.header.dataArrayVarType = elementType;
    arrayList.header.flags            = (elementsArePointers) ? ObjectFlagContentsIsPointers : 0;
    arrayList.header.objectType      = ListArrayList;

    arrayList.list = malloc(elementSize * intialSize);
    if (arrayList.list == NULL) {
        arrayList.totalAmountOfElements = 0;
        arrayList.elementSize = 0;
        return arrayList;
    }
    if (mtx_init(&arrayList.mutex, mtx_plain) == thrd_success)
        arrayList.header.flags |= ObjectFlagMutexExists;
    arrayList.totalAmountOfElements = intialSize;
    return arrayList;
}

ArrayList* arrayListCreate(size_t intialSize, size_t elementSize, ListTypes_t elementType, bool elementsArePointers) {
    ArrayList* arrayList = malloc(sizeof(ArrayList));
    if (arrayList == NULL)
        return NULL;
    arrayList->amountOfElements        = 0;
    arrayList->elementSize             = elementSize;
    arrayList->header.dataArrayVarType = elementType;
    arrayList->header.flags            = ((elementsArePointers) ? elementsArePointers : 0) | ObjectFlagIsOnHeap;
    arrayList->header.objectType       = ListArrayList;

    arrayList->list = malloc(elementSize * intialSize);
    if (arrayList->list == NULL) {
        free(arrayList);
        return NULL;
    }
    if (mtx_init(&arrayList->mutex, mtx_plain)== thrd_success)
        arrayList->header.flags |= ObjectFlagMutexExists;
    arrayList->totalAmountOfElements = intialSize;
    return arrayList;
}


inline void arrayListDestroy(void* arrayList) {
    ArrayList* arraylist = arrayList;
    mtx_destroy(&arraylist->mutex);
    if (arraylist->list != NULL)
        free(arraylist->list);
    if (arraylist->header.flags & ObjectFlagIsOnHeap)
        free(arraylist);
}

void arrayListDestroyWithElements(ArrayList* arrayList,void(elementDestructor)(void* element)) {
    for (size_t i = 0; i < arrayList->amountOfElements; i++)
        elementDestructor((arrayList->header.flags & ObjectFlagContentsIsPointers) ? *((unsigned char**)arrayList->list + arrayList->elementSize * i) : ((unsigned char*)arrayList->list) + arrayList->elementSize*i);
    arrayListDestroy(arrayList);
}

ArrayList* arrayListMoveStackToHeap(ArrayList arrayList,bool destroyInputOnFailiure) {
    if ((arrayList.header.flags & ObjectFlagIsOnHeap) || arrayList.header.dataArrayVarType == ListNone)
        return NULL;
    ArrayList* arrayListNew = malloc(sizeof(ArrayList));
    if (arrayListNew == NULL) {
        if (destroyInputOnFailiure)
            arrayListDestroy(&arrayList);
        return NULL;
    }
    *arrayListNew = arrayList;
    arrayListNew->header.flags |= ObjectFlagIsOnHeap;
    return arrayListNew;
}

ArrayList arrayListMoveStack(ArrayList* arrayList) {
    ArrayList arrayListNew = *arrayList;
    arrayListNew.header.flags &= ~ObjectFlagIsOnHeap;
    if (arrayList->header.flags & ObjectFlagIsOnHeap)
        free(arrayList);
    return arrayListNew;
}

ArrayList* arrayListCopyStackToHeap(ArrayList* arrayList) {
    if ((arrayList->header.flags & ObjectFlagIsOnHeap) ||arrayList->list == NULL)
        return NULL;
    ArrayList* arrayListNew = arrayListCreate(arrayList->totalAmountOfElements,arrayList->elementSize,arrayList->header.dataArrayVarType,arrayList->header.flags & ObjectFlagContentsIsPointers);
    if (arrayListNew == NULL)
        return NULL;
    memcpy_s(arrayListNew->list,
        arrayListNew->totalAmountOfElements * arrayListNew->elementSize,
        arrayList->list,
        arrayList->totalAmountOfElements * arrayList->elementSize);
    arrayListNew->amountOfElements = arrayList->amountOfElements;
    return arrayListNew;
}

ArrayList arrayListCopyStack(ArrayList* arrayList) {
    ArrayList arrayListNew = arrayListCreateStack(arrayList->totalAmountOfElements, arrayList->elementSize, arrayList->header.dataArrayVarType, arrayList->header.flags & ObjectFlagContentsIsPointers);
    if (arrayListNew.list == NULL)
        return arrayListNew;

    memcpy_s(arrayListNew.list,
        arrayListNew.totalAmountOfElements * arrayListNew.elementSize,
        arrayList->list,
        arrayList->totalAmountOfElements * arrayList->elementSize);
    arrayListNew.amountOfElements = arrayList->amountOfElements;
    return arrayListNew;
}