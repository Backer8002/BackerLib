#ifndef CONTAINER_H
#define CONTAINER_H

#include "TypesMain.h"
#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#else
#define noexcept
#endif

        /**
         * @brief Sets index in container to element. If element is smaller than a single element in the array junk values may exist after size of element.
         * If Container is a dynamic container then access range is only within current use range
         * @param container Pointer to valid container object
         * @param index index in container to set
         * @param elementSize size of element to set
         * @param element element used to set
         * @return ContainerOPSuccessful if valid operation
         * @return ContainerInvalidIndex if index is out of bounds
         * @return ContainerInvalidSize if element was larger than a single element in array
         */
        extern ContainerError containerSet(Container* container,size_t index,size_t elementSize, const void* restrict element) noexcept;
        /**
         * @brief Gets an element in the array.
         * @param container Pointer to valid container object
         * @param index index in container to access
         * @return NULL if index was out of range.
         * @return Pointer to object in array. If elementsArePointers were specified this instead returns the pointer to object within the array.
         */
        extern void* containerGet(const Container* container,size_t index) noexcept;
        /**
         * @brief Makes a copy of subarray specefied by first and last index. Use isValidObject to check validity of return object.
         * @param container Pointer to valid Container
         * @param firstIndex Lowest index to copy (inclusive)
         * @param lastIndex Highest index to copy (inclusive)
         * @param copyInReverse Should the elements be copied in reverse order
         * @return Container containing copy of subarray.
         */
        extern Container containerGetSubArray(const Container* container, size_t firstIndex, size_t lastIndex,bool copyInReverse) noexcept;
        /**
         * @brief Wrapper of containerGetSubArray(container,0,size,false)
         * @param container Pointer to valid Container
         * @return Invalid container if allocation failed.
         */
        extern Container containerCopy(const Container* container) noexcept;
        /**
         * @brief Reverses the order of elemenets in container inplace.
         * @param container Pointer to valid container
         */
        extern void containerReverse(Container* container) noexcept;
        /**
         * @brief Creates and returns a container allocated on the stack.
         * @param size Amount of elements to allocate for
         * @param elementSize Size of single element
         * @param elementsArePointers Is the array to store the pointers to elements instead of copying them
         * @return Stack allocated Container. Use isValidObject to determine validity.
         */
        extern Container containerCreateStack(size_t size, size_t elementSize, bool elementsArePointers) noexcept;
        /**
         * @brief Creates and returns a container allocated on the heap.
         * @param size Amount of elements to allocate for
         * @param elementSize Size of single element
         * @param elementsArePointers Is the array to store the pointers to elements instead of copying them
         * @return Heap allocated Container
         * @return NULL if alloc failed
         */
        extern Container* containerCreateHeap(size_t size, size_t elementSize, bool elementsArePointers) noexcept;
        /**
         * @brief Checks if container is empty.
         * @param container Pointer to valid Container
         * @return true if container does not contain any elements, else false.
         */
        extern bool containerIsEmpty(const Container* container) noexcept;
        /**
         * @param container Pointer to valid Container
         * @return Size of current container.
         */
        extern size_t containerSize(const Container* container) noexcept;
        /**
         * @brief Returns a reference index in a container
         * @param container Pointer to valid container
         * @param reference Pointer to reference within container
         * @return reference's index in container
         * @note It is undefined behavoir to use a reference to outside the container.
         * @note Reference may point to any part of member object.
         */
        extern size_t containerIndexFromReference(const Container* container, const void* reference) noexcept;
        /**
         * @brief Returns pointer to first index in container. Index is NULL if container is empty.
         * @param container Pointer to valid Container
         * @return Pointer to first index in container.
         */
        extern void* containerFront(const Container* container) noexcept;
        /**
         * @brief Gets the next element in container.
         * @param container Pointer to valid Container
         * @param element Pointer to element in container
         * @return Next element in container, NULL if no such exists
         */
        extern void* containerNext(const Container* container, const void* element) noexcept;
        /**
         * @brief Gets the next element in reversed order in container.
         * @param container Pointer to valid Container
         * @param element Pointer to element in container
         * @return Next element in reverse order in container, NULL if no such exists
         */
        extern void* containerPrev(const Container* container, const void* element) noexcept;
        /**
         * @brief Returns pointer to last index in container. Index is NULL if container is empty
         * @param container Pointer to valid Container
         * @return Pointer to last index in container.
         */
        extern void* containerBack(const Container* container) noexcept;
        /**
         * @brief Returns pointer to the first invalid index in the container.
         * @param container Pointer to valid Container
         * @return Pointer to the first invalid index in container.
         */
        extern void* containerEnd(const Container* container) noexcept;
        /**
         * @brief Destroys and frees object if applicable. Sets object state to invalid. Will not free container if amountOfIndexes is set to 0.
         * @param container Container to destroy
         */
        extern void containerDestroy(void* container) noexcept;

#ifdef __cplusplus
    }
};
#endif

#endif //CONTAINER_H
