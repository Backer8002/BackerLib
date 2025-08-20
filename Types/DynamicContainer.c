#include "DynamicContainer.h"

#include "Container.h"
#include "TypesMain.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static void    internal_containerDynamicInit(DynamicContainer* container, size_t initialSize, size_t elementSize, bool elementsArePointers);

ContainerError containerDynamicSizeCheckAdd(DynamicContainer* container) {
    if (container->container.amountOfIndexes >= container->containerMaxSize) {
        size_t indexesToAssign = container->container.amountOfIndexes * 2 + 1;
        void*  newPointer      = realloc(container->container.array, container->container.byteSizeOfSingleElement * indexesToAssign);
        if (newPointer == NULL)
            return ContainerAllocFailure;
        container->container.array  = newPointer;
        container->containerMaxSize = indexesToAssign;
    }
    return ContainerOPSuccessful;
}

ContainerError containerDynamicSizeCheckRemove(DynamicContainer* container) {
    if (container->containerMaxSize >> 2 > container->container.amountOfIndexes) {
        void* newPointer = realloc(container->container.array, container->container.byteSizeOfSingleElement * container->container.amountOfIndexes);
        if (newPointer == NULL)
            return ContainerAllocFailure;
        container->container.array           = newPointer;
        container->containerMaxSize = container->container.amountOfIndexes;
    }
    return ContainerOPSuccessful;
}

ContainerError containerDynamicReserve(DynamicContainer* container, size_t amountOfIndexesToReserve) {
    if (container->containerMaxSize - container->container.amountOfIndexes >= amountOfIndexesToReserve)
        return ContainerOPSuccessful;

    size_t newAmountOfIndexes = amountOfIndexesToReserve + container->container.amountOfIndexes;
    void*  newArray           = realloc(container->container.array, newAmountOfIndexes * container->container.byteSizeOfSingleElement);
    if (!newArray)
        return ContainerAllocFailure;
    container->container.array  = newArray;
    container->containerMaxSize = newAmountOfIndexes;
    return ContainerOPSuccessful;
}

inline void containerDynamicClear(DynamicContainer* container) {
    container->container.amountOfIndexes = 0;
}

ContainerError containerDynamicPop(DynamicContainer* container) {
    if (container->container.amountOfIndexes == 0)
        return ContainerInvalidIndex;
    container->container.amountOfIndexes--;
    containerDynamicSizeCheckRemove(container);
    return ContainerOPSuccessful;
}

ContainerError containerDynamicRemove(DynamicContainer* container, size_t index, size_t lastIndex) {
    if (index > lastIndex)
        return ContainerInvalidIndex;
    if (lastIndex >= container->container.amountOfIndexes)
        return ContainerInvalidIndex;
    if (lastIndex != container->container.amountOfIndexes - 1) {
        void*  currentIndex        = (Bytes) container->container.array + index * container->container.byteSizeOfSingleElement;
        void*  nextIndex           = (Bytes) container->container.array + (lastIndex + 1) * container->container.byteSizeOfSingleElement;
        size_t amountOfBytesToMove = (container->container.amountOfIndexes - lastIndex - 1) * container->container.byteSizeOfSingleElement;
        memmove(currentIndex, nextIndex, amountOfBytesToMove);
    }
    container->container.amountOfIndexes -= lastIndex - index + 1;
    containerDynamicSizeCheckRemove(container);
    return ContainerOPSuccessful;
}

ContainerError containerDynamicInsert(DynamicContainer* container, size_t index, size_t amountOfElements, size_t sizeOfElement, const void* elements) {
    if (index > container->container.amountOfIndexes)
        return ContainerInvalidIndex;

    if (sizeOfElement > container->container.byteSizeOfSingleElement)
        return ContainerInvalidSize;

    if (container->header & ObjectFlagElementsArePointers && sizeOfElement != container->container.byteSizeOfSingleElement)
        return ContainerInvalidSize;

    container->container.amountOfIndexes += amountOfElements;
    if (containerDynamicSizeCheckAdd(container) == ContainerAllocFailure) {
        container->container.amountOfIndexes -= amountOfElements;
        return ContainerAllocFailure;
    }

    memmove((Bytes) container->container.array + (index + amountOfElements) * container->container.byteSizeOfSingleElement,
            (Bytes) container->container.array + index * container->container.byteSizeOfSingleElement,
            (container->container.amountOfIndexes - amountOfElements - index) * container->container.byteSizeOfSingleElement);
    for (size_t i = 0; i < amountOfElements; i++) {
        memcpy((Bytes) container->container.array + container->container.byteSizeOfSingleElement * (index + i),
               (Bytes) elements + sizeOfElement * i,
               sizeOfElement);
    }
    return ContainerOPSuccessful;
}

ContainerError containerDynamicInsertContainer(DynamicContainer* restrict container, size_t index, const Container* restrict containerToInsert) {
    if (index > container->container.amountOfIndexes)
        return ContainerInvalidIndex;

    if (containerToInsert->byteSizeOfSingleElement > container->container.byteSizeOfSingleElement)
        return ContainerInvalidSize;

    container->container.amountOfIndexes += containerToInsert->amountOfIndexes;
    if (containerDynamicSizeCheckAdd(container) == ContainerAllocFailure) {
        container->container.amountOfIndexes -= containerToInsert->amountOfIndexes;
        return ContainerAllocFailure;
    }

    memmove((Bytes) container->container.array + (index + containerToInsert->amountOfIndexes) * container->container.byteSizeOfSingleElement,
            (Bytes) container->container.array + index * container->container.byteSizeOfSingleElement,
            (container->container.amountOfIndexes - containerToInsert->amountOfIndexes - index) * container->container.byteSizeOfSingleElement);
    for (size_t i = 0; i < containerToInsert->amountOfIndexes; i++) {
        memcpy((Bytes) container->container.array + container->container.byteSizeOfSingleElement * (index + i),
               (Bytes) containerToInsert->array + containerToInsert->byteSizeOfSingleElement * i,
               containerToInsert->byteSizeOfSingleElement);
    }
    return ContainerOPSuccessful;
}

inline ContainerError containerDynamicAppend(DynamicContainer* container, size_t sizeOfElement, const void* element) {
    return containerDynamicInsert(container,
                                  container->container.amountOfIndexes,
                                  1,
                                  (container->header & ObjectFlagElementsArePointers) ? sizeof(&element) : sizeOfElement,
                                  (container->header & ObjectFlagElementsArePointers) ? &element : element);
}

static void internal_containerDynamicInit(DynamicContainer* container, size_t initialSize, size_t elementSize, bool elementsArePointers) {
    container->container.amountOfIndexes         = 0;
    container->header                            = 0;
    container->container.byteSizeOfSingleElement = elementsArePointers ? sizeof(void*) : elementSize;
    container->containerMaxSize                  = initialSize;

    container->container.array                   = malloc(elementSize * initialSize);
    if (container->container.array == NULL)
        return;

    container->header = (elementsArePointers ? ObjectFlagElementsArePointers : 0) | ObjectFlagIsValid | ObjectFlagIsDynamicContainer | ObjectFlagIsContainer;
}

DynamicContainer containerDynamicCreateStack(size_t initialSize, size_t elementSize, bool elementsArePointers) {
    DynamicContainer container = {0};
    internal_containerDynamicInit(&container, initialSize, elementSize, elementsArePointers);
    return container;
}

DynamicContainer* containerDynamicCreateHeap(size_t initialSize, size_t elementSize, bool elementsArePointers) {
    DynamicContainer* container = malloc(sizeof(*container));
    if (container == NULL)
        return NULL;
    internal_containerDynamicInit(container, initialSize, elementSize, elementsArePointers);
    if (isValidObject((DataTypeFlags*) container)) {
        container->header |= ObjectFlagIsOnHeap;
        return container;
    }
    free(container);
    return NULL;
}

void containerDynamicDestroyWithElements(DynamicContainer* container, void(elementDestructor)(void* element)) {
    for (size_t i = 0; i < container->container.amountOfIndexes; i++) {
        elementDestructor((container->header & ObjectFlagElementsArePointers)
                              ? ((Bytes*) container->container.array)[i]
                              : (Bytes) container->container.array + container->container.byteSizeOfSingleElement * i);
    }
    containerDestroy(container);
}
