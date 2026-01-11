#ifndef CONCURRENCYDEFINES_H
#define CONCURRENCYDEFINES_H

#include <errno.h>
#include <stdbool.h>
#include <time.h>
#include <stddef.h>

#if __has_include(<threads.h>)
#define USES_PTHREAD 0
#elif __has_include(<pthread.h>)
#define USES_PTHREAD 1
#else
#error No threading library
#endif

#if USES_PTHREAD
#include <pthread.h>
#if __STDC_VERSION__ < 202311l && !defined(__cplusplus)
#define thread_local _Thread_local
#endif
#else
#include <threads.h>
#endif

#ifdef __cplusplus
extern "C" {
#define _Noreturn [[noreturn]]
#else

#define noexcept

#endif
/**
 * @brief Error type for BackerLibConcurrency.
 **/
typedef enum BL_ConcurrencyError {
    BL_ConcurrencySuccess,
    BL_ConcurrencyFailure,
    BL_ConcurrencyBusy,
    BL_ConcurrencyInterrupt
} BL_ConcurrencyError;

/**
 * @brief Available types of Mutexes. Contains BL_MutexPlain, BL_MutexRecursive and BL_MutexTimed.
 */
typedef enum BL_MutexType {
    BL_MutexPlain,
    BL_MutexRecursive,
    BL_MutexTimed
} BL_MutexType;

/**
 * @brief Mutex type for BackerLibConcurrency. Type is defined based on available headers.
 */
typedef struct BL_Mutex BL_Mutex;
/**
 * @brief Thread type for BackerLibConcurrency. Type is defined based on available headers.
 */
typedef struct BL_Thread BL_Thread;

#if USES_PTHREAD
struct BL_Mutex {
    pthread_mutex_t mutex;
    bool            isValid;
};

struct BL_Thread {
    pthread_t thread;
    bool      isValid;
};
#else
struct BL_Mutex {
    mtx_t mutex;
    bool  isValid;
};

struct BL_Thread {
    thrd_t thread;
    bool   isValid;
};
#endif

/**
 * @brief Checks if thread is a valid object.
 * @param thread Pointer to thread
 * @return true if thread is active, else false.
 */
extern bool bl_thread_is_valid(const BL_Thread* thread) noexcept;

/**
 * @brief Creates a new thread. bl_thread_is_valid will only return true for thread if function was successful.
 * @param sharedState Valid pointer to predefined shared state
 * @param function Function to be executed at start of thread
 * @return Thread object.
 */
extern BL_Thread bl_thread_create(void* sharedState, int (*function)(void* sharedState)) noexcept;

/**
 *
 * @return Returns current thread.
 * @note Always valid and cannot fail.
 */
extern BL_Thread bl_thread_get_current(void) noexcept;

/**
 * @brief Compares equality between first and second threads
 * @param first Pointer to valid thread
 * @param second Pointer to valid thread
 * @return true if first and second are identifying the same thread, else false.
 */
extern bool bl_thread_is_equal(const BL_Thread* first, const BL_Thread* second) noexcept;

/**
 * @brief Function to make current thread sleep for duration.
 * @param duration Duration to sleep
 * @param remaining Remaining time to sleep after interrupt
 * @return ConcurrencySuccess if sleep was successful.
 * @return BL_ConcurrencyInterrupt if sleep was interrupted by signal. Remaining will be set if possible.
 * @return BL_ConcurrencyFailure if sleep could not be executed.
 * @note remaining can be NULL.
 */
extern BL_ConcurrencyError bl_thread_sleep(const struct timespec* duration, struct timespec* remaining) noexcept;

/**
 * @brief Yield execution priority to other threads.
 */
extern void bl_thread_yield(void) noexcept;

/**
 * @brief Exits thread.
 */
_Noreturn extern void bl_thread_exit(void) noexcept;

/**
 * @brief Detaches thread. Thread can never be reattached after detaching.
 * @param thread Pointer to valid Thread
 * @return BL_ConcurrencyFailure if thread could not be detached.
 */
extern BL_ConcurrencyError bl_thread_detach(BL_Thread* thread) noexcept;

/**
 * @brief Awaits the exit of thread and destroys it.
 * @param thread Pointer to valid Thread
 * @return BL_ConcurrencyFailure if thread could not be joined.
 */
extern BL_ConcurrencyError bl_thread_join(BL_Thread* thread) noexcept;

/**
 * @param mutex Pointer to Mutex
 * @return true if mutex is valid, else false.
 */
extern bool bl_mutex_is_valid(const BL_Mutex* mutex) noexcept;

/**
 * @brief Creates a new Mutex. Is invalid if operation fails otherwise valid.
 * @param mutexType Type of mutex to use
 * @return Mutex object.
 */
extern BL_Mutex bl_mutex_create(BL_MutexType mutexType) noexcept;

/**
 * @brief Locks a valid mutex. Awaits if mutex is already locked.
 * @param mutex Pointer to valid mutex
 * @return BL_ConcurrencyFailure if mutex could not be locked.
 */
extern BL_ConcurrencyError bl_mutex_lock(BL_Mutex* mutex) noexcept;

/**
 * @brief Locks a valid mutex. Does not await a locked mutex.
 * @param mutex Pointer to valid mutex.
 * @return ConcurrenyBusy if mutex was locked.
 * @return BL_ConcurrencyFailure if mutex could not be locked.
 */
extern BL_ConcurrencyError bl_mutex_lock_try(BL_Mutex* mutex) noexcept;

/**
 * @brief Locks valid mutex. Awaits locked mutex until timepoint in UTC time.
 * @param mutex Pointer to valid timed mutex
 * @param timepoint Pointer to valid timespec
 * @return BL_ConcurrencyBusy if timePoint has passed.
 * @return BL_ConcurrencyFailure if mutex could not be locked.
 */
extern BL_ConcurrencyError bl_mutex_lock_time(BL_Mutex* mutex, const struct timespec* timepoint) noexcept;

/**
 * @brief Locks valid timed mutex. Awaits locked mutex for timeToWait.
 * @param mutex Pointer to valid mutex
 * @param timeToWait Pointer to valid timespec
 * @return BL_ConcurrencyBusy if waited time is more than or equal to timeToWait.
 * @return BL_ConcurrencyFailure if mutex could not be locked.
 */
extern BL_ConcurrencyError mutex_lock_time_relative(BL_Mutex* mutex, const struct timespec* timeToWait) noexcept;

/**
 * @brief Unlocks mutex.
 * @param mutex Pointer to valid mutex
 * @return BL_ConcurrencyFailure if mutex could not be unlocked.
 */
extern BL_ConcurrencyError bl_mutex_unlock(BL_Mutex* mutex) noexcept;

/**
 * @brief Destroys mutex. It is no longer valid, but if it is heap allocated, still allocated.
 * @param mutex Pointer to mutex
 */
extern void bl_mutex_destroy(void* mutex) noexcept;


#ifdef __cplusplus
#undef _Noreturn
}
#else
#undef noexcept
#endif

#endif // CONCURRENCYDEFINES_H
