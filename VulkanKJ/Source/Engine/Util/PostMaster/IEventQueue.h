#pragma once
#include <queue>
#include <vector>
#include "Events.h"

class IEventQueue
{
public:
    virtual ~IEventQueue() = default;

public:
    virtual void PollEvents() = 0;
};

template <typename T>
class EventQueue final : public IEventQueue
{
public:
    virtual void PollEvents() override;

    void EnqueueEvent(const T& aEvent);

    void AddObserver(const Action<T>& aObserver);

    void RemoveObserver(const Action<T>& aObserver);

private:
    std::queue<T> myEvents;
    std::vector<Action<T>> myObservers;
};

template <typename T>
void EventQueue<T>::PollEvents() 
{
    while (!myEvents.empty())
    {
        T event = myEvents.front();
        myEvents.pop();

        for (Action<T>& observer : myObservers)
        {
            if (observer != nullptr)
            {
                observer.Invoke(event);
            }
        }
    }
}

template <typename T>
void EventQueue<T>::EnqueueEvent(const T& aEvent)
{
    myEvents.push(aEvent);
}

template <typename T>
void EventQueue<T>::AddObserver(const Action<T>& aObserver)
{
    myObservers.push_back(aObserver);
}

template <typename T>
void EventQueue<T>::RemoveObserver(const Action<T>& aObserver)
{
    myObservers.erase(std::remove(myObservers.begin(), myObservers.end(), aObserver), myObservers.end());
}
