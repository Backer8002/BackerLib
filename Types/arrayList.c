#include"pch.h"
#include<arrayList.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<threads.h>
#include<assert.h>
#undef arrayListElementSet
#undef arrayListElementInsert

ArrayListError_t arrayListSizeCheckAdd(ArrayList* arrayList) {
    if(arrayList->amountOfElements >= arrayList->totalAmountOfElements) {
        size_t indexesToAssign = arrayList->amountOfElements * 2 + 1;
        void*  newPointer      = realloc(arrayList->list, arrayList->elementSize * indexesToAssign);
        if(newPointer == NULL) 
            return ArrayListCannotAllocMemory;
        arrayList->list                  = newPointer;
        arrayList->totalAmountOfElements = indexesToAssign;
    }
    return ArrayListOperationSuccsess;
}
ArrayListError_t arrayListSizeCheckRemove(ArrayList* arrayList) {
    if(arrayList->totalAmountOfElements>>1 > arrayList->amountOfElements) {
        void* newPointer = realloc(arrayList->list,arrayList->elementSize*(arrayList->totalAmountOfElements>>1));
        if (newPointer == NULL)
            return ArrayListCannotAllocMemory;
        arrayList->list = newPointer;
        arrayList->totalAmountOfElements >>= 1;
    }
    return ArrayListOperationSuccsess;
}

inline void arrayListElementsClear(ArrayList* arrayList) {
    mtx_lock(&arrayList->mutex);
    memset(arrayList->list,0,arrayList->totalAmountOfElements*arrayList->elementSize);
    mtx_unlock(&arrayList->mutex);
}

void* arrayListElementGet(const ArrayList* arrayList,size_t index) {
    if (index >= arrayList->amountOfElements)
        return NULL;
    Bytes returnVal = (arrayList->header.flags & ObjectFlagContentsIsPointers) ? *((Bytes*) arrayList->list + (index * arrayList->elementSize)) : ((Bytes) arrayList->list + (index * arrayList->elementSize));
    return returnVal;
}

ArrayListError_t arrayListElementPop(ArrayList* arrayList) {
    if (arrayList->amountOfElements == 0)
        return ArrayListAccessViolation;
    arrayList->amountOfElements--;
    arrayListSizeCheckRemove(arrayList);
    return ArrayListOperationSuccsess;
}

void arrayListElementRemove(ArrayList* arrayList, size_t index,size_t lastIndex) {
    assert(index <= lastIndex);
    if (lastIndex != arrayList->amountOfElements-1){
        void*  currentIndex        = arrayListElementGet(arrayList, index);
        void*  nextIndex           = arrayListElementGet(arrayList, lastIndex + 1);
        size_t amountOfBytesToMove = (arrayList->amountOfElements - lastIndex - 1) * arrayList->elementSize;   
        memmove(currentIndex,nextIndex,amountOfBytesToMove);
    }
    arrayList->amountOfElements -= lastIndex-index+1;
    arrayListSizeCheckRemove(arrayList);
}

ArrayListError_t arrayListElementInsert(ArrayList* arrayList, size_t index, void* elements,size_t amountOfElements,ListTypes_t elementType) {
    if (index > arrayList->amountOfElements)
        return ArrayListAccessViolation;
    if (elementType != arrayList->header.dataArrayVarType)
        return ArrayListInvalidType;
    arrayList->amountOfElements += amountOfElements;
    if (arrayListSizeCheckAdd(arrayList) == ArrayListCannotAllocMemory) {
        arrayList->amountOfElements -= amountOfElements;
        return ArrayListCannotAllocMemory;
    }

    memmove((Bytes) arrayList->list + (index + amountOfElements) * arrayList->elementSize,
        (Bytes) arrayList->list + index * arrayList->elementSize,
        (arrayList->totalAmountOfElements - index) * arrayList->elementSize);

    memcpy((Bytes) arrayList->list + arrayList->elementSize * index, elements, arrayList->elementSize*amountOfElements);
    return ArrayListOperationSuccsess;
}

ArrayListError_t arrayListElementSet(ArrayList* arrayList, size_t index,void* element,ListTypes_t elementType) {
    if (index > arrayList->amountOfElements)
        return ArrayListAccessViolation;
    if (elementType != arrayList->header.dataArrayVarType)
        return ArrayListInvalidType;

    if (index == arrayList->amountOfElements) {
        if (arrayListSizeCheckAdd(arrayList) == ArrayListCannotAllocMemory)
            return ArrayListCannotAllocMemory;
        arrayList->amountOfElements++;
    }
    memcpy((Bytes) arrayList->list + arrayList->elementSize * index, element,arrayList->elementSize);
    return ArrayListOperationSuccsess;
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
    memcpy(arrayListNew->list,
        arrayList->list,
        arrayListNew->totalAmountOfElements * arrayListNew->elementSize);
    arrayListNew->amountOfElements = arrayList->amountOfElements;
    return arrayListNew;
}

ArrayList arrayListCopyStack(ArrayList* arrayList) {
    ArrayList arrayListNew = arrayListCreateStack(arrayList->totalAmountOfElements, arrayList->elementSize, arrayList->header.dataArrayVarType, arrayList->header.flags & ObjectFlagContentsIsPointers);
    if (arrayListNew.list == NULL)
        return arrayListNew;

    memcpy(arrayListNew.list,
        arrayList->list,
        arrayListNew.totalAmountOfElements * arrayListNew.elementSize);
    arrayListNew.amountOfElements = arrayList->amountOfElements;
    return arrayListNew;
}