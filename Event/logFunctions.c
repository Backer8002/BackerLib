#include <BackerLibEvent.h>

#undef logCall

static BL_StringView InfoLogGroupIDs[]     = {InfoLogLevel};
static BL_StringView DebugLogGroupIDs[]    = {DebugLogLevel};
static BL_StringView WarningLogGroupIDs[]  = {WarningLogLevel};
static BL_StringView ErrorLogGroupIDs[]    = {ErrorLogLevel};
static BL_StringView CriticalLogGroupIDs[] = {CriticalLogLevel};

void logInfoCall(const char* message, uint32_t line, const char* file) {
    Event event = {.id = bl_stringview_init(message), .groupIds = InfoLogGroupIDs, .amountOfGroups = 1};
    logCall(&event, line, file);
}

void logDebugCall(const char* message, uint32_t line, const char* file) {
    Event event = {.id = bl_stringview_init(message), DebugLogGroupIDs, 1};
    logCall(&event, line, file);
}

void logWarnCall(const char* message, uint32_t line, const char* file) {
    Event event = {.id = bl_stringview_init(message), WarningLogGroupIDs, 1};
    logCall(&event, line, file);
}

void logErrorCall(const char* message, uint32_t line, const char* file) {
    Event event = {.id = bl_stringview_init(message), ErrorLogGroupIDs, 1};
    logCall(&event, line, file);
}

void logCriticalCall(const char* message, uint32_t line, const char* file) {
    Event event = {.id = bl_stringview_init(message), CriticalLogGroupIDs, 1};
    logCall(&event, line, file);
}