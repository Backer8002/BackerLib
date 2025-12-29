#include "Event_Internal.h"
#include <BackerLibConcurrency.h>
#include <BackerLibTypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static struct {
    bool                isInited;
    bool                shouldExit;
    BL_Hashmap          eventMap;
    BL_DynamicContainer jobs;
    BL_Mutex            mutex,eventMutex;
    BL_DynamicContainer threads;
} EventInfo = {0};

static int internal_thread_function(void* sharedState) {
    while (!EventInfo.shouldExit) {
        if (mutexLock(&EventInfo.mutex) != ConcurrencySuccess) {
            threadYield();
            continue;
        }

        void (**functionPtr)(void) = bl_container_dynamic_back(&EventInfo.jobs);
        void (*function)(void)     = functionPtr ? *functionPtr : NULL;

        mutexUnlock(&EventInfo.mutex);

        if (function)
            function();
        else
            threadYield();
    }
    return 0;
}

static void internal_destroy_global_invokable(void* func) {
    ((void (*)(void)) func)();
}

static void internal_cleanup(void) {
    EventInfo.shouldExit = true;
    for (BL_Thread* thread = bl_container_dynamic_front(&EventInfo.threads); thread; thread = bl_container_dynamic_next(&EventInfo.threads, thread))
        threadJoin(thread);
    bl_container_dynamic_destroy(&EventInfo.threads);
    bl_container_dynamic_destroy_with_elements(&EventInfo.jobs,internal_destroy_global_invokable);
    bl_hashmap_destroy(&EventInfo.eventMap,NULL,bl_event_destroy);
    mutexDestroy(&EventInfo.mutex);
}

BL_ContainerError bl_event_global_init(void) {
    if (EventInfo.isInited)
        return BL_ContainerOPSuccessful;

    EventInfo.mutex = mutexCreate(MutexPlain);

    if (!mutexIsValid(&EventInfo.mutex))
        return BL_ContainerAllocFailure;
    
    EventInfo.eventMutex = mutexCreate(MutexPlain);

    if (!mutexIsValid(&EventInfo.eventMutex)) {
        mutexDestroy(&EventInfo.mutex);
        return BL_ContainerAllocFailure;
    }

    EventInfo.eventMap = bl_hashmap_create_stack(0, sizeof(BL_StringView), sizeof(BL_Event), bl_string_equal, bl_hashfunction_string_view);

    if (!bl_hashmap_is_valid(&EventInfo.eventMap)) {
        mutexDestroy(&EventInfo.mutex);
        mutexDestroy(&EventInfo.eventMutex);
        return BL_ContainerAllocFailure;
    }

    EventInfo.jobs       = bl_container_dynamic_create_stack(0, sizeof(void (*)(void)));
    EventInfo.threads    = bl_container_dynamic_create_stack(0, sizeof(BL_Thread));
    EventInfo.isInited   = true;
    EventInfo.shouldExit = false;
    atexit(internal_cleanup);
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_event_global_register(const BL_StringView* eventName, const BL_Event* event) {
    if (!EventInfo.isInited)
        return BL_ContainerOPUnsuccessful;

    if (mutexLock(&EventInfo.eventMutex) != ConcurrencySuccess)
        return BL_ContainerOPUnsuccessful;

    BL_ContainerError errorCode = bl_hashmap_insert(&EventInfo.eventMap, sizeof *eventName, eventName, sizeof *event, event);

    mutexUnlock(&EventInfo.eventMutex);

    return errorCode;
}

BL_ContainerError bl_event_global_register_func(const BL_StringView* eventName, void (*func)(void)) {
    if (!EventInfo.isInited)
        return BL_ContainerOPUnsuccessful;

    if (mutexLock(&EventInfo.eventMutex) != ConcurrencySuccess)
        return BL_ContainerOPUnsuccessful;

    BL_ContainerError errorCode = BL_ContainerOPSuccessful;

    BL_Event* event = bl_hashmap_get(&EventInfo.eventMap, sizeof *event, eventName);

    if (event) {
        errorCode = bl_container_dynamic_append(&event->functions, sizeof func, &func);
        goto Exit;
    }

    BL_Event newEvent = {.functions = bl_container_dynamic_create_stack(1, sizeof func)};
    if (!bl_container_dynamic_is_valid(&newEvent.functions)) {
        errorCode =  BL_ContainerAllocFailure;
        goto Exit;
    }

    bl_container_dynamic_append(&newEvent.functions, sizeof func, &func);
    if (bl_hashmap_insert(&EventInfo.eventMap, sizeof *eventName, eventName, sizeof newEvent, &newEvent) != BL_ContainerOPSuccessful) {
        bl_container_dynamic_destroy(&newEvent.functions);
        errorCode = BL_ContainerAllocFailure;
        goto Exit;
    }

    Exit:;

    mutexUnlock(&EventInfo.eventMutex);

    return errorCode;
}

BL_ContainerError bl_event_global_unregister(const BL_StringView* eventName) {
    if (!EventInfo.isInited)
        return BL_ContainerOPSuccessful;
    if (mutexLock(&EventInfo.eventMutex) != ConcurrencySuccess)
        return BL_ContainerOPUnsuccessful;
    bl_hashmap_remove(&EventInfo.eventMap, sizeof *eventName, eventName, NULL, bl_event_destroy);
    mutexUnlock(&EventInfo.eventMutex);
    return BL_ContainerOPSuccessful;
}

BL_ContainerError bl_event_global_unregister_func(const BL_StringView* eventName, void (*func)(void)) {
    if (!EventInfo.isInited)
        return BL_ContainerOPSuccessful;

    if (mutexLock(&EventInfo.eventMutex) != ConcurrencySuccess)
        return BL_ContainerOPUnsuccessful;

    BL_Event* event = bl_hashmap_get(&EventInfo.eventMap, sizeof *eventName, eventName);

    if (!event)
        goto Exit;

    bl_event_unregister(event, func);

    if (bl_container_dynamic_is_empty(&event->functions))
        bl_hashmap_remove(&EventInfo.eventMap, sizeof *eventName, eventName, NULL, bl_event_destroy);

    Exit:;
    mutexUnlock(&EventInfo.eventMutex);
    return BL_ContainerOPSuccessful;
}

BL_Event bl_event_create(void) {
    return (BL_Event) {.functions = bl_container_dynamic_create_stack(0, sizeof(void (*)(void)))};
}

void bl_event_destroy(void* event) {
    bl_container_dynamic_destroy(&((BL_Event*) event)->functions);
}

BL_ContainerError bl_event_register(BL_Event* event, void (*func)(void)) {
    return bl_container_dynamic_append(&event->functions, sizeof func, &func);
}

void bl_event_unregister(BL_Event* event, void (*func)(void)) {
    size_t indexToRemove = SIZE_MAX;
    for (void (**function)(void) = bl_container_dynamic_front(&event->functions); function; function = bl_container_dynamic_next(&event->functions, function)) {
        if (*function == func) {
            indexToRemove = bl_container_dynamic_index_from_reference(&event->functions, function);
            break;
        }
    }
    if (indexToRemove != SIZE_MAX)
        bl_container_dynamic_remove(&event->functions, indexToRemove, indexToRemove);
}

void bl_event_invoke(const BL_Event* event) {
    for (void (**func)(void) = bl_container_dynamic_front(&event->functions); func; func = bl_container_dynamic_next(&event->functions, func))
        (*func)();
}

BL_ContainerError bl_event_global_invoke(const BL_StringView* eventName) {
    if (!EventInfo.isInited)
        return BL_ContainerOPUnsuccessful;

    if (mutexLock(&EventInfo.eventMutex) != ConcurrencySuccess)
        return BL_ContainerOPUnsuccessful;

    BL_Event* event = bl_hashmap_get(&EventInfo.eventMap, sizeof *eventName, eventName);

    if (!event) {
        mutexUnlock(&EventInfo.eventMutex);
        return BL_ContainerInvalidIndex;
    }

    if (mutexLock(&EventInfo.mutex) != ConcurrencySuccess) {
        mutexUnlock(&EventInfo.eventMutex);
        return BL_ContainerOPUnsuccessful;
    }

    if (bl_container_dynamic_reserve(&EventInfo.jobs, bl_container_dynamic_size(&event->functions)) != BL_ContainerOPSuccessful) {
        mutexUnlock(&EventInfo.eventMutex);
        mutexUnlock(&EventInfo.mutex);
        return BL_ContainerAllocFailure;
    }

    bl_container_dynamic_insert_container(&EventInfo.jobs, bl_container_dynamic_size(&EventInfo.jobs), bl_container_const_ptr_cast_dynamic_container(&event->functions));

    mutexUnlock(&EventInfo.eventMutex);

    if (bl_container_dynamic_is_empty(&EventInfo.threads) || bl_container_dynamic_size(&EventInfo.jobs) / bl_container_dynamic_size(&EventInfo.threads) > BL_EVENT_MAX_JOBS_PER_THREAD) {
        size_t amountOfThreadsToAdd = (bl_container_dynamic_size(&EventInfo.jobs) + BL_EVENT_MAX_JOBS_PER_THREAD - 1) / BL_EVENT_MAX_JOBS_PER_THREAD - bl_container_dynamic_size(&EventInfo.threads);

        if (bl_container_dynamic_reserve(&EventInfo.threads, amountOfThreadsToAdd) != BL_ContainerOPSuccessful)
            goto Exit;

        for (size_t i = 0; i < amountOfThreadsToAdd; i++) {
            BL_Thread thread = threadCreate(NULL, internal_thread_function);
            if (!threadIsValid(&thread))
                goto Exit;
            bl_container_dynamic_append(&EventInfo.threads, sizeof thread, &thread);
        }
    }
Exit:;
    mutexUnlock(&EventInfo.mutex);

    return BL_ContainerOPSuccessful;
}
