// Cuckoo hash algorith mostly as described by https://en.wikipedia.org/wiki/Cuckoo_hashing, site last updated 30 April 2025

#include "HashMap.h"
#include "UnorderedContainer.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint64_t internal_hashMapHashSingelVarWithSalt(size_t elementSize, const void* element, bool elementIsDataTypeFlags, uint32_t salt, uint64_t (*hashFunction)(const void* element, size_t elementSize, uint32_t salt)) {
    if (elementIsDataTypeFlags && (*(DataTypeFlags*) element & ObjectFlagIsContainer) && (*(DataTypeFlags*) element & ObjectFlagIsNotContinuous) == 0)
        return hashFunction(((Container*) element)->array, ((Container*) element)->byteSizeOfSingleElement * ((Container*) element)->amountOfIndexes, salt);

    return hashFunction(element, elementSize, salt);
}

static bool internal_memCmpKey(size_t keySize, const void* key, const void* hashArrayElement, bool isDataTypeFlagsQualified) {
    if (!hashArrayElement || !key)
        return false;
    if (isDataTypeFlagsQualified) {
        if ((DataTypeFlags*) key != (DataTypeFlags*) hashArrayElement)
            return false;
        if ((*(DataTypeFlags*) key & ObjectFlagIsContainer) && (*(DataTypeFlags*) key & ObjectFlagIsNotContinuous) == 0) {
            if (((Container*) key)->amountOfIndexes != ((Container*) hashArrayElement)->amountOfIndexes || ((Container*) key)->byteSizeOfSingleElement != ((Container*) hashArrayElement)->byteSizeOfSingleElement)
                return false;
            for (size_t i = 0; i < ((Container*) key)->amountOfIndexes; i++) {
                if (memcmp((Bytes) ((Container*) key)->array + ((Container*) key)->byteSizeOfSingleElement,
                           (Bytes) ((Container*) hashArrayElement)->array + ((Container*) key)->byteSizeOfSingleElement,
                           ((Container*) key)->byteSizeOfSingleElement) != 0)
                    return false;
            }
            return true;
        }
    }
    if (memcmp(key, hashArrayElement, keySize) != 0)
        return false;
    return true;
}

static void internal_hashMapCuckooInit(HashMapCuckoo* hashMap, size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags, uint64_t (*hashFunction)(const void* element, size_t elementSize, uint32_t salt)) {
    hashMap->hashArray = calloc(initialSize * 2, sizeof(*hashMap->hashArray));
    if (!hashMap->hashArray)
        return;

    if (hashFunction == NULL)
        hashMap->hashFunction = hashFunctionDefualtSingleVarWithSalt;
    else
        hashMap->hashFunction = hashFunction;
    size_t sizeOfKeyValuePair   = keySize + ((keySize % alignof(max_align_t)) ? (alignof(max_align_t) - keySize % alignof(max_align_t)) : 0) + elementSize + ((elementSize % alignof(max_align_t)) ? (alignof(max_align_t) - elementSize % alignof(max_align_t)) : 0);
    hashMap->elementOffset      = keySize + ((keySize % alignof(max_align_t)) ? (alignof(max_align_t) - keySize % alignof(max_align_t)) : 0);
    hashMap->unorderedContainer = unorderedContainerCreateStack(initialSize * 2 + 1, sizeOfKeyValuePair, false); // will be invalid index
    if (!isValidObject((DataTypeFlags*) hashMap)) {
        free(hashMap->hashArray);
        return;
    }
    hashMap->header            = (keyIsDataTypeFlags ? FlagHashMapKeyIsDataTypeFlags : 0);
    hashMap->lengthOfHashArray = initialSize;
    hashMap->keySize           = keySize;
    hashMap->salt1             = rand();
    hashMap->salt2             = rand();
    hashMap->unorderedContainer.bitset[0] |= 0x80;
    hashMap->container.amountOfIndexes++;
}

HashMapCuckoo hashMapCuckooCreateStack(size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags,
                                       uint64_t (*hashFunction)(const void* element, size_t elementSize, uint32_t salt)) {
    HashMapCuckoo hashMap = {0};
    internal_hashMapCuckooInit(&hashMap, initialSize, keySize, elementSize, keyIsDataTypeFlags, hashFunction);
    return hashMap;
}

HashMapCuckoo* hashMapCuckooCreateHeap(size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags,
                                       uint64_t (*hashFunction)(const void* element, size_t elementSize, uint32_t salt)) {
    HashMapCuckoo* hashMap = malloc(sizeof(*hashMap));
    if (!hashMap)
        return NULL;
    internal_hashMapCuckooInit(hashMap, initialSize, keySize, elementSize, keyIsDataTypeFlags, hashFunction);
    if (!isValidObject((DataTypeFlags*) hashMap)) {
        free(hashMap);
        return NULL;
    }
    hashMap->header |= ObjectFlagIsOnHeap;
    return hashMap;
}

static bool internal_rehash(HashMapCuckoo* hashMap, bool forcedResize) {
    size_t newLengthOfHashArray = forcedResize ? hashMap->lengthOfHashArray * 2 : hashMap->lengthOfHashArray;
rehashNewAlloc:
    size_t  rehashCount  = 0;
    size_t* newHashArray = calloc(newLengthOfHashArray * 2, sizeof(*newHashArray));
    if (newHashArray == NULL)
        return false;
rehashNoNewAlloc:
    uint32_t newSalt1 = rand();
    uint32_t newSalt2 = rand();

    for (size_t i = 0; i < hashMap->lengthOfHashArray * 2; i++) {
        size_t elementToInsert = hashMap->hashArray[i];
        if (!elementToInsert)
            continue;
