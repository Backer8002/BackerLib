#ifndef DYNAMICCONTAINER_H
#define DYNAMICCONTAINER_H

#include "BL_Container.h"
#include "TypesMain.h"
#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#else
#define noexcept
#endif

    typedef struct BL_DynamicContainer {
        BL_Container container;
        size_t       containerMaxSize;
    } BL_DynamicContainer;

    /**
     *
     * @param container Pointer to valid DynamicContainer
     * @param index Index to set
     * @param elementSize Size of element
     * @param element Element to set
     * @return ContainerInvalidIndex if index was invalid, ContainerInvalidSize if elementSize was too large.
     */
    extern BL_ContainerError    bl_container_dynamic_set(BL_DynamicContainer* container, size_t index, size_t elementSize, const void* element) noexcept;
    /**
     *
     * @param container Pointer to valid DynamicContainer
     * @param index Index to get
     * @return NULL if index was invalid, else pointer to object.
     */
    extern void*                bl_container_dynamic_get(const BL_DynamicContainer* container, size_t index) noexcept;
    /**
     * @brief Returns subarray in range [firstIndex, lastIndex]
     * @param container Pointer to valid DynamicContainer
     * @param copyInReverse Should it copy in reverse
     * @return Invalid object if it could not be allocated or if lastIndex < firstIndex.
     */
    extern BL_DynamicContainer  bl_container_dynamic_get_subarray(const BL_DynamicContainer* container, size_t firstIndex, size_t lastIndex, bool copyInReverse) noexcept;
    /**
     *
     * @param container Pointer to valid DynamicContainer
     * @return Copy of container, is invalid if allocation fails.
     */
    extern BL_DynamicContainer  bl_container_dynamic_copy(const BL_DynamicContainer* container) noexcept;
    /**
     * @brief Reverses internal array.
     * @param container Pointer to valid DynamicContainer
     */
    extern void                 bl_container_dynamic_reverse(BL_DynamicContainer* container) noexcept;
    /**
     * @param container Pointer to valid DynamicContainer
     * @return true if container is empty, else false
     */
    extern bool                 bl_container_dynamic_is_empty(const BL_DynamicContainer* container) noexcept;
    /**
     *
     * @param container Pointer to valid DynamicContainer
     * @return Amount of elements in container.
     */
    extern size_t               bl_container_dynamic_size(const BL_DynamicContainer* container) noexcept;
    /**
     *
     * @param container Pointer to valid Container
     * @param reference Pointer to element in container
     * @return Index of element in container.
     */
    extern size_t               bl_container_dynamic_index_from_reference(const BL_DynamicContainer* container, const void* reference) noexcept;
    /**
     * @brief This function checks if the DynamicContainer must be resized. Resizes if necessary.
     * @param container Pointer to valid DynamicContainer
     * @return ContainerAllocFailure if alloc failed.
     */
    extern BL_ContainerError    bl_container_dynamic_size_check_add(BL_DynamicContainer* container) noexcept;
    /**
     * @brief This function checks if DynamicContainers used indexes are less then a fourth of available indexes.
     * If this is true, then the container is shrunk.
     * @param container Pointer to valid DynamicContainer
     * @return ContainerAllocFailure if alloc of downsized container was unsuccessful.
     */
    extern BL_ContainerError    bl_container_dynamic_size_check_remove(BL_DynamicContainer* container) noexcept;
    /**
     * @brief Ensures that at least amountOfIndexesToReserve are available to append to before resizeing is needed.
     * @param container Pointer to valid DynamicContainer
     * @param amountOfIndexesToReserve Amount of unused indexes to ensure exists
     * @return ContainerAllocFailure if unable to reallocate
     */
    extern BL_ContainerError    bl_container_dynamic_reserve(BL_DynamicContainer* container, size_t amountOfIndexesToReserve) noexcept;
    /**
     * @brief Sets amount of elements used to 0 and does not resize to 0 elements.
     * @param container Pointer to valid DynamicContainer
     */
    extern void                 bl_container_dynamic_clear(BL_DynamicContainer* container) noexcept;
    /**
     * @brief Removes the last element from the DynamicContainer.
     * @param container Pointer to valid DynamicContainer
     * @return ContainerInvalidIndex if there was no element to pop.
     */
    extern BL_ContainerError    bl_container_dynamic_pop(BL_DynamicContainer* container) noexcept;
    /**
     * @brief Removes elements from DynamicContainer given a range.
     * @param container Pointer to valid DynamicContainer
     * @param index First index in removal range (inclusive)
     * @param lastIndex Last index in removal range (inclusive)
     * @return ContainerInvalidIndex if index is larger than lastIndex or if the indexes are invalid
     */
    extern BL_ContainerError    bl_container_dynamic_remove(BL_DynamicContainer* container, size_t index, size_t lastIndex) noexcept;
    /**
     * @brief Insert elements into container.
     * @param container Pointer to valid DynamicContainer
     * @param index Index to start inserting from
     * @param amountOfElements Amount of elements to insert
     * @param sizeOfElement Size of individual element
     * @param elements List of elements to insert
     * @return ContainerInvalidIndex if index was larger than the size of the array.
     * @return ContainerInvalidSize if the sizeOfElement was larger than the size of a single element in the array.
     * @return ContainerAllocFailure if the array cannot grow for the new elements.
     */
    extern BL_ContainerError    bl_container_dynamic_insert(BL_DynamicContainer* container, size_t index, size_t amountOfElements, size_t sizeOfElement, const void* elements) noexcept;
    /**
     * @brief Insert elements in containerToInsert into container.
     * @param container Pointer to valid DynamicContainer
     * @param index Index to start inserting from
     * @param containerToInsert Valid pointer to a Container containing elements wished to insert
     * @return ContainerInvalidIndex if index was larger than the size of the array.
     * @return ContainerInvalidSize if the size of a single element in the container wished to insert were larger than the size of a single element in the array.
     * @return ContainerAllocFailure if the array cannot grow for the new elements.
     */
    extern BL_ContainerError    bl_container_dynamic_insert_container(BL_DynamicContainer* container, size_t index, const BL_Container* containerToInsert) noexcept;
    /**
     * @brief Appends element to the end of the DynamicContainer.
     * @param container Pointer to valid DynamicContainer
     * @param sizeOfElement Size of element
     * @param element Element to insert
     * @return ContainerInvalidSize if the sizeOfElement was larger than the size of a single element in the array.
     * @return ContainerAllocFailure if the array cannot grow for the new element.
     */
    extern BL_ContainerError    bl_container_dynamic_append(BL_DynamicContainer* container, size_t sizeOfElement, const void* element) noexcept;
    /**
     * @brief Creates a DynamicContainer on the stack. Use isValidObject to check validity.
     * @param initialSize Initial size of internal array
     * @param elementSize Size of the largest element to be stored in array
     */
    extern BL_DynamicContainer  bl_container_dynamic_create_stack(size_t initialSize, size_t elementSize);
    /**
     * @brief Creates a DynamicContainer on the heap.
     * @param initialSize Initial size of internal array
     * @param elementSize Size of the largest element to be stored in array
     * @return NULL if allocation failed.
     */
    extern BL_DynamicContainer* bl_container_dynamic_create_heap(size_t initialSize, size_t elementSize);
    /**
     * @brief Runs destructor in each element in container and then destroys container.
     * @param container Pointer to valid DynamicContainer
     * @param elementDestructor Valid function pointer of type void(*)(void*)
     */
    extern void                 bl_container_dynamic_destroy_with_elements(BL_DynamicContainer* container, void(elementDestructor)(void* element));
    /**
     * @brief Returns pointer to first index in container. Index is NULL if container is empty.
     * @param container Pointer to valid DynamicContainer
     * @return Pointer to first index in container.
     */
    extern void*                bl_container_dynamic_front(const BL_DynamicContainer* container) noexcept;
    /**
     * @brief Gets the next element in container.
     * @param container Pointer to valid Container
     * @param element Pointer to element in container
     * @return Next element in container, NULL if no such exists
     */
    extern void*                bl_container_dynamic_next(const BL_DynamicContainer* container, const void* element) noexcept;
    /**
     * @brief Gets the next element in reversed order in container.
     * @param container Pointer to valid Container
     * @param element Pointer to element in container
     * @return Next element in reverse order in container, NULL if no such exists
     */
    extern void*                bl_container_dynamic_prev(const BL_DynamicContainer* container, const void* element) noexcept;
    /**
     * @brief Returns pointer to last index in container. Index is NULL if container is empty.
     * @param container Pointer to valid DynamicContainer
     * @return Pointer to last index in container.
     */
    extern void*                bl_container_dynamic_back(const BL_DynamicContainer* container) noexcept;
    /**
     * @brief Returns pointer to the first invalid index in the container.
     * @param container Pointer to valid DynamicContainer
     * @return Pointer to the first invalid index in container.
     */
    extern void*                bl_container_dynamic_end(const BL_DynamicContainer* container) noexcept;
    /**
     * @brief Will convert a container to a dynamic one. This will not invalidate the previous container however the old Container shall not be used.
     * @param container Any Container object
     * @return DynamicContainer of Container
     */
    extern BL_DynamicContainer  bl_dynamic_container_cast_container(BL_Container container) noexcept;
    /**
     * @param container Valid Container
     * @return Internal Container
     */
    extern BL_Container         bl_container_cast_dynamic_container(BL_DynamicContainer container) noexcept;
    /**
     *
     * @param container Pointer to valid DynamicContainer
     * @return Pointer to internal Container.
     */
    extern BL_Container*        bl_container_ptr_cast_dynamic_container(BL_DynamicContainer* container) noexcept;
    /**
     *
     * @param container Pointer to valid DynamicContainer
     * @return Pointer to internal Container
     */
    extern const BL_Container*  bl_container_const_ptr_cast_dynamic_container(const BL_DynamicContainer* container) noexcept;
    /**
     *
     * @param container Pointer to DynamicContainer or NULL
     * @return true if container is valid, else false
     */
    extern bool                 bl_container_dynamic_is_valid(const BL_DynamicContainer* container) noexcept;
    /**
     *
     * @param obj Object to destroy
     */
    extern void                 bl_container_dynamic_destroy(void* obj) noexcept;

#ifdef __cplusplus
    }
};
#else
#undef noexcept
#endif

#endif // DYNAMICCONTAINER_H
