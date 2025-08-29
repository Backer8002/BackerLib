#include "HashMap_internal.h"
#include "HashMap.h"

inline ContainerError hashMapInsert(void* hashMap, size_t keySize, const void* key, size_t elementSize, void* element) {
    if (*(DataTypeFlags*)hashMap & FlagHashMapUsesCuckoo)
        return hashMapCuckooInsert(hashMap,keySize,key,elementSize,element);
    return hashMapClosedInsert(hashMap,keySize,key,elementSize,element);
}

inline void* hashMapGet(void* hashMap, size_t keySize, const void* key) {
    if (*(DataTypeFlags*)hashMap & FlagHashMapUsesCuckoo)
        return hashMapCuckooGet(hashMap,keySize,key);
    return hashMapClosedGet(hashMap,keySize,key);
}

inline bool hashMapContainsKey(void* hashMap, size_t keySize, const void* key) {
    return hashMapGet(hashMap,keySize,key) ? true : false;
}

inline ContainerError hashMapRemove(void* hashMap, size_t keySize, const void* key, void(*keyDestructor)(void* object),void(*elementDestructor)(void* object)) {
    if (*(DataTypeFlags*)hashMap & FlagHashMapUsesCuckoo)
        return hashMapCuckooRemove(hashMap,keySize,key,keyDestructor,elementDestructor);
    return hashMapClosedRemove(hashMap,keySize,key,keyDestructor,elementDestructor);
}

inline ContainerError hashMapReplace(void* hashMap, size_t keySize, const void* key, size_t elementSize, void* element, void(*elementDestructor)(void* object)) {
    if (*(DataTypeFlags*)hashMap & FlagHashMapUsesCuckoo)
        return hashMapCuckooReplace(hashMap,keySize,key,elementSize,element,elementDestructor);
    return hashMapReplace(hashMap,keySize,key,elementSize,element,elementDestructor);
}

inline void hashMapDestroy(void* hashMap, void(*keyDestructor)(void* object),void(*elementDestructor)(void* object)) {
    if (*(DataTypeFlags*)hashMap & FlagHashMapUsesCuckoo)
        hashMapCuckooDestroy(hashMap,keyDestructor,elementDestructor);
    else
        hashMapClosedDestroy(hashMap,keyDestructor,elementDestructor);
}