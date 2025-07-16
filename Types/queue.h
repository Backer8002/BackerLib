#ifndef Queue_h_
#define Queue_h_

#ifdef DLL
#ifdef BASICFUNCTIONS_EXPORTS
#define QUEUE __declspec(dllexport);
#else
#define QUEUE __declspec(dllimport);
#endif
#else
#define QUEUE
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "backerLibListTypes.h"
#include <stdbool.h>
#include <stdint.h>
#include <threads.h>

#define FlagQueueIsFull 0x100
    typedef struct {
        DataTypeHeader header;
        mtx_t          mutex;
        size_t         queueSize;
        size_t         currentEnqueueIndex;
        size_t         currentDequeueIndex;
        size_t         elementSize;
        void*          queue;
    } Queue;

    typedef enum {
        QueueOperationSuccess = 0,
        QueueQueueWasFull,
        QueueQueueWasEmpty,
        QueueInvalidSizeOfBuffer
    } QueueError_t;

    extern Queue        queueCreateStack(size_t size, size_t elementSize, ListTypes_t listType, bool elementsArePointers);

    extern Queue*       queueCreate(size_t size, size_t elementSize, ListTypes_t listType, bool elementsArePointers);

    extern Queue*       queueMoveStackToHeap(Queue queue);

    extern Queue        queueMoveStack(Queue* queue);

    extern Queue*       queueCopyStackToHeap(Queue* queue);

    extern Queue        queueCopyStack(Queue* queue);

    extern QueueError_t queueEnqueue(Queue* queue, const void* element, size_t elementSize);

    extern QueueError_t queueDequeue(Queue* queue, void* element, size_t elementSize);

    extern void         queueClearOut(Queue* queue, void(operation)(void* element, ListTypes_t listType), void(elementDestructor)(void* element),uint32_t line,const char* file);

    extern size_t queueGetAmountOfElements(const Queue* queue);
    extern const void* queuePeak(const Queue* queue, size_t offset);
    extern void         queueDestroy(Queue* queue, void(elementDestructor)(void* element),uint32_t line, const char* file);


#ifdef __cplusplus
}
#endif


#endif