#ifndef CONTAINER_H
#define CONTAINER_H

#include "TypesMain.h"
#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
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
        extern ContainerError containerSet(Container* container,size_t index,size_t elementSize, const void* restrict element);
        /**
         * @brief Gets an element in the array.
         * @param container Pointer to valid container object
         * @param index index in container to access
         * @return NULL if index was out of range.
         * @return Pointer to object in array. If elementsArePointers were specified this instead returns the pointer to object within the array.
         */
        extern void* containerGet(const Container* container,size_t index);
        /**
         * @brief Makes a copy of subarray specefied by first and last index. Use isValidObject to check validity of return object.
         * @param container Pointer to valid Container
         * @param firstIndex Lowest index to copy (inclusive)
         * @param lastIndex Highest index to copy (inclusive)
         * @param copyInReverse Should the elements be copied in reverse order
         * @return Container containing copy of subarray.
         */
        extern Container containerGetSubArray(const Container* container, size_t firstIndex, size_t lastIndex,bool copyInReverse);
        /**
         * @brief Reverses the order of elemenets in container inplace.
         * @param container Pointer to valid container
         */
        extern void containerReverse(Container* container);
        /**
         * @brief Creates and returns a container allocated on the stack.
         * @param size Amount of elements to allocate for
         * @param elementSize Size of single element
         * @param elementsArePointers Is the array to store the pointers to elements instead of copying them
         * @return Stack allocated Container. Use isValidObject to determine validity.
         */
        extern Container containerCreateStack(size_t size, size_t elementSize, bool elementsArePointers);
        /**
         * @brief Creates and returns a container allocated on the heap.
         * @param size Amount of elements to allocate for
         * @param elementSize Size of single element
         * @param elementsArePointers Is the array to store the pointers to elements instead of copying them
         * @return Heap allocated Container
         * @return NULL if alloc failed
         */
        extern Container* containerCreateHeap(size_t size, size_t elementSize, bool elementsArePointers);
        /**
         * @brief Checks if container is empty.
         * @param container Pointer to valid Container
         * @return true if container does not contain any elements, else false.
         */
        static inline bool containerIsEmpty(const Container* container) {
            return container->amountOfIndexes == 0;
        }
        /**
         * @param container Pointer to valid Container
         * @return Size of current container.
         */
        static inline size_t containerSize(const Container* container) {
            return container->amountOfIndexes;
        }

        /**
         * @brief Returns a reference index in a container
         * @param container Pointer to valid container
         * @param reference Pointer to reference within container
         * @return reference's index in container
         * @note It is undefined behavoir to use a reference to outside the container.
         * @note Reference may point to any part of member object.
         */
        static inline size_t containerIndexFromReference(const Container* container, const void* const reference) {
            return ((uintptr_t)reference - (uintptr_t)container->array)/container->byteSizeOfSingleElement;
        }

        /**
         * @brief Destroys and frees object if applicable. Sets object state to invalid. Will not free container if amountOfIndexes is set to 0.
         * @param container Container to destroy
         */
        extern void containerDestroy(void* container);

#ifdef __cplusplus
    }
};
#endif

#endif //CONTAINER_H
