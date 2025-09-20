#include "Container.h"

#include <stdlib.h>
#include <string.h>

static void    internal_containerInit(Container* container, size_t size, size_t elementSize, bool elementsArePointers);

ContainerError containerSet(Container* container, size_t index, size_t elementSize, const void* restrict element) {
    if (container->amountOfIndexes <= index)
        return ContainerInvalidIndex;
    if (container->byteSizeOfSingleElement < elementSize && (container->header & ObjectFlagElementsArePointers) == 0)
        return ContainerInvalidSize;
    if (container->header & ObjectFlagElementsArePointers)
        ((Bytes*) container->array)[index] = (Bytes) element;
    else
        memcpy((Bytes) container->array + container->byteSizeOfSingleElement * index, element, elementSize);
    return ContainerOPSuccessful;
}

void* containerGet(const Container* container, size_t index) {
    if (container->amountOfIndexes <= index)
        return NULL;
    return container->header & ObjectFlagElementsArePointers
             ? ((Bytes*) container->array)[index]
             : (Bytes) container->array + index * container->byteSizeOfSingleElement;
}

static void internal_containerInit(Container* container, size_t size, size_t elementSize, bool elementsArePointers) {
    container->header = 0;
    container->array  = size ? calloc(size, elementSize) : NULL;
    if (!container->array && size)
        return;
    container->byteSizeOfSingleElement = elementsArePointers ? sizeof(void*) : elementSize;
    container->amountOfIndexes         = size;
    container->header                  = ObjectFlagIsValid | (elementsArePointers ? ObjectFlagElementsArePointers : 0) | ObjectFlagIsContainer;
}


Container containerGetSubArray(const Container* container, size_t firstIndex, size_t lastIndex, bool copyInReverse) {
    if (container->amountOfIndexes <= lastIndex || firstIndex > lastIndex) {
        Container newContainer = {0};
        return newContainer;
    }

    Container returnContainer = containerCreateStack(lastIndex - firstIndex + 1, container->byteSizeOfSingleElement, (container->header & ObjectFlagElementsArePointers) ? true : false);
    if (!isValidObject(&returnContainer.header))
        return returnContainer;


    for (size_t iterator = 0; iterator < lastIndex - firstIndex + 1; iterator++) {
        memcpy((Bytes) returnContainer.array + iterator * returnContainer.byteSizeOfSingleElement,
               (Bytes) container->array + (copyInReverse ? lastIndex - iterator : firstIndex + iterator) * returnContainer.byteSizeOfSingleElement,
               returnContainer.byteSizeOfSingleElement);
    }
    return returnContainer;
}

void containerReverse(Container* container) {
    if (container->amountOfIndexes == 0)
        return;

    uintptr_t front = (uintptr_t) container->array;
    uintptr_t end   = (uintptr_t) container->array + (container->amountOfIndexes - 1) * container->byteSizeOfSingleElement;

    while (front < end) {
        for (size_t i = 0; i < container->byteSizeOfSingleElement; i++) {
            *(Byte*) (front + i) ^= *(Byte*) (end + i);
            *(Byte*) (end + i) ^= *(Byte*) (front + i);
            *(Byte*) (front + i) ^= *(Byte*) (end + i);
        }
        front += container->byteSizeOfSingleElement;
        end -= container->byteSizeOfSingleElement;
    }
}

Container containerCreateStack(size_t size, size_t elementSize, bool elementsArePointers) {
    Container container = {0};
    internal_containerInit(&container, size, elementSize, elementsArePointers);
    return container;
}

Container* containerCreateHeap(size_t size, size_t elementSize, bool elementsArePointers) {
    Container* container = malloc(sizeof *container);
    if (!container)
        return NULL;

    internal_containerInit(container, size, elementSize, elementsArePointers);
    if (!isValidObject((DataTypeFlags*) container)) {
        free(container);
        return NULL;
    }

    container->header |= ObjectFlagIsOnHeap;
    return container;
}

void containerDestroy(void* container) {
    if (isValidObject(container)) {
        free(((Container*) container)->array);
        if ((*(DataTypeFlags*) container) & ObjectFlagIsOnHeap)
            free(container);
        else
            *(DataTypeFlags*) container &= ~ObjectFlagIsValid;
    }
}
