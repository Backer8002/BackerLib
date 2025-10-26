#ifndef DYNAMICCONTAINER_H
#define DYNAMICCONTAINER_H

#include "TypesMain.h"
#include "Container.h"
#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#endif


        /**
         * @brief This function checks if the DynamicContainer must be resized. Resizes if necessary.
         * @param container Pointer to valid DynamicContainer
         * @return ContainerAllocFailure if alloc failed.
         */
        extern  ContainerError containerDynamicSizeCheckAdd(DynamicContainer* container);
        /**
         * @brief This function checks if DynamicContainers used indexes are less then a fourth of available indexes.
         * If this is true, then the container is shrunk.
         * @param container Pointer to valid DynamicContainer
         * @return ContainerAllocFailure if alloc of downsized container was unsuccessful.
         */
        extern  ContainerError containerDynamicSizeCheckRemove(DynamicContainer* container);
        /**
         * @brief Ensures that at least amountOfIndexesToReserve are available to append to before resizeing is needed.
         * @param container Pointer to valid DynamicContainer
         * @param amountOfIndexesToReserve Amount of unused indexes to ensure exists
         * @return ContainerAllocFailure if unable to reallocate
         */
        extern ContainerError containerDynamicReserve(DynamicContainer* container, size_t amountOfIndexesToReserve);
        /**
         * @brief Sets amount of elements used to 0 and resizes to 0 elements.
         * @param container Pointer to valid DynamicContainer
         */
        extern  void             containerDynamicClear(DynamicContainer* container);
        /**
         * @brief Removes the last element from the DynamicContainer.
         * @param container Pointer to valid DynamicContainer
         * @return ContainerInvalidIndex if there was no element to pop.
         */
        extern  ContainerError containerDynamicPop(DynamicContainer* container);
        /**
         * @brief Removes elements from DynamicContainer given a range.
         * @param container Pointer to valid DynamicContainer
         * @param index First index in removal range (inclusive)
         * @param lastIndex Last index in removal range (inclusive)
         * @return ContainerInvalidIndex if index is larger than lastIndex or if the indexes are invalid
         */
        extern  ContainerError containerDynamicRemove(DynamicContainer* container, size_t index, size_t lastIndex);
        /**
         * @brief Insert elements into container.
         * @param container Pointer to valid DynamicContainer
         * @param index Index to start inserting from
         * @param amountOfElements Amount of elements to insert
         * @param sizeOfElement Size of individual element
         * @param elements List of elements to insert. This should be a list of pointers if ElementsArePointers
         * @return ContainerInvalidIndex if index was larger than the size of the array.
         * @return ContainerInvalidSize if the sizeOfElement was larger than the size of a single element in the array.
         * @return ContainerAllocFailure if the array cannot grow for the new elements.
         */
        extern  ContainerError containerDynamicInsert(DynamicContainer* container, size_t index, size_t amountOfElements, size_t sizeOfElement,const void* elements);
        /**
         * @brief Insert elements in containerToInsert into container.
         * @param container Pointer to valid DynamicContainer
         * @param index Index to start inserting from
         * @param containerToInsert Valid pointer to a Container containing elements wished to insert
         * @return ContainerInvalidIndex if index was larger than the size of the array.
         * @return ContainerInvalidSize if the size of a single element in the container wished to insert were larger than the size of a single element in the array.
         * @return ContainerAllocFailure if the array cannot grow for the new elements.
         */
        extern  ContainerError containerDynamicInsertContainer(DynamicContainer* container, size_t index,const Container* containerToInsert);
        /**
         * @brief Appends element to the end of the DynamicContainer.
         * @param container Pointer to valid DynamicContainer
         * @param sizeOfElement Size of element
         * @param element Element to insert
         * @return ContainerInvalidSize if the sizeOfElement was larger than the size of a single element in the array.
         * @return ContainerAllocFailure if the array cannot grow for the new element.
         */
        extern  ContainerError containerDynamicAppend(DynamicContainer* container, size_t sizeOfElement, const void* element);
        /**
         * @brief Will convert a container to a dynamic one. This will not invalidate the previous container however the old Container shall not be used.
         * @param container Any Container object
         * @return DynamicContainer of Container
         */
        static inline DynamicContainer containerConvertToDynamicStack(Container container) {
            DynamicContainer containerToReturn = {.container = container, .containerMaxSize = container.amountOfIndexes};
            return containerToReturn;
        }
        /**
         * @brief Creates a DynamicContainer on the stack. Use isValidObject to check validity.
         * @param initialSize Initial size of internal array
         * @param elementSize Size of the largest element to be stored in array
         * @param elementsArePointers Is the array to store the pointers to elements instead of copying them
         *
         */
        extern DynamicContainer        containerDynamicCreateStack(size_t initialSize, size_t elementSize, bool elementsArePointers);
        /**
         * @brief Creates a DynamicContainer on the heap.
         * @param initialSize Initial size of internal array
         * @param elementSize Size of the largest element to be stored in array
         * @param elementsArePointers Is the array to store the pointers to elements instead of copying them
         * @return NULL if allocation failed.
         */
        extern  DynamicContainer*       containerDynamicCreateHeap(size_t initialSize, size_t elementSize, bool elementsArePointers);
        /**
         * @brief Runs destructor in each element in container and then destroys container.
         * @param container Pointer to valid DynamicContainer
         * @param elementDestructor Valid function pointer of type void(*)(void*)
         */
        extern  void             containerDynamicDestroyWithElements(DynamicContainer* container, void(elementDestructor)(void* element));
        /**
         * @brief Returns pointer to first index in container. Index is NULL if container is empty.
         * @param container Pointer to valid DynamicContainer
         * @return Pointer to first index in container.
         */
        static inline void* containerDynamicFront(const DynamicContainer* container) {
            return containerFront((Container*)container);
        }
        /**
         * @brief Gets the next element in container.
         * @param container Pointer to valid Container
         * @param element Pointer to element in container
         * @return Next element in container, NULL if no such exists
         */
        static void* containerDynamicNext(const DynamicContainer* container, const void* element) {
            return containerNext((Container*)container,element);
        }
        /**
         * @brief Gets the next element in reversed order in container.
         * @param container Pointer to valid Container
         * @param element Pointer to element in container
         * @return Next element in reverse order in container, NULL if no such exists
         */
        static void* containerDynamicPrev(const DynamicContainer* container, const void* element) {
            return containerPrev((Container*)container,element);
        }

        /**
         * @brief Returns pointer to last index in container. Index is NULL if container is empty.
         * @param container Pointer to valid DynamicContainer
         * @return Pointer to last index in container.
         */
        static inline void* containerDynamicBack(const DynamicContainer* container) {
            return containerBack((Container*)container);
        }
        /**
         * @brief Returns pointer to the first invalid index in the container.
         * @param container Pointer to valid DynamicContainer
         * @return Pointer to the first invalid index in container.
         */
        static inline void* containerDynamicEnd(const DynamicContainer* container) {
            return containerEnd((Container*)container);
        }

#ifdef __cplusplus
    }
};
#endif

#endif //DYNAMICCONTAINER_H
