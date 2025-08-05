#include "Queue.h"
#include "TypesMain.h"
#include "UnorderedContainer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct QueueElement {
    size_t next;
    Byte   element[];
} QueueElement;

typedef struct DequeueElement {
    size_t next;
    size_t prev;
    Byte   element[];
} DequeueElement;


static void internal_queueInit(Queue* queue, size_t initialSize, size_t elementSize, bool isDequeue) {
    size_t correctedElementSize = sizeof(size_t) + isDequeue ? sizeof(size_t) : 0 + elementSize + (alignof(size_t) - elementSize % alignof(size_t));
    queue->unorderedContainer   = unorderedContainerCreateStack(initialSize, correctedElementSize, false);
    queue->tailIndex            = 0;
    queue->headIndex            = 0;
    queue->header |= isDequeue ? FlagQueueIsDequeue : 0;
}

Queue queueCreateStack(size_t initialSize, size_t elementSize, bool isDequeue) {
    Queue returnQueue;
    internal_queueInit(&returnQueue, initialSize, elementSize, isDequeue);
    return returnQueue;
}

Queue* queueCreateHeap(size_t initialSize, size_t elementSize, bool isDequeue) {
    Queue* returnQueue = malloc(sizeof(*returnQueue));
    if (returnQueue == NULL)
        return NULL;
    internal_queueInit(returnQueue, initialSize, elementSize,isDequeue);
    if (!isValidObject((DataTypeFlags*)returnQueue)) {
        free(returnQueue);
        return NULL;
    }
    returnQueue->header |= ObjectFlagIsOnHeap;
    return returnQueue;
}

ContainerError queueEnqueueFront(Queue* queue,size_t elementSize, const void* element) {
    if (queue->header & FlagQueueIsDequeue)
        ;
    else
        return ContainerOPUnsuccessful;

    if (elementSize > queue->container.byteSizeOfSingleElement - ((queue->header & FlagQueueIsDequeue) ? sizeof(DequeueElement) : sizeof(QueueElement)))
        return ContainerInvalidSize;

    DequeueElement elementToInsert = {.next = queue->headIndex, .prev = queue->tailIndex};
    UnorderedContainerPutResult result = unorderedContainerPut((UnorderedContainer*)queue, sizeof(elementToInsert), &elementToInsert);
    if (result.resultCode != ContainerOPSuccessful)
        return result.resultCode;

    if (queue->container.amountOfIndexes == 1) {
        queue->headIndex = result.locationOfElement;
        queue->tailIndex = result.locationOfElement;
        DequeueElement* elementInContainer = unorderedContainerGet((UnorderedContainer*)queue,result.locationOfElement).element;
        elementInContainer->next = result.locationOfElement;
        elementInContainer->prev = result.locationOfElement;
        memcpy(&elementInContainer->element,element,elementSize);
        return ContainerOPSuccessful;
    }

    ((DequeueElement*)unorderedContainerGet((UnorderedContainer*)queue,queue->tailIndex).element)->next = result.locationOfElement;
    ((DequeueElement*)unorderedContainerGet((UnorderedContainer*)queue,queue->headIndex).element)->prev = result.locationOfElement;
    queue->headIndex = result.locationOfElement;
    memcpy(&((DequeueElement*)unorderedContainerGet((UnorderedContainer*)queue,result.locationOfElement).element)->element,
        element,
        elementSize);
    return ContainerOPSuccessful;
}

ContainerError queueEnqueueBack(Queue* queue, size_t elementSize, const void* element) {
    if (elementSize > queue->container.byteSizeOfSingleElement - ((queue->header & FlagQueueIsDequeue) ? sizeof(DequeueElement) : sizeof(QueueElement)))
        return ContainerInvalidSize;

    if (queue->header & FlagQueueIsDequeue) {
        DequeueElement elementToInsert = {.next = queue->headIndex, .prev = queue->tailIndex};
        UnorderedContainerPutResult result = unorderedContainerPut((UnorderedContainer*)queue, sizeof(elementToInsert), &elementToInsert);
        if (result.resultCode != ContainerOPSuccessful)
            return result.resultCode;

        if (queue->container.amountOfIndexes == 1) {
            queue->headIndex = result.locationOfElement;
            queue->tailIndex = result.locationOfElement;
            DequeueElement* elementInContainer = unorderedContainerGet((UnorderedContainer*)queue,result.locationOfElement).element;
            elementInContainer->next = result.locationOfElement;
            elementInContainer->prev = result.locationOfElement;
            memcpy(&elementInContainer->element,element,elementSize);
            return ContainerOPSuccessful;
        }

        ((DequeueElement*)unorderedContainerGet((UnorderedContainer*)queue,queue->tailIndex).element)->next = result.locationOfElement;
        ((DequeueElement*)unorderedContainerGet((UnorderedContainer*)queue,queue->headIndex).element)->prev = result.locationOfElement;
        queue->tailIndex = result.locationOfElement;
        memcpy(&((DequeueElement*)unorderedContainerGet((UnorderedContainer*)queue,result.locationOfElement).element)->element,
            element,
            elementSize);
        return ContainerOPSuccessful;
    }
    QueueElement elementToInsert = {.next = queue->headIndex};
    UnorderedContainerPutResult result = unorderedContainerPut((UnorderedContainer*)queue, sizeof(elementToInsert), &elementToInsert);
    if (result.resultCode != ContainerOPSuccessful)
        return result.resultCode;

    if (queue->container.amountOfIndexes == 1) {
        queue->headIndex = result.locationOfElement;
        queue->tailIndex = result.locationOfElement;

        QueueElement* elementInContainer = unorderedContainerGet((UnorderedContainer*)queue,result.locationOfElement).element;
        elementInContainer->next = result.locationOfElement;
        memcpy(&elementInContainer->element,element,elementSize);
        return ContainerOPSuccessful;
    }

    ((QueueElement*)unorderedContainerGet((UnorderedContainer*)queue,queue->tailIndex).element)->next = result.locationOfElement;
    queue->tailIndex = result.locationOfElement;
    memcpy(&((QueueElement*)unorderedContainerGet((UnorderedContainer*)queue,result.locationOfElement).element)->element,
        element,
        elementSize);
    return ContainerOPSuccessful;
}

ContainerError queueDequeueFront(Queue* queue, size_t elementSize, void* element) {
    if (queue->container.amountOfIndexes == 0)
        return ContainerOPUnsuccessful;
    if (elementSize > queue->container.byteSizeOfSingleElement - ((queue->header & FlagQueueIsDequeue) ? sizeof(DequeueElement) : sizeof(QueueElement)))
        return ContainerInvalidSize;

    if (queue->header & FlagQueueIsDequeue) {
        DequeueElement* headElement = unorderedContainerGet((UnorderedContainer*)queue,queue->headIndex).element;
        memcpy(element,&headElement->element,elementSize);
        ((DequeueElement*)unorderedContainerGet((UnorderedContainer*)queue,headElement->next).element)->prev = headElement->prev;
        ((DequeueElement*)unorderedContainerGet((UnorderedContainer*)queue,headElement->prev).element)->next = headElement->next;
        size_t elementToRemoveIndex = queue->headIndex;
        queue->headIndex = headElement->next;
        unorderedContainerRemove((UnorderedContainer*)queue,elementToRemoveIndex,NULL);
        return ContainerOPSuccessful;
    }

    size_t currentHeadIndex = queue->headIndex;
    QueueElement* headElement = unorderedContainerGet((UnorderedContainer*)queue,queue->headIndex).element;
    memcpy(element,headElement->element,elementSize);
    queue->headIndex = headElement->next;
    unorderedContainerRemove((UnorderedContainer*)queue,currentHeadIndex,NULL);
    return ContainerOPSuccessful;
}

ContainerError queueDequeueBack(Queue* queue, size_t elementSize, void* element) {
    if (queue->header & FlagQueueIsDequeue)
        ;
    else
        return ContainerOPUnsuccessful;

    if (elementSize > queue->container.byteSizeOfSingleElement - sizeof(DequeueElement))
        return ContainerInvalidSize;

    DequeueElement* tailElement = unorderedContainerGet((UnorderedContainer*)queue,queue->tailIndex).element;
    memcpy(element,&tailElement->element,elementSize);
    ((DequeueElement*)unorderedContainerGet((UnorderedContainer*)queue,tailElement->next).element)->prev = tailElement->prev;
    ((DequeueElement*)unorderedContainerGet((UnorderedContainer*)queue,tailElement->prev).element)->next = tailElement->next;
    size_t elementToRemoveIndex = queue->tailIndex;
    queue->tailIndex = tailElement->prev;
    unorderedContainerRemove((UnorderedContainer*)queue,elementToRemoveIndex,NULL);
    return ContainerOPSuccessful;

}
const void* queuePeakFront(const Queue* queue, size_t offset) {
    if (offset > queue->container.amountOfIndexes)
        return NULL;
    size_t nextIndex = queue->headIndex;
    QueueElement* queueElement = unorderedContainerGet((UnorderedContainer*)queue,nextIndex).element;
    for (size_t i = 0; i < offset; i++) {
        nextIndex = queueElement->next;
        queueElement = unorderedContainerGet((UnorderedContainer*)queue,nextIndex).element;
    }
    return (const void*)((uintptr_t)queueElement + (queue->header & FlagQueueIsDequeue) ? sizeof(DequeueElement) : sizeof(QueueElement));
}

const void* queuePeakBack(const Queue* queue, size_t offset) {
    if (offset > queue->container.amountOfIndexes)
        return NULL;
    if (offset == 0 && (queue->header & FlagQueueIsDequeue) == 0) {
        return (const void*)((uintptr_t)unorderedContainerGet((UnorderedContainer*)queue,queue->tailIndex).element + sizeof(QueueElement));
    }
    if ((queue->header & FlagQueueIsDequeue) == 0)
        return NULL;

    size_t nextIndex = queue->tailIndex;
    DequeueElement* queueElement = unorderedContainerGet((UnorderedContainer*)queue,nextIndex).element;
    for (size_t i = 0; i < offset; i++) {
        nextIndex = queueElement->prev;
        queueElement = unorderedContainerGet((UnorderedContainer*)queue,nextIndex).element;
    }
    return (const void*)((uintptr_t)queueElement + sizeof(DequeueElement));
}

void queueDestroy(Queue* queue, void(*destructor)(void* element)) {
    if (destructor && isValidObject((DataTypeFlags*)queue))
        unorderedContainerDestroyWithElements((UnorderedContainer*)queue,destructor);
    else
        unorderedContainerDestroy(queue);
}