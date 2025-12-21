#include "BL_ArrayList.h"
#include <BackerLibConcurrency.h>
#include <stdbool.h>
#include <stdlib.h>

bool arrayListClear(BL_ArrayList* arrayList) {
    if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
        return false;
    bl_container_dynamic_clear((BL_DynamicContainer*) arrayList);
    mutexUnlock(&arrayList->mutex);
    return true;
}

BL_ContainerError arrayListGet(BL_ArrayList* arrayList, size_t index, size_t sizeOfElement, void* restrict element) {
    if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
        return BL_ContainerOPUnsuccessful;
    void* elementToGet = bl_container_get((BL_Container*) arrayList, index);
    if (!elementToGet) {
        mutexUnlock(&arrayList->mutex);
        return BL_ContainerInvalidIndex;
    }
    if (arrayList->dynamicContainer.container.byteSizeOfSingleElement < sizeOfElement) {
        mutexUnlock(&arrayList->mutex);
        return BL_ContainerInvalidSize;
    }
    memcpy(element, elementToGet, sizeOfElement);
    mutexUnlock(&arrayList->mutex);
    return BL_ContainerOPSuccessful;
}

BL_ContainerError arrayListPop(BL_ArrayList* arrayList) {
    if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
        return BL_ContainerOPUnsuccessful;
    BL_ContainerError result = bl_container_dynamic_pop((BL_DynamicContainer*) arrayList);
    mutexUnlock(&arrayList->mutex);
    return result;
}

BL_ContainerError arrayListRemove(BL_ArrayList* arrayList, size_t index, size_t lastIndex) {
    if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
        return BL_ContainerOPUnsuccessful;
    BL_ContainerError result = bl_container_dynamic_remove((BL_DynamicContainer*) arrayList, index, lastIndex);
    mutexUnlock(&arrayList->mutex);
    return result;
}

BL_ContainerError arrayListInsert(BL_ArrayList* arrayList, size_t index, size_t amountOfElements, size_t sizeOfElement, const void* elements) {
    if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
        return BL_ContainerOPUnsuccessful;
    BL_ContainerError result = bl_container_dynamic_insert((BL_DynamicContainer*) arrayList, index, amountOfElements, sizeOfElement, elements);
    mutexUnlock(&arrayList->mutex);
    return result;
}

BL_ContainerError arrayListSet(BL_ArrayList* arrayList, size_t index, size_t elementSize, const void* element) {
    if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
        return BL_ContainerOPUnsuccessful;
    BL_ContainerError result = bl_container_set((BL_Container*) arrayList, index, elementSize, element);
    mutexUnlock(&arrayList->mutex);
    return result;
}

BL_ArrayList arrayListCreateStack(size_t initialSize, size_t elementSize) {
    BL_ArrayList arrayList = {.dynamicContainer = bl_container_dynamic_create_stack(initialSize, elementSize)};
    if (!bl_container_dynamic_is_valid(&arrayList.dynamicContainer))
        return arrayList;
    arrayList.mutex = mutexCreate(MutexPlain);
    if (!mutexIsValid(&arrayList.mutex)) {
        bl_container_destroy(&arrayList);
        return arrayList;
    }
    arrayList.dynamicContainer.container.header |= ObjectFlagMutexExists;
    return arrayList;
}

BL_ArrayList* arrayListCreateHeap(size_t initialSize, size_t elementSize ) {
    BL_ArrayList* arrayList = malloc(sizeof(*arrayList));
    if (!arrayList)
        return NULL;
    arrayList->dynamicContainer = bl_container_dynamic_create_stack(initialSize, elementSize);
    if (!bl_container_dynamic_is_valid(&arrayList->dynamicContainer)) {
        free(arrayList);
        return NULL;
    }
    arrayList->dynamicContainer.container.header |= ObjectFlagIsOnHeap;
    arrayList->mutex = mutexCreate(MutexPlain);
    if (!mutexIsValid(&arrayList->mutex)) {
        bl_container_destroy((void*)&arrayList);
        return arrayList;
    }
    arrayList->dynamicContainer.container.header |= ObjectFlagMutexExists;
    return arrayList;
}

void arrayListDestroy(void* arrayList) {
    if (!bl_container_dynamic_is_valid(&((BL_ArrayList*)arrayList)->dynamicContainer)) {
        mutexDestroy(&((BL_ArrayList*) arrayList)->mutex);
        bl_container_destroy(&((BL_ArrayList*)arrayList)->dynamicContainer.container);
    }
}

void arrayListDestroyWithElements(BL_ArrayList* arrayList, void(elementDestructor)(void* element)) {
    mutexDestroy(&arrayList->mutex);
    bl_container_dynamic_destroy_with_elements(&arrayList->dynamicContainer, elementDestructor);
}