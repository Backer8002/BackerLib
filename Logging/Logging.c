#include "Logging_Internal.h"
#include <BackerLibConcurrency.h>
#include <BackerLibTypes.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct LogFile {
    uint8_t flags;
    FILE*   file;
};

typedef struct BL_LogBuffer {
    unsigned char buffer[BL_LOGGING_BUFFERSIZE];
    size_t        bytesWriten;
} BL_LogBuffer;

static thread_local BL_LogBuffer logBuffer;

time_t                           bl_beginTime = 0;

static struct {
    BL_DynamicContainer files;
    BL_Mutex            logMutex;
    bool                isInited;
} LoggingInfo = {0};

static void internal_cleanup(void) {
    if (!LoggingInfo.isInited)
        return;

    for (struct LogFile* file = bl_container_dynamic_front(&LoggingInfo.files); file; file = bl_container_dynamic_next(&LoggingInfo.files, file)) {
        if (!(BL_LOG_IS_STD_STREAM & file->flags))
            fclose(file->file);
    }
    bl_container_dynamic_destroy(&LoggingInfo.files);
}

void bl_log(const char* format, ...) {
    va_list args;
    va_start(args, format);
    bl_vlog(format, args);
    va_end(args);
}

void bl_vlog(const char* format, va_list args) {
    if (!LoggingInfo.isInited)
        return;

    int availableSpace = BL_LOGGING_BUFFERSIZE - (int)logBuffer.bytesWriten;
    if (availableSpace == 0) {
        bl_log_flush();
        availableSpace = BL_LOGGING_BUFFERSIZE;
    }
    va_list argsCopy;
    va_copy(argsCopy, args);
    int bytesNeeded = vsnprintf((char*) logBuffer.buffer + logBuffer.bytesWriten, (size_t)availableSpace, format, argsCopy) + 1;
    if (bytesNeeded < 0)
        return;
    if (availableSpace >= bytesNeeded) {
        logBuffer.bytesWriten += (size_t)bytesNeeded;
        return;
    }

    bl_log_flush();

    if (bytesNeeded <= BL_LOGGING_BUFFERSIZE) {
        logBuffer.bytesWriten = (size_t)bytesNeeded;
        vsnprintf((char*) logBuffer.buffer, BL_LOGGING_BUFFERSIZE, format, args);
        return;
    }

    char* buffer = malloc((size_t)bytesNeeded);
    if (!buffer)
        return;

    vsnprintf(buffer, (size_t)bytesNeeded, format, args);

    bl_internal_write_log(buffer, (size_t)bytesNeeded);

    free(buffer);
}

void bl_assert(bool condition, const char* format, ...) {
    if (!condition) {
        va_list args;
        va_start(args, format);
        bl_vlog(format, args);
        va_end(args);
    }
}

void bl_internal_write_log(const char* buffer, size_t amountToWrite) {
    if (amountToWrite == 0)
        return;

    if (bl_mutex_lock(&LoggingInfo.logMutex) != BL_ConcurrencySuccess)
        return;

    const char* beginOfString = buffer + 1;
    uint8_t     flags         = (uint8_t)(1 << ((unsigned char)*buffer - 0xf8));

    for (size_t i = 1; i < amountToWrite; i++) {
        if ((unsigned char) buffer[i] < 0xf8) {
            if (i != amountToWrite - 1)
                continue;
            i++;
        }

        size_t length = i - (uintptr_t) beginOfString + (uintptr_t) buffer;

        for (struct LogFile* file = bl_container_dynamic_front(&LoggingInfo.files); file; file = bl_container_dynamic_next(&LoggingInfo.files, file)) {
            if (file->flags & flags) {
                fwrite(beginOfString, length, 1, file->file);
                fputc('\n', file->file);
            }
        }

        if (i < amountToWrite) {
            flags         = (uint8_t)(0x1 << ((unsigned char) buffer[i] - 0xf8));
            beginOfString = buffer + i + 1;
        }
    }
    bl_mutex_unlock(&LoggingInfo.logMutex);
}

void bl_log_flush(void) {
    if (!LoggingInfo.isInited)
        return;
    bl_internal_write_log((char*) logBuffer.buffer, logBuffer.bytesWriten);
    logBuffer.bytesWriten = 0;
}



BL_ContainerError bl_log_init(void) {
    if (LoggingInfo.isInited)
        return BL_ContainerOPSuccessful;
    bl_beginTime         = time(NULL);

    LoggingInfo.logMutex = bl_mutex_create(BL_MutexPlain);

    if (!bl_mutex_is_valid(&LoggingInfo.logMutex))
        return BL_ContainerAllocFailure;

    LoggingInfo.files = bl_container_dynamic_create_stack(0, sizeof(struct LogFile));
    atexit(internal_cleanup);
    LoggingInfo.isInited = true;
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_log_register(FILE* file, uint8_t flags) {
    if (!LoggingInfo.isInited)
        return BL_ContainerOPUnsuccessful;
    if (bl_mutex_lock(&LoggingInfo.logMutex) != BL_ConcurrencySuccess)
        return BL_ContainerAllocFailure;
    BL_ContainerError statusCode = bl_container_dynamic_append(&LoggingInfo.files, sizeof(struct LogFile), &(struct LogFile) {.flags = flags, .file = file});
    bl_mutex_unlock(&LoggingInfo.logMutex);
    return statusCode;
}
