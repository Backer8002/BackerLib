#ifndef ArrayList_h
#define ArrayList_h

#include "BL_DynamicContainer.h"
#include "../Concurrency/ConcurrencyDefines.h"
#include <stddef.h>



#ifdef __cplusplus
extern "C" {
#else
#define noexcept
#endif
    /**
     * @brief A threadsafe implementation of DynamicContainer. Can work as a dynamic container, but will not lock the mutex then.
     */
    typedef struct BL_ArrayList {
            BL_DynamicContainer dynamicContainer;
            BL_Mutex            mutex;
    } BL_ArrayList;

    /**
     * @brief Sets amount of elements used to 0 and resizes to 0 elements.
     * @param arrayList Pointer to valid ArrayList
     * @return false if failed to lock mutex.
     */
    extern bool bl_arraylist_clear(BL_ArrayList* arrayList) noexcept;
    /**
     * @brief
     * @param arrayList Pointer to valid ArrayList
     * @param index index in ArrayList to get from
     * @param sizeOfElement Size of element which will store the element from the ArrayList
     * @param element Container to store element in
     * @return ContainerInvalidIndex if index is out of range
     * @return ContainerInvalidSize if sizeOfElement is larger than a single element in the array.
     * @return ContainerOPUnsuccessful if mutex could not be locked.
     */
    extern BL_ContainerError bl_arraylist_get(BL_ArrayList* arrayList, size_t index, size_t sizeOfElement, void* element) noexcept;
    /**
     * @brief Removes the last element from the ArrayList.
     * @param arrayList Pointer to valid ArrayList
     * @return ContainerInvalidIndex if there was no element to pop.
     * @return ContainerOPUnsuccessful if mutex could not be locked.
     */
    extern BL_ContainerError bl_arraylist_pop(BL_ArrayList* arrayList) noexcept;
    /**
     * @brief Removes elements from ArrayList given a range.
     * @param arrayList Pointer to valid ArrayList
     * @param index First index in removal range (inclusive)
     * @param lastIndex Last index in removal range (inclusive)
     * @return ContainerInvalidIndex if index is larger than lastIndex or if the indexes are invalid
     * @return ContainerOPUnsuccessful if mutex could not be locked.
     */
    extern BL_ContainerError bl_arraylist_remove(BL_ArrayList* arrayList, size_t index, size_t lastIndex) noexcept;
    /**
     * @brief Insert elements into ArrayList.
     * @param arrayList Pointer to valid ArrayList
     * @param index Index to start inserting from
     * @param amountOfElements Amount of elements to insert
     * @param sizeOfElement Size of individual element
     * @param elements List of elements to insert
     * @return ContainerInvalidIndex if index was larger than the size of the array.
     * @return ContainerInvalidSize if the sizeOfElement was larger than the size of a single element in the array.
     * @return ContainerAllocFailure if the array cannot grow for the new elements.
     * @return ContainerOPUnsuccessful if mutex could not be locked.
     */
    extern BL_ContainerError bl_arraylist_insert(BL_ArrayList* arrayList, size_t index, size_t amountOfElements, size_t sizeOfElement, const void* elements) noexcept;
    /**
     * @brief Sets index in ArrayList to element. If element is smaller than a single element in the array junk values may exist after size of element.
     * @param arrayList Pointer to valid ArrayList object
     * @param index index in container to set
     * @param elementSize size of element to set
     * @param element element used to set
     * @return ContainerOPSuccessful if valid operation
     * @return ContainerInvalidIndex if index is out of bounds
     * @return ContainerInvalidSize if element was larger than a single element in array
     * @return ContainerOPUnsuccessful if mutex could not be locked.
     */
    extern BL_ContainerError bl_arraylist_set(BL_ArrayList* arrayList, size_t index, size_t elementSize, const void* element) noexcept;
    /**
     * @brief Creates a ArrayList on the stack. Use is_valid to check validity.
     * @param initialSize Initial size of internal array
     * @param elementSize Size of the largest element to be stored in array
     *
     */
    extern BL_ArrayList bl_arraylist_create_stack(size_t initialSize, size_t elementSize) noexcept;
    /**
     * @brief Creates an ArrayList on the heap.
     * @param initialSize Initial size of internal array
     * @param elementSize Size of the largest element to be stored in array
     * @return NULL if allocation failed.
     */
    extern BL_ArrayList* bl_arraylist_create_heap(size_t initialSize, size_t elementSize) noexcept;
    /**
     * @brief Frees arraylist if applicable and puts it in an invalid state.
     * @param arrayList Pointer to ArrayList
     */
    extern void bl_arraylist_destroy(void* arrayList) noexcept;
    /**
     * @brief Frees ArrayList and executes destructor on each element.
     * @param arrayList Pointer to valid ArrayList
     * @param elementDestructor Destructor that will be executed on each element. Must be a valid reference
     */
    extern void bl_arraylist_destroy_with_elements(BL_ArrayList* arrayList, void(elementDestructor)(void* element)) noexcept;
    /**
        @brief Returns true if arraylist is valid.
    */
    bool bl_arraylist_is_valid(const BL_ArrayList* arrayList) noexcept;

#ifdef __cplusplus
    }
#else
#undef noexcept
#endif
#endif
