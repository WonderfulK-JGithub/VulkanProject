#include "ImGuiWidgetsEditor.h"

#include <filesystem>
#include <iostream>

#include "../WonderfulEditor.h"

namespace fs = std::filesystem;

static void DrawCenteredText(const std::string& aText, const float aMaxWidth)
{
	const ImVec2 startCursorPos = ImGui::GetCursorPos();
	float cursorPosY            = startCursorPos.y;

	const int stringSize = static_cast<int>(aText.size());
	int charCount        = 0;

	std::string currentString = aText;
	while (charCount < stringSize)
	{
		ImVec2 lastSize;
		bool cutLine = false;
		for (int i = charCount; i < stringSize - charCount; ++i)
		{
			const ImVec2 currentSize = ImGui::CalcTextSize(currentString.c_str() + charCount, currentString.c_str() + charCount + i);
			if (currentSize.x > aMaxWidth)
			{
				std::string lineToDraw = currentString.substr(charCount, i);

				const float indentation = (aMaxWidth - lastSize.x) * 0.5f;
				ImGui::SetCursorPos(ImVec2(startCursorPos.x + indentation, cursorPosY));
				ImGui::TextUnformatted(lineToDraw.c_str());

				//currentString.erase(currentString.begin(), currentString.begin() + i);
				//currentString = currentString.substr(charCount, i);
				charCount += i;

				cursorPosY += lastSize.y;
				cutLine = true;
				break;
			}
			lastSize = currentSize;
		}

		if (cutLine == false)
		{
			break;
		}
	}

	const std::string lineToDraw = currentString.substr(charCount, static_cast<int>(currentString.size()) - charCount);

	const float indentation = (aMaxWidth - ImGui::CalcTextSize(lineToDraw.c_str()).x) * 0.5f;
	ImGui::SetCursorPos(ImVec2(startCursorPos.x + indentation, cursorPosY));
	ImGui::TextUnformatted(lineToDraw.c_str());
}

static void DrawAssetListItem(const char* aLabel, const bool aSelected, const ImTextureID aTextureID, const float aThumbSize)
{
	if (aTextureID != 0)
	{
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::BeginGroup();
		
		ImGui::Selectable("##", aSelected, ImGuiSelectableFlags_None, ImVec2(0, aThumbSize));
		ImGui::SameLine();
		ImGui::Image(aTextureID, ImVec2(aThumbSize, aThumbSize));
		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(aLabel);
		
		ImGui::EndGroup();
		ImGui::Spacing();
		ImGui::Spacing();
	}
}

void Wonder::CreateFolderDropTarget([[maybe_unused]] const Filepath& aFolderPath)
{
	const ImGuiPayload* payload = ImGui::GetDragDropPayload();

	if (payload == nullptr)
	{
		return;
	}

	const char* acceptablePayloads[] = {MODEL_FILE_EXTENSION,
		PREFAB_FILE_EXTENSION, SCENE_FILE_EXTENSION, TEXTURE_FILE_EXTENSION,
		ANIMCONTROLLER_FILE_EXTENSION, PARTICLE_FILE_EXTENSION,
		"Directory",
		MATERIAL_FILE_EXTENSION, UI_FILE_EXTENSION, LIGHT_FILE_EXTENSION,ANIMATION_FILE_EXTENSION};
	const char* currentPayloadType   = nullptr;

	for (const auto type : acceptablePayloads)
	{
		if (payload->IsDataType(type))
		{
			currentPayloadType = type;
			break;
		}
	}

	if (currentPayloadType == nullptr)
	{
		return;
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (ImGui::AcceptDragDropPayload(currentPayloadType))
		{
			// const FilePath filePath = static_cast<char*>(payload->Data);
			//
			// globalAssetBrowserCall = [filePath, aFolderPath]()
			// {
			// 	LOG_INFO("File %s dropped into editor", filePath.string().c_str());
			// 	if (filePath.has_extension())
			// 	{
			// 		WonderfulEditor::GetEditor()->MoveFileToFolder(filePath, aFolderPath);
			// 	}
			// 	else
			// 	{
			// 		WonderfulEditor::GetEditor()->MoveFolderToFolder(filePath, aFolderPath);
			// 	}
			// };
			
		}
		ImGui::EndDragDropTarget();
	}
}

Wonder::AssetListItemStatus Wonder::AssetListItem(const fs::path& aAssetPath, const bool aSelected, const ImTextureID aTextureID, const ImVec2 aItemSize, const ImVec2 aCursorPos, const bool aFolder, ImDrawCallback
	aDrawCallback)
{
	std::string filename = aAssetPath.filename().string();

	ImGui::SetCursorPos(aCursorPos);
	AssetListItemStatus result;
	ImGui::Selectable(("##" + filename).c_str(), aSelected, ImGuiSelectableFlags_AllowDoubleClick, aItemSize);

	result.selectedAfter = ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right);
	result.hovered       = ImGui::IsItemHovered();
	
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) 
	{
		const std::string path = aAssetPath.string();
		const std::string ext  = aFolder ? "Directory" : aAssetPath.filename().extension().string();

		ImGui::SetDragDropPayload(ext.c_str(), path.c_str(), path.size() + 1);
		DrawAssetListItem(filename.c_str(), false, aTextureID, aItemSize.y);

		ImGui::EndDragDropSource();
	}
	else if (aFolder)
	{
		CreateFolderDropTarget(aAssetPath);
	}

	result.clicked       = ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right);
	result.doubleClicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

	const ImVec2 imageSize   = aItemSize * 0.8f;
	const ImVec2 imageOffset = ImVec2(aItemSize.x * 0.1f, 0.f);

	ImGui::SetCursorPos(aCursorPos + imageOffset);
	
	if (aDrawCallback != nullptr)
	{
		ImGui::GetWindowDrawList()->AddCallback(aDrawCallback, nullptr);
	}
	
	ImGui::Image(aTextureID, imageSize);
	
	if (aDrawCallback != nullptr)
	{
		ImGui::GetWindowDrawList()->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
	}

	const float textWidth = aItemSize.x;
	const ImVec2 textOffset(0.f, imageSize.y);
	ImGui::SetCursorPos(aCursorPos + textOffset);

	constexpr uint64_t MaxCharCount = 16;
	if (filename.size() > MaxCharCount)
	{
		filename.resize(MaxCharCount);
		filename.append("...");
	}

	DrawCenteredText(filename, textWidth);

	return result;
}

Wonder::AssetListItemStatus Wonder::AssetListItemCollapsed(const Filepath& aAssetPath, bool aSelected, ImTextureID aTextureID, const bool aFolder, ImDrawCallback aDrawCallback)
{
	const std::string filename = aAssetPath.filename().string();
	constexpr float imageSize  = 32.f;

	AssetListItemStatus result;
	ImGui::Selectable(("##" + filename).c_str(), aSelected, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SpanAllColumns,
	                  ImVec2(0, imageSize));

	result.selectedAfter = ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right);
	result.hovered       = ImGui::IsItemHovered();
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		const std::string path = aAssetPath.string();
		const std::string ext  = aFolder ? "Directory" : aAssetPath.filename().extension().string();

		ImGui::SetDragDropPayload(ext.c_str(), path.c_str(), path.size() + 1);
		DrawAssetListItem(filename.c_str(), false, aTextureID, imageSize);

		ImGui::EndDragDropSource();
	}
	else if (aFolder)
	{
		CreateFolderDropTarget(aAssetPath);
	}
	result.clicked       = ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right);
	result.doubleClicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

	ImGui::SameLine();
	if (aDrawCallback != nullptr)
	{
		ImGui::GetWindowDrawList()->AddCallback(aDrawCallback, nullptr);
	}
	
	ImGui::Image(aTextureID, ImVec2(imageSize, imageSize));
	
	if (aDrawCallback != nullptr)
	{
		ImGui::GetWindowDrawList()->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
	}
	
	ImGui::SameLine();
	ImGui::TextUnformatted(filename.c_str());

	return result;
}

void Wonder::DockFlagOverride(ImGuiDockNodeFlags aFlags)
{
	ImGuiWindowClass window_class{};
	window_class.DockNodeFlagsOverrideSet = aFlags;
	ImGui::SetNextWindowClass(&window_class);
}
