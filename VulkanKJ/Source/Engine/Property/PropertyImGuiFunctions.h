#pragma once
#include <string>
#include <vector>
#include <imgui/imgui.h>

#include "CustomPropertyStructs.h"
#include "PropertyAttribute.h"
#include "Util/ImGuiUtil.h"

#include <IconFontHeaders/IconsLucide.h>

#define HAS_FLAG(VAR,FLAG) ((VAR) & (FLAG)) != 0

namespace PropertyEdit
{
	namespace LocalInfo
	{
		inline Goose::PropertyAttribute defaultAttribute;
		inline Goose::PropertyAttribute currentAttribute;
	}

	enum class EditType : uint8_t
	{
		Prefab,
		PrefabChild,
		Override,
	};

	struct PropertyEditInfo
	{
		const char** propertyNames                             = nullptr;
		const char** propertyDescriptions                      = nullptr;
		const Goose::PropertyAttribute* propertyAttributes		= nullptr;
		EditType editType                                      = EditType::Prefab;

		// Says if property set was changed and if edit was ended
		uint8_t infoFlag;

		uint64_t allowOverrideFlag = 0;
		// Flag for every property in a property set that was edited, used for overrides
		uint64_t overrideEditFlag = 0;
	};

	inline PropertyEditInfo globalPropertyEditInfo;

	void SetNextPropertyWidth(float aWidth);
	uint8_t DisplayImGuiPropertyEdit(bool& aData, const char* aLabel);
	uint8_t DisplayImGuiPropertyEdit(int& aData, const char* aLabel);
	uint8_t DisplayImGuiPropertyEdit(uint32_t& aData, const char* aLabel);
	uint8_t DisplayImGuiPropertyEdit(float& aData, const char* aLabel);
	uint8_t DisplayImGuiPropertyEdit(glm::vec2& aData, const char* aLabel);
	uint8_t DisplayImGuiPropertyEdit(glm::vec3& aData, const char* aLabel);
	uint8_t DisplayImGuiPropertyEdit(glm::vec4& aData, const char* aLabel);
	uint8_t DisplayImGuiPropertyEdit(Wonder::ColorProperty& aData, const char* aLabel);
	uint8_t DisplayImGuiPropertyEdit(std::string& aData, const char* aLabel);
	uint8_t DisplayImGuiPropertyEdit(Tga::StringId& aData, const char* aLabel);
	
	uint8_t DisplayImGuiPropertyEdit(Wonder::ModelId& aData, const char* aLabel);

	template<typename T>
	static uint8_t DisplayImGuiPropertyEdit(std::vector<T>& aData, const char* aLabel);
	template<typename T, size_t Size>
	static uint8_t DisplayImGuiPropertyEdit(std::array<T, Size>& aData, const char* aLabel);

	template<typename T>
	uint8_t DisplayImGuiPropertyEdit(std::vector<T>& aData, const char* aLabel)
	{
		using namespace Wonder::ImGuiUtil::PropertyTable;
		uint8_t info = 0;

		ImGui::PushID(aLabel);

		SwitchToLabel();
		if (ImGui::TreeNode(aLabel))
		{
			SwitchToLabel();
			ImGui::TextUnformatted("Size");
			SwitchToValue();
			const int oldSize = static_cast<int>(aData.size());
			int newSize       = oldSize;
			ImGui::InputInt("##size", &newSize);
			newSize = std::clamp(newSize, 0, 69420);

			ImGui::Indent(16);

			int indexToErase = -1;

			int swapFromIndex = -1;
			int swapToIndex = -1;

			std::string elementName;
			for (int i = 0; i < static_cast<int>(aData.size()); ++i)
			{
				ImGui::PushID(i);

				elementName = "Element";
				elementName.append(std::to_string(i));

				NextProperty();
				SwitchToLabel();
				if (ImGui::GetIO().KeyShift && ImGui::Button("X"))
				{
					indexToErase = i;
				}
				ImGui::SameLine();

				ImGui::Spacing();
				if (ImGui::Button(ICON_LC_ARROW_UP))
				{
					swapFromIndex = i;
					swapToIndex = i - 1;
				}
				ImGui::SameLine();

				if (ImGui::Button(ICON_LC_ARROW_DOWN))
				{
					swapFromIndex = i;
					swapToIndex = i + 1;
				}

				info |= DisplayImGuiPropertyEdit(aData[i], elementName.c_str());

				

				ImGui::PopID();
			}

			ImGui::Unindent(16);

			swapFromIndex = std::clamp(swapFromIndex, 0, std::max(oldSize - 1, 0));
			swapToIndex = std::clamp(swapToIndex, 0, std::max(oldSize - 1, 0));

			if (oldSize != newSize)
			{
				aData.resize(newSize);
				info = Wonder::InfoFlag_EditEnded | Wonder::InfoFlag_ChangedValue;
			}
			else if (swapFromIndex != swapToIndex)
			{
				std::swap(aData[swapFromIndex], aData[swapToIndex]);
				info = Wonder::InfoFlag_EditEnded | Wonder::InfoFlag_ChangedValue;
			}
			else if (indexToErase != -1)
			{
				aData.erase(aData.begin() + indexToErase);
				info = Wonder::InfoFlag_EditEnded | Wonder::InfoFlag_ChangedValue;
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
		return info;
	}

	template<typename T, size_t Size>
	uint8_t DisplayImGuiPropertyEdit(std::array<T, Size>& aData, const char* aLabel)
	{
		using namespace Wonder::ImGuiUtil::PropertyTable;
		uint8_t info = 0;

		ImGui::PushID(aLabel);

		SwitchToLabel();
		if (ImGui::TreeNode(aLabel))
		{
			/*SwitchToLabel();
			ImGui::TextUnformatted("Size");
			SwitchToValue();
			const int oldSize = static_cast<int>(aData.size());
			int newSize = oldSize;
			ImGui::InputInt("##size", &newSize);
			newSize = FMath::Clamp(newSize, 0, 69420);*/

			ImGui::Indent(16);

			std::string elementName;
			for (int i = 0; i < static_cast<int>(aData.size()); ++i)
			{
				ImGui::PushID(i);

				elementName = "Element";
				elementName.append(std::to_string(i));

				NextProperty();
				SwitchToLabel();
				info |= DisplayImGuiPropertyEdit(aData[i], elementName.c_str());

				ImGui::PopID();
			}

			ImGui::Unindent(16);

			ImGui::TreePop();
		}

		ImGui::PopID();
		return info;
	}

	template<typename T>
	concept is_enum = std::is_enum_v<T>;
	
	template<is_enum T>
	static uint8_t DisplayImGuiPropertyEdit(T& aData, const char* aLabel)
	{
		uint8_t info = 0;

		ImGui::PushID(aLabel);
		ImGui::Indent(8.0f);
		Wonder::ImGuiUtil::PropertyTable::PresentLabel(aLabel);
		ImGui::Unindent(8.0f);
		Wonder::ImGuiUtil::PropertyTable::SwitchToValue();
		
		std::array<std::string_view, magic_enum::enum_count<T>()> enumNames = magic_enum::enum_names<T>();

		const std::string_view enumName = enumNames[static_cast<size_t>(aData)];
		if (ImGui::BeginCombo("##", enumName.data()))
		{
			for (size_t i = 0; i < enumNames.size(); ++i)
			{
				if (ImGui::Selectable(enumNames[i].data()))
				{
					aData = static_cast<T>(i);
					info = Wonder::InfoFlag_ChangedValue | Wonder::InfoFlag_EditEnded;
				}
			}
			
			ImGui::EndCombo();
		}

		ImGui::PopID();
		return info;
	}
	
}




