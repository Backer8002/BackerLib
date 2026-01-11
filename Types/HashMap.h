#ifndef HashMap_h_
#define HashMap_h_

#include "BL_UnorderedContainer.h"
#include "TypesMain.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#else
#define noexcept
#endif // __cplusplus

    typedef struct BL_Bitset {
        BL_DataTypeFlags header;
        size_t           maxAmountOfElements;
        uint8_t*         array;
    } BL_Bitset;

    /**
     * @brief Returns the state of an index
     * @param set Pointer to valid BitSet
     * @param index Bit to access
     * @return ContainerInvalidIndex if index was invalid.
     * @return ContainerOPUnsuccessful if bit was low.
     * @return ContainerOPSuccessful if bit was high.
     */
    extern BL_ContainerError bl_bitset_get(const BL_Bitset* set, size_t index) noexcept;
    /**
     * @brief Sets index bit to high. Returns previous state.
     * @param set Pointer to valid BitSet
     * @param index Bit to change
     * @return ContainerInvalidIndex if index was invalid.
     * @return ContainerOPUnsuccessful if bit was low.
     * @return ContainerOPSuccessful if bit was high.
     */
    extern BL_ContainerError bl_bitset_add(BL_Bitset* set, size_t index) noexcept;
    /**
     * @brief Sets index bit to high. Returns previous state.
     * @param set Pointer to valid BitSet
     * @param index Bit to change
     * @return ContainerInvalidIndex if index was invalid.
     * @return ContainerOPUnsuccessful if bit was low.
     * @return ContainerOPSuccessful if bit was high.
     */
    extern BL_ContainerError bl_bitset_remove(BL_Bitset* set, size_t index) noexcept;
    /**
     * @param set Pointer to valid BitSet
     * @return true if BitSet is empty.
     */
    extern bool              bl_bitset_is_empty(BL_Bitset* set) noexcept;
    /**
     * @brief Preforms Exclusive AND on firstSet with secondSet
     * @param firstSet Pointer to valid BitSet
     * @param secondSet Pointer to valid BitSet
     * @return ContainerOPUnsuccessful if the BitSets were of different size.
     */
    extern BL_ContainerError bl_bitset_and(BL_Bitset* firstSet, BL_Bitset* secondSet) noexcept;
    /**
     * @brief Preforms OR on firstSet with secondSet
     * @param firstSet Pointer to valid BitSet
     * @param secondSet Pointer to valid BitSet
     * @return ContainerOPUnsuccessful if the BitSets were of different size.
     */
    extern BL_ContainerError bl_bitset_or(BL_Bitset* firstSet, BL_Bitset* secondSet) noexcept;
    /**
     * @brief Preforms Exclusive OR on firstSet with secondSet
     * @param firstSet Pointer to valid BitSet
     * @param secondSet Pointer to valid BitSet
     * @return ContainerOPUnsuccessful if the BitSets were of different size.
     */
    extern BL_ContainerError bl_bitset_xor(BL_Bitset* firstSet, BL_Bitset* secondSet) noexcept;
    /**
     * @brief Inverts all indexes in BitSet.
     * @param set Pointer to valid BitSet
     */
    extern void              bl_bitset_not(BL_Bitset* set) noexcept;
    /**
     * @brief Destroys BitSet if applicable.
     * @param set Pointer to BitSet
     */
    extern void              bl_bitset_destroy(BL_Bitset* set) noexcept;
    /**
     * @brief Creates a BitSet on the Stack. Use is_valid to check validity.
     * @param amountOfElements Amount of indexes to allocate
     * @param objectIsHeapAllocated Is the bitset going to be on the heap?
     */
    extern BL_Bitset         bl_bitset_create(size_t amountOfElements, bool objectIsHeapAllocated) noexcept;
    /**
     *
     * @param set Pointer to Bitset or NULL
     * @return true if set is valid, else false
     */
    extern bool              bl_bitset_is_valid(const BL_Bitset* set) noexcept;

    typedef struct BL_HashmapClosed {
        BL_UnorderedContainer unorderedContainer;
        uint64_t (*hashFunction)(const void* element);
        bool (*compare)(const void*, const void*);
        size_t* hashArray;
        size_t  lengthOfHashArray;
        size_t  keySize;
        size_t  elementOffset;
    } BL_Hashmap;

#define HASHMAP_MAX_DEPTH             UINT32_MAX
#define HASHMAP_MAX_LOADFACTOR        1.0f

#define FlagHashMapKeyIsDataTypeFlags 0x100

    /**
     * @brief Creates an HashMap on the stack. Use isValidObject to check validity.
     * @param initialSize Initial amount of elements to store
     * @param keySize Size of Key
     * @param elementSize Maximum size of element to store
     * @param compare Callback to compare equality of keys
     * @param hashFunction Callback to hash a key
     * @return HashMap on the stack.
     */
    BL_Hashmap               bl_hashmap_create_stack(size_t initialSize, size_t keySize, size_t elementSize, bool (*compare)(const void* first, const void* second),
                                                     uint64_t (*hashFunction)(const void* element)) noexcept;

    /**
     * @brief Creates an HashMap on the heap.
     * @param initialSize Initial amount of elements to store
     * @param keySize Size of Key
     * @param elementSize Maximum size of element to store
     * @param compare Callbak to compare equality of keys
     * @param hashFunction Callback to hash a key
     * @return NULL if allocation failed.
     */
    BL_Hashmap*              bl_hashmap_create_heap(size_t initialSize, size_t keySize, size_t elementSize, bool (*compare)(const void* first, const void* second),
                                                    uint64_t (*hashFunction)(const void* element)) noexcept;

    /**
     * @brief Hash function. Defualt for the HashMap implementations.
     * @param amountOfVars Amount of vars to insert
     * @param ... size of element followed by pointer to element
     * @return Hash.
     */
    extern uint64_t          bl_hashfunction_defualt(size_t amountOfVars, ...) noexcept;
    /**
     *
     * @param element Pointer to element to hash
     * @param size Size of element to hash
     * @return Hash
     */
    extern uint64_t          bl_hashfunction_defualt_single_var(const void* element, size_t size) noexcept;

    extern uint64_t bl_hashfunction_string_view(const void* element) noexcept;

    /**
     * @brief Inserts element into hashMap using key.
     * @param hashMap Pointer to valid HashMap
     * @param keySize Size of key
     * @param key Key to insert with
     * @param elementSize Size of element to insert
     * @param element Element to insert
     * @return ContainerInvalidSize if key has different size than the size of a key in the hashMap
     * or if elementSize is larger than the size of a single element in the hashMap.
     * @return ContainerOPUnsuccessful if key already is associated with an element in the hashMap.
     * @return ContainerAllocFailure if hashMap could not grow.
     */
    extern BL_ContainerError bl_hashmap_insert(BL_Hashmap* hashMap, size_t keySize, const void* key, size_t elementSize, const void* element) noexcept;
    /**
     *
     * @param hashMap Pointer to valid HashMap
     * @param keySize Size of key
     * @param key Key to locate element with
     * @return NULL if key does not exist in hashMap, if key does not match the size of a key in the hashMap, else pointer to element.
     */
    extern void*             bl_hashmap_get(const BL_Hashmap* hashMap, size_t keySize, const void* key) noexcept;
    /**
     *
     * @param hashMap Pointer to valid HashMap
     * @param keySize Size of key
     * @param key Key
     * @return true if Key does exist in hashMap.
     * @return false if keySize was invalid or if element does not exist.
     */
    extern bool              bl_hashmap_contains_key(const BL_Hashmap* hashMap, size_t keySize, const void* key) noexcept;
    /**
     * @brief Removes element from hashMap.
     * @param hashMap Pointer to valid HashMap
     * @param keySize Size of key
     * @param key Key
     * @param keyDestructor Optional destructor of key
     * @param elementDestructor Optional destructor of element
     * @return ContainerInvalidSize if size of key was different from the size of a key in the hashMap.
     * @return ContainerOPUnsuccessful if key did not exist in hashMap.
     */
    extern BL_ContainerError bl_hashmap_remove(BL_Hashmap* hashMap, size_t keySize, const void* key, void (*keyDestructor)(void* object), void (*elementDestructor)(void* object)) noexcept;
    /**
     * @brief Replaces element at key.
     * @param hashMap Pointer to valid HashMap
     * @param keySize Size of key
     * @param key Key
     * @param elementSize Size of element to insert
     * @param element Element to insert
     * @param elementDestructor Optional destructor of element
     * @return ContainerInvalidSize if size of key was different then the size of a key in the hashMap.
     * @return ContainerOPUnsuccessful if key did not exist in hashMap.
     */
    extern BL_ContainerError bl_hashmap_replace(BL_Hashmap* hashMap, size_t keySize, const void* key, size_t elementSize, const void* element, void (*elementDestructor)(void* object)) noexcept;
    /**
     * @brief Destroys hashMap if applicable
     * @param hashMap Pointer to HashMap
     * @param keyDestructor Optional key destructor
     * @param elementDestructor Optional element destructor
     */
    extern void              bl_hashmap_destroy(BL_Hashmap* hashMap, void (*keyDestructor)(void* object), void (*elementDestructor)(void* object)) noexcept;

    extern bool              bl_hashmap_is_valid(const BL_Hashmap* hashMap);
#ifdef __cplusplus
}
#else
#undef noexcept
#endif // __cplusplus

#endif
