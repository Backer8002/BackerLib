#pragma once
#include "../Event/eventInternalHeader.h"


static StringView InfoLogLevel = stringViewInitConstExpr("Info");
static StringView DebugLogLevel = stringViewInitConstExpr("Debug");
static StringView WarningLogLevel = stringViewInitConstExpr("Warning");
static StringView ErrorLogLevel = stringViewInitConstExpr("Error");
static StringView CriticalLogLevel = stringViewInitConstExpr("Critical");
static StringView MemoryGroupId = stringViewInitConstExpr("Memory");
static StringView AllocMemoryGroupId = stringViewInitConstExpr("AllocMem");
static StringView DeallocMemoryGroupId = stringViewInitConstExpr("DeallocMem");

static StringView LogUnableToAllocMemGroupIds[] = {WarningLogLevel,
                                                          MemoryGroupId};
static const Event LogUnableToAllocMem = {
    .id = stringViewInitConstExpr("Unable to allocate memory"),
    .amountOfGroups = 2,
    .groupIds = LogUnableToAllocMemGroupIds};
