#include "BL_DynamicContainer.h"

#include "BL_Container.h"
#include "TypesMain.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static void       internal_container_dynamic_init(BL_DynamicContainer* container, size_t initialSize, size_t elementSize);

BL_ContainerError bl_container_dynamic_set(BL_DynamicContainer* container, size_t index, size_t elementSize, const void* restrict element) {
    return bl_container_set(bl_container_ptr_cast_dynamic_container(container), index, elementSize, element);
}

void* bl_container_dynamic_get(const BL_DynamicContainer* container, size_t index) {
    return bl_container_get(bl_container_const_ptr_cast_dynamic_container(container), index);
}

BL_DynamicContainer bl_container_dynamic_get_subarray(const BL_DynamicContainer* container, size_t firstIndex, size_t lastIndex, bool copyInReverse) {
    return bl_dynamic_container_cast_container(bl_container_get_subarray(bl_container_const_ptr_cast_dynamic_container(container), firstIndex, lastIndex, copyInReverse));
}

BL_DynamicContainer bl_container_dynamic_copy(const BL_DynamicContainer* container) {
    return bl_dynamic_container_cast_container(bl_container_copy(bl_container_const_ptr_cast_dynamic_container(container)));
}

void bl_container_dynamic_reverse(BL_DynamicContainer* container) {
    bl_container_reverse(bl_container_ptr_cast_dynamic_container(container));
}

bool bl_container_dynamic_is_empty(const BL_DynamicContainer* container) {
    return container->container.amountOfIndexes == 0;
}

size_t bl_container_dynamic_size(const BL_DynamicContainer* container) {
    return container->container.amountOfIndexes;
}

size_t bl_container_dynamic_index_from_reference(const BL_DynamicContainer* container, const void* reference) {
    return bl_container_index_from_reference(bl_container_const_ptr_cast_dynamic_container(container), reference);
}

BL_ContainerError bl_container_dynamic_size_check_add(BL_DynamicContainer* container) {
    if (container->container.amountOfIndexes >= container->containerMaxSize) {
        size_t indexesToAssign = container->container.amountOfIndexes * 2 + 1;
        void*  newPointer      = container->containerMaxSize
                                   ? realloc(container->container.array, container->container.byteSizeOfSingleElement * indexesToAssign)
                                   : malloc(container->container.byteSizeOfSingleElement * indexesToAssign);
        if (newPointer == NULL)
            return BL_ContainerAllocFailure;
        container->container.array  = newPointer;
        container->containerMaxSize = indexesToAssign;
    }
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_container_dynamic_size_check_remove(BL_DynamicContainer* container) {
    if (container->containerMaxSize >> 2 > container->container.amountOfIndexes) {
        void* newPointer;
        if (container->container.amountOfIndexes) {
            newPointer = realloc(container->container.array, container->container.byteSizeOfSingleElement * container->container.amountOfIndexes);
            if (newPointer == NULL)
                return BL_ContainerAllocFailure;
        } else {
            free(container->container.array);
            newPointer = NULL;
        }
        container->container.array  = newPointer;
        container->containerMaxSize = container->container.amountOfIndexes;
    }
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_container_dynamic_reserve(BL_DynamicContainer* container, size_t amountOfIndexesToReserve) {
    if (container->containerMaxSize - container->container.amountOfIndexes >= amountOfIndexesToReserve)
        return BL_ContainerOPSuccessful;

    size_t newAmountOfIndexes = amountOfIndexesToReserve + container->container.amountOfIndexes;
    void*  newArray           = realloc(container->container.array, newAmountOfIndexes * container->container.byteSizeOfSingleElement);
    if (!newArray)
        return BL_ContainerAllocFailure;
    container->container.array  = newArray;
    container->containerMaxSize = newAmountOfIndexes;
    return BL_ContainerOPSuccessful;
}

inline void bl_container_dynamic_clear(BL_DynamicContainer* container) {
    container->container.amountOfIndexes = 0;
}

BL_ContainerError bl_container_dynamic_pop(BL_DynamicContainer* container) {
    if (container->container.amountOfIndexes == 0)
        return BL_ContainerInvalidIndex;
    container->container.amountOfIndexes--;
    bl_container_dynamic_size_check_remove(container);
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_container_dynamic_remove(BL_DynamicContainer* container, size_t index, size_t lastIndex) {
    if (index > lastIndex)
        return BL_ContainerInvalidIndex;
    if (lastIndex >= container->container.amountOfIndexes)
        return BL_ContainerInvalidIndex;
    if (lastIndex != container->container.amountOfIndexes - 1) {
        void*  currentIndex        = (BL_Bytes) container->container.array + index * container->container.byteSizeOfSingleElement;
        void*  nextIndex           = (BL_Bytes) container->container.array + (lastIndex + 1) * container->container.byteSizeOfSingleElement;
        size_t amountOfBytesToMove = (container->container.amountOfIndexes - lastIndex - 1) * container->container.byteSizeOfSingleElement;
        memmove(currentIndex, nextIndex, amountOfBytesToMove);
    }
    container->container.amountOfIndexes -= lastIndex - index + 1;
    bl_container_dynamic_size_check_remove(container);
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_container_dynamic_insert(BL_DynamicContainer* container, size_t index, size_t amountOfElements, size_t sizeOfElement, const void* elements) {
    if (index > container->container.amountOfIndexes)
        return BL_ContainerInvalidIndex;

    if (sizeOfElement > container->container.byteSizeOfSingleElement)
        return BL_ContainerInvalidSize;

    container->container.amountOfIndexes += amountOfElements;
    if (bl_container_dynamic_size_check_add(container) == BL_ContainerAllocFailure) {
        container->container.amountOfIndexes -= amountOfElements;
        return BL_ContainerAllocFailure;
    }

    memmove((BL_Bytes) container->container.array + (index + amountOfElements) * container->container.byteSizeOfSingleElement,
            (BL_Bytes) container->container.array + index * container->container.byteSizeOfSingleElement,
            (container->container.amountOfIndexes - amountOfElements - index) * container->container.byteSizeOfSingleElement);
    for (size_t i = 0; i < amountOfElements; i++) {
        memcpy((BL_Bytes) container->container.array + container->container.byteSizeOfSingleElement * (index + i),
               (BL_Bytes) elements + sizeOfElement * i,
               sizeOfElement);
    }
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_container_dynamic_insert_container(BL_DynamicContainer* restrict container, size_t index, const BL_Container* restrict containerToInsert) {
    if (index > container->container.amountOfIndexes)
        return BL_ContainerInvalidIndex;

    if (containerToInsert->byteSizeOfSingleElement > container->container.byteSizeOfSingleElement)
        return BL_ContainerInvalidSize;

    container->container.amountOfIndexes += containerToInsert->amountOfIndexes;
    if (bl_container_dynamic_size_check_add(container) == BL_ContainerAllocFailure) {
        container->container.amountOfIndexes -= containerToInsert->amountOfIndexes;
        return BL_ContainerAllocFailure;
    }

    memmove((BL_Bytes) container->container.array + (index + containerToInsert->amountOfIndexes) * container->container.byteSizeOfSingleElement,
            (BL_Bytes) container->container.array + index * container->container.byteSizeOfSingleElement,
            (container->container.amountOfIndexes - containerToInsert->amountOfIndexes - index) * container->container.byteSizeOfSingleElement);
    for (size_t i = 0; i < containerToInsert->amountOfIndexes; i++) {
        memcpy((BL_Bytes) container->container.array + container->container.byteSizeOfSingleElement * (index + i),
               (BL_Bytes) containerToInsert->array + containerToInsert->byteSizeOfSingleElement * i,
               containerToInsert->byteSizeOfSingleElement);
    }
    return BL_ContainerOPSuccessful;
}

inline BL_ContainerError bl_container_dynamic_append(BL_DynamicContainer* container, size_t sizeOfElement, const void* element) {
    return bl_container_dynamic_insert(container,container->container.amountOfIndexes,1,sizeOfElement,element);
}

static void internal_container_dynamic_init(BL_DynamicContainer* container, size_t initialSize, size_t elementSize) {
    container->container.amountOfIndexes         = 0;
    container->container.header                  = 0;
    container->container.byteSizeOfSingleElement = elementSize;
    container->containerMaxSize                  = initialSize;

    container->container.array                   = initialSize ? malloc(elementSize * initialSize) : NULL;
    if (container->container.array == NULL && initialSize)
        return;
    container->container.header = ObjectFlagIsValid | ObjectFlagIsDynamicContainer | ObjectFlagIsContainer;
}

BL_DynamicContainer bl_dynamic_container_cast_container(BL_Container container) {
    BL_DynamicContainer containerToReturn = {.container = container, .containerMaxSize = container.amountOfIndexes};
    container.header |= ObjectFlagIsDynamicContainer;
    return containerToReturn;
}

BL_Container bl_container_cast_dynamic_container(BL_DynamicContainer container) {
    return container.container;
}

BL_Container* bl_container_ptr_cast_dynamic_container(BL_DynamicContainer* container) {
    return &container->container;
}

const BL_Container* bl_container_const_ptr_cast_dynamic_container(const BL_DynamicContainer* container) {
    return &container->container;
}

BL_DynamicContainer bl_container_dynamic_create_stack(size_t initialSize, size_t elementSize) {
    BL_DynamicContainer container = {0};
    internal_container_dynamic_init(&container, initialSize, elementSize);
    return container;
}

BL_DynamicContainer* bl_container_dynamic_create_heap(size_t initialSize, size_t elementSize) {
    BL_DynamicContainer* container = malloc(sizeof(*container));
    if (container == NULL)
        return NULL;
    internal_container_dynamic_init(container, initialSize, elementSize);
    if (bl_container_dynamic_is_valid(container)) {
        bl_container_ptr_cast_dynamic_container(container)->header |= ObjectFlagIsOnHeap;
        return container;
    }
    free(container);
    return NULL;
}

void bl_container_dynamic_destroy_with_elements(BL_DynamicContainer* container, void(elementDestructor)(void* element)) {
    for (size_t i = 0; i < container->container.amountOfIndexes; i++)
        elementDestructor((BL_Bytes) container->container.array + container->container.byteSizeOfSingleElement * i);
    bl_container_destroy(container);
}

void* bl_container_dynamic_front(const BL_DynamicContainer* container) {
    return bl_container_front(&container->container);
}

void* bl_container_dynamic_next(const BL_DynamicContainer* container, const void* element) {
    return bl_container_next(&container->container, element);
}

void* bl_container_dynamic_prev(const BL_DynamicContainer* container, const void* element) {
    return bl_container_prev(&container->container, element);
}

void* bl_container_dynamic_back(const BL_DynamicContainer* container) {
    return bl_container_back(&container->container);
}

void* bl_container_dynamic_end(const BL_DynamicContainer* container) {
    return bl_container_end((BL_Container*) container);
}

bool bl_container_dynamic_is_valid(const BL_DynamicContainer* container) {
    return container->container.header & ObjectFlagIsValid && container->container.header & ObjectFlagIsDynamicContainer && container->container.header & ObjectFlagIsContainer;
}

void bl_container_dynamic_destroy(void* obj) {
    if (obj)
        bl_container_destroy(bl_container_ptr_cast_dynamic_container(obj));
}