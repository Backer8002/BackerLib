#include "ThreadPool.h"
#include <BackerLibTypes.h>
#include <stdlib.h>

struct JobInformation {
    size_t priority;

    void (*function)(void*);
};

static bool jobCompare(const void* first, const void* second) {
    return (*(struct JobInformation**) first)->priority <= (*(struct JobInformation**) second)->priority;
}

static int threadWorkerFunction(void* sharedState) {
    while (!((ThreadPool*) sharedState)->mustExit) {
        if (mutexLock(&((ThreadPool*) sharedState)->mutexForQueue) == ConcurrencyFailure)
            continue;
        struct JobInformation* const * nextOrder = bl_heap_top(&((ThreadPool*) sharedState)->jobsQueue);
        struct JobInformation*         order     = nextOrder ? *nextOrder : NULL;
        bl_heap_pop(&((ThreadPool*) sharedState)->jobsQueue);
        mutexUnlock(&((ThreadPool*) sharedState)->mutexForQueue);

        if (order)
            order->function(order);
    }
    return 0;
}

static void threadExitWrapper(void* sharedState) { threadExit(); }

ConcurrencyError bl_threadpool_init(ThreadPool* threadPool, size_t amountOfThreads, size_t sizeOfLargestAsyncArgsPack) {
    threadPool->mustExit  = false;
    threadPool->jobsQueue = bl_heap_create_stack(sizeof(void*), jobCompare);
    if (bl_heap_is_valid(&threadPool->jobsQueue))
        return ConcurrencyFailure;

    threadPool->orders = bl_unordered_container_create_stack(16, sizeOfLargestAsyncArgsPack);

    if (!bl_unordered_container_is_valid(&threadPool->orders)) {
        bl_heap_destroy(&threadPool->jobsQueue);
        return ConcurrencyFailure;
    }

    threadPool->mutexForQueue = mutexCreate(MutexPlain);
    if (!mutexIsValid(&threadPool->mutexForQueue)) {
        bl_unordered_container_destroy(&threadPool->orders);
        bl_heap_destroy(&threadPool->jobsQueue);
        return ConcurrencyFailure;
    }

    threadPool->threads = malloc(sizeof *threadPool->threads * amountOfThreads);
    if (!threadPool->threads) {
        mutexDestroy(&threadPool->mutexForQueue);
        bl_unordered_container_destroy(&threadPool->orders);
        bl_heap_destroy(&threadPool->jobsQueue);
        return ConcurrencyFailure;
    }

    size_t iterator = 0;

    for (; iterator < amountOfThreads; iterator++) {
        threadPool->threads[iterator] = threadCreate(threadPool, threadWorkerFunction);
        if (!threadIsValid(&threadPool->threads[iterator]))
            goto ThreadPoolInitErrorPath;
    }
    threadPool->amountOfThreads = amountOfThreads;
    return ConcurrencySuccess;

ThreadPoolInitErrorPath:
    threadPool->mustExit = true;

    for (size_t i = 0; i < iterator; i++)
        threadJoin(&threadPool->threads[i]);
    free(threadPool->threads);
    mutexDestroy(&threadPool->mutexForQueue);
    bl_unordered_container_destroy(&threadPool->orders);
    bl_heap_destroy(&threadPool->jobsQueue);
    return ConcurrencyFailure;
}

void* bl_threadpool_job_assign(ThreadPool* threadPool,
                          size_t      priority,
                          void (*     function)(void*),
                          size_t      futureOffset,
                          void*       args, size_t argsSize, size_t argsOffset) {
    if (priority == 0 || priority == SIZE_MAX
        || argsOffset + argsSize > threadPool->orders.container.byteSizeOfSingleElement)
        return NULL;
    struct JobInformation jobInfo = (struct JobInformation){.priority = priority, .function = function};

    if (mutexLock(&threadPool->mutexForQueue) == ConcurrencyFailure)
        return NULL;

    BL_UnorderedContainerPutResult putResult = bl_unordered_container_put(&threadPool->orders, sizeof jobInfo, &jobInfo);

    if (putResult.resultCode != BL_ContainerOPSuccessful) {
        mutexUnlock(&threadPool->mutexForQueue);
        return NULL;
    }

    void* locationOfElement = bl_unordered_container_get(&threadPool->orders, putResult.locationOfElement);

    *(FutureVoid*) ((BL_Bytes) locationOfElement + futureOffset) = false;

    memcpy((BL_Bytes) locationOfElement + argsOffset, args, argsSize);

    BL_ContainerError errorCode = bl_heap_insert(&threadPool->jobsQueue, sizeof locationOfElement, &locationOfElement);

    mutexUnlock(&threadPool->mutexForQueue);

    if (errorCode != BL_ContainerOPSuccessful)
        return NULL;
    return (BL_Bytes) locationOfElement + futureOffset;
}

ConcurrencyError bl_threadpool_join(ThreadPool* threadPool) {
    struct JobInformation exitCommand = {.priority = 0, .function = threadExitWrapper};
    struct JobInformation* exitPointer = &exitCommand;

    if (mutexLock(&threadPool->mutexForQueue) == ConcurrencyFailure)
        return ConcurrencyFailure;

    if (bl_container_dynamic_reserve(&threadPool->jobsQueue.dynamicContainer, threadPool->amountOfThreads) == BL_ContainerAllocFailure) {
        mutexUnlock(&threadPool->mutexForQueue);
        return ConcurrencyFailure;
    }

    for (size_t i = 0; i < threadPool->amountOfThreads; i++)
        bl_heap_insert(&threadPool->jobsQueue, sizeof(void*), &exitPointer);

    if (mutexUnlock(&threadPool->mutexForQueue) == ConcurrencyFailure)
        return ConcurrencyFailure;

    for (size_t i = 0; i < threadPool->amountOfThreads; i++)
        threadJoin(&threadPool->threads[i]);

    free(threadPool->threads);
    mutexDestroy(&threadPool->mutexForQueue);
    bl_unordered_container_destroy(&threadPool->orders);
    bl_heap_destroy(&threadPool->jobsQueue);
    return ConcurrencySuccess;
}

void bl_threadpool_exit(ThreadPool* threadPool) {
    threadPool->mustExit = true;

    for (size_t i = 0; i < threadPool->amountOfThreads; i++)
        threadJoin(&threadPool->threads[i]);
    free(threadPool->threads);
    mutexDestroy(&threadPool->mutexForQueue);
    bl_unordered_container_destroy(&threadPool->orders);
    bl_heap_destroy(&threadPool->jobsQueue);
}

bool bl_future_destroy(ThreadPool* threadPool, void* future) {
    if (!*(FutureVoid*) future)
        return false;

    bl_unordered_container_remove(&threadPool->orders,
                             ((uintptr_t) future - (uintptr_t) threadPool->orders.container.array) / threadPool->orders.container.byteSizeOfSingleElement,
                             NULL);
    return true;
}

static void asyncFileReadMain(void* sharedState) {
    AsyncFileReadArg* args = sharedState;

    rewind(args->args);
    fseek(args->args, 0,SEEK_END);
    size_t fileSize = ftell(args->args);
    rewind(args->args);

    args->future.future = bl_container_dynamic_create_stack(fileSize, sizeof(char));
    if (!bl_container_dynamic_is_valid(&args->future.future)) {
        args->future.isValid = true;
        return;
    }

    args->future.future.container.amountOfIndexes = fread(args->future.future.container.array,
                                                          sizeof(char),
                                                          fileSize,
                                                          args->args);
    args->future.isValid = true;
}

FutureString* bl_async_file_read(ThreadPool* threadPool, size_t priority, FILE* file) {
    return bl_threadpool_job_assign(threadPool,
                               priority,
                               asyncFileReadMain,
                               asyncArgsFutureOffset(AsyncFileReadArg),
                               &file, sizeof file,asyncArgsOffset(AsyncFileReadArg));
}

bool bl_threadpool_is_valid(const ThreadPool* threadPool) {
    return bl_heap_is_valid(&threadPool->jobsQueue);
}