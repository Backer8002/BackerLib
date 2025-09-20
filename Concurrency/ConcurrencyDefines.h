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
} MutexType;

/**
 * @brief Mutex type for BackerLibConcurrency. Type is defined based on available headers.
 */
typedef struct Mutex Mutex;
/**
 * @brief Thread type for BackerLibConcurrency. Type is defined based on available headers.
 */
typedef struct Thread Thread;

#if USES_PTHREAD
struct Mutex {
    pthread_mutex_t mutex;
    bool            isValid;
};

struct Thread {
    pthread_t thread;
    bool      isValid;
};
#else
struct Mutex {
    mtx_t mutex;
    bool  isValid;
};

struct Thread {
    thrd_t thread;
    bool   isValid;
};
#endif

/**
 * @brief Checks if thread is a valid object.
 * @param thread Pointer to thread
 * @return true if thread is active, else false.
 */
static inline bool threadIsValid(const Thread* thread) { return thread->isValid; }

#if USES_PTHREAD

static pthread_mutex_t threadStartMutex             = PTHREAD_MUTEX_INITIALIZER;
static int (*          threadFunctionToExec)(void*) = NULL;

static inline void* internal_pthreadThreadInitFunc(void* sharedState) {
    int (*realFunction)(void*) = threadFunctionToExec;
    pthread_mutex_unlock(&threadStartMutex);
    realFunction(sharedState);
    return NULL;
}

#endif
/**
 * @brief Creates a new thread. threadIsValid will only return true for thread if function was successful.
 * @param sharedState Valid pointer to predefined shared state
 * @param function Function to be executed at start of thread
 * @return Thread object.
 */
static inline Thread threadCreate(void* sharedState, int (*function)(void* sharedState)) {
    Thread threadToUse = {.isValid = false};
#if USES_PTHREAD

    pthread_t newThread;
    if (pthread_mutex_lock(&threadStartMutex) != 0)
        return threadToUse;
    threadFunctionToExec = function;
    int threadStatus     = pthread_create(&newThread, NULL, internal_pthreadThreadInitFunc, sharedState);
    if (threadStatus != 0)
        return threadToUse;
#else

    thrd_t newThread;
    int    threadStatus = thrd_create(&newThread, function, sharedState);
    if (threadStatus != thrd_success)
        return threadToUse;

#endif
    threadToUse.thread  = newThread;
    threadToUse.isValid = true;
    return threadToUse;
}

/**
 *
 * @return Returns current thread.
 * @note Always valid and cannot fail.
 */
static inline Thread threadGetCurrent(void) {
#if USES_PTHREAD
    return (Thread){.thread = pthread_self(), .isValid = true};
#else
    return (Thread){.thread = thrd_current(), .isValid = true};
#endif
}

/**
 * @brief Compares equality between first and second threads
 * @param first Pointer to valid thread
 * @param second Pointer to valid thread
 * @return true if first and second are identifying the same thread, else false.
 */
static inline bool threadIsEqual(const Thread* first, const Thread* second) {
#if USES_PTHREAD

    return pthread_equal(first->thread, second->thread);

#else

    return thrd_equal(first->thread, second->thread) != 0;

#endif
}

/**
 * @brief Function to make current thread sleep for duration.
 * @param duration Duration to sleep
 * @param remaining Remaining time to sleep after interrupt
 * @return ConcurrencySuccess if sleep was successful.
 * @return ConcurrencyInterrupt if sleep was interrupted by signal. Remaining will be set if possible.
 * @return ConcurrencyFailure if sleep could not be executed.
 * @note remaining can be NULL.
 */
static inline ConcurrencyError threadSleep(const struct timespec* duration, struct timespec* remaining) {
#if USES_PTHREAD
    if (nanosleep(duration, remaining) == 0)
        return ConcurrencySuccess;
    if (errno == EINTR)
        return ConcurrencyInterrupt;
    return ConcurrencyFailure;
#else
    int errorCode = thrd_sleep(duration, remaining);
    if (errorCode == 0)
        return ConcurrencySuccess;
    if (errorCode == -1)
        return ConcurrencyInterrupt;
    return ConcurrencyFailure;
#endif
}

/**
 * @brief Yield execution priority to other threads.
 */
static inline void threadYield(void) {
#if USES_PTHREAD
    sched_yield();
#else
    thrd_yield();
#endif
}

/**
 * @brief Exits thread.
 */
_Noreturn static inline void threadExit(void) {
#if USES_PTHREAD
    pthread_exit(NULL);
#else
    thrd_exit(0);
#endif
}

/**
 * @brief Detaches thread. Thread can never be reattached after detaching.
 * @param thread Pointer to valid Thread
 * @return ConcurrencyFailure if thread could not be detached.
 */
static inline ConcurrencyError threadDetach(Thread* thread) {
#if USES_PTHREAD

    if (pthread_detach(thread->thread))
        return ConcurrencyFailure;
    return ConcurrencySuccess;

#else

    if (thrd_detach(thread->thread) != thrd_success)
        return ConcurrencyFailure;
    return ConcurrencySuccess;

#endif
}

/**
 * @brief Awaits the exit of thread and destroys it.
 * @param thread Pointer to valid Thread
 * @return ConcurrencyFailure if thread could not be joined.
 */
static inline ConcurrencyError threadJoin(Thread* thread) {
#if USES_PTHREAD

    if (pthread_join(thread->thread, NULL) != 0)
        return ConcurrencyFailure;

#else

    if (thrd_join(thread->thread, NULL) != thrd_success)
        return ConcurrencyFailure;

#endif

    thread->isValid = false;
    return ConcurrencySuccess;
}

/**
 * @param mutex Pointer to Mutex
 * @return true if mutex is valid, else false.
 */
static inline bool mutexIsValid(const Mutex* mutex) { return mutex->isValid; }

/**
 * @brief Creates a new Mutex. Is invalid if operation fails otherwise valid.
 * @param mutexType Type of mutex to use
 * @return Mutex object.
 */
static inline Mutex mutexCreate(MutexType mutexType) {
    Mutex returnValue = {.isValid = false};
#if USES_PTHREAD
    pthread_mutex_t mutex;
    if (mutexType != MutexPlain) {
        pthread_mutexattr_t mutexAttribute;
        if (pthread_mutexattr_init(&mutexAttribute))
            return returnValue;
        pthread_mutexattr_settype(&mutexAttribute,
                                  mutexType == MutexRecursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_TIMED_NP);
        int errorCode = pthread_mutex_init(&mutex, &mutexAttribute);
        pthread_mutexattr_destroy(&mutexAttribute);
        if (errorCode)
            return returnValue;
    } else { mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER; }
#else
    mtx_t mutex;
    if (mtx_init(&mutex, mutexType == MutexPlain ? mtx_plain : mutexType == MutexRecursive ? mtx_recursive : mtx_timed)
        != thrd_success)
        return returnValue;
#endif
    returnValue = (Mutex){.mutex = mutex, .isValid = true};
    return returnValue;
}

/**
 * @brief Locks a valid mutex. Awaits if mutex is already locked.
 * @param mutex Pointer to valid mutex
 * @return ConcurrencyFailure if mutex could not be locked.
 */
static inline ConcurrencyError mutexLock(Mutex* mutex) {
#if USES_PTHREAD
    if (pthread_mutex_lock(&mutex->mutex))
        return ConcurrencyFailure;
    return ConcurrencySuccess;
#else
    if (mtx_lock(&mutex->mutex) != thrd_success)
        return ConcurrencyFailure;
    return ConcurrencySuccess;
#endif
}

/**
 * @brief Locks a valid mutex. Does not await a locked mutex.
 * @param mutex Pointer to valid mutex.
 * @return ConcurrenyBusy if mutex was locked.
 * @return ConcurrencyFailure if mutex could not be locked.
 */
static inline ConcurrencyError mutexTryLock(Mutex* mutex) {
#if USES_PTHREAD
    int errorCode = pthread_mutex_trylock(&mutex->mutex);
    if (errorCode == EBUSY)
        return ConcurrencyBusy;
    if (errorCode)
        return ConcurrencyFailure;
    return ConcurrencySuccess;
#else
    int errorCode = mtx_trylock(&mutex->mutex);
    if (errorCode == thrd_success)
        return ConcurrencySuccess;
    if (errorCode == thrd_busy)
        return ConcurrencyBusy;
    return ConcurrencyFailure;
#endif
}

/**
 * @brief Locks valid mutex. Awaits locked mutex until timepoint in UTC time.
 * @param mutex Pointer to valid mutex
 * @param timepoint Pointer to valid timespec
 * @return ConcurrencyBusy if timePoint has passed.
 * @return ConcurrencyFailure if mutex could not be locked.
 */
static inline ConcurrencyError mutexTimeLock(Mutex* mutex, const struct timespec* timepoint) {
#if USES_PTHREAD
    int errorCode = pthread_mutex_timedlock(&mutex->mutex, timepoint);
    if (errorCode == ETIMEDOUT)
        return ConcurrencyBusy;
    if (errorCode)
        return ConcurrencyFailure;
    return ConcurrencySuccess;
#else
    int errorCode = mtx_timedlock(&mutex->mutex, timepoint);
    if (errorCode == thrd_success)
        return ConcurrencySuccess;
    if (errorCode == thrd_timedout)
        return ConcurrencyBusy;
    return ConcurrencyFailure;
#endif
}

/**
 * @brief Locks valid mutex. Awaits locked mutex for timeToWait.
 * @param mutex Pointer to valid mutex
 * @param timeToWait Pointer to valid timespec
 * @return ConcurrencyBusy if waited time is more than or equal to timeToWait.
 * @return ConcurrencyFailure if mutex could not be locked.
 */
static inline ConcurrencyError mutexTimeLockRelative(Mutex* mutex, const struct timespec* timeToWait) {
    struct timespec currentTime;
#if USES_PTHREAD
    clock_gettime(CLOCK_REALTIME, &currentTime);
#else
    timespec_get(&currentTime, TIME_UTC);
#endif
    return mutexTimeLock(mutex,
                         &(struct timespec){
                             .tv_sec = currentTime.tv_sec + timeToWait->tv_sec + (currentTime.tv_nsec + timeToWait->tv_nsec) / (long) 1e9,
                             .tv_nsec = (currentTime.tv_nsec + timeToWait->tv_nsec) % (long) 1e9});
}

/**
 * @brief Unlocks mutex.
 * @param mutex Pointer to valid mutex
 * @return ConcurrencyFailure if mutex could not be unlocked.
 */
static inline ConcurrencyError mutexUnlock(Mutex* mutex) {
#if USES_PTHREAD
    if (pthread_mutex_unlock(&mutex->mutex))
        return ConcurrencyFailure;
    return ConcurrencySuccess;
#else
    if (mtx_unlock(&mutex->mutex) != thrd_success)
        return ConcurrencyFailure;
    return ConcurrencySuccess;
#endif
}

/**
 * @brief Destroys mutex. It is no longer valid, but if it is heap allocated, still allocated.
 * @param mutex Pointer to mutex
 */
static inline void mutexDestroy(void* mutex) {
    if (((Mutex*) mutex)->isValid) {
#if USES_PTHREAD
        pthread_mutex_destroy(mutex);
#else
        mtx_destroy(mutex);
#endif
        ((Mutex*) mutex)->isValid = false;
    }
}


#ifdef __cplusplus
}
};
#endif

#endif // CONCURRENCYDEFINES_H