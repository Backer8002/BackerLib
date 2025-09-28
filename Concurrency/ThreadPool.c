#include "ThreadPool.h"
#include <BackerLibTypes.h>

struct JobInformation {
    size_t priority;

    void (*function)(void*);
};

static bool jobCompare(const void* first, const void* second) {
    return (*(struct JobInformation**) first)->priority >= (*(struct JobInformation**) second)->priority;
}

static int threadWorkerFunction(void* sharedState) {
    while (!((ThreadPool*) sharedState)->mustExit) {
        if (mutexLock(&((ThreadPool*) sharedState)->mutexForQueue) == ConcurrencyFailure)
            continue;
        struct JobInformation* const * nextOrder = heapTop(&((ThreadPool*) sharedState)->jobsQueue);
        struct JobInformation*         order     = nextOrder ? *nextOrder : NULL;
        heapPop(&((ThreadPool*) sharedState)->jobsQueue);
        mutexUnlock(&((ThreadPool*) sharedState)->mutexForQueue);

        if (order)
            order->function(order);
    }
    return 0;
}

static void threadExitWrapper(void* sharedState) { threadExit(); }

ConcurrencyError threadPoolInit(ThreadPool* threadPool, size_t amountOfThreads, size_t sizeOfLargestAsyncArgsPack) {
    threadPool->mustExit  = false;
    threadPool->jobsQueue = heapCreateStack(sizeof(void*), jobCompare);
    if (!isValidObject(&threadPool->jobsQueue.header))
        return ConcurrencyFailure;

    threadPool->orders = unorderedContainerCreateStack(16, sizeOfLargestAsyncArgsPack,false);

    if (!isValidObject(&threadPool->orders.header)) {
        heapDestroy(&threadPool->jobsQueue);
        return ConcurrencyFailure;
    }

    threadPool->mutexForQueue = mutexCreate(MutexPlain);
    if (!mutexIsValid(&threadPool->mutexForQueue)) {
        unorderedContainerDestroy(&threadPool->orders);
        heapDestroy(&threadPool->jobsQueue);
        return ConcurrencyFailure;
    }

    threadPool->threads = malloc(sizeof *threadPool->threads * amountOfThreads);
    if (!threadPool->threads) {
        mutexDestroy(&threadPool->mutexForQueue);
        unorderedContainerDestroy(&threadPool->orders);
        heapDestroy(&threadPool->jobsQueue);
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
    unorderedContainerDestroy(&threadPool->orders);
    heapDestroy(&threadPool->jobsQueue);
    return ConcurrencyFailure;
}

void* threadPoolJobAssign(ThreadPool* threadPool,
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

    UnorderedContainerPutResult putResult = unorderedContainerPut(&threadPool->orders, sizeof jobInfo, &jobInfo);

    if (putResult.resultCode != ContainerOPSuccessful) {
        mutexUnlock(&threadPool->mutexForQueue);
        return NULL;
    }

    void* locationOfElement = unorderedContainerGet(&threadPool->orders, putResult.locationOfElement).element;

    *(FutureVoid*) ((Bytes) locationOfElement + futureOffset) = false;

    memcpy((Bytes) locationOfElement + argsOffset, args, argsSize);

    ContainerError errorCode = heapInsert(&threadPool->jobsQueue, sizeof locationOfElement, &locationOfElement);

    mutexUnlock(&threadPool->mutexForQueue);

    if (errorCode != ContainerOPSuccessful)
        return NULL;
    return (Bytes) locationOfElement + futureOffset;
}

ConcurrencyError threadPoolJoin(ThreadPool* threadPool) {
    struct JobInformation exitCommand = {.priority = 0, .function = threadExitWrapper};
    struct JobInformation* exitPointer = &exitCommand;

    if (mutexLock(&threadPool->mutexForQueue) == ConcurrencyFailure)
        return ConcurrencyFailure;

    if (containerDynamicReserve(&threadPool->jobsQueue.dynamicContainer, threadPool->amountOfThreads) == ContainerAllocFailure) {
        mutexUnlock(&threadPool->mutexForQueue);
        return ConcurrencyFailure;
    }

    for (size_t i = 0; i < threadPool->amountOfThreads; i++)
        heapInsert(&threadPool->jobsQueue, sizeof(void*), &exitPointer);

    if (mutexUnlock(&threadPool->mutexForQueue) == ConcurrencyFailure)
        return ConcurrencyFailure;

    for (size_t i = 0; i < threadPool->amountOfThreads; i++)
        threadJoin(&threadPool->threads[i]);

    free(threadPool->threads);
    mutexDestroy(&threadPool->mutexForQueue);
    unorderedContainerDestroy(&threadPool->orders);
    heapDestroy(&threadPool->jobsQueue);
    return ConcurrencySuccess;
}

void threadPoolExit(ThreadPool* threadPool) {
    threadPool->mustExit = true;

    for (size_t i = 0; i < threadPool->amountOfThreads; i++)
        threadJoin(&threadPool->threads[i]);
    free(threadPool->threads);
    mutexDestroy(&threadPool->mutexForQueue);
    unorderedContainerDestroy(&threadPool->orders);
    heapDestroy(&threadPool->jobsQueue);
}

bool futureDestroy(ThreadPool* threadPool, void* future) {
    if (!*(FutureVoid*) future)
        return false;

    unorderedContainerRemove(&threadPool->orders,
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

    args->future.future = containerDynamicCreateStack(fileSize, sizeof(char),false);
    if (!isValidObject(&args->future.future.header)) {
        args->future.isValid = true;
        return;
    }

    args->future.future.container.amountOfIndexes = fread(args->future.future.container.array,
                                                          sizeof(char),
                                                          fileSize,
                                                          args->args);
    args->future.isValid = true;
}

FutureString* asyncFileRead(ThreadPool* threadPool, size_t priority, FILE* file) {
    return threadPoolJobAssign(threadPool,
                               priority,
                               asyncFileReadMain,
                               asyncArgsFutureOffset(AsyncFileReadArg),
                               &file, sizeof file,asyncArgsOffset(AsyncFileReadArg));
}
