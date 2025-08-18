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
        if (*(DataTypeFlags*) key != *(DataTypeFlags*) hashArrayElement)
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
    hashMap->header            |= (keyIsDataTypeFlags ? FlagHashMapKeyIsDataTypeFlags : 0);
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

        size_t count = 0;
        while (1) {
            uint64_t hash1 = internal_hashMapHashSingelVarWithSalt(hashMap->keySize,
                                                                   unorderedContainerGet((UnorderedContainer*) hashMap, elementToInsert).element,
                                                                   (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                                   newSalt1,
                                                                   hashMap->hashFunction) %
                             newLengthOfHashArray;
            if (!newHashArray[hash1]) {
                newHashArray[hash1] = elementToInsert;
                break;
            }

            size_t swapSpace    = newHashArray[hash1];
            newHashArray[hash1] = elementToInsert;
            uint64_t hash2      = (internal_hashMapHashSingelVarWithSalt(hashMap->keySize,
                                                                         unorderedContainerGet((UnorderedContainer*) hashMap, swapSpace).element,
                                                                    (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                                         newSalt2,
                                                                         hashMap->hashFunction) %
                              newLengthOfHashArray) +
                             newLengthOfHashArray;
            if (!newHashArray[hash2]) {
                newHashArray[hash2] = swapSpace;
                break;
            }

            elementToInsert     = newHashArray[hash2];
            newHashArray[hash2] = swapSpace;
            if ((float) (++count) >= (float) hashMap->lengthOfHashArray * HASHMAP_CUCKOO_MAX_CUCKOO_OF_SIZE) {
                if (++rehashCount >= HASHMAP_CUCKOO_MAX_REHASH_BEFORE_RESIZE) {
                    newLengthOfHashArray *= 2;
                    free(newHashArray);
                    goto rehashNewAlloc;
                }
                memset(newHashArray, 0, newLengthOfHashArray * 2);
                goto rehashNoNewAlloc;
            }
        }
    }
    free(hashMap->hashArray);
    hashMap->hashArray         = newHashArray;
    hashMap->lengthOfHashArray = newLengthOfHashArray;
    hashMap->salt1             = newSalt1;
    hashMap->salt2             = newSalt2;
    return true;
}

static bool internal_insertKeyValuePair(HashMapCuckoo* hashMap, size_t keyValuePairToInsert) {
    size_t count       = 0;
    size_t rehashCount = 0;
    while (1) {
        uint64_t hash1 = internal_hashMapHashSingelVarWithSalt(hashMap->keySize,
                                                               unorderedContainerGet((UnorderedContainer*) hashMap, keyValuePairToInsert).element,
                                                               (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                               hashMap->salt1,
                                                               hashMap->hashFunction) %
                         hashMap->lengthOfHashArray;
        size_t swapSpace;
        if (!hashMap->hashArray[hash1]) {
            hashMap->hashArray[hash1] = keyValuePairToInsert;
            return true;
        }

        swapSpace                 = hashMap->hashArray[hash1];
        hashMap->hashArray[hash1] = keyValuePairToInsert;
        uint64_t hash2            = internal_hashMapHashSingelVarWithSalt(hashMap->keySize,
                                                                          unorderedContainerGet((UnorderedContainer*) hashMap, swapSpace).element,
                                                               (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                                          hashMap->salt2,
                                                                          hashMap->hashFunction) %
                             hashMap->lengthOfHashArray +
                         hashMap->lengthOfHashArray;
        if (!hashMap->hashArray[hash2]) {
            hashMap->hashArray[hash2] = swapSpace;
            return true;
        }

        keyValuePairToInsert      = hashMap->hashArray[hash2];
        hashMap->hashArray[hash2] = swapSpace;
        if ((float) (++count) >= (float) hashMap->lengthOfHashArray * HASHMAP_CUCKOO_MAX_CUCKOO_OF_SIZE) {
            if (++rehashCount > HASHMAP_CUCKOO_MAX_REHASH_BEFORE_RESIZE) {
                if (!internal_rehash(hashMap, true)) {
                    hashMap->swapspace = keyValuePairToInsert;
                    hashMap->header |= HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ;
                    return false;
                }
                rehashCount = 0;
            } else {
                if (!internal_rehash(hashMap, false)) {
                    hashMap->swapspace = keyValuePairToInsert;
                    hashMap->header |= HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ;
                    return false;
                }
            }
            count = 0;
        }
    }
}

ContainerError hashMapCuckooInsert(HashMapCuckoo* hashMap, size_t keySize, const void* key, size_t elementSize, const void* element) {
    if (hashMap->keySize != keySize || hashMap->container.byteSizeOfSingleElement - hashMap->elementOffset < elementSize)
        return ContainerInvalidSize;

    if (hashMap->header & HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ && hashMap->container.array != key) {
        if (!internal_insertKeyValuePair(hashMap, hashMap->swapspace))
            return ContainerAllocFailure;
        hashMap->header &= ~HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ;
    }

    uint64_t hash1 = internal_hashMapHashSingelVarWithSalt(hashMap->keySize,
                                                           key,
                                                           (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                           hashMap->salt1,
                                                           hashMap->hashFunction) %
                     hashMap->lengthOfHashArray;
    uint64_t hash2 = internal_hashMapHashSingelVarWithSalt(hashMap->keySize,
                                                           key,
                                                           (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                           hashMap->salt2,
                                                           hashMap->hashFunction) %
                         hashMap->lengthOfHashArray +
                     hashMap->lengthOfHashArray;
    if (hashMap->hashArray[hash1] && internal_memCmpKey(keySize, key, unorderedContainerGet((UnorderedContainer*) hashMap, hashMap->hashArray[hash1]).element, (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false))
        return ContainerOPUnsuccessful;
    if (hashMap->hashArray[hash2] && internal_memCmpKey(keySize, key, unorderedContainerGet((UnorderedContainer*) hashMap, hashMap->hashArray[hash2]).element, (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false))
        return ContainerOPUnsuccessful;

    UnorderedContainerPutResult result = unorderedContainerPut((UnorderedContainer*) hashMap, keySize, key);
    if (result.resultCode != ContainerOPSuccessful)
        return ContainerAllocFailure;
    memcpy((Bytes) unorderedContainerGet((UnorderedContainer*) hashMap, result.locationOfElement).element + hashMap->elementOffset, element, elementSize);
    internal_insertKeyValuePair(hashMap, result.locationOfElement);

    return ContainerOPSuccessful;
}

void* hashMapCuckooGet(HashMapCuckoo* hashMap, size_t keySize, const void* key) {
    if (keySize != hashMap->keySize)
        return NULL;

    if (hashMap->header & HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ && hashMap->container.array != key) {
        if (!internal_insertKeyValuePair(hashMap, hashMap->swapspace))
            return NULL;
        hashMap->header &= ~HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ;
    }
    uint64_t hash1 = internal_hashMapHashSingelVarWithSalt(hashMap->keySize,
                                                           key,
                                                           (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                           hashMap->salt1,
                                                           hashMap->hashFunction) %
                     hashMap->lengthOfHashArray;

    if (hashMap->hashArray[hash1] && internal_memCmpKey(hashMap->keySize,
                                                        key,
                                                        unorderedContainerGet((UnorderedContainer*) hashMap, hashMap->hashArray[hash1]).element,
                                                        (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false))
        return (Bytes) unorderedContainerGet((UnorderedContainer*) hashMap, hashMap->hashArray[hash1]).element + hashMap->elementOffset;

    uint64_t hash2 = internal_hashMapHashSingelVarWithSalt(hashMap->keySize,
                                                           key,
                                                           (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                           hashMap->salt2,
                                                           hashMap->hashFunction) %
                         hashMap->lengthOfHashArray +
                     hashMap->lengthOfHashArray;
    if (hashMap->hashArray[hash2] && internal_memCmpKey(hashMap->keySize,
                                                        key,
                                                        unorderedContainerGet((UnorderedContainer*) hashMap, hashMap->hashArray[hash2]).element,
                                                        (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false))
        return (Bytes) unorderedContainerGet((UnorderedContainer*) hashMap, hashMap->hashArray[hash2]).element + hashMap->elementOffset;

    return NULL;
}

ContainerError hashMapCuckooRemove(HashMapCuckoo* hashMap, size_t keySize, const void* key, void (*keyDestructor)(void* object), void (*elementDestructor)(void* object)) {
    if (keySize != hashMap->keySize)
        return ContainerInvalidSize;

    if (hashMap->header & HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ && hashMap->container.array != key) {
        if (!internal_insertKeyValuePair(hashMap, hashMap->swapspace))
            return ContainerAllocFailure;
        hashMap->header &= ~HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ;
    }

    void*    thingToFree;

    uint64_t hash1 = internal_hashMapHashSingelVarWithSalt(hashMap->keySize,
                                                           key,
                                                           (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                           hashMap->salt1,
                                                           hashMap->hashFunction) %
                     hashMap->lengthOfHashArray;

    if (hashMap->hashArray[hash1] && internal_memCmpKey(hashMap->keySize,
                                                        key,
                                                        unorderedContainerGet((UnorderedContainer*) hashMap, hashMap->hashArray[hash1]).element,
                                                        (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false))
        thingToFree = unorderedContainerGet((UnorderedContainer*) hashMap, hashMap->hashArray[hash1]).element;

    else {
        uint64_t hash2 = internal_hashMapHashSingelVarWithSalt(hashMap->keySize,
                                                               key,
                                                               (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                               hashMap->salt2,
                                                               hashMap->hashFunction) %
                             hashMap->lengthOfHashArray +
                         hashMap->lengthOfHashArray;
        if (hashMap->hashArray[hash2] && internal_memCmpKey(hashMap->keySize,
                                                            key,
                                                            unorderedContainerGet((UnorderedContainer*) hashMap, hashMap->hashArray[hash2]).element,
                                                            (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false))
            thingToFree = unorderedContainerGet((UnorderedContainer*) hashMap, hashMap->hashArray[hash2]).element;
        else
            return ContainerOPUnsuccessful;
    }

    if (keyDestructor)
        keyDestructor(thingToFree);
    if (elementDestructor)
        elementDestructor((Bytes) thingToFree + hashMap->elementOffset);

    return ContainerOPSuccessful;
}

void hashMapCuckooDestroy(HashMapCuckoo* hashMap, void (*keyDestructor)(void* object), void (*elementDestructor)(void* object)) {
    if (!isValidObject((DataTypeFlags*)hashMap))
        return;
    for (size_t i = (hashMap->header & HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ) ? 0 : 1; i < hashMap->container.amountOfIndexes; i++) {
        if ((hashMap->unorderedContainer.bitset[i / 8] & (0x80 >> (i % 8))) == 0)
            continue;
        if (keyDestructor)
            keyDestructor((Bytes) hashMap->container.array + hashMap->container.byteSizeOfSingleElement * i);
        if (elementDestructor)
            elementDestructor((Bytes) hashMap->container.array + hashMap->container.byteSizeOfSingleElement * i + hashMap->elementOffset);
    }
    free(hashMap->hashArray);
    unorderedContainerDestroy(hashMap);
}