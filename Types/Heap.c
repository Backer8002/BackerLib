#include "Heap.h"
#include <stdlib.h>

static inline void internal_swap(size_t sizeOfElements, void* a, void* b) {
    if (a == b)
        return;
    for (size_t i = 0; i < sizeOfElements; i++) {
        *((Bytes) a + i) ^= *((Bytes) b + i);
        *((Bytes) b + i) ^= *((Bytes) a + i);
        *((Bytes) a + i) ^= *((Bytes) b + i);
    }
}

const void* heapTop(const Heap* heap) {
    if (heap->container.amountOfIndexes == 0)
        return NULL;
    return heap->container.array;
}

static void internal_bubbleDown(Container* container, size_t sizeOfHeap, bool (*compare)(const void*, const void*)) {
    size_t currentIndex = 1;
    while (currentIndex * 2 < sizeOfHeap) {
        size_t bestSuitedIndex = (compare((Bytes) container->array + (currentIndex * 2 - 1) * container->byteSizeOfSingleElement, (Bytes) container->array + currentIndex * 2 * container->byteSizeOfSingleElement))
                                   ? currentIndex * 2
                                   : currentIndex * 2 + 1;
        if (!compare((Bytes) container->array + (currentIndex - 1) * container->byteSizeOfSingleElement, (Bytes) container->array + (bestSuitedIndex - 1) * container->byteSizeOfSingleElement))
            internal_swap(container->byteSizeOfSingleElement, (Bytes) container->array + (currentIndex - 1) * container->byteSizeOfSingleElement,
                          (Bytes) container->array + (bestSuitedIndex - 1) * container->byteSizeOfSingleElement);
        currentIndex = bestSuitedIndex;
    }

    if (currentIndex * 2 == sizeOfHeap) {
        if (!compare((Bytes) container->array + (currentIndex - 1) * container->byteSizeOfSingleElement, (Bytes) container->array + (currentIndex * 2 - 1) * container->byteSizeOfSingleElement))
            internal_swap(container->byteSizeOfSingleElement,
                          (Bytes) container->array + (currentIndex - 1) * container->byteSizeOfSingleElement,
                          (Bytes) container->array + (currentIndex * 2 - 1) * container->byteSizeOfSingleElement);
    }
}
void heapPop(Heap* heap) {
    if (heap->container.amountOfIndexes == 0)
        return;
    internal_swap(heap->container.byteSizeOfSingleElement,
                  heap->container.array,
                  (Bytes) heap->container.array + heap->container.byteSizeOfSingleElement * (heap->container.amountOfIndexes - 1));
    containerDynamicPop((DynamicContainer*) heap);
    internal_bubbleDown((Container*) heap,heap->container.amountOfIndexes, heap->compare);
}

static void internal_bubbleUp(Container* container, size_t currentIndex, bool (*compare)(const void*, const void*)) {
    while (currentIndex > 1) {
        if (compare((Bytes) container->array + (currentIndex / 2 - 1) * container->byteSizeOfSingleElement, (Bytes) container->array + (currentIndex - 1) * container->byteSizeOfSingleElement))
            break;
        internal_swap(container->byteSizeOfSingleElement, (Bytes) container->array + (currentIndex / 2 - 1) * container->byteSizeOfSingleElement, (Bytes) container->array + (currentIndex - 1) * container->byteSizeOfSingleElement);
        currentIndex /= 2;
    }
}

ContainerError heapInsert(Heap* heap, size_t sizeOfElement, const void* element) {
    if (sizeOfElement > heap->container.byteSizeOfSingleElement)
        return ContainerInvalidSize;

    ContainerError errorCode = containerDynamicAppend((DynamicContainer*) heap, sizeOfElement, element);
    if (errorCode != ContainerOPSuccessful)
        return errorCode;

    internal_bubbleUp((Container*) heap, heap->container.amountOfIndexes, heap->compare);
    return ContainerOPSuccessful;
}

void heapSort(Container* container, bool (*compare)(const void*, const void*)) {
    if (container->header & ObjectFlagArrayNoSort)
        return;
    for (size_t i = 1; i <= container->amountOfIndexes; i++)
        internal_bubbleUp(container, i, compare);
    for (size_t i = 1; i < container->amountOfIndexes; i++) {
        internal_swap(container->byteSizeOfSingleElement, container->array, (Bytes) container->array + container->byteSizeOfSingleElement * (container->amountOfIndexes - i));
        internal_bubbleDown(container, container->amountOfIndexes - i,compare);
    }
}

Heap heapCreateStack(size_t elementSize, bool (*compare)(const void*, const void*)) {
    Heap heap = {.dynamicContainer = containerDynamicCreateStack(0, elementSize, false), .compare = compare};
    heap.header |= ObjectFlagIsNotContinuousCustomTracking | ObjectFlagArrayNoSort;
    return heap;
}

Heap* heapCreateHeap(size_t elementSize, bool (*compare)(const void*, const void*)) {
    Heap* heap = malloc(sizeof *heap);
    if (!heap)
        return NULL;

    heap->dynamicContainer = containerDynamicCreateStack(0, elementSize, false);
    if (!isValidObject((DataTypeFlags*) heap)) {
        free(heap);
        return NULL;
    }

    heap->header |= ObjectFlagIsOnHeap | ObjectFlagIsNotContinuousCustomTracking | ObjectFlagArrayNoSort;
    heap->compare = compare;
    return heap;
}
