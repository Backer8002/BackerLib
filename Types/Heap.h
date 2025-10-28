#ifndef HEAP_H
#define HEAP_H
#include "Container.h"
#include "DynamicContainer.h"

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#else
#define noexcept
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
    extern const void*    heapTop(const Heap* heap) noexcept;
    /**
     * @brief Removes the topmost element from the heap.
     * @param heap Pointer to valid heap
     */
    extern void           heapPop(Heap* heap) noexcept;
    /**
     * @brief Inserts a new element into the heap.
     * @param heap Pointer to valid heap
     * @param sizeOfElement Size of element to insert
     * @param element Pointer to element
     * @return ContainerInvalidSize if sizeOfElement was greater than the largest element in the heap.
     * @return ContainerAllocFailure if the heap's array could not grow.
     */
    extern ContainerError heapInsert(Heap* heap, size_t sizeOfElement, const void* element) noexcept;
    /**
     * @brief Uses heap sort to inplace sort the array with a given compare function.
     * Should return true if elements were in correct order.
     * Will do nothing to Containers with NoSortFlag
     * @param container Pointer to valid Container
     * @param compare Pointer to valid compare function
     */
    extern void           heapSort(Container* container, bool (*compare)(const void*, const void*)) noexcept;
    /**
     * @brief Creates a Heap on the stack. Use isValidObject to check validity.
     * @param elementSize Size of element in Heap
     * @param compare Pointer to valid compare function. Should return true if elements were in correct order. second param is higher up the heap
     * @return Heap
     */
    extern Heap           heapCreateStack(size_t elementSize, bool (*compare)(const void*, const void*)) noexcept;
    /**
     * @brief Creates a Heap on the heap.
     * @param elementSize Size of element in Heap
     * @param compare Pointer to valid compare function. Should return true if elements were in correct order. second param is higher up the heap
     * @return NULL if object could not be allocated.
     */
    extern Heap*          heapCreateHeap(size_t elementSize, bool (*compare)(const void*, const void*)) noexcept;

    /**
     * @brief Reinterperates container as a Heap. No insertion operations are performed
     * @param container DynamicContainer
     * @param compare Compare function
     * @return Returns Heap reinterpretation of container
     */
    extern Heap containerDynamicToHeapReinterperate(DynamicContainer container,bool(*compare)(const void*, const void*)) noexcept;

    /**
     * @brief Destroys heap if necessary
     * @param heap Pointer to Heap
     */
    extern void    heapDestroy(void* heap) noexcept;

#ifdef __cplusplus
    }
};
#endif
#endif // HEAP_H
