// Cuckoo hash algorith mostly as described by https://en.wikipedia.org/wiki/Cuckoo_hashing, site last updated 30 April 2025

#include "HashMap.h"
#include "HashMap_internal.h"
#include "UnorderedContainer.h"
#include <stdalign.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static bool     internal_insertKeyValuePair(HashMapCuckoo*, size_t keyValuePairToInsert);

static uint64_t internal_hashMapHashSingleVarWithSalt(size_t elementSize, const void* element, bool elementIsDataTypeFlags, uint32_t salt, uint64_t (*hashFunction)(const void* element, size_t elementSize, uint32_t salt)) {
    if (elementIsDataTypeFlags && (*((DataTypeFlags*) element) & ObjectFlagIsContainer) && ((*(DataTypeFlags*) element) & ObjectFlagIsNotContinuous) == 0)
        return hashFunction(((Container*) element)->array, ((Container*) element)->byteSizeOfSingleElement * ((Container*) element)->amountOfIndexes, salt);

    return hashFunction(element, elementSize, salt);
}

static void internal_hashMapCuckooInit(HashMapCuckoo* hashMap, size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags, uint64_t (*hashFunction)(const void* element, size_t elementSize, uint32_t salt)) {
    if (!initialSize)
        initialSize = 1;
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
    hashMap->header |= (keyIsDataTypeFlags ? FlagHashMapKeyIsDataTypeFlags : 0) | FlagHashMapUsesCuckoo;
    hashMap->lengthOfHashArray = initialSize;
    hashMap->keySize           = keySize;
    hashMap->salt1             = rand();
    hashMap->salt2             = rand();
    hashMap->unorderedContainer.bitset[0] |= 0x80;
    hashMap->container.amountOfIndexes++;
}

static bool internal_checkSwapspace(HashMapCuckoo* hashMap, const void* key) {
    if (hashMap->header & HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ && hashMap->container.array != key) {
        if (!internal_insertKeyValuePair(hashMap, hashMap->swapspace))
            return false;
        hashMap->header &= ~HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ;
    }
    return true;
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
            uint64_t hash1 = internal_hashMapHashSingleVarWithSalt(hashMap->keySize,
                                                                   unorderedContainerGet((UnorderedContainer*) hashMap, elementToInsert).element,
                                                                   (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                                   newSalt1,
                                                                   hashMap->hashFunction) %
                             newLengthOfHashArray;
            if (newHashArray[hash1] == 0) {
                newHashArray[hash1] = elementToInsert;
                break;
            }

            size_t swapSpace    = newHashArray[hash1];
            newHashArray[hash1] = elementToInsert;
            uint64_t hash2      = (internal_hashMapHashSingleVarWithSalt(hashMap->keySize,
                                                                         unorderedContainerGet((UnorderedContainer*) hashMap, swapSpace).element,
                                                                    (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                                         newSalt2,
                                                                         hashMap->hashFunction) %
                              newLengthOfHashArray) +
                             newLengthOfHashArray;
            if (newHashArray[hash2] == 0) {
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
    if ((float) hashMap->lengthOfHashArray * HASHMAP_CUCKOO_MAX_CUCKOO_OF_SIZE * 2.0f < (float) hashMap->container.amountOfIndexes)
        internal_rehash(hashMap, true);
    while (1) {
        uint64_t hash1 = internal_hashMapHashSingleVarWithSalt(hashMap->keySize,
                                                               unorderedContainerGet((UnorderedContainer*) hashMap, keyValuePairToInsert).element,
                                                               (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                               hashMap->salt1,
                                                               hashMap->hashFunction) %
                         hashMap->lengthOfHashArray;
        if (!hashMap->hashArray[hash1]) {
            hashMap->hashArray[hash1] = keyValuePairToInsert;
            return true;
        }

        size_t swapSpace          = hashMap->hashArray[hash1];
        hashMap->hashArray[hash1] = keyValuePairToInsert;
        uint64_t hash2            = internal_hashMapHashSingleVarWithSalt(hashMap->keySize,
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

    if (!internal_checkSwapspace(hashMap, key))
        return ContainerAllocFailure;

    uint64_t hash1 = internal_hashMapHashSingleVarWithSalt(hashMap->keySize,
                                                           key,
                                                           (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                           hashMap->salt1,
                                                           hashMap->hashFunction) %
                     hashMap->lengthOfHashArray;
    uint64_t hash2 = internal_hashMapHashSingleVarWithSalt(hashMap->keySize,
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

    if (!internal_checkSwapspace(hashMap, NULL))
        return NULL;

    uint64_t hash1 = internal_hashMapHashSingleVarWithSalt(hashMap->keySize,
                                                           key,
                                                           (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                           hashMap->salt1,
                                                           hashMap->hashFunction) %
                     hashMap->lengthOfHashArray;

    if (hashMap->hashArray[hash1] && internal_memCmpKey(hashMap->keySize, key,
                                                        unorderedContainerGet((UnorderedContainer*) hashMap, hashMap->hashArray[hash1]).element,
                                                        (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false))
        return (Bytes) unorderedContainerGet((UnorderedContainer*) hashMap, hashMap->hashArray[hash1]).element + hashMap->elementOffset;

    uint64_t hash2 = internal_hashMapHashSingleVarWithSalt(hashMap->keySize,
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

    if (!internal_checkSwapspace(hashMap, NULL))
        return ContainerAllocFailure;

    void*    thingToFree;

    uint64_t hash1 = internal_hashMapHashSingleVarWithSalt(hashMap->keySize,
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
        uint64_t hash2 = internal_hashMapHashSingleVarWithSalt(hashMap->keySize,
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

ContainerError hashMapCuckooReplace(HashMapCuckoo* hashMap, size_t keySize, const void* key, size_t elementSize, void* element, void (*elementDestructor)(void* object)) {
    if (hashMap->keySize != keySize || hashMap->container.amountOfIndexes - hashMap->elementOffset < elementSize)
        return ContainerInvalidSize;

    if (!internal_checkSwapspace(hashMap, NULL))
        return ContainerAllocFailure;

    void* keyValuePair = hashMapCuckooGet(hashMap, keySize, key);
    if (!keyValuePair)
        return ContainerOPUnsuccessful;

    elementDestructor((Bytes) keyValuePair + hashMap->elementOffset);
    memcpy((Bytes) keyValuePair + hashMap->elementOffset, element, elementSize);
    return ContainerOPSuccessful;
}

void hashMapCuckooDestroy(HashMapCuckoo* hashMap, void (*keyDestructor)(void* object), void (*elementDestructor)(void* object)) {
    if (!isValidObject((DataTypeFlags*) hashMap))
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
