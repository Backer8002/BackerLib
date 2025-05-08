#include"pch.h"
#include<hashMap.h>
#include<assert.h>
#include<stdbool.h>
#include<math.h>
#include<stdlib.h>

#include<stdio.h>

bool setGet(Set* set, uint64_t hash) {
    printf("SizeOfArray: %llu, Hash: %llu\n",set->sizeOfArray,hash);
    assert(set->sizeOfArray*64 >= hash);
    uint64_t byte = (uint64_t)floor((double)hash/64);
    return (bool)((uint64_t)0x1&((*(set->array+byte))>>(hash%64)));
}

bool setAdd(Set* set,uint64_t hash) {
    if (setGet(set,hash)) return true;
    uint64_t byte = (uint64_t)floor((double)hash/64);
    *(set->array+byte)|=(uint64_t)1<<(hash%64);
    return false;
}

bool setRemove(Set* set, uint64_t hash) {
    if (!setGet(set,hash)) return false;
    uint64_t byte = (uint64_t)floor((double)hash/64);
    *(set->array+byte)&=~((uint64_t)1<<(hash%64));
    return true;
}

bool setCompare(Set* firstSet,Set* secondSet) {
    return (firstSet->typeOfHash == secondSet->typeOfHash && firstSet->theHashOfTheSet == secondSet->theHashOfTheSet);
}

bool setIsEmpty(Set* set) {
    for(size_t iterator = 0; iterator < set->sizeOfArray ; iterator++)
        if(*(set->array+iterator)!=0) return false;
    return true;
}

int setAnd(Set* firstSet,Set* secondSet) {
    if(!firstSet->equalityChecker(firstSet,secondSet)) return -2;
    for(size_t iterator = 0; iterator <firstSet->sizeOfArray;iterator++)
        *(firstSet->array+iterator) &= *(secondSet->array+iterator);
    return 0;
}
