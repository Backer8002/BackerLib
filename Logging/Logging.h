#ifndef BL_LOGGING
#define BL_LOGGING

#include "Logging_Internal.h"
#include <time.h>

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#else
#define noexcept
#endif

#ifdef BL_LOGGING_TRACE
#define bl_log_trace(fmtMsg,...) bl_log("%c%ld%s"(fmtMsg),TRACE_CHAR,time()," [TRACE] | ",##__VA_ARGS__) 
#define bl_log_trace_location(fmtMsg,...) bl_log("%c%ld%s%s:%zu %s | "(fmtMsg),TRACE_CHAR,time()," [TRACE] ",__FILE__,__LINE__,__FUNC__,##__VA_ARGS__)
#define bl_assert_trace(cond,fmtMsg,...) bl_assert((cond),"%c%ld%s%s:%zu %s | "(fmtMsg),TRACE_CHAR,time()," [TRACE] ",__FILE__,__LINE__,__FUNC__,##__VA_ARGS__)
#else
#define bl_log_trace(fmtMsg,...)
#define bl_log_trace_location(fmtMsg,...)
#define bl_assert_trace(cond,fmtMsg,...)
#endif

#ifdef BL_LOGGING_DEBUG
#define bl_log_debug(fmtMsg,...) bl_log("%c%ld%s"(fmtMsg),DEBUG_CHAR,time()," [DEBUG] | ",##__VA_ARGS__) 
#define bl_log_debug_location(fmtMsg,...) bl_log("%c%ld%s%s:%zu %s | "(fmtMsg),DEBUG_CHAR,time()," [DEBUG] ",__FILE__,__LINE__,__FUNC__,##__VA_ARGS__)
#define bl_assert_debug(cond,fmtMsg,...) bl_assert((cond),"%c%ld%s%s:%zu %s | "(fmtMsg),DEBUG_CHAR,time()," [DEBUG] ",__FILE__,__LINE__,__FUNC__,##__VA_ARGS__)
#else
#define bl_log_debug(fmtMsg,...)
#define bl_log_debug_location(fmtMsg,...)
#define bl_assert_debug(cond,fmtMsg,...)
#endif

#ifdef BL_LOGGING_INFO
#define bl_log_info(fmtMsg,...) bl_log("%c%ld%s"(fmtMsg),INFO_CHAR,time()," [INFO] | ",##__VA_ARGS__) 
#define bl_log_info_location(fmtMsg,...) bl_log("%c%ld%s%s:%zu %s | "(fmtMsg),INFO_CHAR,time()," [INFO] ",__FILE__,__LINE__,__FUNC__,##__VA_ARGS__)
#define bl_assert_info(cond,fmtMsg,...) bl_assert((cond),"%c%ld%s%s:%zu %s | "(fmtMsg),INFO_CHAR,time()," [INFO] ",__FILE__,__LINE__,__FUNC__,##__VA_ARGS__)
#else
#define bl_log_info(fmtMsg,...)
#define bl_log_info_location(fmtMsg,...)
#define bl_assert_info(cond,fmtMsg,...)
#endif

#ifdef BL_LOGGING_WARN
#define bl_log_warn(fmtMsg,...) bl_log("%c%ld%s"(fmtMsg),WARN_CHAR,time()," [WARNING] | ",##__VA_ARGS__) 
#define bl_log_warn_location(fmtMsg,...) bl_log("%c%ld%s%s:%zu %s | "(fmtMsg),WARN_CHAR,time()," [WARNING] ",__FILE__,__LINE__,__FUNC__,##__VA_ARGS__)
#define bl_assert_warn(cond,fmtMsg,...) bl_assert((cond),"%c%ld%s%s:%zu %s | "(fmtMsg),WARN_CHAR,time()," [WARNING] ",__FILE__,__LINE__,__FUNC__,##__VA_ARGS__)
#else
#define bl_log_warn(fmtMsg,...)
#define bl_log_warn_location(fmtMsg,...)
#define bl_assert_warn(cond,fmtMsg,...)
#endif

#ifdef BL_LOGGING_ERROR
#define bl_log_error(fmtMsg,...) bl_log("%c%ld%s"(fmtMsg),ERROR_CHAR,time()," [ERROR] | ",##__VA_ARGS__) 
#define bl_log_error_location(fmtMsg,...) bl_log("%c%ld%s%s:%zu %s | "(fmtMsg),ERROR_CHAR,time()," [ERROR] ",__FILE__,__LINE__,__FUNC__,##__VA_ARGS__)
#define bl_assert_error(cond,fmtMsg,...) bl_assert((cond),"%c%ld%s%s:%zu %s | "(fmtMsg),ERROR_CHAR,time()," [ERROR] ",__FILE__,__LINE__,__FUNC__,##__VA_ARGS__)
#else
#define bl_log_error(fmtMsg,...)
#define bl_log_error_location(fmtMsg,...)
#define bl_assert_error(cond,fmtMsg,...)
#endif

#ifdef BL_LOGGING_CRITICAL
#define bl_log_critical(fmtMsg,...) bl_log("%c%ld%s"(fmtMsg),CRITICAL_CHAR,time()," [CRITICAL] | ",##__VA_ARGS__) 
#define bl_log_critical_location(fmtMsg,...) bl_log("%c%ld%s%s:%zu %s | "(fmtMsg),CRITICAL_CHAR,time()," [CRITICAL] ",__FILE__,__LINE__,__FUNC__,##__VA_ARGS__)
#define bl_assert_critical(cond,fmtMsg,...) bl_assert((cond),"%c%ld%s%s:%zu %s | "(fmtMsg),CRITICAL_CHAR,time()," [CRITICAL] ",__FILE__,__LINE__,__FUNC__,##__VA_ARGS__)
#else
#define bl_log_critical(fmtMsg,...)
#define bl_log_critical_location(fmtMsg,...)
#define bl_assert_critical(cond,fmtMsg,...)
#endif

#ifdef __cplusplus
    }
};
#else
#undef noexcept
#endif
#endif
