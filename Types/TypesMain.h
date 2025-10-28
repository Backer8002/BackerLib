#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#endif

    typedef unsigned char Byte;
    typedef Byte*         Bytes;

    typedef uint32_t      DataTypeFlags;

    typedef struct Container {
        DataTypeFlags header;
        uint32_t      byteSizeOfSingleElement;
        size_t        amountOfIndexes;
        void*         array;
    } Container;

    typedef union UnorderedContainer {
        struct {
            DataTypeFlags header;
        };
        struct {
            Container container;
            size_t    maxSize;
            uint64_t*  bitset;
        };
    } UnorderedContainer;

    typedef union DynamicContainer {
        struct {
            DataTypeFlags header;
        };
        struct {
            Container container;
            size_t    containerMaxSize;
        };
    } DynamicContainer;

    typedef enum ContainerError {
        ContainerOPSuccessful = 0,
        ContainerOPUnsuccessful,
        ContainerInvalidIndex,
        ContainerInvalidSize,
        ContainerAllocFailure
    } ContainerError;

#define ObjectFlagIsValid                       0x1
#define ObjectFlagIsOnHeap                      0x2
#define ObjectFlagMutexExists                   0x4

#define ObjectFlagIsContainer                   0x00010000
#define ObjectFlagIsDynamicContainer            0x00020000
#define ObjectFlagIsNotContinuous               0x00040000
#define ObjectFlagIsNotContinuousCustomTracking 0x00080000
#define ObjectFlagElementsArePointers           0x00400000
#define ObjectFlagArrayNoSort                   0x00800000

    static bool isValidObject(const DataTypeFlags* flags) {
        if (flags)
            return *flags & ObjectFlagIsValid;
        return false;
    }
#ifdef __cplusplus
    }
};
#endif
