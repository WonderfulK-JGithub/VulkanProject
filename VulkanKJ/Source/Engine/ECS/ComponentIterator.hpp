#pragma once
#include <vector>

#include "ArchetypeManager.h"
#include "Component.h"
#include "ComponentPack.hpp"


namespace ECS
{
	template<typename ...T>
    class View;
}

namespace ECS
{
    template<typename ...T>
    class ComponentIterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = decltype(GetComponentPack<T&...>());
        using pointer           = value_type*;  // or also value_type*
        using reference         = value_type&;
        
    public:
        ComponentIterator() = default;
        ComponentIterator(View<T...>* aView, size_t aStart);

        ComponentIterator& operator++();
        ComponentIterator operator++(int);
        bool operator!=(const ComponentIterator& aIterator);
        /*template<size_t ...U>
        value_type Funny(std::index_sequence<U...>);*/

        value_type operator*(); 

    private:
        View<T...>*  myView;
        size_t myCurrentArchetypeIndex;
        size_t myCurrentEntityIndex;
        size_t myCurrentArchetypeSize;
        Archetype* myCurrentArchetype;
        ComponentArray myCurrentComponentArrays[sizeof...(T)];
    };

    template <typename ...T>
    ComponentIterator<T...>::ComponentIterator(View<T...>* const aView, size_t aStart)
        : myView(aView)
        , myCurrentArchetypeIndex(aStart)
        , myCurrentEntityIndex(0)
    {
        if (myCurrentArchetypeIndex < myView->myComponentArrays.size()) [[likely]]
    	{
            myCurrentArchetypeSize = myView->myArchetypes[myCurrentArchetypeIndex]->entityCount;
            myCurrentArchetype = myView->myArchetypes[myCurrentArchetypeIndex];
            std::memcpy(myCurrentComponentArrays, myView->myComponentArrays[myCurrentArchetypeIndex].data(), sizeof(myCurrentComponentArrays));
        }
        else
        {
            myCurrentArchetypeSize = 0;
            myCurrentArchetype = nullptr;
            //myCurrentComponentArrays = nullptr;
        }
    }


    template <typename ...T>
    ComponentIterator<T...>& ComponentIterator<T...>::operator++()
    {
        ++myCurrentEntityIndex;
        if (myCurrentEntityIndex >= myCurrentArchetypeSize) [[unlikely]]
        {
            myCurrentEntityIndex = 0;
            ++myCurrentArchetypeIndex;
            if (myCurrentArchetypeIndex < myView->myArchetypes.size()) [[unlikely]]
            {
                myCurrentArchetypeSize = myView->myArchetypes[myCurrentArchetypeIndex]->entityCount;
                std::memcpy(myCurrentComponentArrays, myView->myComponentArrays[myCurrentArchetypeIndex].data(), sizeof(myCurrentComponentArrays));
                myCurrentArchetype = myView->myArchetypes[myCurrentArchetypeIndex];
            }
        }

        return *this;
    }

    template <typename ... T>
    ComponentIterator<T...> ComponentIterator<T...>::operator++(int)
    {
        ComponentIterator old = *this;
        operator++();
        return old;
    }

    template <typename ... T>
    bool ComponentIterator<T...>::operator!=(const ComponentIterator& aIterator)
    {
        if (myCurrentEntityIndex != aIterator.myCurrentEntityIndex) [[likely]]
        {
            return true;
        }
        if (myCurrentArchetypeIndex != aIterator.myCurrentArchetypeIndex) [[likely]]
        {
            return true;
        }

        return false;
    }

    /*template <typename ... T>
    template <size_t... U>
    typename ComponentIterator<T...>::value_type ComponentIterator<T...>::Funny(std::index_sequence<U...> a)
    {
        __noop(a);
        return {
                *reinterpret_cast<ECS::Entity*>(myCurrentArchetype->indexToEntityID.data() + myCurrentEntityIndex),
                reinterpret_cast<T*>(myCurrentComponentArrays[U].data)[myCurrentEntityIndex]... };
    }*/


    /*template <std::size_t... U>
	void funny(std::index_sequence<U...>)
    {
	    
    }

    template<template<typename ...T> typename test, size_t ...U>
    ComponentIterator<T...>::value_type testa()
    {
	    
    }*/

    template <typename... T>
    ComponentIterator<T...>::value_type ComponentIterator<T...>::operator*()
    {
        /*testa<*this, 2>();
        consteval std::size_t... IndexSequence;
        constexpr IndexSequence indexSequence{};*/

        //return Funny(std::index_sequence_for<T...>{});
		/*return [&]<size_t... Is>(std::index_sequence<Is...>) -> value_type
    	{
            return {
                *reinterpret_cast<ECS::Entity*>(myCurrentArchetype->indexToEntityID.data() + myCurrentEntityIndex),
                reinterpret_cast<T*>(myCurrentComponentArrays[Is].data)[myCurrentEntityIndex]... };
        }(std::index_sequence_for<T...>{});*/

        int index = 0;
        return { 
        	*reinterpret_cast<ECS::Entity*>(myCurrentArchetype->indexToEntityID.data() + myCurrentEntityIndex),
        	reinterpret_cast<T*>(myCurrentComponentArrays[index++].data)[myCurrentEntityIndex]... };
    }

}
