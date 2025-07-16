#include "queue.h"
#include "backerLibListTypes.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <BackerLibLogging.h>

#undef queueEnqueue
#undef queueDequeue
#undef queueClearOut
#undef queueDestroy

static bool internal_queueInit(Queue* queue,size_t size,size_t elementSize, ListTypes_t listType, bool elementsArePointers) {
    queue->currentDequeueIndex     = 0;
    queue->currentEnqueueIndex     = 0;
    queue->header.flags            = (elementsArePointers) ? ObjectFlagContentsIsPointers : 0;
    queue->elementSize             = elementsArePointers? sizeof(intptr_t):elementSize;
    queue->header.dataArrayVarType = listType;
    queue->header.objectType       = ListQueue;
    queue->queueSize               = size;

    queue->queue                   = malloc(elementSize * size);
    if (queue->queue == NULL) {
        queue->header.dataArrayVarType = ListNone;
        return false;
    }
    if (mtx_init(&queue->mutex, mtx_plain) == thrd_success)
        queue->header.flags |= ObjectFlagMutexExists;
    return true;
}

//Creates a queue object on the stack. If elements are pointers is specified the queue will not copy the data of element when enqueueing
Queue queueCreateStack(size_t size, size_t elementSize, ListTypes_t listType, bool elementsArePointers) {
    Queue returnQueue;
    if (!internal_queueInit(&returnQueue,size,elementSize,listType,elementsArePointers))
        returnQueue.header.dataArrayVarType = ListNone;
    return returnQueue;
}

//Creates a queue object on the heap. If elements are pointers is specified the queue will not copy the data of element when enqueueing
Queue* queueCreate(size_t size, size_t elementSize, ListTypes_t listType, bool elementsArePointers) {
    Queue* returnQueue = malloc(sizeof(Queue));
    if (returnQueue == NULL)
        return NULL;

    if (!internal_queueInit(returnQueue,size,elementSize,listType,elementsArePointers)) {
        free(returnQueue);
        return NULL;
    }
    return returnQueue;
}

//Moves a stack alloced queue object to a heap allocated object.
Queue* queueMoveStackToHeap(Queue queue) {
    if (queue.header.flags & ObjectFlagIsOnHeap || queue.header.dataArrayVarType == ListNone)
        return NULL;
    Queue* queueNew = malloc(sizeof(Queue));
    if (queueNew == NULL) {
        return NULL;
    }
    *queueNew = queue;
    queueNew->header.flags |= ObjectFlagIsOnHeap;
    return queueNew;
}

//Moves a queue object to a new stack one. If queue was on the heap it is now freed.
Queue queueMoveStack(Queue* queue) {
    Queue queueNew = *queue;
    queueNew.header.flags &= ~ObjectFlagIsOnHeap;
    if (queue->header.flags & ObjectFlagIsOnHeap)
        free(queue);
    return queueNew;
}

//Copies a stack alloced queue to a heap alloced one. Will lock the mutex.
Queue* queueCopyStackToHeap(Queue* queue) {
    if (queue->header.flags & ObjectFlagMutexExists)
        mtx_lock(&queue->mutex);
    if (queue->header.flags & ObjectFlagIsOnHeap || queue->queue == NULL) {
        if (queue->header.flags & ObjectFlagMutexExists)
            mtx_unlock(&queue->mutex);
        return NULL;
    }
    Queue* queueNew = queueCreate(queue->queueSize, queue->elementSize, queue->header.dataArrayVarType, queue->header.flags & ObjectFlagContentsIsPointers);
    if (queueNew == NULL) {
        if (queue->header.flags & ObjectFlagMutexExists)
            mtx_unlock(&queue->mutex);
        return NULL;
    }
    memcpy(queueNew->queue,
           queue->queue,
           queueNew->queueSize * queueNew->elementSize);
    queueNew->currentDequeueIndex = queue->currentDequeueIndex;
    queueNew->currentEnqueueIndex = queue->currentEnqueueIndex;
    if (mtx_init(&queueNew->mutex, mtx_plain) == thrd_success)
        queueNew->header.flags &= ObjectFlagMutexExists;

    if (queue->header.flags & ObjectFlagMutexExists)
        mtx_unlock(&queue->mutex);
    queueNew->header.flags |= queue->header.flags & FlagQueueIsFull;
    return queueNew;
}

//Copies a queue to a stack object. Will lock the mutex.
Queue queueCopyStack(Queue* queue) {
    if (queue->header.flags & ObjectFlagMutexExists)
        mtx_lock(&queue->mutex);
    Queue queueNew = queueCreateStack(queue->queueSize, queue->elementSize, queue->header.dataArrayVarType, queue->header.flags & ObjectFlagContentsIsPointers);
    if (queueNew.queue == NULL) {
        if (queue->header.flags & ObjectFlagMutexExists)
            mtx_unlock(&queue->mutex);
        return queueNew;
    }

    memcpy(queueNew.queue,
           queue->queue,
           queueNew.queueSize * queueNew.elementSize);
    queueNew.currentDequeueIndex = queue->currentDequeueIndex;
    queueNew.currentEnqueueIndex = queue->currentEnqueueIndex;
    queueNew.header.flags |= queue->header.flags & FlagQueueIsFull;
    if (queue->header.flags & ObjectFlagMutexExists)
        mtx_unlock(&queue->mutex);
    return queueNew;
}

/*Enqueues element to queue of size elementSize. If elementsArePointers were specifed the element is not copied to the queue but instead the pointer provided.


 Returns QueueInvalidSizeOfBuffer if elementSize was larger than the size of a singel queue element and elementsArePointers was not specified.

 Returns QueueQueueWasFull if queue was full.*/
QueueError_t queueEnqueue(Queue* queue, const void* element, size_t elementSize) {
    if (queue->header.flags & FlagQueueIsFull)
        return QueueQueueWasFull;

    if (queue->header.flags & ObjectFlagContentsIsPointers)
        ((void**)queue->queue)[queue->currentEnqueueIndex] = element;
    else {
        if (elementSize > queue->elementSize)
            return QueueInvalidSizeOfBuffer;
        for (size_t i = 0; i < elementSize; i++)
            *((Bytes) queue->queue + elementSize * queue->currentEnqueueIndex + i) = *((Bytes) element + i);
    }
    queue->currentEnqueueIndex = (queue->currentEnqueueIndex + 1) % queue->queueSize;
    if (queue->currentDequeueIndex == queue->currentEnqueueIndex)
        queue->header.flags |= FlagQueueIsFull;
    return QueueOperationSuccess;
}

/*Copies the value at the current dequeue index to element and removes it from the queue.

 Returns QueueInvalidSizeOfBuffer if element was larger than a single element in the queue.
 Returns QueueQueueWasEmpty if there was not element to dequeue*/
QueueError_t queueDequeue(Queue* queue, void* element, size_t elementSize) {
    if (!(queue->header.flags & FlagQueueIsFull) && queue->currentDequeueIndex == queue->currentEnqueueIndex)
        return QueueQueueWasEmpty;
    if (elementSize > queue->elementSize)
        return QueueInvalidSizeOfBuffer;

    for (size_t i = 0; i < elementSize; i++)
        *((Bytes) element + i) = *((Bytes) queue->queue + elementSize * queue->currentDequeueIndex + i);

    queue->currentDequeueIndex++;
    if (queue->currentDequeueIndex == queue->queueSize)
        queue->currentDequeueIndex = 0;

    queue->header.flags &= ~FlagQueueIsFull;
    return QueueOperationSuccess;
}

//Returns the amount of elements in the queue.
size_t queueGetAmountOfElements(const Queue* const queue) {
    if (queue->header.flags & FlagQueueIsFull)
        return queue->queueSize;
    return (queue->currentEnqueueIndex + queue->queueSize - queue->currentDequeueIndex) % queue->queueSize;
}

/*Returns a pointer to the queue element given offset from element to dequeue.
If elementsArePointers this returns the pointer in the array. If offset is out of range this returns NULL.
Pointer is not to be considered having ownership.*/
const void* queuePeak(const Queue* queue, size_t offset) {
    if (offset >= queueGetAmountOfElements(queue))
        return NULL;
    return (queue->header.flags & ObjectFlagContentsIsPointers)
                    ? ((void**)queue->queue)[(queue->currentDequeueIndex + offset) % queue->queueSize]
                    : (Bytes) queue->queue + ((offset + queue->currentDequeueIndex) % queue->queueSize) * queue->elementSize;
}

/*Preforms an operation on each element in the queue and removes them with elementDestructor.

elementDestructor should be provided if elementsArePointers, otherwise elementDestructor is optional.
Locks the mutex of queue if it exists.*/
void queueClearOut(Queue* queue, void(operation)(void* element, ListTypes_t listType), void(elementDestructor)(void* element),uint32_t line,const char* file) {
    if (queue->header.flags & ObjectFlagMutexExists)
        mtx_lock(&queue->mutex);

    if (queue->header.flags & ObjectFlagContentsIsPointers && !elementDestructor)
        logWarnDebugCall("Memory leak might have occurred. No destructor was passed in to function when one might have been needed.",line,file);

    if (!operation) {
        logErrorCall("No operation was provided. If destruction of all elements was the goal, consider passing the destructor as operation. Function will now return",line,file);
        return;
    }

    if ((queue->currentDequeueIndex != queue->currentEnqueueIndex || queue->header.flags & FlagQueueIsFull)) {
        size_t currentIndex = queue->currentDequeueIndex;
        do {
            operation(
                (queue->header.flags & ObjectFlagContentsIsPointers)
                    ? ((void**)queue->queue)[currentIndex]
                    : (Bytes) queue->queue + currentIndex * queue->elementSize,
                queue->header.dataArrayVarType);
            if (elementDestructor != NULL)
                elementDestructor(
                    (queue->header.flags & ObjectFlagContentsIsPointers)
                        ? ((void**)queue->queue)[currentIndex]
                        : (Bytes) queue->queue + currentIndex * queue->elementSize);
            currentIndex = (currentIndex + 1) % queue->queueSize;
        } while (currentIndex != queue->currentEnqueueIndex);

        queue->header.flags &= ~FlagQueueIsFull;
        queue->currentDequeueIndex = 0;
        queue->currentEnqueueIndex = 0;
    }


    if (queue->header.flags & ObjectFlagMutexExists)
        mtx_unlock(&queue->mutex);
}

//elementDestructor should be provided if elementsArePointers.
void queueDestroy(Queue* queue, void(elementDestructor)(void* element), uint32_t line, const char* file) {
    if (queue->header.flags & ObjectFlagContentsIsPointers && !elementDestructor)
        logWarnDebugCall("Memory leak might have occurred. No destructor was passed in to function when one might have been needed.",line,file);

    if ((queue->currentDequeueIndex != queue->currentEnqueueIndex || queue->header.flags & FlagQueueIsFull) && elementDestructor != NULL) {
        for (size_t currentIndex = queue->currentDequeueIndex; currentIndex != queue->currentEnqueueIndex; currentIndex = (currentIndex + 1) % queue->queueSize)
            elementDestructor((queue->header.flags & ObjectFlagContentsIsPointers) ? *(Bytes*) queue->queue + currentIndex * queue->elementSize : (Bytes) queue->queue + currentIndex * queue->elementSize);
    }
    if (queue->queue != NULL)
        free(queue->queue);
    if (queue->header.flags & ObjectFlagMutexExists)
        mtx_destroy(&queue->mutex);
    if (queue->header.flags & ObjectFlagIsOnHeap)
        free(queue);
}