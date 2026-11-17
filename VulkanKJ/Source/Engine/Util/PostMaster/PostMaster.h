#pragma once
#include <memory>
#include <vector>

#include "Events.h"
#include "IEventQueue.h"

class PostMaster
{
public:
    /**
     * @brief Subscribe to an event type
     */
    template <typename E, typename std::enable_if<EventTypeTraits<E>::IsEvent, bool>::type = true>
    static void Subscribe(const Action<E>& aObserver);

    /**
     * @brief Unsubscribe from an event type
     */
    template <typename E, typename std::enable_if<EventTypeTraits<E>::IsEvent, bool>::type = true>
    static void UnSubscribe(const Action<E>& aObserver);

    /**
     * @brief Publish an event
     */
    template <typename E, typename std::enable_if<EventTypeTraits<E>::IsEvent, bool>::type = true, typename... Args>
    static void Publish(Args&&... aArgs);

    /**
     * @brief Poll all events
     */
    static void PollEvents();

private:
    static PostMaster& GetInstance();

    template <typename E>
    static uint32_t GetEventIndex();

private:
    template <typename E>
    EventQueue<E>* GetEventQueue();

private:
    inline static uint32_t ourNextIndex = 0;

private:
    std::vector<std::unique_ptr<IEventQueue>> myEventQueues;
};

template <typename E, std::enable_if_t<EventTypeTraits<E>::IsEvent, bool>>
void PostMaster::Subscribe(const Action<E>& aObserver)
{
    auto eventQueue = GetInstance().GetEventQueue<E>();
    eventQueue->AddObserver(aObserver);
}

template <typename E, std::enable_if_t<EventTypeTraits<E>::IsEvent, bool>>
void PostMaster::UnSubscribe(const Action<E>& aObserver)
{
    auto eventQueue = GetInstance().GetEventQueue<E>();
    eventQueue->RemoveObserver(aObserver);
}

template <typename E, std::enable_if_t<EventTypeTraits<E>::IsEvent, bool>, typename... Args>
void PostMaster::Publish(Args&&... aArgs)
{
    EventQueue<E>* eventQueue = GetInstance().GetEventQueue<E>();
    
    E event(std::forward<Args>(aArgs)...);
    eventQueue->EnqueueEvent(event);
}

template <typename E>
uint32_t PostMaster::GetEventIndex()
{
    static uint32_t index = ourNextIndex++;
    return index;
}

template <typename E>
EventQueue<E>* PostMaster::GetEventQueue()
{
    const uint32_t index = GetEventIndex<E>();
    if (index >= myEventQueues.size())
    {
        myEventQueues.resize(index + 1);
        myEventQueues[index] = std::make_unique<EventQueue<E>>();
    }

    return static_cast<EventQueue<E>*>(myEventQueues[index].get());
}
