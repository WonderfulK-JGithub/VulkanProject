#pragma once
#include "Delegates/Delegate.h"

template<typename E>
using Action = Delegate<void(E)>;

template<typename E>
struct EventTypeTraits
{
    constexpr static bool IsEvent = false;
};

#define DECLARE_EVENT_TYPE(EventType)                  \
    template<>                                         \
    struct EventTypeTraits<EventType>                  \
    {                                                  \
        constexpr static bool IsEvent = true;          \
        constexpr static size_t EventID = __COUNTER__; \
    }                                                  