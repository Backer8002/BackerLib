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

#ifndef BL_LOGGING_BUFFERSIZE
#define BL_LOGGING_BUFFERSIZE 1
#endif

#define BL_LOGLEVEL_TRACE 0x1
#define BL_LOGLEVEL_DEBUG 0x2
#define BL_LOGLEVEL_INFO 0x4
#define BL_LOGLEVEL_WARN 0x8
#define BL_LOGLEVEL_ERROR 0x10
#define BL_LOGLEVEL_CRITICAL 0x20
#define BL_LOG_IS_STD_STREAM 0x80
#define BL_LOGLEVEL_ALL (BL_LOGLEVEL_CRITICAL | BL_LOGLEVEL_DEBUG | BL_LOGLEVEL_ERROR | BL_LOGLEVEL_INFO | BL_LOGLEVEL_TRACE | BL_LOGLEVEL_WARN)
#define BL_LOGLEVEL_IMPORTANT (BL_LOGLEVEL_CRITICAL | BL_LOGLEVEL_ERROR | BL_LOGLEVEL_WARN | BL_LOGLEVEL_INFO)

#define TRACE_CHAR 0xf8
#define DEBUG_CHAR 0xf9
#define INFO_CHAR 0xfa
#define WARN_CHAR 0xfb
#define ERROR_CHAR 0xfc
#define CRITICAL_CHAR 0xfd

extern time_t bl_beginTime;
/**
 * @brief Saves a log to the buffer. Should begin with one of the chars above.
 * @param format Format string for va_args
 */
void bl_log(const char* format,...) noexcept;
/**
 * @brief Saves a log to the buffer.Should begin with one of the chars above.
 * @param format Format string for args
 * @param args for format string
 */
void bl_vlog(const char* format, va_list args) noexcept;
/**
 * @brief Runs bl_vlog if condition is false.
 */
void bl_assert(bool condition,const char* format,...) noexcept;
/**
 * @private
 * @brief Writes buffer to file streams.
 */
void bl_internal_write_log(const char* buffer,size_t amountToWrite) noexcept;
/**
 * @brief Flushes internal buffer. Like fflush but for logs buffer.
 */
void bl_log_flush(void) noexcept;
/**
 * @brief Intializes logging. It is not thread safe. Returns Bl_ContainerOPSuccess on successful initialization.
 */
BL_ContainerError bl_log_init(void) noexcept;
/**
 * @brief Registers file for logging and takes ownership of it.
 * @param file to register
 * @param flags Either BL_LOGLEVEL_* for loglevels to write to file or BL_LOG_IS_STD_STREAM to indicate that it is either stdout or stderr.
 */
BL_ContainerError bl_log_register(FILE* file,uint8_t flags) noexcept;

#ifdef __cplusplus
    }
};
#else
#undef noexcept
#endif
#endif
