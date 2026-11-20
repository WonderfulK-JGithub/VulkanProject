#pragma once
#include "Util/Container/PrimitiveVector.hpp"

#include <cstdint>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace Wonder
{
	class ComponentTypeBase;
}

namespace Wonder
{
	class ComponentData
	{
	public:
		ComponentData() = default;

		ComponentData(const ComponentData& aOther);
		ComponentData& operator=(const ComponentData& aOther);
		ComponentData(ComponentData&& aOther) noexcept;
		ComponentData& operator=(ComponentData&& aOther) noexcept;

		~ComponentData();

		ComponentData(const ComponentTypeBase* aType);
		ComponentData(const ComponentTypeBase* aType, const char* aData, uint64_t aAllowOverrideFlag);

		char* AccessData();
		const char* AccessData() const;

		uint64_t& AccessOverridesFlag();
		uint64_t GetOverridesFlag() const;

		const ComponentTypeBase* GetType() const;
	private:
		PrimitiveVector<char> myData;
		const ComponentTypeBase* myType = nullptr;
		uint64_t myAllowOverridesFlag = 0;
	};
	
	using ComponentOverrides = std::unordered_map<const ComponentTypeBase*,ComponentData>;
	
	std::vector<ComponentData> GetOverridenComponents(const std::vector<ComponentData>& aComponents, const ComponentOverrides& aOverrides);
	
	void SaveComponentsToJson(std::vector<ComponentData>& aComponentDatas, nlohmann::json& aJson);
	void LoadComponentsFromJson(std::vector<ComponentData>& aComponentDatas,const nlohmann::json& aJson);
	
	void SaveOverridesToJson(const ComponentOverrides& aOverrides, nlohmann::json& aJson);
	void LoadOverridesFromJson(ComponentOverrides& aOverrides,const nlohmann::json& aJson);
}
