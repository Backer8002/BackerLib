#ifndef UNORDEREDCONTAINER_H
#define UNORDEREDCONTAINER_H

#include "BL_Container.h"
#include "TypesMain.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#else
#define noexcept
#endif

    #define BL_UNORDERED_CONTAINER_PAGE_SIZE_IN_AMOUNT 256

    typedef struct BL_UnorderedContainer {
        BL_DataTypeFlags header;
        uint32_t byteSizeOfElement;
        size_t amountOfElements;
        void** pages;
        uint64_t*    bitset;
        size_t       amountOfPages;
    } BL_UnorderedContainer;

    /**
     * @brief Puts element in the next available slot. Resizes if needed.
     * @param container Pointer to valid UnorderedContainer
     * @param sizeOfElement Size of element to put in
     * @param element Element to put in
     * @return NULL if allocation failed or if sizeOfElement was too large
     */
    extern void* bl_unordered_container_put(BL_UnorderedContainer* container, size_t sizeOfElement, const void* element) noexcept;
    /**
     * @brief Overwrites and sets element at index. Index may not be larger than the internal container.
     * @param container Pointer to valid UnorderedContainer
     * @param index Index to replace or set at
     * @param sizeOfElement Size of element
     * @param element Element to set
     * @return ContainerInvalidSize if sizeOfElement was larger than the largestElement in the array. Only applicable if elementsArePointers is not set.
     * @return ContainerInvalidIndex if index was out of bounds.
     */
    extern BL_ContainerError              bl_unordered_container_set(BL_UnorderedContainer* container, size_t index, size_t sizeOfElement, const void* element) noexcept;
    /**
     * @brief Like bl_unordered_container_set this sets an element at an index. Though this only sets the element if it is not yet set.
     * Index may not be larger than the internal container.
     * @param container Pointer to valid UnorderedContainer
     * @param index Index to set at
     * @param sizeOfElement Size of element
     * @param element Element to set
     * @return ContainerOPUnsuccessful if element was already set.
     * @return ContainerInvalidSize if sizeOfElement was larger than the largestElement in the array. Only applicable if elementsArePointers is not set.
     * @return ContainerInvalidIndex if index was out of bounds.
     */
    extern BL_ContainerError              bl_unordered_container_set_try(BL_UnorderedContainer* container, size_t index, size_t sizeOfElement, const void* element) noexcept;
    /**
     * @brief Gets element in UnorderedContainer.
     * @param container Pointer to valid UnorderedContainer
     * @param index Index to get from
     * @return .element = NULL if element was not valid. Check .resultCode.
     * @return ResultCode is ContainerInvalidIndex if index was out of bounds in the container.
     * @return ResultCode is ContainerOPUnsuccessful if index was not occupied.
     */
    extern void*                          bl_unordered_container_get(const BL_UnorderedContainer* container, size_t index) noexcept;
    /**
     * @brief Removes element at index.
     * @param container Pointer to valid UnorderedContainer
     * @param index Index to remove at
     * @param destructor Optional function to be run on element. Pass NULL if no function should run
     * @return ContainerInvalidIndex if index was out of bounds of the container.
     * @return ContainerOPUnsuccessful if index was not occupied.
     */
    extern BL_ContainerError              bl_unordered_container_remove(BL_UnorderedContainer* container, size_t index, void (*destructor)(void* element)) noexcept;
    /**
     * @brief Creates a UnorderedContainer on the stack. Use isValidObject to check validity.
     * @param initialSize Initial size of internal array
     * @param sizeOfElement Size of the largest element to be stored in array
     */
    extern BL_UnorderedContainer          bl_unordered_container_create_stack(size_t initialSize, size_t sizeOfElement) noexcept;
    /**
     * @brief Creates a UnorderedContainer on the heap.
     * @param initialSize Initial size of internal array
     * @param sizeOfElement Size of the largest element to be stored in array
     * @return NULL if allocation failed.
     */
    extern BL_UnorderedContainer*         bl_unordered_container_create_heap(size_t initialSize, size_t sizeOfElement) noexcept;
    /**
     * @brief Gets the first valid index in an UnorderedContainer.
     * @param container Pointer to valid UnorderedContainer
     * @return Pointer to first valid index, else NULL.
     */
    extern void*                          bl_unordered_container_front(const BL_UnorderedContainer* container) noexcept;
    /**
     * @brief Gets the next valid index after element. Returns NULL if no such index exists.
     * @param container Pointer to valid UnorderedContainer
     * @param element Element in container
     * @return Next valid index, else NULL.
     */
    extern void*                          bl_unordered_container_next(const BL_UnorderedContainer* container, const void* element) noexcept;
    /**
     * @brief Gets the next valid index in reverse order after element. Returns NULL if no such index exists.
     * @param container Pointer to valid UnorderedContainer
     * @param element Element in container
     * @return Next valid index in reverse order, else NULL
     */
    extern void*                          bl_unordered_container_prev(const BL_UnorderedContainer* container, const void* element) noexcept;
    /**
     * @brief Gets the last valid index in container.
     * @param container Pointer to valid UnorderedContainer
     * @return Last valid index in container, else NULL.
     */
    extern void*                          bl_unordered_container_back(const BL_UnorderedContainer* container) noexcept;
    /**
     * @brief Gets the index after unorderedContainerBack. Will always be invalid.
     * @param container Pointer to valid UnorderedContainer
     * @return NULL if no valid index exists.
     */
    extern void*                          bl_unordered_container_end(const BL_UnorderedContainer* container) noexcept;
    /**
     *
     * @param container Pointer to UnorderedContainer or NULL
     * @return true if UnorderedContainer is valid, else NULL.
     */
    extern bool                           bl_unordered_container_is_valid(const BL_UnorderedContainer* container) noexcept;
    /**
     *
     * @param container Pointer to valid UnorderedContainer
     * @return Amount of elements in container.
     */
    extern size_t                         bl_unordered_container_size(const BL_UnorderedContainer* container) noexcept;
    /**
     * @brief Destroys an UnorderedContainer if applicable. Does nothing if not an UnorderedContainer qualified.
     * @param container Pointer to UnorderedContainer
     */
    extern void                           bl_unordered_container_destroy(void* container) noexcept;
    /**
     * @param container Pointer to valid UnorderedContainer
     * @param element Pointer
     * @return Index of containing element in container. Invalid index if element not in container.
     */
    extern size_t bl_unordered_container_index_from_ref(const BL_UnorderedContainer* container, const void* element) noexcept;
    /**
     * @brief Destroys an UnorderedContainer and runs destructor on every valid element.
     * @param container Pointer to valid UnorderedContainer
     * @param destructor Pointer to valid function matching argument type
     */
    extern void                           bl_unordered_container_destroy_with_elements(BL_UnorderedContainer* container, void (*destructor)(void* element)) noexcept;


#ifdef __cplusplus
}
#else
#undef noexcept
#endif
#endif // UNORDEREDCONTAINER_H
