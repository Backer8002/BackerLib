#include"pch.h"
#include<queue.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<threads.h>

#undef queueEnqueue
#undef queueDequeue

Queue queueCreateStack(size_t size, size_t elementSize, ListTypes_t listType, bool elementsArePointers) {
    Queue returnQueue;

    returnQueue.currentDequeueIndex     = 0;
    returnQueue.currentEnqueueIndex     = 0;
    returnQueue.header.flags            = (elementsArePointers) ? ObjectFlagContentsIsPointers : 0;
    returnQueue.elementSize             = elementSize;
    returnQueue.header.dataArrayVarType = listType;
    returnQueue.header.objectType      = ListQueue;
    returnQueue.queueSize               = size;

    returnQueue.queue = malloc(elementSize*size);
    if (returnQueue.queue == NULL) {
        returnQueue.header.dataArrayVarType = ListNone;
        return returnQueue;
    }
    if (mtx_init(&returnQueue.mutex, mtx_plain) == thrd_success)
        returnQueue.header.flags |= ObjectFlagMutexExists;
    return returnQueue;
}

Queue* queueCreate(size_t size, size_t elementSize, ListTypes_t listType, bool elementsArePointers) {
    Queue* returnQueue = malloc(sizeof(Queue));
    if (returnQueue == NULL)
        return NULL;

    returnQueue->header.flags            = ((elementsArePointers) ? ObjectFlagContentsIsPointers : 0) | ObjectFlagIsOnHeap;
    returnQueue->currentDequeueIndex     = 0;
    returnQueue->currentEnqueueIndex     = 0;
    returnQueue->elementSize             = elementSize;
    returnQueue->header.dataArrayVarType = listType;
    returnQueue->header.objectType       = ListQueue;
    returnQueue->queueSize               = size;

    returnQueue->queue = malloc(elementSize * size);
    if (returnQueue->queue == NULL) {
        free(returnQueue);
        return NULL;
    }
    if (mtx_init(&returnQueue->mutex, mtx_plain) == thrd_success)
        returnQueue->header.flags |= ObjectFlagMutexExists;
    return returnQueue;
}

Queue* queueMoveStackToHeap(Queue queue, bool destroyInputOnFailiure) {
    if (queue.header.flags & ObjectFlagIsOnHeap || queue.header.dataArrayVarType == ListNone)
        return NULL;
    Queue* queueNew = malloc(sizeof(Queue));
    if (queueNew == NULL) {
        if (destroyInputOnFailiure)
            queueDestroy(&queue,NULL);
        return NULL;
    }
    *queueNew = queue;
    queueNew->header.flags |= ObjectFlagIsOnHeap;
    return queueNew;
}

Queue queueMoveStack(Queue* queue) {
    Queue queueNew = *queue;
    queueNew.header.flags &= ~ObjectFlagIsOnHeap;
    if (queue->header.flags & ObjectFlagIsOnHeap)
        free(queue);
    return queueNew;
}

Queue* queueListCopyStackToHeap(Queue* queue) {
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

Queue queueListCopyStack(Queue* queue) {
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
    queue->currentDequeueIndex = queue->currentDequeueIndex;
    queue->currentEnqueueIndex = queue->currentEnqueueIndex;
    if (queue->header.flags & ObjectFlagMutexExists)
        mtx_unlock(&queue->mutex);
    queueNew.header.flags |= queue->header.flags & FlagQueueIsFull;
    return queueNew;
}

QueueError_t queueEnqueue(Queue* queue, void* element, size_t elementSize) {
    if (queue->header.flags & FlagQueueIsFull)
        return ENQUEUE_QUEUE_FULL;
    assert(elementSize == queue->elementSize);
    for (size_t i = 0; i < elementSize; i++)
        *((unsigned char*) queue->queue + elementSize * queue->currentEnqueueIndex + i) = *((unsigned char*) element + i);
    queue->currentEnqueueIndex = (queue->currentEnqueueIndex + 1) % queue->queueSize;
    if (queue->currentDequeueIndex == queue->currentEnqueueIndex)
        queue->header.flags |= FlagQueueIsFull;
    return ENQUEUE_SUCCSESS;
}

QueueError_t queueDequeue(Queue* queue, void* element, size_t elementSize) {
    if (!(queue->header.flags & FlagQueueIsFull) && queue->currentDequeueIndex == queue->currentEnqueueIndex)
        return DEQUEUE_QUEUE_EMPTY;
    assert(elementSize = queue->elementSize);

    for (size_t i = 0; i < elementSize; i++)
        *((unsigned char*) element + i) = *((unsigned char*) queue->queue + elementSize * queue->currentDequeueIndex + i);

    queue->currentDequeueIndex++;
    if (queue->currentDequeueIndex == queue->queueSize)
        queue->currentDequeueIndex = 0;

    queue->header.flags &= ~FlagQueueIsFull;
    return DEQUEUE_SUCCESS;
}

void queueClearOut(Queue* queue, void(operation)(void* element,ListTypes_t listType), void(elementDestructor)(void* element)) {
    if (queue->header.flags & ObjectFlagMutexExists)
        mtx_lock(&queue->mutex);

    if ((queue->currentDequeueIndex != queue->currentEnqueueIndex || queue->header.flags & FlagQueueIsFull)) {
        for (size_t currentIndex = queue->currentDequeueIndex; currentIndex != queue->currentEnqueueIndex; currentIndex = (currentIndex + 1) % queue->queueSize) {
            operation(
                (queue->header.flags & ObjectFlagContentsIsPointers) 
                    ? *(unsigned char**) queue->queue + currentIndex * queue->elementSize 
                    : (unsigned char*) queue->queue + currentIndex * queue->elementSize, 
                queue->header.dataArrayVarType);
            if (elementDestructor != NULL)
                elementDestructor(
                    (queue->header.flags & ObjectFlagContentsIsPointers)
                        ? *(unsigned char**) queue->queue + currentIndex * queue->elementSize
                        : (unsigned char*) queue->queue + currentIndex * queue->elementSize);
        }
        queue->header.flags &= ~FlagQueueIsFull;
        queue->currentDequeueIndex = 0;
        queue->currentEnqueueIndex = 0;
    }

    if (queue->header.flags & ObjectFlagMutexExists)
        mtx_unlock(&queue->mutex);
}

void queueDestroy(Queue* queue, void(elementDestructor)(void* element)) {
    if ((queue->currentDequeueIndex != queue->currentEnqueueIndex || queue->header.flags & FlagQueueIsFull)&&elementDestructor != NULL) {
        for (size_t currentIndex = queue->currentDequeueIndex; currentIndex != queue->currentEnqueueIndex; currentIndex = (currentIndex + 1) % queue->queueSize)
            elementDestructor((queue->header.flags & ObjectFlagContentsIsPointers) ? *(unsigned char**) queue->queue + currentIndex * queue->elementSize : (unsigned char*) queue->queue + currentIndex * queue->elementSize);
    }
    if (queue->queue != NULL)
        free(queue->queue);
    if (queue->header.flags & ObjectFlagMutexExists)
        mtx_destroy(&queue->mutex);
    if (queue->header.flags & ObjectFlagIsOnHeap)
        free(queue);
}