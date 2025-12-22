#include "ThreadPool.h"

#include <BackerLibTypes.h>
#include <assert.h>
#include <stdlib.h>
#include <BackerLibLogging.h>

struct JobInformation {
    size_t priority;
    size_t location;
};

struct Job { void (*function)(void*); size_t futureOffset;};

static bool jobCompare(const void* first, const void* second) {
    return ((struct JobInformation*)first)->priority <= ((struct JobInformation*) second)->priority;
}

static int threadWorkerFunction(void* sharedState) {
    BL_ThreadPool* threadPool = sharedState;
    void* jobBuffer = malloc(threadPool->orders.byteSizeOfElement);
    if (!jobBuffer)
        return 1;
    bl_log_debug_location("Finished starting thread %li",threadGetCurrent().thread);
    while (!threadPool->mustExit) {
        if (mutexLock(&threadPool->mutexForQueue) == ConcurrencyFailure)
            continue;
        const struct JobInformation* nextOrder = bl_heap_top(&((BL_ThreadPool*) sharedState)->jobsQueue);
        struct JobInformation         order     = nextOrder ? *nextOrder : (struct JobInformation){0};

        if (!nextOrder) {
            mutexUnlock(&threadPool->mutexForQueue);
            threadYield();
            continue;
        }

        if (order.location == 0) {
            mutexUnlock(&threadPool->mutexForQueue);
            break;
        }
        void* job = bl_unordered_container_get(&((BL_ThreadPool*)sharedState)->orders, order.location);
        memcpy(jobBuffer,job,threadPool->orders.byteSizeOfElement);
        bl_heap_pop(&threadPool->jobsQueue);
        mutexUnlock(&threadPool->mutexForQueue);

        ((struct Job*)jobBuffer)->function(jobBuffer);

        if (mutexLock(&threadPool->mutexForQueue) != ConcurrencySuccess)
            continue;

        job = bl_unordered_container_get(&((BL_ThreadPool*)sharedState)->orders, order.location);
        memcpy(job,jobBuffer,threadPool->orders.byteSizeOfElement);

        mutexUnlock(&threadPool->mutexForQueue);
    }
    free(jobBuffer);
    bl_log_flush();
    return 0;
}

ConcurrencyError bl_threadpool_init(BL_ThreadPool* threadPool, size_t amountOfThreads, size_t sizeOfLargestAsyncArgsPack) {
    threadPool->mustExit  = false;
    threadPool->jobsQueue = bl_heap_create_stack(sizeof(struct JobInformation), jobCompare);
    if (!bl_heap_is_valid(&threadPool->jobsQueue))
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
    bl_unordered_container_put(&threadPool->orders, sizeof(void*),(void*)&(void*){NULL});
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

void* bl_threadpool_job_assign(BL_ThreadPool* threadPool,
                          size_t      priority,
                          void (*     function)(void*),
                          size_t      futureOffset,
                          const void*       args, size_t argsSize, size_t argsOffset) {
    if (priority == 0 || priority == SIZE_MAX
        || argsOffset + argsSize > threadPool->orders.byteSizeOfElement)
        return 0;
    struct Job job = {.function = function,.futureOffset = futureOffset};

    if (mutexLock(&threadPool->mutexForQueue) == ConcurrencyFailure)
        return 0;

    void* element = bl_unordered_container_put(&threadPool->orders, sizeof job, &job);

    if (!element) {
        mutexUnlock(&threadPool->mutexForQueue);
        return 0;
    }

    *(FutureVoid*) ((BL_Bytes) element + futureOffset) = false;

    memcpy((BL_Bytes) element + argsOffset, args, argsSize);

    BL_ContainerError errorCode = bl_heap_insert(&threadPool->jobsQueue, sizeof (struct JobInformation), &(struct JobInformation){.priority = priority, .location = bl_unordered_container_index_from_ref(&threadPool->orders,element)});

    mutexUnlock(&threadPool->mutexForQueue);

    if (errorCode != BL_ContainerOPSuccessful)
        return 0;
    return (BL_Bytes)element + futureOffset;
}

ConcurrencyError bl_threadpool_join(BL_ThreadPool* threadPool) {

    if (mutexLock(&threadPool->mutexForQueue) == ConcurrencyFailure)
        return ConcurrencyFailure;

    if (bl_container_dynamic_reserve(&threadPool->jobsQueue.dynamicContainer, threadPool->amountOfThreads) == BL_ContainerAllocFailure) {
        mutexUnlock(&threadPool->mutexForQueue);
        return ConcurrencyFailure;
    }

    for (size_t i = 0; i < threadPool->amountOfThreads; i++)
        bl_heap_insert(&threadPool->jobsQueue, sizeof(struct JobInformation), &(struct JobInformation){.priority = 0, .location = 0});

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

void bl_threadpool_exit(BL_ThreadPool* threadPool) {
    threadPool->mustExit = true;

    for (size_t i = 0; i < threadPool->amountOfThreads; i++)
        threadJoin(&threadPool->threads[i]);
    free(threadPool->threads);
    mutexDestroy(&threadPool->mutexForQueue);
    bl_unordered_container_destroy(&threadPool->orders);
    bl_heap_destroy(&threadPool->jobsQueue);
}

void* bl_future_get(const BL_ThreadPool* threadPool, size_t future) {
    if (future) {
        struct Job* order = bl_unordered_container_get(&threadPool->orders, future);
        return (BL_Byte*)order + order->futureOffset;
    }
    return NULL;
}

bool bl_future_destroy(BL_ThreadPool* threadPool, void* future) {
    if (!*(bool*)future)
        return false;

    bl_unordered_container_remove(&threadPool->orders,bl_unordered_container_index_from_ref(&threadPool->orders,future),NULL);
    return true;
}

static void asyncFileReadMain(void* sharedState) {
    AsyncFileReadArg* args = sharedState;

    fseek(args->args,0,SEEK_SET);
    fseek(args->args, 0,SEEK_END);
    size_t fileSize = ftell(args->args);
    fseek(args->args,0,SEEK_SET);

    args->future.future = bl_container_dynamic_create_stack(fileSize, sizeof(char));
    if (!bl_container_dynamic_is_valid((BL_DynamicContainer*)&args->future.future)) {
        args->future.isValid = true;
        return;
    }

    args->future.future.container.amountOfIndexes = fread(args->future.future.container.array,
                                                          sizeof(char),
                                                          fileSize,
                                                          args->args);
    args->future.isValid = true;
}

FutureString* bl_async_file_read(BL_ThreadPool* threadPool, size_t priority, FILE* file) {
    return bl_threadpool_job_assign(threadPool,
                               priority,
                               asyncFileReadMain,
                               asyncArgsFutureOffset(AsyncFileReadArg),
                               (void*)&file, sizeof(FILE*),asyncArgsOffset(AsyncFileReadArg));
}

bool bl_threadpool_is_valid(const BL_ThreadPool* threadPool) {
    return bl_heap_is_valid(&threadPool->jobsQueue);
}
