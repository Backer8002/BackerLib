#ifndef BL_TYPESMAIN_H
#define BL_TYPESMAIN_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#endif

    typedef unsigned char BL_Byte;
    typedef BL_Byte*         BL_Bytes;

    typedef uint32_t      BL_DataTypeFlags;

    typedef enum BL_ContainerError {
        BL_ContainerOPSuccessful = 0,
        BL_ContainerOPUnsuccessful,
        BL_ContainerInvalidIndex,
        BL_ContainerInvalidSize,
        BL_ContainerAllocFailure
    } BL_ContainerError;

#define ObjectFlagIsValid                       0x1
#define ObjectFlagIsOnHeap                      0x2
#define ObjectFlagMutexExists                   0x4

#define ObjectFlagIsContainer                   0x00010000
#define ObjectFlagIsDynamicContainer            0x00020000
#define ObjectFlagIsNotContinuous               0x00040000
#define ObjectFlagIsNotContinuousCustomTracking 0x00080000
#define ObjectFlagElementsArePointers           0x00400000
#define ObjectFlagArrayNoSort                   0x00800000
#ifdef __cplusplus
    }
};
#endif
#endif
