#pragma once
#include <memory>

#include "ConstMemberFunctor.h"
#include "FreeFunctor.h"
#include "Functor.h"
#include "MemberFunctor.h"

template<typename>
class Delegate;

template<typename T, typename... Args>
class Delegate<T(Args...)> final
{
private:
    using Func = Functor<T(Args...)>;
    using FunctorPtr = std::shared_ptr<Func>;
    
public:
    Delegate();
    Delegate(T(*aFreeFunc)(Args...));

    template<class C>
    Delegate(T(C::*aMemberFunc)(Args...), C* aInstance);

    template<class C>
    Delegate(T(C::*aMemberFunc)(Args...) const, C* aInstance);

public:
    void Reset();

    void Reset(T(*aFreeFunc)(Args...));

    template<class C>
    void Reset(T(C::*aMemberFunc)(Args...), C* aInst);

    template<class C>
    void Reset(T(C::*aMemberFunc)(Args...) const, C* aInst);

public:
    T Invoke(Args... aArgs);
    T operator()(Args&&... aArgs);

    bool operator==(const Delegate& aOther) const;
    bool operator!=(const Delegate& aOther) const;
    bool operator==(nullptr_t) const;
    bool operator!=(nullptr_t) const;

private:
    FunctorPtr myFunctor;
};

template <typename T, typename ... Args>
Delegate<T(Args...)>::Delegate()
{
    Reset();
}

template <typename T, typename ... Args>
Delegate<T(Args...)>::Delegate(T(* aFreeFunc)(Args...))
{
    Reset(aFreeFunc); 
}

template <typename T, typename ... Args>
template <class C>
Delegate<T(Args...)>::Delegate(T(C::* aMemberFunc)(Args...), C* aInstance)
{
    Reset(aMemberFunc, aInstance); 
}

template <typename T, typename ... Args>
template <class C>
Delegate<T(Args...)>::Delegate(T(C::* aMemberFunc)(Args...) const, C* aInstance)
{
    Reset(aMemberFunc, aInstance);
}

template <typename T, typename... Args>
void Delegate<T(Args...)>::Reset()
{
    myFunctor.reset();   
}

template <typename T, typename ... Args>
void Delegate<T(Args...)>::Reset(T(* aFreeFunc)(Args...))
{
    myFunctor.reset(new FreeFunctor<T(Args...)>(aFreeFunc));  
}

template <typename T, typename ... Args>
template <class C>
void Delegate<T(Args...)>::Reset(T(C::* aMemberFunc)(Args...), C* aInst)
{
    myFunctor.reset(new MemberFunctor<T(C::*)(Args...)>(aInst, aMemberFunc));
}

template <typename T, typename ... Args>
template <class C>
void Delegate<T(Args...)>::Reset(T(C::* aMemberFunc)(Args...) const, C* aInst)
{
    myFunctor.reset(new ConstMemberFunctor<T(C::*)(Args...) const>(aInst, aMemberFunc));
}

template <typename T, typename ... Args>
T Delegate<T(Args...)>::Invoke(Args... aArgs)
{
    assert(myFunctor != nullptr && "Delegate::Invoke called with nullptr functor");

    return myFunctor->Invoke(std::forward<Args>(aArgs)...);  
}

template <typename T, typename ... Args>
T Delegate<T(Args...)>::operator()(Args&&... aArgs)
{
    assert(myFunctor != nullptr && "Delegate::Invoke called with nullptr functor");

    return myFunctor->Invoke(std::forward<Args>(aArgs)...);  
}

template <typename T, typename ... Args>
bool Delegate<T(Args...)>::operator==(const Delegate& aOther) const
{
    Func* thisFunc = myFunctor.get();
    Func* otherFunc = aOther.myFunctor.get();
    
    const bool bothEmpty = thisFunc == nullptr && otherFunc == nullptr;
    const bool bothNonEmpty = thisFunc != nullptr && otherFunc != nullptr;
    const bool equal = bothNonEmpty && thisFunc->operator==(*otherFunc);

    return bothEmpty || equal;  
}

template <typename T, typename ... Args>
bool Delegate<T(Args...)>::operator!=(const Delegate& aOther) const
{
    return !(*this == aOther);
}

template <typename T, typename ... Args>
bool Delegate<T(Args...)>::operator==(nullptr_t) const
{
    return myFunctor.get() == nullptr; 
}

template <typename T, typename ... Args>
bool Delegate<T(Args...)>::operator!=(nullptr_t) const
{
    return !(*this == nullptr);
}
