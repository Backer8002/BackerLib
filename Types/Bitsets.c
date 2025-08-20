#include "HashMap.h"
#include <stdbool.h>
#include <stdlib.h>

ContainerError bitSetGet(const BitSet* set, size_t index) {
    if (set->maxAmountOfElements <= index)
        return ContainerInvalidIndex;
    return (set->array[index / 8] & 0x80 >> (index % 8)) ? ContainerOPSuccessful : ContainerOPUnsuccessful;
}

ContainerError bitSetAdd(BitSet* set, size_t index) {
    ContainerError errorCode = bitSetGet(set, index);
    if ((errorCode != ContainerOPUnsuccessful))
        return errorCode;
    set->array[index / 8] |= 0x80 >> (index % 8);
    return ContainerOPUnsuccessful;
}

ContainerError bitSetRemove(BitSet* set, size_t index) {
    ContainerError errorCode = bitSetGet(set, index);
    if ((errorCode != ContainerOPSuccessful))
        return errorCode;
    set->array[index / 8] &= ~(0x80 >> (index % 8));
    return ContainerOPSuccessful;
}

bool bitSetIsEmpty(BitSet* set) {
    for (size_t iterator = 0; iterator < (set->maxAmountOfElements - 1) / 8; iterator++) {
        if (*(set->array + iterator) != 0)
            return false;
    }
    return true;
}

ContainerError bitSetAnd(BitSet* firstSet, BitSet* secondSet) {
    if (firstSet->maxAmountOfElements != secondSet->maxAmountOfElements)
        return ContainerOPUnsuccessful;

    for (size_t iterator = 0; iterator < (firstSet->maxAmountOfElements - 1) / 8; iterator++)
        *(firstSet->array + iterator) &= *(secondSet->array + iterator);

    return ContainerOPSuccessful;
}

ContainerError bitSetOr(BitSet* firstSet, BitSet* secondSet) {
    if (firstSet->maxAmountOfElements != secondSet->maxAmountOfElements)
        return ContainerOPUnsuccessful;

    for (size_t iterator = 0; iterator < (firstSet->maxAmountOfElements - 1) / 8; iterator++)
        *(firstSet->array + iterator) |= *(secondSet->array + iterator);

    return ContainerOPSuccessful;
}

ContainerError bitSetXOr(BitSet* firstSet, BitSet* secondSet) {
    if (firstSet->maxAmountOfElements != secondSet->maxAmountOfElements)
        return ContainerOPUnsuccessful;

    for (size_t iterator = 0; iterator < (firstSet->maxAmountOfElements - 1) / 8; iterator++)
        *(firstSet->array + iterator) ^= *(secondSet->array + iterator);

    return ContainerOPSuccessful;
}

void bitSetNot(BitSet* set) {
    for (size_t iterator = 0; iterator < (set->maxAmountOfElements - 1) / 8; iterator++)
        *(set->array + iterator) = ~*(set->array + iterator);
}

void bitSetDestroy(BitSet* set) {
    if (isValidObject((DataTypeFlags*) set)) {
        free(set->array);
        if (set->header & ObjectFlagIsOnHeap)
            free(set);
    }
}

BitSet bitSetCreate(size_t amountOfElements, bool objectIsHeapAllocated) {
    BitSet set = {0};
    set.array    = malloc((amountOfElements + sizeof(*set.array) - 1) / sizeof(*set.array));
    if (set.array == NULL)
        return set;

    set.maxAmountOfElements = amountOfElements;
    set.header              = (objectIsHeapAllocated ? ObjectFlagIsOnHeap : 0) | ObjectFlagIsValid;
    return set;
}
