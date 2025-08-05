#ifndef HashMap_h_
#define HashMap_h_

#include "TypesMain.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#endif // __cplusplus
#include <threads.h>

    typedef enum HashMapError {
        HashMapOperationSuccsess = 0,
        HashMapCannotAllocMem,
        HashMapKeyAlreadyExists,
        HashMapKeyDoesNotExist,
        HashMapCannotRealloc,
        HashMapInvalidOperation
    } HashMapError_t;

    typedef struct {
        DataTypeFlags header;
        size_t        maxAmountOfElements;
        uint8_t*      array;
    } BitSet;

    /**
     * @brief Returns the state of an index
     * @param set Pointer to valid BitSet
     * @param index Bit to access
     * @return ContainerInvalidIndex if index was invalid.
     * @return ContainerOPUnsuccessful if bit was low.
     * @return ContainerOPSuccessful if bit was high.
     */
    extern ContainerError bitSetGet(BitSet* set, size_t index);
    /**
     * @brief Sets index bit to high. Returns previous state.
     * @param set Pointer to valid BitSet
     * @param index Bit to change
     * @return ContainerInvalidIndex if index was invalid.
     * @return ContainerOPUnsuccessful if bit was low.
     * @return ContainerOPSuccessful if bit was high.
     */
    extern ContainerError bitSetAdd(BitSet* set, size_t index);
    /**
     * @brief Sets index bit to high. Returns previous state.
     * @param set Pointer to valid BitSet
     * @param index Bit to change
     * @return ContainerInvalidIndex if index was invalid.
     * @return ContainerOPUnsuccessful if bit was low.
     * @return ContainerOPSuccessful if bit was high.
     */
    extern ContainerError bitSetRemove(BitSet* set, size_t index);
    /**
     * @param set Pointer to valid BitSet
     * @return true if BitSet is empty.
     */
    extern bool           bitSetIsEmpty(BitSet* set);
    /**
     * @brief Preforms Exclusive AND on firstSet with secondSet
     * @param firstSet Pointer to valid BitSet
     * @param secondSet Pointer to valid BitSet
     * @return ContainerOPUnsuccessful if the BitSets were of different size.
     */
    extern ContainerError bitSetAnd(BitSet* firstSet, BitSet* secondSet);
    /**
     * @brief Preforms OR on firstSet with secondSet
     * @param firstSet Pointer to valid BitSet
     * @param secondSet Pointer to valid BitSet
     * @return ContainerOPUnsuccessful if the BitSets were of different size.
     */
    extern ContainerError bitSetOr(BitSet* firstSet, BitSet* secondSet);
    /**
     * @brief Preforms Exclusive OR on firstSet with secondSet
     * @param firstSet Pointer to valid BitSet
     * @param secondSet Pointer to valid BitSet
     * @return ContainerOPUnsuccessful if the BitSets were of different size.
     */
    extern ContainerError bitSetXOr(BitSet* firstSet, BitSet* secondSet);
    /**
     * @brief Inverts all indexes in BitSet.
     * @param set Pointer to valid BitSet
     */
    extern void           bitSetNot(BitSet* set);
    /**
     * @brief Destroys BitSet if applicable.
     * @param set Pointer to BitSet
     */
    extern void           bitSetDestroy(BitSet* set);
    /**
     * @brief Creates a BitSet on the Stack. Use isValidObject to check validity.
     * @param amountOfElements Amount of indexes to allocate
     * @param objectIsHeapAllocated Is the bitset going to be on the heap?
     */
    extern BitSet         bitSetCreate(size_t amountOfElements, bool objectIsHeapAllocated);

    typedef union HashMap {
        DataTypeFlags header;
        Container     container;
        struct {
            UnorderedContainer unorderedContainer;
            uint64_t (*hashFunction)(const void* element, size_t elementSize);
            size_t* hashArray;
            size_t  lengthOfHashArray;
            size_t  keySize;
            size_t  elementOffset;
        };
    } HashMap;

#define HASHMAP_MAX_DEPTH                       UINT32_MAX
#define HASHMAP_MAX_LOADFACTOR                  0.75f
#define HASHMAP_CUCKOO_MAX_CUCKOO_OF_SIZE       0.75f
#define HASHMAP_CUCKOO_MAX_REHASH_BEFORE_RESIZE 25ull

#define FlagHashMapKeyIsDataTypeFlags           0x100

    extern HashMap         hashMapCreateStack(size_t initialSize, size_t keySize, size_t elementSize, bool elementsArePointers, bool keyIsDataTypeFlags,
                                              uint64_t (*hashFunction)(const void* element, size_t elementSize));

    extern HashMap*        hashMapCreateHeap(size_t initialSize, size_t keySize, size_t elementSize, bool elementsArePointers, bool keyIsDataTypeFlags,
                                             uint64_t (*hashFunction)(const void* element, size_t elementSize));

    extern ContainerError  hashMapInsert(HashMap* hashMap, size_t sizeOfKey, const void* key, size_t sizeOfElement, void* element);
    extern void*           hashMapGet(const HashMap* hashMap, size_t sizeOfKey, const void* key);
    extern bool     hashMapContainsKey(const HashMap* hashMap, size_t sizeOfKey, const void* key);
    extern ContainerError  hashMapRemove(HashMap* hashMap, size_t sizeOfKey, const void* key, void (*keyDestructor)(void* element), void (*elementDestructor)(void* element));
    extern ContainerError  hashMapReplace(HashMap* hashMap, size_t sizeOfKey, const void* key, size_t sizeOfElement, void* element, void (*elementDestructor)(void* element));
    extern void            hashMapDestroy(HashMap* hashMap, void (*keyDestructor)(void* key), void (*elementDestructor)(void* element));

    extern uint64_t        hashFunctionDefualt(size_t amountOfVars, ...);
    static inline uint64_t hashFunctionDefualtSingleVar(const void* element, size_t size) {
        return hashFunctionDefualt(1, size, element);
    }
    static inline uint64_t hashFunctionDefualtSingleVarWithSalt(const void* element, size_t size, uint32_t salt) {
        return hashFunctionDefualt(2, sizeof(salt), &salt, size, element);
    }

#define HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ 0x200

    typedef union {
        DataTypeFlags header;
        Container     container;
        struct {
            UnorderedContainer unorderedContainer;
            uint64_t (*hashFunction)(const void* element, size_t elementSize, uint32_t salt);
            size_t*  hashArray;
            size_t   lengthOfHashArray;
            size_t   keySize;
            size_t   elementOffset;
            uint32_t salt1, salt2;
            size_t   swapspace;
        };
    } HashMapCuckoo;

    extern HashMapCuckoo  hashMapCuckooCreateStack(size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags,
                                                   uint64_t (*hashFunction)(const void* element, size_t elementSize, uint32_t salt));
    extern HashMapCuckoo* hashMapCuckooCreateHeap(size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags,
                                                  uint64_t (*hashFunction)(const void* element, size_t elementSize, uint32_t salt));
    extern ContainerError hashMapCuckooInsert(HashMapCuckoo* hashMap, size_t keySize, const void* key, size_t elementSize, const void* element);
    extern void*          hashMapCuckooGet(HashMapCuckoo* hashMap, size_t keySize, const void* key);
    extern ContainerError hashMapCuckooRemove(HashMapCuckoo* hashMap, size_t keySize, const void* key, void (*keyDestructor)(void* object), void (*elementDestructor)(void* object));
    extern void           hashMapCuckooDestroy(HashMapCuckoo* hashMap, void (*keyDestructor)(void* object), void (*elementDestructor)(void* object));

#ifdef __cplusplus
    }
};
#endif // __cplusplus

#endif
