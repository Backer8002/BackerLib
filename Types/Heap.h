#ifndef HEAP_H
#define HEAP_H
#include "Container.h"
#include "DynamicContainer.h"

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#endif

    typedef union Heap {
        struct {
            DataTypeFlags header;
        };
        struct {
            Container     container;
        };
        struct {
            DynamicContainer dynamicContainer;
            bool (*compare)(const void*, const void*);
        };
    } Heap;

    /**
     * @param heap Pointer to valid Heap
     * @return Pointer to array (the top of the heap) if there is any element in the heap otherwise returns NULL.
     */
    extern const void*    heapTop(const Heap* heap);
    /**
     * @brief Removes the topmost element from the heap.
     * @param heap Pointer to valid heap
     */
    extern void           heapPop(Heap* heap);
    /**
     * @brief Inserts a new element into the heap.
     * @param heap Pointer to valid heap
     * @param sizeOfElement Size of element to insert
     * @param element Pointer to element
     * @return ContainerInvalidSize if sizeOfElement was greater than the largest element in the heap.
     * @return ContainerAllocFailure if the heap's array could not grow.
     */
    extern ContainerError heapInsert(Heap* heap, size_t sizeOfElement, const void* element);
    /**
     * @brief Uses heap sort to inplace sort the array with a given compare function.
     * Should return true if elements were in correct order.
     * Will do nothing to Containers with NoSortFlag
     * @param container Pointer to valid Container
     * @param compare Pointer to valid compare function
     */
    extern void           heapSort(Container* container, bool (*compare)(const void*, const void*));
    /**
     * @brief Creates a Heap on the stack. Use isValidObject to check validity.
     * @param elementSize Size of element in Heap
     * @param compare Pointer to valid compare function. Should return true if elements were in correct order. second param is higher up the heap
     * @return Heap
     */
    extern Heap           heapCreateStack(size_t elementSize, bool (*compare)(const void*, const void*));
    /**
     * @brief Creates a Heap on the heap.
     * @param elementSize Size of element in Heap
     * @param compare Pointer to valid compare function. Should return true if elements were in correct order. second param is higher up the heap
     * @return NULL if object could not be allocated.
     */
    extern Heap*          heapCreateHeap(size_t elementSize, bool (*compare)(const void*, const void*));

/**
     * @brief Reinterperates container as a Heap. No insertion operations are performed
     * @param container DynamicContainer
     * @param compare Compare function
     * @return Returns Heap reinterpretation of container
     */
    static inline Heap containerDynamicToHeapReinterperate(DynamicContainer container,bool(*compare)(const void*, const void*)) {
        container.header |= ObjectFlagIsNotContinuousCustomTracking | ObjectFlagArrayNoSort;
        return (Heap){.dynamicContainer = container, .compare = compare};
    }

    /**
     * @brief Destroys heap if necessary
     * @param heap Pointer to Heap
     */
    static inline void    heapDestroy(void* heap) {
        containerDestroy(heap);
    }

#ifdef __cplusplus
    }
};
#endif
#endif // HEAP_H
