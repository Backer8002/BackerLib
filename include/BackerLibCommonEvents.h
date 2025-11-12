#pragma once
#include "../Event/eventInternalHeader.h"


static const  BL_StringView InfoLogLevel = stringViewInitConstExpr("Info");
static const  BL_StringView DebugLogLevel = stringViewInitConstExpr("Debug");
static const BL_StringView WarningLogLevel = stringViewInitConstExpr("Warning");
static const BL_StringView ErrorLogLevel = stringViewInitConstExpr("Error");
static const BL_StringView CriticalLogLevel = stringViewInitConstExpr("Critical");
static const BL_StringView MemoryGroupId = stringViewInitConstExpr("Memory");
static const BL_StringView AllocMemoryGroupId = stringViewInitConstExpr("AllocMem");
static const BL_StringView DeallocMemoryGroupId = stringViewInitConstExpr("DeallocMem");

static const BL_StringView LogUnableToAllocMemGroupIds[] = {WarningLogLevel,
                                                          MemoryGroupId};
static const Event LogUnableToAllocMem = {
    .id = stringViewInitConstExpr("Unable to allocate memory"),
    .amountOfGroups = 2,
    .groupIds = LogUnableToAllocMemGroupIds};
