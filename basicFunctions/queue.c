#include"pch.h"
#include<queue.h>
#include<stdlib.h>
#include<assert.h>
#include<threads.h>

Queue queueCreateStack(size_t size, size_t elementSize, ListTypes_t listType, bool elementsArePointers) {
    Queue returnQueue;

    returnQueue.currentDequeueIndex = 0;
    returnQueue.currentEnqueueIndex = 0;
    returnQueue.elementsArePointers = elementsArePointers;
    returnQueue.elementSize = elementSize;
    returnQueue.isHeapAlloced = false;
    returnQueue.listType = listType;
    returnQueue.queueSize = size;
    returnQueue.queueIsFull = false;

    returnQueue.queue = malloc(elementSize*size);
    if (returnQueue.queue == NULL) {
        returnQueue.listType = ListNone;
        return returnQueue;
    }
    if (mtx_init(&returnQueue.mutex, mtx_plain) == thrd_success)
        returnQueue.mutexExists = true;
    else
        returnQueue.mutexExists = false;
    return returnQueue;
}

Queue* queueCreate(size_t size, size_t elementSize, ListTypes_t listType, bool elementsArePointers) {
    Queue* returnQueue = malloc(sizeof(Queue));
    if (returnQueue == NULL)
        return NULL;

    returnQueue->currentDequeueIndex = 0;
    returnQueue->currentEnqueueIndex = 0;
    returnQueue->elementsArePointers = elementsArePointers;
    returnQueue->elementSize = elementSize;
    returnQueue->isHeapAlloced = false;
    returnQueue->listType = listType;
    returnQueue->queueSize = size;
    returnQueue->queueIsFull = false;

    returnQueue->queue = malloc(elementSize * size);
    if (returnQueue->queue == NULL) {
        free(returnQueue);
        return NULL;
    }
    if (mtx_init(&returnQueue->mutex, mtx_plain) == thrd_success)
        returnQueue->mutexExists = true;
    else
        returnQueue->mutexExists = false;
    return returnQueue;
}

Queue* queueMoveStackToHeap(Queue queue, bool destroyInputOnFailiure) {
    if (queue.isHeapAlloced || queue.queue == NULL)
        return NULL;
    Queue* queueNew = malloc(sizeof(Queue));
    if (queueNew == NULL) {
        if (destroyInputOnFailiure)
            queueDestroy(&queue,NULL);
        return NULL;
    }
    *queueNew = queue;
    queueNew->isHeapAlloced = true;
    return queueNew;
}

Queue queueMoveStack(Queue* queue) {
    Queue queueNew = *queue;
    queueNew.isHeapAlloced = false;
    if (queue->isHeapAlloced)
        free(queue);
    return queueNew;
}

Queue* queueListCopyStackToHeap(Queue* queue) {
    if (queue->mutexExists)
        mtx_lock(&queue->mutex);
    if (queue->isHeapAlloced || queue->queue == NULL) {
        if (queue->mutexExists)
            mtx_unlock(&queue->mutex);
        return NULL;
    }
    Queue* queueNew = queueCreate(queue->queueSize, queue->elementSize, queue->listType, queue->elementsArePointers);
    if (queueNew == NULL) {
        if (queue->mutexExists)
            mtx_unlock(&queue->mutex);
        return NULL;
    }
    memcpy_s(queueNew->queue,
        queueNew->queueSize * queueNew->elementSize,
        queue->queue,
        queue->queueSize * queue->elementSize);
    queueNew->currentDequeueIndex = queue->currentDequeueIndex;
    queueNew->currentEnqueueIndex = queue->currentEnqueueIndex;
    if (mtx_init(&queueNew->mutex, mtx_plain) == thrd_success)
        queueNew->mutexExists = true;
    else
        queueNew->mutexExists = false;
    if (queue->mutexExists)
        mtx_unlock(&queue->mutex);
    queueNew->queueIsFull = (queue->queueIsFull) ? true : false;
    return queueNew;
}

Queue queueListCopyStack(Queue* queue) {
    if (queue->mutexExists)
        mtx_lock(&queue->mutex);
    Queue queueNew = queueCreateStack(queue->queueSize, queue->elementSize, queue->listType, queue->elementsArePointers);
    if (queueNew.queue == NULL) {
        if (queue->mutexExists)
            mtx_unlock(&queue->mutex);
        return queueNew;
    }

    memcpy_s(queueNew.queue,
        queueNew.queueSize * queueNew.elementSize,
        queue->queue,
        queue->queueSize * queue->elementSize);
    queue->currentDequeueIndex = queue->currentDequeueIndex;
    queue->currentEnqueueIndex = queue->currentEnqueueIndex;
    if (queue->mutexExists)
        mtx_unlock(&queue->mutex);
    queueNew.queueIsFull = (queue->queueIsFull) ? true : false;
    return queueNew;
}

QueueError_t queueEnqueue(Queue* queue, void* element, size_t elementSize) {
    if (queue->queueIsFull)
        return ENQUEUE_QUEUE_FULL;
    assert(elementSize == queue->elementSize);
    for (size_t i = 0; i < elementSize; i++)
        *((unsigned char*) queue->queue + elementSize * queue->currentEnqueueIndex + i) = *((unsigned char*) element + i);
    queue->currentEnqueueIndex = (queue->currentEnqueueIndex + 1) % queue->queueSize;
    if (queue->currentDequeueIndex == queue->currentEnqueueIndex)
        queue->queueIsFull = true;
    return ENQUEUE_SUCCSESS;
}

QueueError_t queueDequeue(Queue* queue, void* element, size_t elementSize) {
    if (!queue->queueIsFull && queue->currentDequeueIndex == queue->currentEnqueueIndex)
        return DEQUEUE_QUEUE_EMPTY;
    assert(elementSize = queue->elementSize);

    for (size_t i = 0; i < elementSize; i++)
        *((unsigned char*) element + i) = *((unsigned char*) queue->queue + elementSize * queue->currentDequeueIndex + i);

    queue->currentDequeueIndex++;
    if (queue->currentDequeueIndex == queue->queueSize)
        queue->currentDequeueIndex = 0;

    queue->queueIsFull = false;
    return DEQUEUE_SUCCESS;
}

void queueClearOut(Queue* queue, void(operation)(void* element,ListTypes_t listType), void(elementDestructor)(void* element)) {
    if (queue->mutexExists)
        mtx_lock(&queue->mutex);
    if ((queue->currentDequeueIndex != queue->currentEnqueueIndex || queue->queueIsFull)) {
        for (size_t currentIndex = queue->currentDequeueIndex; currentIndex != queue->currentEnqueueIndex; currentIndex = (currentIndex + 1) % queue->queueSize) {
            operation((queue->elementsArePointers) ? *(unsigned char**) queue->queue + currentIndex * queue->elementSize : (unsigned char*) queue->queue + currentIndex * queue->elementSize, 
                queue->listType);
            if (elementDestructor != NULL)
                elementDestructor((queue->elementsArePointers) ? *(unsigned char**) queue->queue + currentIndex * queue->elementSize : (unsigned char*) queue->queue + currentIndex * queue->elementSize);
        }
        queue->queueIsFull = false;
        queue->currentDequeueIndex = 0;
        queue->currentEnqueueIndex = 0;
    }
    if (queue->mutexExists)
        mtx_unlock(&queue->mutex);
}

void queueDestroy(Queue* queue, void(elementDestructor)(void* element)) {
    if ((queue->currentDequeueIndex != queue->currentEnqueueIndex || queue->queueIsFull)&&elementDestructor != NULL) {
        for (size_t currentIndex = queue->currentDequeueIndex; currentIndex != queue->currentEnqueueIndex; currentIndex = (currentIndex + 1) % queue->queueSize)
            elementDestructor((queue->elementsArePointers) ? *(unsigned char**) queue->queue + currentIndex * queue->elementSize : (unsigned char*) queue->queue + currentIndex * queue->elementSize);
    }
    if (queue->queue != NULL)
        free(queue->queue);
    if (queue->mutexExists)
        mtx_destroy(&queue->mutex);
    if (queue->isHeapAlloced)
        free(queue);
}