#pragma once

#ifdef DLL
#ifdef BASICFUNCTIONS_EXPORTS
#define QUEUE __declspec(dllexport);
#else
#define	QUEUE __declspec(dllimport);
#endif
#else
#define QUEUE
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include<threads.h>
#include<stdbool.h>
#include<stdint.h>
#include<backerLibListsTypes.h>

	typedef struct {
		mtx_t mutex;
		bool elementsArePointers;
		bool isHeapAlloced;
		bool mutexExists;
		bool queueIsFull;
		ListTypes_t listType;
		size_t queueSize;
		size_t currentEnqueueIndex;
		size_t currentDequeueIndex;
		size_t elementSize;
		void* queue;
	} Queue;

	typedef enum {
		ENQUEUE_SUCCSESS = 0,
		ENQUEUE_QUEUE_FULL,
		ENQUEUE_UNKNOWN_FAILURE,
		DEQUEUE_SUCCESS = 0,
		DEQUEUE_QUEUE_EMPTY,
		DEQUEUE_UNKOWN_FAILURE
} QueueError_t;

	Queue queueCreateStack(size_t size, size_t elementSize, ListTypes_t listType, bool elementsArePointers);

	Queue* queueCreate(size_t size, size_t elementSize, ListTypes_t listType, bool elementsArePointers);

	Queue* queueMoveStackToHeap(Queue queue, bool destroyInputOnFailiure);

	Queue queueMoveStack(Queue* queue);

	Queue* queueListCopyStackToHeap(Queue* queue);

	Queue queueListCopyStack(Queue* queue);

	QueueError_t queueEnqueue(Queue* queue, void* element, size_t elementSize);

	QueueError_t queueDequeue(Queue* queue, void* element, size_t elementSize);

	void queueClearOut(Queue* queue, void(operation)(void* element, ListTypes_t listType), void(elementDestructor)(void* element));

	void queueDestroy(Queue* queue, void(elementDestructor)(void* element));

#ifdef __cplusplus
}
#endif


