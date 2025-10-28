#include "ThreadPool.h"

void futureAwait(void* future) { while (!*(bool*) future) {} }

bool futureAwaitUntil(void* future, const struct timespec* timepoint) {
    while (true) {
        if (*(bool*) future)
            return true;
        struct timespec currentTime;
#if USES_PTHREAD
        clock_gettime(CLOCK_REALTIME, &currentTime);
#else
        timespec_get(&currentTime, TIME_UTC);
#endif
        if (currentTime.tv_sec > timepoint->tv_sec ||
            (currentTime.tv_sec == timepoint->tv_sec && currentTime.tv_nsec > timepoint->tv_nsec))
            return false;
    }
}

bool futureAwaitFor(void* future, const struct timespec* duration) {
    struct timespec currentTime;
#if USES_PTHREAD
    clock_gettime(CLOCK_REALTIME, &currentTime);
#else
    timespec_get(&currentTime, TIME_UTC);
#endif
    return futureAwaitUntil(future, &(struct timespec){
                                .tv_sec = currentTime.tv_sec + duration->tv_sec + (currentTime.tv_nsec + duration->tv_nsec) / (long) 1e9,
                                .tv_nsec = (currentTime.tv_nsec + duration->tv_nsec) % (long) 1e9});
}