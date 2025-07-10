#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef unsigned char Byte;
typedef Byte*         Bytes;


typedef enum ListTypes {
    ListNone = 0,
    ListCString,
    ListInt8,
    ListInt16,
    ListInt32,
    ListInt64,
    ListUInt8,
    ListUInt16,
    ListUInt32,
    ListUInt64,
    ListFloat,
    ListDouble,
    ListArrayList,
    ListString,
    ListQueue,
    ListHashMap,
    ListSet,
    ListBitSet,
    ListMatrix,
    ListEventList
} ListTypes_t;

typedef struct {
    uint32_t    flags;
    ListTypes_t dataArrayVarType;
    ListTypes_t objectType;
} DataTypeHeader;

#define ObjectFlagIsOnHeap           0x1
#define ObjectFlagMutexExists        0x2
#define ObjectFlagContentsIsPointers 0x4

static inline bool isValidObject(DataTypeHeader* header) {
    return header->dataArrayVarType != ListNone;
}

static inline bool typeIsPrimitive(ListTypes_t listType) {
    switch (listType) {
    case ListNone:
    case ListInt8:
    case ListInt16:
    case ListInt32:
    case ListInt64:
    case ListUInt8:
    case ListUInt16:
    case ListUInt32:
    case ListUInt64:
    case ListFloat:
    case ListDouble:
        return true;
    default:
        return false;
    }
}