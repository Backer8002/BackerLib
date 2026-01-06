#include "BL_ArrayList.h"
#include <BackerLibConcurrency.h>
#include <stdbool.h>
#include <stdlib.h>

bool arrayListClear(BL_ArrayList* arrayList) {
    if (bl_mutex_lock(&arrayList->mutex) == BL_ConcurrencyFailure)
        return false;
    bl_container_dynamic_clear((BL_DynamicContainer*) arrayList);
    bl_mutex_unlock(&arrayList->mutex);
    return true;
}

BL_ContainerError arrayListGet(BL_ArrayList* arrayList, size_t index, size_t sizeOfElement, void* restrict element) {
    if (bl_mutex_lock(&arrayList->mutex) == BL_ConcurrencyFailure)
        return BL_ContainerOPUnsuccessful;
    void* elementToGet = bl_container_get((BL_Container*) arrayList, index);
    if (!elementToGet) {
        bl_mutex_unlock(&arrayList->mutex);
        return BL_ContainerInvalidIndex;
    }
    if (arrayList->dynamicContainer.container.byteSizeOfSingleElement < sizeOfElement) {
        bl_mutex_unlock(&arrayList->mutex);
        return BL_ContainerInvalidSize;
    }
    memcpy(element, elementToGet, sizeOfElement);
    bl_mutex_unlock(&arrayList->mutex);
    return BL_ContainerOPSuccessful;
}

BL_ContainerError arrayListPop(BL_ArrayList* arrayList) {
    if (bl_mutex_lock(&arrayList->mutex) == BL_ConcurrencyFailure)
        return BL_ContainerOPUnsuccessful;
    BL_ContainerError result = bl_container_dynamic_pop((BL_DynamicContainer*) arrayList);
    bl_mutex_unlock(&arrayList->mutex);
    return result;
}

BL_ContainerError arrayListRemove(BL_ArrayList* arrayList, size_t index, size_t lastIndex) {
    if (bl_mutex_lock(&arrayList->mutex) == BL_ConcurrencyFailure)
        return BL_ContainerOPUnsuccessful;
    BL_ContainerError result = bl_container_dynamic_remove((BL_DynamicContainer*) arrayList, index, lastIndex);
    bl_mutex_unlock(&arrayList->mutex);
    return result;
}

BL_ContainerError arrayListInsert(BL_ArrayList* arrayList, size_t index, size_t amountOfElements, size_t sizeOfElement, const void* elements) {
    if (bl_mutex_lock(&arrayList->mutex) == BL_ConcurrencyFailure)
        return BL_ContainerOPUnsuccessful;
    BL_ContainerError result = bl_container_dynamic_insert((BL_DynamicContainer*) arrayList, index, amountOfElements, sizeOfElement, elements);
    bl_mutex_unlock(&arrayList->mutex);
    return result;
}

BL_ContainerError arrayListSet(BL_ArrayList* arrayList, size_t index, size_t elementSize, const void* element) {
    if (bl_mutex_lock(&arrayList->mutex) == BL_ConcurrencyFailure)
        return BL_ContainerOPUnsuccessful;
    BL_ContainerError result = bl_container_set((BL_Container*) arrayList, index, elementSize, element);
    bl_mutex_unlock(&arrayList->mutex);
    return result;
}

BL_ArrayList arrayListCreateStack(size_t initialSize, size_t elementSize) {
    BL_ArrayList arrayList = {.dynamicContainer = bl_container_dynamic_create_stack(initialSize, elementSize)};
    if (!bl_container_dynamic_is_valid(&arrayList.dynamicContainer))
        return arrayList;
    arrayList.mutex = bl_mutex_create(BL_MutexPlain);
    if (!bl_mutex_is_valid(&arrayList.mutex)) {
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
    arrayList->mutex = bl_mutex_create(BL_MutexPlain);
    if (!bl_mutex_is_valid(&arrayList->mutex)) {
        bl_container_destroy((void*)&arrayList);
        return arrayList;
    }
    arrayList->dynamicContainer.container.header |= ObjectFlagMutexExists;
    return arrayList;
}

void arrayListDestroy(void* arrayList) {
    if (!bl_container_dynamic_is_valid(&((BL_ArrayList*)arrayList)->dynamicContainer)) {
        bl_mutex_destroy(&((BL_ArrayList*) arrayList)->mutex);
        bl_container_destroy(&((BL_ArrayList*)arrayList)->dynamicContainer.container);
    }
}

void arrayListDestroyWithElements(BL_ArrayList* arrayList, void(elementDestructor)(void* element)) {
    bl_mutex_destroy(&arrayList->mutex);
    bl_container_dynamic_destroy_with_elements(&arrayList->dynamicContainer, elementDestructor);
}