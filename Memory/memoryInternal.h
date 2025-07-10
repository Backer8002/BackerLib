#ifndef BACKERLIB_MEMORY_INTERNAL
#define BACKERLIB_MEMORY_INTERNAL

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#endif // __cplusplus


#include <stddef.h>
#include <stdint.h>
    typedef struct memoryPoolNext {
        size_t                 sizeOfFragment;
        uint32_t               pageOffset;
        struct memoryPoolNext* next;
    } MemoryPoolNext;


    typedef struct {
        size_t         totalSizeOfMemoryPool;
        const size_t   pageSize;
        MemoryPoolNext firstFragment;
    } MemoryPool;


#ifdef __cplusplus
    }
}
#endif // __cplusplus

#endif // !BACKERLIB_MEMORY_INTERNAL
