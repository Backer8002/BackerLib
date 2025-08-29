#ifndef CONTAINERTESTS_HASHMAP_INTERNAL_H
#define CONTAINERTESTS_HASHMAP_INTERNAL_H

#include "HashMap.h"

#define HASHMAP_MAX_DEPTH                       UINT32_MAX
#define HASHMAP_MAX_LOADFACTOR                  0.9f
#define HASHMAP_CUCKOO_MAX_CUCKOO_OF_SIZE       0.9f
#define HASHMAP_CUCKOO_MAX_REHASH_BEFORE_RESIZE 250ull

#define FlagHashMapKeyIsDataTypeFlags           0x100
#define HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ   0x200
#define FlagHashMapUsesCuckoo                   0x400

bool internal_memCmpKey(size_t keySize, const void* key, const void* otherKey, bool isDataTypeFlagsQualified);

extern ContainerError hashMapClosedInsert(HashMapClosed* hashMap, size_t sizeOfKey, const void* key, size_t sizeOfElement, void* element);
extern void*          hashMapClosedGet(const HashMapClosed* hashMap, size_t sizeOfKey, const void* key);
extern ContainerError hashMapClosedRemove(HashMapClosed* hashMap, size_t sizeOfKey, const void* key, void (*keyDestructor)(void* element), void (*elementDestructor)(void* element));
extern ContainerError hashMapClosedReplace(HashMapClosed* hashMap, size_t sizeOfKey, const void* key, size_t sizeOfElement, void* element, void (*elementDestructor)(void* element));
extern void           hashMapClosedDestroy(HashMapClosed* hashMap, void (*keyDestructor)(void* key), void (*elementDestructor)(void* element));
extern ContainerError hashMapCuckooInsert(HashMapCuckoo* hashMap, size_t keySize, const void* key, size_t elementSize, const void* element);
extern void*          hashMapCuckooGet(HashMapCuckoo* hashMap, size_t keySize, const void* key);
extern ContainerError hashMapCuckooRemove(HashMapCuckoo* hashMap, size_t keySize, const void* key, void (*keyDestructor)(void* object), void (*elementDestructor)(void* object));
extern ContainerError hashMapCuckooReplace(HashMapCuckoo* hashMap, size_t keySize, const void* key,size_t elementSize, void* element, void(*elementDestructor)(void* object));
extern void           hashMapCuckooDestroy(HashMapCuckoo* hashMap, void (*keyDestructor)(void* object), void (*elementDestructor)(void* object));


#endif // CONTAINERTESTS_HASHMAP_INTERNAL_H
