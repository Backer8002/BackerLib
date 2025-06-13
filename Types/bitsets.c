#include"pch.h"
#include<hashMap.h>
#include<assert.h>
#include<stdbool.h>
#include<math.h>
#include<stdlib.h>

bool bitSetGet(BitSet* set, uint64_t hash) {
    assert(set->sizeOfArray*64 >= hash);
    uint64_t byte = (uint64_t)floor((double)hash/64);
    return (bool)((uint64_t)0x1&((*(set->array+byte))>>(hash%64)));
}

bool bitSetAdd(BitSet* set,uint64_t hash) {
    if (bitSetGet(set,hash)) return true;
    uint64_t byte = (uint64_t)floor((double)hash/64);
    *(set->array+byte)|=(uint64_t)1<<(hash%64);
    return false;
}

bool bitSetRemove(BitSet* set, uint64_t hash) {
    if (!bitSetGet(set,hash)) return false;
    uint64_t byte = (uint64_t)floor((double)hash/64);
    *(set->array+byte)&=~((uint64_t)1<<(hash%64));
    return true;
}

bool bitSetCompare(BitSet* firstSet,BitSet* secondSet) {
    return (firstSet->header.dataArrayVarType == secondSet->header.dataArrayVarType && firstSet->hashFunction == secondSet->hashFunction);
}

bool bitSetIsEmpty(BitSet* set) {
    for(size_t iterator = 0; iterator < set->sizeOfArray ; iterator++)
        if(*(set->array+iterator)!=0) return false;
    return true;
}

int bitSetAnd(BitSet* firstSet,BitSet* secondSet) {
    if(bitSetCompare(firstSet,secondSet)) return -2;
    for(size_t iterator = 0; iterator <firstSet->sizeOfArray;iterator++)
        *(firstSet->array+iterator) &= *(secondSet->array+iterator);
    return 0;
}

void bitSetDestroy(BitSet* set) {
    free(set->array);
    free(set);
}