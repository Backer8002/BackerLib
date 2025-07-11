#include "eventInternalHeader.h"
#include <assert.h>
#include <signal.h>
#include <threads.h>
#include <BackerLibCommonEvents.h>

#ifdef _DEBUG
static HashMap memoryAllocs = {0};

#undef malloc
#undef calloc
#undef realloc
#undef free
#else
static int memoryAllocs;
#endif

static EventHandle* mainEventHandle = NULL;

#include <stdlib.h>

int          internal_eventMainLoop(EventHandle* eventHandle);
EventError_t internal_invokeSubscriber(EventCall* eventCall, const char* subscriberID, EventSubscriber* eventSubscriber);
void         internal_memoryLogCall(const Event* const event, uint32_t line, const char* file, size_t size, size_t address);

EventHandle* eventSystemInit(void) {
    EventHandle* eventHandle = malloc(sizeof(EventHandle));
    if (eventHandle == NULL)
        return NULL;

    eventHandle->eventQueue = arrayListCreateStack(0, sizeof(EventCall), ListEventList, false);
    if (eventHandle->eventQueue.header.dataArrayVarType == ListNone) {
        free(eventHandle);
        return NULL;
    }

    eventHandle->eventSubscribers = hashMapCuckooCreateStack(1, sizeof(const char*), sizeof(EventSubscriber), ListCString, ListEventList, false, hashFunctionDefualtSingleVarWithSalt);

    if (eventHandle->eventSubscribers.header.dataArrayVarType == ListNone) {
        arrayListDestroy(&(eventHandle->eventQueue));
        free(eventHandle);
        return NULL;
    }

    if (!(eventHandle->eventQueue.header.flags & ObjectFlagMutexExists && eventHandle->eventSubscribers.header.flags & ObjectFlagMutexExists)) {
        hashMapCuckooDestroy(&(eventHandle->eventSubscribers), NULL, NULL);
        arrayListDestroy(&(eventHandle->eventQueue));
        free(eventHandle);
        return NULL;
    }
#ifdef _DEBUG
    memoryAllocs = hashMapCreateStack(1, sizeof(size_t), sizeof(size_t), ListUInt64, ListUInt64, true, hashFunctionDefualtSingleVar);
    if (memoryAllocs.header.dataArrayVarType == ListNone) {
        hashMapCuckooDestroy(&(eventHandle->eventSubscribers), NULL, NULL);
        arrayListDestroy(&(eventHandle->eventQueue));
        free(eventHandle);
        return NULL;
    }
#endif // _DEBUG


    if (thrd_create(&(eventHandle->eventThread), (int (*)(void*)) internal_eventMainLoop, eventHandle) != thrd_success) {
        hashMapCuckooDestroy(&(eventHandle->eventSubscribers), NULL, NULL);
        arrayListDestroy(&(eventHandle->eventQueue));
        free(eventHandle);
#ifdef _DEBUG
        hashMapDestroy(&memoryAllocs, NULL, NULL);
#endif // _DEBUG

        return NULL;
    }

    mainEventHandle = eventHandle;
    return eventHandle;
}

EventError_t internal_invokeSubscriber(EventCall* eventCall, const char* subscriberID, EventSubscriber* eventSubscriber) {
    if (eventSubscriber->eventSubscriberMain.flags & EVENT_FLAG_CREATE_THREAD) {
        assert("Not implemented");
    } else {
        if (eventSubscriber->typeOfSubscriber == EventSubscriberNormal)
            eventSubscriber->eventSubscriberMain.function(*eventCall, subscriberID);
        else if (eventSubscriber->typeOfSubscriber == EventSubscriberLogging)
            eventSubscriber->eventSubscriberLog.function(*eventCall, subscriberID, eventSubscriber->eventSubscriberLog.amountOfOutputFiles, eventSubscriber->eventSubscriberLog.outputFiles);
    }

    return EventOperationSuccess;
}

int internal_eventMainLoop(EventHandle* eventHandle) {
    struct timespec timeToSleep = {.tv_sec = 1, .tv_nsec = 0};
    bool            quit        = false;

    while (!quit) {
        if (eventHandle->shouldQuit)
            quit = true;

        thrd_sleep(&timeToSleep, NULL);

        mtx_lock(&eventHandle->eventQueue.mutex);
        mtx_lock(&eventHandle->eventSubscribers.mutex);

        for (EventCall* currentCall = (EventCall*) eventHandle->eventQueue.list; currentCall < ((EventCall*) eventHandle->eventQueue.list + eventHandle->eventQueue.totalAmountOfElements); currentCall++) {
            EventSubscriber* mainSubscriber = hashMapCuckooGet(&eventHandle->eventSubscribers, currentCall->eventCallMain.id, ListCString);
            if (mainSubscriber == NULL) {
            } else {
                if (internal_invokeSubscriber(currentCall, currentCall->eventCallMain.id, mainSubscriber) != EventOperationSuccess)
                    ;
            }

            for (uint32_t groupCallIterator = 0; groupCallIterator < currentCall->eventCallMain.amountOfGroups; groupCallIterator++) {
                EventSubscriber* groupSubscriber = (EventSubscriber*) hashMapCuckooGet(&eventHandle->eventSubscribers, currentCall->eventCallMain.groupIds[groupCallIterator], ListCString);
                if (groupSubscriber == NULL) {
                    continue;
                }
                if (internal_invokeSubscriber(currentCall, currentCall->eventCallMain.groupIds[groupCallIterator], groupSubscriber) != EventOperationSuccess)
                    ;
            }
        }

        eventHandle->eventQueue.amountOfElements = 0;

        mtx_unlock(&eventHandle->eventQueue.mutex);
        mtx_unlock(&eventHandle->eventSubscribers.mutex);
    }
    return 0;
}

EventError_t eventRegSubToID(EventHandle* eventHandle, const char* ID, void (*function)(EventCall, const char*), bool shouldRunOnSeperateThread) {
    mtx_lock(&eventHandle->eventSubscribers.mutex);

    EventSubscriber eventSubscriberToReg = {.eventSubscriberMain = {.eventSubscriberType = EventSubscriberNormal, .function = function, .flags = (shouldRunOnSeperateThread) ? EVENT_FLAG_CREATE_THREAD : 0}};

    if (hashMapCuckooInsert(&eventHandle->eventSubscribers, ID, ListCString, &eventSubscriberToReg, ListEventList) != HashMapOperationSuccsess) {
        mtx_unlock(&eventHandle->eventSubscribers.mutex);
        LogCrit();
        return EventCritError;
    }

    mtx_unlock(&eventHandle->eventSubscribers.mutex);
    return EventOperationSuccess;
}

EventError_t eventRegLogSubToID(EventHandle* eventHandle, const char* ID, void (*function)(EventCall, const char*, size_t, FILE**), bool shouldRunOnSeperateThread, size_t amountOfOutputs, FILE** outputs) {
    mtx_lock(&eventHandle->eventSubscribers.mutex);

    EventSubscriber eventSubscriberToReg = {.eventSubscriberLog = {.eventSubscriberType = EventSubscriberLogging,
                                                                   .amountOfOutputFiles = amountOfOutputs,
                                                                   .outputFiles         = outputs,
                                                                   .function            = function,
                                                                   .flags               = (shouldRunOnSeperateThread) ? EVENT_FLAG_CREATE_THREAD : 0}};

    if (hashMapCuckooInsert(&eventHandle->eventSubscribers, ID, ListCString, &eventSubscriberToReg, ListEventList) != HashMapOperationSuccsess) {
        mtx_unlock(&eventHandle->eventSubscribers.mutex);
        LogCrit();
        return EventCritError;
    }

    mtx_unlock(&eventHandle->eventSubscribers.mutex);
    return EventOperationSuccess;
}


void eventCall(const Event* const event, thrd_t currentThread) {
    EventCall eventCallToSend = {
        .eventCallMain = {.eventType      = EventNormal,
                          .id             = event->id,
                          .groupIds       = event->groupIds,
                          .amountOfGroups = event->amountOfGroups,
                          .callerThread   = currentThread}};

    mtx_lock(&mainEventHandle->eventQueue.mutex);

    arrayListElementSet(&mainEventHandle->eventQueue, mainEventHandle->eventQueue.amountOfElements, &eventCallToSend, ListEventList);

    mtx_unlock(&mainEventHandle->eventQueue.mutex);
}

void logCall(const Event* const event, thrd_t currentThread, uint32_t line, const char* file) {
    EventCall eventCallToSend = {
        .eventCallLog = {
            .line          = line,
            .file          = file,
            .eventCallMain = {.eventType      = EventLogEvent,
                              .id             = event->id,
                              .groupIds       = event->groupIds,
                              .amountOfGroups = event->amountOfGroups,
                              .callerThread   = currentThread}}};

    mtx_lock(&mainEventHandle->eventQueue.mutex);

    arrayListElementSet(&mainEventHandle->eventQueue, mainEventHandle->eventQueue.amountOfElements, &eventCallToSend, ListEventList);

    mtx_unlock(&mainEventHandle->eventQueue.mutex);
}

const char* freeMemDoesNotExistGroupIds[] = {MemoryGroupId,ErrorLogLevel};
const char* allocEventsGroupIds[] = {MemoryGroupId,DebugLogLevel,AllocMemoryGroupId};
const char* freeEventGroupId[] = {DebugLogLevel,MemoryGroupId,DeallocMemoryGroupId};
const Event freeMemDoesNotExist = {.id = "Unable to free memory since it does not exist", .amountOfGroups = 2, .groupIds = freeMemDoesNotExistGroupIds};
const Event mallocEvent         = {.id = "Allocated memory via malloc", .amountOfGroups = 3, .groupIds = AllocMemoryGroupId};
const Event callocEvent         = {.id = "Allocated memory via calloc", .amountOfGroups = 3, .groupIds = AllocMemoryGroupId};
const Event reallocEvent        = {.id = "Reallocated memory", .amountOfGroups = 3, .groupIds = AllocMemoryGroupId};
const Event freeEvent           = {.id = "Freed memory", .amountOfGroups = 3, .groupIds = DeallocMemoryGroupId};

void        internal_memoryLogCall(const Event* const event, uint32_t line, const char* file, size_t size, size_t address) {


    EventCall eventCallToSend = {
               .eventCallMemory = {
                   .allocSize    = size,
                   .address      = address,
                   .eventCallLog = {
                       .line          = line,
                       .file          = file,
                       .eventCallMain = {.eventType      = EventMemLogEvent,
                                         .id             = event->id,
                                         .groupIds       = event->groupIds,
                                         .amountOfGroups = event->amountOfGroups,
                                         .callerThread   = noThread}}}};

    mtx_lock(&mainEventHandle->eventQueue.mutex);

    arrayListElementSet(&mainEventHandle->eventQueue, mainEventHandle->eventQueue.amountOfElements, &eventCallToSend, ListEventList) != EventOperationSuccess;

    mtx_unlock(&mainEventHandle->eventQueue.mutex);
}


void* mallocLogVersion(size_t size, uint32_t line, const char* file) {
    void* memory = malloc(size);
    if (memory == NULL)
        return NULL;

    internal_memoryLogCall(&mallocEvent, line, file, size, (size_t) memory);
    hashMapInsert(&memoryAllocs, &memory, ListUInt64, (size_t*) size, ListUInt64);
    return memory;
}

void* callocLogVersion(size_t amount, size_t size, uint32_t line, const char* file) {
    void* memory = calloc(amount, size);
    if (memory == NULL)
        return NULL;

    internal_memoryLogCall(&callocEvent, line, file, size * amount, (size_t) memory);
    hashMapInsert(&memoryAllocs, &memory, ListUInt64, (size_t*) (size * amount), ListUInt64);
    return memory;
}

void* reallocLogVersion(void* currentPtr, size_t size, uint32_t line, const char* file) {
    if (currentPtr == NULL)
        return mallocLogVersion(size, line, file);

    void* memory = realloc(currentPtr, size);
    if (memory == NULL)
        return NULL;

    internal_memoryLogCall(&reallocEvent, line, file, size, (size_t) memory);
    if (currentPtr == memory)
        hashMapReplace(&memoryAllocs, &memory, ListUInt64, (size_t*) size, ListUInt64, NULL);
    else {
        hashMapRemove(&memoryAllocs, &currentPtr, ListUInt64, NULL, NULL);
        hashMapInsert(&memoryAllocs, &memory, ListUInt64, (size_t*) size, ListUInt64);
    }
    return memory;
}

void freeLogVersion(void* ptr, uint32_t line, const char* file) {
    size_t size = 0;
    if (hashMapGet(&memoryAllocs, &ptr, ListUInt64, (size_t**) (&size)) == HashMapKeyDoesNotExist) {
        logCall(&freeMemDoesNotExist, noThread, line, file);
        return;
    }
    internal_memoryLogCall(&freeEvent, line, file, size, (size_t) ptr);
    hashMapRemove(&memoryAllocs, &ptr, ListUInt64, NULL, NULL);
}

void writeEventToLogLocations(EventCall event, const char* logLevel, size_t amountOfLocations, FILE** locations) {
    switch (event.type) {
    case EventNormal:
        for (size_t i = 0; i < amountOfLocations; i++)
            fprintf(locations[i], "%lli [%s] %s\n", time(NULL), logLevel, event.eventCallMain.id);
        break;
    case EventLogEvent:
        for (size_t i = 0; i < amountOfLocations; i++)
            fprintf(locations[i], "%lli [%s] %s in file: %s at line: %llu\n", time(NULL), logLevel, event.eventCallMain.id, event.eventCallLog.file, event.eventCallLog.line);
        break;
    case EventMemLogEvent:
        for (size_t i = 0; i < amountOfLocations; i++)
            fprintf(
                locations[i],
                "%lli [%s] %s of size: %llu at address: %llu in file: %s at line: %llu\n",
                time(NULL),
                logLevel,
                event.eventCallMain.id,
                event.eventCallMemory.allocSize,
                event.eventCallMemory.address,
                event.eventCallLog.file,
                event.eventCallLog.line);
        break;
    default:
        break;
    }
}

void logInfoCall(EventCall* eventCall) {
}

void logDebugCall(EventCall* eventCall) {
}

void logWarnCall(EventCall* eventCall) {
}

void logErrorCall(EventCall* eventCall) {
}

void logCriticalCall(EventCall* eventCall) {
}
