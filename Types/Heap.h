#ifndef HEAP_H
#define HEAP_H
#include "DynamicContainer.h"

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#endif

    typedef union Heap {
        DataTypeFlags header;
        Container     container;
        struct {
            DynamicContainer dynamicContainer;
            bool (*compare)(const void*, const void*);
        };
    } Heap;

    /**
     * @param heap Pointer to valid Heap
     * @return Pointer to array (the top of the heap) if there is any element in the heap otherwise returns NULL.
     */
    extern void*          heapTop(const Heap* heap);
    /**
     * @brief Removes the topmost element from the heap.
     * @param heap Pointer to valid heap
     */
    extern void           heapPop(Heap* heap);
    /**
     * @brief Inserts a new element into the heap.
     * @param heap Pointer to valid heap
     * @param sizeOfElement Size of element to insert. Must be the same size as an element in heap
     * @param element Pointer to element
     * @return ContainerInvalidSize if sizeOfElement was invalid.
     * @return ContainerAllocFailure if the heap's array could not grow.
     */
    extern ContainerError heapInsert(Heap* heap, size_t sizeOfElement, const void* element);
    /**
     * @brief Uses heap sort to inplace sort the array with a given compare function. compare should return true if elements were in correct order. Will do nothing to Containers with NoSortFlag
     * @param container Pointer to valid Container
     * @param compare Pointer to valid compare function
     */
    extern void           heapSort(Container* container, bool (*compare)(const void*, const void*));
    /**
     * @brief Creates a Heap on the stack. Use isValidObject to check validity.
     * @param elementSize Size of element in Heap
     * @param compare Pointer to valid compare function. Should return true if elements were in correct order
     * @return Heap
     */
    extern Heap           heapCreateStack(size_t elementSize, bool (*compare)(const void*, const void*));
    /**
     * @brief Creates a Heap on the heap.
     * @param elementSize Size of element in Heap
     * @param compare Pointer to valid compare function. Should return true if elements were in correct order
     * @return NULL if object could not be allocated.
     */
    extern Heap*          heapCreateHeap(size_t elementSize, bool (*compare)(const void*, const void*));

#ifdef __cplusplus
    }
};
#endif
#endif // HEAP_H
