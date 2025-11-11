#include "ThreadPool.h"

void bl_future_await(size_t future,const ThreadPool* threadPool) { while (!*(bool*) bl_unordered_container_get(&threadPool->orders, future)) {} }

bool bl_future_await_until(size_t future, const struct timespec* timepoint, const ThreadPool* threadPool) {
    while (true) {
        if (*(bool*) bl_unordered_container_get(&threadPool->orders, future))
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

bool bl_future_await_for(size_t future, const struct timespec* duration, const ThreadPool* threadPool) {
    struct timespec currentTime;
#if USES_PTHREAD
    clock_gettime(CLOCK_REALTIME, &currentTime);
#else
    timespec_get(&currentTime, TIME_UTC);
#endif
    return bl_future_await_until(future, &(struct timespec){
                                .tv_sec = currentTime.tv_sec + duration->tv_sec + (currentTime.tv_nsec + duration->tv_nsec) / (long) 1e9,
                                .tv_nsec = (currentTime.tv_nsec + duration->tv_nsec) % (long) 1e9},threadPool);
}