#include "HashMap.h"
#include <stdbool.h>
#include <stdlib.h>

BL_ContainerError bl_bitset_get(const BL_Bitset* set, size_t index) {
    if (set->maxAmountOfElements <= index)
        return BL_ContainerInvalidIndex;
    return (set->array[index / 8] & 0x80 >> (index % 8)) ? BL_ContainerOPSuccessful : BL_ContainerOPUnsuccessful;
}

BL_ContainerError bl_bitset_add(BL_Bitset* set, size_t index) {
    BL_ContainerError errorCode = bl_bitset_get(set, index);
    if ((errorCode != BL_ContainerOPUnsuccessful))
        return errorCode;
    set->array[index / 8] |= 0x80 >> (index % 8);
    return BL_ContainerOPUnsuccessful;
}

BL_ContainerError bl_bitset_remove(BL_Bitset* set, size_t index) {
    BL_ContainerError errorCode = bl_bitset_get(set, index);
    if ((errorCode != BL_ContainerOPSuccessful))
        return errorCode;
    set->array[index / 8] &= ~(0x80 >> (index % 8));
    return BL_ContainerOPSuccessful;
}

bool bl_bitset_is_empty(BL_Bitset* set) {
    for (size_t iterator = 0; iterator <= (set->maxAmountOfElements - 1) / 8; iterator++) {
        if (set->array[iterator] != 0)
            return false;
    }
    return true;
}

BL_ContainerError bl_bitset_and(BL_Bitset* firstSet, BL_Bitset* secondSet) {
    if (firstSet->maxAmountOfElements != secondSet->maxAmountOfElements)
        return BL_ContainerOPUnsuccessful;

    for (size_t iterator = 0; iterator <= (firstSet->maxAmountOfElements - 1) / 8; iterator++)
        *(firstSet->array + iterator) &= *(secondSet->array + iterator);

    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_bitset_or(BL_Bitset* firstSet, BL_Bitset* secondSet) {
    if (firstSet->maxAmountOfElements != secondSet->maxAmountOfElements)
        return BL_ContainerOPUnsuccessful;

    for (size_t iterator = 0; iterator <= (firstSet->maxAmountOfElements - 1) / 8; iterator++)
        *(firstSet->array + iterator) |= *(secondSet->array + iterator);

    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_bitset_xor(BL_Bitset* firstSet, BL_Bitset* secondSet) {
    if (firstSet->maxAmountOfElements != secondSet->maxAmountOfElements)
        return BL_ContainerOPUnsuccessful;

    for (size_t iterator = 0; iterator <= (firstSet->maxAmountOfElements - 1) / 8; iterator++)
        *(firstSet->array + iterator) ^= *(secondSet->array + iterator);

    return BL_ContainerOPSuccessful;
}

void bl_bitset_not(BL_Bitset* set) {
    for (size_t iterator = 0; iterator <= (set->maxAmountOfElements - 1) / 8; iterator++)
        *(set->array + iterator) = ~*(set->array + iterator);
}

void bl_bitset_destroy(BL_Bitset* set) {
    if (bl_bitset_is_valid(set)) {
        free(set->array);
        if (set->header & ObjectFlagIsOnHeap)
            free(set);
    }
}

bool bl_bitset_is_valid(const BL_Bitset* set) {
    return set->header & ObjectFlagIsValid;
}

BL_Bitset bl_bitset_create(size_t amountOfElements, bool objectIsHeapAllocated) {
    BL_Bitset set = {0};
    if (!amountOfElements)
        return set;
    set.array    = calloc((amountOfElements + sizeof(*set.array) - 1) / sizeof(*set.array),sizeof *set.array);
    if (set.array == NULL)
        return set;

    set.maxAmountOfElements = amountOfElements;
    set.header              = (objectIsHeapAllocated ? ObjectFlagIsOnHeap : 0) | ObjectFlagIsValid;
    return set;
}
