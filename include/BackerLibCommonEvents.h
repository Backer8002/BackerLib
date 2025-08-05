#pragma once
#include "../Event/eventInternalHeader.h"


static const char InfoLogLevel[] = "Info";
static const char DebugLogLevel[] = "Debug";
static const char WarningLogLevel[] = "Warning";
static const char ErrorLogLevel[] = "Error";
static const char CriticalLogLevel[] = "Critical";
static const char MemoryGroupId[] = "Memory";
static const char AllocMemoryGroupId[] = "AllocMem";
static const char DeallocMemoryGroupId[] = "DeallocMem";

static const char* LogUnableToAllocMemGroupIds[] = {WarningLogLevel,
                                                          MemoryGroupId};
static const Event LogUnableToAllocMem = {
    .id = "Unable to allocate memory",
    .amountOfGroups = 2,
    .groupIds = LogUnableToAllocMemGroupIds};
