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
#if __STDC_VERSION__ < 202311l
#define thread_local _Thread_local
#endif
#else
#include <threads.h>
#endif

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#define _Noreturn [[noreturn]]
#else

#define noexcept

#endif
/**
 * @brief Error type for BackerLibConcurrency.
 **/
typedef enum ConcurrencyError {
    ConcurrencySuccess,
    ConcurrencyFailure,
    ConcurrencyBusy,
    ConcurrencyInterrupt
} ConcurrencyError;

/**
 * @brief Available types of Mutexes. Contains MutexPlain, MutexRecursive and MutexTimed.
 */
typedef enum MutexType {
    MutexPlain,
    MutexRecursive,
    MutexTimed
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
extern bool threadIsValid(const BL_Thread* thread) noexcept;

/**
 * @brief Creates a new thread. threadIsValid will only return true for thread if function was successful.
 * @param sharedState Valid pointer to predefined shared state
 * @param function Function to be executed at start of thread
 * @return Thread object.
 */
extern BL_Thread threadCreate(void* sharedState, int (*function)(void* sharedState)) noexcept;

/**
 *
 * @return Returns current thread.
 * @note Always valid and cannot fail.
 */
extern BL_Thread threadGetCurrent(void) noexcept;

/**
 * @brief Compares equality between first and second threads
 * @param first Pointer to valid thread
 * @param second Pointer to valid thread
 * @return true if first and second are identifying the same thread, else false.
 */
extern bool threadIsEqual(const BL_Thread* first, const BL_Thread* second) noexcept;

/**
 * @brief Function to make current thread sleep for duration.
 * @param duration Duration to sleep
 * @param remaining Remaining time to sleep after interrupt
 * @return ConcurrencySuccess if sleep was successful.
 * @return ConcurrencyInterrupt if sleep was interrupted by signal. Remaining will be set if possible.
 * @return ConcurrencyFailure if sleep could not be executed.
 * @note remaining can be NULL.
 */
extern ConcurrencyError threadSleep(const struct timespec* duration, struct timespec* remaining) noexcept;

/**
 * @brief Yield execution priority to other threads.
 */
extern void threadYield(void) noexcept;

/**
 * @brief Exits thread.
 */
_Noreturn extern void threadExit(void) noexcept;

/**
 * @brief Detaches thread. Thread can never be reattached after detaching.
 * @param thread Pointer to valid Thread
 * @return ConcurrencyFailure if thread could not be detached.
 */
extern ConcurrencyError threadDetach(BL_Thread* thread) noexcept;

/**
 * @brief Awaits the exit of thread and destroys it.
 * @param thread Pointer to valid Thread
 * @return ConcurrencyFailure if thread could not be joined.
 */
extern ConcurrencyError threadJoin(BL_Thread* thread) noexcept;

/**
 * @param mutex Pointer to Mutex
 * @return true if mutex is valid, else false.
 */
extern bool mutexIsValid(const BL_Mutex* mutex) noexcept;

/**
 * @brief Creates a new Mutex. Is invalid if operation fails otherwise valid.
 * @param mutexType Type of mutex to use
 * @return Mutex object.
 */
extern BL_Mutex mutexCreate(BL_MutexType mutexType) noexcept;

/**
 * @brief Locks a valid mutex. Awaits if mutex is already locked.
 * @param mutex Pointer to valid mutex
 * @return ConcurrencyFailure if mutex could not be locked.
 */
extern ConcurrencyError mutexLock(BL_Mutex* mutex) noexcept;

/**
 * @brief Locks a valid mutex. Does not await a locked mutex.
 * @param mutex Pointer to valid mutex.
 * @return ConcurrenyBusy if mutex was locked.
 * @return ConcurrencyFailure if mutex could not be locked.
 */
extern ConcurrencyError mutexTryLock(BL_Mutex* mutex) noexcept;

/**
 * @brief Locks valid mutex. Awaits locked mutex until timepoint in UTC time.
 * @param mutex Pointer to valid mutex
 * @param timepoint Pointer to valid timespec
 * @return ConcurrencyBusy if timePoint has passed.
 * @return ConcurrencyFailure if mutex could not be locked.
 */
extern ConcurrencyError mutexTimeLock(BL_Mutex* mutex, const struct timespec* timepoint) noexcept;

/**
 * @brief Locks valid mutex. Awaits locked mutex for timeToWait.
 * @param mutex Pointer to valid mutex
 * @param timeToWait Pointer to valid timespec
 * @return ConcurrencyBusy if waited time is more than or equal to timeToWait.
 * @return ConcurrencyFailure if mutex could not be locked.
 */
extern ConcurrencyError mutexTimeLockRelative(BL_Mutex* mutex, const struct timespec* timeToWait) noexcept;

/**
 * @brief Unlocks mutex.
 * @param mutex Pointer to valid mutex
 * @return ConcurrencyFailure if mutex could not be unlocked.
 */
extern ConcurrencyError mutexUnlock(BL_Mutex* mutex) noexcept;

/**
 * @brief Destroys mutex. It is no longer valid, but if it is heap allocated, still allocated.
 * @param mutex Pointer to mutex
 */
extern void mutexDestroy(void* mutex) noexcept;


#ifdef __cplusplus
#undef _Noreturn
}
};
#endif

#endif // CONCURRENCYDEFINES_H
