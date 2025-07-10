#include "arrayList.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

// Function to be called when a resize to a larger array is needed. Will resize if needed. If resize does not happen this returns ArrayListCannotAllocMemory
ArrayListError_t arrayListSizeCheckAdd(ArrayList* arrayList) {
    if (arrayList->amountOfElements >= arrayList->totalAmountOfElements) {
        size_t indexesToAssign = arrayList->amountOfElements * 2 + 1;
        void*  newPointer      = realloc(arrayList->list, arrayList->elementSize * indexesToAssign);
        if (newPointer == NULL)
            return ArrayListCannotAllocMemory;
        arrayList->list                  = newPointer;
        arrayList->totalAmountOfElements = indexesToAssign;
    }
    return ArrayListOperationSuccsess;
}

// Function to be called when a resize to a smaller array seems fit. Will resize if needed. If resize cannot happen this returns ArrayListCannotAllocMemory.
ArrayListError_t arrayListSizeCheckRemove(ArrayList* arrayList) {
    if (arrayList->totalAmountOfElements >> 1 > arrayList->amountOfElements) {
        void* newPointer = realloc(arrayList->list, arrayList->elementSize * (arrayList->totalAmountOfElements >> 1));
        if (newPointer == NULL)
            return ArrayListCannotAllocMemory;
        arrayList->list = newPointer;
        arrayList->totalAmountOfElements >>= 1;
    }
    return ArrayListOperationSuccsess;
}

// Sets then amount of elements in the arraylist to 0. Locks the mutex.
inline void arrayListElementsClear(ArrayList* arrayList) {
    mtx_lock(&arrayList->mutex);
    arrayList->amountOfElements = 0;
    arrayListSizeCheckRemove(arrayList);
    mtx_unlock(&arrayList->mutex);
}

// Returns the pointer to an element in the arraylist. If the index was invalid this function returns void. If ObjectFlagContentsIsPointers is set then this returns the set pointer and not the pointer to the object in the array.
void* arrayListElementGet(const ArrayList* arrayList, size_t index) {
    if (index >= arrayList->amountOfElements)
        return NULL;
    return (arrayList->header.flags & ObjectFlagContentsIsPointers) ? *((Bytes*) arrayList->list + (index * arrayList->elementSize)) : ((Bytes) arrayList->list + (index * arrayList->elementSize));
}

// Removes the last element from the arraylist. Returns ArrayListAccessViolation if the list is empty.
ArrayListError_t arrayListElementPop(ArrayList* arrayList) {
    if (arrayList->amountOfElements == 0)
        return ArrayListAccessViolation;
    arrayList->amountOfElements--;
    arrayListSizeCheckRemove(arrayList);
    return ArrayListOperationSuccsess;
}

// Removes elements from index to lastIndex (inclusive) from the array. Returns ArrayListAccessViolation if index is larger than lastIndex or if the indexes are invalid
ArrayListError_t arrayListElementRemove(ArrayList* arrayList, size_t index, size_t lastIndex) {
    if (index > lastIndex)
        return ArrayListAccessViolation;
    if (lastIndex >= arrayList->amountOfElements)
        return ArrayListAccessViolation;

    if (lastIndex != arrayList->amountOfElements - 1) {
        void*  currentIndex        = arrayListElementGet(arrayList, index);
        void*  nextIndex           = arrayListElementGet(arrayList, lastIndex + 1);
        size_t amountOfBytesToMove = (arrayList->amountOfElements - lastIndex - 1) * arrayList->elementSize;
        memmove(currentIndex, nextIndex, amountOfBytesToMove);
    }
    arrayList->amountOfElements -= lastIndex - index + 1;
    arrayListSizeCheckRemove(arrayList);
    return ArrayListOperationSuccsess;
}

/*
Inserts array elements starting at index.

Returns ArrayListAccessViolation if the index was invalid.
Returns ArrayListInvalidType if the gived type was not the one of the ArrayList.
Returns ArrayListCannotAllocMemory if the list cannot grow.
*/
ArrayListError_t arrayListElementInsert(ArrayList* arrayList, size_t index, size_t amountOfElements, void* elements, ListTypes_t elementType) {
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

    memcpy((Bytes) arrayList->list + arrayList->elementSize * index, elements, arrayList->elementSize * amountOfElements);
    return ArrayListOperationSuccsess;
}

/*
Sets element at index.

Returns ArrayListAccessViolation if the index was invalid.
Returns ArrayListInvalidType if the gived type was not the one of the ArrayList.
Returns ArrayListCannotAllocMemory if the list cannot grow.
*/
ArrayListError_t arrayListElementSet(ArrayList* arrayList, size_t index, void* element, ListTypes_t elementType) {
    if (index > arrayList->amountOfElements)
        return ArrayListAccessViolation;

    if (elementType != arrayList->header.dataArrayVarType)
        return ArrayListInvalidType;

    if (index == arrayList->amountOfElements) {
        if (arrayListSizeCheckAdd(arrayList) == ArrayListCannotAllocMemory)
            return ArrayListCannotAllocMemory;
        arrayList->amountOfElements++;
    }
    memcpy((Bytes) arrayList->list + arrayList->elementSize * index, element, arrayList->elementSize);
    return ArrayListOperationSuccsess;
}

// Creates an ArrayList object on the stack.
ArrayList arrayListCreateStack(size_t intialSize, size_t elementSize, ListTypes_t elementType, bool elementsArePointers) {
    ArrayList arrayList               = {0};
    arrayList.amountOfElements        = 0;
    arrayList.elementSize             = elementSize;
    arrayList.header.dataArrayVarType = elementType;
    arrayList.header.flags            = (elementsArePointers) ? ObjectFlagContentsIsPointers : 0;
    arrayList.header.objectType       = ListArrayList;

    arrayList.list                    = malloc(elementSize * intialSize);
    if (arrayList.list == NULL) {
        arrayList.totalAmountOfElements = 0;
        arrayList.elementSize           = 0;
        return arrayList;
    }
    if (mtx_init(&arrayList.mutex, mtx_plain) == thrd_success)
        arrayList.header.flags |= ObjectFlagMutexExists;
    arrayList.totalAmountOfElements = intialSize;
    return arrayList;
}

// Creates an ArrayList object on the heap.
ArrayList* arrayListCreate(size_t intialSize, size_t elementSize, ListTypes_t elementType, bool elementsArePointers) {
    ArrayList* arrayList = malloc(sizeof(ArrayList));
    if (arrayList == NULL)
        return NULL;
    arrayList->amountOfElements        = 0;
    arrayList->elementSize             = elementSize;
    arrayList->header.dataArrayVarType = elementType;
    arrayList->header.flags            = ((elementsArePointers) ? elementsArePointers : 0) | ObjectFlagIsOnHeap;
    arrayList->header.objectType       = ListArrayList;

    arrayList->list                    = malloc(elementSize * intialSize);
    if (arrayList->list == NULL) {
        free(arrayList);
        return NULL;
    }
    if (mtx_init(&arrayList->mutex, mtx_plain) == thrd_success)
        arrayList->header.flags |= ObjectFlagMutexExists;
    arrayList->totalAmountOfElements = intialSize;
    return arrayList;
}

// Destorys an ArrayList
inline void arrayListDestroy(void* arrayList) {
    ArrayList* arraylist = arrayList;
    mtx_destroy(&arraylist->mutex);
    if (arraylist->list != NULL)
        free(arraylist->list);
    if (arraylist->header.flags & ObjectFlagIsOnHeap)
        free(arraylist);
}

// Destroys the elements in the ArrayList with the given destructor.
void arrayListDestroyWithElements(ArrayList* arrayList, void(elementDestructor)(void* element)) {
    for (size_t i = 0; i < arrayList->amountOfElements; i++)
        elementDestructor((arrayList->header.flags & ObjectFlagContentsIsPointers) ? *((Bytes*) arrayList->list + arrayList->elementSize * i) : ((Bytes) arrayList->list) + arrayList->elementSize * i);
    arrayListDestroy(arrayList);
}

// Moves the input to a heap allocated object. If the the input already is an heap alloced object or if allocation fails this function will return NULL. destroyInputOnFailure should be set if this is a part of a pipeline.
ArrayList* arrayListMoveStackToHeap(ArrayList arrayList, bool destroyInputOnFailiure) {
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

// Moves the input to a stack object. If the input was a heap allocated object it has now been freed. This function cannot fail.
ArrayList arrayListMoveStack(ArrayList* arrayList) {
    ArrayList arrayListNew = *arrayList;
    arrayListNew.header.flags &= ~ObjectFlagIsOnHeap;
    if (arrayList->header.flags & ObjectFlagIsOnHeap)
        free(arrayList);
    return arrayListNew;
}

// Copies the input and it's list to a heap allocated object. If it already is a heap allocated object or allocation fails this function returns NULL.
ArrayList* arrayListCopyStackToHeap(ArrayList* arrayList) {
    if ((arrayList->header.flags & ObjectFlagIsOnHeap) || arrayList->list == NULL)
        return NULL;
    ArrayList* arrayListNew = arrayListCreate(arrayList->totalAmountOfElements, arrayList->elementSize, arrayList->header.dataArrayVarType, arrayList->header.flags & ObjectFlagContentsIsPointers);
    if (arrayListNew == NULL)
        return NULL;
    memcpy(arrayListNew->list,
           arrayList->list,
           arrayListNew->totalAmountOfElements * arrayListNew->elementSize);
    arrayListNew->amountOfElements = arrayList->amountOfElements;
    return arrayListNew;
}

// Copies the input and it's list to a stack object. .header.dataArrayVarType will be ListNone if allocation of list fails.
ArrayList arrayListCopyStack(ArrayList* arrayList) {
    ArrayList arrayListNew = arrayListCreateStack(arrayList->totalAmountOfElements, arrayList->elementSize, arrayList->header.dataArrayVarType, arrayList->header.flags & ObjectFlagContentsIsPointers);
    if (arrayListNew.header.dataArrayVarType == ListNone)
        return arrayListNew;

    memcpy(arrayListNew.list,
           arrayList->list,
           arrayListNew.totalAmountOfElements * arrayListNew.elementSize);
    arrayListNew.amountOfElements = arrayList->amountOfElements;
    return arrayListNew;
}