#include "Component.h"

#include <unordered_map>

#include "ArchetypeManager.h"
#include "Entity.h"

namespace 
{
	ECS::ComponentID ourNextFreeComponentID{0};
	std::unordered_map<ECS::ComponentID, ECS::ComponentInfo> ourComponentIndex{};
}

namespace ECS
{
	namespace INTERNAL
	{
		ComponentID RegisterComponent(const char* const aName, const size_t aSize, const std::function<void(void*)>& aDestructor, const std::function<void(void*, void*)>& aMoveConstructor)
		{
			ComponentID id = ourNextFreeComponentID++;
			ourComponentIndex.emplace(id, ComponentInfo{
				{},
				aName,
				aSize,
				aDestructor,
				aMoveConstructor});
			Entity::CreateFutureArray(id, aSize);
			return id;
		};
	}
	
	ComponentInfo* GetComponentInfo(ComponentID aComponentID)
	{
		return &ourComponentIndex.at(aComponentID);
	}

	const std::unordered_map<ECS::ComponentID, ECS::ComponentInfo>& GetComponentIndex()
	{
		return ourComponentIndex;
	}
}
