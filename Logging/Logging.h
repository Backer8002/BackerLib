#ifndef BL_LOGGING
#define BL_LOGGING

#include "Logging_Internal.h"
#include <time.h>

#ifdef __cplusplus
using BackerLib::bl_log,BackerLib::bl_assert,BackerLib::bl_beginTime;
#endif

#ifdef BL_LOGGING_TRACE
#define bl_log_trace(fmtMsg,...) bl_log("%c%lds %s" fmtMsg,TRACE_CHAR,time(NULL)-bl_beginTime," [TRACE] | ",##__VA_ARGS__) 
#define bl_log_trace_location(fmtMsg,...) bl_log("%c%lds %s:%zu %s()%s| " fmtMsg,TRACE_CHAR,time(NULL)-bl_beginTime,__FILE__,__LINE__,__func__," [TRACE] ",##__VA_ARGS__)
#define bl_assert_trace(cond,fmtMsg,...) bl_assert((cond),"%c%lds %s:%zu %s()%s| " fmtMsg,TRACE_CHAR,time(NULL)-bl_beginTime,__FILE__,__LINE__,__func__," [TRACE] ",##__VA_ARGS__)
#else
#define bl_log_trace(fmtMsg,...)
#define bl_log_trace_location(fmtMsg,...)
#define bl_assert_trace(cond,fmtMsg,...)
#endif

#ifdef BL_LOGGING_DEBUG
#define bl_log_debug(fmtMsg,...) bl_log("%c%lds %s" fmtMsg,DEBUG_CHAR,time(NULL)-bl_beginTime," [DEBUG] | ",##__VA_ARGS__) 
#define bl_log_debug_location(fmtMsg,...) bl_log("%c%lds %s:%zu %s()%s| " fmtMsg,DEBUG_CHAR,time(NULL)-bl_beginTime,__FILE__,__LINE__,__func__," [DEBUG] ",##__VA_ARGS__)
#define bl_assert_debug(cond,fmtMsg,...) bl_assert((cond),"%c%lds %s:%zu %s()%s| " fmtMsg,DEBUG_CHAR,time(NULL)-bl_beginTime,__FILE__,__LINE__,__func__," [DEBUG] ",##__VA_ARGS__)
#else
#define bl_log_debug(fmtMsg,...)
#define bl_log_debug_location(fmtMsg,...)
#define bl_assert_debug(cond,fmtMsg,...)
#endif

#ifdef BL_LOGGING_INFO
#define bl_log_info(fmtMsg,...) bl_log("%c%lds %s" fmtMsg,INFO_CHAR,time(NULL)-bl_beginTime," [INFO] | ",##__VA_ARGS__) 
#define bl_log_info_location(fmtMsg,...) bl_log("%c%lds %s:%zu %s()%s| " fmtMsg,INFO_CHAR,time(NULL)-bl_beginTime,__FILE__,__LINE__,__func__," [INFO] ",##__VA_ARGS__)
#define bl_assert_info(cond,fmtMsg,...) bl_assert((cond),"%c%lds %s:%zu %s()%s| " fmtMsg,INFO_CHAR,time(NULL)-bl_beginTime,__FILE__,__LINE__,__func__," [INFO] ",##__VA_ARGS__)
#else
#define bl_log_info(fmtMsg,...)
#define bl_log_info_location(fmtMsg,...)
#define bl_assert_info(cond,fmtMsg,...)
#endif

#ifdef BL_LOGGING_WARN
#define bl_log_warn(fmtMsg,...) bl_log("%c%lds %s" fmtMsg,WARN_CHAR,time(NULL)-bl_beginTime," [WARNING] | ",##__VA_ARGS__) 
#define bl_log_warn_location(fmtMsg,...) bl_log("%c%lds %s:%zu %s()%s| " fmtMsg,WARN_CHAR,time(NULL)-bl_beginTime,__FILE__,__LINE__,__func__," [WARNING] ",##__VA_ARGS__)
#define bl_assert_warn(cond,fmtMsg,...) bl_assert((cond),"%c%lds %s:%zu %s()%s| " fmtMsg,WARN_CHAR,time(NULL)-bl_beginTime,__FILE__,__LINE__,__func__," [WARNING] ",##__VA_ARGS__)
#else
#define bl_log_warn(fmtMsg,...)
#define bl_log_warn_location(fmtMsg,...)
#define bl_assert_warn(cond,fmtMsg,...)
#endif

#ifdef BL_LOGGING_ERROR
#define bl_log_error(fmtMsg,...) bl_log("%c%lds %s" fmtMsg,ERROR_CHAR,time(NULL)-bl_beginTime," [ERROR] | ",##__VA_ARGS__) 
#define bl_log_error_location(fmtMsg,...) bl_log("%c%lds %s:%zu %s()%s| " fmtMsg,ERROR_CHAR,time(NULL)-bl_beginTime,__FILE__,__LINE__,__func__," [ERROR] ",##__VA_ARGS__)
#define bl_assert_error(cond,fmtMsg,...) bl_assert((cond),"%c%lds %s:%zu %s()%s| " fmtMsg,ERROR_CHAR,time(NULL)-bl_beginTime,__FILE__,__LINE__,__func__," [ERROR] ",##__VA_ARGS__)
#else
#define bl_log_error(fmtMsg,...)
#define bl_log_error_location(fmtMsg,...)
#define bl_assert_error(cond,fmtMsg,...)
#endif

#ifdef BL_LOGGING_CRITICAL
#define bl_log_critical(fmtMsg,...) bl_log("%c%lds %s" fmtMsg,CRITICAL_CHAR,time(NULL)-bl_beginTime," [CRITICAL] | ",##__VA_ARGS__) 
#define bl_log_critical_location(fmtMsg,...) bl_log("%c%lds %s:%zu %s()%s| " fmtMsg,CRITICAL_CHAR,time(NULL)-bl_beginTime,__FILE__,__LINE__,__func__," [CRITICAL] ",##__VA_ARGS__)
#define bl_assert_critical(cond,fmtMsg,...) bl_assert((cond),"%c%lds %s:%zu %s()%s| " fmtMsg,CRITICAL_CHAR,time(NULL)-bl_beginTime,__FILE__,__LINE__,__func__," [CRITICAL] ",##__VA_ARGS__)
#else
#define bl_log_critical(fmtMsg,...)
#define bl_log_critical_location(fmtMsg,...)
#define bl_assert_critical(cond,fmtMsg,...)
#endif

#endif
