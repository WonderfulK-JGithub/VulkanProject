#pragma once

#include <array>
#include <functional>

#include "ArchetypeManager.h"
#include "Component.h"
#include "ComponentIterator.hpp"
#include "Entity.h"

namespace ECS
{
    template<typename ...T>
    struct RegistryView
    {
        size_t entityCount;
        Entity* entities;
        std::tuple<T*...> componentArrays;
    };

    template<typename ...Ignores>
    struct ViewIgnoreFilter
    {
	    
    };
    
template<typename ...T>
class View
{
    template<typename ...U>
    friend class ComponentIterator;

public:
    View();

    template<typename ...Ignores>
    View(ViewIgnoreFilter<Ignores...> aFilter);

    using iterator = ComponentIterator<T...>;
    iterator begin();
    iterator end();

    static std::vector<RegistryView<T...>> GetRegistryViews();

    void ForEach(const std::function<void(T&...)>& aOperation);
    void ForEach(const std::function<void(Entity aEntity, T&...)>& aOperation);
    void ForEachRegistry(const std::function<void(size_t, T*...)>& aOperation);
    void ForEachRegistry(const std::function<void(size_t, Entity* aEntities, T*...)>& aOperation);

    void ControlledForEach(const std::function<bool(T&...)>& aOperation);
    void ControlledForEach(const std::function<bool(Entity aEntity, T&...)>& aOperation);
    void ControlledForEachRegistry(const std::function<bool(size_t, T*...)>& aOperation);
    void ControlledForEachRegistry(const std::function<bool(size_t, Entity* aEntities, T*...)>& aOperation);
    
    size_t GetEntityCount() const;

private:
    std::vector<Archetype*> myArchetypes;
    std::vector<std::array<ComponentArray, sizeof...(T)>> myComponentArrays;
};

template <typename ...T>
View<T...>::View()
{
    std::vector<ComponentID> componentIDs{ ECS::GetComponentID<T>()... };

    for (Archetype* archetype : globalCurrentECS->archetypeManager.GetArchetypes())
    {
        if (globalCurrentECS->archetypeManager.ArchetypeHasComponents(archetype, componentIDs) && archetype->entityCount != 0)
        {
            myArchetypes.emplace_back(archetype);
            std::array<ComponentArray, sizeof...(T)> componentArrays{ archetype->componentArrays.at(ECS::GetComponentID<T>())... };
            myComponentArrays.push_back(componentArrays);
        }
    }
}

template<typename ... T>
template<typename ... Ignores>
View<T...>::View([[maybe_unused]] ViewIgnoreFilter<Ignores...> aFilter)
{
    std::vector<ComponentID> componentIDs{ ECS::GetComponentID<T>()... };
    std::vector<ComponentID> filteredIDs{ ECS::GetComponentID<Ignores>()... };

    for (Archetype* archetype : globalCurrentECS->archetypeManager.GetArchetypes())
    {
        const bool passesFilter = filteredIDs.empty() || globalCurrentECS->archetypeManager.ArchetypeHasComponents(archetype, filteredIDs) == false;

        if (globalCurrentECS->archetypeManager.ArchetypeHasComponents(archetype, componentIDs) && 
            passesFilter && archetype->entityCount != 0)
        {
            myArchetypes.emplace_back(archetype);
            std::array<ComponentArray, sizeof...(T)> componentArrays{ archetype->componentArrays.at(ECS::GetComponentID<T>())... };
            myComponentArrays.push_back(componentArrays);
        }
    }
}

template <typename ... T>
View<T...>::iterator View<T...>::begin()
{
    return iterator{ this, 0 };
}

template <typename ... T>
View<T...>::iterator View<T...>::end()
{
    return iterator{ this, myArchetypes.size() };
}

template <typename ... T>
std::vector<RegistryView<T...>> View<T...>::GetRegistryViews()
{
    std::vector<RegistryView<T...>> registryViews;

    std::vector<ComponentID> componentIDs{ ECS::GetComponentID<T>()... };

    for (Archetype* archetype : globalCurrentECS->archetypeManager.GetArchetypes())
    {
        if (globalCurrentECS->archetypeManager.ArchetypeHasComponents(archetype, componentIDs))
        {
            registryViews.emplace_back(
                archetype->entityCount,
                reinterpret_cast<Entity*>(archetype->indexToEntityID.data()),
                std::tuple<T*...>{
                reinterpret_cast<T*>(archetype->componentArrays.at(ECS::GetComponentID<T>()).data)...
            });
        }
    }

    return registryViews;
}

template <typename ... T>
void View<T...>::ForEach(const std::function<void(T&...)>& aOperation)
{
    for (Archetype* archetype : myArchetypes)
    {
        std::tuple<T*...> datas{ reinterpret_cast<T*>(archetype->componentArrays.at(ECS::GetComponentID<T>()).data)... };

        for (size_t i = 0; i < archetype->entityCount; ++i)
        {
            if (GetEntityRecord(archetype->indexToEntityID[i]).isActive == false) continue;
            aOperation(std::get<T*>(datas)[i]...);
        }
    }
}

template <typename ... T>
void View<T...>::ForEach(const std::function<void(Entity aEntity, T&...)>& aOperation)
{
    for (Archetype* archetype : myArchetypes)
    {
        std::tuple<T*...> datas{ reinterpret_cast<T*>(archetype->componentArrays.at(ECS::GetComponentID<T>()).data)... };

        for (size_t i = 0; i < archetype->entityCount; ++i)
        {
            if (GetEntityRecord(archetype->indexToEntityID[i]).isActive == false) continue;
            aOperation(*reinterpret_cast<Entity*>(&archetype->indexToEntityID[i]), std::get<T*>(datas)[i]...);
        }
    }
}

template <typename ... T>
void View<T...>::ForEachRegistry(const std::function<void(size_t, T*...)>& aOperation)
{
    for (auto archetype : myArchetypes)
    {
        std::tuple<T*...> datas{ reinterpret_cast<T*>(archetype->componentArrays.at(ECS::GetComponentID<T>()).data)... };

        aOperation(archetype->entityCount, std::get<T*>(datas)...);
    }
}

template <typename ... T>
void View<T...>::ForEachRegistry(const std::function<void(size_t, Entity* aEntities, T*...)>& aOperation)
{
    for (auto archetype : myArchetypes)
    {
        std::tuple<T*...> datas{ reinterpret_cast<T*>(archetype->componentArrays.at(ECS::GetComponentID<T>()).data)... };

        aOperation(archetype->entityCount, reinterpret_cast<Entity*>(archetype->indexToEntityID.data()), std::get<T*>(datas)...);
    }
}

template <typename ... T>
void View<T...>::ControlledForEach(const std::function<bool(T&...)>& aOperation)
{
    for (Archetype* archetype : myArchetypes)
    {
        std::tuple<T*...> datas{ reinterpret_cast<T*>(archetype->componentArrays.at(ECS::GetComponentID<T>()).data)... };

        for (size_t i = 0; i < archetype->entityCount; ++i)
        {
            if (GetEntityRecord(archetype->indexToEntityID[i]).isActive == false) continue;
            if (aOperation(std::get<T*>(datas)[i]...) == false)
            {
                return;
            }
        }
    }
}

template <typename ... T>
void View<T...>::ControlledForEach(const std::function<bool(Entity aEntity, T&...)>& aOperation)
{
    for (Archetype* archetype : myArchetypes)
    {
        std::tuple<T*...> datas{ reinterpret_cast<T*>(archetype->componentArrays.at(ECS::GetComponentID<T>()).data)... };

        for (size_t i = 0; i < archetype->entityCount; ++i)
        {
            if (aOperation(*reinterpret_cast<Entity*>(&archetype->indexToEntityID[i]), std::get<T*>(datas)[i]...) == false)
            {
                return;
            }
        }
    }
}

template <typename ... T>
void View<T...>::ControlledForEachRegistry(const std::function<bool(size_t, T*...)>& aOperation)
{
    for (auto archetype : myArchetypes)
    {
        std::tuple<T*...> datas{ reinterpret_cast<T*>(archetype->componentArrays.at(ECS::GetComponentID<T>()).data)... };

        if (aOperation(archetype->entityCount, std::get<T*>(datas)...) == false)
        {
            return;
        }
    }
}

template <typename ... T>
void View<T...>::ControlledForEachRegistry(const std::function<bool(size_t, Entity* aEntities, T*...)>& aOperation)
{
    for (auto archetype : myArchetypes)
    {
        std::tuple<T*...> datas{ reinterpret_cast<T*>(archetype->componentArrays.at(ECS::GetComponentID<T>()).data)... };

        if (aOperation(archetype->entityCount, reinterpret_cast<Entity*>(archetype->indexToEntityID.data()), std::get<T*>(datas)...) == false)
        {
            return;
        }
    }
}

template <typename ... T>
size_t View<T...>::GetEntityCount() const
{
    size_t count = 0;
    
    for (const auto& archetype : myArchetypes)
    {
        count += archetype->entityCount;
    }
    
    return count;
}
}
