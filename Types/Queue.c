#include "Queue.h"
#include "TypesMain.h"
#include "UnorderedContainer.h"

#include <assert.h>
#include <stdalign.h>
#include <stdlib.h>
#include <string.h>

typedef struct QueueElement {
    size_t next;
    alignas(max_align_t) Byte element[];
} QueueElement;

typedef struct DequeueElement {
    size_t next;
    size_t prev;
    alignas(max_align_t) Byte element[];
} DequeueElement;


static inline void internal_queueInit(Queue* queue, size_t initialSize, size_t elementSize, bool isDequeue) {
    size_t correctedElementSize = sizeof(size_t) + (isDequeue ? sizeof(size_t) : 0 + elementSize) + (alignof(size_t) - elementSize % alignof(size_t));
    queue->unorderedContainer   = unorderedContainerCreateStack(initialSize, correctedElementSize, false);
    queue->tailIndex            = 0;
    queue->headIndex            = 0;
    queue->header |= isDequeue ? FlagQueueIsDequeue : 0;
}

Queue queueCreateStack(size_t initialSize, size_t elementSize) {
    Queue returnQueue;
    internal_queueInit(&returnQueue, initialSize, elementSize, false);
    return returnQueue;
}

Dequeue dequeueCreateStack(size_t initialSize, size_t elementSize) {
    Dequeue dequeue;
    internal_queueInit(&dequeue, initialSize, elementSize, true);
    return dequeue;
}

static inline Queue* internal_queueCreateHeap(size_t initialSize, size_t elementSize, bool isDequeue) {
    Queue* returnQueue = malloc(sizeof(*returnQueue));
    if (returnQueue == NULL)
        return NULL;
    internal_queueInit(returnQueue, initialSize, elementSize, isDequeue);
    if (!isValidObject((DataTypeFlags*) returnQueue)) {
        free(returnQueue);
        return NULL;
    }
    returnQueue->header |= ObjectFlagIsOnHeap;
    return returnQueue;
}

Queue* queueCreateHeap(size_t initialSize, size_t elementSize) {
    return internal_queueCreateHeap(initialSize, elementSize, false);
}

Dequeue* dequeueCreateHeap(size_t initialSize, size_t elementSize) {
    return internal_queueCreateHeap(initialSize, elementSize, true);
}

ContainerError dequeueEnqueueFront(Dequeue* dequeue, size_t elementSize, const void* element) {
    if (elementSize > dequeue->container.byteSizeOfSingleElement - sizeof(DequeueElement))
        return ContainerInvalidSize;

    DequeueElement              elementToInsert = {.next = dequeue->headIndex, .prev = dequeue->tailIndex};
    UnorderedContainerPutResult result          = unorderedContainerPut((UnorderedContainer*) dequeue, sizeof(elementToInsert), &elementToInsert);
    if (result.resultCode != ContainerOPSuccessful)
        return result.resultCode;

    if (dequeue->container.amountOfIndexes == 1) {
        dequeue->headIndex                 = result.locationOfElement;
        dequeue->tailIndex                 = result.locationOfElement;
        DequeueElement* elementInContainer = unorderedContainerGet((UnorderedContainer*) dequeue, result.locationOfElement).element;
        elementInContainer->next           = result.locationOfElement;
        elementInContainer->prev           = result.locationOfElement;
        memcpy(&elementInContainer->element, element, elementSize);
        return ContainerOPSuccessful;
    }

    ((DequeueElement*) unorderedContainerGet((UnorderedContainer*) dequeue, dequeue->tailIndex).element)->next = result.locationOfElement;
    ((DequeueElement*) unorderedContainerGet((UnorderedContainer*) dequeue, dequeue->headIndex).element)->prev = result.locationOfElement;
    dequeue->headIndex                                                                                         = result.locationOfElement;
    memcpy(&((DequeueElement*) unorderedContainerGet((UnorderedContainer*) dequeue,
                                                     result.locationOfElement)
                 .element)
                ->element,
           element,
           elementSize);
    return ContainerOPSuccessful;
}

ContainerError dequeueEnqueueBack(Dequeue* dequeue, size_t elementSize, const void* element) {
    if (elementSize > dequeue->container.byteSizeOfSingleElement - sizeof(DequeueElement))
        return ContainerInvalidSize;

    DequeueElement              elementToInsert = {.next = dequeue->headIndex, .prev = dequeue->tailIndex};
    UnorderedContainerPutResult result          = unorderedContainerPut((UnorderedContainer*) dequeue, sizeof(elementToInsert), &elementToInsert);
    if (result.resultCode != ContainerOPSuccessful)
        return result.resultCode;

    if (dequeue->container.amountOfIndexes == 1) {
        dequeue->headIndex                 = result.locationOfElement;
        dequeue->tailIndex                 = result.locationOfElement;
        DequeueElement* elementInContainer = unorderedContainerGet((UnorderedContainer*) dequeue, result.locationOfElement).element;
        elementInContainer->next           = result.locationOfElement;
        elementInContainer->prev           = result.locationOfElement;
        memcpy(&elementInContainer->element, element, elementSize);
        return ContainerOPSuccessful;
    }

    ((DequeueElement*) unorderedContainerGet((UnorderedContainer*) dequeue, dequeue->tailIndex).element)->next = result.locationOfElement;
    ((DequeueElement*) unorderedContainerGet((UnorderedContainer*) dequeue, dequeue->headIndex).element)->prev = result.locationOfElement;
    dequeue->tailIndex                                                                                         = result.locationOfElement;
    memcpy(&((DequeueElement*) unorderedContainerGet((UnorderedContainer*) dequeue, result.locationOfElement).element)->element,
           element,
           elementSize);
    return ContainerOPSuccessful;
}

ContainerError queueEnqueue(Queue* queue, size_t elementSize, const void* element) {
    if (elementSize > queue->container.byteSizeOfSingleElement - sizeof(QueueElement))
        return ContainerInvalidSize;

    QueueElement                elementToInsert = {.next = queue->headIndex};
    UnorderedContainerPutResult result          = unorderedContainerPut((UnorderedContainer*) queue, sizeof(elementToInsert), &elementToInsert);
    if (result.resultCode != ContainerOPSuccessful)
        return result.resultCode;

    if (queue->container.amountOfIndexes == 1) {
        queue->headIndex                 = result.locationOfElement;
        queue->tailIndex                 = result.locationOfElement;

        QueueElement* elementInContainer = unorderedContainerGet((UnorderedContainer*) queue, result.locationOfElement).element;
        elementInContainer->next         = result.locationOfElement;
        memcpy(&elementInContainer->element, element, elementSize);
        return ContainerOPSuccessful;
    }

    ((QueueElement*) unorderedContainerGet((UnorderedContainer*) queue, queue->tailIndex).element)->next = result.locationOfElement;
    queue->tailIndex                                                                                     = result.locationOfElement;
    memcpy(&((QueueElement*) unorderedContainerGet((UnorderedContainer*) queue, result.locationOfElement).element)->element,
           element,
           elementSize);
    return ContainerOPSuccessful;
}

ContainerError queueDequeue(Queue* queue, size_t elementSize, void* element) {
    if (queue->container.amountOfIndexes == 0)
        return ContainerOPUnsuccessful;
    if (elementSize > queue->container.byteSizeOfSingleElement - sizeof(QueueElement))
        return ContainerInvalidSize;

    size_t        currentHeadIndex = queue->headIndex;
    QueueElement* headElement      = unorderedContainerGet((UnorderedContainer*) queue, queue->headIndex).element;
    memcpy(element, headElement->element, elementSize);
    queue->headIndex = headElement->next;
    unorderedContainerRemove((UnorderedContainer*) queue, currentHeadIndex, NULL);
    return ContainerOPSuccessful;
}

ContainerError dequeueDequeueFront(Dequeue* dequeue, size_t elementSize, void* element) {
    if (dequeue->container.amountOfIndexes == 0)
        return ContainerOPUnsuccessful;
    if (elementSize > dequeue->container.byteSizeOfSingleElement - sizeof(DequeueElement))
        return ContainerInvalidSize;

    DequeueElement* headElement = unorderedContainerGet((UnorderedContainer*) dequeue, dequeue->headIndex).element;
    memcpy(element, &headElement->element, elementSize);
    ((DequeueElement*) unorderedContainerGet((UnorderedContainer*) dequeue, headElement->next).element)->prev = headElement->prev;
    ((DequeueElement*) unorderedContainerGet((UnorderedContainer*) dequeue, headElement->prev).element)->next = headElement->next;
    size_t elementToRemoveIndex                                                                               = dequeue->headIndex;
    dequeue->headIndex                                                                                        = headElement->next;
    unorderedContainerRemove((UnorderedContainer*) dequeue, elementToRemoveIndex, NULL);
    return ContainerOPSuccessful;
}

ContainerError dequeueDequeueBack(Dequeue* dequeue, size_t elementSize, void* element) {
    if (dequeue->container.amountOfIndexes == 0)
        return ContainerOPUnsuccessful;

    if (elementSize > dequeue->container.byteSizeOfSingleElement - sizeof(DequeueElement))
        return ContainerInvalidSize;

    DequeueElement* tailElement = unorderedContainerGet((UnorderedContainer*) dequeue, dequeue->tailIndex).element;
    memcpy(element, &tailElement->element, elementSize);
    ((DequeueElement*) unorderedContainerGet((UnorderedContainer*) dequeue, tailElement->next).element)->prev = tailElement->prev;
    ((DequeueElement*) unorderedContainerGet((UnorderedContainer*) dequeue, tailElement->prev).element)->next = tailElement->next;
    size_t elementToRemoveIndex                                                                               = dequeue->tailIndex;
    dequeue->tailIndex                                                                                        = tailElement->prev;
    unorderedContainerRemove((UnorderedContainer*) dequeue, elementToRemoveIndex, NULL);
    return ContainerOPSuccessful;
}

inline const void* dequeuePeakFront(const Dequeue* dequeue, size_t offset) {
    return queuePeakFront(dequeue, offset);
}

const void* queuePeakFront(const Queue* queue, size_t offset) {
    if (offset >= queue->container.amountOfIndexes)
        return NULL;
    size_t        nextIndex    = queue->headIndex;
    QueueElement* queueElement = unorderedContainerGet((UnorderedContainer*) queue, nextIndex).element;
    for (size_t i = 0; i < offset; i++) {
        if (!queueElement)
            return NULL;
        nextIndex    = queueElement->next;
        queueElement = unorderedContainerGet((UnorderedContainer*) queue, nextIndex).element;
    }
    return (queue->header & FlagQueueIsDequeue) ? ((DequeueElement*) queueElement)->element : queueElement->element;
}

const void* dequeuePeakBack(const Dequeue* queue, size_t offset) {
    if (offset >= queue->container.amountOfIndexes)
        return NULL;

    size_t          nextIndex    = queue->tailIndex;
    DequeueElement* queueElement = unorderedContainerGet((UnorderedContainer*) queue, nextIndex).element;
    for (size_t i = 0; i < offset; i++) {
        nextIndex    = queueElement->prev;
        queueElement = unorderedContainerGet((UnorderedContainer*) queue, nextIndex).element;
    }
    return queueElement->element;
}

const void* queuePeakBack(const Queue* queue) {
    QueueElement* queueElement = unorderedContainerGet(&queue->unorderedContainer, queue->tailIndex).element;
    if (queueElement)
        return queueElement->element;
    return NULL;
}

void queueDestroy(Queue* queue) {
    unorderedContainerDestroy(queue);
}

void dequeueDestroy(Dequeue* dequeue) {
    unorderedContainerDestroy(dequeue);
}
