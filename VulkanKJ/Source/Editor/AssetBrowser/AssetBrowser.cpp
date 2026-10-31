#include "AssetBrowser.h"

#include <cstdint>
#include <Engine.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>
#include <AssetManagement/AssetRegistry.h>
#include <AssetManagement/TextureAsset.h>
#include <IconFontHeaders/IconsLucide.h>
#include <Util/ImGuiUtil.h>
#include <Util/Container/FixedString.h>
#include <Vulkan/Image/Sampler.h>
#include <Vulkan/Image/Texture.h>
#include <Util/FileManagement/Settings.h>

#include "ImGuiWidgetsEditor.h"
#include "WonderfulEditor.h"

using namespace AssetBrowser::Internal;

namespace AssetBrowser
{
    enum class WindowNames : uint8_t
    {
        Directories,
        Files,
        Count,
    };

    static const char* localAssetBrowserDockNames[static_cast<int>(WindowNames::Count)] = {
        "Directories",
        "Files",
    };
    
    struct DirectoryCache
    {
        std::vector<Filepath> directories;
        std::vector<Filepath> files;
    };

    static Goose::MappedVector<Filepath, DirectoryCache> localPathCache;
    static Filepath localCorrectAssetRoot;
    static Filepath localCurrentDirectory;
	static Filepath localCurrentRelativeSelectedPath;
    
    static bool localDockingIsInitialized = false;
    static ImGuiWindowClass localWindowClass{};
    
    static Vulkan::Texture localDefaultFileTexture;
    static VkDescriptorSet localDefaultFileDS; //nintendo DS reference
    
	static Vulkan::Texture localFolderFileTexture;
	static VkDescriptorSet localFolderFileDS;
	
	static std::unordered_map<Filepath,Vulkan::Texture> localFiletypeIconTextures;
	static std::unordered_map<Filepath,VkDescriptorSet> localFiletypeIconDS;
	
	static std::vector<std::pair<Filepath,std::string>> localCurrentDirectoryOrder;
	
	constexpr const char* RIGHT_CLICK_POPUP_NAME = "Create Popup";
	constexpr const char* NAMING_POPUP_NAME = "Naming Popup";
	
	enum class NamingMode
	{
		CreatePrefab,
		CreateMaterial,
	};
	
	static NamingMode localNamingMode = NamingMode::CreatePrefab;
	static Goose::String64 localNameInput; 
}

static ImTextureID GetFileTextureIcon(const Filepath& aFilepath)
{
	const std::string extension = aFilepath.extension().string();
	VkDescriptorSet textureDescriptor = AssetBrowser::localDefaultFileDS;
	
	if (extension == Wonder::TEXTURE_FILE_EXTENSION)
	{
		Wonder::TextureAsset& textureAsset = Wonder::GetTextureRegistry().AccessAssetFromName(aFilepath.filename().replace_extension().string().c_str());
				
		if (textureAsset.IsLoaded())
		{
			textureDescriptor = textureAsset.GetImGuiDescriptorSet();
		}
	}
	else if (AssetBrowser::localFiletypeIconDS.contains(extension))
	{
		textureDescriptor = AssetBrowser::localFiletypeIconDS.at(extension);
	}
	
	return reinterpret_cast<ImTextureID>(textureDescriptor);
}

void AssetBrowser::Init()
{
    GeneratePathCache();
	SetNewDirectory(Wonder::GetDataPath());
    
    Vulkan::TextureInput textureInput{};
    textureInput.commandBuffer = Engine::GetInstance().GetMainCommandBuffer();
    textureInput.queue = Engine::GetInstance().GetGraphicsQueue();
    textureInput.logicalDevice = Engine::GetInstance().GetLogicalDevice();
    textureInput.physicalDevice = Engine::GetInstance().GetPhysicalDevice();
    textureInput.filepaths = {Wonder::GetEngineAssetPath() / "Textures/DefaultFileIcon.png"};
    //
    localDefaultFileTexture.Init(textureInput);
    
    localDefaultFileDS =
    ImGui_ImplVulkan_AddTexture(
        Vulkan::GetSampler(Vulkan::SamplerType::BilinearRepeat),
        localDefaultFileTexture.GetImageView(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
	
	localFolderFileTexture.Init(Wonder::GetEngineAssetPath() / "Textures/FolderIcon.png");
	
	localFolderFileDS =
	ImGui_ImplVulkan_AddTexture(
		Vulkan::GetSampler(Vulkan::SamplerType::BilinearRepeat),
		localFolderFileTexture.GetImageView(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);
	
	localFiletypeIconTextures[Wonder::PREFAB_FILE_EXTENSION].Init(Wonder::GetEngineAssetPath() / "Textures/PrefabIcon.png");
	localFiletypeIconDS[Wonder::PREFAB_FILE_EXTENSION] =
	ImGui_ImplVulkan_AddTexture(
		Vulkan::GetSampler(Vulkan::SamplerType::BilinearRepeat),
		localFiletypeIconTextures[Wonder::PREFAB_FILE_EXTENSION].GetImageView(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);
}

void AssetBrowser::Display()
{
    GenerateDockSpace();

    Wonder::DockFlagOverride(ImGuiDockNodeFlags_NoTabBar);
    ImGui::Begin(localAssetBrowserDockNames[static_cast<int>(WindowNames::Directories)],nullptr,ImGuiWindowFlags_NoTitleBar);
    
    ImGui::End();

    Wonder::DockFlagOverride(ImGuiDockNodeFlags_NoTabBar);
    ImGui::Begin(localAssetBrowserDockNames[static_cast<int>(WindowNames::Files)],nullptr,ImGuiWindowFlags_NoTitleBar);
    DisplayFiles();
	DisplayNamingPopup();
    ImGui::End();
}

void AssetBrowser::Destroy()
{
    localDefaultFileTexture.Destroy();
	localFolderFileTexture.Destroy();
    for (auto& texture : localFiletypeIconTextures | std::views::values)
    {
    	texture.Destroy();
    }
}

void AssetBrowser::GeneratePathCache()
{
    const Filepath& dataPath = Wonder::GetDataPath();

    localPathCache.clear();
	
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dataPath))
    {
        if (entry.is_directory())
        {
            localPathCache.try_emplace(entry.path().parent_path());
            localPathCache[entry.path().parent_path()].directories.push_back(entry.path());

            if (localCorrectAssetRoot.empty() && std::filesystem::equivalent(entry.path().parent_path(), Wonder::GetDataPath()))
            {
                localCorrectAssetRoot = entry.path().parent_path();
            }
        }
        else
        {
            localPathCache.try_emplace(entry.path().parent_path());
            localPathCache[entry.path().parent_path()].files.push_back(entry.path());
        }
    }
}

void AssetBrowser::Internal::DisplayDirectoryOrder()
{
	for (int i = static_cast<int>(localCurrentDirectoryOrder.size()) - 1; i > 0; --i)
	{
		ImGui::Text("//");
		ImGui::SameLine();
		if (ImGui::Button(localCurrentDirectoryOrder[i].second.c_str()))
		{
			SetNewDirectory(localCurrentDirectoryOrder[i].first);
			break;
		}

		Wonder::CreateFolderDropTarget(localCurrentDirectoryOrder[i].first);

		ImGui::SameLine();
	}

	ImGui::Text("//");
	ImGui::SameLine();
	ImGui::TextDisabled("%s", localCurrentDirectoryOrder.front().second.c_str());

	//constexpr ImVec2 spacingOffset(0.f, 10.f);
	//ImGui::Dummy(spacingOffset);

	//ImGui::SetCursorPos(ImGui::GetCursorPos() + spacingOffset);
}

void AssetBrowser::Internal::SetNewDirectory(const Filepath& aDirectoryPath)
{
	localCurrentDirectory = aDirectoryPath;
	localCurrentRelativeSelectedPath.clear();
	localCurrentDirectoryOrder.clear();

	Filepath currentPath     = aDirectoryPath;
	const Filepath& rootPath = Wonder::GetDataPath();
	while (currentPath.empty() == false && equivalent(currentPath, rootPath) == false)
	{
		localCurrentDirectoryOrder.emplace_back(currentPath, currentPath.filename().generic_string());
		currentPath = currentPath.parent_path();
	}

	localCurrentDirectoryOrder.emplace_back(rootPath, "Data");
}

void AssetBrowser::Internal::DisplayNamingPopup()
{
	if (ImGui::BeginPopupModal(NAMING_POPUP_NAME,nullptr,ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted("Name:");
		ImGui::SameLine();
		Wonder::ImGuiUtil::DisplayTextInput("##NameInput",localNameInput,true);
		
		bool allowedName = localNameInput.has_only_letters_or_numbers_or_chosen("_");
		
		ImGui::BeginDisabled(allowedName == false);
		if (ImGui::Button("Create"))
		{
			switch (localNamingMode)
			{
				case NamingMode::CreatePrefab:
					WonderfulEditor::GetInstance().CreateAsset(GetCurrentDirectory() / localNameInput.c_str(),Wonder::PREFAB_FILE_EXTENSION);
					break;
				case NamingMode::CreateMaterial:
					WonderfulEditor::GetInstance().CreateAsset(GetCurrentDirectory() / localNameInput.c_str(),Wonder::MATERIAL_FILE_EXTENSION);
					break;
			}
			
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		
		ImGui::SameLine();
		
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		
		ImGui::EndPopup();
	}
}

Filepath& AssetBrowser::GetCurrentDirectory()
{
	return localCurrentDirectory;
}

void AssetBrowser::Internal::GenerateDockSpace()
{
    ImGuiID assetBrowserDockingSpaceID                = ImGui::GetID("AssetBrowserDockingSpace");
    
    localWindowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
    ImGui::DockSpace(assetBrowserDockingSpaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode,&localWindowClass);

    ImGuiID center = assetBrowserDockingSpaceID, left = 0;

    if (localDockingIsInitialized == false)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::DockBuilderRemoveNode(assetBrowserDockingSpaceID); // clear any previous layout
        ImGui::DockBuilderAddNode(assetBrowserDockingSpaceID, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(assetBrowserDockingSpaceID, viewport->WorkSize);

        ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.25f, &left, &center);

        ImGui::DockBuilderDockWindow(localAssetBrowserDockNames[static_cast<int>(WindowNames::Files)], center);
        ImGui::DockBuilderDockWindow(localAssetBrowserDockNames[static_cast<int>(WindowNames::Directories)], left);

        ImGui::DockBuilderFinish(assetBrowserDockingSpaceID);

        localDockingIsInitialized = true;
    }
}

void AssetBrowser::Internal::DisplayFiles()
{
	static bool collapsedDisplay{};
	ImGui::Checkbox("##collapsedDisplay",&collapsedDisplay);
	ImGui::SameLine();
	DisplayDirectoryOrder();
	
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.f, 6.f));
	
	bool anyItemWasHovered{};
	
	if (ImGui::BeginChild("##files"))
	{
		
		ImGui::Dummy(ImVec2(0.f, 10.f));
		
		const DirectoryCache* parentIt = localPathCache.find(localCurrentDirectory);

		if (parentIt == nullptr)
		{
			for (unsigned i = 0; i < localPathCache.size(); ++i)
			{
				auto& parentPath = localPathCache.dataKeys()[i];
				if (std::filesystem::equivalent(localCurrentDirectory, parentPath))
				{
					localCurrentDirectory = parentPath;
					break;
				}
			}
			//return;
		}


		constexpr ImVec2 itemSize     = {64.f, 64.f};
		constexpr float heightSpacing = itemSize.y / 2.f;
		const float totalWidthSpacing = ImGui::GetContentRegionAvail().x / 4.f;

		const int columnCount    = static_cast<int>((ImGui::GetContentRegionAvail().x - totalWidthSpacing) / itemSize.x);
		const float widthSpacing = totalWidthSpacing / static_cast<float>(columnCount + 1);
		ImVec2 startCursorPos    = ImGui::GetCursorPos();
		startCursorPos.x += widthSpacing * 0.5f;
		ImVec2 currentCursorPos = startCursorPos;

		int columnIndex = 0;
		auto displayFileItem = [&](const Filepath& aAbsPath, bool aFolder)
		{
			const Filepath& root = Wonder::GetDataPath();
			const Filepath path = relative(aAbsPath, root);

			const bool isSelected = localCurrentRelativeSelectedPath == path;

			
			ImTextureID imguiTexture = aFolder ? reinterpret_cast<ImTextureID>(localFolderFileDS) : GetFileTextureIcon(path);

			ImDrawCallback drawCallback = nullptr;

			Wonder::AssetListItemStatus itemStatus{};
			if (collapsedDisplay)
			{
				itemStatus = Wonder::AssetListItemCollapsed(path, isSelected, imguiTexture, aFolder, drawCallback);
			}
			else
			{
				itemStatus = Wonder::AssetListItem(path, isSelected,imguiTexture, itemSize, currentCursorPos, aFolder, drawCallback);
			}

			if (itemStatus.doubleClicked)
			{
				if (aFolder)
				{
					SetNewDirectory(aAbsPath);
				}
				else
				{
					WonderfulEditor::GetInstance().OpenDocument(aAbsPath);
				}
			}
			
			anyItemWasHovered |= itemStatus.hovered;
			
			columnIndex++;
			currentCursorPos.x += itemSize.x + widthSpacing;
			if (columnIndex == columnCount)
			{
				columnIndex = 0;
				currentCursorPos.x = startCursorPos.x;
				currentCursorPos.y += itemSize.y + heightSpacing;
			}
		};

		if (parentIt != nullptr)
		{
			const DirectoryCache& parentCache = *parentIt;

			for (const std::filesystem::path& absPath : parentCache.directories)
			{
				displayFileItem(absPath, true);
			}

			for (const std::filesystem::path& absPath : parentCache.files)
			{
				displayFileItem(absPath, false);
			}
		}
	}
	ImGui::EndChild();
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor();
	
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && anyItemWasHovered == false)
	{
		ImGui::OpenPopup(RIGHT_CLICK_POPUP_NAME);
	}
	
	bool openNamePopup = false;
	if (ImGui::BeginPopupContextWindow(RIGHT_CLICK_POPUP_NAME))
	{
		if (ImGui::Button("Prefab " ICON_LC_BOX))
		{
			ImGui::CloseCurrentPopup();
			openNamePopup = true;
			
			localNameInput = "New_Prefab";
			localNamingMode = NamingMode::CreatePrefab;
		}
		if (ImGui::Button("Material " ICON_LC_CIRCLE_ELLIPSIS))
		{
			ImGui::CloseCurrentPopup();
			openNamePopup = true;
			
			localNameInput = "New_Material";
			localNamingMode = NamingMode::CreateMaterial;
		}
		
		ImGui::EndPopup();
	}
	
	if (openNamePopup)
	{
		ImGui::OpenPopup(NAMING_POPUP_NAME);
	}
}
