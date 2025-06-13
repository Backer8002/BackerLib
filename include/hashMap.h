#ifndef HashMap_h_
#define HashMap_h_

#ifdef DLL
#ifdef BASICFUNCTIONS_EXPORTS 
#define HASHMAP __declspec(dllexport)
#else
#define HASHMAP __declspec(dllimport)
#endif
#else
#define HASHMAP
#endif

#include<stddef.h>
#include<stdint.h>
#include<stdarg.h>
#include<arrayList.h>
#include<backerLibListTypes.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    HASHMAP typedef struct
    {
        DataTypeHeader header;
        size_t sizeOfArray;
        uint64_t* array;
        uint64_t (*hashFunction)(void* element,size_t elementSize);
        
    }BitSet;
    extern HASHMAP bool bitSetGet(BitSet* set, uint64_t hash);
    extern HASHMAP bool bitSetAdd(BitSet* set, uint64_t hash);
    extern HASHMAP bool bitSetRemove(BitSet* set, uint64_t hash);
    extern HASHMAP bool bitSetIsEmpty(BitSet* set);
    extern HASHMAP int bitSetAnd(BitSet* firstSet, BitSet* secondSet);
    extern HASHMAP bool bitSetCompare(BitSet* firstSet, BitSet* secondSet);
    extern HASHMAP void bitSetDestroy(BitSet* set);


    typedef struct hashArrayNode{
        struct hashArrayNode* next;
        void* element;
        void* key;
    } HashArrayNode;

    typedef struct {
        DataTypeHeader header;
        ListTypes_t keyType;
        uint64_t(*hashFunction)(const void* element,size_t elementSize);
        size_t sizeOfHashArray;
        size_t sizeOfKey;
        size_t sizeOfElement;
        size_t amountOfElements;
        HashArrayNode** hashArray;
    } HashMap;

    typedef struct { HashMap hashMap; } Set;

    typedef enum HashMapError {
        HashMapOperationSuccsess = 0,
        HashMapCannotAllocMem,
        HashMapKeyAlreadyExists,
        HashMapKeyDoesNotExist,
        HashMapCannotRealloc
    } HashMapError_t;

    #define HASHMAP_MAX_DEPTH UINT32_MAX
    #define HASHMAP_MAX_LOADFACTOR 0.75f
    #define MAX_CUCKOO_OF_SIZE 0.5f
    #define MAX_REHASH 5ull

extern uint64_t hashFunctionDefualt(size_t amountOfVars, ...);
extern HashMap hashMapCreateStack(size_t intialSize, size_t keySize, size_t elementSize, ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers, uint64_t(*hashFunction)(const void* element, size_t elementSize));
extern HashMap* hashMapCreate(size_t intialSize, size_t keySize, size_t elementSize, ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers, uint64_t(*hashFunction)(const void* element, size_t elementSize));
extern Set setCreateStack(size_t intialSize, size_t keySize, ListTypes_t keyType, uint64_t(*hashFunction)(const void* element, size_t elementSize));
extern Set* setCreate(size_t intialSize, size_t keySize, ListTypes_t keyType,uint64_t(*hashFunction)(const void* element, size_t elementSize));

extern HashMapError_t hashMapInsert(HashMap* hashMap, void* key, ListTypes_t keyType, void* element, ListTypes_t elementType);
extern HashMapError_t setInsert(HashMap* hashMap, void* key, ListTypes_t keyType);
extern HashMapError_t hashMapGet(const HashMap* hashMap, const void* key, ListTypes_t keyType, void** element);
extern inline bool    setGet(const HashMap* hashMap, const void* key, ListTypes_t keyType);
inline bool hashMapContainsKey(const HashMap* hashMap, const void* key, ListTypes_t keyType) { setGet(hashMap, key, keyType); }
extern HashMapError_t hashMapRemove(HashMap* hashMap, const void* key, ListTypes_t keyType, void (*keyDestructor)(void* element), void (*elementDestructor)(void* element));
extern inline HashMapError_t setRemove(HashMap* hashMap, void* key, ListTypes_t keyType, void (*keyDestructor)(void* element));
extern HashMapError_t hashMapReplace(HashMap* hashMap, const void* key, ListTypes_t keyType, void* element, ListTypes_t elementType, void (*destrutorOfPreviousElement)(void* element));
extern void hashMapDestroy(HashMap* hashMap, void(*keyDestructor)(void* key), void(*elementDestructor)(void* element));
inline uint64_t hashFunctionDefualtSingleVar(const void* element, size_t size) { return hashFunctionDefualt(1, size, element); }
inline uint64_t hashFunctionDefualtSingleVarWithSalt(const void* element, size_t size,uint32_t salt) { return hashFunctionDefualt(2, sizeof(uint32_t),&salt, size, element); }

typedef struct {
    void* key;
    void* element;
} HashArrayElement;

typedef struct {
    DataTypeHeader header;
    ListTypes_t keyType;
    uint64_t(*hashFunction)(const void* element, size_t elementSize, uint32_t salt);
    size_t sizeOfOneHashArray;
    size_t sizeOfKey;
    size_t sizeOfElement;
    uint32_t salt1;
    uint32_t salt2;
    HashArrayElement* hashArray;
} HashMapCuckoo;

extern HashMapError_t hashMapCuckooRemove(HashMapCuckoo* hashMap, const void* key, ListTypes_t keyType, void(*keyDestructor)(void* object), void(*elementDestructor)(void* object));
extern void* hashMapCuckooGet(const HashMapCuckoo* hashMap, const void* key, ListTypes_t keyType);
extern HashMapError_t hashMapCuckooInsert(HashMapCuckoo* hashMap, void** key, ListTypes_t keyType, void** element, ListTypes_t elementType);
extern HashMapCuckoo* hashMapCuckooCreate(size_t intialSize, size_t keySize, size_t elementSize, ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers, uint64_t(*hashFunction)(const void* element, size_t elementSize, uint32_t salt));
extern HashMapCuckoo hashMapCuckooCreateStack(size_t intialSize, size_t keySize, size_t elementSize, ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers, uint64_t(*hashFunction)(const void* element, size_t elementSize, uint32_t salt));
#ifdef __cplusplus
}
#endif // __cplusplus

#endif

