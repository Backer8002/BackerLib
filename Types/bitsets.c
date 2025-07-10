#include "backerLibListTypes.h"
#include "hashMap.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

// Returns the sign of an index in the bitset.
bool bitSetGet(BitSet* set, uint64_t hash) {
    assert(set->sizeOfArray >= hash);
    uint64_t byte = (uint64_t) floor((double) hash / 64);
    return (bool) ((uint64_t) 0x1 & ((*(set->array + byte)) >> (hash % 64)));
}

// Sets the index to high. Returns the sign of the index before operation.
bool bitSetAdd(BitSet* set, uint64_t hash) {
    if (bitSetGet(set, hash))
        return true;
    uint64_t byte = hash / 64;
    *(set->array + byte) |= (uint64_t) 1 << (hash % 64);
    return false;
}

// Sets the index to low. Returns the sign of the index before operation.
bool bitSetRemove(BitSet* set, uint64_t hash) {
    if (!bitSetGet(set, hash))
        return false;
    uint64_t byte = hash / 64;
    *(set->array + byte) &= ~((uint64_t) 1 << (hash % 64));
    return true;
}

// Returns true if all indexes are low.
bool bitSetIsEmpty(BitSet* set) {
    for (size_t iterator = 0; iterator < set->sizeOfArray * 64; iterator++)
        if (*(set->array + iterator) != 0)
            return false;
    return true;
}

// Preformes an and operation on two bitsets if there size is the same. Returns HashMapInvalidOperation if sizes are not equal.
HashMapError_t bitSetAnd(BitSet* firstSet, BitSet* secondSet) {
    if (firstSet->sizeOfArray != secondSet->sizeOfArray)
        return HashMapInvalidOperation;

    for (size_t iterator = 0; iterator < firstSet->sizeOfArray * 64; iterator++)
        *(firstSet->array + iterator) &= *(secondSet->array + iterator);

    return HashMapOperationSuccsess;
}
// Preformes an or operation on two bitsets if there size is the same. Returns HashMapInvalidOperation if sizes are not equal.
HashMapError_t bitSetOr(BitSet* firstSet, BitSet* secondSet) {
    if (firstSet->sizeOfArray != secondSet->sizeOfArray)
        return HashMapInvalidOperation;

    for (size_t iterator = 0; iterator < firstSet->sizeOfArray * 64; iterator++)
        *(firstSet->array + iterator) |= *(secondSet->array + iterator);

    return HashMapOperationSuccsess;
}

// Destroys a bitset.
void bitSetDestroy(BitSet* set) {
    free(set->array);
    if (set->header.flags & ObjectFlagIsOnHeap)
        free(set);
}

// Initilizes an bitset. Returns HashMapCannotAllocMem if it cannot alloc mem.
HashMapError_t bitSetInit(BitSet* set, size_t amountOfElements, bool objectIsHeapAlloced) {
    set->array = malloc(sizeof(uint64_t) * (size_t) ceil((float) amountOfElements / (float) 64));
    if (set->array == NULL)
        return HashMapCannotAllocMem;

    set->sizeOfArray       = (size_t) ceil((float) amountOfElements / 64.0f);
    set->header.objectType = ListBitSet;
    set->header.flags      = objectIsHeapAlloced ? ObjectFlagIsOnHeap : 0;
    return HashMapOperationSuccsess;
}