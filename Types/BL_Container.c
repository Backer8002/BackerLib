#include "BL_Container.h"

#include <stdlib.h>
#include <string.h>

static void    internal_containerInit(BL_Container* container, size_t size, size_t elementSize);

BL_ContainerError bl_container_set(BL_Container* container, size_t index, size_t elementSize, const void* restrict element) {
    if (container->amountOfIndexes <= index)
        return BL_ContainerInvalidIndex;
    if (container->byteSizeOfSingleElement < elementSize && (container->header & ObjectFlagElementsArePointers) == 0)
        return BL_ContainerInvalidSize;
    memcpy((BL_Bytes) container->array + container->byteSizeOfSingleElement * index, element, elementSize);
    return BL_ContainerOPSuccessful;
}

void* bl_container_get(const BL_Container* container, size_t index) {
    if (container->amountOfIndexes <= index)
        return NULL;
    return (BL_Bytes) container->array + index * container->byteSizeOfSingleElement;
}

static void internal_containerInit(BL_Container* container, size_t size, size_t elementSize) {
    container->header = 0;
    container->array  = size ? calloc(size, elementSize) : NULL;
    if (!container->array && size)
        return;
    container->byteSizeOfSingleElement = elementSize;
    container->amountOfIndexes         = size;
    container->header                  = ObjectFlagIsValid | ObjectFlagIsContainer;
}


BL_Container bl_container_get_subarray(const BL_Container* container, size_t firstIndex, size_t lastIndex, bool copyInReverse) {
    if (container->amountOfIndexes <= lastIndex || firstIndex > lastIndex) {
        BL_Container newContainer = {0};
        return newContainer;
    }

    BL_Container returnContainer = bl_container_create_stack(lastIndex - firstIndex + 1, container->byteSizeOfSingleElement);
    if (!bl_container_is_valid(container))
        return returnContainer;


    for (size_t iterator = 0; iterator < lastIndex - firstIndex + 1; iterator++) {
        memcpy((BL_Bytes) returnContainer.array + iterator * returnContainer.byteSizeOfSingleElement,
               (BL_Bytes) container->array + (copyInReverse ? lastIndex - iterator : firstIndex + iterator) * returnContainer.byteSizeOfSingleElement,
               returnContainer.byteSizeOfSingleElement);
    }
    return returnContainer;
}

BL_Container bl_container_copy(const BL_Container* container) {
    return bl_container_get_subarray(container,0,container->amountOfIndexes,false);
}

void bl_container_reverse(BL_Container* container) {
    if (container->amountOfIndexes < 2)
        return;

    uintptr_t front = (uintptr_t) container->array;
    uintptr_t end   = (uintptr_t) container->array + (container->amountOfIndexes - 1) * container->byteSizeOfSingleElement;

    while (front < end) {
        for (size_t i = 0; i < container->byteSizeOfSingleElement; i++) {
            *(BL_Byte*) (front + i) ^= *(BL_Byte*) (end + i);
            *(BL_Byte*) (end + i) ^= *(BL_Byte*) (front + i);
            *(BL_Byte*) (front + i) ^= *(BL_Byte*) (end + i);
        }
        front += container->byteSizeOfSingleElement;
        end -= container->byteSizeOfSingleElement;
    }
}

bool bl_container_is_empty(const BL_Container* container) {
    return container->amountOfIndexes == 0;
}

size_t bl_container_size(const BL_Container* container) {
    return container->amountOfIndexes;
}

size_t bl_container_index_from_reference(const BL_Container* container, const void* const reference) {
    return ((uintptr_t)reference - (uintptr_t)container->array)/container->byteSizeOfSingleElement;
}

void* bl_container_front(const BL_Container* container) {
    return container->array;
}

void* bl_container_next(const BL_Container* container, const void* element) {
    size_t nextIndex = bl_container_index_from_reference(container,element) + 1;
    return bl_container_get(container,nextIndex);
}

void* bl_container_prev(const BL_Container* container, const void* element) {
    size_t index = bl_container_index_from_reference(container, element);
    if (index == 0)
        return NULL;
    return bl_container_get(container, index - 1);
}

void* bl_container_back(const BL_Container* container) {
    return container->array ? (BL_Bytes)container->array + container->byteSizeOfSingleElement * (container->amountOfIndexes - 1) : NULL;
}

void* bl_container_end(const BL_Container* container) {
    return (BL_Bytes)container->array + container->amountOfIndexes * container->byteSizeOfSingleElement;
}

BL_Container bl_container_create_stack(size_t size, size_t elementSize) {
    BL_Container container = {0};
    internal_containerInit(&container, size, elementSize);
    return container;
}

BL_Container* bl_container_create_heap(size_t size, size_t elementSize) {
    BL_Container* container = malloc(sizeof *container);
    if (!container)
        return NULL;

    internal_containerInit(container, size, elementSize);
    if (!bl_container_is_valid(container)) {
        free(container);
        return NULL;
    }

    container->header |= ObjectFlagIsOnHeap;
    return container;
}

bool bl_container_is_valid(const BL_Container* container) {
    if (container->header & ObjectFlagIsValid && container->header & ObjectFlagIsContainer)
        return true;
    return false;
}

void bl_container_destroy(void* container) {
    if (bl_container_is_valid(container)) {
        free(((BL_Container*) container)->array);
        if (((BL_Container*) container)->header & ObjectFlagIsOnHeap)
            free(container);
        else
            (*(BL_Container*) container).header &= ~ObjectFlagIsValid;
    }
}
