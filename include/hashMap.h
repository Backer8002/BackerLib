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
        HashArrayNode* hashArray;
    } HashMap;

    typedef HashMap Set;

    typedef enum HashMapError {
        HashMapOperationSuccsess = 0,
        HashMapCannotAllocMem,
        HashMapKeyAlreadyExists,
        HashMapKeyDoesNotExist
    } HashMapError_t;


extern uint64_t hashFunctionDefualt(size_t amountOfVars, ...);
extern HashMap hashMapCreateStack(size_t intialSize, size_t keySize, size_t elementSize, ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers, uint64_t(*hashFunction)(const void* element, size_t elementSize));
extern HashMap* hashMapCreate(size_t intialSize, size_t keySize, size_t elementSize, ListTypes_t keyType, ListTypes_t elementType, bool elementsArePointers, uint64_t(*hashFunction)(const void* element, size_t elementSize));
extern Set setCreateStack(size_t intialSize, size_t keySize, ListTypes_t keyType, uint64_t(*hashFunction)(const void* element, size_t elementSize));
extern Set* setCreate(size_t intialSize, size_t keySize, ListTypes_t keyType,uint64_t(*hashFunction)(const void* element, size_t elementSize));

extern HashMapError_t hashMapInsert(HashMap* hashMap, void* key, ListTypes_t keyType, void* element, ListTypes_t elementType, size_t reHashingIteration);
extern HashMapError_t setInsert(HashMap* hashMap, void* key, ListTypes_t keyType, size_t reHashingIteration);
extern HashMapError_t hashMapGet(const HashMap* hashMap, const void* key, ListTypes_t keyType, void** element);
extern inline bool    setGet(const HashMap* hashMap, const void* key, ListTypes_t keyType);
extern HashMapError_t hashMapRemove(HashMap* hashMap, const void* key, ListTypes_t keyType, void (*keyDestructor)(void* element), void (*elementDestructor)(void* element));
extern inline HashMapError_t setRemove(HashMap* hashMap, void* key, ListTypes_t keyType, void (*keyDestructor)(void* element));
extern HashMapError_t hashMapReplace(HashMap* hashMap, const void* key, ListTypes_t keyType, void* element, ListTypes_t elementType, void (*destrutorOfPreviousElement)(void* element));

inline uint64_t hashFunctionDefualtSingleVar(const void* element, size_t size) { return hashFunctionDefualt(1, size, element); }

#ifndef Implementation
#define HashArrayNode void
#define hashArrayNode void

#define hashMapInsert(hashMap, key, keyType,element,elementType) hashMapInsert(hashMap,key,keyType,element,elementType,0)
#endif // !Implementation

#ifdef __cplusplus
}
#endif // __cplusplus

#endif

