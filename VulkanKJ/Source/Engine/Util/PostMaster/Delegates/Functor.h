#pragma once

template<typename>
class Functor;

template<typename T, typename... Args>
class Functor<T(Args...)>
{
public:
    virtual ~Functor() = default;

    virtual bool operator==(const Functor& aOther) = 0;

    virtual T operator()(Args&&... aArgs) = 0;

    virtual T Invoke(Args&&... aArgs) = 0;
};
