#pragma once
#include <ECS/Defines.h>
#include <Util/Container/VectorSet.h>

namespace Wonder
{
    class ObjectSelection
    {
    public:
        void AddToSelection(ECS::EntityReference aSelection);
        void RemoveFromSelection(ECS::EntityReference aSelection);
        void ClearSelection();
        
        bool IsEmpty() const;
        bool Contains(ECS::EntityReference aSelection) const;
        
        const Goose::VectorSet<ECS::EntityReference>& GetSelection() const;
        const Goose::VectorSet<ECS::EntityReference>& GetTopObjects() const;
    private:
        
        void AddedChildRecursion(ECS::EntityID aEntityId,ECS::EntityID aHeadId);
        void RemovedChildRecursion(ECS::EntityID aEntityId,ECS::EntityID aHeadId);
        
        bool IsAnyParentSelected(ECS::EntityID aEntityId,ECS::EntityID aHeadId);
        
        Goose::VectorSet<ECS::EntityReference> mySelection;
        Goose::VectorSet<ECS::EntityReference> mySelectedTopObjects;
    };
}
