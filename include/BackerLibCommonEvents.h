#pragma once
#include "../Event/eventInternalHeader.h"


static BL_StringView InfoLogLevel = stringViewInitConstExpr("Info");
static BL_StringView DebugLogLevel = stringViewInitConstExpr("Debug");
static BL_StringView WarningLogLevel = stringViewInitConstExpr("Warning");
static BL_StringView ErrorLogLevel = stringViewInitConstExpr("Error");
static BL_StringView CriticalLogLevel = stringViewInitConstExpr("Critical");
static BL_StringView MemoryGroupId = stringViewInitConstExpr("Memory");
static BL_StringView AllocMemoryGroupId = stringViewInitConstExpr("AllocMem");
static BL_StringView DeallocMemoryGroupId = stringViewInitConstExpr("DeallocMem");

static BL_StringView LogUnableToAllocMemGroupIds[] = {WarningLogLevel,
                                                          MemoryGroupId};
static const Event LogUnableToAllocMem = {
    .id = stringViewInitConstExpr("Unable to allocate memory"),
    .amountOfGroups = 2,
    .groupIds = LogUnableToAllocMemGroupIds};
