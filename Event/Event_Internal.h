#ifndef BL_EVENT_INTERNAL_H
#define BL_EVENT_INTERNAL_H

#include <BackerLibTypes.h>

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#else
#define noexcept
#endif

    typedef struct BL_Event {
        BL_DynamicContainer functions;
    } BL_Event;

#define BL_EVENT_MAX_JOBS_PER_THREAD 20

    BL_ContainerError bl_event_global_init(void) noexcept;

    BL_ContainerError bl_event_global_register(const BL_StringView* eventName, const BL_Event* event) noexcept;

    BL_ContainerError bl_event_global_register_func(const BL_StringView* eventName, void (*func)(void)) noexcept;

    BL_ContainerError bl_event_global_unregister(const BL_StringView* eventName) noexcept;

    BL_ContainerError bl_event_global_unregister_func(const BL_StringView* eventName, void (*func)(void)) noexcept;

    BL_Event          bl_event_create(void) noexcept;

    void              bl_event_destroy(void* event) noexcept;

    BL_ContainerError bl_event_register(BL_Event* event, void (*func)(void)) noexcept;

    void              bl_event_unregister(BL_Event* event, void (*func)(void)) noexcept;

    void              bl_event_invoke(const BL_Event* event) noexcept;

    BL_ContainerError bl_event_global_invoke(const BL_StringView* eventName) noexcept;

#ifdef __cplusplus
    }
};
#else
#undef noexcept
#endif
#endif
