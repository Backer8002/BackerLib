#define Implementation

#include"pch.h"
#include<hashMap.h>
#include<assert.h>
#include<backerLibListTypes.h>
#include<stdint.h>
#include<stdlib.h>
#include<backerStrings.h>
#include<string.h>
#include<stdarg.h>


uint64_t hashFunctionDefualt(size_t amountOfVars,...) {
    if (amountOfVars == 0)
        return 0;
    va_list argsToHash;
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


inline void m_hashMapCreateInit(
    HashMap* newHashMap,
    size_t intialSize, size_t keySize, size_t elementSize,
    ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers,
    uint64_t(*hashFunction)(void* element, size_t elementSize)
) {
    if (hashFunction == NULL)
        newHashMap->hashFunction = hashFunctionDefualtSingleVar;
    else
        newHashMap->hashFunction = hashFunction;
    newHashMap->sizeOfHashArray = intialSize;
    newHashMap->hashArray       = calloc(intialSize, sizeof(HashArrayNode));
    if (newHashMap->hashArray == NULL) {
        newHashMap->header.dataArrayVarType = ListNone;
        return;
    }
    newHashMap->sizeOfKey               = keySize;
    newHashMap->sizeOfElement           = elementSize;
    newHashMap->keyType                 = keyType;
    newHashMap->header.dataArrayVarType = elementType;
    newHashMap->header.flags            = ((elementsArePointers) ? ObjectFlagContentsIsPointers : 0);
    newHashMap->header.objectType       = ListHashMap;
}

HashMap hashMapCreateStack(
    size_t intialSize, size_t keySize, size_t elementSize,
    ListTypes_t keyType ,ListTypes_t elementType, bool elementsArePointers, 
    uint64_t (*hashFunction)(void* element, size_t elementSize)
) {
    HashMap newHashMap         = {.header.dataArrayVarType = ListNone};
    m_hashMapCreateInit(&newHashMap,intialSize,keySize,elementSize,keyType,elementType,elementsArePointers,hashFunction);
    return newHashMap;
}

HashMap* hashMapCreate(
    size_t intialSize, size_t keySize, size_t elementSize,
    ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers, 
    uint64_t (*hashFunction)(void* element, size_t elementSize)
) {
    HashMap* newHashMap = malloc(sizeof(HashMap));
    if (newHashMap == NULL)
        return NULL;
    m_hashMapCreateInit(newHashMap, intialSize,keySize,elementSize, keyType, elementType, elementsArePointers, hashFunction);
    if (newHashMap->header.dataArrayVarType == ListNone) {
        free(newHashMap);
        return NULL;
    }
    newHashMap->header.flags |= ObjectFlagIsOnHeap;
    return newHashMap;
}

inline Set setCreateStack(
    size_t intialSize, size_t keySize, size_t elementSize,
    ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers,
    uint64_t (*hashFunction)(void* element, size_t elementSize)
) {
    Set newSet = hashMapCreateStack(intialSize,keySize,elementSize,keyType,elementType,elementsArePointers,hashFunction);
    newSet.header.objectType = ListSet;
    return newSet;
}

inline Set* setCreate(
    size_t intialSize, size_t keySize, size_t elementSize,
    ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers,
    uint64_t (*hashFunction)(void* element, size_t elementSize)
) {
    Set* newSet = hashMapCreate(intialSize,keySize,elementSize, keyType, elementType, elementsArePointers, hashFunction);
    if (newSet != NULL)
        newSet->header.objectType = ListSet;
    return newSet;
}

bool m_memCmpKey(ListTypes_t keyType, const void* key, size_t keySize, const void* hashArrayElement) {
    if (typeIsPrimitive(keyType))
        return (memcmp(key, hashArrayElement, keySize) == 0) ? true : false;

    switch (keyType) {
    case ListString:
        if (((String*) key)->amountOfElements != ((String*)hashArrayElement)->amountOfElements)
            return false;
        return (memcmp(((String*) key)->list, ((String*) hashArrayElement)->list, ((String*) key)->amountOfElements) == 0) ? true : false;
        break;
    default: return false; break;
    }
}

HashMapError_t hashMapInsert(HashMap* hashMap, void* key, size_t keySize, void* element, size_t elementSize) {

}

HashArrayNode* m_hashArrayGetFromKey(const HashMap* hashMap, const void* key, ListTypes_t keyType) {
    assert(keyType == hashMap->keyType);

    uint64_t       hash     = hashFunctionDefualtSingleVar(key, hashMap->sizeOfKey) % hashMap->sizeOfHashArray;

    HashArrayNode* hashNode = hashMap->hashArray + hash;
    if (hashNode->key == NULL)
        return NULL;
    while (true) {
        if (m_memCmpKey(keyType, key, hashMap->sizeOfKey, hashNode->key)) {
            return hashNode;
        }

        if (hashNode->next == NULL)
            return NULL;

        hashNode = hashNode->next;
    }
}

HashMapError_t hashMapGet(const HashMap* hashMap, const void* key, ListTypes_t keyType,void** element) {
    assert(keyType == hashMap->keyType);
    assert(hashMap->header.objectType == ListHashMap);


    HashArrayNode* hashArrayNode = m_hashArrayGetFromKey(hashMap, key, keyType);
    if (hashArrayNode == NULL)
        return HashMapKeyDoesNotExist;

    *element = hashArrayNode->element;
    return HashMapOperationSuccsess;
}

inline bool    setGet(const HashMap* hashMap, const void* key, ListTypes_t keyType) { 
    assert(keyType == hashMap->keyType);
    assert(hashMap->header.objectType == ListSet);
    return (m_hashArrayGetFromKey != NULL) ? true : false; 
}

HashMapError_t hashMapRemove(HashMap* hashMap, void* key, ListTypes_t keyType, void (*destructor)(void* element)) {

}

inline HashMapError_t setRemove(HashMap* hashMap, void* key, ListTypes_t keyType) { return hashMapRemove(hashMap, key, keyType,NULL); }

HashMapError_t hashMapReplace(HashMap* hashMap, const void* key, ListTypes_t keyType, const void* element, ListTypes_t elementType) {
    assert(keyType == hashMap->keyType);
    assert(elementType == hashMap->header.dataArrayVarType);
    assert(hashMap->header.objectType == ListHashMap);

    HashArrayNode* hashArrayNode = m_hashArrayGetFromKey(hashMap, key, keyType);
    if (hashArrayNode == NULL)
        return HashMapKeyDoesNotExist;

    if (hashMap->header.flags & ObjectFlagContentsIsPointers)
        hashArrayNode->element = element;
    else
        memcpy_s(hashArrayNode->element, hashMap->sizeOfElement, element, hashMap->sizeOfElement);
    return HashMapOperationSuccsess;
}
