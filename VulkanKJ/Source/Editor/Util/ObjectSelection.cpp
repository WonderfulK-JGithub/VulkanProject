#include "ObjectSelection.h"

#include <ECS/Entity.h>

void Wonder::ObjectSelection::AddToSelection(ECS::EntityReference aSelection)
{
    if (mySelection.insert(aSelection) == false)
    {
        return; // already exists
    }
    
    ECS::EntityID id = ECS::GetEntityFromReference(aSelection).GetID();
    
    if (IsAnyParentSelected(id,aSelection.headEntityId))
    {
        return;
    }
    
    AddedChildRecursion(id,aSelection.headEntityId);
    mySelectedTopObjects.push_back(aSelection);
}

void Wonder::ObjectSelection::RemoveFromSelection(ECS::EntityReference aSelection)
{
    mySelection.erase(aSelection);
    
    if (mySelectedTopObjects.contains(aSelection))
    {
        mySelectedTopObjects.erase(aSelection);
        RemovedChildRecursion(ECS::GetEntityFromReference(aSelection).GetID(),aSelection.headEntityId);
    }
}

void Wonder::ObjectSelection::ClearSelection()
{
    mySelection.clear();
    mySelectedTopObjects.clear();
}

bool Wonder::ObjectSelection::IsEmpty() const
{
    return mySelection.empty();
}

bool Wonder::ObjectSelection::Contains(ECS::EntityReference aSelection) const
{
    return mySelection.contains(aSelection);
}

const Goose::VectorSet<ECS::EntityReference>& Wonder::ObjectSelection::GetSelection() const
{
    return mySelection;
}

const Goose::VectorSet<ECS::EntityReference>& Wonder::ObjectSelection::GetTopObjects() const
{
    return mySelectedTopObjects;
}

void Wonder::ObjectSelection::AddedChildRecursion(ECS::EntityID aEntityId, ECS::EntityID aHeadId)
{
    ECS::EntityReference reference{.headEntityId = aHeadId,.prefabObjectId = ECS::Entity(aEntityId).GetPrefabObjectId()};
    
    if (mySelectedTopObjects.contains(reference))
    {
        mySelectedTopObjects.erase(reference);
        return;
    }
    
    auto it = ECS::globalCurrentECS->parents.find(aEntityId);
    
    if (it == ECS::globalCurrentECS->parents.end())
    {
        return;
    }

    for (auto& childId : it->second.children)
    {
        AddedChildRecursion(childId,aHeadId);
    }
}

void Wonder::ObjectSelection::RemovedChildRecursion(ECS::EntityID aEntityId, ECS::EntityID aHeadId)
{
    ECS::EntityReference reference{.headEntityId = aHeadId,.prefabObjectId = ECS::Entity(aEntityId).GetPrefabObjectId()};
    
    if (mySelection.contains(reference))
    {
        mySelectedTopObjects.push_back(reference);
        return;
    }
    
    auto it = ECS::globalCurrentECS->parents.find(aEntityId);
    
    if (it == ECS::globalCurrentECS->parents.end())
    {
        return;
    }

    for (auto& childId : it->second.children)
    {
        RemovedChildRecursion(childId,aHeadId);
    }
}

bool Wonder::ObjectSelection::IsAnyParentSelected(ECS::EntityID aEntityId, ECS::EntityID aHeadId)
{
    ECS::EntityID parentId = ECS::Entity(aEntityId).GetParentId();
    
    if (parentId == ECS::INVALID_ENTITY)
    {
        return false;
    }
    
    ECS::EntityReference parentReference{.headEntityId = aHeadId,
        .prefabObjectId = ECS::Entity(parentId).GetPrefabObjectId()};
    if (mySelection.contains(parentReference))
    {
        return true;
    }
    
    return IsAnyParentSelected(parentId,aHeadId);
}
