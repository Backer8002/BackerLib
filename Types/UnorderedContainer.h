#ifndef UNORDEREDCONTAINER_H
#define UNORDEREDCONTAINER_H

#include "TypesMain.h"
#include <stddef.h>

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#endif
    typedef struct UnorderedContainerPutResult {
        ContainerError resultCode;
        size_t         locationOfElement;
    } UnorderedContainerPutResult;
    typedef struct UnorderedContainerGetResult {
        ContainerError resultCode;
        void*          element;
    } UnorderedContainerGetResult;
    /**
     * @brief Puts element in the next available slot. Resizes if needed.
     * @param container Pointer to valid UnorderedContainer
     * @param sizeOfElement Size of element to put in
     * @param element Element to put in
     * @return resultCode is always returned. Only if resultCode is ContainerOPSuccessful is locationOfElement valid.
     * @return If resultCode is ContainerInvalidSize then size of object was invalid. This is only possible if elementsArePointers is not set.
     * @return ContainerAllocFailure if resizing was invalid. No breaking changes have been made.
     */
    extern UnorderedContainerPutResult unorderedContainerPut(UnorderedContainer* container, size_t sizeOfElement, const void* element);
    /**
     * @brief Overwrites and sets element at index. Index may not be larger than the internal container.
     * @param container Pointer to valid UnorderedContainer
     * @param index Index to replace or set at
     * @param sizeOfElement Size of element
     * @param element Element to set
     * @return ContainerInvalidSize if sizeOfElement was larger than the largestElement in the array. Only applicable if elementsArePointers is not set.
     * @return ContainerInvalidIndex if index was out of bounds.
     */
    extern ContainerError              unorderedContainerSet(UnorderedContainer* container, size_t index, size_t sizeOfElement, const void* element);
    /**
     * @brief Like unorderedContainerSet this sets an element at an index. Though this only sets the element if it is not yet set.
     * Index may not be larger than the internal container.
     * @param container Pointer to valid UnorderedContainer
     * @param index Index to set at
     * @param sizeOfElement Size of element
     * @param element Element to set
     * @return ContainerOPUnsuccessful if element was already set.
     * @return ContainerInvalidSize if sizeOfElement was larger than the largestElement in the array. Only applicable if elementsArePointers is not set.
     * @return ContainerInvalidIndex if index was out of bounds.
     */
    extern ContainerError              unorderedContainerSetTry(UnorderedContainer* container, size_t index, size_t sizeOfElement, const void* element);
    /**
     * @brief Gets element in UnorderedContainer.
     * @param container Pointer to valid UnorderedContainer
     * @param index Index to get from
     * @return .element = NULL if element was not valid. Check .resultCode.
     * @return ResultCode is ContainerInvalidIndex if index was out of bounds in the container.
     * @return ResultCode is ContainerOPUnsuccessful if index was not occupied.
     */
    extern UnorderedContainerGetResult unorderedContainerGet(const UnorderedContainer* container, size_t index);
    /**
     * @brief Removes element at index.
     * @param container Pointer to valid UnorderedContainer
     * @param index Index to remove at
     * @param destructor Optional function to be run on element. Pass NULL if no function should run
     * @return ContainerInvalidIndex if index was out of bounds of the container.
     * @return ContainerOPUnsuccessful if index was not occupied.
     */
    extern ContainerError              unorderedContainerRemove(UnorderedContainer* container, size_t index, void (*destructor)(void* element));
    /**
     * @brief Creates a UnorderedContainer on the stack. Use isValidObject to check validity.
     * @param initialSize Initial size of internal array
     * @param sizeOfElement Size of the largest element to be stored in array
     * @param elementsArePointers Is the array to store the pointers to elements instead of copying them
     */
    extern UnorderedContainer          unorderedContainerCreateStack(size_t initialSize, size_t sizeOfElement, bool elementsArePointers);
    /**
     * @brief Creates a UnorderedContainer on the heap.
     * @param initialSize Initial size of internal array
     * @param sizeOfElement Size of the largest element to be stored in array
     * @param elementsArePointers Is the array to store the pointers to elements instead of copying them
     * @return NULL if allocation failed.
     */
    extern UnorderedContainer*         unorderedContainerCreateHeap(size_t initialSize, size_t sizeOfElement, bool elementsArePointers);

    /**
     * @brief Gets the first valid index in an UnorderedContainer.
     * @param container Pointer to valid UnorderedContainer
     * @return Pointer to first valid index, else NULL.
     */
    extern void* unorderedContainerFront(const UnorderedContainer* container);
    /**
     * @brief Gets the next valid index after element. Returns NULL if no such index exists.
     * @param container Pointer to valid UnorderedContainer
     * @param element Element in container
     * @return Next valid index, else NULL.
     */
    extern void* unorderedContainerNext(const UnorderedContainer* container, const void* element);
    /**
     * @brief Gets the next valid index in reverse order after element. Returns NULL if no such index exists.
     * @param container Pointer to valid UnorderedContainer
     * @param element Element in container
     * @return Next valid index in reverse order, else NULL
     */
    extern void* unorderedContainerPrev(const UnorderedContainer* container, const void* element);
    /**
     * @brief Gets the last valid index in container.
     * @param container Pointer to valid UnorderedContainer
     * @return Last valid index in container, else NULL.
     */
    extern void* unorderedContainerBack(const UnorderedContainer* container);
    /**
     * @brief Gets the index after unorderedContainerBack. Will always be invalid.
     * @param container Pointer to valid UnorderedContainer
     * @return NULL if no valid index exists.
     */
    extern void* unorderedContainerEnd(const UnorderedContainer* container);
    /**
     * @brief Destroys an UnorderedContainer if applicable. Does nothing if not an UnorderedContainer qualified.
     * @param container Pointer to UnorderedContainer
     */
    extern void                        unorderedContainerDestroy(void* container);
    /**
     * @brief Destroys an UnorderedContainer and runs destructor on every valid element.
     * @param container Pointer to valid UnorderedContainer
     * @param destructor Pointer to valid function matching argument type
     */
    extern void                        unorderedContainerDestroyWithElements(UnorderedContainer* container, void (*destructor)(void* element));


#ifdef __cplusplus
    }
};
#endif
#endif // UNORDEREDCONTAINER_H
