#pragma once
#include <utility>
#include <assert.h>

#include "Functor.h"

template<typename>
class ConstMemberFunctor;

template<class I, typename T, typename... Args>
class ConstMemberFunctor<T(I::*)(Args...) const> final : public Functor<T(Args...)>
{
private:
    using MemberInst = const I*;
    using MemberFunc = T(I::*)(Args...) const;

public:
    ConstMemberFunctor(MemberInst aInstance, MemberFunc aFunc);
    ~ConstMemberFunctor() override = default;
    
public:
    virtual bool operator==(const Functor<T(Args...)>& aOther) override;
    virtual T operator()(Args&&... aArgs) override;
    virtual T Invoke(Args&&... aArgs) override;

private:
    MemberInst myInstance;
    MemberFunc myFunc;
};

template <class I, typename T, typename ... Args>
ConstMemberFunctor<T(I::*)(Args...) const>::ConstMemberFunctor(const MemberInst aInstance, const MemberFunc aFunc)
    : myInstance(aInstance), myFunc(aFunc)
{
}

template <class I, typename T, typename ... Args>
bool ConstMemberFunctor<T(I::*)(Args...) const>::operator==(const Functor<T(Args...)>& aOther)
{
    const ConstMemberFunctor* otherFunc = dynamic_cast<const ConstMemberFunctor*>(&aOther);
    
    if (otherFunc == nullptr)
    {
        return false;
    }

    return myFunc == otherFunc->myFunc && myInstance == otherFunc->myInstance;
}

template <class I, typename T, typename ... Args>
T ConstMemberFunctor<T(I::*)(Args...) const>::operator()(Args&&... aArgs)
{
    assert(myInstance != nullptr && "MemberFunctor::operator() called with nullptr instance pointer");
    assert(myFunc != nullptr && "MemberFunctor::operator() called with nullptr function pointer");
    
    return (myInstance->*myFunc)(std::forward<Args>(aArgs)...);   
}

template <class I, typename T, typename ... Args>
T ConstMemberFunctor<T(I::*)(Args...) const>::Invoke(Args&&... aArgs)
{
    assert(myInstance != nullptr && "MemberFunctor::operator() called with nullptr instance pointer");
    assert(myFunc != nullptr && "MemberFunctor::operator() called with nullptr function pointer");
    
    return (myInstance->*myFunc)(std::forward<Args>(aArgs)...);   
}

