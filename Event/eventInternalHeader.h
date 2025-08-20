#pragma once

#include <BackerLibTypes.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

#define EVENT_FLAG_CREATE_THREAD 0x2

#ifdef _WIN32
static const thrd_t noThread = {0};
#else
#define noThread 0
#endif


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
    StringView  id;
    StringView* groupIds;
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
    StringView  id;
    StringView* groupIds;
    uint32_t     amountOfGroups;
} Event;

typedef struct {
    EventSubscriberType eventSubscriberType;
    uint32_t            flags;
    void (*function)(EventCall, StringView);
} EventSubscriberMain;

typedef struct {
    EventSubscriberType eventSubscriberType;
    uint32_t            flags;
    void (*function)(EventCall, StringView, size_t, FILE**);
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
    mtx_t         mutexForSubscriber;
    _Atomic(bool)  shouldQuit;
} EventHandle;

extern EventHandle* eventSystemInit(void);
extern void         eventSystemShutdown(void);
// Returns true if init was successful
extern bool         eventInitDefualtLog(const char* debugLogFile, bool outputToStdout, const char* errorLogFile, bool outputErrorsToStdErr);
// Registers function to be called when ID is intercepeted.
extern EventError_t eventRegSubToID(EventHandle* eventHandle, StringView ID, void (*function)(EventCall, StringView), bool shouldRunOnSeperateThread);
// Registers function to be called when ID is intercepted. This function also takes in an array of files when calling function.
extern EventError_t eventRegLogSubToID(EventHandle* eventHandle, StringView ID, void (*function)(EventCall, StringView, size_t, FILE**), bool shouldRunOnSeperateThread, size_t amountOfOutputs, FILE** outputs);

// Puts event in the event queue given currentThread as the caller
extern void         eventCall(const Event* const event, thrd_t currentThread);

extern void         logCall(const Event* const event, thrd_t currentThread, uint32_t line, const char* file);


#if 1

extern void* mallocLogVersion(size_t size, uint32_t line, const char* file);
extern void* callocLogVersion(size_t amount, size_t size, uint32_t line, const char* file);
extern void* reallocLogVersion(void* currentPtr, size_t size, uint32_t line, const char* file);
extern void  freeLogVersion(void* ptr, uint32_t line, const char* file);

#define malloc(size)              mallocLogVersion(size, (uint32_t) __LINE__, __FILE__)
#define calloc(amount, size)      callocLogVersion(amount, size, (uint32_t) __LINE__, __FILE__)
#define realloc(currentPtr, size) reallocLogVersion(currentPtr, size, (uint32_t) __LINE__, __FILE__)
#define free(ptr)                 freeLogVersion(ptr, (uint32_t) __LINE__, __FILE__)
#endif

extern void writeEventToLogLocations(EventCall event, StringView logLevel, size_t amountOfLocations, FILE** locations);
extern void logInfoCall(const char* restrict message, uint32_t line, const char* file);
extern void logDebugCall(const char* restrict message, uint32_t line, const char* file);
extern void logWarnCall(const char* restrict message, uint32_t line, const char* file);
extern void logErrorCall(const char* restrict message, uint32_t line, const char* file);
extern void logCriticalCall(const char* restrict message, uint32_t line, const char* file);
