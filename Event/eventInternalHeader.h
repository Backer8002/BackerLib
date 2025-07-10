#pragma once
#pragma once
#include "../include/BackerLibTypes.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <threads.h>

#define EVENT_FLAG_ADDITIONAL_ARGS 0x1
#define EVENT_FLAG_CREATE_THREAD   0x2


typedef enum {
    EventOperationSuccess = 0,
    EventUnableToCreateThread,
    EventOperationFailure,
    EventCritError
} EventError_t;

typedef enum {
    EventNormal,
    EventLogEvent,
    EventMemLogEvent
} EventType;

typedef enum {
    EventSubscriberNormal,
    EventSubscriberLogging
} EventSubscriberType;

typedef struct {
    EventType    eventType;
    const char*  id;
    const char** groupIds;
    thrd_t       callerThread;
    uint32_t     amountOfGroups;
} EventCallMain;

typedef struct {
    EventCallMain eventCallMain;
    size_t        line;
    const char*   file;
} EventCallLog;

typedef struct {
    EventCallLog eventCallLog;
    size_t       allocSize;
    size_t       address;
} EventCallMemory;

typedef union {
    EventType       type;
    EventCallMain   eventCallMain;
    EventCallLog    eventCallLog;
    EventCallMemory eventCallMemory;
} EventCall;

typedef struct {
    const char*  id;
    const char** groupIds;
    uint32_t     amountOfGroups;
} Event;

typedef struct {
    EventSubscriberType eventSubscriberType;
    uint32_t            flags;
    void (*function)(EventCall, const char*);
} EventSubscriberMain;

typedef struct {
    EventSubscriberType eventSubscriberType;
    uint32_t            flags;
    void (*function)(EventCall, const char*, size_t, FILE**);
    FILE** outputFiles;
    size_t amountOfOutputFiles;
} EventSubscriberLog;

typedef union {
    EventSubscriberType typeOfSubscriber;
    EventSubscriberMain eventSubscriberMain;
    EventSubscriberLog  eventSubscriberLog;
} EventSubscriber;

typedef struct {
    ArrayList     eventQueue;
    HashMapCuckoo eventSubscribers;
    thrd_t        eventThread;
    bool          shouldQuit;
} EventHandle;

extern EventHandle* eventSystemInit(void);
extern void         loggingInit(EventHandle* eventHandle);
extern EventError_t eventRegSubToID(EventHandle* eventHandle, const char* ID, void (*function)(EventCall, const char*), bool shouldRunOnSeperateThread);
extern EventError_t eventRegLogSubToID(EventHandle* eventHandle, const char* ID, void (*function)(EventCall, const char*, size_t, FILE**), bool shouldRunOnSeperateThread, size_t amountOfOutputs, FILE** outputs);

extern void         eventCall(const Event* const event, thrd_t currentThread);
extern void         logCall(const Event* const event, thrd_t currentThread, uint32_t line, const char* file);

#ifdef _DEBUG

extern void* mallocLogVersion(size_t size, uint32_t line, const char* file);
extern void* callocLogVersion(size_t amount, size_t size, uint32_t line, const char* file);
extern void* reallocLogVersion(void* currentPtr, size_t size, uint32_t line, const char* file);
extern void  freeLogVersion(void* ptr, uint32_t line, const char* file);

#define malloc(size)              mallocLogVersion(size, (uint32_t) (__LINE__), __FILE__)
#define calloc(amount, size)      callocLogVersion(amount, size, (uint32_t) (__LINE__), __FILE__)
#define realloc(currentPtr, size) reallocLogVersion(currentPtr, size, (uint32_t) (__LINE__), __FILE__)
#define free(ptr)                 freeLogVersion(ptr, (uint32_t) (__LINE__), __FILE__)
#endif

extern void writeEventToLogLocations(EventCall event, const char* logLevel, size_t amountOfLocations, FILE** locations);
extern void logInfoCall(EventCall*);
extern void logDebug(EventCall*);
extern void logWarnCall(EventCall*);
extern void logErrorCall(EventCall*);
extern void logCriticalCall(EventCall*);


#define LogInfo()  logInfoCall(NULL)
#define LogDebug() logDebug(NULL)
#define LogWarn()  logWarnCall(NULL)
#define LogError() logErrorCall(NULL)
#define LogCrit()  logCriticalCall(NULL)

static thrd_t noThread = {._Handle = NULL, ._Tid = 0};