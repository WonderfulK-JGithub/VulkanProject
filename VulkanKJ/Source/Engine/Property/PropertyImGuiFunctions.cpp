#include "PropertyImGuiFunctions.h"

#include <IconFontHeaders/IconsLucide.h>

#include "../Util/ImGuiUtil.h"
#include "Clipboard/Clipboard.h"
#include "Clipboard/VariableCopyItem.h"
#include "ECS/Reflection/ComponentReflectionUtil.h"
#include "Util/StringRegistry.h"

using namespace PropertyEdit;
using namespace Wonder::ImGuiUtil;

namespace 
{
	constexpr float NULL_PROPERTY_WIDTH = -1.0f;
	float nextPropertyWidth = NULL_PROPERTY_WIDTH;
}

template<typename T>
uint8_t DisplayCopiableLabel(const char* aLabel, T& aData)
{
	ImGui::Indent(8.0f);
	PropertyTable::PresentLabel(aLabel);
	
	using namespace Wonder::Clipboard;
	
	uint8_t info = 0;
	
	if (ImGui::BeginPopupContextItem("abc"))
	{
		if (ImGui::Button("Copy"))
		{
			Wonder::Clipboard::Store(std::make_shared<VariableCopyItem<T>>(aData));
		}
		ImGui::BeginDisabled(Wonder::Clipboard::HasItem<VariableCopyItem<T>>() == false);
		if (ImGui::Button("Paste"))
		{
			aData = *reinterpret_cast<const T*>(GetItem()->GetData());
			info = Wonder::InfoFlag_ChangedValue | Wonder::InfoFlag_EditEnded;
		}
		ImGui::EndDisabled();
		
		ImGui::EndPopup();
	}
	
	ImGui::Unindent(8.0f);
	PropertyTable::SwitchToValue();
	
	return info;
}

void PropertyEdit::SetNextPropertyWidth(const float aWidth)
{
	nextPropertyWidth = aWidth;
}

uint8_t PropertyEdit::DisplayImGuiPropertyEdit(bool& aData, const char* aLabel)
{
	uint8_t info = 0;

	ImGui::PushID(aLabel);
	info |=DisplayCopiableLabel(aLabel,aData);
	info |= DisplayBoolCheckbox("##boolCheckbox", &aData);

	ImGui::PopID();

	return info;
}

uint8_t PropertyEdit::DisplayImGuiPropertyEdit(int& aData, const char* aLabel)
{
	using namespace LocalInfo;
	uint8_t info = 0;

	ImGui::PushID(aLabel);
	info |=DisplayCopiableLabel(aLabel,aData);
	
	ImGui::PushItemWidth((nextPropertyWidth > 0.0f ? nextPropertyWidth : ImGui::GetContentRegionAvail().x - 4.0f));
	
	const char* format = currentAttribute.format.empty() ? "%d" : currentAttribute.format.c_str();
	
	info |= DisplayDragInt("##intVariable", &aData, currentAttribute.speed,
	                       static_cast<int>(currentAttribute.minValue), static_cast<int>(currentAttribute.maxValue), 0, format);
	ImGui::PopItemWidth();

	ImGui::PopID();
	
	nextPropertyWidth = NULL_PROPERTY_WIDTH;

	return info;
}

uint8_t PropertyEdit::DisplayImGuiPropertyEdit(uint32_t& aData, const char* aLabel)
{
	using namespace LocalInfo;
	uint8_t info = 0;

	ImGui::PushID(aLabel);
	info |=DisplayCopiableLabel(aLabel,aData);
	
	ImGui::PushItemWidth((nextPropertyWidth > 0.0f ? nextPropertyWidth : ImGui::GetContentRegionAvail().x - 4.0f));
	auto intermediate = static_cast<int32_t>(aData);
	info |= DisplayDragInt("##uintVariable", &intermediate, currentAttribute.speed,
						   std::max(static_cast<int>(currentAttribute.minValue),0), static_cast<int>(currentAttribute.maxValue));
	aData = intermediate;
	ImGui::PopItemWidth();

	ImGui::PopID();
	
	nextPropertyWidth = NULL_PROPERTY_WIDTH;

	return info;
}

uint8_t PropertyEdit::DisplayImGuiPropertyEdit(float& aData, const char* aLabel)
{
	using namespace LocalInfo;

	uint8_t info = 0;

	ImGui::PushID(aLabel);
	info |=DisplayCopiableLabel(aLabel,aData);
	
	ImGui::PushItemWidth((nextPropertyWidth > 0.0f ? nextPropertyWidth : ImGui::GetContentRegionAvail().x - 4.0f));
	const char* format = currentAttribute.format.empty() ? "%.3f" : currentAttribute.format.c_str();
	info |= DisplayDragFloat("##floatVariable", &aData, currentAttribute.speed, currentAttribute.minValue, currentAttribute.maxValue,
	                         ImGuiSliderFlags_AlwaysClamp, format);
	
	ImGui::PopItemWidth();

	ImGui::PopID();
	
	nextPropertyWidth = NULL_PROPERTY_WIDTH;

	return info;
}

uint8_t PropertyEdit::DisplayImGuiPropertyEdit(glm::vec2& aData, const char* aLabel)
{
	using namespace LocalInfo;
	uint8_t info = 0;

	ImGui::PushID(aLabel);
	info |=DisplayCopiableLabel(aLabel,aData);

	ImGui::PushItemWidth((nextPropertyWidth > 0.0f ? nextPropertyWidth : ImGui::GetContentRegionAvail().x - 4.0f));
	info |= DisplayDragFloat2("##vector2variable", &aData.x, currentAttribute.speed, currentAttribute.minValue,
	                          currentAttribute.maxValue, ImGuiSliderFlags_AlwaysClamp);
	ImGui::PopItemWidth();

	ImGui::PopID();
	
	nextPropertyWidth = NULL_PROPERTY_WIDTH;

	return info;
}

uint8_t PropertyEdit::DisplayImGuiPropertyEdit(glm::vec3& aData, const char* aLabel)
{
	using namespace LocalInfo;
	uint8_t info = 0;

	ImGui::PushID(aLabel);
	info |=DisplayCopiableLabel(aLabel,aData);

	ImGui::PushItemWidth((nextPropertyWidth > 0.0f ? nextPropertyWidth : ImGui::GetContentRegionAvail().x - 4.0f));
	info |= DisplayDragFloat3("##vector3variable", &aData.x, currentAttribute.speed, currentAttribute.minValue,
	                          currentAttribute.maxValue, ImGuiSliderFlags_AlwaysClamp);
	ImGui::PopItemWidth();

	ImGui::PopID();
	
	nextPropertyWidth = NULL_PROPERTY_WIDTH;

	return info;
}

uint8_t PropertyEdit::DisplayImGuiPropertyEdit(glm::vec4& aData, const char* aLabel)
{
	using namespace LocalInfo;
	uint8_t info = 0;

	ImGui::PushID(aLabel);
	info |=DisplayCopiableLabel(aLabel,aData);

	ImGui::PushItemWidth((nextPropertyWidth > 0.0f ? nextPropertyWidth : ImGui::GetContentRegionAvail().x - 4.0f));
	info |= DisplayDragFloat4("##vector4variable", &aData.x, currentAttribute.speed, currentAttribute.minValue,
		currentAttribute.maxValue, ImGuiSliderFlags_AlwaysClamp);
	ImGui::PopItemWidth();

	ImGui::PopID();
	
	nextPropertyWidth = NULL_PROPERTY_WIDTH;

	return info;
}

uint8_t PropertyEdit::DisplayImGuiPropertyEdit(Wonder::ColorProperty& aData, const char* aLabel)
{
	using namespace LocalInfo;
	uint8_t info = 0;

	const Wonder::ColorProperty initialValue = aData;
	ImGui::PushID(aLabel);
	info |= DisplayCopiableLabel(aLabel,aData);

	/*ImGui::DragFloat3("##vector3variable", aData.myValues, currentAttribute.speed, currentAttribute.minValue, currentAttribute.maxValue,
		"%.3f", ImGuiSliderFlags_AlwaysClamp);*/

	ImGui::ColorEdit4("##colourColor", &aData.color.r,ImGuiColorEditFlags_NoInputs);
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= Wonder::InfoFlag_EditEnded;
	}

	if (initialValue.color != aData.color)
	{
		info |= Wonder::InfoFlag_ChangedValue;
	}

	ImGui::PopID();

	return info;
}

uint8_t PropertyEdit::DisplayImGuiPropertyEdit(std::string& aData, const char* aLabel)
{
	using namespace LocalInfo;
	uint8_t info = 0;
	
	ImGui::PushID(aLabel);
	info |= DisplayCopiableLabel(aLabel,aData);

	ImGui::PushItemWidth((nextPropertyWidth > 0.0f ? nextPropertyWidth : ImGui::GetContentRegionAvail().x - 4.0f));
	info |= DisplayTextInput("##TextInput", aData,true);
	ImGui::PopItemWidth();

	ImGui::PopID();
	
	nextPropertyWidth = NULL_PROPERTY_WIDTH;

	return info;
}

uint8_t PropertyEdit::DisplayImGuiPropertyEdit(Tga::StringId& aData, const char* aLabel)
{
	uint8_t info = 0;
	
	ImGui::PushID(aLabel);
	info |= DisplayCopiableLabel(aLabel,aData);
	std::string variable = aData.GetString();

	ImGui::PushItemWidth((nextPropertyWidth > 0.0f ? nextPropertyWidth : ImGui::GetContentRegionAvail().x - 4.0f));
	info |= DisplayTextInput("##TextInput", variable);
	ImGui::PopItemWidth();

	if (info != 0)
	{
		aData = Tga::StringRegistry::RegisterOrGetString(variable);
	}

	ImGui::PopID();
	
	nextPropertyWidth = NULL_PROPERTY_WIDTH;

	return info;
}

uint8_t PropertyEdit::DisplayImGuiPropertyEdit(Wonder::ModelId& aData, const char* aLabel)
{
	uint8_t info = 0;

	ImGui::PushID(aLabel);
	info |= DisplayCopiableLabel(aLabel,aData);

	info |= DisplayModelReferenceEdit(aData);

	ImGui::PopID();
	return info;
}


