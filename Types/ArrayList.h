#ifndef ArrayList_h
#define ArrayList_h

#include "Container.h"
#include "DynamicContainer.h"
#include "TypesMain.h"
#include "../Concurrency/ConcurrencyDefines.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>



#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#endif
    /**
     * @brief A threadsafe implementation of DynamicContainer. Can work as a dynamic container, but will not lock the mutex then.
     */
    typedef union ArrayList {
        struct {
            DynamicContainer dynamicContainer;
            Mutex            mutex;
        };
        Container     container;
        DataTypeFlags header;
    } ArrayList;

    /**
     * @brief Sets amount of elements used to 0 and resizes to 0 elements.
     * @param arrayList Pointer to valid ArrayList
     * @return false if failed to lock mutex.
     */
    static inline bool arrayListClear(ArrayList* arrayList) {
        if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
            return false;
        containerDynamicClear((DynamicContainer*) arrayList);
        mutexUnlock(&arrayList->mutex);
        return true;
    }
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
    static ContainerError arrayListGet(ArrayList* arrayList, size_t index, size_t sizeOfElement, void* restrict element) {
        if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
            return ContainerOPUnsuccessful;
        void* elementToGet = containerGet((Container*) arrayList, index);
        if (!elementToGet) {
            mutexUnlock(&arrayList->mutex);
            return ContainerInvalidIndex;
        }
        if (arrayList->container.byteSizeOfSingleElement < sizeOfElement) {
            mutexUnlock(&arrayList->mutex);
            return ContainerInvalidSize;
        }
        memcpy(element, elementToGet, sizeOfElement);
        mutexUnlock(&arrayList->mutex);
        return ContainerOPSuccessful;
    }
    /**
     * @brief Removes the last element from the ArrayList.
     * @param arrayList Pointer to valid ArrayList
     * @return ContainerInvalidIndex if there was no element to pop.
     * @return ContainerOPUnsuccessful if mutex could not be locked.
     */
    static inline ContainerError arrayListPop(ArrayList* arrayList) {
        if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
            return ContainerOPUnsuccessful;
        ContainerError result = containerDynamicPop((DynamicContainer*) arrayList);
        mutexUnlock(&arrayList->mutex);
        return result;
    }
    /**
     * @brief Removes elements from ArrayList given a range.
     * @param arrayList Pointer to valid ArrayList
     * @param index First index in removal range (inclusive)
     * @param lastIndex Last index in removal range (inclusive)
     * @return ContainerInvalidIndex if index is larger than lastIndex or if the indexes are invalid
     * @return ContainerOPUnsuccessful if mutex could not be locked.
     */
    static inline ContainerError arrayListRemove(ArrayList* arrayList, size_t index, size_t lastIndex) {
        if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
            return ContainerOPUnsuccessful;
        ContainerError result = containerDynamicRemove((DynamicContainer*) arrayList, index, lastIndex);
        mutexUnlock(&arrayList->mutex);
        return result;
    }
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
    static inline ContainerError arrayListInsert(ArrayList* arrayList, size_t index, size_t amountOfElements, size_t sizeOfElement, const void* elements) {
        if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
            return ContainerOPUnsuccessful;
        ContainerError result = containerDynamicInsert((DynamicContainer*) arrayList, index, amountOfElements, sizeOfElement, elements);
        mutexUnlock(&arrayList->mutex);
        return result;
    }
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
    static inline ContainerError arrayListSet(ArrayList* arrayList, size_t index, size_t elementSize, const void* element) {
        if (mutexLock(&arrayList->mutex) == ConcurrencyFailure)
            return ContainerOPUnsuccessful;
        ContainerError result = containerSet((Container*) arrayList, index, elementSize, element);
        mutexUnlock(&arrayList->mutex);
        return result;
    }
    /**
     * @brief Creates a ArrayList on the stack. Use isValidObject to check validity.
     * @param initialSize Initial size of internal array
     * @param elementSize Size of the largest element to be stored in array
     * @param elementsArePointers Are the elements that are going to be stored in this array going to be pointer to objects
     *
     */
    static ArrayList arrayListCreateStack(size_t initialSize, size_t elementSize, bool elementsArePointers) {
        ArrayList arrayList = {.dynamicContainer = containerDynamicCreateStack(initialSize, elementSize, elementsArePointers)};
        if (!isValidObject(&arrayList.header))
            return arrayList;
        arrayList.mutex = mutexCreate(MutexPlain);
        if (!mutexIsValid(&arrayList.mutex)) {
            containerDestroy(&arrayList);
            return arrayList;
        }
        arrayList.header |= ObjectFlagMutexExists;
        return arrayList;
    }
    /**
     * @brief Creates an ArrayList on the heap.
     * @param initialSize Initial size of internal array
     * @param elementSize Size of the largest element to be stored in array
     * @param elementsArePointers Are the elements that are going to be stored in this array going to be pointer to objects
     * @return NULL if allocation failed.
     */
    static ArrayList* arrayListCreateHeap(size_t initialSize, size_t elementSize, bool elementsArePointers) {
        ArrayList* arrayList = malloc(sizeof(*arrayList));
        if (!arrayList)
            return NULL;
        arrayList->dynamicContainer = containerDynamicCreateStack(initialSize, elementSize, elementsArePointers);
        if (!isValidObject((DataTypeFlags*) arrayList)) {
            free(arrayList);
            return NULL;
        }
        arrayList->header |= ObjectFlagIsOnHeap;
        arrayList->mutex = mutexCreate(MutexPlain);
        if (!mutexIsValid(&arrayList->mutex)) {
            containerDestroy(&arrayList);
            return arrayList;
        }
        arrayList->header |= ObjectFlagMutexExists;
        return arrayList;
    }
    /**
     * @brief Frees arraylist if applicable and puts it in an invalid state.
     * @param arrayList Pointer to ArrayList
     */
    static inline void arrayListDestroy(void* arrayList) {
        if (isValidObject(arrayList)) {
            mutexDestroy(&((ArrayList*) arrayList)->mutex);
            containerDestroy(arrayList);
        }
    }
    /**
     * @brief Frees ArrayList and executes destructor on each element.
     * @param arrayList Pointer to valid ArrayList
     * @param elementDestructor Destructor that will be executed on each element. Must be a valid reference
     */
    static inline void arrayListDestroyWithElements(ArrayList* arrayList, void(elementDestructor)(void* element)) {
        mutexDestroy(&arrayList->mutex);
        containerDynamicDestroyWithElements((DynamicContainer*) arrayList, elementDestructor);
    }

#ifdef __cplusplus
    }
};
#endif
#endif
