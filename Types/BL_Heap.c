#include "BL_Heap.h"
#include <stdlib.h>

static void internal_swap(size_t sizeOfElements, void* a, void* b) {
    if (a == b)
        return;
    for (size_t i = 0; i < sizeOfElements; i++) {
        *((BL_Bytes) a + i) ^= *((BL_Bytes) b + i);
        *((BL_Bytes) b + i) ^= *((BL_Bytes) a + i);
        *((BL_Bytes) a + i) ^= *((BL_Bytes) b + i);
    }
}

const void* bl_heap_top(const BL_Heap* heap) {
    if (heap->dynamicContainer.container.amountOfIndexes == 0)
        return NULL;
    return heap->dynamicContainer.container.array;
}

static void internal_bubble_down(BL_Container* container, size_t sizeOfHeap, bool (*compare)(const void*, const void*)) {
    size_t currentIndex = 1;
    while (currentIndex * 2 < sizeOfHeap) {
        size_t bestSuitedIndex = (compare((BL_Bytes) container->array + (currentIndex * 2 - 1) * container->byteSizeOfSingleElement, (BL_Bytes) container->array + currentIndex * 2 * container->byteSizeOfSingleElement))
                                   ? currentIndex * 2 + 1
                                   : currentIndex * 2;
        if (!compare((BL_Bytes) container->array + (bestSuitedIndex - 1) * container->byteSizeOfSingleElement, (BL_Bytes) container->array + (currentIndex - 1) * container->byteSizeOfSingleElement)) {
            internal_swap(container->byteSizeOfSingleElement, (BL_Bytes) container->array + (currentIndex - 1) * container->byteSizeOfSingleElement,
                          (BL_Bytes) container->array + (bestSuitedIndex - 1) * container->byteSizeOfSingleElement);
        }
        currentIndex = bestSuitedIndex;
    }

    if (currentIndex * 2 == sizeOfHeap) {
        if (!compare((BL_Bytes) container->array + (currentIndex * 2 - 1) * container->byteSizeOfSingleElement, (BL_Bytes) container->array + (currentIndex - 1) * container->byteSizeOfSingleElement))
            internal_swap(container->byteSizeOfSingleElement,
                          (BL_Bytes) container->array + (currentIndex - 1) * container->byteSizeOfSingleElement,
                          (BL_Bytes) container->array + (currentIndex * 2 - 1) * container->byteSizeOfSingleElement);
    }
}
void bl_heap_pop(BL_Heap* heap) {
    if (heap->dynamicContainer.container.amountOfIndexes == 0)
        return;
    internal_swap(heap->dynamicContainer.container.byteSizeOfSingleElement,
                  heap->dynamicContainer.container.array,
                  (BL_Bytes) heap->dynamicContainer.container.array + heap->dynamicContainer.container.byteSizeOfSingleElement * (heap->dynamicContainer.container.amountOfIndexes - 1));
    bl_container_dynamic_pop((BL_DynamicContainer*) heap);
    internal_bubble_down((BL_Container*) heap,heap->dynamicContainer.container.amountOfIndexes, heap->compare);
}

static void internal_bubble_up(BL_Container* container, size_t currentIndex, bool (*compare)(const void*, const void*)) {
    while (currentIndex > 1) {
        if (!compare((BL_Bytes) container->array + (currentIndex / 2 - 1) * container->byteSizeOfSingleElement, (BL_Bytes) container->array + (currentIndex - 1) * container->byteSizeOfSingleElement))
            break;
        internal_swap(container->byteSizeOfSingleElement, (BL_Bytes) container->array + (currentIndex / 2 - 1) * container->byteSizeOfSingleElement, (BL_Bytes) container->array + (currentIndex - 1) * container->byteSizeOfSingleElement);
        currentIndex /= 2;
    }
}

BL_ContainerError bl_heap_insert(BL_Heap* heap, size_t sizeOfElement, const void* restrict element) {
    if (sizeOfElement > heap->dynamicContainer.container.byteSizeOfSingleElement)
        return BL_ContainerInvalidSize;

    BL_ContainerError errorCode = bl_container_dynamic_append((BL_DynamicContainer*) heap, sizeOfElement, element);
    if (errorCode != BL_ContainerOPSuccessful)
        return errorCode;

    internal_bubble_up(&heap->dynamicContainer.container, heap->dynamicContainer.container.amountOfIndexes, heap->compare);
    return BL_ContainerOPSuccessful;
}


void bl_sort_heap(BL_Container* container, bool (*compare)(const void*, const void*)) {
    if (container->header & ObjectFlagArrayNoSort)
        return;
    for (size_t i = 1; i <= container->amountOfIndexes; i++)
        internal_bubble_up(container, i, compare);
    for (size_t i = 1; i < container->amountOfIndexes; i++) {
        internal_swap(container->byteSizeOfSingleElement, container->array, (BL_Bytes) container->array + container->byteSizeOfSingleElement * (container->amountOfIndexes - i));
        internal_bubble_down(container, container->amountOfIndexes - i,compare);
    }
}

BL_Heap bl_heap_create_stack(size_t elementSize, bool (*compare)(const void*, const void*)) {
    BL_Heap heap = {.dynamicContainer = bl_container_dynamic_create_stack(0, elementSize), .compare = compare};
    heap.dynamicContainer.container.header |= ObjectFlagIsNotContinuous | ObjectFlagArrayNoSort;
    return heap;
}

BL_Heap* bl_heap_create_heap(size_t elementSize, bool (*compare)(const void*, const void*)) {
    BL_Heap* heap = malloc(sizeof *heap);
    if (!heap)
        return NULL;

    heap->dynamicContainer = bl_container_dynamic_create_stack(0, elementSize);
    if (bl_container_dynamic_is_valid(&heap->dynamicContainer)) {
        free(heap);
        return NULL;
    }

    heap->dynamicContainer.container.header |= ObjectFlagIsOnHeap | ObjectFlagIsNotContinuous | ObjectFlagArrayNoSort;
    heap->compare = compare;
    return heap;
}

BL_Heap bl_heap_cast_container_dynamic(BL_DynamicContainer container,bool(*compare)(const void*, const void*)) {
    container.container.header |= ObjectFlagIsNotContinuous | ObjectFlagArrayNoSort;
    return (BL_Heap){.dynamicContainer = container, .compare = compare};
}

bool bl_heap_is_valid(const BL_Heap* heap) {
    return bl_container_dynamic_is_valid(&heap->dynamicContainer) && heap->dynamicContainer.container.header & ObjectFlagArrayNoSort && heap->dynamicContainer.container.header & ObjectFlagIsNotContinuous;
}

void    bl_heap_destroy(void* heap) {
    bl_container_dynamic_destroy(&((BL_Heap*)heap)->dynamicContainer);
}
