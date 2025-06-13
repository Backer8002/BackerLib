// Cuckoo hash algorith mostly as described by https://en.wikipedia.org/wiki/Cuckoo_hashing, last updated 30 April 2025

#include<hashMap.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<assert.h>

static uint64_t m_hashMapHashSingelVarWithSalt(size_t elementSize, const void* element, ListTypes_t elementType,uint32_t salt, uint64_t (*hashFunction)(const void* element, size_t elementSize,uint32_t salt)) {
    if (typeIsPrimitive(elementType))
        return hashFunction(element, elementSize,salt);

    switch (elementType) {
    case ListArrayList:
    case ListString:
        return hashFunction(((ArrayList*) element)->list, ((ArrayList*) element)->amountOfElements * elementSize,salt);
        break;
    default:
        return 0;
        break;
    }
}

static bool m_memCmpKey(ListTypes_t keyType, size_t sizeOfKey, const void* key, const void* hashArrayElement) {
    if (hashArrayElement == NULL)
        return false;
    if (typeIsPrimitive(keyType))
        return (memcmp(key,hashArrayElement,sizeOfKey) == 0) ? true : false;

    switch (keyType) {
    case ListString:
        if (((ArrayList*) key)->amountOfElements != ((ArrayList*) hashArrayElement)->amountOfElements)
            return false;
        return (memcmp(((ArrayList*) key)->list, ((ArrayList*) hashArrayElement)->list, ((ArrayList*) key)->amountOfElements) == 0) ? true : false;
        break;
    default:
        return false;
        break;
    }
}

static void m_hashMapCuckooInit(HashMapCuckoo* newHashMap,size_t intialSize, size_t keySize, size_t elementSize, ListTypes_t keyType, ListTypes_t elementType,bool elementsArePointers, uint64_t(*hashFunction)(const void* element, size_t elementSize, uint32_t salt)) {
    newHashMap->hashArray = calloc(intialSize, sizeof(HashArrayElement));
    if (newHashMap->hashArray == NULL) {
        newHashMap->header.dataArrayVarType = ListNone;
        return;
    }
    if (hashFunction == NULL)
        newHashMap->hashFunction = hashFunctionDefualtSingleVarWithSalt;
    else
        newHashMap->hashFunction = hashFunction;
    newHashMap->sizeOfOneHashArray = intialSize/2;
    newHashMap->sizeOfKey               = keySize;
    newHashMap->sizeOfElement           = elementSize;
    newHashMap->keyType                 = keyType;
    newHashMap->header.dataArrayVarType = elementType;
    newHashMap->header.objectType       = ListHashMap;
    newHashMap->header.flags            = elementsArePointers ? ObjectFlagContentsIsPointers : 0;
    newHashMap->salt1                   = rand();
    newHashMap->salt2                   = rand();
}

HashMapCuckoo hashMapCuckooCreateStack(
    size_t intialSize, size_t keySize, size_t elementSize,
    ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers,
    uint64_t (*hashFunction)(const void* element, size_t elementSize,uint32_t salt)
) {
    HashMapCuckoo newHashMap;
    newHashMap.header.dataArrayVarType = ListNone;
    m_hashMapCuckooInit(&newHashMap, intialSize, keySize, elementSize, keyType, elementType, elementsArePointers, hashFunction);
    return newHashMap;
}

HashMapCuckoo* hashMapCuckooCreate(
    size_t intialSize, size_t keySize, size_t elementSize,
    ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers,
    uint64_t (*hashFunction)(const void* element, size_t elementSize,uint32_t salt)
) {
    HashMapCuckoo* newHashMap = malloc(sizeof(HashMapCuckoo));
    if (newHashMap == NULL)
        return NULL;
    m_hashMapCuckooInit(newHashMap, intialSize, keySize, elementSize, keyType, elementType, elementsArePointers, hashFunction);
    if (newHashMap->header.dataArrayVarType == ListNone) {
        free(newHashMap);
        return NULL;
    }
    newHashMap->header.flags |= ObjectFlagIsOnHeap;
    return newHashMap;
}

static bool m_rehash(HashMapCuckoo* hashMap) {
    size_t newAllocedSize = hashMap->sizeOfOneHashArray*2;
    size_t rehashCount = 0;
rehashNewAlloc:
    HashArrayElement* newHashArray = calloc(newAllocedSize, sizeof(HashArrayElement));
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
            uint64_t hash1 = m_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, elementToInsert.key, hashMap->keyType, newSalt1, hashMap->hashFunction) % (newAllocedSize / 2);
            HashArrayElement swapSpace;
            if (newHashArray[hash1].key == NULL) {
                newHashArray[hash1] = elementToInsert;
                break;
            }

            swapSpace                 = newHashArray[hash1];
            newHashArray[hash1] = elementToInsert;
            uint64_t hash2            = (m_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, swapSpace.key, hashMap->keyType, newSalt2, hashMap->hashFunction) % (newAllocedSize / 2)) + newAllocedSize / 2;
            if (newHashArray[hash2].key == NULL) {
                newHashArray[hash2] = swapSpace;
                break;
            }

            elementToInsert           = newHashArray[hash2];
            newHashArray[hash2] = swapSpace;
            if (++count >= (float)hashMap->sizeOfOneHashArray * MAX_CUCKOO_OF_SIZE) {
                if (++rehashCount >= MAX_REHASH) {
                    newAllocedSize *= 2;
                    rehashCount = 0;
                    free(newHashArray);
                    goto rehashNewAlloc;
                }
                memset(newHashArray, 0, newAllocedSize);
                goto rehashNoNewAlloc;
            }
        }
    }
    free(hashMap->hashArray);
    hashMap->hashArray = newHashArray;
    hashMap->sizeOfOneHashArray = newAllocedSize / 2;
    hashMap->salt1              = newSalt1;
    hashMap->salt2              = newSalt2;
    return true;
}

HashMapError_t hashMapCuckooInsert(HashMapCuckoo* hashMap, void** key, ListTypes_t keyType, void** element, ListTypes_t elementType) {
    assert(keyType == hashMap->keyType);
    assert(elementType == hashMap->header.dataArrayVarType);
    
    uint64_t hash1 = m_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey,*key,keyType,hashMap->salt1,hashMap->hashFunction) % hashMap->sizeOfOneHashArray;
    uint64_t hash2 = (m_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, *key, keyType, hashMap->salt2, hashMap->hashFunction) % hashMap->sizeOfOneHashArray) + hashMap->sizeOfOneHashArray;

    if (m_memCmpKey(keyType,hashMap->sizeOfKey, *key, hashMap->hashArray[hash1].key) || m_memCmpKey(keyType,hashMap->sizeOfKey, *key, hashMap->hashArray[hash2].key))
        return HashMapKeyAlreadyExists;

    void*            keyAlloced      = malloc(hashMap->sizeOfKey);
    if (keyAlloced == NULL)
        return HashMapCannotAllocMem;
    memcpy(keyAlloced, *key, hashMap->sizeOfKey);
    void* elementAlloced;

    if (hashMap->header.flags & ObjectFlagContentsIsPointers)
        elementAlloced = *element;
    else {
        elementAlloced = malloc(hashMap->sizeOfElement);
        if (elementAlloced == NULL) {
            free(keyAlloced);
            return HashMapCannotAllocMem;
        }
        memcpy(elementAlloced, *element, hashMap->sizeOfElement);
    }


    HashArrayElement elementToInsert = {.key = keyAlloced, .element = elementAlloced};
    size_t           count = 0;
    while (1) {
        HashArrayElement swapSpace;

        if (hashMap->hashArray[hash1].key == NULL) {
            hashMap->hashArray[hash1] = elementToInsert;
            return HashMapOperationSuccsess;
        }

        swapSpace = hashMap->hashArray[hash1];
        hashMap->hashArray[hash1] = elementToInsert;
        hash2                     = (m_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, swapSpace.key, keyType, hashMap->salt2, hashMap->hashFunction) % hashMap->sizeOfOneHashArray) + hashMap->sizeOfOneHashArray;
        if (hashMap->hashArray[hash2].key == NULL) {
            hashMap->hashArray[hash2] = swapSpace;
            return HashMapOperationSuccsess;
        }

        elementToInsert = hashMap->hashArray[hash2];
        hashMap->hashArray[hash2] = swapSpace;
        if (++count >= (float)hashMap->sizeOfOneHashArray * MAX_CUCKOO_OF_SIZE) {
            if (!m_rehash(hashMap)) {
                *key = elementToInsert.key;
                *element = elementToInsert.element;
                return HashMapCannotRealloc;
            }
            count = 0;
        }
        hash1                     = m_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, elementToInsert.key, keyType, hashMap->salt1, hashMap->hashFunction) % hashMap->sizeOfOneHashArray;
    }
}

void* hashMapCuckooGet(const HashMapCuckoo* hashMap, const void* key, ListTypes_t keyType) {
    assert(hashMap->keyType == keyType);
    uint64_t hash1 = m_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, key, keyType, hashMap->salt1, hashMap->hashFunction) % hashMap->sizeOfOneHashArray;

    if (m_memCmpKey(keyType, hashMap->sizeOfKey, key, hashMap->hashArray[hash1].key))
        return hashMap->hashArray[hash1].element;

    uint64_t hash2 = (m_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, key, keyType, hashMap->salt2, hashMap->hashFunction) % hashMap->sizeOfOneHashArray) + hashMap->sizeOfOneHashArray;
    if (m_memCmpKey(keyType, hashMap->sizeOfKey, key, hashMap->hashArray[hash2].key))
        return hashMap->hashArray[hash2].element;

    return NULL;
}

HashMapError_t hashMapCuckooRemove(HashMapCuckoo* hashMap, const void* key, ListTypes_t keyType, void (*keyDestructor)(void* object), void (*elementDestructor)(void* object)) {
    assert(hashMap->keyType == keyType);
    uint64_t hash1 = m_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, key, keyType, hashMap->salt1, hashMap->hashFunction) % hashMap->sizeOfOneHashArray;

    HashArrayElement thingToFree;
    if (m_memCmpKey(keyType, hashMap->sizeOfKey, key, hashMap->hashArray[hash1].key)) {
        thingToFree                   = hashMap->hashArray[hash1];
        hashMap->hashArray[hash1].key = NULL;
    } else {
        uint64_t hash2 = (m_hashMapHashSingelVarWithSalt(hashMap->sizeOfKey, key, keyType, hashMap->salt1, hashMap->hashFunction) % hashMap->sizeOfOneHashArray) + hashMap->sizeOfOneHashArray;
        if (!m_memCmpKey(keyType, hashMap->sizeOfKey, key, hashMap->hashArray[hash2].key))
            return HashMapKeyDoesNotExist;
        thingToFree    = hashMap->hashArray[hash2];
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