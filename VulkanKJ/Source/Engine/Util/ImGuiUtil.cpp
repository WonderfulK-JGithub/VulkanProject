#include "ImGuiUtil.h"


#include <filesystem>
#include <utility>
#include <imgui.h>
#include <imgui_internal.h>
#include <IconFontHeaders/IconsLucide.h>

#include <magic_enum/magic_enum.hpp>

#include "AssetManagement/AssetRegistry.h"

static bool localLabelNeedsSeparator = false;
static bool localValueNeedsSeparator = false;

static bool localUsingPropertyTable = false;

static bool localSkipLabel = false;

constexpr uint32_t ALL_BITS = static_cast<uint32_t>(-1);

std::function<bool(Wonder::IdType)> assetExistFunction;
std::function<const char*(Wonder::IdType)> assetNameFunction;
std::function<Wonder::IdType(const char*)> assetIdFunction;
std::function<FilePath(Wonder::IdType)> assetPathFunction;

std::unordered_map<std::string, Goose::String32> assetFilterMap;

std::function<std::tuple<const Wonder::IdType*, const Tga::StringId*, size_t> ()> allIdsAndNamesFunction;


static uint8_t DisplayAssetReferenceEdit(Wonder::IdType& aId, const char* aExtension, const float aRectOffset = 5.0f)
{
	constexpr ImVec4 ASSET_TEXT_COLOR   = ImVec4(1.f, 1.f, 0.9f, 1.f);
	constexpr ImVec4 MISSING_TEXT_COLOR = ImVec4(1.f, 0.1f, 0.1f, 1.f);

	ImVec2 rectOffset{aRectOffset, aRectOffset};
	uint8_t info = 0;

	//ImGui::Dummy({0.f, 1.5f});

	const char* assetText = "-- Drag Here! --";
	ImVec4 textColor      = ImVec4{1.f, 1.f, 1.f, 1.f};

	//ImGui::Dummy(rectOffset);

	if (assetExistFunction(aId) == false)
	{
		if (aId != Wonder::INVALID_ID_TYPE)
		{
			assetText = "(Missing asset)";
			textColor = MISSING_TEXT_COLOR;
		}
	}
	else
	{
		assetText = assetNameFunction(aId);
		textColor = ASSET_TEXT_COLOR;
	}

	auto [ids, names, count] = allIdsAndNamesFunction();

	Wonder::ImGuiUtil::IdScope comboId(&aId);

	ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertFloat4ToU32(textColor));
	bool comboOpen = ImGui::BeginCombo("##AssetDropMenu", assetText, ImGuiComboFlags_WidthFitPreview);
	ImGui::PopStyleColor();

	if (comboOpen)
	{
		assetFilterMap.try_emplace(aExtension);
		Wonder::ImGuiUtil::DisplayTextInput("##AssetTextFilter", assetFilterMap[aExtension], true);

		const Goose::String32 filterString = assetFilterMap.at(aExtension);
		for (uint32_t i = 0; i < count; ++i)
		{
			if (filterString.empty() == false)
			{
				Goose::String32 assetFixedString;
				assetFixedString.safe_assign(names[i].GetString());
				if (assetFixedString.contains(filterString.c_str()) == false)
				{
					continue;
				}
			}

			const bool isSelected = aId == ids[i];
			if (ImGui::Selectable(names[i].GetString(), isSelected))
			{
				aId  = ids[i];
				info = Wonder::InfoFlag_EditEnded | Wonder::InfoFlag_ChangedValue;
			}
		}
		info |= Wonder::InfoFlag_Editing;
		ImGui::EndCombo();
	}

	[[maybe_unused]] bool wasDoubleClicked = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsMouseHoveringRect(
	                                                                                                                          ImGui::GetItemRectMin() - rectOffset, ImGui::GetItemRectMax() + rectOffset);

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(aExtension))
		{
			const FilePath dropped      = static_cast<const char*>(payload->Data);
			const std::string assetName = dropped.filename().generic_string();
			aId                         = assetIdFunction(assetName.c_str());
			info                        = Wonder::InfoFlag_EditEnded | Wonder::InfoFlag_ChangedValue;
		}
		ImGui::EndDragDropTarget();
	}

	// if (wasDoubleClicked && assetExistFunction(aId))
	// {
	// 	Goose::AssetBrowserFocusEvent event;
	// 	event.newFocusPath = assetPathFunction(aId).generic_string();
	//
	// 	if (event.newFocusPath.empty() == false)
	// 	{
	// 		PostMaster::Publish<Goose::AssetBrowserFocusEvent>(event);
	// 	}
	// }

	if (ImGui::GetIO().KeyShift)
	{
		ImGui::SameLine();
		if (ImGui::Button("X"))
		{
			aId  = Wonder::INVALID_ID_TYPE;
			info = Wonder::InfoFlag_EditEnded | Wonder::InfoFlag_ChangedValue;
		}
	}
	ImGui::Dummy(rectOffset);

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayModelReferenceEdit(ModelId& aId)
{
	assetExistFunction = [](const IdType aId)
	{
		return GetModelRegistry().AssetExists(ModelId(aId));
	};
	assetNameFunction = [](const IdType aId)
	{
		return GetModelRegistry().GetNameFromID(ModelId(aId));
	};
	assetPathFunction = [](const IdType aId)
	{
		return GetModelRegistry().GetAssetFilePath(ModelId(aId));
	};

	assetIdFunction = [](const char* aName)
	{
		return GetModelRegistry().GetIDFromName(aName).id;
	};

	allIdsAndNamesFunction = []()
	{
		auto& map = GetModelRegistry().GetIdToNameMap();
		return std::tuple{reinterpret_cast<const IdType*>(map.dataKeys()), map.data(), map.size()};
	};

	return DisplayAssetReferenceEdit(aId.id, MODEL_FILE_EXTENSION);
}

uint8_t Wonder::ImGuiUtil::DisplayTextureReferenceEdit(TextureId& aId)
{
	assetExistFunction = [](const IdType aId)
	{
		return GetTextureRegistry().AssetExists(TextureId(aId));
	};
	assetNameFunction = [](const IdType aId)
	{
		return GetTextureRegistry().GetNameFromID(TextureId(aId));
	};
	assetPathFunction = [](const IdType aId)
	{
		return GetTextureRegistry().GetAssetFilePath(TextureId(aId));
	};

	assetIdFunction = [](const char* aName)
	{
		return GetTextureRegistry().GetIDFromName(aName).id;
	};

	allIdsAndNamesFunction = []()
	{
		auto& map = GetTextureRegistry().GetIdToNameMap();
		return std::tuple{reinterpret_cast<const IdType*>(map.dataKeys()), map.data(), map.size()};
	};

	return DisplayAssetReferenceEdit(aId.id, TEXTURE_FILE_EXTENSION);
}

void Wonder::ImGuiUtil::DrawFieldBox(const char* aFieldText)
{
	constexpr ImVec2 rectOffset{ 5.f, 5.f };

	ImGui::TextUnformatted(aFieldText);

	ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin() - rectOffset, ImGui::GetItemRectMax() + rectOffset,
		IM_COL32(70, 70, 70, 255));
}

void Wonder::ImGuiUtil::DrawFieldBoxColored(const char* aFieldText,const ImVec4 aColor)
{
	constexpr ImVec2 rectOffset{ 5.f, 5.f };

	ImGui::TextColored(aColor, "%s", aFieldText);

	ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin() - rectOffset, ImGui::GetItemRectMax() + rectOffset,
		IM_COL32(70, 70, 70, 255));
}

bool Wonder::ImGuiUtil::DisplayCombo(const char* aLabel, const char** aNames, const uint32_t aCount, uint32_t& aCurrentSelection, const float* aColors, const char* aDefaultDisplayValue)
{
	bool edited = false;

	if (aColors != nullptr)
	{
		const float* currentColor = aColors + aCurrentSelection * 4;
		ImVec4 color{ currentColor[0],currentColor[1] ,currentColor[2] ,currentColor[3] };
		ImGui::PushStyleColor(ImGuiCol_Text, color);
	}

	const char* displayValue = aCurrentSelection < 0 || aCurrentSelection >= aCount ? aDefaultDisplayValue : aNames[aCurrentSelection];
	if (ImGui::BeginCombo(aLabel, displayValue))
	{
		for (uint32_t typeIndex = 0; typeIndex < aCount; ++typeIndex)
		{
			const bool isSelected = typeIndex == aCurrentSelection;
			if (aColors != nullptr)
			{
				const float* currentColor = aColors + typeIndex * 4;
				ImVec4 color{ currentColor [0],currentColor[1] ,currentColor[2] ,currentColor[3] };
				ImGui::PushStyleColor(ImGuiCol_Text, color);
			}

			if (ImGui::Selectable(aNames[typeIndex], isSelected))
			{
				aCurrentSelection = typeIndex;
				edited            = true;
			}

			if (aColors != nullptr)
			{
				ImGui::PopStyleColor();
			}

			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}

	if (aColors != nullptr)
	{
		ImGui::PopStyleColor();
	}

	return edited;
}

bool Wonder::ImGuiUtil::DisplayCombo(const char* aLabel, const Tga::StringId* aNames, const uint32_t aCount, Tga::StringId& aCurrentSelection)
{
	bool edited = false;
	if (ImGui::BeginCombo(aLabel, aCurrentSelection.GetString()))
	{
		for (uint32_t typeIndex = 0; typeIndex < aCount; ++typeIndex)
		{
			const bool isSelected = aCurrentSelection == aNames[typeIndex];
			if (ImGui::Selectable(aNames[typeIndex].GetString(), isSelected))
			{
				aCurrentSelection = aNames[typeIndex];
				edited            = true;
			}

			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}

	return edited;
}

bool Wonder::ImGuiUtil::DisplayComboFiltered(const char* aLabel, const Tga::StringId* aNames, const uint32_t aCount, Tga::StringId& aCurrentSelection, Goose::String32& aFilter)
{
	bool edited = false;
	if (ImGui::BeginCombo(aLabel, aCurrentSelection.GetString()))
	{
		ImGui::PushID(aLabel);
		DisplayTextInput("##Text", aFilter, true);
		ImGui::PopID();

		for (uint32_t typeIndex = 0; typeIndex < aCount; ++typeIndex)
		{
			if (aFilter.empty() == false)
			{
				Goose::String64 temp;
				temp.safe_assign(aNames[typeIndex].GetString());
				if (temp.contains(aFilter.c_str()) == false)
				{
					continue;
				}
			}

			const bool isSelected = aCurrentSelection == aNames[typeIndex];
			if (ImGui::Selectable(aNames[typeIndex].GetString(), isSelected))
			{
				aCurrentSelection = aNames[typeIndex];
				edited = true;
			}

			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}
	

	return edited;
}

bool Wonder::ImGuiUtil::DisplayMultiCombo(const char* aLabel, const char* aPreview, const char** aNames, bool* aSelection, const int aCount)
{
	bool edited = false;
	if (ImGui::BeginCombo(aLabel, aPreview))
	{
		for (int typeIndex = 0; typeIndex < aCount; ++typeIndex)
		{
			const bool isSelected = aSelection[typeIndex];
			if (ImGui::Selectable(aNames[typeIndex], isSelected))
			{
				aSelection[typeIndex] = !aSelection[typeIndex];
				edited = true;
			}

			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}

	return edited;
}

// uint8_t Wonder::ImGuiUtil::DisplayLayerEdit(const char* aLabel, uint32_t& aLayerIndex)
// {
// 	uint8_t info = 0u;
//
// 	auto& names = EngineLayers::AccessLayers();
//
// 	if (ImGui::BeginCombo(aLabel, names[aLayerIndex].GetString()))
// 	{
// 		for (int i = 0; i < static_cast<int>(names.size()); ++i)
// 		{
// 			if (names[i].IsEmpty())
// 			{
// 				continue;
// 			}
//
// 			ImGui::PushID(i);
//
// 			const bool isSelected = std::cmp_equal(aLayerIndex, i);
// 			if (ImGui::Selectable(names[i].GetString(), isSelected))
// 			{
// 				aLayerIndex = i;
// 				info = PropertyEdit::InfoFlag_ChangedValue | PropertyEdit::InfoFlag_EditEnded;
// 			}
//
// 			if (isSelected)
// 			{
// 				ImGui::SetItemDefaultFocus();
// 			}
//
// 			ImGui::PopID();
// 		}
// 		info |= PropertyEdit::InfoFlag_Editing;
// 		ImGui::EndCombo();
// 	}
//
// 	return info;
// }
//
// uint8_t Wonder::ImGuiUtil::DisplayLayerMaskEdit(const char* aLabel, uint32_t& aLayerMask)
// {
// 	uint8_t info = 0u;
//
// 	auto& names = EngineLayers::AccessLayers();
//
// 	const char* comboPreviewName = "- Mixed -";
//
// 	if (aLayerMask == 0u)
// 	{
// 		comboPreviewName = "- Nothing -";
// 	}
// 	else
// 	{
// 		bool allLayersAreOn = true;
// 		for (uint32_t i = 0; i < names.size(); ++i)
// 		{
// 			const uint32_t currentMask = (1u << i);
//
// 			if (names[i].IsEmpty())
// 			{
// 				continue;
// 			}
//
// 			if ((currentMask & aLayerMask) == 0u)
// 			{
// 				allLayersAreOn = false;
// 			}
// 		}
//
// 		if (allLayersAreOn)
// 		{
// 			comboPreviewName = "- Everything -";
// 		}
// 	}
//
// 	if (ImGui::BeginCombo(aLabel, comboPreviewName))
// 	{
// 		for (uint32_t i = 0; i < names.size(); ++i)
// 		{
// 			if (names[i].IsEmpty())
// 			{
// 				continue;
// 			}
//
// 			const uint32_t currentMask = (1u << i);
//
// 			const bool isSelected = (aLayerMask & currentMask) != 0u;
// 			if (ImGui::Selectable(names[i].GetString(), isSelected,ImGuiSelectableFlags_DontClosePopups))
// 			{
// 				if (isSelected)
// 				{
// 					aLayerMask &= currentMask ^ ALL_BITS;
// 				}
// 				else
// 				{
// 					aLayerMask |= currentMask;
// 				}
// 				
// 				info = PropertyEdit::InfoFlag_ChangedValue | PropertyEdit::InfoFlag_EditEnded;
// 			}
//
// 			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 16.f);
//
// 			if (isSelected)
// 			{
// 				ImGui::Text("O");
// 			}
// 			else
// 			{
// 				ImGui::Text(" ");
// 			}
// 		}
//
// 		ImGui::Separator();
// 		ImGui::Dummy({ 0.f,10.f });
// 		if (ImGui::Selectable("Nothing", false, ImGuiSelectableFlags_DontClosePopups))
// 		{
// 			aLayerMask = 0u;
// 			info = PropertyEdit::InfoFlag_ChangedValue | PropertyEdit::InfoFlag_EditEnded;
// 		}
// 		if (ImGui::Selectable("Everything", false, ImGuiSelectableFlags_DontClosePopups))
// 		{
// 			aLayerMask = ALL_BITS;
// 			info = PropertyEdit::InfoFlag_ChangedValue | PropertyEdit::InfoFlag_EditEnded;
// 		}
//
// 		info |= PropertyEdit::InfoFlag_Editing;
// 		ImGui::EndCombo();
// 	}
//
// 	return info;
// }

// uint8_t Wonder::ImGuiUtil::DisplayUserIndexEdit(const char* aLabel, UserIndex& aUserIndex)
// {
// 	uint8_t info = 0;
//
// 	/*int currentIndexIndex = -1;
// 	for (uint32_t i = 0; i < std::size(UserInfo::USER_INDICES_IN_USE); ++i)
// 	{
// 		if (aUserIndex == )
// 	}*/
//
// 	if (ImGui::BeginCombo(aLabel, UserInfo::GetNameFromUserIndex(aUserIndex)))
// 	{
// 		for (uint32_t typeIndex = 0; typeIndex < std::size(UserInfo::USER_INDICES_IN_USE); ++typeIndex)
// 		{
// 			const UserIndex currentIndex = UserInfo::USER_INDICES_IN_USE[typeIndex];
// 			const bool isSelected = aUserIndex == currentIndex;
// 			if (ImGui::Selectable(UserInfo::GetNameFromUserIndex(currentIndex), isSelected))
// 			{
// 				aUserIndex = currentIndex;
// 				info = PropertyEdit::InfoFlag_ChangedValue | PropertyEdit::InfoFlag_EditEnded;
// 			}
//
// 			if (isSelected)
// 			{
// 				ImGui::SetItemDefaultFocus();
// 			}
// 		}
// 		info |= PropertyEdit::InfoFlag_Editing;
// 		ImGui::EndCombo();
// 	}
//
// 	return info;
// }

uint8_t Wonder::ImGuiUtil::DisplayDragFloat(const char* label, float* v, const float v_speed, const float v_min, const float v_max,
	const ImGuiSliderFlags flags, const char* format)
{
	uint8_t info        = 0;
	const float initial = *v;

	ImGui::DragFloat(label, v, v_speed, v_min, v_max, format, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial != *v)
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayBoolCheckbox(const char* label, bool* v)
{
	uint8_t info = 0;
	const bool initial = *v;

	ImGui::Checkbox(label, v);

	if (initial != *v)
	{
		info |= InfoFlag_EditEnded;
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayDragFloat2(const char* label, float* v, const float v_speed, const float v_min, const float v_max,
	const ImGuiSliderFlags flags,
	const char* aFormat)
{
	uint8_t info        = 0;
	const float initial[2] = { v[0], v[1] };
	ImGui::DragFloat2(label, v, v_speed, v_min, v_max, aFormat, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial[0] != v[0] || initial[1] != v[1])
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayDragFloat3(const char* label, float* v, const float v_speed, const float v_min, const float v_max,
	const ImGuiSliderFlags flags,
	const char* format)
{
	uint8_t info           = 0;
	const float initial[3] = {v[0], v[1], v[2]};
	
	ImGui::DragFloat3(label, v, v_speed, v_min, v_max, format, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial[0] != v[0] || initial[1] != v[1] || initial[2] != v[2])
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayDragFloat4(const char* label, float* v, const float v_speed, const float v_min, const float v_max, const ImGuiSliderFlags flags, const char* format)
{
	uint8_t info           = 0;
	const float initial[4] = { v[0], v[1], v[2], v[3] };
	ImGui::DragFloat4(label, v, v_speed, v_min, v_max, format, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial[0] != v[0] || initial[1] != v[1] || initial[2] != v[2] || initial[3] != v[3])
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplaySliderFloat(const char* label, float* v, const float v_min, const float v_max, const ImGuiSliderFlags flags, const char* format)
{
	uint8_t info        = 0;
	const float initial = *v;
	
	ImGui::SliderFloat(label, v, v_min, v_max, format, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial != *v)
	{
		info |= InfoFlag_ChangedValue;
	}
	
	return info;
}

uint8_t Wonder::ImGuiUtil::DisplaySliderFloat2(const char* label, float* v, const float v_min, const float v_max, const ImGuiSliderFlags flags, const char* format)
{
	uint8_t info           = 0;
	const float initial[2] = { v[0], v[1] };

	ImGui::SliderFloat2(label, v, v_min, v_max, format, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial[0] != v[0] || initial[1] != v[1])
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplaySliderFloat3(const char* label, float* v, const float v_min, const float v_max, const ImGuiSliderFlags flags, const char* format)
{
	uint8_t info           = 0;
	const float initial[3] = { v[0], v[1], v[2] };

	ImGui::SliderFloat3(label, v, v_min, v_max, format, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial[0] != v[0] || initial[1] != v[1] || initial[2] != v[2])
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplaySliderFloat4(const char* label, float* v, const float v_min, const float v_max, const ImGuiSliderFlags flags, const char* format)
{
	uint8_t info           = 0;
	const float initial[4] = { v[0], v[1], v[2], v[3] };

	ImGui::SliderFloat4(label, v, v_min, v_max, format, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial[0] != v[0] || initial[1] != v[1] || initial[2] != v[2] || initial[3] != v[3])
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayDragInt(const char* label, int* v, const float v_speed, const int v_min, const int v_max, const ImGuiSliderFlags flags, const char* format)
{
	uint8_t info        = 0;
	const int initial = *v;

	ImGui::DragInt(label, v, v_speed, v_min, v_max, format, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial != *v)
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayDragInt2(const char* label, int* v, const float v_speed, const int v_min, const int v_max, const ImGuiSliderFlags flags, const char* format)
{
	uint8_t info = 0;
	const int initial[3] = { v[0], v[1] };
	ImGui::DragInt2(label, v, v_speed, v_min, v_max, format, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial[0] != v[0] || initial[1] != v[1])
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayDragInt3(const char* label, int* v, const float v_speed, const int v_min, const int v_max, const ImGuiSliderFlags flags, const char* format)
{
	uint8_t info = 0;
	const int initial[3] = { v[0], v[1], v[2] };
	ImGui::DragInt3(label, v, v_speed, v_min, v_max, format, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial[0] != v[0] || initial[1] != v[1] || initial[2] != v[2])
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayDragInt4(const char* label, int* v, const float v_speed, const int v_min, const int v_max, const ImGuiSliderFlags flags, const char* format)
{
	uint8_t info = 0;
	const int initial[4] = { v[0], v[1], v[2], v[3] };
	ImGui::DragInt4(label, v, v_speed, v_min, v_max, format, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial[0] != v[0] || initial[1] != v[1] || initial[2] != v[2] || initial[3] != v[3])
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplaySliderInt(const char* label, int* v, const int v_min, const int v_max, const ImGuiSliderFlags flags, const char* format)
{
	uint8_t info        = 0;
	const int initial = *v;
	
	ImGui::SliderInt(label, v, v_min, v_max, format, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial != *v)
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplaySliderInt2(const char* label, int* v, const int v_min, const int v_max, const ImGuiSliderFlags flags, const char* format)
{
	uint8_t info = 0;
	const int initial[4] = { v[0], v[1] };

	ImGui::SliderInt2(label, v, v_min, v_max, format, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial[0] != v[0] || initial[1] != v[1])
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplaySliderInt3(const char* label, int* v, const int v_min, const int v_max, const ImGuiSliderFlags flags, const char* format)
{
	uint8_t info = 0;
	const int initial[3] = { v[0], v[1], v[2] };

	ImGui::SliderInt3(label, v, v_min, v_max, format, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial[0] != v[0] || initial[1] != v[1] || initial[2] != v[2])
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplaySliderInt4(const char* label, int* v, const int v_min, const int v_max, const ImGuiSliderFlags flags, const char* format)
{
	uint8_t info = 0;
	const int initial[4] = { v[0], v[1], v[2], v[3] };

	ImGui::SliderInt4(label, v, v_min, v_max, format, flags);
	
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial[0] != v[0] || initial[1] != v[1] || initial[2] != v[2] || initial[3] != v[3])
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayColor3(const char* label, float* v)
{
	uint8_t info           = 0;
	const float initial[3] = {v[0], v[1], v[2]};
	ImGui::ColorEdit3(label, v);
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial[0] != v[0] || initial[1] != v[1] || initial[2] != v[2])
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayColor4(const char* label, float* v, ImGuiColorEditFlags flags)
{
	uint8_t info           = 0;
	const float initial[4] = { v[0], v[1], v[2], v[3] };
	ImGui::ColorEdit4(label, v, flags);
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial[0] != v[0] || initial[1] != v[1] || initial[2] != v[2] || initial[3] != v[3])
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayTextInput(const char* label, std::string& v,const bool aMultiLine)
{
	uint8_t info = 0;
	const std::string initial = v;

	char buffer[256];
	strncpy(buffer, v.c_str(), sizeof(buffer));
	buffer[sizeof(buffer) - 1] = '\0';

	if (aMultiLine)
	{
		if (ImGui::InputTextMultiline(label, buffer, sizeof(buffer)))
		{
			v = buffer;
		}
	}
	else
	{
		if (ImGui::InputText(label, buffer, sizeof(buffer)))
		{
			v = buffer;
		}
	}
	

	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial != v)
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayTextInput(const char* aLabel, Tga::StringId& aString, const bool aMultiLine)
{
	uint8_t info = 0;

	char buffer[256];
	strncpy(buffer, aString.GetString(), sizeof(buffer));
	buffer[sizeof(buffer) - 1] = '\0';

	if (aMultiLine)
	{
		ImGui::InputTextMultiline(aLabel, buffer, sizeof(buffer));
	}
	else
	{
		ImGui::InputText(aLabel, buffer, sizeof(buffer));
	}


	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
		info |= InfoFlag_ChangedValue;
		aString = Tga::StringRegistry::RegisterOrGetString(buffer);
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayTextInput(const char* aLabel, Goose::String32& aText, const bool aConstantUpdate, const bool aMultiLine)
{
	uint8_t info = 0;

	char buffer[30];
	strncpy(buffer, aText.c_str(), sizeof(buffer));
	buffer[sizeof(buffer) - 1] = '\0';

	const Goose::String32 initial = aText;

	if (aMultiLine)
	{
		ImGui::InputTextMultiline(aLabel, buffer, sizeof(buffer));
	}
	else
	{
		ImGui::InputText(aLabel, buffer, sizeof(buffer));
	}

	if (aConstantUpdate)
	{
		aText = buffer;
		if (initial != aText)
		{
			info |= InfoFlag_ChangedValue;
		}
	}

	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded | InfoFlag_ChangedValue;
		aText = buffer;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayTextInput(const char* aLabel, Goose::String64& aText, const bool aConstantUpdate, const bool aMultiLine)
{
	uint8_t info = 0;

	char buffer[62];
	strncpy(buffer, aText.c_str(), sizeof(buffer));
	buffer[sizeof(buffer) - 1] = '\0';

	const Goose::String64 initial = aText;

	if (aMultiLine)
	{
		ImGui::InputTextMultiline(aLabel, buffer, sizeof(buffer));
	}
	else
	{
		ImGui::InputText(aLabel, buffer, sizeof(buffer));
	}

	if (aConstantUpdate)
	{
		aText = buffer;
		if (initial != aText)
		{
			info |= InfoFlag_ChangedValue;
		}
	}

	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded | InfoFlag_ChangedValue;
		aText = buffer;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplaySearchInput(const char* aLabel, Goose::String64& aText, const char* aHint, bool aConstantUpdate, ImGuiInputTextCallback aInputTextCallback)
{
	uint8_t info = 0;
	
	char buffer[62];
	strncpy(buffer, aText.c_str(), sizeof(buffer));
	buffer[sizeof(buffer) - 1] = '\0';

	const Goose::String64 initial = aText;
	
	ImGui::InputTextWithHint(aLabel, aHint, buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue, aInputTextCallback);

	if (aConstantUpdate)
	{
		aText = buffer;
		if (initial != aText)
		{
			info |= InfoFlag_ChangedValue;
		}
	}

	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded | InfoFlag_ChangedValue;
		aText = buffer;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}

	return info;
}

uint8_t Wonder::ImGuiUtil::DisplayEnumDropdown(const char* label, const std::vector<const char*>& enumList, int* v)
{
	uint8_t info = 0;
	const int initial = *v;

	if (ImGui::BeginCombo(label, enumList[*v])) 
	{
		for (int i = 0; i < static_cast<int>(enumList.size()); ++i) 
		{
			bool isSelected = *v == i;

			if (ImGui::Selectable(enumList[i], isSelected)) 
			{
				*v = i;
			}
			if (isSelected) 
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		info |= InfoFlag_EditEnded;
	}
	if (ImGui::IsItemActive())
	{
		info |= InfoFlag_Editing;
	}
	if (initial != *v)
	{
		info |= InfoFlag_ChangedValue;
	}

	return info;
}

bool Wonder::ImGuiUtil::CustomSelectable(const char* label, bool selected, const ImU32 bg_color, const ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->ChannelsSplit(2);
	draw_list->ChannelsSetCurrent(1);

	ImGui::PushID(label);
	// Make default selectable visuals transparent so we can draw rounded backgrounds ourselves
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));

	const bool result = ImGui::Selectable("##sel", selected, flags, size_arg);

	ImGui::PopStyleColor(3);

	const ImVec2 p_min = ImGui::GetItemRectMin();
	const ImVec2 p_max = ImGui::GetItemRectMax();

	const bool hovered = ImGui::IsItemHovered();
	const bool active = ImGui::IsItemActive();

	// Determine rounding (use style FrameRounding or fraction of height)
	float rounding = ImGui::GetStyle().FrameRounding;
	if (rounding <= 0.0f) rounding = size_arg.y * 0.2f;
	
	// Resolve colors from style so they match the rest of your UI
	const ImU32 idleColor = bg_color;
	const ImU32 hoverColor = selected ? ImGui::GetColorU32(ImGuiCol_HeaderHovered) : ImGui::ColorConvertFloat4ToU32(ImVec4(0.13f, 0.13f, 0.13f, 1.f));
	const ImU32 activeColor = ImGui::GetColorU32(ImGuiCol_HeaderActive);
	const ImU32 selectedColor = ImGui::GetColorU32(ImGuiCol_Header);
	ImU32 textColor = IM_COL32(255, 255, 255, 255);
	// Draw rounded background on channel 0 (behind other visuals)
	draw_list->ChannelsSetCurrent(0);
	if (selected && !hovered)
	{
		draw_list->AddRectFilled(p_min, p_max, selectedColor, rounding);
	}
	else if (active && !hovered)
	{
		draw_list->AddRectFilled(p_min, p_max, activeColor, rounding);
	}
	else if (hovered)
	{
		draw_list->AddRectFilled(p_min, p_max, hoverColor, rounding);
	}
	else
	{
		draw_list->AddRectFilled(p_min, p_max, idleColor, rounding);
		//textColor = IM_COL32(255, 255, 255, 255);
	}

	// Draw the label centered on top channel (channel 1)
	draw_list->ChannelsSetCurrent(1);
	const ImVec2 textSize = ImGui::CalcTextSize(label);
	const ImVec2 center = ImVec2((p_min.x + p_max.x) * 0.5f, (p_min.y + p_max.y) * 0.5f);
	const ImVec2 textPos = ImVec2(center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f);


	draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize(), textPos, textColor, label);

	ImGui::PopID();
	draw_list->ChannelsMerge();
	return result;
}

bool Wonder::ImGuiUtil::IsMouseOverCircle(const ImVec2& aCenter, const float aRadius)
{
	const ImVec2 distanceVector = ImGui::GetMousePos() - aCenter;
	return aRadius * aRadius >= (distanceVector.x * distanceVector.x + distanceVector.y * distanceVector.y);
}

ImVec2 Wonder::ImGuiUtil::GetNormalized(const ImVec2& aVector)
{
	return aVector / sqrtf(aVector.x * aVector.x + aVector.y * aVector.y);
}

ImVec4 Wonder::ImGuiUtil::BrightenColor(const ImVec4& aColor, const float aFactor)
{
	ImVec4 newColor = aColor * aFactor;
	newColor.x      = std::clamp(newColor.x, 0.f, 1.f);
	newColor.y      = std::clamp(newColor.y, 0.f, 1.f);
	newColor.z      = std::clamp(newColor.z, 0.f, 1.f);
	newColor.w      = std::clamp(newColor.w, 0.f, 1.f);
	return newColor;
}

void Wonder::ImGuiUtil::HelpMarker(const char* aDescription, const bool aSameLine)
{
	if (aSameLine)
	{
		ImGui::SameLine();
	}
	
	ImGui::TextDisabled(ICON_LC_CIRCLE_HELP);
	if (ImGui::BeginItemTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(aDescription);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void Wonder::ImGuiUtil::ThickSeparator(const float aThickness, const ImU32 aColor)
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 size = ImGui::GetContentRegionAvail();
	size.y = aThickness;
	drawList->AddRectFilled(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + size, aColor);
	ImGui::Dummy({ 0.f,aThickness });
}

bool Wonder::ImGuiUtil::DisplayWorldSpaceHoverCircle(const glm::vec3 aWorldPos,const float aRadius, const glm::mat4x4& aWorldToProjection, 
	const glm::ivec2 aViewportPos, const glm::vec2 aViewportSize)
{
	const glm::vec4 worldSpace = glm::vec4(aWorldPos,1.f);
	glm::vec4 projectionSpace = worldSpace * aWorldToProjection;
	glm::vec4 ndc = projectionSpace / projectionSpace.w;

	const glm::vec2 viewPortSize = static_cast<glm::vec2>(aViewportSize);
	const ImVec2 screenPos = { (viewPortSize.x + ndc.x * viewPortSize.x) * 0.5f,
		(viewPortSize.y - ndc.y * viewPortSize.y) * 0.5f };

	constexpr ImU32 defaultColor = IM_COL32(200, 200, 0, 255);
	constexpr ImU32 hoverColor = IM_COL32(180, 200, 100, 255);
	const ImVec2 viewPortPos{ static_cast<float>(aViewportPos.x), static_cast<float>(aViewportPos.y) };

	bool isHovered = IsMouseOverCircle(viewPortPos + screenPos, aRadius);

	ImGui::GetWindowDrawList()->AddCircleFilled(viewPortPos + screenPos, aRadius, isHovered ? hoverColor : defaultColor);

	return isHovered;
}

bool Wonder::ImGuiUtil::PropertyTable::PropertyHeader(const char* aHeaderName)
{
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{ 0.16f, 0.16f, 0.16f, 1.0f });
	bool result = ImGui::CollapsingHeader(aHeaderName, ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PopStyleColor();

	return result;
}

bool Wonder::ImGuiUtil::PropertyTable::BeginPropertyTable()
{
	localUsingPropertyTable = true;
	const bool returnValue = ImGui::BeginTable("##Properties", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable);
	return returnValue;
}

void Wonder::ImGuiUtil::PropertyTable::EndPropertyTable()
{
	ImGui::EndTable();
	localUsingPropertyTable = false;
}

void Wonder::ImGuiUtil::PropertyTable::SwitchToLabel()
{
	if (localUsingPropertyTable == false)
	{
		return;
	}
	ImGui::TableSetColumnIndex(0);

	if (localLabelNeedsSeparator)
	{
		localLabelNeedsSeparator = false;
		ImGui::Separator();
	}

	ImGui::AlignTextToFramePadding();
}

void Wonder::ImGuiUtil::PropertyTable::SwitchToValue()
{
	if (localUsingPropertyTable == false)
	{
		return;
	}
	ImGui::TableSetColumnIndex(1);

	if (localValueNeedsSeparator)
	{
		localValueNeedsSeparator = false;
		ImGui::Separator();
	}

	//ImGui::PushItemWidth(-1);
}

void Wonder::ImGuiUtil::PropertyTable::PresentLabel(const char* aName)
{
	if (localSkipLabel)
	{
		localSkipLabel = false;
		return;
	}
	if (localUsingPropertyTable == false)
	{
		return;
	}

	SwitchToLabel();
	ImGui::TextUnformatted(aName);
}

void Wonder::ImGuiUtil::PropertyTable::SetSkipLabel(const bool aSkipLabel)
{
	localSkipLabel = aSkipLabel;
}

void Wonder::ImGuiUtil::PropertyTable::NextProperty()
{
	if (localUsingPropertyTable == false)
	{
		return;
	}

	ImGui::TableNextRow();
	localValueNeedsSeparator = true;
	localLabelNeedsSeparator = true;
}

