#ifndef CONTAINER_H
#define CONTAINER_H

#include "TypesMain.h"
#ifdef __cplusplus
extern "C" {
#else
#define noexcept
#endif

        typedef struct BL_Container {
            BL_DataTypeFlags header;
            uint32_t      byteSizeOfSingleElement;
            size_t        amountOfIndexes;
            void*         array;
        } BL_Container;

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
        extern BL_ContainerError bl_container_set(BL_Container* container,size_t index,size_t elementSize, const void* element) noexcept;
        /**
         * @brief Gets an element in the array.
         * @param container Pointer to valid container object
         * @param index index in container to access
         * @return NULL if index was out of range.
         * @return Pointer to object in array. If elementsArePointers were specified this instead returns the pointer to object within the array.
         */
        extern void* bl_container_get(const BL_Container* container,size_t index) noexcept;
        /**
         * @brief Makes a copy of subarray specefied by first and last index. Use is_valid to check validity of return object.
         * @param container Pointer to valid Container
         * @param firstIndex Lowest index to copy (inclusive)
         * @param lastIndex Highest index to copy (inclusive)
         * @param copyInReverse Should the elements be copied in reverse order
         * @return Container containing copy of subarray.
         */
        extern BL_Container bl_container_get_subarray(const BL_Container* container, size_t firstIndex, size_t lastIndex,bool copyInReverse) noexcept;
        /**
         * @brief Wrapper of bl_container_get_subarray(container,0,size,false)
         * @param container Pointer to valid Container
         * @return Invalid container if allocation failed.
         */
        extern BL_Container bl_container_copy(const BL_Container* container) noexcept;
        /**
         * @brief Reverses the order of elements in container inplace.
         * @param container Pointer to valid container
         */
        extern void bl_container_reverse(BL_Container* container) noexcept;
        /**
         * @brief Creates and returns a container allocated on the stack.
         * @param size Amount of elements to allocate for
         * @param elementSize Size of single element
         * @return Stack allocated Container. Use is_valid to determine validity.
         */
        extern BL_Container bl_container_create_stack(size_t size, size_t elementSize) noexcept;
        /**
         * @brief Creates and returns a container allocated on the heap.
         * @param size Amount of elements to allocate for
         * @param elementSize Size of single element
         * @return Heap allocated Container
         * @return NULL if alloc failed
         */
        extern BL_Container* bl_container_create_heap(size_t size, size_t elementSize) noexcept;
        /**
         * @brief Checks if container is empty.
         * @param container Pointer to valid Container
         * @return true if container does not contain any elements, else false.
         */
        extern bool bl_container_is_empty(const BL_Container* container) noexcept;
        /**
         * @param container Pointer to valid Container
         * @return Size of current container.
         */
        extern size_t bl_container_size(const BL_Container* container) noexcept;
        /**
         * @brief Returns a reference index in a container
         * @param container Pointer to valid container
         * @param reference Pointer to reference within container
         * @return reference's index in container
         * @note It is undefined behavoir to use a reference to outside the container.
         * @note Reference may point to any part of member object.
         */
        extern size_t bl_container_index_from_reference(const BL_Container* container, const void* reference) noexcept;
        /**
         * @brief Returns pointer to first index in container. Index is NULL if container is empty.
         * @param container Pointer to valid Container
         * @return Pointer to first index in container.
         */
        extern void* bl_container_front(const BL_Container* container) noexcept;
        /**
         * @brief Gets the next element in container.
         * @param container Pointer to valid Container
         * @param element Pointer to element in container
         * @return Next element in container, NULL if no such exists
         */
        extern void* bl_container_next(const BL_Container* container, const void* element) noexcept;
        /**
         * @brief Gets the next element in reversed order in container.
         * @param container Pointer to valid Container
         * @param element Pointer to element in container
         * @return Next element in reverse order in container, NULL if no such exists
         */
        extern void* bl_container_prev(const BL_Container* container, const void* element) noexcept;
        /**
         * @brief Returns pointer to last index in container. Index is NULL if container is empty
         * @param container Pointer to valid Container
         * @return Pointer to last index in container.
         */
        extern void* bl_container_back(const BL_Container* container) noexcept;
        /**
         * @brief Returns pointer to the first invalid index in the container.
         * @param container Pointer to valid Container
         * @return Pointer to the first invalid index in container.
         */
        extern void* bl_container_end(const BL_Container* container) noexcept;
        /**
         * @param container Pointer to container
         * @return true if container is a valid Container, else false
         */
        extern bool bl_container_is_valid(const BL_Container* container) noexcept;
        /**
         * @brief Destroys and frees object if applicable. Sets object state to invalid. Will not free container if amountOfIndexes is set to 0.
         * @param container Container to destroy
         */
        extern void bl_container_destroy(void* container) noexcept;

#ifdef __cplusplus
}
#else
#undef noexcept
#endif

#endif //CONTAINER_H
