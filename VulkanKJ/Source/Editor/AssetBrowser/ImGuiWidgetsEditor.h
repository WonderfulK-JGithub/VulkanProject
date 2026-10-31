#pragma once

#include <filesystem>

#include <imgui/imgui.h>
#include <Util/FileManagement/FileTypes.h>

namespace Wonder
{
	struct AssetListItemStatus
	{
		bool selectedAfter = false;
		bool clicked = false;
		bool doubleClicked = false;
		bool hovered = false;
	};

	void CreateFolderDropTarget(const Filepath& aFolderPath);

	AssetListItemStatus AssetListItem(const Filepath& aAssetPath, bool aSelected, ImTextureID aTextureID, ImVec2 aItemSize, ImVec2 aCursorPos, bool aFolder, ImDrawCallback
		aDrawCallback = nullptr);
	AssetListItemStatus AssetListItemCollapsed(const Filepath& aAssetPath, bool aSelected, ImTextureID aTextureID, bool aFolder, ImDrawCallback aDrawCallback = nullptr);

	void DockFlagOverride(ImGuiDockNodeFlags aFlags);
}
