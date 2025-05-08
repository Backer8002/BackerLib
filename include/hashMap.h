#ifndef hashMap_h_
#define hashMap_h_

#ifdef _WINDOWS
#ifdef BASICFUNCTIONS_EXPORTS 
#define HASHMAP __declspec(dllexport)
#else
#define HASHMAP __declspec(dllimport)
#endif
#else
#define HASHMAP
#endif

#include<stddef.h>
#include<stdbool.h>
#include<stdint.h>
#include<arrayList.h>
HASHMAP typedef enum {
    Vector
} hashTypes;

//sets.c
HASHMAP typedef struct set
{
    size_t sizeOfArray;
    uint64_t* array;
    uint64_t theHashOfTheSet;
    uint64_t (*hashFunction)(void*);
    uint8_t typeOfHash;
    bool (*equalityChecker)(struct set*,struct set*);
}Set;

extern HASHMAP bool setGet(Set* set, uint64_t hash);
extern HASHMAP bool setAdd(Set* set,uint64_t hash);
extern HASHMAP bool setRemove(Set* set, uint64_t hash);
extern HASHMAP bool setIsEmpty(Set* set);
extern HASHMAP int setAnd(Set* firstSet,Set* secondSet);
extern HASHMAP bool setCompare(Set* firstSet, Set* secondSet);

//hashes.c
extern HASHMAP uint64_t baseXtoBase10Vector(void* vector);


//hashMap.c

HASHMAP typedef struct hashMap {
    size_t sizeOfArray;
    size_t amountOfValues;
    ArrayList* array;
    uint64_t theHashOfTheSet;
    uint64_t (*hashFunction)(void*);
    void* (*get)(struct hashMap,uint64_t);
    void (*put)(struct hashMap,uint64_t, void*);
    void* (*remove)(struct hashMap, uint64_t);
    Set* (*keySet)(struct hashMap);
    ArrayList* (*valueList)(struct hashMap);
    hashTypes typeOfHash;
} HashMap;
#endif