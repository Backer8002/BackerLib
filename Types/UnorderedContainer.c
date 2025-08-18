#include "UnorderedContainer.h"
#include "TypesMain.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define bitsetAdd(bitset, i)    ((bitset)[(i) / 8] |= (0x80 >> ((i) % 8)))
#define bitsetGet(bitset, i)    ((bitset)[(i) / 8] & (0x80 >> ((i) % 8)))
#define bitsetRemove(bitset, i) ((bitset)[(i) / 8] &= ~(0x80 >> ((i) % 8)))

static inline void internal_unorderedContainerInit(UnorderedContainer* container, size_t initialSize, size_t sizeOfElements, bool elementsArePointers);

UnorderedContainerPutResult unorderedContainerPut(UnorderedContainer* container, size_t sizeOfElement, const void* element) {
    UnorderedContainerPutResult result = {.resultCode = ContainerOPSuccessful, .locationOfElement = 0};
    if (sizeOfElement > container->container.byteSizeOfSingleElement && (container->header & ObjectFlagElementsArePointers) == 0) {
        result.resultCode = ContainerInvalidSize;
        return result;
    }

    if (container->container.amountOfIndexes == container->maxSize) {
        void* newArr = realloc(container->container.array, (container->maxSize * 2 + 1) * container->container.byteSizeOfSingleElement);
        if (!newArr) {
            result.resultCode = ContainerAllocFailure;
            return result;
        }

        container->container.array = newArr;

        uint8_t* newBitSet         = realloc(container->bitset, (container->maxSize * 2 + 8) / 8); // Ceiling division used ((container->maxSize * 2 + 1) + 7) / 8
        if (!newBitSet) {
            result.resultCode = ContainerAllocFailure;
            return result;
        }

        container->bitset = newBitSet;
        memset(container->bitset + (container->maxSize + 7) / 8, 0, (container->maxSize * 2 + 8) / 8 - (container->maxSize + 7) / 8);
        container->maxSize += container->maxSize + 1;
    }

    size_t availableIndex = 0;
    for (size_t byteIterator = 0; byteIterator < (container->maxSize + 7) / 8; byteIterator++) {
        if (container->bitset[byteIterator] == UINT8_MAX)
            continue;
        for (size_t bitIterator = 0; bitIterator < 8; bitIterator++) {
            if (!bitsetGet(container->bitset, byteIterator * 8 + bitIterator)) {
                bitsetAdd(container->bitset, byteIterator * 8 + bitIterator);
                availableIndex = byteIterator * 8 + bitIterator;
                break;
            }
        }
        break;
    }
    if (container->header & ObjectFlagElementsArePointers)
        ((const Byte**) container->container.array)[availableIndex] = (const Byte*) element;
    else
        memcpy((Bytes) container->container.array + container->container.byteSizeOfSingleElement * availableIndex, element, sizeOfElement);
    container->container.amountOfIndexes++;
    result.locationOfElement = availableIndex;
    return result;
}

ContainerError unorderedContainerSet(UnorderedContainer* container, size_t index, size_t sizeOfElement, const void* element) {
    if (sizeOfElement > container->container.byteSizeOfSingleElement && (container->header & ObjectFlagElementsArePointers) == 0)
        return ContainerInvalidSize;
    if (index >= container->maxSize)
        return ContainerInvalidIndex;
    if (container->header & ObjectFlagElementsArePointers)
        ((const Byte**) container->container.array)[index] = (const Byte*) element;
    else
        memcpy((Bytes) container->container.array + container->container.byteSizeOfSingleElement * index, element, sizeOfElement);
    bitsetAdd(container->bitset, index);
    container->container.amountOfIndexes++;
    return ContainerOPSuccessful;
}

ContainerError unorderedContainerSetTry(UnorderedContainer* container, size_t index, size_t sizeOfElement, const void* element) {
    if (bitsetGet(container->bitset, index))
        return ContainerOPUnsuccessful;
    return unorderedContainerSet(container,index,sizeOfElement,element);
}

UnorderedContainerGetResult unorderedContainerGet(const UnorderedContainer* container, size_t index) {
    UnorderedContainerGetResult result = {.resultCode = ContainerOPSuccessful, .element = NULL};
    if (container->maxSize <= index) {
        result.resultCode = ContainerInvalidIndex;
        return result;
    }
    if (!bitsetGet(container->bitset, index)) {
        result.resultCode = ContainerOPUnsuccessful;
        return result;
    }
    result.element = (container->header & ObjectFlagElementsArePointers)
                            ? ((Bytes*) container->container.array)[index]
                            : (Bytes) container->container.array + index * container->container.byteSizeOfSingleElement;
    return result;
}

ContainerError unorderedContainerRemove(UnorderedContainer* container, size_t index, void (*destructor)(void* element)) {
    if (index >= container->container.amountOfIndexes)
        return ContainerInvalidIndex;
    if (!bitsetGet(container->bitset, index))
        return ContainerOPUnsuccessful;
    if (destructor)
        destructor((container->header & ObjectFlagElementsArePointers)
                       ? ((Bytes*) container->container.array)[index]
                       : (Bytes) container->container.array + index * container->container.byteSizeOfSingleElement);
    bitsetRemove(container->bitset, index);
    container->container.amountOfIndexes--;
    return ContainerOPSuccessful;
}

static inline void internal_unorderedContainerInit(UnorderedContainer* container, size_t initialSize, size_t sizeOfElements, bool elementsArePointers) {
    container->header = 0;
    container->maxSize = initialSize;
    container->container.amountOfIndexes = 0;
    container->container.byteSizeOfSingleElement = elementsArePointers ? sizeof(void*) : sizeOfElements;
    container->container.array = calloc(initialSize,container->container.byteSizeOfSingleElement);
    if (!container->container.array)
        return;
    container->bitset = calloc((initialSize + 7) / 8, sizeof(*container->bitset));
    if (!container->bitset) {
        free(container->container.array);
        return;
    }
    container->header = (elementsArePointers ? ObjectFlagElementsArePointers : 0) | ObjectFlagIsValid | ObjectFlagArrayNoSort | ObjectFlagIsContainer | ObjectFlagIsNotContinuous;
}

UnorderedContainer unorderedContainerCreateStack(size_t initialSize, size_t sizeOfElements, bool elementsArePointers) {
    UnorderedContainer container;
    internal_unorderedContainerInit(&container,initialSize,sizeOfElements,elementsArePointers);
    return container;
}

UnorderedContainer* unorderedContainerCreateHeap(size_t initialSize, size_t sizeOfElements, bool elementsArePointers) {
    UnorderedContainer* container = malloc(sizeof(*container));
    if (!container)
        return NULL;
    internal_unorderedContainerInit(container,initialSize,sizeOfElements,elementsArePointers);
    if (!isValidObject((DataTypeFlags*)container)) {
        free(container);
        return NULL;
    }
    container->header |= ObjectFlagIsOnHeap;
    return container;
}

void unorderedContainerDestroy(void* container) {
    if (!isValidObject(container))
        return;
    if ((*(DataTypeFlags*)container & ObjectFlagIsNotContinuous) == 0)
        return;
    free(((UnorderedContainer*)container)->bitset);
    free(((UnorderedContainer*)container)->container.array);
    if (*(DataTypeFlags*)container & ObjectFlagIsOnHeap)
        free(container);
    else
        *(DataTypeFlags*)container &= ~(ObjectFlagIsValid);
}

void unorderedContainerDestroyWithElements(UnorderedContainer* container, void(*destructor)(void* element)) {
    for (size_t i = 0; i < container->maxSize; i++) {
        if (bitsetGet(container->bitset,i))
            destructor(container->header & ObjectFlagElementsArePointers
                ? ((Bytes*) container->container.array)[i] : (Bytes) container->container.array + i * container->container.byteSizeOfSingleElement);
    }
    unorderedContainerDestroy(container);
}