#pragma once

#include "../Types/arrayList.h"
#include "../Types/backerLibListTypes.h"
#include "../Types/backerStrings.h"
#include "../Types/hashMap.h"
#include "../Types/queue.h"

//#define valueTypeOfStandardTypes()                                             \                                                   \
//               uint8_t : ListUInt8,                                            \
//                         uint16_t : ListUInt16,                                \
//                                    uint32_t : ListUInt32,                     \
//                                               uint64_t : ListUInt64,          \
//                                                          int8_t : ListInt8,   \
//                                                                   int16_t     \
//      : ListInt16,                                                             \
//        int32_t : ListInt32,                                                   \
//                  int64_t : ListInt64,                                         \
//                            float : ListFloat,                                 \
//                                    double : ListDouble,                       \
//                                             default : ListNone
//#define valueTypeOfArrayList()                                                 \
//  ArrayList:                                                                   \
//  ListArrayList, valueTypeOfStandardTypes()
//#define valueTypeOfString()                                                    \
//  String:                                                                      \
//  ListString, valueTypeOfArrayList()
//#define valueTypeOfQueue()                                                     \
//  Queue:                                                                       \
//  ListQueue, valueTypeOfString()
//#define valueTypeOfHashMap()                                                   \
//  HashMap:                                                                     \
//  ListHashMap, Set : ListSet, BitSet : ListBitSet, valueTypeOfQueue()
//
// #define valueTypeOf(x) _Generic((x), valueTypeOfHashMap())
//
// #define HashArrayNode void
// #define hashArrayNode void
//
// #define hashMapCreateStack(size, key, element, elementsArePointers, \
//                           hashFunction) \
//  hashMapCreateStack(size, sizeof(key), sizeof(element), valueTypeOf(key), \
//                     valueTypeOf(element), elementsArePointers, hashFunction)
// #define hashMapCreate(size, key, element, elementsArePointers, hashFunction)
// \
//  hashMapCreate(size, sizeof(key), sizeof(element), valueTypeOf(key), \
//                valueTypeOf(element), elementsArePointers, hashFunction)
// #define setCreateStack(size, key, hashFunction) \
//  setCreateStack(size, sizeof(key), valueTypeOf(key), hashFunction)
// #define setCreate(size, key, hashFunction) \
//  setCreate(size, sizeof(key), valueTypeOf(key), hashFunction)
//
// #define hashMapInsert(hashMap, key, element) \
//  hashMapInsert(hashMap, key, valueTypeOf(*key), element,
//  valueTypeOf(*element))
// #define setInsert(hashMap, key) setInsert(hashMap, key, valueTypeOf(*key))
// #define hashMapGet(hashMap, key, elementStore) \
//  hashMapGet(hashMap, key, valueTypeOf(*key), &elementStore)
// #define setGet(hashMap, key) setGet(hashMap, key, valueTypeOf(*key))
// #define hashMapRemove(hashMap, key, keyDestructor, elementDestructor) \
//  hashMapRemove(hashMap, key, valueTypeOf(*key), keyDestructor, \
//                elementDestructor)
// #define setRemove(hashMap, key, keyDestructor) \
//  setRemove(hashMap, key, valueTypeOf(*key), keyDestructor)
// #define hashMapReplace(hashMap, key, element, destructorOfPrevElement) \
//  hashMapReplace(hashMap, key, valueTypeOf(*key), element, \
//                 valueTypeOf(element), destructorOfPrevElement)
//
// #define arrayListElementInsert(arrayList, index, elements, amountOfElements)
// \
//  arrayListElementInsert(arrayList, index, element, amountOfElements, \
//                         valueTypeOf(*element))
// #define arrayListElementSet(arrayList, index, element) \
//  arrayListElementSet(arrayList, index, element, valueTypeOf(*element))

#define queueEnqueue(queue, element)                                           \
  queueEnqueue(queue, element, sizeof(element))
#define queueDequeue(queue, elementBuff)                                       \
  queueDequeue(queue, elementBuff, sizeof(elementBuff)