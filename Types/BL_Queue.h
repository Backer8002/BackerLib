#ifndef Queue_h_
#define Queue_h_

#include "TypesMain.h"
#include <stdbool.h>
#include <stdint.h>
#include "BL_UnorderedContainer.h"

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#else
#define noexcept
#endif

#define FlagQueueIsDequeue 0x100
    typedef struct BL_Queue {
        BL_UnorderedContainer unorderedContainer;
        size_t             tailIndex;
        size_t             headIndex;
    } BL_Queue;


    typedef BL_Queue             BL_Dequeue;

    /**
     * @brief Creates a Queue object on stack. Use is_valid to check validity.
     * @param initialSize Initial amount of space for elements in Queue
     * @param elementSize Size of largest element to be inserted
     * @return Queue object on stack
     */
    extern BL_Queue              bl_queue_create_stack(size_t initialSize, size_t elementSize) noexcept;
    /**
     * @brief Creates a Queue object on heap.
     * @param initialSize Initial amount of space for elements in Queue
     * @param elementSize Size of largest element to be inserted
     * @return NULL if allocation failed
     */
    extern BL_Queue*             bl_queue_create_heap(size_t initialSize, size_t elementSize) noexcept;
    /**
     * @brief Inserts element at the back of the queue.
     * @param queue Pointer to valid Queue
     * @param elementSize Size of element to insert
     * @param element Element to insert
     * @return ContainerAllocFailure if queue could not grow to accommodate more elements.
     */
    extern BL_ContainerError     bl_queue_enqueue(BL_Queue* queue, size_t elementSize, const void* element) noexcept;
    /**
     * @brief Copies front element to element and removes it from the queue.
     * @param queue Pointer to valid Queue
     * @param elementSize Size of element
     * @param element Buffer to store queue element in
     * @return ContainerOPUnsuccessful if queue was empty.
     * @return ContainerInvalidSize if buffer is larger than element.
     */
    extern BL_ContainerError     bl_queue_dequeue(BL_Queue* queue, size_t elementSize, void* element) noexcept;
    /**
     * @brief Returns pointer to offset element from head element. 0 is head element.
     * @param queue Pointer to valid Queue
     * @param offset Offset from head element
     * @return NULL if offset is results in an out-of-bounds access.
     */
    extern const void*        bl_queue_peak_front(const BL_Queue* queue, size_t offset) noexcept;
    /**
     * @brief Returns pointer to the element last inserted. If no element exists in queue it returns NULL.
     * @param queue Pointer to valid Queue
     * @return NULL if there is no element in queue.
     */
    extern const void*        bl_queue_peak_back(const BL_Queue* queue) noexcept;
    /**
     *
     * @param queue Pointer to Queue or NULL
     * @return true if queue is valid, else false
     */
    extern bool bl_queue_is_valid(const BL_Queue* queue) noexcept;

    /**
     * @brief Destroys Queue.
     * @param queue Pointer to Queue
     */
    extern void               bl_queue_destroy(BL_Queue* queue) noexcept;

    /**
     * @brief Creates a Dequeue object on stack. Use is_valid to check validity.
     * @param initialSize Initial amount of space for elements in Dequeue
     * @param elementSize Size of largest element to be inserted
     * @return Dequeue object on stack
     */
    extern BL_Dequeue            bl_dequeue_create_stack(size_t initialSize, size_t elementSize) noexcept;
    /**
     * @brief Creates a Dequeue object on heap.
     * @param initialSize Initial amount of space for elements in Dequeue
     * @param elementSize Size of largest element to be inserted
     * @return NULL if allocation failed
     */
    extern BL_Dequeue*           bl_dequeue_create_heap(size_t initialSize, size_t elementSize) noexcept;

    /**
     * @brief Inserts element at the front of the dequeue.
     * @param dequeue Pointer to valid Dequeue
     * @param elementSize Size of element to insert
     * @param element Element to insert
     * @return ContainerInvalidSize if element was larger than the size of a single element in the dequeue.
     * @return ContainerAllocFailure if queue could not grow to accommodate more elements.
     */
    extern BL_ContainerError     bl_dequeue_enqueue_front(BL_Dequeue* dequeue, size_t elementSize, const void* element) noexcept;
    /**
     * @brief Inserts element at the back of the dequeue.
     * @param dequeue Pointer to valid Dequeue
     * @param elementSize Size of element to insert
     * @param element Element to insert
     * @return ContainerInvalidSize if element was larger than the size of a single element in the dequeue.
     * @return ContainerAllocFailure if queue could not grow to accommodate more elements.
     */
    extern BL_ContainerError     bl_dequeue_enqueue_back(BL_Dequeue* dequeue, size_t elementSize, const void* element) noexcept;

    /**
     * @brief Copies front element to element and removes it from the dequeue.
     * @param dequeue Pointer to valid Dequeue
     * @param elementSize Size of element
     * @param element Buffer to store dequeue element in
     * @return ContainerOPUnsuccessful if Dequeue was empty.
     * @return ContainerInvalidSize if buffer is larger than element.
     */
    extern BL_ContainerError     bl_dequeue_dequeue_front(BL_Dequeue* dequeue, size_t elementSize, void* element) noexcept;

    /**
     * @brief Copies tail element to element and removes it from the dequeue.
     * @param dequeue Pointer to valid Dequeue
     * @param elementSize Size of element
     * @param element Buffer to store queue element in
     * @return ContainerOPUnsuccessful if Dequeue is empty.
     * @return ContainerInvalidSize if buffer is larger than element.
     */
    extern BL_ContainerError     bl_dequeue_dequeue_back(BL_Dequeue* dequeue, size_t elementSize, void* element) noexcept;
    /**
     * @brief Returns pointer to offset element from head element. 0 is head element.
     * @param dequeue Pointer to valid Dequeue
     * @param offset Offset from head element
     * @return NULL if offset is results in an out-of-bounds access.
     */
    extern const void* bl_dequeue_peak_front(const BL_Dequeue* dequeue, size_t offset) noexcept;

    /**
     * @brief Returns pointer to offset element from tail element backwards in queue. 0 is tail element.
     * @param queue Pointer to valid Dequeue
     * @param offset Offset from tail element
     * @return NULL if offset is results in an out-of-bounds access.
     */
    extern const void*        bl_dequeue_peak_back(const BL_Dequeue* queue, size_t offset) noexcept;
    /**
     *
     * @param dequeue Pointer to dequeue or NULL
     * @return true if dequeue is valid, else false.
     */
    extern bool bl_dequeue_is_valid(const BL_Queue* dequeue) noexcept;
    /**
     * @brief Destroys Dequeue.
     * @param dequeue Pointer to Dequeue
     */
    extern void               bl_dequeue_destroy(BL_Dequeue* dequeue) noexcept;

#ifdef __cplusplus
    }
};
#else
#undef noexcept
#endif
#endif
