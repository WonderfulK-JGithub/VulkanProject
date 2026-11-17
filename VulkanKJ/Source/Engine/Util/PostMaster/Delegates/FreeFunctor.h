#pragma once
#include <assert.h>
#include <utility>

#include "Functor.h"

template<typename>
class FreeFunctor;

template<typename T, typename... Args>
class FreeFunctor<T(Args...)> final : public Functor<T(Args...)>
{
private:
    using FreeFunc = T(*)(Args...);

public:
    FreeFunctor(const FreeFunc& aFunc);
    ~FreeFunctor() override = default;

public:
    virtual bool operator==(const Functor<T(Args...)>& aOther) override;
    virtual T operator()(Args&&... aArgs) override;
    virtual T Invoke(Args&&... aArgs) override;

private:
    FreeFunc myFunc;
};

template <typename T, typename ... Args>
FreeFunctor<T(Args...)>::FreeFunctor(const FreeFunc& aFunc)
    : myFunc(aFunc)
{
}

template <typename T, typename ... Args>
bool FreeFunctor<T(Args...)>::operator==(const Functor<T(Args...)>& aOther)
{
    const FreeFunctor* otherFunc = dynamic_cast<const FreeFunctor*>(&aOther);

    if (otherFunc == nullptr)
    {
        return false;
    }

    return myFunc == otherFunc->myFunc;
}

template <typename T, typename ... Args>
T FreeFunctor<T(Args...)>::operator()(Args&&... aArgs)
{
    assert(myFunc != nullptr && "FreeFunctor::operator() called with nullptr function pointer");
    return myFunc(std::forward<Args>(aArgs)...);
}

template <typename T, typename ... Args>
T FreeFunctor<T(Args...)>::Invoke(Args&&... aArgs)
{
    assert(myFunc != nullptr && "FreeFunctor::operator() called with nullptr function pointer");
    return myFunc(std::forward<Args>(aArgs)...);
}
