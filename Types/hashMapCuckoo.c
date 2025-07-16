// Cuckoo hash algorith mostly as described by https://en.wikipedia.org/wiki/Cuckoo_hashing, site last updated 30 April 2025

#include "hashMap.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <BackerLibLogging.h>

static uint64_t Internal_hashMapHashSingelVarWithSalt(size_t elementSize, const void* element, ListTypes_t elementType, uint32_t salt, uint64_t (*hashFunction)(const void* element, size_t elementSize, uint32_t salt)) {
    if (typeIsPrimitive(elementType))
        return hashFunction(element, elementSize, salt);

    switch (elementType) {
    case ListArrayList:
    case ListString:
        return hashFunction(((ArrayList*) element)->list, ((ArrayList*) element)->amountOfElements * elementSize, salt);
        break;
    case ListCString:
        return hashFunction(element, strlen(element), salt);
        break;
    default:
        return 0;
        break;
    }
}

static bool CuckooInternal_memCmpKey(ListTypes_t keyType, size_t sizeOfKey, const void* key, const void* hashArrayElement) {
    if (hashArrayElement == NULL)
        return false;
    if (typeIsPrimitive(keyType))
        return (memcmp(key, hashArrayElement, sizeOfKey) == 0) ? true : false;

    switch (keyType) {
    case ListString:
        if (((ArrayList*) key)->amountOfElements != ((ArrayList*) hashArrayElement)->amountOfElements)
            return false;
        return (memcmp(((ArrayList*) key)->list, ((ArrayList*) hashArrayElement)->list, ((ArrayList*) key)->amountOfElements) == 0) ? true : false;
        break;
    case ListCString:
        size_t keySize = strlen(key);
        if (keySize != strlen(hashArrayElement))
            return false;
        return (memcmp(key, hashArrayElement, keySize) == 0) ? true : false;
        break;
    default:
        return false;
        break;
    }
}

static void Internal_hashMapCuckooInit(HashMapCuckoo* newHashMap, size_t intialSize, size_t keySize, size_t elementSize, ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers, uint64_t (*hashFunction)(const void* element, size_t elementSize, uint32_t salt)) {
    intialSize            = intialSize == 0 ? 1 : intialSize;
    newHashMap->hashArray = calloc(intialSize * 2, sizeof(HashArrayElement));
    if (newHashMap->hashArray == NULL) {
        newHashMap->header.dataArrayVarType = ListNone;
        return;
    }
    if (hashFunction == NULL)
        newHashMap->hashFunction = hashFunctionDefualtSingleVarWithSalt;
    else
        newHashMap->hashFunction = hashFunction;
    newHashMap->sizeOfOneHashArray      = intialSize;
    newHashMap->sizeOfKey               = keySize;
    newHashMap->sizeOfElement           = elementSize;
    newHashMap->keyType                 = keyType;
    newHashMap->header.dataArrayVarType = elementType;
    newHashMap->header.objectType       = ListHashMap;
    newHashMap->header.flags            = elementsArePointers ? ObjectFlagContentsIsPointers : 0;
    newHashMap->salt1                   = rand();
    newHashMap->salt2                   = rand();

    if (mtx_init(&newHashMap->mutex, mtx_plain) == thrd_success)
        newHashMap->header.flags |= ObjectFlagMutexExists;
}

HashMapCuckoo hashMapCuckooCreateStack(
    size_t intialSize, size_t keySize, size_t elementSize,
    ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers,
    uint64_t (*hashFunction)(const void* element, size_t elementSize, uint32_t salt)) {
    HashMapCuckoo newHashMap           = {0};
    newHashMap.header.dataArrayVarType = ListNone;
    Internal_hashMapCuckooInit(&newHashMap, intialSize, keySize, elementSize, keyType, elementType, elementsArePointers, hashFunction);
    return newHashMap;
}

HashMapCuckoo* hashMapCuckooCreate(
    size_t intialSize, size_t keySize, size_t elementSize,
    ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers,
    uint64_t (*hashFunction)(const void* element, size_t elementSize, uint32_t salt)) {
    HashMapCuckoo* newHashMap = malloc(sizeof(HashMapCuckoo));
    if (newHashMap == NULL)
        return NULL;
    Internal_hashMapCuckooInit(newHashMap, intialSize, keySize, elementSize, keyType, elementType, elementsArePointers, hashFunction);
    if (newHashMap->header.dataArrayVarType == ListNone) {
        free(newHashMap);
        return NULL;
    }
    newHashMap->header.flags |= ObjectFlagIsOnHeap;
    return newHashMap;
}

static bool Internal_rehash(HashMapCuckoo* hashMap) {
    size_t newAllocedSizeForOneHashArray = hashMap->sizeOfOneHashArray;
rehashNewAlloc:
    size_t            rehashCount  = 0;
    HashArrayElement* newHashArray = calloc(newAllocedSizeForOneHashArray * 2, sizeof(HashArrayElement));
    if (newHashArray == NULL)
        return false;
rehashNoNewAlloc:
    uint32_t newSalt1 = rand();
    uint32_t newSalt2 = rand();

    for (size_t i = 0; i < hashMap->sizeOfOneHashArray * 2; i++) {
        HashArrayElement elementToInsert = hashMap->hashArray[i];
        if (elementToInsert.key == NULL)
            continue;

        size_t count = 0;
        while (1) {
            uint64_t         hash1 = Internal_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, elementToInsert.key, hashMap->keyType, newSalt1, hashMap->hashFunction) % (newAllocedSizeForOneHashArray);
            HashArrayElement swapSpace;
            if (newHashArray[hash1].key == NULL) {
                newHashArray[hash1] = elementToInsert;
                break;
            }

            swapSpace           = newHashArray[hash1];
            newHashArray[hash1] = elementToInsert;
            uint64_t hash2      = (Internal_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, swapSpace.key, hashMap->keyType, newSalt2, hashMap->hashFunction) % (newAllocedSizeForOneHashArray)) + newAllocedSizeForOneHashArray;
            if (newHashArray[hash2].key == NULL) {
                newHashArray[hash2] = swapSpace;
                break;
            }

            elementToInsert     = newHashArray[hash2];
            newHashArray[hash2] = swapSpace;
            if (++count >= (float) hashMap->sizeOfOneHashArray * HASHMAP_CUCKOO_MAX_CUCKOO_OF_SIZE) {
                if (++rehashCount >= HASHMAP_CUCKOO_MAX_REHASH_BEFORE_RESIZE) {
                    newAllocedSizeForOneHashArray *= 2;
                    free(newHashArray);
                    goto rehashNewAlloc;
                }
                memset(newHashArray, 0, newAllocedSizeForOneHashArray);
                goto rehashNoNewAlloc;
            }
        }
    }
    free(hashMap->hashArray);
    hashMap->hashArray          = newHashArray;
    hashMap->sizeOfOneHashArray = newAllocedSizeForOneHashArray;
    hashMap->salt1              = newSalt1;
    hashMap->salt2              = newSalt2;
    return true;
}

HashMapError_t hashMapCuckooInsert(HashMapCuckoo* hashMap, const void* key, ListTypes_t keyType, const void* element, ListTypes_t elementType) {
    assert(keyType == hashMap->keyType);
    assert(elementType == hashMap->header.dataArrayVarType);

    if (hashMap->header.flags & HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ && hashMap->swapSpaceIfInvalidOperation.key != key) {
        if (hashMapCuckooInsert(hashMap, hashMap->swapSpaceIfInvalidOperation.key, keyType, hashMap->swapSpaceIfInvalidOperation.element, elementType) != HashMapOperationSuccsess)
            return HashMapCannotRealloc;
        hashMap->header.flags &= ~HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ;
        free(hashMap->swapSpaceIfInvalidOperation.key);
        free(hashMap->swapSpaceIfInvalidOperation.element);
    }

    uint64_t hash1 = Internal_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, key, keyType, hashMap->salt1, hashMap->hashFunction) % hashMap->sizeOfOneHashArray;
    uint64_t hash2 = (Internal_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, key, keyType, hashMap->salt2, hashMap->hashFunction) % hashMap->sizeOfOneHashArray) + hashMap->sizeOfOneHashArray;

    if (CuckooInternal_memCmpKey(keyType, hashMap->sizeOfKey, key, hashMap->hashArray[hash1].key) || CuckooInternal_memCmpKey(keyType, hashMap->sizeOfKey, key, hashMap->hashArray[hash2].key))
        return HashMapKeyAlreadyExists;

    size_t sizeOfKey  = keyType == ListCString ? strlen(key) + 1 : hashMap->sizeOfKey;
    void*  keyAlloced = malloc(sizeOfKey);
    if (keyAlloced == NULL)
        return HashMapCannotAllocMem;

    memcpy(keyAlloced, key, sizeOfKey);

    if (!typeIsPrimitive(keyType) && keyType != ListCString)
        ((DataTypeHeader*) keyAlloced)->flags |= ObjectFlagIsOnHeap;

    void* elementAlloced;
    if (hashMap->header.flags & ObjectFlagContentsIsPointers)
        elementAlloced = element;
    else {
        size_t sizeOfElement = elementType == ListCString ? strlen(element) + 1 : hashMap->sizeOfElement;
        elementAlloced       = malloc(sizeOfElement);
        if (elementAlloced == NULL) {
            free(keyAlloced);
            return HashMapCannotAllocMem;
        }
        memcpy(elementAlloced, element, sizeOfElement);
    }

    if (!typeIsPrimitive(elementType) && elementType != ListCString)
        ((DataTypeHeader*) elementAlloced)->flags |= ObjectFlagIsOnHeap;

    HashArrayElement elementToInsert = {.key = keyAlloced, .element = elementAlloced};
    size_t           count           = 0;
    while (1) {
        HashArrayElement swapSpace;
        if (hashMap->hashArray[hash1].key == NULL) {
            hashMap->hashArray[hash1] = elementToInsert;
            return HashMapOperationSuccsess;
        }

        swapSpace                 = hashMap->hashArray[hash1];
        hashMap->hashArray[hash1] = elementToInsert;
        hash2                     = (Internal_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, swapSpace.key, keyType, hashMap->salt2, hashMap->hashFunction) % hashMap->sizeOfOneHashArray) + hashMap->sizeOfOneHashArray;
        if (hashMap->hashArray[hash2].key == NULL) {
            hashMap->hashArray[hash2] = swapSpace;
            return HashMapOperationSuccsess;
        }

        elementToInsert           = hashMap->hashArray[hash2];
        hashMap->hashArray[hash2] = swapSpace;
        if (++count >= (float) hashMap->sizeOfOneHashArray * HASHMAP_CUCKOO_MAX_CUCKOO_OF_SIZE) {
            if (!Internal_rehash(hashMap)) {
                hashMap->swapSpaceIfInvalidOperation = elementToInsert;
                hashMap->header.flags |= HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ;
                return HashMapOperationSuccsess;
            }
            count = 0;
        }
        hash1 = Internal_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, elementToInsert.key, keyType, hashMap->salt1, hashMap->hashFunction) % hashMap->sizeOfOneHashArray;
    }
}

void* hashMapCuckooGet(HashMapCuckoo* hashMap, const void* key, ListTypes_t keyType) {
    assert(hashMap->keyType == keyType);

    if (hashMap->header.flags & HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ) {
        if (hashMapCuckooInsert(hashMap, hashMap->swapSpaceIfInvalidOperation.key, keyType, hashMap->swapSpaceIfInvalidOperation.element, hashMap->header.dataArrayVarType) != HashMapOperationSuccsess)
            return NULL;
        hashMap->header.flags &= ~HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ;
        free(hashMap->swapSpaceIfInvalidOperation.key);
        free(hashMap->swapSpaceIfInvalidOperation.element);
    }

    uint64_t hash1 = Internal_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, key, keyType, hashMap->salt1, hashMap->hashFunction) % hashMap->sizeOfOneHashArray;

    if (CuckooInternal_memCmpKey(keyType, hashMap->sizeOfKey, key, hashMap->hashArray[hash1].key))
        return hashMap->hashArray[hash1].element;

    uint64_t hash2 = (Internal_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, key, keyType, hashMap->salt2, hashMap->hashFunction) % hashMap->sizeOfOneHashArray) + hashMap->sizeOfOneHashArray;
    if (CuckooInternal_memCmpKey(keyType, hashMap->sizeOfKey, key, hashMap->hashArray[hash2].key))
        return hashMap->hashArray[hash2].element;

    return NULL;
}

HashMapError_t hashMapCuckooRemove(HashMapCuckoo* hashMap, const void* key, ListTypes_t keyType, void (*keyDestructor)(void* object), void (*elementDestructor)(void* object)) {
    assert(hashMap->keyType == keyType);

    if (hashMap->header.flags & HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ) {
        if (hashMapCuckooInsert(hashMap, hashMap->swapSpaceIfInvalidOperation.key, keyType, hashMap->swapSpaceIfInvalidOperation.element, hashMap->header.dataArrayVarType) != HashMapOperationSuccsess)
            return HashMapCannotRealloc;
        hashMap->header.flags &= ~HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ;
        free(hashMap->swapSpaceIfInvalidOperation.key);
        free(hashMap->swapSpaceIfInvalidOperation.element);
    }

    uint64_t         hash1 = Internal_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, key, keyType, hashMap->salt1, hashMap->hashFunction) % hashMap->sizeOfOneHashArray;

    HashArrayElement thingToFree;
    if (CuckooInternal_memCmpKey(keyType, hashMap->sizeOfKey, key, hashMap->hashArray[hash1].key)) {
        thingToFree                   = hashMap->hashArray[hash1];
        hashMap->hashArray[hash1].key = NULL;
    } else {
        uint64_t hash2 = (Internal_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, key, keyType, hashMap->salt1, hashMap->hashFunction) % hashMap->sizeOfOneHashArray) + hashMap->sizeOfOneHashArray;
        if (!CuckooInternal_memCmpKey(keyType, hashMap->sizeOfKey, key, hashMap->hashArray[hash2].key))
            return HashMapKeyDoesNotExist;
        thingToFree                   = hashMap->hashArray[hash2];
        hashMap->hashArray[hash2].key = NULL;
    }
    if (thingToFree.key == NULL)
        return HashMapKeyDoesNotExist;

    if (keyDestructor != NULL)
        keyDestructor(thingToFree.key);
    if (elementDestructor != NULL)
        elementDestructor(thingToFree.element);

    return HashMapOperationSuccsess;
}

void hashMapCuckooDestroy(HashMapCuckoo* hashMap, void (*keyDestructor)(void* object), void (*elementDestructor)(void* object)) {
    for (size_t i = 0; i < hashMap->sizeOfOneHashArray * 2; i++) {
        HashArrayElement* objectToFree = hashMap->hashArray + i;
        if (objectToFree->key == NULL)
            continue;
        if (keyDestructor)
            keyDestructor(objectToFree->key);
        if (elementDestructor && objectToFree->element)
            elementDestructor(objectToFree->element);
    }

    free(hashMap->hashArray);

    if (hashMap->header.flags & ObjectFlagIsOnHeap)
        free(hashMap);
}