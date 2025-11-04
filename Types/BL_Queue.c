#include "BL_Queue.h"
#include "BL_UnorderedContainer.h"
#include "TypesMain.h"

#include <assert.h>
#include <stdalign.h>
#include <stdlib.h>
#include <string.h>

typedef struct QueueElement {
    size_t next;
    alignas(max_align_t) BL_Byte element[];
} QueueElement;

typedef struct DequeueElement {
    size_t next;
    size_t prev;
    alignas(max_align_t) BL_Byte element[];
} DequeueElement;


static void internal_queue_init(BL_Queue* queue, size_t initialSize, size_t elementSize, bool isDequeue) {
    size_t correctedElementSize = sizeof(size_t) + (isDequeue ? sizeof(size_t) : 0 + elementSize) + (alignof(size_t) - elementSize % alignof(size_t));
    queue->unorderedContainer   = bl_unordered_container_create_stack(initialSize, correctedElementSize);
    queue->tailIndex            = 0;
    queue->headIndex            = 0;
    queue->unorderedContainer.container.header |= isDequeue ? FlagQueueIsDequeue : 0;
}

BL_Queue bl_queue_create_stack(size_t initialSize, size_t elementSize) {
    BL_Queue returnQueue;
    internal_queue_init(&returnQueue, initialSize, elementSize, false);
    return returnQueue;
}

BL_Dequeue bl_dequeue_create_stack(size_t initialSize, size_t elementSize) {
    BL_Dequeue dequeue;
    internal_queue_init(&dequeue, initialSize, elementSize, true);
    return dequeue;
}

static BL_Queue* internal_queue_create_heap(size_t initialSize, size_t elementSize, bool isDequeue) {
    BL_Queue* returnQueue = malloc(sizeof(*returnQueue));
    if (returnQueue == NULL)
        return NULL;
    internal_queue_init(returnQueue, initialSize, elementSize, isDequeue);
    if (bl_queue_is_valid(returnQueue)) {
        free(returnQueue);
        return NULL;
    }
    returnQueue->unorderedContainer.container.header |= ObjectFlagIsOnHeap;
    return returnQueue;
}

BL_Queue* bl_queue_create_heap(size_t initialSize, size_t elementSize) {
    return internal_queue_create_heap(initialSize, elementSize, false);
}

BL_Dequeue* bl_dequeue_create_heap(size_t initialSize, size_t elementSize) {
    return internal_queue_create_heap(initialSize, elementSize, true);
}

BL_ContainerError bl_dequeue_enqueue_front(BL_Dequeue* dequeue, size_t elementSize, const void* element) {
    if (elementSize > dequeue->unorderedContainer.container.byteSizeOfSingleElement - sizeof(DequeueElement))
        return BL_ContainerInvalidSize;

    DequeueElement              elementToInsert = {.next = dequeue->headIndex, .prev = dequeue->tailIndex};
    BL_UnorderedContainerPutResult result          = bl_unordered_container_put(&dequeue->unorderedContainer, sizeof(elementToInsert), &elementToInsert);
    if (result.resultCode != BL_ContainerOPSuccessful)
        return result.resultCode;

    if (bl_unordered_container_size(&dequeue->unorderedContainer) == 1) {
        dequeue->headIndex                 = result.locationOfElement;
        dequeue->tailIndex                 = result.locationOfElement;
        DequeueElement* elementInContainer = bl_unordered_container_get(&dequeue->unorderedContainer, result.locationOfElement);
        elementInContainer->next           = result.locationOfElement;
        elementInContainer->prev           = result.locationOfElement;
        memcpy(&elementInContainer->element, element, elementSize);
        return BL_ContainerOPSuccessful;
    }

    ((DequeueElement*) bl_unordered_container_get(&dequeue->unorderedContainer, dequeue->tailIndex))->next = result.locationOfElement;
    ((DequeueElement*) bl_unordered_container_get(&dequeue->unorderedContainer, dequeue->headIndex))->prev = result.locationOfElement;
    dequeue->headIndex                                                                                         = result.locationOfElement;
    memcpy(&((DequeueElement*) bl_unordered_container_get(&dequeue->unorderedContainer,result.locationOfElement))->element,element,elementSize);
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_dequeue_enqueue_back(BL_Dequeue* dequeue, size_t elementSize, const void* element) {
    if (elementSize > dequeue->unorderedContainer.container.byteSizeOfSingleElement - sizeof(DequeueElement))
        return BL_ContainerInvalidSize;

    DequeueElement              elementToInsert = {.next = dequeue->headIndex, .prev = dequeue->tailIndex};
    BL_UnorderedContainerPutResult result          = bl_unordered_container_put( &dequeue->unorderedContainer, sizeof(elementToInsert), &elementToInsert);
    if (result.resultCode != BL_ContainerOPSuccessful)
        return result.resultCode;

    if (bl_unordered_container_size(&dequeue->unorderedContainer) == 1) {
        dequeue->headIndex                 = result.locationOfElement;
        dequeue->tailIndex                 = result.locationOfElement;
        DequeueElement* elementInContainer = bl_unordered_container_get(&dequeue->unorderedContainer, result.locationOfElement);
        elementInContainer->next           = result.locationOfElement;
        elementInContainer->prev           = result.locationOfElement;
        memcpy(&elementInContainer->element, element, elementSize);
        return BL_ContainerOPSuccessful;
    }

    ((DequeueElement*) bl_unordered_container_get(&dequeue->unorderedContainer, dequeue->tailIndex))->next = result.locationOfElement;
    ((DequeueElement*) bl_unordered_container_get(&dequeue->unorderedContainer, dequeue->headIndex))->prev = result.locationOfElement;
    dequeue->tailIndex                                                                                         = result.locationOfElement;
    memcpy(&((DequeueElement*) bl_unordered_container_get(&dequeue->unorderedContainer, result.locationOfElement))->element,
           element,
           elementSize);
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_queue_enqueue(BL_Queue* queue, size_t elementSize, const void* element) {
    if (elementSize > queue->unorderedContainer.container.byteSizeOfSingleElement - sizeof(QueueElement))
        return BL_ContainerInvalidSize;

    QueueElement                elementToInsert = {.next = queue->headIndex};
    BL_UnorderedContainerPutResult result          = bl_unordered_container_put(&queue->unorderedContainer, sizeof(elementToInsert), &elementToInsert);
    if (result.resultCode != BL_ContainerOPSuccessful)
        return result.resultCode;

    if (bl_unordered_container_size(&queue->unorderedContainer) == 1) {
        queue->headIndex                 = result.locationOfElement;
        queue->tailIndex                 = result.locationOfElement;

        QueueElement* elementInContainer = bl_unordered_container_get(&queue->unorderedContainer, result.locationOfElement);
        elementInContainer->next         = result.locationOfElement;
        memcpy(&elementInContainer->element, element, elementSize);
        return BL_ContainerOPSuccessful;
    }

    ((QueueElement*) bl_unordered_container_get(&queue->unorderedContainer, queue->tailIndex))->next = result.locationOfElement;
    queue->tailIndex                                                                                     = result.locationOfElement;
    memcpy(&((QueueElement*) bl_unordered_container_get(&queue->unorderedContainer, result.locationOfElement))->element,
           element,
           elementSize);
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_queue_dequeue(BL_Queue* queue, size_t elementSize, void* element) {
    if (bl_unordered_container_size(&queue->unorderedContainer) == 0)
        return BL_ContainerOPUnsuccessful;
    if (elementSize > queue->unorderedContainer.container.byteSizeOfSingleElement - sizeof(QueueElement))
        return BL_ContainerInvalidSize;

    size_t        currentHeadIndex = queue->headIndex;
    QueueElement* headElement      = bl_unordered_container_get(&queue->unorderedContainer, queue->headIndex);
    memcpy(element, headElement->element, elementSize);
    queue->headIndex = headElement->next;
    bl_unordered_container_remove(&queue->unorderedContainer, currentHeadIndex, NULL);
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_dequeue_dequeue_front(BL_Dequeue* dequeue, size_t elementSize, void* element) {
    if (bl_unordered_container_size(&dequeue->unorderedContainer) == 0)
        return BL_ContainerOPUnsuccessful;
    if (elementSize > dequeue->unorderedContainer.container.byteSizeOfSingleElement - sizeof(DequeueElement))
        return BL_ContainerInvalidSize;

    DequeueElement* headElement = bl_unordered_container_get(&dequeue->unorderedContainer, dequeue->headIndex);
    memcpy(element, &headElement->element, elementSize);
    ((DequeueElement*) bl_unordered_container_get(&dequeue->unorderedContainer, headElement->next))->prev = headElement->prev;
    ((DequeueElement*) bl_unordered_container_get(&dequeue->unorderedContainer, headElement->prev))->next = headElement->next;
    size_t elementToRemoveIndex                                                                               = dequeue->headIndex;
    dequeue->headIndex                                                                                        = headElement->next;
    bl_unordered_container_remove(&dequeue->unorderedContainer, elementToRemoveIndex, NULL);
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_dequeue_dequeue_back(BL_Dequeue* dequeue, size_t elementSize, void* element) {
    if (bl_unordered_container_size(&dequeue->unorderedContainer) == 0)
        return BL_ContainerOPUnsuccessful;

    if (elementSize > dequeue->unorderedContainer.container.byteSizeOfSingleElement - sizeof(DequeueElement))
        return BL_ContainerInvalidSize;

    DequeueElement* tailElement = bl_unordered_container_get(&dequeue->unorderedContainer, dequeue->tailIndex);
    memcpy(element, &tailElement->element, elementSize);
    ((DequeueElement*) bl_unordered_container_get(&dequeue->unorderedContainer, tailElement->next))->prev = tailElement->prev;
    ((DequeueElement*) bl_unordered_container_get(&dequeue->unorderedContainer, tailElement->prev))->next = tailElement->next;
    size_t elementToRemoveIndex                                                                               = dequeue->tailIndex;
    dequeue->tailIndex                                                                                        = tailElement->prev;
    bl_unordered_container_remove(&dequeue->unorderedContainer, elementToRemoveIndex, NULL);
    return BL_ContainerOPSuccessful;
}

inline const void* bl_dequeue_peak_front(const BL_Dequeue* dequeue, size_t offset) {
    return bl_queue_peak_front(dequeue, offset);
}

const void* bl_queue_peak_front(const BL_Queue* queue, size_t offset) {
    if (offset >= bl_unordered_container_size(&queue->unorderedContainer))
        return NULL;
    size_t        nextIndex    = queue->headIndex;
    QueueElement* queueElement = bl_unordered_container_get(&queue->unorderedContainer, nextIndex);
    for (size_t i = 0; i < offset; i++) {
        if (!queueElement)
            return NULL;
        nextIndex    = queueElement->next;
        queueElement = bl_unordered_container_get(&queue->unorderedContainer, nextIndex);
    }
    return (queue->unorderedContainer.container.header & FlagQueueIsDequeue) ? ((DequeueElement*) queueElement)->element : queueElement->element;
}

const void* bl_dequeue_peak_back(const BL_Dequeue* queue, size_t offset) {
    if (offset >= bl_unordered_container_size(&queue->unorderedContainer))
        return NULL;

    size_t          nextIndex    = queue->tailIndex;
    DequeueElement* queueElement = bl_unordered_container_get(&queue->unorderedContainer, nextIndex);
    for (size_t i = 0; i < offset; i++) {
        nextIndex    = queueElement->prev;
        queueElement = bl_unordered_container_get(&queue->unorderedContainer, nextIndex);
    }
    return queueElement->element;
}

const void* bl_queue_peak_back(const BL_Queue* queue) {
    QueueElement* queueElement = bl_unordered_container_get(&queue->unorderedContainer, queue->tailIndex);
    if (queueElement)
        return queueElement->element;
    return NULL;
}

void bl_queue_destroy(BL_Queue* queue) {
    bl_unordered_container_destroy(&queue->unorderedContainer);
}

void bl_dequeue_destroy(BL_Dequeue* dequeue) {
    bl_unordered_container_destroy(&dequeue->unorderedContainer);
}

bool bl_queue_is_valid(const BL_Queue* queue) {
    return bl_unordered_container_is_valid(&queue->unorderedContainer) && !(queue->unorderedContainer.container.header & FlagQueueIsDequeue);
}

bool bl_dequeue_is_valid(const BL_Dequeue* dequeue) {
    return  bl_unordered_container_is_valid(&dequeue->unorderedContainer) && dequeue->unorderedContainer.container.header & FlagQueueIsDequeue;
}