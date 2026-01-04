#include "ConcurrencyDefines.h"
#include <stdbool.h>

#if USES_PTHREAD
#include<pthread.h>
#else
#include<threads.h>
#endif

bool bl_thread_is_valid(const BL_Thread* thread) { return thread->isValid; }

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

BL_Thread bl_thread_create(void* sharedState, int (*function)(void* sharedState)) {
    BL_Thread threadToUse = {.isValid = false};
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

BL_Thread bl_thread_get_current(void) {
#if USES_PTHREAD
    return (BL_Thread){.thread = pthread_self(), .isValid = true};
#else
    return (BL_Thread){.thread = thrd_current(), .isValid = true};
#endif
}

bool bl_thread_is_equal(const BL_Thread* first, const BL_Thread* second) {
#if USES_PTHREAD

    return pthread_equal(first->thread, second->thread);

#else

    return thrd_equal(first->thread, second->thread) != 0;

#endif
}

BL_ConcurrencyError bl_thread_sleep(const struct timespec* duration, struct timespec* remaining) {
#if USES_PTHREAD
    if (nanosleep(duration, remaining) == 0)
        return BL_ConcurrencySuccess;
    if (errno == EINTR)
        return BL_ConcurrencyInterrupt;
    return BL_ConcurrencyFailure;
#else
    int errorCode = thrd_sleep(duration, remaining);
    if (errorCode == 0)
        return BL_ConcurrencySuccess;
    if (errorCode == -1)
        return BL_ConcurrencyInterrupt;
    return BL_ConcurrencyFailure;
#endif
}

void bl_thread_yield(void) {
#if USES_PTHREAD
    sched_yield();
#else
    thrd_yield();
#endif
}

void bl_thread_exit(void) {
#if USES_PTHREAD
    pthread_exit(NULL);
#else
    thrd_exit(0);
#endif
while(true) {}
}
BL_ConcurrencyError bl_thread_detach(BL_Thread* thread) {
#if USES_PTHREAD

    if (pthread_detach(thread->thread))
        return BL_ConcurrencyFailure;
    return BL_ConcurrencySuccess;

#else

    if (thrd_detach(thread->thread) != thrd_success)
        return BL_ConcurrencyFailure;
    return BL_ConcurrencySuccess;

#endif
}
BL_ConcurrencyError bl_thread_join(BL_Thread* thread) {
#if USES_PTHREAD

    if (pthread_join(thread->thread, NULL) != 0)
        return BL_ConcurrencyFailure;

#else

    if (thrd_join(thread->thread, NULL) != thrd_success)
        return BL_ConcurrencyFailure;

#endif

    thread->isValid = false;
    return BL_ConcurrencySuccess;
}
bool bl_mutex_is_valid(const BL_Mutex* mutex) {
    return mutex->isValid;
}

BL_Mutex bl_mutex_create(BL_MutexType mutexType) {
    BL_Mutex returnValue = {.isValid = false};
#if USES_PTHREAD
    pthread_mutex_t mutex;
    if (mutexType != BL_MutexPlain) {
        pthread_mutexattr_t mutexAttribute;
        if (pthread_mutexattr_init(&mutexAttribute))
            return returnValue;
        pthread_mutexattr_settype(&mutexAttribute,
                                  mutexType == BL_MutexRecursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_TIMED_NP);
        int errorCode = pthread_mutex_init(&mutex, &mutexAttribute);
        pthread_mutexattr_destroy(&mutexAttribute);
        if (errorCode)
            return returnValue;
    } else { mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER; }
#else
    mtx_t mutex;
    if (mtx_init(&mutex, mutexType == BL_MutexPlain ? mtx_plain : mutexType == BL_MutexRecursive ? mtx_recursive : mtx_timed)
        != thrd_success)
        return returnValue;
#endif
    returnValue = (BL_Mutex){.mutex = mutex, .isValid = true};
    return returnValue;
}

BL_ConcurrencyError bl_mutex_lock(BL_Mutex* mutex) {
#if USES_PTHREAD
    if (pthread_mutex_lock(&mutex->mutex))
        return BL_ConcurrencyFailure;
    return BL_ConcurrencySuccess;
#else
    if (mtx_lock(&mutex->mutex) != thrd_success)
        return BL_ConcurrencyFailure;
    return BL_ConcurrencySuccess;
#endif
}

BL_ConcurrencyError bl_mutex_lock_try(BL_Mutex* mutex) {
#if USES_PTHREAD
    int errorCode = pthread_mutex_trylock(&mutex->mutex);
    if (errorCode == EBUSY)
        return BL_ConcurrencyBusy;
    if (errorCode)
        return BL_ConcurrencyFailure;
    return BL_ConcurrencySuccess;
#else
    int errorCode = mtx_trylock(&mutex->mutex);
    if (errorCode == thrd_success)
        return BL_ConcurrencySuccess;
    if (errorCode == thrd_busy)
        return BL_ConcurrencyBusy;
    return BL_ConcurrencyFailure;
#endif
}

BL_ConcurrencyError bl_mutex_lock_time(BL_Mutex* mutex, const struct timespec* timepoint) {
#if USES_PTHREAD
    int errorCode = pthread_mutex_timedlock(&mutex->mutex, timepoint);
    if (errorCode == ETIMEDOUT)
        return BL_ConcurrencyBusy;
    if (errorCode)
        return BL_ConcurrencyFailure;
    return BL_ConcurrencySuccess;
#else
    int errorCode = mtx_timedlock(&mutex->mutex, timepoint);
    if (errorCode == thrd_success)
        return BL_ConcurrencySuccess;
    if (errorCode == thrd_timedout)
        return BL_ConcurrencyBusy;
    return BL_ConcurrencyFailure;
#endif
}

BL_ConcurrencyError mutex_lock_time_relative(BL_Mutex* mutex, const struct timespec* timeToWait) {
    struct timespec currentTime;
#if USES_PTHREAD
    clock_gettime(CLOCK_REALTIME, &currentTime);
#else
    timespec_get(&currentTime, TIME_UTC);
#endif
    return bl_mutex_lock_time(mutex,
                         &(struct timespec){
                             .tv_sec = currentTime.tv_sec + timeToWait->tv_sec + (currentTime.tv_nsec + timeToWait->tv_nsec) / (long) 1e9,
                             .tv_nsec = (currentTime.tv_nsec + timeToWait->tv_nsec) % (long) 1e9});
}

BL_ConcurrencyError bl_mutex_unlock(BL_Mutex* mutex) {
#if USES_PTHREAD
    if (pthread_mutex_unlock(&mutex->mutex))
        return BL_ConcurrencyFailure;
    return BL_ConcurrencySuccess;
#else
    if (mtx_unlock(&mutex->mutex) != thrd_success)
        return BL_ConcurrencyFailure;
    return BL_ConcurrencySuccess;
#endif
}

void bl_mutex_destroy(void* mutex) {
    if (((BL_Mutex*) mutex)->isValid) {
#if USES_PTHREAD
        pthread_mutex_destroy(mutex);
#else
        mtx_destroy(mutex);
#endif
        ((BL_Mutex*) mutex)->isValid = false;
    }
}
