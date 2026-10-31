#pragma once
#include "ECS/Defines.h"

#include <nlohmann/json.hpp>

#include "imgui.h"
#include "Property/PropertyImGuiFunctions.h"
#include "Property/PropertySaveAndLoadFunctions.h"
#include "Util/ImGuiUtil.h"

namespace Wonder::Reflection
{
	enum class ReflectionMode
	{
		Load,
		Save,
		Display,
		OverrideMerge,
		RemoveOverride
	};

	enum class EditType : uint8_t
	{
		Prefab,
		PrefabChild,
		Override,
	};

	inline ReflectionMode globalReflectionMode;

	inline char* globalDataStartAddress;

	inline const char* globalPrefabData;
	inline const char* globalOverrideData;

	inline uint64_t globalPropertyIndex{};

	inline uint8_t globalInfoFlag{};
	inline EditType globalEditType{};
	inline ECS::EntityID globalCurrentReflectedId = ECS::INVALID_ENTITY;

	inline uint64_t globalAllowedOverridesFlag{};
	inline uint64_t globalOverridesEditFlag{};

	inline nlohmann::json* globalJsonSaveData = nullptr;

	inline uint8_t PreEditStuff(size_t aIndex)
	{
		uint8_t info = 0u;

		ImGuiUtil::PropertyTable::SwitchToLabel();

		if (globalEditType == EditType::Prefab && ImGui::GetIO().KeyCtrl)
		{
			ImGui::PushID(static_cast<int>(aIndex));

			const uint64_t allowFlag = 1ui64 << aIndex;
			bool isAllowed = (globalAllowedOverridesFlag & allowFlag) != 0;

			if (ImGui::Checkbox("##AllowOverride", &isAllowed))
			{
				info = InfoFlag_EditEnded | InfoFlag_ChangedValue;

				if (isAllowed)
				{
					globalAllowedOverridesFlag |= allowFlag;
				}
				else
				{
					globalAllowedOverridesFlag &= static_cast<uint64_t>(-1) ^ (allowFlag);
				}
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Allow Override");
			}

			ImGui::PopID();

			ImGui::SameLine();
		}

		if (globalOverridesEditFlag & (static_cast<uint64_t>(1) << aIndex))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.8f, 1.f, 1.f));
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.f));
		}

		return info;
	}

	inline bool CanEdit(const size_t aIndex)
	{
		return (globalEditType != EditType::Override) || (globalAllowedOverridesFlag & (1ui64 << aIndex));
	}
	
	void Header(const char* aText);
	

	template<typename T>
	void ReflectProperty(T& aValue, const char* aName)
	{
		const uint32_t addressDiff = reinterpret_cast<char*>(&aValue) - globalDataStartAddress;

		switch (globalReflectionMode)
		{
		case ReflectionMode::Load:
			LoadProperty(aValue, *globalJsonSaveData, aName);
			break;
		case ReflectionMode::Save:SaveProperty(aValue, *globalJsonSaveData, aName);
			break;
		case ReflectionMode::Display:
		{
			if (CanEdit(globalPropertyIndex))
			{
				ImGuiUtil::PropertyTable::NextProperty();
				globalInfoFlag |= PreEditStuff(globalPropertyIndex);

				if (ImGui::GetIO().KeyCtrl)
				{
					ImGui::Indent(20.0f);
				}
				
				const uint8_t propertyInfoFlag = PropertyEdit::DisplayImGuiPropertyEdit(aValue, aName);

				if (ImGui::GetIO().KeyCtrl)
				{
					ImGui::Unindent(20.0f);
				}
				
				if (propertyInfoFlag & InfoFlag_EditEnded)
				{
					globalOverridesEditFlag |= (1ui64 << globalPropertyIndex);
				}
				globalInfoFlag |= propertyInfoFlag;

				ImGui::PopStyleColor();
			}
		}
			break;
		case ReflectionMode::OverrideMerge:
		{
			if (((globalAllowedOverridesFlag & (1ui64 << globalPropertyIndex)) && (globalOverridesEditFlag & (1ui64 << globalPropertyIndex))))
			{
				aValue = *reinterpret_cast<const T*>(globalOverrideData + addressDiff);
			}
			else
			{
				aValue = *reinterpret_cast<const T*>(globalPrefabData + addressDiff);
			}
		}
		break;
		case ReflectionMode::RemoveOverride:
			{
				if (((globalAllowedOverridesFlag & (1ui64 << globalPropertyIndex))) == 0ui64)
				{
					aValue = *reinterpret_cast<const T*>(globalOverrideData + addressDiff);
				}
				else
				{
					aValue = *reinterpret_cast<const T*>(globalPrefabData + addressDiff);
				}
			}
			break;
		}

		PropertyEdit::LocalInfo::currentAttribute = PropertyEdit::LocalInfo::defaultAttribute;
		globalPropertyIndex++;
	}
}



