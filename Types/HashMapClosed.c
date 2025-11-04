#include "HashMap.h"

#include "BL_UnorderedContainer.h"
#include "TypesMain.h"

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
    BL_Byte data[];
} HashArrayNode;

static bool internal_memcmp_key(size_t keySize, const void* key, const void* otherKey, bool isDataTypeFlagsQualified);

static bool internal_memcmp_key(size_t keySize, const void* key, const void* otherKey, bool isDataTypeFlagsQualified) {
    if (!otherKey || !key)
        return false;
    if (isDataTypeFlagsQualified) {
        if (*(BL_DataTypeFlags*) key != *(BL_DataTypeFlags*) otherKey)
            return false;
        if ((*(BL_DataTypeFlags*) key & ObjectFlagIsContainer) && (*(BL_DataTypeFlags*) key & ObjectFlagIsNotContinuous) == 0) {
            if (((BL_Container*) key)->amountOfIndexes != ((BL_Container*) otherKey)->amountOfIndexes || ((BL_Container*) key)->byteSizeOfSingleElement != ((BL_Container*) otherKey)->byteSizeOfSingleElement)
                return false;
            for (size_t i = 0; i < ((BL_Container*) key)->amountOfIndexes; i++) {
                if (memcmp((BL_Bytes) ((BL_Container*) key)->array + ((BL_Container*) key)->byteSizeOfSingleElement,
                           (BL_Bytes) ((BL_Container*) otherKey)->array + ((BL_Container*) key)->byteSizeOfSingleElement,
                           ((BL_Container*) key)->byteSizeOfSingleElement) != 0)
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
    BL_Hashmap* hashMap,
    size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags,
    uint64_t (*hashFunction)(const void* element, size_t elementSize)) {

    if (!initialSize)
        initialSize = 1;
    if (hashFunction == NULL)
        hashMap->hashFunction = bl_hashfunction_defualt_single_var;
    else
        hashMap->hashFunction = hashFunction;
    size_t sizeOfSingelLinkedListElement = sizeof(HashArrayNode) + keySize + ((keySize % alignof(max_align_t)) ? (alignof(max_align_t) - keySize % alignof(max_align_t)) : 0) + elementSize + ((elementSize % alignof(max_align_t)) ? (alignof(max_align_t) - elementSize % alignof(max_align_t)) : 0);
    hashMap->elementOffset               = sizeof(HashArrayNode) + keySize + ((keySize % alignof(max_align_t)) ? (alignof(max_align_t) - keySize % alignof(max_align_t)) : 0);
    hashMap->unorderedContainer          = bl_unordered_container_create_stack(initialSize + 1, sizeOfSingelLinkedListElement); // 0 will be invalid index

    if (!bl_unordered_container_is_valid(&hashMap->unorderedContainer))
        return;
    hashMap->hashArray = calloc(initialSize, sizeof(*hashMap->hashArray));
    if (hashMap->hashArray == NULL) {
        hashMap->unorderedContainer.container.header &= ~ObjectFlagIsValid;
        bl_unordered_container_destroy(hashMap);
        return;
    }
    hashMap->lengthOfHashArray = initialSize;
    hashMap->keySize           = keySize;
    hashMap->unorderedContainer.container.header |= (keyIsDataTypeFlags ? FlagHashMapKeyIsDataTypeFlags : 0);
    hashMap->unorderedContainer.bitset[0] |= UINT64_MAX - (uint64_t) INT64_MAX;
    hashMap->unorderedContainer.container.amountOfIndexes++; // Use 0 as invalid index
}

BL_Hashmap bl_hashmap_create_stack(size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags,
                           uint64_t (*hashFunction)(const void* element, size_t elementSize)) {
    BL_Hashmap hashMap;
    internal_hashMapInit(&hashMap, initialSize, keySize, elementSize, keyIsDataTypeFlags, hashFunction);
    return hashMap;
}

BL_Hashmap* bl_hashmap_create_heap(size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags,
                           uint64_t (*hashFunction)(const void* element, size_t elementSize)) {
    BL_Hashmap* hashMap = malloc(sizeof(*hashMap));
    if (hashMap == NULL)
        return NULL;
    internal_hashMapInit(hashMap, initialSize, keySize, elementSize, keyIsDataTypeFlags, hashFunction);
    if (!bl_hashmap_is_valid(hashMap)) {
        free(hashMap);
        return NULL;
    }
    hashMap->unorderedContainer.container.header |= ObjectFlagIsOnHeap;
    return hashMap;
}

static uint64_t internal_hashmap_hash_singel_var(size_t elementSize, const void* element, bool elementIsDataTypeFlags, uint64_t (*hashFunction)(const void* element, size_t elementSize)) {
    if (elementIsDataTypeFlags && (*(BL_DataTypeFlags*) element & ObjectFlagIsContainer) && (*(BL_DataTypeFlags*) element & ObjectFlagIsNotContinuous) == 0)
        return hashFunction(((BL_Container*) element)->array, ((BL_Container*) element)->byteSizeOfSingleElement * ((BL_Container*) element)->amountOfIndexes);

    return hashFunction(element, elementSize);
}

static BL_ContainerError internal_rehash(BL_Hashmap* hashMap, size_t newSize) {
    size_t* newHashArray = calloc(newSize, sizeof *newHashArray);
    if (newHashArray == NULL)
        return BL_ContainerAllocFailure;

    for (size_t* nextBucket = hashMap->hashArray; nextBucket < hashMap->hashArray + hashMap->lengthOfHashArray; nextBucket++) {
        size_t next    = *nextBucket;
        size_t oldNext = 0;
        while (next) {
            oldNext                    = next;
            HashArrayNode* currentNode = bl_unordered_container_get(&hashMap->unorderedContainer, next);
            next                       = currentNode->next;
            uint64_t newHash           = internal_hashmap_hash_singel_var(hashMap->keySize,
                                                                       &currentNode->data,
                                                             (hashMap->unorderedContainer.container.header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                                       hashMap->hashFunction) %
                               newSize;
            currentNode->next     = newHashArray[newHash];
            newHashArray[newHash] = oldNext;
        }
    }
    free(hashMap->hashArray);
    hashMap->hashArray         = newHashArray;
    hashMap->lengthOfHashArray = newSize;
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_hashmap_insert(BL_Hashmap* hashMap, size_t sizeOfKey, const void* key, size_t sizeOfElement, const void* element) {
    if (sizeOfElement > hashMap->unorderedContainer.container.byteSizeOfSingleElement - hashMap->elementOffset || sizeOfKey != hashMap->keySize)
        return BL_ContainerInvalidSize;
    uint64_t hash = internal_hashmap_hash_singel_var(sizeOfKey,
                                                  key,
                                                  (hashMap->unorderedContainer.container.header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                  hashMap->hashFunction) %
                    hashMap->lengthOfHashArray;

    size_t  iterator = 1;
    size_t* hashNode = hashMap->hashArray + hash;

    while (*hashNode) {
        if (internal_memcmp_key(sizeOfKey,
                               key,
                               &((HashArrayNode*) bl_unordered_container_get(&hashMap->unorderedContainer, *hashNode))->data,
                               (hashMap->unorderedContainer.container.header & FlagHashMapKeyIsDataTypeFlags) ? true : false))
            return BL_ContainerOPUnsuccessful;

        hashNode = bl_unordered_container_get(&hashMap->unorderedContainer, *hashNode);
        iterator++;
    }

    HashArrayNode               node   = {.next = hashMap->hashArray[hash]};
    BL_UnorderedContainerPutResult result = bl_unordered_container_put(&hashMap->unorderedContainer, sizeof(node), &node);
    if (result.resultCode != BL_ContainerOPSuccessful)
        return BL_ContainerAllocFailure;

    hashMap->hashArray[hash]       = result.locationOfElement;
    HashArrayNode* nodeInContainer = bl_unordered_container_get(&hashMap->unorderedContainer, result.locationOfElement);
    memcpy((BL_Bytes) &nodeInContainer->data, key, sizeOfKey);
    if (element)
        memcpy((BL_Bytes) nodeInContainer + hashMap->elementOffset, element, sizeOfElement);

    if ((float) bl_unordered_container_size(&hashMap->unorderedContainer)/ (float) hashMap->lengthOfHashArray >= HASHMAP_MAX_LOADFACTOR || iterator >= HASHMAP_MAX_DEPTH)
        return internal_rehash(hashMap, hashMap->lengthOfHashArray * 2 + 1);
    return BL_ContainerOPSuccessful;
}

static HashArrayNode* internal_hashArrayGetFromKey(const BL_Hashmap* hashMap, const void* key) {
    uint64_t hash = internal_hashmap_hash_singel_var(hashMap->keySize,
                                                  key,
                                                  (hashMap->unorderedContainer.container.header & FlagHashMapKeyIsDataTypeFlags) ? true : false,
                                                  hashMap->hashFunction) %
                    hashMap->lengthOfHashArray;

    size_t hashNode = hashMap->hashArray[hash];
    while (hashNode) {
        HashArrayNode* nodeInContainer = bl_unordered_container_get(&hashMap->unorderedContainer, hashNode);
        if (internal_memcmp_key(hashMap->keySize,
                               key,
                               &nodeInContainer->data,
                               (hashMap->unorderedContainer.container.header & FlagHashMapKeyIsDataTypeFlags) ? true : false))
            return nodeInContainer;
        hashNode = nodeInContainer->next;
    }
    return NULL;
}

void* bl_hashmap_get(const BL_Hashmap* hashMap, size_t sizeOfKey, const void* key) {
    if (sizeOfKey != hashMap->keySize)
        return NULL;
    HashArrayNode* hashArrayNode = internal_hashArrayGetFromKey(hashMap, key);
    if (hashArrayNode == NULL)
        return NULL;
    return (BL_Bytes) hashArrayNode + hashMap->elementOffset;
}

bool bl_hashmap_contains_key(const BL_Hashmap* hashMap, size_t keySize, const void* key) {
    return bl_hashmap_get(hashMap, keySize, key) ? true : false;
}

BL_ContainerError bl_hashmap_remove(BL_Hashmap* hashMap, size_t sizeOfKey, const void* key, void (*keyDestructor)(void* element), void (*elementDestructor)(void* element)) {
    if (sizeOfKey != hashMap->keySize)
        return BL_ContainerInvalidSize;

    uint64_t       hash            = internal_hashmap_hash_singel_var(sizeOfKey, key, (hashMap->unorderedContainer.container.header & FlagHashMapKeyIsDataTypeFlags) ? true : false, hashMap->hashFunction) % hashMap->lengthOfHashArray;

    size_t         hashNode        = hashMap->hashArray[hash];
    HashArrayNode* currentHashNode = NULL;
    size_t         prevHashNode    = 0;
    while (hashNode) {
        currentHashNode = bl_unordered_container_get(&hashMap->unorderedContainer, hashNode);
        if (!currentHashNode)
            return BL_ContainerOPUnsuccessful;
        if (internal_memcmp_key(sizeOfKey, key, currentHashNode->data, (hashMap->unorderedContainer.container.header & FlagHashMapKeyIsDataTypeFlags) ? true : false))
            goto HashMapRemoveHandleDestruct;
        prevHashNode = hashNode;
        hashNode     = currentHashNode->next;
    }

    return BL_ContainerOPUnsuccessful;

HashMapRemoveHandleDestruct:
    if (keyDestructor)
        keyDestructor(&currentHashNode->data);

    if (elementDestructor)
        elementDestructor((BL_Bytes) currentHashNode + hashMap->elementOffset);
    if (!prevHashNode)
        hashMap->hashArray[hash] = currentHashNode->next;
    else
        ((HashArrayNode*) bl_unordered_container_get(&hashMap->unorderedContainer, prevHashNode))->next = currentHashNode->next;
    bl_unordered_container_remove(&hashMap->unorderedContainer, hashNode, NULL);
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_hashmap_replace(BL_Hashmap* hashMap, size_t sizeOfKey, const void* key, size_t sizeOfElement, const void* element, void (*elementDestructor)(void* element)) {
    if (sizeOfKey != hashMap->keySize || sizeOfElement > hashMap->unorderedContainer.container.byteSizeOfSingleElement - hashMap->elementOffset)
        return BL_ContainerInvalidSize;

    HashArrayNode* hashNode = internal_hashArrayGetFromKey(hashMap, key);
    if (hashNode == NULL)
        return BL_ContainerOPUnsuccessful;

    if (elementDestructor)
        elementDestructor((BL_Bytes) hashNode + hashMap->elementOffset);
    memcpy((BL_Bytes) hashNode + hashMap->elementOffset, element, sizeOfElement);
    return BL_ContainerOPSuccessful;
}

void bl_hashmap_destroy(BL_Hashmap* hashMap, void (*keyDestructor)(void* key), void (*elementDestructor)(void* element)) {
    if (!(BL_DataTypeFlags*) hashMap)
        return;
    for (size_t i = 1; i < bl_unordered_container_size(&hashMap->unorderedContainer); i++) {
        if (bl_unordered_container_get((BL_UnorderedContainer*) hashMap, i) == NULL)
            continue;
        if (keyDestructor)
            keyDestructor(&((HashArrayNode*) ((BL_Bytes) hashMap->unorderedContainer.container.array + hashMap->unorderedContainer.container.byteSizeOfSingleElement * i))->data);
        if (elementDestructor)
            elementDestructor((BL_Bytes) hashMap->unorderedContainer.container.array + hashMap->unorderedContainer.container.byteSizeOfSingleElement * i + hashMap->elementOffset);
    }
    free(hashMap->hashArray);
    bl_unordered_container_destroy(hashMap);
}

bool bl_hashmap_is_valid(const BL_Hashmap* hashmap) {
    return bl_unordered_container_is_valid(&hashmap->unorderedContainer);
}