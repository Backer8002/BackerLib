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

void* bl_unordered_container_put(BL_UnorderedContainer* container, size_t sizeOfElement, const void* element) {
    if (sizeOfElement > container->byteSizeOfElement)
        return NULL;

    if (container->amountOfElements/BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT == container->amountOfPages) {
        void** newArr = (void**)realloc((void*)container->pages, (container->amountOfPages * 2 + 1) * sizeof *(container->pages));
        if (!newArr)
            return NULL;
        container->pages = newArr;
        size_t newSizeOfBitset = sizeof *container->bitset *(((container->amountOfPages * 2 + 1) * BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT + 8 * sizeof*container->bitset - 1) / (8 * sizeof *container->bitset));
        uint64_t* newBitSet         = realloc(container->bitset, newSizeOfBitset);
        if (!newBitSet)
            return NULL;
        container->bitset = newBitSet;

        for (size_t i = container->amountOfPages; i < container->amountOfPages*2 + 1; i++) {
            void* page = malloc((size_t)container->byteSizeOfElement * BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT);
            if (!page) {
                for (size_t j = container->amountOfPages; j < i; j++)
                    free(container->pages[j]);
                return NULL;
            }
            container->pages[i] = page;
        }

        memset(container->bitset + (container->amountOfPages * BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT + 8 * sizeof *container->bitset - 1) / (8 * sizeof *container->bitset),
            0,
            sizeof *container->bitset * (((container->amountOfPages * 2 + 1) * BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT + 8 * sizeof*container->bitset - 1) / (8 * sizeof *container->bitset) - (container->amountOfPages * BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT + 8 * sizeof *container->bitset - 1) / (8 * sizeof * container->bitset)));
        container->amountOfPages += container->amountOfPages + 1;
    }

    size_t availableIndex = 0;
    for (size_t bytesIterator = 0; bytesIterator < (container->amountOfPages * BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT + 8 * sizeof*container->bitset - 1) / (8 * sizeof *container->bitset); bytesIterator++) {
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
    void* object = (BL_Bytes) container->pages[availableIndex/BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT] + container->byteSizeOfElement * (availableIndex % BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT);
    memcpy(object, element, sizeOfElement);
    container->amountOfElements++;
    return object;
}

BL_ContainerError bl_unordered_container_set(BL_UnorderedContainer* container, size_t index, size_t sizeOfElement, const void* element) {
    if (sizeOfElement > container->byteSizeOfElement)
        return BL_ContainerInvalidSize;
    if (index >= container->amountOfPages * BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT)
        return BL_ContainerInvalidIndex;
    memcpy((BL_Bytes) container->pages[index/BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT] + container->byteSizeOfElement * (index % BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT), element, sizeOfElement);
    if (!internal_bitset_get(container->bitset,index)) {
        container->amountOfElements++;
        internal_bitset_add(container->bitset, index);
    }
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_unordered_container_set_try(BL_UnorderedContainer* container, size_t index, size_t sizeOfElement, const void* element) {
    if (index >= container->amountOfPages*BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT)
        return BL_ContainerInvalidIndex;
    if (internal_bitset_get(container->bitset, index))
        return BL_ContainerOPUnsuccessful;
    return bl_unordered_container_set(container, index, sizeOfElement, element);
}

void* bl_unordered_container_get(const BL_UnorderedContainer* container, size_t index) {
    if (container->amountOfPages * BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT <= index)
        return NULL;
    if (!internal_bitset_get(container->bitset, index))
        return NULL;
    return (BL_Bytes) container->pages[index/BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT] + (index % BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT) * container->byteSizeOfElement;
}

BL_ContainerError bl_unordered_container_remove(BL_UnorderedContainer* container, size_t index, void (*destructor)(void* element)) {
    if (index >= container->amountOfPages * BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT)
        return BL_ContainerInvalidIndex;
    if (!internal_bitset_get(container->bitset, index))
        return BL_ContainerOPUnsuccessful;
    if (destructor)
        destructor(bl_unordered_container_get(container,index));
    internal_bitset_remove(container->bitset, index);
    container->amountOfElements--;
    return BL_ContainerOPSuccessful;
}

static void internal_unordered_container_init(BL_UnorderedContainer* container, size_t initialSize, size_t sizeOfElements) {
    if (initialSize == 0)
        initialSize = 1;
    container->header                            = 0;
    container->amountOfPages                     = (initialSize + BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT - 1)/BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT;
    container->amountOfElements         = 0;
    container->byteSizeOfElement = sizeOfElements;
    container->pages                  = (void**)malloc(container->amountOfPages * sizeof *container->pages);
    if (!container->pages)
        return;
    container->bitset = calloc(((initialSize + BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT-1) / BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT * BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT + 8 * sizeof *container->bitset - 1) / (8 * sizeof *container->bitset), sizeof(*container->bitset));
    if (!container->bitset) {
        free((void*)container->pages);
        return;
    }

    for(size_t i = 0; i < container->amountOfPages; i++) {
        void* page = malloc(sizeOfElements * BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT);
        if(!page) {
            for(size_t j = 0; j < i; j++)
                free(container->pages[j]);
            free((void*)container->pages);
            free(container->bitset);
            return;
        }
        container->pages[i] = page;
    }

    container->header = ObjectFlagIsValid | ObjectFlagArrayNoSort | ObjectFlagIsNotContinuous;
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
    container->header |= ObjectFlagIsOnHeap;
    return container;
}

void* bl_unordered_container_front(const BL_UnorderedContainer* container) {
    if (container->pages[0] == NULL)
        return NULL;
    for (size_t i = 0; i < container->amountOfPages * BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT; i++) {
        if (internal_bitset_get(container->bitset,i))
            return bl_unordered_container_get(container,i);
    }
    return NULL;
}

void* bl_unordered_container_next(const BL_UnorderedContainer* container, const void* element) {
    size_t startingIndex = bl_container_index_from_reference((BL_Container*)container,element) + 1;
    for (size_t i = startingIndex; i < container->amountOfPages * BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT; i++) {
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
    if (container->pages[0] == NULL)
        return NULL;
    return bl_unordered_container_prev(container,(BL_Bytes)container->pages[container->amountOfPages-1] + (size_t)container->byteSizeOfElement * BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT);
}

void* bl_unordered_container_end(const BL_UnorderedContainer* container) {
    void* back = bl_unordered_container_back(container);
    if (back == NULL)
        return NULL;
    return (BL_Bytes)back + container->byteSizeOfElement;
}

bool bl_unordered_container_is_valid(const BL_UnorderedContainer* container) {
    return container->header & ObjectFlagIsValid  && container->header & ObjectFlagIsNotContinuous;
}

size_t bl_unordered_container_size(const BL_UnorderedContainer* container) {
    return container->amountOfElements;
}

size_t bl_unordered_container_index_from_ref(const BL_UnorderedContainer* container, const void* element) {
    for (size_t i = 0; i < container->amountOfPages; i++) {
        if (((uintptr_t)element - (uintptr_t)container->pages[i]) / container->byteSizeOfElement < BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT)
            return ((uintptr_t)element - (uintptr_t)container->pages[i]) / container->byteSizeOfElement + i * BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT;
    }
    return BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT * container->amountOfPages;
}

void bl_unordered_container_destroy(void* container) {
    if (bl_unordered_container_is_valid(container))
        return;
    free(((BL_UnorderedContainer*) container)->bitset);
    for (size_t i = 0; i < ((BL_UnorderedContainer*)container)->amountOfPages; i++)
        free(((BL_UnorderedContainer*)container)->pages[i]);
    free((void*)((BL_UnorderedContainer*) container)->pages);
    if (((BL_UnorderedContainer*) container)->header & ObjectFlagIsOnHeap)
        free(container);
    else
        ((BL_UnorderedContainer*) container)->header &= ~ObjectFlagIsValid;
}

void bl_unordered_container_destroy_with_elements(BL_UnorderedContainer* container, void (*destructor)(void* element)) {
    for (size_t i = 0; i < container->amountOfPages*BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT; i++) {
        if (internal_bitset_get(container->bitset, i))
            destructor(bl_unordered_container_get(container,i));
    }
    bl_unordered_container_destroy(container);
}
