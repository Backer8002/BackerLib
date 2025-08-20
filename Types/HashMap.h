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
    extern ContainerError bitSetGet(const BitSet* set, size_t index);
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

    /**
     * @brief Creates an HashMap on the stack. Use isValidObject to check validity.
     * @param initialSize Initial amount of elements to store
     * @param keySize Size of Key
     * @param elementSize Maximum size of element to store
     * @param keyIsDataTypeFlags Can Key* be cast to DataTypeFlags*
     * @param hashFunction Callback to hash a key
     * @return HashMap on the stack.
     */
    extern HashMap         hashMapCreateStack(size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags,
                                              uint64_t (*hashFunction)(const void* element, size_t elementSize));

    /**
     * @brief Creates an HashMap on the heap.
     * @param initialSize Initial amount of elements to store
     * @param keySize Size of Key
     * @param elementSize Maximum size of element to store
     * @param keyIsDataTypeFlags Can Key* be cast to DataTypeFlags*
     * @param hashFunction Callback to hash a key
     * @return NULL if allocation failed.
     */
    extern HashMap*        hashMapCreateHeap(size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags,
                                             uint64_t (*hashFunction)(const void* element, size_t elementSize));

    /**
     * @brief Inserts element into hashMap using key.
     * @param hashMap Pointer to valid HashMap
     * @param sizeOfKey Size of key
     * @param key Key to insert with
     * @param sizeOfElement Size of element to insert
     * @param element Element to insert
     * @return ContainerInvalidSize if key has different size than the size of a key in the hashMap
     * or if elementSize is larger than the size of a singel element in the hashMap.
     * @return ContainerOPUnsuccessful if key already is associated with an element in the hashMap.
     * @return ContainerAllocFailure if hashMap could not grow.
     */
    extern ContainerError  hashMapInsert(HashMap* hashMap, size_t sizeOfKey, const void* key, size_t sizeOfElement, void* element);
    /**
     *
     * @param hashMap Pointer to valid HashMap
     * @param sizeOfKey Size of key
     * @param key Key to locate element with
     * @return NULL if key does not exist in hashMap or if key does not match the size of a key in the hashMap, else pointer to element.
     */
    extern void*           hashMapGet(const HashMap* hashMap, size_t sizeOfKey, const void* key);
    /**
     *
     * @param hashMap Pointer to valid HashMap
     * @param sizeOfKey Size of key
     * @param key Key
     * @return true if Key does exist in hashMap.
     */
    extern bool            hashMapContainsKey(const HashMap* hashMap, size_t sizeOfKey, const void* key);
    /**
     * @brief Removes element from hashMap.
     * @param hashMap Pointer to valid HashMap
     * @param sizeOfKey Size of key
     * @param key Key
     * @param keyDestructor Optional destructor of key
     * @param elementDestructor Optional destructor of element
     * @return ContainerInvalidSize if size of key was different than the size of a key in the hashMap.
     * @return ContainerOPUnsuccessful if key did not exist in hashMap.
     */
    extern ContainerError  hashMapRemove(HashMap* hashMap, size_t sizeOfKey, const void* key, void (*keyDestructor)(void* element), void (*elementDestructor)(void* element));
    /**
     * @brief Replaces element at key.
     * @param hashMap Pointer to valid HashMap
     * @param sizeOfKey Size of key
     * @param key Key
     * @param sizeOfElement Size of element to insert
     * @param element Element to insert
     * @param elementDestructor Optional destructor of element
     * @return ContainerInvalidSize if size of key was different than the size of a key in the hashMap.
     * @return ContainerOPUnsuccessful if key did not exist in hashMap.
     */
    extern ContainerError  hashMapReplace(HashMap* hashMap, size_t sizeOfKey, const void* key, size_t sizeOfElement, void* element, void (*elementDestructor)(void* element));
    /**
     * @brief Destroys HashMap if applicable
     * @param hashMap Pointer to HashMap
     * @param keyDestructor Optional key destructor
     * @param elementDestructor Optional element destructor
     */
    extern void            hashMapDestroy(HashMap* hashMap, void (*keyDestructor)(void* key), void (*elementDestructor)(void* element));

    /**
     * @brief Hash function. Defualt for the HashMap implementations.
     * @param amountOfVars Amount of vars to insert
     * @param ... size of element followed by pointer to element
     * @return Hash.
     */
    extern uint64_t       [[unsequenced]] hashFunctionDefualt(size_t amountOfVars, ...);
    /**
     *
     * @param element Pointer to element to hash
     * @param size Size of element to hash
     * @return Hash
     */
    static inline uint64_t hashFunctionDefualtSingleVar(const void* element, size_t size) {
        return hashFunctionDefualt(1, size, element);
    }
    /**
     *
     * @param element Pointer to element to hash
     * @param size Size of element to hash
     * @param salt 32-bit paramiter to hash with
     * @return Hash
     */
    static inline uint64_t hashFunctionDefualtSingleVarWithSalt(const void* element, size_t size, uint32_t salt) {
        return hashFunctionDefualt(2, sizeof(salt), &salt, size, element);
    }

#define HASHMAP_CUCKOO_SWAPSPACE_CONTAINS_OBJ 0x200

    typedef union HashMapCuckoo {
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

    /**
     * @brief Creates an HashMapCuckoo on the stack. Use isValidObject to check validity.
     * @param initialSize Initial amount of elements to store
     * @param keySize Size of Key
     * @param elementSize Maximum size of element to store
     * @param keyIsDataTypeFlags Can Key* be cast to DataTypeFlags*
     * @param hashFunction Callback to hash a key
     * @return HashMapCuckoo on the stack.
     */
    extern HashMapCuckoo  hashMapCuckooCreateStack(size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags,
                                                   uint64_t (*hashFunction)(const void* element, size_t elementSize, uint32_t salt));
    /**
     * @brief Creates an HashMapCuckoo on the heap.
     * @param initialSize Initial amount of elements to store
     * @param keySize Size of Key
     * @param elementSize Maximum size of element to store
     * @param keyIsDataTypeFlags Can Key* be cast to DataTypeFlags*
     * @param hashFunction Callback to hash a key
     * @return NULL if allocation failed.
     */
    extern HashMapCuckoo* hashMapCuckooCreateHeap(size_t initialSize, size_t keySize, size_t elementSize, bool keyIsDataTypeFlags,
                                                  uint64_t (*hashFunction)(const void* element, size_t elementSize, uint32_t salt));
    /**
     * @brief Inserts element into hashMap using key.
     * @param hashMap Pointer to valid HashMapCuckoo
     * @param keySize Size of key
     * @param key Key to insert with
     * @param elementSize Size of element to insert
     * @param element Element to insert
     * @return ContainerInvalidSize if key has different size than the size of a key in the hashMap
     * or if elementSize is larger than the size of a singel element in the hashMap.
     * @return ContainerOPUnsuccessful if key already is associated with an element in the hashMap.

     */
    extern ContainerError hashMapCuckooInsert(HashMapCuckoo* hashMap, size_t keySize, const void* key, size_t elementSize, const void* element);
    /**
     *
     * @param hashMap Pointer to valid HashMapCuckoo
     * @param keySize Size of key
     * @param key Key to locate element with
     * @return NULL if key does not exist in hashMap, if key does not match the size of a key in the hashMap, or if hashMap could not grow, else pointer to element.
     */
    extern void*          hashMapCuckooGet(HashMapCuckoo* hashMap, size_t keySize, const void* key);
    /**
     * @brief Removes element from hashMap.
     * @param hashMap Pointer to valid HashMapCuckoo
     * @param keySize Size of key
     * @param key Key
     * @param keyDestructor Optional destructor of key
     * @param elementDestructor Optional destructor of element
     * @return ContainerInvalidSize if size of key was different from the size of a key in the hashMap.
     * @return ContainerOPUnsuccessful if key did not exist in hashMap.
     * @return ContainerAllocFailure if hashMap could not grow for a element in the previously could not be inserted.
     */
    extern ContainerError hashMapCuckooRemove(HashMapCuckoo* hashMap, size_t keySize, const void* key, void (*keyDestructor)(void* object), void (*elementDestructor)(void* object));
    /**
     * @brief Destroys hashMap if applicable
     * @param hashMap Pointer to HashMap
     * @param keyDestructor Optional key destructor
     * @param elementDestructor Optional element destructor
     */
    extern void           hashMapCuckooDestroy(HashMapCuckoo* hashMap, void (*keyDestructor)(void* object), void (*elementDestructor)(void* object));

#ifdef __cplusplus
    }
};
#endif // __cplusplus

#endif
