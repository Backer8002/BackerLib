#include "eventInternalHeader.h"
#include <BackerLibTypes.h>
#include <stdbool.h>
#include <stdint.h>


#include <BackerLibCommonEvents.h>
#include <assert.h>
#include <inttypes.h>
#include <signal.h>
#include <threads.h>


static EventHandle* mainEventHandle     = NULL;
static HashMap      memoryAllocs        = {0};
static FILE *       debugFileStreams[2] = {NULL, NULL}, *errorFileStreams[2] = {NULL, NULL};

#include <stdlib.h>

int          internal_eventMainLoop(EventHandle* eventHandle);
EventError_t internal_invokeSubscriber(const EventCall* eventCall, StringView subscriberID, EventSubscriber* eventSubscriber);
void         internal_memoryLogCall(const Event* const event, uint32_t line, const char* file, size_t size, size_t address);

EventHandle* eventSystemInit(void) {
    EventHandle* eventHandle = malloc(sizeof(EventHandle));
    if (eventHandle == NULL)
        return NULL;

    eventHandle->eventQueue = arrayListCreateStack(0, sizeof(EventCall), false);
    if (!isValidObject((DataTypeFlags*) &eventHandle->eventQueue)) {
        free(eventHandle);
        return NULL;
    }

    eventHandle->eventSubscribers = hashMapCuckooCreateStack(5, sizeof(StringView), sizeof(EventSubscriber), true, hashFunctionDefualtSingleVarWithSalt);

    if (!isValidObject((DataTypeFlags*) &eventHandle->eventSubscribers)) {
        arrayListDestroy(&(eventHandle->eventQueue));
        free(eventHandle);
        return NULL;
    }

    if (mtx_init(&eventHandle->mutexForSubscriber, mtx_plain) != thrd_success) {
        hashMapCuckooDestroy(&(eventHandle->eventSubscribers), NULL, NULL);
        arrayListDestroy(&(eventHandle->eventQueue));
        free(eventHandle);
        return NULL;
    }
#if 1
    memoryAllocs = hashMapCreateStack(100, sizeof(size_t), sizeof(size_t), false, true, hashFunctionDefualtSingleVar);
    if (!isValidObject((DataTypeFlags*) &memoryAllocs)) {
        hashMapCuckooDestroy(&(eventHandle->eventSubscribers), NULL, NULL);
        arrayListDestroy(&(eventHandle->eventQueue));
        mtx_destroy(&eventHandle->mutexForSubscriber);
        free(eventHandle);
        return NULL;
    }
#endif // _DEBUG


    if (thrd_create(&(eventHandle->eventThread), (int (*)(void*)) internal_eventMainLoop, eventHandle) != thrd_success) {
        hashMapCuckooDestroy(&(eventHandle->eventSubscribers), NULL, NULL);
        arrayListDestroy(&(eventHandle->eventQueue));
        mtx_destroy(&eventHandle->mutexForSubscriber);
        free(eventHandle);
#if 1
        hashMapDestroy(&memoryAllocs, NULL, NULL);
#endif // _DEBUG

        return NULL;
    }
    atexit(eventSystemShutdown);
    mainEventHandle = eventHandle;
    return eventHandle;
}

static void internal_freeMemAllocsKey(void* key) {
    fprintf(stderr, "Memory alloc 0x%zx was not freed ", (size_t) key);
}

static void internal_freeMemAllocsElements(void* element) {
    fprintf(stderr, "of size %zu\n", (size_t) element);
}

void eventSystemShutdown(void) {
    mainEventHandle->shouldQuit = true;
    thrd_join(mainEventHandle->eventThread, NULL);
    EventHandle* eventHandleToDestroy = mainEventHandle;
    mainEventHandle                   = NULL;
    mtx_destroy(&eventHandleToDestroy->mutexForSubscriber);
    arrayListDestroy(&eventHandleToDestroy->eventQueue);
    hashMapCuckooDestroy(&eventHandleToDestroy->eventSubscribers, NULL, NULL);

    if (debugFileStreams[0] && debugFileStreams[0] != stdout)
        fclose(debugFileStreams[0]);
    if (debugFileStreams[1])
        fclose(debugFileStreams[1]);
    if (errorFileStreams[0] && errorFileStreams[0] != stdout && errorFileStreams[0] != stderr)
        fclose(errorFileStreams[0]);
    if (errorFileStreams[1])
        fclose(errorFileStreams[1]);
#if 1
    fflush(stdout);
    fprintf(stderr, "\e[33mMemory leaks:\e[31m\n");
    hashMapDestroy(&memoryAllocs, internal_freeMemAllocsKey, internal_freeMemAllocsElements);
    fprintf(stderr, "\e[32mEnd of memory leaks.\e[0m\n");
    fflush(stderr);
#endif
    fprintf(stdout, "Bye from event handler.\n");
    free(eventHandleToDestroy);
}


extern bool eventInitDefualtLog(const char* debugLogFile, bool outputToStdout, const char* errorLogFile, bool outputErrorsToStdErr) {
    FILE* debugLog = NULL;
    FILE* errorLog = NULL;

    if (debugLogFile) {
        debugLog = fopen(debugLogFile, "wt");
        if (!debugLog)
            return false;
    }
    if (errorLogFile) {
        errorLog = fopen(errorLogFile, "wt");
        if (!errorLog) {
            if (debugLog)
                fclose(debugLog);
            fclose(errorLog);
            return false;
        }
    }

    if (outputToStdout) {
        debugFileStreams[0] = stdout;
        errorFileStreams[0] = stdout;
        debugFileStreams[1] = debugLog;
    } else {
        debugFileStreams[0] = debugLog;
    }

    if (outputErrorsToStdErr) {
        errorFileStreams[0] = stderr;
        errorFileStreams[1] = errorLog;
    } else if (outputToStdout) {
        errorFileStreams[1] = errorLog;
    } else {
        errorFileStreams[0] = errorLog;
    }

    eventRegLogSubToID(mainEventHandle, InfoLogLevel, writeEventToLogLocations, false, (errorLog != NULL) + outputToStdout, debugFileStreams);
    eventRegLogSubToID(mainEventHandle, DebugLogLevel, writeEventToLogLocations, false, (errorLog != NULL) + outputToStdout, debugFileStreams);
    eventRegLogSubToID(mainEventHandle, WarningLogLevel, writeEventToLogLocations, false, (errorLog != NULL) + (outputToStdout || outputErrorsToStdErr), errorFileStreams);
    eventRegLogSubToID(mainEventHandle, ErrorLogLevel, writeEventToLogLocations, false, (errorLog != NULL) + (outputToStdout || outputErrorsToStdErr), errorFileStreams);
    eventRegLogSubToID(mainEventHandle, CriticalLogLevel, writeEventToLogLocations, false, (errorLog != NULL) + (outputToStdout || outputErrorsToStdErr), errorFileStreams);

    return true;
}

EventError_t internal_invokeSubscriber(const EventCall* eventCall, StringView subscriberID, EventSubscriber* eventSubscriber) {
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
        mtx_lock(&eventHandle->mutexForSubscriber);

        for (EventCall* currentCall = eventHandle->eventQueue.container.array; currentCall < ((EventCall*) eventHandle->eventQueue.container.array + eventHandle->eventQueue.container.amountOfIndexes); currentCall++) {
            EventSubscriber* mainSubscriber = hashMapCuckooGet(&eventHandle->eventSubscribers, sizeof(StringView), &currentCall->eventCallMain.id);
            if (mainSubscriber == NULL) {
            } else {
                if (internal_invokeSubscriber(currentCall, currentCall->eventCallMain.id, mainSubscriber) != EventOperationSuccess)
                    ;
            }

            for (uint32_t groupCallIterator = 0; groupCallIterator < currentCall->eventCallMain.amountOfGroups; groupCallIterator++) {
                EventSubscriber* groupSubscriber = hashMapCuckooGet(&eventHandle->eventSubscribers, sizeof(StringView), currentCall->eventCallMain.groupIds + groupCallIterator);
                if (groupSubscriber == NULL) {
                    continue;
                }
                if (internal_invokeSubscriber(currentCall, currentCall->eventCallMain.groupIds[groupCallIterator], groupSubscriber) != EventOperationSuccess)
                    ;
            }
        }

        containerDynamicClear(&eventHandle->eventQueue.dynamicContainer);

        mtx_unlock(&eventHandle->eventQueue.mutex);
        mtx_unlock(&eventHandle->mutexForSubscriber);
    }
    return 0;
}

EventError_t eventRegSubToID(EventHandle* eventHandle, const StringView ID, void (*function)(EventCall, StringView), bool shouldRunOnSeperateThread) {
    mtx_lock(&eventHandle->mutexForSubscriber);

    EventSubscriber eventSubscriberToReg = {.eventSubscriberMain = {.eventSubscriberType = EventSubscriberNormal, .function = function, .flags = (shouldRunOnSeperateThread) ? EVENT_FLAG_CREATE_THREAD : 0}};

    if (hashMapCuckooInsert(&eventHandle->eventSubscribers, sizeof ID, &ID, sizeof eventSubscriberToReg, &eventSubscriberToReg) != ContainerOPSuccessful) {
        mtx_unlock(&eventHandle->mutexForSubscriber);
        logCriticalCall("Cannot register id to eventSubscribers", __LINE__, __FILE__);
        return EventCritError;
    }

    mtx_unlock(&eventHandle->mutexForSubscriber);
    return EventOperationSuccess;
}

EventError_t eventRegLogSubToID(EventHandle* eventHandle, StringView ID, void (*function)(EventCall, StringView, size_t, FILE**), bool shouldRunOnSeperateThread, size_t amountOfOutputs, FILE** outputs) {
    mtx_lock(&eventHandle->mutexForSubscriber);

    EventSubscriber eventSubscriberToReg = {.eventSubscriberLog = {.eventSubscriberType = EventSubscriberLogging,
                                                                   .amountOfOutputFiles = amountOfOutputs,
                                                                   .outputFiles         = outputs,
                                                                   .function            = function,
                                                                   .flags               = (shouldRunOnSeperateThread) ? EVENT_FLAG_CREATE_THREAD : 0}};

    if (hashMapCuckooInsert(&eventHandle->eventSubscribers, sizeof ID, &ID, sizeof eventSubscriberToReg, &eventSubscriberToReg) != ContainerOPSuccessful) {
        mtx_unlock(&eventHandle->mutexForSubscriber);
        logCriticalCall("Cannot register id to eventSubscribers", __LINE__, __FILE__);
        return EventCritError;
    }

    mtx_unlock(&eventHandle->mutexForSubscriber);
    return EventOperationSuccess;
}


void eventCall(const Event* const event, thrd_t currentThread) {
    EventCall eventCallToSend = {
        .eventCallMain = {.eventType      = EventNormal,
                          .id             = event->id,
                          .groupIds       = event->groupIds,
                          .amountOfGroups = event->amountOfGroups,
                          .callerThread   = currentThread}};
    if (!mainEventHandle)
        return;

    mtx_lock(&mainEventHandle->eventQueue.mutex);

    containerDynamicAppend(&mainEventHandle->eventQueue.dynamicContainer, sizeof eventCallToSend, &eventCallToSend);

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
    if (!mainEventHandle)
        return;

    mtx_lock(&mainEventHandle->eventQueue.mutex);

    containerDynamicAppend(&mainEventHandle->eventQueue.dynamicContainer, sizeof eventCallToSend, &eventCallToSend);

    mtx_unlock(&mainEventHandle->eventQueue.mutex);
}

StringView freeMemDoesNotExistGroupIds[] = {MemoryGroupId, ErrorLogLevel};
StringView allocEventsGroupIds[]         = {MemoryGroupId, DebugLogLevel, AllocMemoryGroupId};
StringView freeEventGroupId[]            = {DebugLogLevel, MemoryGroupId, DeallocMemoryGroupId};
const Event freeMemDoesNotExist           = {.id = stringViewInitConstExpr("Unable to free memory since it does not exist"), .amountOfGroups = 2, .groupIds = freeMemDoesNotExistGroupIds};
const Event mallocEvent                   = {.id = stringViewInitConstExpr("Allocated memory via malloc"), .amountOfGroups = 3, .groupIds = allocEventsGroupIds};
const Event callocEvent                   = {.id = stringViewInitConstExpr("Allocated memory via calloc"), .amountOfGroups = 3, .groupIds = allocEventsGroupIds};
const Event reallocEvent                  = {.id = stringViewInitConstExpr("Reallocated memory"), .amountOfGroups = 3, .groupIds = allocEventsGroupIds};
const Event freeEvent                     = {.id = stringViewInitConstExpr("Freed memory"), .amountOfGroups = 3, .groupIds = freeEventGroupId};

#if 1
#undef malloc
#undef calloc
#undef realloc
#undef free
#endif

void internal_memoryLogCall(const Event* const event, uint32_t line, const char* file, size_t size, size_t address) {


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
    if (!mainEventHandle)
        return;

    mtx_lock(&mainEventHandle->eventQueue.mutex);

    containerDynamicAppend(&mainEventHandle->eventQueue.dynamicContainer, sizeof eventCallToSend, &eventCallToSend);

    mtx_unlock(&mainEventHandle->eventQueue.mutex);
}


void* mallocLogVersion(size_t size, uint32_t line, const char* file) {
    void* memory = malloc(size);
    if (memory == NULL)
        return NULL;

    internal_memoryLogCall(&mallocEvent, line, file, size, (size_t) memory);
    if (isValidObject((DataTypeFlags*) &memoryAllocs))
        hashMapInsert(&memoryAllocs, sizeof memory, &memory, sizeof size, &size);
    return memory;
}

void* callocLogVersion(size_t amount, size_t size, uint32_t line, const char* file) {
    void* memory = calloc(amount, size);
    if (memory == NULL)
        return NULL;

    internal_memoryLogCall(&callocEvent, line, file, size * amount, (size_t) memory);
    if (isValidObject((DataTypeFlags*) &memoryAllocs))
        hashMapInsert(&memoryAllocs, sizeof memory, &memory, sizeof size, &size);
    return memory;
}

void* reallocLogVersion(void* currentPtr, size_t size, uint32_t line, const char* file) {
    if (currentPtr == NULL)
        return mallocLogVersion(size, line, file);

    void* memory = realloc(currentPtr, size);
    if (memory == NULL)
        return NULL;

    internal_memoryLogCall(&reallocEvent, line, file, size, (size_t) memory);
    if (isValidObject((DataTypeFlags*) &memoryAllocs)) {
        if (currentPtr == memory)
            hashMapReplace(&memoryAllocs, sizeof memory, &memory, sizeof size, &size, NULL);
        else {
            hashMapRemove(&memoryAllocs, sizeof currentPtr, &currentPtr, NULL, NULL);
            hashMapInsert(&memoryAllocs, sizeof memory, &memory, sizeof size, &size);
        }
    }
    return memory;
}
void freeLogVersion(void* ptr, uint32_t line, const char* file) {
    if (!isValidObject((DataTypeFlags*) &memoryAllocs)) {
        free(ptr);
        return;
    }
    size_t size = 0;
    if (hashMapGet(&memoryAllocs, sizeof ptr, &ptr) == NULL) {
        logCall(&freeMemDoesNotExist, noThread, line, file);
        return;
    }
    internal_memoryLogCall(&freeEvent, line, file, size, (size_t) ptr);
    hashMapRemove(&memoryAllocs, sizeof ptr, &ptr, NULL, NULL);
    free(ptr);
}

void writeEventToLogLocations(EventCall event, StringView logLevel, size_t amountOfLocations, FILE** locations) {
    switch (event.type) {
    case EventNormal:
        for (size_t i = 0; i < amountOfLocations; i++)
            fprintf(locations[i], "\e[33m%" PRIi64 " \e[37m[%s] \e[35m%s\e[0m\n", time(NULL), logLevel.array, event.eventCallMain.id.array);
        break;
    case EventLogEvent:
        for (size_t i = 0; i < amountOfLocations; i++)
            fprintf(locations[i], "\e[33m%" PRIi64 " \e[37m[%s] \e[35m%s \e[0min file: \e[36m%s\e[0m at line: \e[32m%zu\e[0m\n", time(NULL), logLevel.array, event.eventCallMain.id.array, event.eventCallLog.file, event.eventCallLog.line);
        break;
    case EventMemLogEvent:
        for (size_t i = 0; i < amountOfLocations; i++)
            fprintf(
                locations[i],
                "\e[33m%" PRIi64 " \e[37m[%s] \e[35m%s\e[0m of size: \e[36m%zu\e[0m at address: \e[31m0x%zx\e[0m in file: \e[36m%s\e[0m at line: \e[32m%zu\e[0m\n",
                time(NULL),
                logLevel.array,
                event.eventCallMain.id.array,
                event.eventCallMemory.allocSize,
                event.eventCallMemory.address,
                event.eventCallLog.file,
                event.eventCallLog.line);
        break;
    default:
        break;
    }
}

static StringView InfoLogGroupIDs[]     = {InfoLogLevel};
static StringView DebugLogGroupIDs[]    = {DebugLogLevel};
static StringView WarningLogGroupIDs[]  = {WarningLogLevel};
static StringView ErrorLogGroupIDs[]    = {ErrorLogLevel};
static StringView CriticalLogGroupIDs[] = {CriticalLogLevel};

void               logInfoCall(const char* message, uint32_t line, const char* file) {
    Event event = {.id = stringViewInit(message), .groupIds = InfoLogGroupIDs, .amountOfGroups = 1};
    logCall(&event, noThread, line, file);
}

void logDebugCall(const char* message, uint32_t line, const char* file) {
    Event event = {.id = stringViewInit(message), DebugLogGroupIDs, 1};
    logCall(&event, noThread, line, file);
}

void logWarnCall(const char* message, uint32_t line, const char* file) {
    Event event = {.id = stringViewInit(message), WarningLogGroupIDs, 1};
    logCall(&event, noThread, line, file);
}

void logErrorCall(const char* message, uint32_t line, const char* file) {
    Event event = {.id = stringViewInit(message), ErrorLogGroupIDs, 1};
    logCall(&event, noThread, line, file);
}

void logCriticalCall(const char* message, uint32_t line, const char* file) {
    Event event = {.id = stringViewInit(message), CriticalLogGroupIDs, 1};
    logCall(&event, noThread, line, file);
}
