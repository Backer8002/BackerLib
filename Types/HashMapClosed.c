#include "HashMap.h"

#include "TypesMain.h"
#include "UnorderedContainer.h"

#include <assert.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct HashArrayNode {
    _Alignas(max_align_t) size_t next;
    Byte data[];
} HashArrayNode;

static bool internal_memCmpKey(size_t keySize, const void* key, const void* otherKey, bool isDataTypeFlagsQualified);

static bool internal_memCmpKey(size_t keySize, const void* key, const void* otherKey, bool isDataTypeFlagsQualified) {
    if (!otherKey || !key)
        return false;
    if (isDataTypeFlagsQualified) {
        if (*(DataTypeFlags*) key != *(DataTypeFlags*) otherKey)
            return false;
        if ((*(DataTypeFlags*) key & ObjectFlagIsContainer) && (*(DataTypeFlags*) key & ObjectFlagIsNotContinuous) == 0) {
            if (((Container*) key)->amountOfIndexes != ((Container*) otherKey)->amountOfIndexes || ((Container*) key)->byteSizeOfSingleElement != ((Container*) otherKey)->byteSizeOfSingleElement)
                return false;
            for (size_t i = 0; i < ((Container*) key)->amountOfIndexes; i++) {
                if (memcmp((Bytes) ((Container*) key)->array + ((Container*) key)->byteSizeOfSingleElement,
                           (Bytes) ((Container*) otherKey)->array + ((Container*) key)->byteSizeOfSingleElement,
                           ((Container*) key)->byteSizeOfSingleElement) != 0)
                    return false;
            }
            return true;
        }
    }
    if (memcmp(key, otherKey, keySize) != 0)
        return false;
    return true;
}

static inline void internal_hashMapInit(
    HashMap* hashMap,
    size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags,
    uint64_t (*hashFunction)(const void* element, size_t elementSize)) {

    if (!initialSize)
        initialSize = 1;
    if (hashFunction == NULL)
        hashMap->hashFunction = hashFunctionDefualtSingleVar;
    else
        hashMap->hashFunction = hashFunction;
    size_t sizeOfSingelLinkedListElement = sizeof(HashArrayNode) + keySize + ((keySize % alignof(max_align_t)) ? (alignof(max_align_t) - keySize % alignof(max_align_t)) : 0) + elementSize + ((elementSize % alignof(max_align_t)) ? (alignof(max_align_t) - elementSize % alignof(max_align_t)) : 0);
    hashMap->elementOffset               = sizeof(HashArrayNode) + keySize + ((keySize % alignof(max_align_t)) ? (alignof(max_align_t) - keySize % alignof(max_align_t)) : 0);
    hashMap->unorderedContainer          = unorderedContainerCreateStack(initialSize + 1, sizeOfSingelLinkedListElement, false); // 0 will be invalid index

    if (!isValidObject((DataTypeFlags*) hashMap))
        return;
    hashMap->hashArray = calloc(initialSize, sizeof(*hashMap->hashArray));
    if (hashMap->hashArray == NULL) {
        hashMap->header &= ~ObjectFlagIsValid;
        unorderedContainerDestroy(hashMap);
        return;
    }
    hashMap->lengthOfHashArray = initialSize;
    hashMap->keySize           = keySize;
    hashMap->header |= (keyIsDataTypeFlags ? FlagHashMapKeyIsDataTypeFlags : 0);
    hashMap->unorderedContainer.bitset[0] |= UINT64_MAX - (uint64_t) INT64_MAX;
    hashMap->container.amountOfIndexes++; // Use 0 as invalid index
}

HashMap hashMapCreateStack(size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags,
                           uint64_t (*hashFunction)(const void* element, size_t elementSize)) {
    HashMap hashMap;
    internal_hashMapInit(&hashMap, initialSize, keySize, elementSize, keyIsDataTypeFlags, hashFunction);
    return hashMap;
}

HashMap* hashMapCreateHeap(size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags,
                           uint64_t (*hashFunction)(const void* element, size_t elementSize)) {
    HashMap* hashMap = malloc(sizeof(*hashMap));
    if (hashMap == NULL)
        return NULL;
    internal_hashMapInit(hashMap, initialSize, keySize, elementSize, keyIsDataTypeFlags, hashFunction);
    if (!isValidObject((DataTypeFlags*) hashMap)) {
        free(hashMap);
        return NULL;
    }
    hashMap->header |= ObjectFlagIsOnHeap;
    return hashMap;
}

static uint64_t internal_hashMapHashSingelVar(size_t elementSize, const void* element, bool elementIsDataTypeFlags, uint64_t (*hashFunction)(const void* element, size_t elementSize)) {
    if (elementIsDataTypeFlags && (*(DataTypeFlags*) element & ObjectFlagIsContainer) && (*(DataTypeFlags*) element & ObjectFlagIsNotContinuous) == 0)
        return hashFunction(((Container*) element)->array, ((Container*) element)->byteSizeOfSingleElement * ((Container*) element)->amountOfIndexes);

    return hashFunction(element, elementSize);
}

static ContainerError internal_rehash(HashMap* hashMap, size_t newSize) {
    size_t* newHashArray = calloc(newSize, sizeof *newHashArray);
    if (newHashArray == NULL)
        return ContainerAllocFailure;

    for (size_t* nextBucket = hashMap->hashArray; nextBucket < hashMap->hashArray + hashMap->lengthOfHashArray; nextBucket++) {
        size_t next    = *nextBucket;
        size_t oldNext = 0;
        while (next) {
            oldNext                    = next;
            HashArrayNode* currentNode = unorderedContainerGet((UnorderedContainer*) hashMap, next).element;
            next                       = currentNode->next;
            uint64_t newHash           = internal_hashMapHashSingelVar(hashMap->keySize,
                                                                       &currentNode->data,
                                                             (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                                       hashMap->hashFunction) %
                               newSize;
            currentNode->next     = newHashArray[newHash];
            newHashArray[newHash] = oldNext;
        }
    }
    free(hashMap->hashArray);
    hashMap->hashArray         = newHashArray;
    hashMap->lengthOfHashArray = newSize;
    return ContainerOPSuccessful;
}

ContainerError hashMapInsert(HashMap* hashMap, size_t sizeOfKey, const void* key, size_t sizeOfElement, void* element) {
    if (sizeOfElement > hashMap->container.byteSizeOfSingleElement - hashMap->elementOffset || sizeOfKey != hashMap->keySize)
        return ContainerInvalidSize;
    uint64_t hash = internal_hashMapHashSingelVar(sizeOfKey,
                                                  key,
                                                  (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                  hashMap->hashFunction) %
                    hashMap->lengthOfHashArray;

    size_t  iterator = 1;
    size_t* hashNode = hashMap->hashArray + hash;

    while (*hashNode) {
        if (internal_memCmpKey(sizeOfKey,
                               key,
                               &((HashArrayNode*) unorderedContainerGet((UnorderedContainer*) hashMap, *hashNode).element)->data,
                               (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false))
            return ContainerOPUnsuccessful;

        hashNode = unorderedContainerGet((UnorderedContainer*) hashMap, *hashNode).element;
        iterator++;
    }

    HashArrayNode               node   = {.next = hashMap->hashArray[hash]};
    UnorderedContainerPutResult result = unorderedContainerPut((UnorderedContainer*) hashMap, sizeof(node), &node);
    if (result.resultCode != ContainerOPSuccessful)
        return ContainerAllocFailure;

    hashMap->hashArray[hash]       = result.locationOfElement;
    HashArrayNode* nodeInContainer = unorderedContainerGet((UnorderedContainer*) hashMap, result.locationOfElement).element;
    memcpy((Bytes) &nodeInContainer->data, key, sizeOfKey);
    if (element)
        memcpy((Bytes) nodeInContainer + hashMap->elementOffset, element, sizeOfElement);

    if ((float) hashMap->container.amountOfIndexes / (float) hashMap->lengthOfHashArray >= HASHMAP_MAX_LOADFACTOR || iterator >= HASHMAP_MAX_DEPTH)
        return internal_rehash(hashMap, hashMap->lengthOfHashArray * 2 + 1);
    return ContainerOPSuccessful;
}

static HashArrayNode* internal_hashArrayGetFromKey(const HashMap* hashMap, const void* key) {
    uint64_t hash = internal_hashMapHashSingelVar(hashMap->keySize,
                                                  key,
                                                  (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                  hashMap->hashFunction) %
                    hashMap->lengthOfHashArray;

    size_t hashNode = hashMap->hashArray[hash];
    while (hashNode) {
        HashArrayNode* nodeInContainer = unorderedContainerGet((UnorderedContainer*) hashMap, hashNode).element;
        if (internal_memCmpKey(hashMap->keySize,
                               key,
                               &nodeInContainer->data,
                               (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false))
            return nodeInContainer;
        hashNode = nodeInContainer->next;
    }
    return NULL;
}

void* hashMapGet(const HashMap* hashMap, size_t sizeOfKey, const void* key) {
    if (sizeOfKey != hashMap->keySize)
        return NULL;
    HashArrayNode* hashArrayNode = internal_hashArrayGetFromKey(hashMap, key);
    if (hashArrayNode == NULL)
        return NULL;
    return (Bytes) hashArrayNode + hashMap->elementOffset;
}

bool hashMapContainsKey(void* hashMap, size_t keySize, const void* key) {
    return hashMapGet(hashMap, keySize, key) ? true : false;
}

ContainerError hashMapRemove(HashMap* hashMap, size_t sizeOfKey, const void* key, void (*keyDestructor)(void* element), void (*elementDestructor)(void* element)) {
    if (sizeOfKey != hashMap->keySize)
        return ContainerInvalidSize;

    uint64_t       hash            = internal_hashMapHashSingelVar(sizeOfKey, key, (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false, hashMap->hashFunction) % hashMap->lengthOfHashArray;

    size_t         hashNode        = hashMap->hashArray[hash];
    HashArrayNode* currentHashNode = NULL;
    size_t         prevHashNode    = 0;
    while (hashNode) {
        currentHashNode = unorderedContainerGet((UnorderedContainer*) hashMap, hashNode).element;
        if (!currentHashNode)
            return ContainerOPUnsuccessful;
        if (internal_memCmpKey(sizeOfKey, key, currentHashNode->data, (hashMap->header & FlagHashMapKeyIsDataTypeFlags) ? true : false))
            goto HashMapRemoveHandleDestruct;
        prevHashNode = hashNode;
        hashNode     = currentHashNode->next;
    }

    return ContainerOPUnsuccessful;

HashMapRemoveHandleDestruct:
    if (keyDestructor)
        keyDestructor(&currentHashNode->data);

    if (elementDestructor)
        elementDestructor((Bytes) currentHashNode + hashMap->elementOffset);
    if (!prevHashNode)
        hashMap->hashArray[hash] = currentHashNode->next;
    else
        ((HashArrayNode*) unorderedContainerGet((UnorderedContainer*) hashMap, prevHashNode).element)->next = currentHashNode->next;
    unorderedContainerRemove((UnorderedContainer*) hashMap, hashNode, NULL);
    return ContainerOPSuccessful;
}

ContainerError hashMapReplace(HashMap* hashMap, size_t sizeOfKey, const void* key, size_t sizeOfElement, void* element, void (*elementDestructor)(void* element)) {
    if (sizeOfKey != hashMap->keySize || sizeOfElement > hashMap->container.byteSizeOfSingleElement - hashMap->elementOffset)
        return ContainerInvalidSize;

    HashArrayNode* hashNode = internal_hashArrayGetFromKey(hashMap, key);
    if (hashNode == NULL)
        return ContainerOPUnsuccessful;

    if (elementDestructor)
        elementDestructor((Bytes) hashNode + hashMap->elementOffset);
    memcpy((Bytes) hashNode + hashMap->elementOffset, element, sizeOfElement);
    return ContainerOPSuccessful;
}

void hashMapDestroy(HashMap* hashMap, void (*keyDestructor)(void* key), void (*elementDestructor)(void* element)) {
    if (!(DataTypeFlags*) hashMap)
        return;
    for (size_t i = 1; i < hashMap->container.amountOfIndexes; i++) {
        if (unorderedContainerGet((UnorderedContainer*) hashMap, i).resultCode == ContainerOPUnsuccessful)
            continue;
        if (keyDestructor)
            keyDestructor(&((HashArrayNode*) ((Bytes) hashMap->container.array + hashMap->container.byteSizeOfSingleElement * i))->data);
        if (elementDestructor)
            elementDestructor((Bytes) hashMap->container.array + hashMap->container.byteSizeOfSingleElement * i + hashMap->elementOffset);
    }
    free(hashMap->hashArray);
    unorderedContainerDestroy(hashMap);
}
