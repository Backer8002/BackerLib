#ifndef BL_LOGGING_INTERNAL
#define BL_LOGGING_INTERNAL


#include<BackerLibConcurrency.h>
#include<stdbool.h>
#include <stdarg.h>
#include <time.h>
#include <stdint.h>

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#else
#define noexcept
#endif

#define BL_LOGGING_BUFFERSIZE 5000

#define BL_LOGLEVEL_TRACE 0x1
#define BL_LOGLEVEL_DEBUG 0x2
#define BL_LOGLEVEL_INFO 0x4
#define BL_LOGLEVEL_WARN 0x8
#define BL_LOGLEVEL_ERROR 0x10
#define BL_LOGLEVEL_CRITICAL 0x20
#define BL_LOG_IS_STD_STREAM 0x80

#define TRACE_CHAR 0xf8
#define DEBUG_CHAR 0xf9
#define INFO_CHAR 0xfa
#define WARN_CHAR 0xfb
#define ERROR_CHAR 0xfc
#define CRITICAL_CHAR 0xfd

extern time_t bl_beginTime;

void bl_log(const char* format,...) noexcept;

void bl_vlog(const char* format, va_list args) noexcept;

void bl_assert(bool condition,const char* format,...) noexcept;

void bl_internal_write_log(const char* buffer,size_t amountToWrite) noexcept;

void bl_log_flush(void);

BL_ContainerError bl_log_init(void);

BL_ContainerError bl_log_register(FILE* file,uint8_t flags);

#ifdef __cplusplus
    }
};
#else
#undef noexcept
#endif
#endif
