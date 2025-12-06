#include <ANSIEscapeSequences.h>
#include "eventInternalHeader.h"
#include <BackerLibTypes.h>
#include <stdbool.h>
#include <stdint.h>


#include <BackerLibCommonEvents.h>
#include <assert.h>
#include <inttypes.h>
#include <signal.h>


static EventHandle* mainEventHandle     = NULL;
static BL_Hashmap      memoryAllocs        = {0};
static FILE*        debugFileStreams[2] = {NULL, NULL},*errorFileStreams[2] = {NULL, NULL};

#include <stdlib.h>

int internal_eventMainLoop(EventHandle* eventHandle);

EventError_t internal_invokeSubscriber(const EventCall* eventCall, BL_StringView subscriberID, const EventSubscriber* eventSubscriber);

void internal_memoryLogCall(const Event* const event, uint32_t line, const char* file, size_t size, size_t address);

EventHandle* eventSystemInit(void) {
    EventHandle* eventHandle = malloc(sizeof(EventHandle));
    if (eventHandle == NULL)
        return NULL;
    eventHandle->shouldQuit = false;
    eventHandle->eventQueue = arrayListCreateStack(0, sizeof(EventCall));
    if (!bl_container_dynamic_is_valid(&eventHandle->eventQueue.dynamicContainer)) {
        free(eventHandle);
        return NULL;
    }

    eventHandle->eventSubscribers = bl_hashmap_create_stack(5, sizeof(BL_StringView), sizeof(EventSubscriber), true, NULL);

    if (!bl_hashmap_is_valid(&eventHandle->eventSubscribers)) {
        arrayListDestroy(&(eventHandle->eventQueue));
        free(eventHandle);
        return NULL;
    }

    eventHandle->mutexForSubscriber = mutexCreate(MutexPlain);
    if (!mutexIsValid(&eventHandle->mutexForSubscriber)) {
        bl_hashmap_destroy(&(eventHandle->eventSubscribers), NULL, NULL);
        arrayListDestroy(&(eventHandle->eventQueue));
        free(eventHandle);
        return NULL;
    }
#if 1
    memoryAllocs = bl_hashmap_create_stack(100, sizeof(size_t), sizeof(size_t), false, bl_hashfunction_defualt_single_var);
    if (!bl_hashmap_is_valid(&memoryAllocs)) {
        bl_hashmap_destroy(&(eventHandle->eventSubscribers), NULL, NULL);
        arrayListDestroy(&(eventHandle->eventQueue));
        mutexDestroy(&eventHandle->mutexForSubscriber);
        free(eventHandle);
        return NULL;
    }
#endif // _DEBUG


    eventHandle->eventThread = threadCreate(eventHandle,(int(*)(void*))internal_eventMainLoop);
    if (!threadIsValid(&eventHandle->eventThread)){
        bl_hashmap_destroy(&(eventHandle->eventSubscribers), NULL, NULL);
        arrayListDestroy(&(eventHandle->eventQueue));
        mutexDestroy(&eventHandle->mutexForSubscriber);
        free(eventHandle);
#if 1
        bl_hashmap_destroy(&memoryAllocs, NULL, NULL);
#endif // _DEBUG

        return NULL;
    }
    atexit(eventSystemShutdown);
    mainEventHandle = eventHandle;
    return eventHandle;
}

static void internal_freeMemAllocsKey(void* key) { fprintf(stderr, "Memory alloc 0x%zx was not freed ", (size_t) key); }

static void internal_freeMemAllocsElements(void* element) { fprintf(stderr, "of size %zu\n", (size_t) element); }

void eventSystemShutdown(void) {
    mainEventHandle->shouldQuit = true;
    threadJoin(&mainEventHandle->eventThread);
    EventHandle* eventHandleToDestroy = mainEventHandle;
    mainEventHandle                   = NULL;
    mutexDestroy(&eventHandleToDestroy->mutexForSubscriber);
    arrayListDestroy(&eventHandleToDestroy->eventQueue);
    bl_hashmap_destroy(&eventHandleToDestroy->eventSubscribers, NULL, NULL);

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
    fprintf(stderr, ANSI_TEXT_RED "Memory leaks:" ANSI_TEXT_GREEN "\n");
    bl_hashmap_destroy(&memoryAllocs, internal_freeMemAllocsKey, internal_freeMemAllocsElements);
    fprintf(stderr, ANSI_TEXT_YELLOW "End of memory leaks." ANSI_RESET_ATTRIBUTE "\n");
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
            return false;
        }
    }

    if (outputToStdout) {
        debugFileStreams[0] = stdout;
        errorFileStreams[0] = stdout;
        debugFileStreams[1] = debugLog;
    } else { debugFileStreams[0] = debugLog; }

    if (outputErrorsToStdErr) {
        errorFileStreams[0] = stderr;
        errorFileStreams[1] = errorLog;
    } else if (outputToStdout) { errorFileStreams[1] = errorLog; } else { errorFileStreams[0] = errorLog; }

    eventRegLogSubToID(mainEventHandle, InfoLogLevel, writeEventToLogLocations, false, (debugLog != NULL) + outputToStdout, debugFileStreams);
    eventRegLogSubToID(mainEventHandle, DebugLogLevel, writeEventToLogLocations, false, (debugLog != NULL) + outputToStdout, debugFileStreams);
    eventRegLogSubToID(mainEventHandle, WarningLogLevel, writeEventToLogLocations, false, (errorLog != NULL) + (outputToStdout || outputErrorsToStdErr), errorFileStreams);
    eventRegLogSubToID(mainEventHandle, ErrorLogLevel, writeEventToLogLocations, false, (errorLog != NULL) + (outputToStdout || outputErrorsToStdErr), errorFileStreams);
    eventRegLogSubToID(mainEventHandle, CriticalLogLevel, writeEventToLogLocations, false, (errorLog != NULL) + (outputToStdout || outputErrorsToStdErr), errorFileStreams);

    return true;
}

EventError_t internal_invokeSubscriber(const EventCall* eventCall, BL_StringView subscriberID, const EventSubscriber* eventSubscriber) {
    if (eventSubscriber->eventSubscriberMain.flags & EVENT_FLAG_CREATE_THREAD) { assert("Not implemented"); } else {
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

        threadSleep(&timeToSleep, NULL);

        mutexLock(&eventHandle->eventQueue.mutex);
        mutexLock(&eventHandle->mutexForSubscriber);

        for (EventCall* currentCall = eventHandle->eventQueue.dynamicContainer.container.array; currentCall < ((EventCall*) eventHandle->eventQueue.dynamicContainer.container.array + eventHandle->eventQueue.dynamicContainer.container.amountOfIndexes); currentCall++) {
            EventSubscriber* mainSubscriber = bl_hashmap_get(&eventHandle->eventSubscribers, sizeof(BL_StringView), &currentCall->eventCallMain.id);
            if (mainSubscriber == NULL) {} else { if (internal_invokeSubscriber(currentCall, currentCall->eventCallMain.id, mainSubscriber) != EventOperationSuccess); }

            for (uint32_t groupCallIterator = 0; groupCallIterator < currentCall->eventCallMain.amountOfGroups; groupCallIterator++) {
                EventSubscriber* groupSubscriber = bl_hashmap_get(&eventHandle->eventSubscribers, sizeof(BL_StringView), currentCall->eventCallMain.groupIds + groupCallIterator);
                if (groupSubscriber == NULL) { continue; }
                if (internal_invokeSubscriber(currentCall, currentCall->eventCallMain.groupIds[groupCallIterator], groupSubscriber) != EventOperationSuccess);
            }
        }

        bl_container_dynamic_clear(&eventHandle->eventQueue.dynamicContainer);

        mutexUnlock(&eventHandle->eventQueue.mutex);
        mutexUnlock(&eventHandle->mutexForSubscriber);
    }
    return 0;
}

EventError_t eventRegSubToID(EventHandle* eventHandle, const BL_StringView ID, void (*function)(EventCall, BL_StringView), bool shouldRunOnSeperateThread) {
    if (mutexLock(&eventHandle->mutexForSubscriber) == ConcurrencyFailure)
        return EventOperationFailure;

    EventSubscriber eventSubscriberToReg = {.eventSubscriberMain = {.eventSubscriberType = EventSubscriberNormal, .function = function, .flags = (shouldRunOnSeperateThread) ? EVENT_FLAG_CREATE_THREAD : 0}};

    if (bl_hashmap_insert(&eventHandle->eventSubscribers, sizeof ID, &ID, sizeof eventSubscriberToReg, &eventSubscriberToReg) != BL_ContainerOPSuccessful) {
        mutexUnlock(&eventHandle->mutexForSubscriber);
        logCriticalCall("Cannot register id to eventSubscribers", __LINE__, __FILE__);
        return EventCritError;
    }

    mutexUnlock(&eventHandle->mutexForSubscriber);
    return EventOperationSuccess;
}

EventError_t eventRegLogSubToID(EventHandle* eventHandle, BL_StringView ID, void (*function)(EventCall, BL_StringView, size_t, FILE**), bool shouldRunOnSeperateThread, size_t amountOfOutputs, FILE** outputs) {
    if (mutexLock(&eventHandle->mutexForSubscriber))
        return EventOperationFailure;

    EventSubscriber eventSubscriberToReg = {.eventSubscriberLog = {.eventSubscriberType = EventSubscriberLogging,
                                                                   .amountOfOutputFiles = amountOfOutputs,
                                                                   .outputFiles = outputs,
                                                                   .function = function,
                                                                   .flags = (shouldRunOnSeperateThread) ? EVENT_FLAG_CREATE_THREAD : 0}};

    if (bl_hashmap_insert(&eventHandle->eventSubscribers, sizeof ID, &ID, sizeof eventSubscriberToReg, &eventSubscriberToReg) != BL_ContainerOPSuccessful) {
        mutexUnlock(&eventHandle->mutexForSubscriber);
        logCriticalCall("Cannot register id to eventSubscribers", __LINE__, __FILE__);
        return EventCritError;
    }

    mutexUnlock(&eventHandle->mutexForSubscriber);
    return EventOperationSuccess;
}


void eventCall(const Event* const event) {
    EventCall eventCallToSend = {
        .eventCallMain = {.eventType = EventNormal,
                          .id = event->id,
                          .groupIds = event->groupIds,
                          .amountOfGroups = event->amountOfGroups,
                          .callerThread = threadGetCurrent()}};
    if (!mainEventHandle)
        return;

    if (mutexLock(&mainEventHandle->eventQueue.mutex) == ConcurrencyFailure)
        return;

    bl_container_dynamic_append(&mainEventHandle->eventQueue.dynamicContainer, sizeof eventCallToSend, &eventCallToSend);

    mutexUnlock(&mainEventHandle->eventQueue.mutex);
}

void logCall(const Event* const event, uint32_t line, const char* file) {
    EventCall eventCallToSend = {
        .eventCallLog = {
            .line = line,
            .file = file,
            .eventCallMain = {.eventType = EventLogEvent,
                              .id = event->id,
                              .groupIds = event->groupIds,
                              .amountOfGroups = event->amountOfGroups,
                              .callerThread = threadGetCurrent()}}};
    if (!mainEventHandle)
        return;

    if (mutexLock(&mainEventHandle->eventQueue.mutex) == ConcurrencyFailure)
        return;

    bl_container_dynamic_append(&mainEventHandle->eventQueue.dynamicContainer, sizeof eventCallToSend, &eventCallToSend);

    mutexUnlock(&mainEventHandle->eventQueue.mutex);
}

BL_StringView  freeMemDoesNotExistGroupIds[] = {MemoryGroupId, ErrorLogLevel};
BL_StringView  allocEventsGroupIds[]         = {MemoryGroupId, DebugLogLevel, AllocMemoryGroupId};
BL_StringView  freeEventGroupId[]            = {DebugLogLevel, MemoryGroupId, DeallocMemoryGroupId};
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
            .allocSize = size,
            .address = address,
            .eventCallLog = {
                .line = line,
                .file = file,
                .eventCallMain = {.eventType = EventMemLogEvent,
                                  .id = event->id,
                                  .groupIds = event->groupIds,
                                  .amountOfGroups = event->amountOfGroups,
                                  .callerThread = threadGetCurrent()}}}};
    if (!mainEventHandle)
        return;

    if (mutexLock(&mainEventHandle->eventQueue.mutex) == ConcurrencyFailure)
        return;

    bl_container_dynamic_append(&mainEventHandle->eventQueue.dynamicContainer, sizeof eventCallToSend, &eventCallToSend);

    mutexUnlock(&mainEventHandle->eventQueue.mutex);
}


void* mallocLogVersion(size_t size, uint32_t line, const char* file) {
    void* memory = malloc(size);
    if (memory == NULL)
        return NULL;

    internal_memoryLogCall(&mallocEvent, line, file, size, (size_t) memory);
    if (bl_hashmap_is_valid(&memoryAllocs))
        bl_hashmap_insert(&memoryAllocs, sizeof memory, &memory, sizeof size, &size);
    return memory;
}

void* callocLogVersion(size_t amount, size_t size, uint32_t line, const char* file) {
    void* memory = calloc(amount, size);
    if (memory == NULL)
        return NULL;

    internal_memoryLogCall(&callocEvent, line, file, size * amount, (size_t) memory);
    if (bl_hashmap_is_valid(&memoryAllocs))
        bl_hashmap_insert(&memoryAllocs, sizeof memory, &memory, sizeof size, &size);
    return memory;
}

void* reallocLogVersion(void* currentPtr, size_t size, uint32_t line, const char* file) {
    if (currentPtr == NULL)
        return mallocLogVersion(size, line, file);

    void* memory = realloc(currentPtr, size);
    if (memory == NULL)
        return NULL;

    internal_memoryLogCall(&reallocEvent, line, file, size, (size_t) memory);
    if (bl_hashmap_is_valid(&memoryAllocs)) {
        if (currentPtr == memory)
            bl_hashmap_replace(&memoryAllocs, sizeof memory, &memory, sizeof size, &size, NULL);
        else {
            bl_hashmap_remove(&memoryAllocs, sizeof currentPtr, &currentPtr, NULL, NULL);
            bl_hashmap_insert(&memoryAllocs, sizeof memory, &memory, sizeof size, &size);
        }
    }
    return memory;
}

void freeLogVersion(void* ptr, uint32_t line, const char* file) {
    if (!bl_hashmap_is_valid(&memoryAllocs)) {
        free(ptr);
        return;
    }
    size_t size = 0;
    if (bl_hashmap_get(&memoryAllocs, sizeof ptr, &ptr) == NULL) {
        logCall(&freeMemDoesNotExist, line, file);
        return;
    }
    internal_memoryLogCall(&freeEvent, line, file, size, (size_t) ptr);
    bl_hashmap_remove(&memoryAllocs, sizeof ptr, &ptr, NULL, NULL);
    free(ptr);
}

void writeEventToLogLocations(EventCall event, BL_StringView logLevel, size_t amountOfLocations, FILE** locations) {
    switch (event.type) {
    case EventNormal:
        for (size_t i = 0; i < amountOfLocations; i++)
            fprintf(locations[i], ANSI_TEXT_YELLOW "%" PRIi64 ANSI_TEXT_WHITE "[%s]" ANSI_TEXT_MAGENTA "%s" ANSI_RESET_ATTRIBUTE "\n", time(NULL), logLevel.array, event.eventCallMain.id.array);
        break;
    case EventLogEvent:
        for (size_t i = 0; i < amountOfLocations; i++)
            fprintf(locations[i], ANSI_TEXT_YELLOW "%" PRIi64 ANSI_TEXT_WHITE "[%s] " ANSI_TEXT_MAGENTA "%s" ANSI_RESET_ATTRIBUTE " in file: " ANSI_TEXT_CYAN ANSI_TEXT_BOLD "%s" ANSI_RESET_ATTRIBUTE " at line: " ANSI_TEXT_BLUE "%zu" ANSI_RESET_ATTRIBUTE "\n", time(NULL), logLevel.array, event.eventCallMain.id.array, event.eventCallLog.file, event.eventCallLog.line);
        break;
    case EventMemLogEvent:
        for (size_t i = 0; i < amountOfLocations; i++) {
            fprintf(
                locations[i],
                ANSI_TEXT_YELLOW "%" PRIi64 ANSI_TEXT_WHITE "[%s]" ANSI_TEXT_MAGENTA "%s" ANSI_RESET_ATTRIBUTE " of size: " ANSI_TEXT_YELLOW "%zu" ANSI_RESET_ATTRIBUTE " at address: " ANSI_TEXT_GREEN "0x%zx" ANSI_RESET_ATTRIBUTE " in file: " ANSI_TEXT_CYAN ANSI_TEXT_BOLD "%s" ANSI_RESET_ATTRIBUTE " at line: " ANSI_TEXT_BLUE "%zu" ANSI_RESET_ATTRIBUTE "\n",
                time(NULL),
                logLevel.array,
                event.eventCallMain.id.array,
                event.eventCallMemory.allocSize,
                event.eventCallMemory.address,
                event.eventCallLog.file,
                event.eventCallLog.line);
        }
        break;
    default:
        break;
    }
}
