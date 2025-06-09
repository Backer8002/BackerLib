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
    uint64_t(*hashFunction)(const void* element, size_t elementSize)
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
    uint64_t (*hashFunction)(const void* element, size_t elementSize)
) {
    HashMap newHashMap;
    newHashMap.header.dataArrayVarType = ListNone;
    m_hashMapCreateInit(&newHashMap,intialSize,keySize,elementSize,keyType,elementType,elementsArePointers,hashFunction);
    return newHashMap;
}

HashMap* hashMapCreate(
    size_t intialSize, size_t keySize, size_t elementSize,
    ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers, 
    uint64_t (*hashFunction)(const void* element, size_t elementSize)
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
    size_t intialSize, size_t keySize,
    ListTypes_t keyType,
    uint64_t (*hashFunction)(const void* element, size_t elementSize)
) {
    Set newSet = hashMapCreateStack(intialSize,keySize,keySize,keyType,keyType,false,hashFunction);
    newSet.header.objectType = ListSet;
    return newSet;
}

inline Set* setCreate(
    size_t intialSize, size_t keySize,
    ListTypes_t keyType,
    uint64_t (*hashFunction)(const void* element, size_t elementSize)
) {
    Set* newSet = hashMapCreate(intialSize,keySize,keySize, keyType, keyType, false, hashFunction);
    if (newSet != NULL)
        newSet->header.objectType = ListSet;
    return newSet;
}

static bool m_memCmpKey(ListTypes_t keyType, const void* key, size_t keySize, const void* hashArrayElement) {
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

HashMapError_t hashMapInsert(HashMap* hashMap, void* key, ListTypes_t keyType, void* element, ListTypes_t elementType,size_t reHashingIteration) {
    assert(keyType == hashMap->keyType);

    uint64_t       hash     = hashFunctionDefualtSingleVar(key, hashMap->sizeOfKey) % hashMap->sizeOfHashArray;
    size_t         iterator = 1;
    HashArrayNode* hashNode = hashMap->hashArray + hash;
    void*          keyToAssign      = malloc(hashMap->sizeOfKey);
    if (keyToAssign == NULL)
        return HashMapCannotAllocMem;
    memcpy(keyToAssign, key, hashMap->sizeOfKey);
    void* elementToAssign = NULL;
    if (!(hashMap->header.flags & ObjectFlagContentsIsPointers) && hashMap->header.objectType != ListSet) {
        elementToAssign = malloc(hashMap->sizeOfElement);
        if (elementToAssign == NULL) {
            free(keyToAssign);
            return HashMapCannotAllocMem;
        }
        memcpy(elementToAssign, element, hashMap->sizeOfElement);
    }

    if (hashNode->key != NULL) {
        while (hashNode->next != NULL) {
            if (m_memCmpKey(keyType, key, hashMap->sizeOfKey, hashNode->key)) {
                free(keyToAssign);
                if (elementToAssign != NULL)
                    free(elementToAssign);
                return HashMapKeyAlreadyExists;
            }
            hashNode = hashNode->next;
            iterator++;
        }
        hashNode->next = malloc(sizeof(HashArrayNode));
        if (hashNode->next == NULL) {
            free(keyToAssign);
            if (elementToAssign != NULL)
                free(elementToAssign);
            return HashMapCannotAllocMem;
        }
        hashNode = hashNode->next;
    }
    
    hashNode->next == NULL;
    hashNode->key = keyToAssign;
    if (elementToAssign == NULL)
        hashNode->element = element;
    else
        hashNode->element = elementToAssign;


    return HashMapOperationSuccsess;
}

inline HashMapError_t setInsert(HashMap* hashMap, void* key, ListTypes_t keyType, size_t reHashingIteration) { hashMapInsert(hashMap, key, keyType, NULL, keyType, 0); }

static HashArrayNode* m_hashArrayGetFromKey(const HashMap* hashMap, const void* key, ListTypes_t keyType) {
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

HashMapError_t hashMapRemove(HashMap* hashMap, const void* key, ListTypes_t keyType, void (*keyDestructor)(void* element), void (*elementDestructor)(void* element)) {
    assert(keyType == hashMap->keyType);

    uint64_t       hash     = hashFunctionDefualtSingleVar(key, hashMap->sizeOfKey) % hashMap->sizeOfHashArray;

    HashArrayNode* hashNode = hashMap->hashArray + hash;
    if (hashNode->key == NULL)
        return HashMapKeyDoesNotExist;
    HashArrayNode* prevHashNode = hashNode;
    while (!m_memCmpKey(keyType, key, hashMap->sizeOfKey, hashNode->key)) {
        if (hashNode->next == NULL)
            return HashMapKeyDoesNotExist;

        prevHashNode = hashNode;
        hashNode = hashNode->next;
    }

    if (keyDestructor != NULL)
        keyDestructor(hashNode->key);

    if (elementDestructor != NULL)
        elementDestructor(hashNode->element);

    if (prevHashNode == hashNode) {
        if (hashNode->next == NULL) {
            hashNode->key = NULL;
            return HashMapOperationSuccsess;
        }
        hashNode = hashNode->next;
        *prevHashNode = *hashNode;
        free(hashNode);
        return HashMapOperationSuccsess;
    }

    if (hashNode->next != NULL)
        prevHashNode->next = hashNode->next;
    else
        prevHashNode->next == NULL;

    free(hashNode);
    return HashMapOperationSuccsess;
}

inline HashMapError_t setRemove(HashMap* hashMap, void* key, ListTypes_t keyType, void (*keyDestructor)(void* element)) { return hashMapRemove(hashMap, key, keyType,keyDestructor,NULL); }

HashMapError_t hashMapReplace(HashMap* hashMap, const void* key, ListTypes_t keyType, void* element, ListTypes_t elementType, void (*destrutorOfPreviousElement)(void* element)) {
    assert(keyType == hashMap->keyType);
    assert(elementType == hashMap->header.dataArrayVarType);
    assert(hashMap->header.objectType == ListHashMap);

    HashArrayNode* hashArrayNode = m_hashArrayGetFromKey(hashMap, key, keyType);
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
    memcpy(hashArrayNode->element, element, hashMap->sizeOfElement);
    if (!typeIsPrimitive(elementType))
        ((DataTypeHeader*) hashArrayNode->element)->flags |= ObjectFlagIsOnHeap;
    return HashMapOperationSuccsess;
}
