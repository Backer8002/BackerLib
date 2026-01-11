#ifndef HEAP_H
#define HEAP_H
#include "BL_Container.h"
#include "BL_DynamicContainer.h"

#ifdef __cplusplus
extern "C" {
#else
#define noexcept
#endif

    typedef struct BL_Heap {
        BL_DynamicContainer dynamicContainer;
        bool (*compare)(const void*, const void*);
    } BL_Heap;

    /**
     * @param heap Pointer to valid Heap
     * @return Pointer to array (the top of the heap) if there is any element in the heap otherwise returns NULL.
     */
    extern const void*       bl_heap_top(const BL_Heap* heap) noexcept;
    /**
     * @brief Removes the topmost element from the heap.
     * @param heap Pointer to valid heap
     */
    extern void              bl_heap_pop(BL_Heap* heap) noexcept;
    /**
     * @brief Inserts a new element into the heap.
     * @param heap Pointer to valid heap
     * @param sizeOfElement Size of element to insert
     * @param element Pointer to element
     * @return ContainerInvalidSize if sizeOfElement was greater than the largest element in the heap.
     * @return ContainerAllocFailure if the heap's array could not grow.
     */
    extern BL_ContainerError bl_heap_insert(BL_Heap* heap, size_t sizeOfElement, const void* element) noexcept;
    /**
     * @brief Uses heap sort to inplace sort the array with a given compare function.
     * Should return true if elements were in correct order.
     * Will do nothing to Containers with NoSortFlag
     * @param container Pointer to valid Container
     * @param compare Pointer to valid compare function
     */
    extern void              bl_sort_heap(BL_Container* container, bool (*compare)(const void*, const void*)) noexcept;
    /**
     * @brief Creates a Heap on the stack. Use isValidObject to check validity.
     * @param elementSize Size of element in Heap
     * @param compare Pointer to valid compare function. Should return true if elements were in correct order. second param is higher up the heap
     * @return Heap
     */
    extern BL_Heap           bl_heap_create_stack(size_t elementSize, bool (*compare)(const void*, const void*)) noexcept;
    /**
     * @brief Creates a Heap on the heap.
     * @param elementSize Size of element in Heap
     * @param compare Pointer to valid compare function. Should return true if elements were in correct order. second param is higher up the heap
     * @return NULL if object could not be allocated.
     */
    extern BL_Heap*          bl_heap_create_heap(size_t elementSize, bool (*compare)(const void*, const void*)) noexcept;
    /**
     * @brief Reinterperates container as a Heap. No insertion operations are performed
     * @param container DynamicContainer
     * @param compare Compare function
     * @return Returns Heap reinterpretation of container
     */
    extern BL_Heap           bl_heap_cast_container_dynamic(BL_DynamicContainer container, bool (*compare)(const void*, const void*)) noexcept;
    /**
     *
     * @param heap Pointer to Heap or NULL
     * @return true if heap is valid, else false
     */
    extern bool              bl_heap_is_valid(const BL_Heap* heap) noexcept;
    /**
     * @brief Destroys heap if necessary
     * @param heap Pointer to Heap
     */
    extern void              bl_heap_destroy(void* heap) noexcept;

#ifdef __cplusplus
}
#else
#undef noexcept
#endif
#endif // HEAP_H
