#include "UnorderedContainer.h"
#include "TypesMain.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//Uncheck since checks should already be in place.
static inline void internal_bitsetAdd(uint64_t* bitset, size_t index) {
    bitset[index/(sizeof *bitset * 8)] |= (uint64_t)INT64_MIN >> (index % (8 * sizeof *bitset));
}

//Uncheck since checks should already be in place.
static inline bool internal_bitsetGet(const uint64_t* bitset, size_t index) {
    return (bitset[index/(sizeof *bitset * 8)] & ((uint64_t)INT64_MIN >> (index % (8 * sizeof *bitset)))) ? true : false;
}

//Unchecked since checks should already be in place
static inline void internal_bitsetRemove(uint64_t* bitset,size_t index) {
    bitset[index/(sizeof *bitset * 8)] &= ~((uint64_t)INT64_MIN >> (index % (8 * sizeof *bitset)));
}

static inline void          internal_unorderedContainerInit(UnorderedContainer* container, size_t initialSize, size_t sizeOfElements, bool elementsArePointers);

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

        uint64_t* newBitSet         = realloc(container->bitset, (container->maxSize * 2 + 8) / (8 * sizeof *container->bitset)); // Ceiling division used ((container->maxSize * 2 + 1) + 7) / 8
        if (!newBitSet) {
            result.resultCode = ContainerAllocFailure;
            return result;
        }

        container->bitset = newBitSet;
        memset(container->bitset + (container->maxSize + 8 * sizeof *container->bitset - 1) / (8 * sizeof *container->bitset), 0, (container->maxSize * 2 + 8 * sizeof *container->bitset) / (8 * sizeof *container->bitset) - (container->maxSize + 8 * sizeof *container->bitset - 1) / (8 * sizeof * container->bitset));
        container->maxSize += container->maxSize + 1;
    }

    size_t availableIndex = 0;
    for (size_t bytesIterator = 0; bytesIterator < (container->maxSize + 8 * sizeof*container->bitset - 1) / (8 * sizeof *container->bitset); bytesIterator++) {
        if (container->bitset[bytesIterator] == UINT64_MAX)
            continue;
        unsigned bitIterator = 0;
        if (container->bitset[bytesIterator] >> 32 == UINT32_MAX)
            bitIterator += 32;
        if (((container->bitset[bytesIterator] >> (48 - bitIterator)) & UINT16_MAX) == UINT16_MAX)
            bitIterator += 16;
        if (((container->bitset[bytesIterator] >> (56 - bitIterator)) & UINT8_MAX)== UINT8_MAX)
            bitIterator += 8;
        for (unsigned i = bitIterator; i < bitIterator + 8; i++) {
            if (internal_bitsetGet(container->bitset, bytesIterator * 8 * sizeof *container->bitset + i) == 0) {
                internal_bitsetAdd(container->bitset, bytesIterator * 8 * sizeof *container->bitset + i);
                availableIndex = bytesIterator * 8 * sizeof *container->bitset + i;
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
    internal_bitsetAdd(container->bitset, index);
    container->container.amountOfIndexes++;
    return ContainerOPSuccessful;
}

ContainerError unorderedContainerSetTry(UnorderedContainer* container, size_t index, size_t sizeOfElement, const void* element) {
    if (index >= container->maxSize)
        return ContainerInvalidIndex;
    if (internal_bitsetGet(container->bitset, index))
        return ContainerOPUnsuccessful;
    return unorderedContainerSet(container, index, sizeOfElement, element);
}

UnorderedContainerGetResult unorderedContainerGet(const UnorderedContainer* container, size_t index) {
    UnorderedContainerGetResult result = {.resultCode = ContainerOPSuccessful, .element = NULL};
    if (container->maxSize <= index) {
        result.resultCode = ContainerInvalidIndex;
        return result;
    }
    if (internal_bitsetGet(container->bitset, index) == 0) {
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
    if (internal_bitsetGet(container->bitset, index) == 0)
        return ContainerOPUnsuccessful;
    if (destructor)
        destructor((container->header & ObjectFlagElementsArePointers)
                       ? ((Bytes*) container->container.array)[index]
                       : (Bytes) container->container.array + index * container->container.byteSizeOfSingleElement);
    internal_bitsetRemove(container->bitset, index);
    container->container.amountOfIndexes--;
    return ContainerOPSuccessful;
}

static inline void internal_unorderedContainerInit(UnorderedContainer* container, size_t initialSize, size_t sizeOfElements, bool elementsArePointers) {
    container->header                            = 0;
    container->maxSize                           = initialSize;
    container->container.amountOfIndexes         = 0;
    container->container.byteSizeOfSingleElement = elementsArePointers ? sizeof(void*) : sizeOfElements;
    container->container.array                   = calloc(initialSize, container->container.byteSizeOfSingleElement);
    if (!container->container.array)
        return;
    container->bitset = calloc((initialSize + 8 * sizeof *container->bitset - 1) / (8 * sizeof *container->bitset), sizeof(*container->bitset));
    if (!container->bitset) {
        free(container->container.array);
        return;
    }
    container->header = (elementsArePointers ? ObjectFlagElementsArePointers : 0) | ObjectFlagIsValid | ObjectFlagArrayNoSort | ObjectFlagIsContainer | ObjectFlagIsNotContinuous;
}

UnorderedContainer unorderedContainerCreateStack(size_t initialSize, size_t sizeOfElements, bool elementsArePointers) {
    UnorderedContainer container;
    internal_unorderedContainerInit(&container, initialSize, sizeOfElements, elementsArePointers);
    return container;
}

UnorderedContainer* unorderedContainerCreateHeap(size_t initialSize, size_t sizeOfElements, bool elementsArePointers) {
    UnorderedContainer* container = malloc(sizeof(*container));
    if (!container)
        return NULL;
    internal_unorderedContainerInit(container, initialSize, sizeOfElements, elementsArePointers);
    if (!isValidObject((DataTypeFlags*) container)) {
        free(container);
        return NULL;
    }
    container->header |= ObjectFlagIsOnHeap;
    return container;
}

void unorderedContainerDestroy(void* container) {
    if (!isValidObject(container))
        return;
    if ((*(DataTypeFlags*) container & ObjectFlagIsNotContinuous) == 0)
        return;
    free(((UnorderedContainer*) container)->bitset);
    free(((UnorderedContainer*) container)->container.array);
    if (*(DataTypeFlags*) container & ObjectFlagIsOnHeap)
        free(container);
    else
        *(DataTypeFlags*) container &= ~(ObjectFlagIsValid);
}

void unorderedContainerDestroyWithElements(UnorderedContainer* container, void (*destructor)(void* element)) {
    for (size_t i = 0; i < container->maxSize; i++) {
        if (internal_bitsetGet(container->bitset, i))
            destructor(container->header & ObjectFlagElementsArePointers
                           ? ((Bytes*) container->container.array)[i]
                           : (Bytes) container->container.array + i * container->container.byteSizeOfSingleElement);
    }
    unorderedContainerDestroy(container);
}
