#include "BL_UnorderedContainer.h"

#include "BL_Container.h"
#include "TypesMain.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//Uncheck since checks should already be in place.
static inline void internal_bitset_add(uint64_t* bitset, size_t index) {
    bitset[index/(sizeof *bitset * 8)] |= (uint64_t)INT64_MIN >> (index % (8 * sizeof *bitset));
}

//Uncheck since checks should already be in place.
static inline bool internal_bitset_get(const uint64_t* bitset, size_t index) {
    return (bitset[index/(sizeof *bitset * 8)] & ((uint64_t)INT64_MIN >> (index % (8 * sizeof *bitset)))) ? true : false;
}

//Unchecked since checks should already be in place
static inline void internal_bitset_remove(uint64_t* bitset,size_t index) {
    bitset[index/(sizeof *bitset * 8)] &= ~((uint64_t)INT64_MIN >> (index % (8 * sizeof *bitset)));
}

static void          internal_unordered_container_init(BL_UnorderedContainer* container, size_t initialSize, size_t sizeOfElements);

BL_UnorderedContainerPutResult bl_unordered_container_put(BL_UnorderedContainer* container, size_t sizeOfElement, const void* element) {
    BL_UnorderedContainerPutResult result = {.resultCode = BL_ContainerOPSuccessful, .locationOfElement = 0};
    if (sizeOfElement > container->container.byteSizeOfSingleElement) {
        result.resultCode = BL_ContainerInvalidSize;
        return result;
    }

    if (container->container.amountOfIndexes == container->maxSize) {
        void* newArr = realloc(container->container.array, (container->maxSize * 2 + 1) * container->container.byteSizeOfSingleElement);
        if (!newArr) {
            result.resultCode = BL_ContainerAllocFailure;
            return result;
        }

        container->container.array = newArr;

        uint64_t* newBitSet         = realloc(container->bitset, sizeof *container->bitset * (container->maxSize * 2 + 8 * sizeof*container->bitset) / (8 * sizeof *container->bitset)); // Ceiling division used ((container->maxSize * 2 + 1) + 7) / 8
        if (!newBitSet) {
            result.resultCode = BL_ContainerAllocFailure;
            return result;
        }

        container->bitset = newBitSet;
        memset(container->bitset + (container->maxSize + 8 * sizeof *container->bitset - 1) / (8 * sizeof *container->bitset),
            0,
            sizeof *container->bitset * ((container->maxSize * 2 + 8 * sizeof *container->bitset) / (8 * sizeof *container->bitset) - (container->maxSize + 8 * sizeof *container->bitset - 1) / (8 * sizeof * container->bitset)));
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
            if (!internal_bitset_get(container->bitset, bytesIterator * 8 * sizeof *container->bitset + i)) {
                internal_bitset_add(container->bitset, bytesIterator * 8 * sizeof *container->bitset + i);
                availableIndex = bytesIterator * 8 * sizeof *container->bitset + i;
                break;
            }
        }
        break;
    }
    memcpy((BL_Bytes) container->container.array + container->container.byteSizeOfSingleElement * availableIndex, element, sizeOfElement);
    container->container.amountOfIndexes++;
    result.locationOfElement = availableIndex;
    return result;
}

BL_ContainerError bl_unordered_container_set(BL_UnorderedContainer* container, size_t index, size_t sizeOfElement, const void* element) {
    if (sizeOfElement > container->container.byteSizeOfSingleElement)
        return BL_ContainerInvalidSize;
    if (index >= container->maxSize)
        return BL_ContainerInvalidIndex;
    memcpy((BL_Bytes) container->container.array + container->container.byteSizeOfSingleElement * index, element, sizeOfElement);
    internal_bitset_add(container->bitset, index);
    container->container.amountOfIndexes++;
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_unordered_container_set_try(BL_UnorderedContainer* container, size_t index, size_t sizeOfElement, const void* element) {
    if (index >= container->maxSize)
        return BL_ContainerInvalidIndex;
    if (internal_bitset_get(container->bitset, index))
        return BL_ContainerOPUnsuccessful;
    return bl_unordered_container_set(container, index, sizeOfElement, element);
}

void* bl_unordered_container_get(const BL_UnorderedContainer* container, size_t index) {
    if (container->maxSize <= index)
        return NULL;
    if (!internal_bitset_get(container->bitset, index))
        return NULL;
    return (BL_Bytes) container->container.array + index * container->container.byteSizeOfSingleElement;
}

BL_ContainerError bl_unordered_container_remove(BL_UnorderedContainer* container, size_t index, void (*destructor)(void* element)) {
    if (index >= container->container.amountOfIndexes)
        return BL_ContainerInvalidIndex;
    if (!internal_bitset_get(container->bitset, index))
        return BL_ContainerOPUnsuccessful;
    if (destructor)
        destructor((BL_Bytes) container->container.array + index * container->container.byteSizeOfSingleElement);
    internal_bitset_remove(container->bitset, index);
    container->container.amountOfIndexes--;
    return BL_ContainerOPSuccessful;
}

static void internal_unordered_container_init(BL_UnorderedContainer* container, size_t initialSize, size_t sizeOfElements) {
    if (initialSize == 0)
        initialSize = 1;
    container->container.header                            = 0;
    container->maxSize                           = initialSize;
    container->container.amountOfIndexes         = 0;
    container->container.byteSizeOfSingleElement = sizeOfElements;
    container->container.array                   = calloc(initialSize, container->container.byteSizeOfSingleElement);
    if (!container->container.array)
        return;
    container->bitset = calloc((initialSize + 8 * sizeof *container->bitset - 1) / (8 * sizeof *container->bitset), sizeof(*container->bitset));
    if (!container->bitset) {
        free(container->container.array);
        return;
    }
    container->container.header = ObjectFlagIsValid | ObjectFlagArrayNoSort | ObjectFlagIsContainer | ObjectFlagIsNotContinuous;
}

BL_UnorderedContainer bl_unordered_container_create_stack(size_t initialSize, size_t sizeOfElement) {
    BL_UnorderedContainer container;
    internal_unordered_container_init(&container, initialSize, sizeOfElement);
    return container;
}

BL_UnorderedContainer* bl_unordered_container_create_heap(size_t initialSize, size_t sizeOfElement) {
    BL_UnorderedContainer* container = malloc(sizeof(*container));
    if (!container)
        return NULL;
    internal_unordered_container_init(container, initialSize, sizeOfElement);
    if (!bl_unordered_container_is_valid(container)) {
        free(container);
        return NULL;
    }
    container->container.header |= ObjectFlagIsOnHeap;
    return container;
}

void* bl_unordered_container_front(const BL_UnorderedContainer* container) {
    if (container->container.array == NULL)
        return NULL;
    for (size_t i = 0; i < container->maxSize; i++) {
        if (internal_bitset_get(container->bitset,i))
            return bl_unordered_container_get(container,i);
    }
    return NULL;
}

void* bl_unordered_container_next(const BL_UnorderedContainer* container, const void* element) {
    size_t startingIndex = bl_container_index_from_reference((BL_Container*)container,element) + 1;
    for (size_t i = startingIndex; i < container->maxSize; i++) {
        if (internal_bitset_get(container->bitset,i))
            return bl_unordered_container_get(container,i);
    }
    return NULL;
}

void* bl_unordered_container_prev(const BL_UnorderedContainer* container, const void* element) {
    size_t startingIndex = bl_container_index_from_reference((BL_Container*)container,element) - 1;
    for (size_t i = startingIndex; i != SIZE_MAX; i--) {
        if (internal_bitset_get(container->bitset,i))
            return bl_unordered_container_get(container,i);
    }
    return NULL;
}

void* bl_unordered_container_back(const BL_UnorderedContainer* container) {
    if (container->container.array == NULL)
        return NULL;
    return bl_unordered_container_prev(container,(BL_Bytes)container->container.array + container->container.byteSizeOfSingleElement * container->container.amountOfIndexes);
}

void* bl_unordered_container_end(const BL_UnorderedContainer* container) {
    void* back = bl_unordered_container_back(container);
    if (back == NULL)
        return NULL;
    return (BL_Bytes)back + container->container.byteSizeOfSingleElement;
}

bool bl_unordered_container_is_valid(const BL_UnorderedContainer* container) {
    return container->container.header & ObjectFlagIsValid && container->container.header & ObjectFlagIsContainer && container->container.header & ObjectFlagIsNotContinuous;
}

size_t bl_unordered_container_size(const BL_UnorderedContainer* container) {
    return container->container.amountOfIndexes;
}

void bl_unordered_container_destroy(void* container) {
    if (bl_unordered_container_is_valid(container))
        return;
    free(((BL_UnorderedContainer*) container)->bitset);
    free(((BL_UnorderedContainer*) container)->container.array);
    if (((BL_UnorderedContainer*) container)->container.header & ObjectFlagIsOnHeap)
        free(container);
    else
        ((BL_UnorderedContainer*) container)->container.header &= ~ObjectFlagIsValid;
}

void bl_unordered_container_destroy_with_elements(BL_UnorderedContainer* container, void (*destructor)(void* element)) {
    for (size_t i = 0; i < container->maxSize; i++) {
        if (internal_bitset_get(container->bitset, i))
            destructor((BL_Bytes) container->container.array + i * container->container.byteSizeOfSingleElement);
    }
    bl_unordered_container_destroy(container);
}
