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

static inline void internal_hashMapInit(
    BL_Hashmap* hashMap,
    size_t initialSize, size_t keySize, size_t elementSize, bool (*compare)(const void*, const void*),
    uint64_t (*hashFunction)(const void* element)) {

    hashMap->unorderedContainer.header = 0;

    if (!compare || !hashFunction)
        return;
    if (!initialSize)
        initialSize = 1;
    hashMap->hashFunction                = hashFunction;
    hashMap->compare                     = compare;
    size_t sizeOfSingelLinkedListElement = sizeof(HashArrayNode) + keySize + ((keySize % alignof(max_align_t)) ? (alignof(max_align_t) - keySize % alignof(max_align_t)) : 0) + elementSize + ((elementSize % alignof(max_align_t)) ? (alignof(max_align_t) - elementSize % alignof(max_align_t)) : 0);
    hashMap->elementOffset               = sizeof(HashArrayNode) + keySize + ((keySize % alignof(max_align_t)) ? (alignof(max_align_t) - keySize % alignof(max_align_t)) : 0);
    hashMap->unorderedContainer          = bl_unordered_container_create_stack(initialSize + 1, sizeOfSingelLinkedListElement); // 0 will be invalid index

    if (!bl_unordered_container_is_valid(&hashMap->unorderedContainer))
        return;
    hashMap->hashArray = calloc(initialSize, sizeof(*hashMap->hashArray));
    if (hashMap->hashArray == NULL) {
        hashMap->unorderedContainer.header &= ~ObjectFlagIsValid;
        bl_unordered_container_destroy(hashMap);
        return;
    }
    hashMap->lengthOfHashArray = initialSize;
    hashMap->keySize           = keySize;
    hashMap->unorderedContainer.bitset[0] |= UINT64_MAX - (uint64_t) INT64_MAX;
    hashMap->unorderedContainer.amountOfElements++; // Use 0 as invalid index
}

BL_Hashmap bl_hashmap_create_stack(size_t initialSize, size_t keySize, size_t elementSize, bool (*compare)(const void* first, const void* second),
                                   uint64_t (*hashFunction)(const void* element)) {
    BL_Hashmap hashMap;
    internal_hashMapInit(&hashMap, initialSize, keySize, elementSize, compare, hashFunction);
    return hashMap;
}

BL_Hashmap* bl_hashmap_create_heap(size_t initialSize, size_t keySize, size_t elementSize, bool (*compare)(const void* first, const void* second),
                                   uint64_t (*hashFunction)(const void* element)) {
    BL_Hashmap* hashMap = malloc(sizeof(*hashMap));
    if (hashMap == NULL)
        return NULL;
    internal_hashMapInit(hashMap, initialSize, keySize, elementSize, compare, hashFunction);
    if (!bl_hashmap_is_valid(hashMap)) {
        free(hashMap);
        return NULL;
    }
    hashMap->unorderedContainer.header |= ObjectFlagIsOnHeap;
    return hashMap;
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
            uint64_t newHash           = hashMap->hashFunction(&currentNode->data) % newSize;
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
    if (sizeOfElement > hashMap->unorderedContainer.byteSizeOfElement - hashMap->elementOffset || sizeOfKey != hashMap->keySize)
        return BL_ContainerInvalidSize;
    uint64_t hash = hashMap->hashFunction(key) % hashMap->lengthOfHashArray;

    size_t  iterator = 1;
    size_t* hashNode = hashMap->hashArray + hash;

    while (*hashNode) {
        if (hashMap->compare(key,&((HashArrayNode*) bl_unordered_container_get(&hashMap->unorderedContainer, *hashNode))->data))
            return BL_ContainerOPUnsuccessful;

        hashNode = bl_unordered_container_get(&hashMap->unorderedContainer, *hashNode);
        iterator++;
    }

    HashArrayNode  node        = {.next = hashMap->hashArray[hash]};
    HashArrayNode* objectPlace = bl_unordered_container_put(&hashMap->unorderedContainer, sizeof(node), &node);
    if (!objectPlace)
        return BL_ContainerAllocFailure;
    size_t objectIndex       = bl_unordered_container_index_from_ref(&hashMap->unorderedContainer, objectPlace);
    hashMap->hashArray[hash] = objectIndex;
    memcpy((BL_Bytes) &objectPlace->data, key, sizeOfKey);
    if (element)
        memcpy((BL_Bytes) objectPlace + hashMap->elementOffset, element, sizeOfElement);

    if ((float) bl_unordered_container_size(&hashMap->unorderedContainer) / (float) hashMap->lengthOfHashArray >= HASHMAP_MAX_LOADFACTOR || iterator >= HASHMAP_MAX_DEPTH)
        return internal_rehash(hashMap, hashMap->lengthOfHashArray * 2 + 1);
    return BL_ContainerOPSuccessful;
}

static HashArrayNode* internal_hashArrayGetFromKey(const BL_Hashmap* hashMap, const void* key) {
    uint64_t hash = hashMap->hashFunction(key) % hashMap->lengthOfHashArray;

    size_t hashNode = hashMap->hashArray[hash];
    while (hashNode) {
        HashArrayNode* nodeInContainer = bl_unordered_container_get(&hashMap->unorderedContainer, hashNode);
        if (hashMap->compare(&nodeInContainer->data,key))
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

    uint64_t       hash            = hashMap->hashFunction(key) % hashMap->lengthOfHashArray;

    size_t         hashNode        = hashMap->hashArray[hash];
    HashArrayNode* currentHashNode = NULL;
    size_t         prevHashNode    = 0;
    while (hashNode) {
        currentHashNode = bl_unordered_container_get(&hashMap->unorderedContainer, hashNode);
        if (!currentHashNode)
            return BL_ContainerOPUnsuccessful;
        if (hashMap->compare(key, &currentHashNode->data))
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
    if (sizeOfKey != hashMap->keySize || sizeOfElement > hashMap->unorderedContainer.byteSizeOfElement - hashMap->elementOffset)
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
        HashArrayNode* element = bl_unordered_container_get((BL_UnorderedContainer*) hashMap, i);
        if (!element)
            continue;
        if (keyDestructor)
            keyDestructor(element->data);
        if (elementDestructor)
            elementDestructor((BL_Bytes) element + hashMap->elementOffset);
    }
    free(hashMap->hashArray);
    bl_unordered_container_destroy(hashMap);
}

bool bl_hashmap_is_valid(const BL_Hashmap* hashmap) {
    return bl_unordered_container_is_valid(&hashmap->unorderedContainer);
}