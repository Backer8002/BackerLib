#include "memoryInternal.h"

// #ifdef _Windows
#include <Windows.h>
// #endif

static MemoryPool heapHead;




void*             myMalloc(size_t size) {
}

void* myCalloc(size_t count, size_t size) {
}

void* myRealloc(void* current, size_t size) {
    if (current == NULL)
        return myMalloc(size);
}

void myFree(void* address) {
}