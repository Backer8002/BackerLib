#pragma once

#include "../Event/eventInternalHeader.h"
#include "BackerLibCommonEvents.h"

#define EventHandle void
#define EventSubscriberMain void
#define EventSubscriberLog void
#define EventSubscriber void
#define EventSubscriberType void

// Puts event in the event queue given currentThread as the caller. Current line
// and file are appended in the call
#define logCall(event, thread) logCall(event, thread, __LINE__, __FILE__)

#define LogWarn(message) logWarnCall(message, __LINE__, __FILE__)
#define LogError(message) logErrorCall(message, __LINE__, __FILE__)
#define LogCrit(message) logCriticalCall(message, __LINE__, __FILE__)

#ifndef _SHIPPING
#define LogInfo(message) logInfoCall(message, __LINE__, __FILE__)
#define LogDebug(message) logDebugCall(message, __LINE__, __FILE__)
#define LogWarnDebug(message) LogWarn(message)
#define logWarnDebugCall(message,line,file) logWarnCall(message,line,file)
#else
#define LogInfo(x)
#define LogDebug(x)
#define LogWarnDebug(x)

#define logInfoCall(x,y,z)
#define logDebugCall(x,y,z)
#define logWarnDebugCall(x,y,z)
#endif
