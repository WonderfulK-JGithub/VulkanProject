#pragma once

#include <unordered_map>
#include <vcruntime_typeinfo.h>

#include "Defines.h"


namespace Goose
{
	struct SpriteShaderInstanceData;
}

namespace ECS
{
	class Entity;

	namespace INTERNAL
	{
		ComponentID RegisterComponent(const char* aName, size_t aSize, const std::function<void(void*)>& aDestructor, const std::function<void(void*, void*)>& aMoveConstructor);
	}
	
	ComponentInfo* GetComponentInfo(ComponentID aComponentID);
	const std::unordered_map<ECS::ComponentID, ECS::ComponentInfo>& GetComponentIndex();

	template <typename T>
	ComponentID GetComponentID();
	
	template <class T>
	const char* GetComponentName();
}

namespace ECS
{
	template <typename T>
	ComponentID GetComponentID()
	{
		//static_assert(std::is_trivially_copyable<T>());
		
		static const ComponentID newComponentID = INTERNAL::RegisterComponent(
			GetComponentName<T>(), 
			sizeof(T), 
			[](void* aInst) { static_cast<T*>(aInst)->~T(); },
			[](void* aDest, void* aSource) { new (aDest) T(std::move(*static_cast<T*>(aSource))); });
		
		return newComponentID;
	}
	
	template <typename T>
	const char* GetComponentName()
	{
		static constexpr const char* COMPONENT_TYPE_PREFIX = "struct ";
		static constexpr const char* START_PREFIX = ":";
		
		const char* typeName = typeid(T).name();
		size_t startPrefixIndex = std::string{typeName}.find_last_of(START_PREFIX);
		if (startPrefixIndex != std::string::npos)
		{
			return typeName + startPrefixIndex + 1;
		}
		if (std::strcmp(typeName, COMPONENT_TYPE_PREFIX) != 0)
		{
			return typeName + std::strlen(COMPONENT_TYPE_PREFIX);
		}

		return typeName;
	}


}