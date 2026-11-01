#pragma once
#include "Id/IdTypes.h"

#include <filesystem>
#include <functional>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <imgui/imgui.h>
#include <magic_enum/magic_enum.hpp>

#include "Clipboard/Clipboard.h"
#include "Clipboard/VariableCopyItem.h"
#include "Container/FixedString.h"

namespace KeyboardInput
{
	enum class Keycode;
}

namespace Tga
{
	class StringId;
}

struct ImVec2;
using FilePath = std::filesystem::path;

namespace Wonder::ImGuiUtil
{
	struct IdScope
	{
		IdScope(const char* aId)
		{
			ImGui::PushID(aId);
		}

		IdScope(const void* aId)
		{
			ImGui::PushID(aId);
		}

		IdScope(int aId)
		{
			ImGui::PushID(aId);
		}

		IdScope(const uint32_t aId)
		{
			ImGui::PushID(static_cast<int>(aId));
		}

		~IdScope()
		{
			ImGui::PopID();
		}
	};

	struct IndentScope
	{
		IndentScope(const float aAmount)
		{
			amount = aAmount;
			ImGui::Indent(aAmount);
		}
		~IndentScope()
		{
			ImGui::Unindent(amount);
		}

		float amount;
	};
	
	uint8_t DisplayModelReferenceEdit(ModelId& aId);
	uint8_t DisplayTextureReferenceEdit(TextureId& aId);
	
	void DrawFieldBox(const char* aFieldText);
	void DrawFieldBoxColored(const char* aFieldText, ImVec4 aColor);

	bool DisplayCombo(const char* aLabel, const char** aNames, uint32_t aCount, uint32_t& aCurrentSelection, const float* aColors = nullptr, const char* aDefaultDisplayValue = "");
	bool DisplayCombo(const char* aLabel, const Tga::StringId* aNames, uint32_t aCount, Tga::StringId& aCurrentSelection);
	bool DisplayComboFiltered(const char* aLabel, const Tga::StringId* aNames, uint32_t aCount, Tga::StringId& aCurrentSelection,
		Goose::String32& aFilter);
	bool DisplayComboFiltered(const char* aLabel, const std::string* aNames, uint32_t aCount, std::string& aCurrentSelection,
		Goose::String32& aFilter);

	bool DisplayMultiCombo(const char* aLabel, const char* aPreview, const char** aNames,bool* aSelection, int aCount);

	uint8_t DisplayLayerEdit(const char* aLabel, uint32_t& aLayerIndex);
	uint8_t DisplayLayerMaskEdit(const char* aLabel, uint32_t& aLayerMask);

	// uint8_t DisplayUserIndexEdit(const char* aLabel,UserIndex& aUserIndex);
	uint8_t DisplayKeyCodeEdit(const char* aLabel, KeyboardInput::Keycode& aKeyCode);
	
	uint8_t DisplayUIScaleAnimEdit(const char* aLabel,int& aAnimIndex);

	uint8_t DisplayDragFloat(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, ImGuiSliderFlags flags = 0, const char* format = "%.3f");
	uint8_t DisplayDragFloat2(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, ImGuiSliderFlags flags = 0, const char* aFormat = "%.3f");
	uint8_t DisplayDragFloat3(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, ImGuiSliderFlags flags = 0, const char* format = "%.3f");
	uint8_t DisplayDragFloat4(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, ImGuiSliderFlags flags = 0, const char* format = "%.3f");
	
	uint8_t DisplaySliderFloat(const char* label, float* v,  float v_min, float v_max, ImGuiSliderFlags flags = 0, const char* format = "%.3f");
	uint8_t DisplaySliderFloat2(const char* label, float* v, float v_min, float v_max, ImGuiSliderFlags flags = 0, const char* format = "%.3f");
	uint8_t DisplaySliderFloat3(const char* label, float* v, float v_min, float v_max, ImGuiSliderFlags flags = 0, const char* format = "%.3f");
	uint8_t DisplaySliderFloat4(const char* label, float* v, float v_min, float v_max, ImGuiSliderFlags flags = 0, const char* format = "%.3f");
	
	uint8_t DisplayDragInt(const char* label, int* v, float v_speed = 1, int v_min = 0, int v_max = 0, ImGuiSliderFlags flags = 0, const char* format = "%d");
	uint8_t DisplayDragInt2(const char* label, int* v, float v_speed = 1, int v_min = 0, int v_max = 0, ImGuiSliderFlags flags = 0, const char* format = "%d");
	uint8_t DisplayDragInt3(const char* label, int* v, float v_speed = 1, int v_min = 0, int v_max = 0, ImGuiSliderFlags flags = 0, const char* format = "%d");
	uint8_t DisplayDragInt4(const char* label, int* v, float v_speed = 1, int v_min = 0, int v_max = 0, ImGuiSliderFlags flags = 0, const char* format = "%d");
	
	uint8_t DisplaySliderInt(const char* label,  int* v, int v_min, int v_max, ImGuiSliderFlags flags = 0, const char* format = "%d");
	uint8_t DisplaySliderInt2(const char* label, int* v, int v_min, int v_max, ImGuiSliderFlags flags = 0, const char* format = "%d");
	uint8_t DisplaySliderInt3(const char* label, int* v, int v_min, int v_max, ImGuiSliderFlags flags = 0, const char* format = "%d");
	uint8_t DisplaySliderInt4(const char* label, int* v, int v_min, int v_max, ImGuiSliderFlags flags = 0, const char* format = "%d");

	uint8_t DisplayColor3(const char* label, float* v);

	uint8_t DisplayColor4(const char* label, float* v,ImGuiColorEditFlags flags = 0);

	//uint8_t DisplayDragInt(const char* label, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0);

	uint8_t DisplayBoolCheckbox(const char* label, bool* v);

	uint8_t DisplayTextInput(const char* label, std::string& v,bool aMultiLine = false);
	uint8_t DisplayTextInput(const char* aLabel, Tga::StringId& aString, bool aMultiLine = false);
	uint8_t DisplayTextInput(const char* aLabel, Goose::String32& aText, bool aConstantUpdate = false, bool aMultiLine = false);
	uint8_t DisplayTextInput(const char* aLabel, Goose::String64& aText, bool aConstantUpdate = false, bool aMultiLine = false);
	uint8_t DisplaySearchInput(const char* aLabel, Goose::String64& aText, const char* aHint = "Search...", bool aConstantUpdate = false, ImGuiInputTextCallback aInputTextCallback = nullptr);


	uint8_t DisplayEnumDropdown(const char* label, const std::vector<const char*>& enumList, int* v);
	
	template<typename E>
	void DisplayEnumFlagsDropdown(const char* label, const char* preview, Flags<E>& aFlags,const ImGuiComboFlags aImGuiFlags = 0)
	{
		std::array<std::string_view, magic_enum::enum_count<E>()> names = magic_enum::enum_names<E>();
		
		if (ImGui::BeginCombo(label, preview, aImGuiFlags))
		{
			for (const std::string_view& name : names)  
			{
				const E value = magic_enum::enum_cast<E>(name).value();
				bool isSelected = aFlags & value;

				if (ImGui::Checkbox(name.data(), &isSelected))  
				{
					aFlags = isSelected ? aFlags | value : aFlags & ~value;
				}
			}
			ImGui::EndCombo();
		}
	}

	[[maybe_unused]] bool CustomSelectable(const char* label, bool selected, ImU32 bg_color, ImGuiSelectableFlags flags, const ImVec2& size_arg);

	//template<std::size_t N>
	//uint8_t DisplayEnumDropdown(const char* label, std::array<const char*, N> enumList, int* v);

	bool IsMouseOverCircle(const ImVec2& aCenter, float aRadius);

	ImVec2 GetNormalized(const ImVec2& aVector);
	ImVec4 BrightenColor(const ImVec4& aColor, float aFactor);

	void HelpMarker(const char* aDescription, bool aSameLine);

	void ThickSeparator(float aThickness,ImU32 aColor = IM_COL32(100, 100, 100,255));

	bool DisplayWorldSpaceHoverCircle(glm::vec3 aWorldPos,float aRadius,const glm::mat4x4& aWorldToProjection,glm::ivec2 aViewportPos, glm::vec2 aViewportSize);
	
	template<typename T>
	uint8_t DisplayCopyPopup(T& aData)
	{
		using namespace Wonder::Clipboard;
	
		uint8_t info = 0;
	
		if (ImGui::BeginPopupContextItem("abc"))
		{
			if (ImGui::Button("Copy"))
			{
				Wonder::Clipboard::Store(std::make_shared<VariableCopyItem<T>>(aData));
				ImGui::CloseCurrentPopup();
			}
			ImGui::BeginDisabled(Wonder::Clipboard::HasItem<VariableCopyItem<T>>() == false);
			if (ImGui::Button("Paste"))
			{
				aData = *reinterpret_cast<const T*>(GetItem()->GetData());
				info = Wonder::InfoFlag_ChangedValue | Wonder::InfoFlag_EditEnded;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndDisabled();
		
			ImGui::EndPopup();
		}
	
		return info;
	}
	
#define DISPLAY_WITH_COPY(INFO,NAME,VALUE,FUNCTION) ImGui::TextUnformatted(NAME);\
	ImGui::PushID(&VALUE);\
	ImGui::SameLine();\
	ImGuiUtil::DisplayCopyPopup(VALUE);\
	info |= (FUNCTION);\
	ImGui::PopID();
	
	namespace PropertyTable
	{
		bool PropertyHeader(const char* aHeaderName);
		
		bool BeginPropertyTable();
		void EndPropertyTable();

		void SwitchToLabel();
		void SwitchToValue();
		void PresentLabel(const char* aName);
		void SetSkipLabel(bool aSkipLabel);

		void NextProperty();
	}
}
