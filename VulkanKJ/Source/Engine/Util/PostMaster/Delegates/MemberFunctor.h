#pragma once
#include <assert.h>
#include <utility>

#include "Functor.h"

template<typename>
class MemberFunctor;

template<class I, typename T, typename... Args>
class MemberFunctor<T(I::*)(Args...)> final : public Functor<T(Args...)>
{
private:
    using MemberInst = I*;
    using MemberFunc = T(I::*)(Args...);

public:
    MemberFunctor(MemberInst aInstance, MemberFunc aFunc);
    ~MemberFunctor() override = default;
    
public:
    virtual bool operator==(const Functor<T(Args...)>& aOther) override;
    virtual T operator()(Args&&... aArgs) override;
    virtual T Invoke(Args&&... aArgs) override;

private:
    MemberInst myInstance;
    MemberFunc myFunc;
};

template <class I, typename T, typename ... Args>
MemberFunctor<T(I::*)(Args...)>::MemberFunctor(const MemberInst aInstance, const MemberFunc aFunc)
    : myInstance(aInstance), myFunc(aFunc)
{
}

template <class I, typename T, typename ... Args>
bool MemberFunctor<T(I::*)(Args...)>::operator==(const Functor<T(Args...)>& aOther)
{
    const MemberFunctor* otherFunc = dynamic_cast<const MemberFunctor*>(&aOther);
    
    if (otherFunc == nullptr)
    {
        return false;
    }

    return myFunc == otherFunc->myFunc && myInstance == otherFunc->myInstance;
}

template <class I, typename T, typename ... Args>
T MemberFunctor<T(I::*)(Args...)>::operator()(Args&&... aArgs)
{
    assert(myInstance != nullptr && "MemberFunctor::operator() called with nullptr instance pointer");
    assert(myFunc != nullptr && "MemberFunctor::operator() called with nullptr function pointer");
    
    return (myInstance->*myFunc)(std::forward<Args>(aArgs)...);   
}

template <class I, typename T, typename ... Args>
T MemberFunctor<T(I::*)(Args...)>::Invoke(Args&&... aArgs)
{
    assert(myInstance != nullptr && "MemberFunctor::operator() called with nullptr instance pointer");
    assert(myFunc != nullptr && "MemberFunctor::operator() called with nullptr function pointer");
    
    return (myInstance->*myFunc)(std::forward<Args>(aArgs)...);   
}
