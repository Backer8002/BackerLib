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
        struct {
            DataTypeFlags header;
        };
        struct {
            Container container;
        };
        struct {
            UnorderedContainer unorderedContainer;
            size_t             headIndex;
            size_t             tailIndex;
        };
    } Queue;

    typedef Queue             Dequeue;

    /**
     * @brief Creates a Queue object on stack. Use isValidObject to check validity.
     * @param initialSize Initial amount of space for elements in Queue
     * @param elementSize Size of largest element to be inserted
     * @return Queue object on stack
     */
    extern Queue              queueCreateStack(size_t initialSize, size_t elementSize);
    /**
     * @brief Creates a Queue object on heap.
     * @param initialSize Initial amount of space for elements in Queue
     * @param elementSize Size of largest element to be inserted
     * @return NULL if allocation failed
     */
    extern Queue*             queueCreateHeap(size_t initialSize, size_t elementSize);
    /**
     * @brief Inserts element at the back of the queue.
     * @param queue Pointer to valid Queue
     * @param elementSize Size of element to insert
     * @param element Element to insert
     * @return ContainerInvalidSize if element was larger than the size of a single element in the queue.
     * @return ContainerAllocFailure if queue could not grow to accommodate more elements.
     */
    extern ContainerError     queueEnqueue(Queue* queue, size_t elementSize, const void* element);
    /**
     * @brief Copies front element to element and removes it from the queue.
     * @param queue Pointer to valid Queue
     * @param elementSize Size of element
     * @param element Buffer to store queue element in
     * @return ContainerOPUnsuccessful if queue was empty.
     * @return ContainerInvalidSize if buffer is larger than element.
     */
    extern ContainerError     queueDequeue(Queue* queue, size_t elementSize, void* element);
    /**
     * @brief Returns pointer to offset element from head element. 0 is head element.
     * @param queue Pointer to valid Queue
     * @param offset Offset from head element
     * @return NULL if offset is results in an out-of-bounds access.
     */
    extern const void*        queuePeakFront(const Queue* queue, size_t offset);
    /**
     * @brief Returns pointer to the element last inserted. If no element exists in queue it returns NULL.
     * @param queue Pointer to valid Queue
     * @return NULL if there is no element in queue.
     */
    extern const void*        queuePeakBack(const Queue* queue);

    /**
     * @brief Destroys Queue.
     * @param queue Pointer to Queue
     */
    extern void               queueDestroy(Queue* queue);

    /**
     * @brief Creates a Dequeue object on stack. Use isValidObject to check validity.
     * @param initialSize Initial amount of space for elements in Dequeue
     * @param elementSize Size of largest element to be inserted
     * @return Dequeue object on stack
     */
    extern Dequeue            dequeueCreateStack(size_t initialSize, size_t elementSize);
    /**
     * @brief Creates a Dequeue object on heap.
     * @param initialSize Initial amount of space for elements in Dequeue
     * @param elementSize Size of largest element to be inserted
     * @return NULL if allocation failed
     */
    extern Dequeue*           dequeueCreateHeap(size_t initialSize, size_t elementSize);

    /**
     * @brief Inserts element at the front of the dequeue.
     * @param dequeue Pointer to valid Dequeue
     * @param elementSize Size of element to insert
     * @param element Element to insert
     * @return ContainerInvalidSize if element was larger than the size of a single element in the dequeue.
     * @return ContainerAllocFailure if queue could not grow to accommodate more elements.
     */
    extern ContainerError     dequeueEnqueueFront(Dequeue* dequeue, size_t elementSize, const void* element);
    /**
     * @brief Inserts element at the back of the dequeue.
     * @param dequeue Pointer to valid Dequeue
     * @param elementSize Size of element to insert
     * @param element Element to insert
     * @return ContainerInvalidSize if element was larger than the size of a single element in the dequeue.
     * @return ContainerAllocFailure if queue could not grow to accommodate more elements.
     */
    extern ContainerError     dequeueEnqueueBack(Dequeue* dequeue, size_t elementSize, const void* element);

    /**
     * @brief Copies front element to element and removes it from the dequeue.
     * @param dequeue Pointer to valid Dequeue
     * @param elementSize Size of element
     * @param element Buffer to store dequeue element in
     * @return ContainerOPUnsuccessful if Dequeue was empty.
     * @return ContainerInvalidSize if buffer is larger than element.
     */
    extern ContainerError     dequeueDequeueFront(Dequeue* dequeue, size_t elementSize, void* element);

    /**
     * @brief Copies tail element to element and removes it from the dequeue.
     * @param dequeue Pointer to valid Dequeue
     * @param elementSize Size of element
     * @param element Buffer to store queue element in
     * @return ContainerOPUnsuccessful if Dequeue is empty.
     * @return ContainerInvalidSize if buffer is larger than element.
     */
    extern ContainerError     dequeueDequeueBack(Dequeue* dequeue, size_t elementSize, void* element);
    /**
     * @brief Returns pointer to offset element from head element. 0 is head element.
     * @param dequeue Pointer to valid Dequeue
     * @param offset Offset from head element
     * @return NULL if offset is results in an out-of-bounds access.
     */
    extern inline const void* dequeuePeakFront(const Dequeue* dequeue, size_t offset);

    /**
     * @brief Returns pointer to offset element from tail element backwards in queue. 0 is tail element.
     * @param queue Pointer to valid Dequeue
     * @param offset Offset from tail element
     * @return NULL if offset is results in an out-of-bounds access.
     */
    extern const void*        dequeuePeakBack(const Dequeue* queue, size_t offset);
    /**
     * @brief Destroys Dequeue.
     * @param dequeue Pointer to Dequeue
     */
    extern void               dequeueDestroy(Dequeue* dequeue);

#ifdef __cplusplus
    }
};
#endif
#endif
