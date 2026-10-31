#include "WonderfulEditor.h"

#include <cstdint>
#include <Engine.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <AssetManagement/AssetRegistry.h>
#include <ID/IdFunctions.h>
#include <nlohmann/json.hpp>
#include <Shaders/Shaders.h>
#include <Util/FileManagement/FileDialog.h>
#include <Util/Logging/Logging.h>

#include "EditorGlobals.h"
#include "AssetManagement/TextureAsset.h"

#include "AssetBrowser/AssetBrowser.h"
#include "Document/DocumentBase.h"
#include "Document/MaterialDocument.h"
#include "Document/ModelDocument.h"
#include "Document/PrefabDocument.h"
#include "Document/TextureDocument.h"

enum class MainWindowNames : uint8_t
{
    DocumentDock,
    AssetBrowser,
    Count,
};

static const char* globalDockNames[static_cast<int>(MainWindowNames::Count)] = {
    "DocumentWindow",
    "AssetBrowser",
};

struct WindowInfo
{
    ImGuiWindowFlags mainWindowFlags   = 0;
    ImGuiWindowFlags assetBrowserFlags = 0;
    ImGuiWindowFlags documentFlags     = 0;
};

namespace 
{
    ImGuiWindowClass myMainWindowClass;

    ImGuiID myMainDockingSpaceID  = 0;

    WindowInfo myWindowInfo;
    
    std::unordered_map<Filepath,std::function<nlohmann::json(const char*)>> localExtensionToDefaultData;
    std::unordered_map<Filepath,std::function<void(const Filepath&)>> localExtensionToRegisterFunction;
    std::unordered_map<Filepath,std::function<std::shared_ptr<Wonder::DocumentBase>()>> localExtensionToDocumentConstructor;
    
    WonderfulEditor* localInstance = nullptr;
}


void WonderfulEditor::Init()
{
    localInstance = this;
    myWindowInfo.mainWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    myWindowInfo.mainWindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
    myWindowInfo.mainWindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoMove;

    myWindowInfo.assetBrowserFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    myWindowInfo.documentFlags     = myWindowInfo.assetBrowserFlags;
    
    AssetBrowser::Init();
    MakeFileExtensionFunctions();
    
    Vulkan::Internal::LoadAllShaderNamesAndPaths();
}

void WonderfulEditor::Update()
{
    for (uint32_t i = 0; i < static_cast<uint32_t>(myDocuments.size()); ++i)
    {
        if (myDocuments[i]->ShouldClose())
        {
            myDocuments[i]->Close();
            myDocuments.erase(myDocuments.begin() + i);
            i--;
        }
    }
    
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    {
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    }

    ImGui::Begin("Editor", nullptr, myWindowInfo.mainWindowFlags);
    
    DisplayTopBar();
    GenerateDocks();
    
    ImGui::Begin(globalDockNames[static_cast<int>(MainWindowNames::DocumentDock)], nullptr, myWindowInfo.documentFlags);
    
    Wonder::globalDocumentDockSize = ImGui::GetContentRegionAvail();
    ImGui::DockSpace(Wonder::globalDocumentDockingSpaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode,
                     nullptr);
    ImGui::End();
    
    ImGui::Begin(globalDockNames[static_cast<int>(MainWindowNames::AssetBrowser)], nullptr, 0);
    
    AssetBrowser::Display();
    
    ImGui::End();
    
    for (uint32_t i = 0; i < static_cast<uint32_t>(myDocuments.size()); ++i)
    {
        myDocuments[i]->Update(1.f / 60.f);
    }
    
    ImGui::End();
    
    ImGui::PopStyleVar(3);
    
    if (Wonder::IdCountersNeedSaving())
    {
        Wonder::SaveIdCounters();
    }
}

void WonderfulEditor::Destroy() const
{
    Engine::GetInstance().GetLogicalDevice().waitIdle();

    for (auto& document : myDocuments)
    {
        document->Close();
    }
    
    AssetBrowser::Destroy();
}

void WonderfulEditor::OpenDocument(const Filepath& aFilepath)
{
    const Filepath extension = aFilepath.extension();
    
    if (localExtensionToDocumentConstructor.contains(extension) == false)
    {
        LOG_WARNING("Cannot open file: %s\nExtension %s not supported",aFilepath.string().c_str(),extension.string().c_str());
    }
    
    const std::string fileName = aFilepath.filename().generic_string();

    const Tga::StringId fileID = Tga::StringRegistry::RegisterOrGetString(fileName);
    
    for (const auto& document : myDocuments)
    {
        if (document->GetImGuiName() == fileID)
        {
            ImGui::SetWindowFocus(fileID.GetString());
            return;
        }
    }
    
    std::shared_ptr<Wonder::DocumentBase> document = localExtensionToDocumentConstructor.at(extension)();
    
    document->Init(aFilepath);
    document->GenerateDocks();
    
    myDocuments.push_back(document);
    
    ImGui::SetWindowFocus(fileID.GetString());
}

WonderfulEditor& WonderfulEditor::GetInstance()
{
    return *localInstance;
}

void WonderfulEditor::GenerateDocks()
{
    myMainDockingSpaceID                 = ImGui::GetID("MainDockSpace");
    Wonder::globalDocumentDockingSpaceID = ImGui::GetID("DocumentDockSpace");

    ImGui::DockSpace(myMainDockingSpaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode, &myMainWindowClass);

    ImGuiID center = myMainDockingSpaceID, top = 0, bottom = 0;

    if (!myDockIsInitialized)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::DockBuilderRemoveNode(myMainDockingSpaceID); // clear any previous layout
        ImGui::DockBuilderAddNode(myMainDockingSpaceID, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(myMainDockingSpaceID, viewport->WorkSize);

        ImGui::DockBuilderSplitNode(center, ImGuiDir_Up, 0.1f, &top, &center);
        ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.25f, &bottom, &center);

        ImGui::DockBuilderDockWindow(globalDockNames[static_cast<int>(MainWindowNames::DocumentDock)], center);
        ImGui::DockBuilderDockWindow(globalDockNames[static_cast<int>(MainWindowNames::AssetBrowser)], bottom);

        ImGui::DockBuilderFinish(myMainDockingSpaceID);

        myDockIsInitialized = true;
    }
}

void WonderfulEditor::DisplayTopBar() const
{
    if (ImGui::BeginMenuBar())
{
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Import"))
		{
		    FileDialog::OpenFile([this](const char* aPath)
		    {
		        LOG_INFO("Importing file: %s", aPath);
		        ImportFile(aPath);
		    });
		}

		ImGui::EndMenu();
	}

	
	
	
	ImGui::EndMenuBar();
	
}
}

void WonderfulEditor::MakeFileExtensionFunctions() const
{
    localExtensionToDocumentConstructor[Wonder::TEXTURE_FILE_EXTENSION] = &std::make_shared<Wonder::TextureDocument>;
    localExtensionToDocumentConstructor[Wonder::MODEL_FILE_EXTENSION] = &std::make_shared<Wonder::ModelDocument>;
    localExtensionToDocumentConstructor[Wonder::PREFAB_FILE_EXTENSION] = &std::make_shared<Wonder::PrefabDocument>;
    localExtensionToDocumentConstructor[Wonder::MATERIAL_FILE_EXTENSION] = &std::make_shared<Wonder::MaterialDocument>;
    
    localExtensionToDefaultData[Wonder::TEXTURE_FILE_EXTENSION] = [](const char* aFilename)
    {
        return nlohmann::json{
            {Wonder::ID_JSON_KEY,Wonder::GenerateTextureId().id},{Wonder::NAME_JSON_KEY,aFilename}
        };
    };
    localExtensionToDefaultData[Wonder::MODEL_FILE_EXTENSION] = [](const char* aFilename)
    {
        return nlohmann::json{
            {Wonder::ID_JSON_KEY,Wonder::GenerateTextureId().id},{Wonder::NAME_JSON_KEY,aFilename}
        };
    };
    localExtensionToDefaultData[Wonder::PREFAB_FILE_EXTENSION] = [](const char* aFilename)
    {
        return nlohmann::json{
            {Wonder::ID_JSON_KEY,Wonder::GeneratePrefabId().id},{Wonder::NAME_JSON_KEY,aFilename}
        };
    };
    localExtensionToDefaultData[Wonder::MATERIAL_FILE_EXTENSION] = [](const char* aFilename)
    {
        return nlohmann::json{
            {Wonder::ID_JSON_KEY,Wonder::GenerateMaterialId().id},{Wonder::NAME_JSON_KEY,aFilename},
            {"myVertexShaderName","shader.vert"},
            {"myFragmentShaderName","shader.frag"},
            {"myRenderStage",Vulkan::RenderStageType::Forward},
            {"myMaterialType",Wonder::MaterialType::Mesh}
        };
    };
    
    localExtensionToRegisterFunction[Wonder::TEXTURE_FILE_EXTENSION] = [](const Filepath& aFilepath)
    {
        Wonder::GetTextureRegistry().RegisterAsset(aFilepath);
    };
    localExtensionToRegisterFunction[Wonder::MODEL_FILE_EXTENSION] = [](const Filepath& aFilepath)
    {
        Wonder::GetModelRegistry().RegisterAsset(aFilepath);
    };
    localExtensionToRegisterFunction[Wonder::PREFAB_FILE_EXTENSION] = [](const Filepath& aFilepath)
    {
        Wonder::GetPrefabRegistry().RegisterAsset(aFilepath);
    };
    localExtensionToRegisterFunction[Wonder::MATERIAL_FILE_EXTENSION] = [](const Filepath& aFilepath)
    {
        Wonder::GetMaterialRegistry().RegisterAsset(aFilepath);
    };
}

void WonderfulEditor::CreateAsset(const Filepath& aFilepath,const char* aExtension) const
{
    Filepath savePath = aFilepath;
    savePath.replace_extension(aExtension);
    
    std::string filename = aFilepath.string();

    const nlohmann::json jsonMetaData = localExtensionToDefaultData[aExtension](filename.c_str());
    FileFunctions::SaveFileFromJson(savePath, jsonMetaData);
    
    localExtensionToRegisterFunction[aExtension](savePath);
    
    AssetBrowser::GeneratePathCache();
}

void WonderfulEditor::ImportFile(const Filepath& aFilepath) const
{
    Filepath extension = aFilepath.extension();
    
    if (extension == ".png")
    {
        Filepath intermediateAssetFolder = Wonder::GetIntermediateAssetPath() / "Textures";
        
        std::filesystem::copy(aFilepath,intermediateAssetFolder / aFilepath.filename());
        CreateAsset(AssetBrowser::GetCurrentDirectory() / aFilepath.filename(),Wonder::TEXTURE_FILE_EXTENSION);
        return;
    }
    if (extension == ".obj")
    {
        Filepath intermediateAssetFolder = Wonder::GetIntermediateAssetPath() / "Models";
        
        std::filesystem::copy(aFilepath,intermediateAssetFolder / aFilepath.filename());
        CreateAsset(AssetBrowser::GetCurrentDirectory() / aFilepath.filename(),Wonder::MODEL_FILE_EXTENSION);
        return;
    }
    
    LOG_WARNING("Failed to import file: %s\nExtension %s not supported", aFilepath.string().c_str(),extension.string().c_str());
}
