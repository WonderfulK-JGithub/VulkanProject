#include "ComponentData.h"

#include "ComponentType.h"

Wonder::ComponentData::ComponentData(const ComponentData& aOther)
{
	if (aOther.myType == nullptr)
	{
		return;
	}

	if (aOther.myType != myType)
	{
		if (myType != nullptr)
		{
			myType->DestructorInAddress(myData.data());
		}
		myType = aOther.myType;
		myData.resize(aOther.myData.size());
	}
	myType->CopyToAddress(myData.data(), aOther.myData.data());
	myAllowOverridesFlag = aOther.myAllowOverridesFlag;
}

Wonder::ComponentData& Wonder::ComponentData::operator=(const ComponentData& aOther)
{
	if (aOther.myType == nullptr)
	{
		return *this;
	}

	if (aOther.myType != myType)
	{
		if (myType != nullptr)
		{
			myType->DestructorInAddress(myData.data());
		}
		myType = aOther.myType;
		myData.resize(aOther.myData.size());
	}
	myType->CopyToAddress(myData.data(), aOther.myData.data());
	myAllowOverridesFlag = aOther.myAllowOverridesFlag;

	return *this;
}

Wonder::ComponentData::ComponentData(ComponentData&& aOther) noexcept
{
	if (myType != nullptr)
	{
		myType->DestructorInAddress(myData.data());
	}

	myType = aOther.myType;
	myAllowOverridesFlag = aOther.myAllowOverridesFlag;

	aOther.myType = nullptr;
	aOther.myAllowOverridesFlag = 0;

	myData = std::move(aOther.myData);
}

Wonder::ComponentData& Wonder::ComponentData::operator=(ComponentData&& aOther) noexcept
{
	if (myType != nullptr)
	{
		myType->DestructorInAddress(myData.data());
	}

	myType = aOther.myType;
	myAllowOverridesFlag = aOther.myAllowOverridesFlag;

	aOther.myType = nullptr;
	aOther.myAllowOverridesFlag = 0;

	myData = std::move(aOther.myData);

	return *this;
}

Wonder::ComponentData::~ComponentData()
{
	if (myType != nullptr)
	{
		myType->DestructorInAddress(myData.data());
	}
}

Wonder::ComponentData::ComponentData(const ComponentTypeBase* aType)
{
	myType = aType;
	myData.resize(myType->GetByteSize());
	myType->EmplaceNewInAddress(myData.data());
}

Wonder::ComponentData::ComponentData(const ComponentTypeBase* aType, const char* aData, const uint64_t aAllowOverrideFlag)
{
	myType = aType;
	myData.resize(myType->GetByteSize());
	myType->EmplaceNewInAddress(myData.data());
	myType->CopyToAddress(myData.data(), aData);
	myAllowOverridesFlag = aAllowOverrideFlag;
}

char* Wonder::ComponentData::AccessData()
{
	return myData.data();
}

const char* Wonder::ComponentData::AccessData() const
{
	return myData.data();
}

uint64_t& Wonder::ComponentData::AccessOverridesFlag()
{
	return myAllowOverridesFlag;
}

uint64_t Wonder::ComponentData::GetOverridesFlag() const
{
	return myAllowOverridesFlag;
}

const Wonder::ComponentTypeBase* Wonder::ComponentData::GetType() const
{
	return myType;
}

std::vector<Wonder::ComponentData> Wonder::GetOverridenComponents(const std::vector<ComponentData>& aComponents,
	const ComponentOverrides& aOverrides)
{
	std::vector<Wonder::ComponentData> overridenComponents;
	
	for (auto& component : aComponents)
	{
		const ComponentTypeBase* typeToUpdate = component.GetType();
		
		auto it = aOverrides.find(typeToUpdate);
		
		if (it == aOverrides.end())
		{
			overridenComponents.push_back(component);
			continue;
		}
		
		auto& componentOverride = it->second;
		
		Reflection::globalOverridesEditFlag = componentOverride.GetOverridesFlag();
		Reflection::globalAllowedOverridesFlag = component.GetOverridesFlag();
		
		Reflection::globalReflectionMode = Reflection::ReflectionMode::OverrideMerge;
		Reflection::globalPropertyIndex = 0u;
		
		Reflection::globalOverrideData = componentOverride.AccessData();
	
		ComponentData finalComponent = component;
					
		Reflection::globalPrefabData = component.AccessData();
		Reflection::globalDataStartAddress = finalComponent.AccessData();
					
		typeToUpdate->ReflectComponent(finalComponent.AccessData());
	}
	
	return overridenComponents;
}

void Wonder::SaveComponentsToJson(std::vector<ComponentData>& aComponentDatas, nlohmann::json& aJson)
{
	for (ComponentData& componentData : aComponentDatas)
	{
		const ComponentTypeBase* type = componentData.GetType();

		nlohmann::json currentSetJson;
		currentSetJson["!type"]            = type->GetName(); // ! in order to make it come first in the file XD
		currentSetJson["allowedOverrides"] = componentData.GetOverridesFlag();

		Reflection::globalReflectionMode = Reflection::ReflectionMode::Save;
		Reflection::globalJsonSaveData = &currentSetJson;
		type->ReflectComponent(componentData.AccessData());
		aJson.emplace_back(currentSetJson);
	}
}

void Wonder::LoadComponentsFromJson(std::vector<ComponentData>& aComponentDatas,const nlohmann::json& aJson)
{
	static std::string typeName;
	for (auto& componentDataJson : aJson)
	{
		typeName                        = componentDataJson["!type"];
		const ComponentTypeBase* type = GetComponentType(Tga::StringRegistry::RegisterOrGetString(typeName));

		if (type == nullptr)
		{
			continue;
		}

		aComponentDatas.emplace_back(type);
		Reflection::globalReflectionMode = Reflection::ReflectionMode::Load;
		Reflection::globalJsonLoadData = &componentDataJson;
		type->ReflectComponent(aComponentDatas.back().AccessData());
		aComponentDatas.back().AccessOverridesFlag() = componentDataJson["allowedOverrides"];
	}
}

void Wonder::SaveOverridesToJson(const ComponentOverrides& aOverrides, nlohmann::json& aJson)
{
	std::vector<ComponentData> components;

	for (const auto& component : aOverrides | std::views::values)
	{
		components.push_back(component);
	}
	
	SaveComponentsToJson(components, aJson);
}

void Wonder::LoadOverridesFromJson(ComponentOverrides& aOverrides,const nlohmann::json& aJson)
{
	std::vector<ComponentData> components;
	
	LoadComponentsFromJson(components, aJson);

	for (auto& component : components)
	{
		aOverrides.insert({component.GetType(), component});
	}
}
