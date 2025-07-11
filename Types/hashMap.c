#define HashMapImplementation

#include "hashMap.h"
#include "arrayList.h"
#include "backerLibListTypes.h"
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


uint64_t hashFunctionDefualt(size_t amountOfVars, ...) {
    if (amountOfVars == 0)
        return 0;
    va_list argsToHash = {0};
    va_start(argsToHash, amountOfVars);
    uint64_t hash = 0;
    for (size_t j = 0; j < amountOfVars; j++) {
        size_t amountOfBytes = va_arg(argsToHash, size_t);
        Byte*  valuesToHash  = va_arg(argsToHash, Byte*);
        hash ^= (uint64_t) valuesToHash[0] ^ ~(amountOfBytes << 10);
        for (size_t i = 0; i < amountOfBytes; i++)
            hash ^= (((uint64_t) valuesToHash[i] + hash) * (hash + (hash << 5)));
    }
    va_end(argsToHash);
    return hash;
}


inline static void internal_hashMapCreateInit(
    HashMap* newHashMap,
    size_t intialSize, size_t keySize, size_t elementSize,
    ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers,
    uint64_t (*hashFunction)(const void* element, size_t elementSize)) {

    if (hashFunction == NULL)
        newHashMap->hashFunction = hashFunctionDefualtSingleVar;
    else
        newHashMap->hashFunction = hashFunction;

    newHashMap->sizeOfHashArray = intialSize;
    newHashMap->hashArray       = calloc(intialSize, sizeof(HashArrayNode*));

    if (newHashMap->hashArray == NULL) {
        newHashMap->header.dataArrayVarType = ListNone;
        return;
    }

    newHashMap->sizeOfKey               = keySize;
    newHashMap->sizeOfElement           = elementSize;
    newHashMap->amountOfElements        = 0;
    newHashMap->keyType                 = keyType;
    newHashMap->header.dataArrayVarType = elementType;
    newHashMap->header.flags            = ((elementsArePointers) ? ObjectFlagContentsIsPointers : 0);
    newHashMap->header.objectType       = ListHashMap;

    if (mtx_init(&newHashMap->mutex, mtx_plain) == thrd_success)
        newHashMap->header.flags |= ObjectFlagMutexExists;
}

HashMap hashMapCreateStack(
    size_t intialSize, size_t keySize, size_t elementSize,
    ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers,
    uint64_t (*hashFunction)(const void* element, size_t elementSize)) {
    HashMap newHashMap                 = {0};
    newHashMap.header.dataArrayVarType = ListNone;
    internal_hashMapCreateInit(&newHashMap, intialSize, keySize, elementSize, keyType, elementType, elementsArePointers, hashFunction);
    return newHashMap;
}

HashMap* hashMapCreate(
    size_t intialSize, size_t keySize, size_t elementSize,
    ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers,
    uint64_t (*hashFunction)(const void* element, size_t elementSize)) {
    HashMap* newHashMap = malloc(sizeof(HashMap));
    if (newHashMap == NULL)
        return NULL;
    internal_hashMapCreateInit(newHashMap, intialSize, keySize, elementSize, keyType, elementType, elementsArePointers, hashFunction);
    if (newHashMap->header.dataArrayVarType == ListNone) {
        free(newHashMap);
        return NULL;
    }
    newHashMap->header.flags |= ObjectFlagIsOnHeap;
    return newHashMap;
}

inline Set setCreateStack(
    size_t intialSize, size_t keySize,
    ListTypes_t keyType,
    uint64_t (*hashFunction)(const void* element, size_t elementSize)) {
    Set newSet                       = {hashMapCreateStack(intialSize, keySize, keySize, keyType, keyType, false, hashFunction)};
    newSet.hashMap.header.objectType = ListSet;
    return newSet;
}

inline Set* setCreate(
    size_t intialSize, size_t keySize,
    ListTypes_t keyType,
    uint64_t (*hashFunction)(const void* element, size_t elementSize)) {
    HashMap* newSet = hashMapCreate(intialSize, keySize, keySize, keyType, keyType, false, hashFunction);
    if (newSet != NULL)
        newSet->header.objectType = ListSet;
    return (Set*) newSet;
}

static uint64_t internal_hashMapHashSingelVar(size_t elementSize, const void* element, ListTypes_t elementType, uint64_t (*hashFunction)(const void* element, size_t elementSize)) {
    if (typeIsPrimitive(elementType))
        return hashFunction(&element, elementSize);

    switch (elementType) {
    case ListArrayList:
    case ListString:
        return hashFunction(((ArrayList*) element)->list, ((ArrayList*) element)->amountOfElements * elementSize);
        break;
    case ListCString:
        return hashFunction(element, strlen(element));
        break;
    default:
        return 0;
        break;
    }
}

static bool internal_memCmpKey(ListTypes_t keyType, const void* key, size_t keySize, const void* hashArrayElement) {
    if (typeIsPrimitive(keyType))
        return (key == hashArrayElement) ? true : false;

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

static HashMapError_t internal_rehash(HashMap* hashMap, size_t newSize) {
    HashArrayNode** newHashArray = calloc(newSize, sizeof(HashArrayNode*));
    if (newHashArray == NULL)
        return HashMapCannotAllocMem;

    for (HashArrayNode** nextBucket = hashMap->hashArray; nextBucket < hashMap->hashArray + hashMap->sizeOfHashArray; nextBucket++) {
        HashArrayNode* next = *nextBucket;
        HashArrayNode* oldNext;
        while (next) {
            oldNext               = next;
            next                  = next->next;
            uint64_t newHash      = internal_hashMapHashSingelVar(hashMap->sizeOfKey, oldNext->key, hashMap->keyType, hashMap->hashFunction) % newSize;
            oldNext->next         = (newHashArray[newHash] == NULL) ? NULL : newHashArray[newHash];
            newHashArray[newHash] = oldNext;
        }
    }
    free(hashMap->hashArray);
    hashMap->hashArray       = newHashArray;
    hashMap->sizeOfHashArray = newSize;
    return HashMapOperationSuccsess;
}

HashMapError_t hashMapInsert(HashMap* hashMap, void* key, ListTypes_t keyType, void* element, ListTypes_t elementType) {
    assert(keyType == hashMap->keyType);
    assert(elementType == hashMap->header.dataArrayVarType);

    uint64_t       hash         = internal_hashMapHashSingelVar(hashMap->sizeOfElement, key, keyType, hashMap->hashFunction) % hashMap->sizeOfHashArray;

    size_t         iterator     = 1;
    HashArrayNode* hashNode     = hashMap->hashArray[hash];
    HashArrayNode* prevHashNode = hashNode;

    if (hashNode != NULL) {
        while (hashNode->next != NULL) {
            if (internal_memCmpKey(keyType, key, hashMap->sizeOfKey, hashNode->key))
                return HashMapKeyAlreadyExists;

            hashNode = hashNode->next;
            iterator++;
        }
        hashNode->next = malloc(sizeof(HashArrayNode));
        if (hashNode->next == NULL)
            return HashMapCannotAllocMem;
        hashNode = hashNode->next;
    } else {
        hashNode = malloc(sizeof(HashArrayNode));
        if (hashNode == NULL)
            return HashMapCannotAllocMem;
        hashMap->hashArray[hash] = hashNode;
    }

    if (typeIsPrimitive(keyType))
        hashNode->key = key;
    else {
        size_t sizeOfKey = keyType == ListCString ? strlen(key) + 1 : hashMap->sizeOfKey;
        hashNode->key    = malloc(sizeOfKey);
        if (hashNode->key == NULL) {
            if (hashNode == hashMap->hashArray[hash])
                goto HashMapInsertErrorExitFirst;
            goto HashMapInsertErrorExitNotFirst;
        }
        memcpy(hashNode->key, key, sizeOfKey);

        if (keyType != ListCString)
            ((DataTypeHeader*) hashNode->key)->flags |= ObjectFlagIsOnHeap;
    }


    if ((hashMap->header.flags & ObjectFlagContentsIsPointers) || hashMap->header.objectType == ListSet)
        hashNode->element = element;
    else {
        size_t sizeOfElement = elementType == ListCString ? strlen(element) + 1 : hashMap->sizeOfElement;
        hashNode->element    = malloc(sizeOfElement);

        if (hashNode->element == NULL) {
            if (!typeIsPrimitive(keyType))
                free(hashNode->key);
            if (hashNode == hashMap->hashArray[hash])
                goto HashMapInsertErrorExitFirst;
            goto HashMapInsertErrorExitNotFirst;
        }
        memcpy(hashNode->element, element, sizeOfElement);
        if (!typeIsPrimitive(elementType) && elementType != ListCString)
            ((DataTypeHeader*) hashNode->element)->flags |= ObjectFlagIsOnHeap;
    }

    hashNode->next = NULL;

    hashMap->amountOfElements++;

    if ((float) hashMap->amountOfElements / (float) hashMap->sizeOfHashArray >= HASHMAP_MAX_LOADFACTOR || iterator >= HASHMAP_MAX_DEPTH)
        internal_rehash(hashMap, hashMap->sizeOfHashArray * 2 + 1);
    return HashMapOperationSuccsess;

HashMapInsertErrorExitFirst:
    hashMap->hashArray[hash] = NULL;
    free(hashNode);
    return HashMapCannotAllocMem;
HashMapInsertErrorExitNotFirst:
    prevHashNode->next = NULL;
    free(hashNode);
    return HashMapCannotAllocMem;
}

inline HashMapError_t setInsert(HashMap* hashMap, void* key, ListTypes_t keyType) { return hashMapInsert(hashMap, key, keyType, NULL, keyType); }

static HashArrayNode* internal_hashArrayGetFromKey(const HashMap* hashMap, const void* key, ListTypes_t keyType) {
    assert(keyType == hashMap->keyType);

    uint64_t       hash     = internal_hashMapHashSingelVar(hashMap->sizeOfElement, key, keyType, hashMap->hashFunction) % hashMap->sizeOfHashArray;

    HashArrayNode* hashNode = hashMap->hashArray[hash];
    if (hashNode == NULL)
        return NULL;

    while (!internal_memCmpKey(keyType, key, hashMap->sizeOfKey, hashNode->key)) {
        if (hashNode->next == NULL)
            return NULL;
        hashNode = hashNode->next;
    }
    return hashNode;
}

HashMapError_t hashMapGet(const HashMap* hashMap, const void* key, ListTypes_t keyType, void** element) {
    assert(keyType == hashMap->keyType);
    assert(hashMap->header.objectType == ListHashMap);


    HashArrayNode* hashArrayNode = internal_hashArrayGetFromKey(hashMap, key, keyType);
    if (hashArrayNode == NULL)
        return HashMapKeyDoesNotExist;

    *element = hashArrayNode->element;
    return HashMapOperationSuccsess;
}

inline bool setGet(const HashMap* hashMap, const void* key, ListTypes_t keyType) {
    assert(keyType == hashMap->keyType);
    return m_hashArrayGetFromKey(hashMap,key,keyType) != NULL ? true : false;
}

HashMapError_t hashMapRemove(HashMap* hashMap, const void* key, ListTypes_t keyType, void (*keyDestructor)(void* element), void (*elementDestructor)(void* element)) {
    assert(keyType == hashMap->keyType);

    uint64_t       hash     = internal_hashMapHashSingelVar(hashMap->sizeOfKey, key, keyType, hashMap->hashFunction) % hashMap->sizeOfHashArray;

    HashArrayNode* hashNode = hashMap->hashArray[hash];
    if (hashNode == NULL)
        return HashMapKeyDoesNotExist;
    HashArrayNode* prevHashNode = hashNode;
    while (!internal_memCmpKey(keyType, key, hashMap->sizeOfKey, hashNode->key)) {
        if (hashNode->next == NULL)
            return HashMapKeyDoesNotExist;

        prevHashNode = hashNode;
        hashNode     = hashNode->next;
    }

    if (keyDestructor == NULL || typeIsPrimitive(keyType))
        ;
    else
        keyDestructor(hashNode->key);

    if ((hashMap->header.flags & ObjectFlagContentsIsPointers) || elementDestructor == NULL)
        ;
    else if (typeIsPrimitive(hashMap->header.dataArrayVarType))
        free(hashNode->element);
    else
        elementDestructor(hashNode->element);

    if (hashNode == prevHashNode)
        hashMap->hashArray[hash] = hashNode->next;
    else {
        if (hashNode->next != NULL)
            prevHashNode->next = hashNode->next;
        else
            prevHashNode->next = NULL;
    }
    free(hashNode);
    hashMap->amountOfElements--;
    return HashMapOperationSuccsess;
}

inline HashMapError_t setRemove(HashMap* hashMap, void* key, ListTypes_t keyType, void (*keyDestructor)(void* element)) { return hashMapRemove(hashMap, key, keyType, keyDestructor, NULL); }

HashMapError_t        hashMapReplace(HashMap* hashMap, const void* key, ListTypes_t keyType, void* element, ListTypes_t elementType, void (*destrutorOfPreviousElement)(void* element)) {
    assert(keyType == hashMap->keyType);
    assert(elementType == hashMap->header.dataArrayVarType);
    assert(hashMap->header.objectType == ListHashMap);

    HashArrayNode* hashArrayNode = internal_hashArrayGetFromKey(hashMap, key, keyType);
    if (hashArrayNode == NULL)
        return HashMapKeyDoesNotExist;

    if (hashMap->header.flags & ObjectFlagContentsIsPointers) {
        hashArrayNode->element = element;
        return HashMapOperationSuccsess;
    }

    if (destrutorOfPreviousElement != NULL) {
        destrutorOfPreviousElement(hashArrayNode->element);
        hashArrayNode->element = malloc(hashMap->sizeOfElement);
        if (hashArrayNode->element == NULL)
            return HashMapCannotAllocMem;
    }
    memcpy(hashArrayNode->element, element, elementType == ListCString ? strlen(element) + 1 : hashMap->sizeOfElement);

    if (!typeIsPrimitive(elementType) && elementType != ListCString)
        ((DataTypeHeader*) hashArrayNode->element)->flags |= ObjectFlagIsOnHeap;

    return HashMapOperationSuccsess;
}


void hashMapDestroy(HashMap* hashMap, void (*keyDestructor)(void* key), void (*elementDestructor)(void* element)) {
    for (HashArrayNode** nextBucket = hashMap->hashArray; nextBucket < hashMap->hashArray + hashMap->sizeOfHashArray; nextBucket++) {
        HashArrayNode* next = *nextBucket;
        HashArrayNode* oldNext;
        while (next) {
            oldNext = next;
            next    = next->next;
            if (keyDestructor != NULL && !typeIsPrimitive(hashMap->keyType))
                keyDestructor(oldNext->key);
            if (elementDestructor != NULL && !(hashMap->header.flags & ObjectFlagContentsIsPointers))
                elementDestructor(oldNext->element);
            free(oldNext);
        }
    }
    free(hashMap->hashArray);
    if (hashMap->header.flags & ObjectFlagIsOnHeap)
        free(hashMap);
}