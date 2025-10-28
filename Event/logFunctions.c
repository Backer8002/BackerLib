#include <BackerLibEvent.h>

#undef logCall

static StringView InfoLogGroupIDs[]     = {InfoLogLevel};
static StringView DebugLogGroupIDs[]    = {DebugLogLevel};
static StringView WarningLogGroupIDs[]  = {WarningLogLevel};
static StringView ErrorLogGroupIDs[]    = {ErrorLogLevel};
static StringView CriticalLogGroupIDs[] = {CriticalLogLevel};

void logInfoCall(const char* message, uint32_t line, const char* file) {
    Event event = {.id = stringViewInit(message), .groupIds = InfoLogGroupIDs, .amountOfGroups = 1};
    logCall(&event, line, file);
}

void logDebugCall(const char* message, uint32_t line, const char* file) {
    Event event = {.id = stringViewInit(message), DebugLogGroupIDs, 1};
    logCall(&event, line, file);
}

void logWarnCall(const char* message, uint32_t line, const char* file) {
    Event event = {.id = stringViewInit(message), WarningLogGroupIDs, 1};
    logCall(&event, line, file);
}

void logErrorCall(const char* message, uint32_t line, const char* file) {
    Event event = {.id = stringViewInit(message), ErrorLogGroupIDs, 1};
    logCall(&event, line, file);
}

void logCriticalCall(const char* message, uint32_t line, const char* file) {
    Event event = {.id = stringViewInit(message), CriticalLogGroupIDs, 1};
    logCall(&event, line, file);
}