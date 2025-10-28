#include "ArrayList.h"
#include <BackerLibConcurrency.h>
#include <stdbool.h>
#include <stdlib.h>

bool arrayListClear(ArrayList* arrayList) {
    if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
        return false;
    containerDynamicClear((DynamicContainer*) arrayList);
    mutexUnlock(&arrayList->mutex);
    return true;
}

ContainerError arrayListGet(ArrayList* arrayList, size_t index, size_t sizeOfElement, void* restrict element) {
    if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
        return ContainerOPUnsuccessful;
    void* elementToGet = containerGet((Container*) arrayList, index);
    if (!elementToGet) {
        mutexUnlock(&arrayList->mutex);
        return ContainerInvalidIndex;
    }
    if (arrayList->container.byteSizeOfSingleElement < sizeOfElement) {
        mutexUnlock(&arrayList->mutex);
        return ContainerInvalidSize;
    }
    memcpy(element, elementToGet, sizeOfElement);
    mutexUnlock(&arrayList->mutex);
    return ContainerOPSuccessful;
}

ContainerError arrayListPop(ArrayList* arrayList) {
    if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
        return ContainerOPUnsuccessful;
    ContainerError result = containerDynamicPop((DynamicContainer*) arrayList);
    mutexUnlock(&arrayList->mutex);
    return result;
}

ContainerError arrayListRemove(ArrayList* arrayList, size_t index, size_t lastIndex) {
    if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
        return ContainerOPUnsuccessful;
    ContainerError result = containerDynamicRemove((DynamicContainer*) arrayList, index, lastIndex);
    mutexUnlock(&arrayList->mutex);
    return result;
}

ContainerError arrayListInsert(ArrayList* arrayList, size_t index, size_t amountOfElements, size_t sizeOfElement, const void* elements) {
    if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
        return ContainerOPUnsuccessful;
    ContainerError result = containerDynamicInsert((DynamicContainer*) arrayList, index, amountOfElements, sizeOfElement, elements);
    mutexUnlock(&arrayList->mutex);
    return result;
}

ContainerError arrayListSet(ArrayList* arrayList, size_t index, size_t elementSize, const void* element) {
    if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
        return ContainerOPUnsuccessful;
    ContainerError result = containerSet((Container*) arrayList, index, elementSize, element);
    mutexUnlock(&arrayList->mutex);
    return result;
}

ArrayList arrayListCreateStack(size_t initialSize, size_t elementSize, bool elementsArePointers) {
    ArrayList arrayList = {.dynamicContainer = containerDynamicCreateStack(initialSize, elementSize, elementsArePointers)};
    if (!isValidObject(&arrayList.header))
        return arrayList;
    arrayList.mutex = mutexCreate(MutexPlain);
    if (!mutexIsValid(&arrayList.mutex)) {
        containerDestroy(&arrayList);
        return arrayList;
    }
    arrayList.header |= ObjectFlagMutexExists;
    return arrayList;
}

ArrayList* arrayListCreateHeap(size_t initialSize, size_t elementSize, bool elementsArePointers) {
    ArrayList* arrayList = malloc(sizeof(*arrayList));
    if (!arrayList)
        return NULL;
    arrayList->dynamicContainer = containerDynamicCreateStack(initialSize, elementSize, elementsArePointers);
    if (!isValidObject((DataTypeFlags*) arrayList)) {
        free(arrayList);
        return NULL;
    }
    arrayList->header |= ObjectFlagIsOnHeap;
    arrayList->mutex = mutexCreate(MutexPlain);
    if (!mutexIsValid(&arrayList->mutex)) {
        containerDestroy(&arrayList);
        return arrayList;
    }
    arrayList->header |= ObjectFlagMutexExists;
    return arrayList;
}

void arrayListDestroy(void* arrayList) {
    if (isValidObject(arrayList)) {
        mutexDestroy(&((ArrayList*) arrayList)->mutex);
        containerDestroy(&((ArrayList*)arrayList)->container);
    }
}

void arrayListDestroyWithElements(ArrayList* arrayList, void(elementDestructor)(void* element)) {
    mutexDestroy(&arrayList->mutex);
    containerDynamicDestroyWithElements(&arrayList->dynamicContainer, elementDestructor);
}