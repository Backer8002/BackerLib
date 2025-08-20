#ifndef Queue_h_
#define Queue_h_

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#endif

#include "TypesMain.h"
#include <stdbool.h>
#include <stdint.h>

#define FlagQueueIsDequeue 0x100

    typedef union Queue {
        DataTypeFlags header;
        Container     container;
        struct {
            UnorderedContainer unorderedContainer;
            size_t             headIndex;
            size_t             tailIndex;
        };
    } Queue;

    /**
     * @brief Creates a queue object on stack. Use isValidObject to check validity.
     * @param initialSize Initial amount of space for elements in queue/dequeue
     * @param elementSize Size of largest element to be inserted
     * @param isDequeue Should this be a dequeue (double linked list instead of singel linked list)
     * @return Queue object on stack
     */
    extern Queue          queueCreateStack(size_t initialSize, size_t elementSize, bool isDequeue);
    /**
     * @brief Creates a queue object on heap.
     * @param initialSize Initial amount of space for elements in queue/dequeue
     * @param elementSize Size of largest element to be inserted
     * @param isDequeue Should this be a dequeue (double linked list instead of singel linked list)
     * @return NULL if allocation failed
     */
    extern Queue*         queueCreateHeap(size_t initialSize, size_t elementSize, bool isDequeue);
    /**
     * @brief Inserts element at the front of the queue.
     * @param queue Pointer to valid Queue with isDequeue set
     * @param elementSize Size of element to insert
     * @param element Element to insert
     * @return ContainerOPUnsuccessful if queue is not a dequeue.
     * @return ContainerInvalidSize if element was larger than the size of a singel element in the queue.
     * @return ContainerAllocFailure if queue could not grow to accommodate more elements.
     */
    extern ContainerError queueEnqueueFront(Queue* queue, size_t elementSize, const void* element);
    /**
     * @brief Inserts element at the back of the queue.
     * @param queue Pointer to valid Queue
     * @param elementSize Size of element to insert
     * @param element Element to insert
     * @return ContainerInvalidSize if element was larger than the size of a singel element in the queue.
     * @return ContainerAllocFailure if queue could not grow to accommodate more elements.
     */
    extern ContainerError queueEnqueueBack(Queue* queue, size_t elementSize, const void* element);
    /**
     * @brief Copies front element to element and removes it from the queue.
     * @param queue Pointer to valid Queue
     * @param elementSize Size of element
     * @param element Buffer to store queue element in
     * @return ContainerInvalidSize if buffer is larger than element.
     */
    extern ContainerError queueDequeueFront(Queue* queue, size_t elementSize, void* element);
    /**
     * @brief Copies back element to element and removes it from the queue.
     * @param queue Pointer to valid Queue with isDequeue set
     * @param elementSize Size of element
     * @param element Buffer to store queue element in
     * @return ContainerOPUnsuccessful if Queue does not have isDequeue set.
     * @return ContainerInvalidSize if buffer is larger than element.
     */
    extern ContainerError queueDequeueBack(Queue* queue, size_t elementSize, void* element);
    /**
     * @brief Returns pointer to offset element from head element. 0 is head element.
     * @param queue Pointer to valid Queue
     * @param offset Offset from head element
     * @return NULL if offset is results in an out-of-bounds access.
     */
    extern const void*    queuePeakFront(const Queue* queue, size_t offset);
    /**
     * @brief Returns pointer to offset element from tail element backwards in queue. 0 is tail element. Must have isDequeue set for offset larger than 0 to be valid.
     * @param queue Pointer to valid Queue
     * @param offset Offset from tail element
     * @return NULL if offset is results in an out-of-bounds access or if offset is larger than 0 and isDequeue is not set.
     */
    extern const void*    queuePeakBack(const Queue* queue, size_t offset);
    /**
     * @brief Destroys Queue with optional destructor.
     * @param queue Pointer to Queue
     * @param destructor Optional pointer to function matching argument
     */
    extern void           queueDestroy(Queue* queue, void (*destructor)(void* element));


#ifdef __cplusplus
    }
};
#endif
#endif
