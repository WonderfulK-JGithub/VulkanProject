#include "MaterialDocument.h"

#include <Engine.h>
#include <imgui_internal.h>
#include <AssetManagement/AssetRegistry.h>
#include <AssetManagement/TextureAsset.h>
#include <AssetManagement/Loaders/MaterialBuildQueue.h>
#include <Command/GenericCommands/UpdateCommand.hpp>
#include <Shaders/Shaders.h>
#include <Util/ImGuiUtil.h>

#include "EditorGlobals.h"

static std::unordered_map<Wonder::MaterialType,Tga::StringId> localDefaultVertexShaders{
    {Wonder::MaterialType::Mesh,"shader.vert"_tgaid},
    {Wonder::MaterialType::Mesh,"shader.vert"_tgaid},
    {Wonder::MaterialType::Mesh,"shader.vert"_tgaid},
    {Wonder::MaterialType::Mesh,"shader.vert"_tgaid},
};
static std::unordered_map<Wonder::MaterialType,Tga::StringId> localDefaultFragmentShaders{{Wonder::MaterialType::Mesh,"shader.vert"_tgaid}};

void Wonder::MaterialDocument::Init(const Filepath& aFilePath)
{
    const std::string fileName = aFilePath.filename().replace_extension().string();
    
    myImGuiName = Tga::StringRegistry::RegisterOrGetString(fileName);
    
    mySavedMaterialId = GetMaterialRegistry().GetIDFromName(fileName.c_str());
    myReferenceMaterial.CopyFromOther(GetMaterialRegistry().AccessAssetFromID(mySavedMaterialId));
    myEditMaterialId = GetMaterialRegistry().RegisterAssetCopy(myReferenceMaterial);
    
    AccessEditMaterial().Build();
    
    myPanelNames.resize(static_cast<uint32_t>(PanelNames::Count));
    myPanelInfos.resize(myPanelNames.size());
    
    myPanelNames[static_cast<uint32_t>(PanelNames::Viewport)] = "Viewport##Document" + fileName;
    myPanelNames[static_cast<uint32_t>(PanelNames::Properties)] = "Properties##Document" + fileName;
    
    myWindowFlags = ImGuiWindowFlags_NoMove;
    
    SceneRenderer::CreateSceneObjectInfo(mySceneObjectInfo);
    mySceneObjectInfo.cubemapMaterial = TEST_CUBEMAP_MATERIAL;
    
    myCameraController.SetPosition(glm::vec3(0.0f, 1.0f, 5.0f));
}

void Wonder::MaterialDocument::Update([[maybe_unused]] float aDeltaTime)
{
    BaseUpdate();
    
    if (myIsVisible == false)
    {
        return;
    }
    
    myCameraController.Update();
    myLineGrid.MakeLines(glm::xyz(myCameraController.GetTransform()[3]),10);
    
    ImGui::Begin(myPanelNames[static_cast<uint32_t>(PanelNames::Properties)].c_str(),nullptr,myWindowFlags);
    DisplayProperties();
    ImGui::End();
    
    ImGui::Begin(myPanelNames[static_cast<uint32_t>(PanelNames::Viewport)].c_str(),nullptr,myWindowFlags);
    RenderMaterial();
    ImGui::End();
}

void Wonder::MaterialDocument::Close()
{
    Engine::GetInstance().WaitForIdle();
    AccessEditMaterial().Destroy();
    GetMaterialRegistry().RemoveRuntimeAsset(myEditMaterialId);
    
    SceneRenderer::DestroySceneObjectInfo(mySceneObjectInfo);
    myEditorViewport.Destroy();
}

void Wonder::MaterialDocument::Save()
{
    MaterialAsset& saveMaterial = GetMaterialRegistry().AccessAssetFromID(mySavedMaterialId);
    saveMaterial.CopyFromOther(AccessEditMaterial());
    
    nlohmann::json saveJson;
    saveMaterial.SaveToJson(saveJson);
    FileFunctions::SaveFileFromJson(GetDataPath() / GetMaterialRegistry().GetAssetFilePath(mySavedMaterialId),saveJson);
    
    Engine::GetInstance().WaitForIdle();
    MaterialBuildQueue::AddMaterialToQueue(mySavedMaterialId);
    
    myCommandManager.SetSaveCommandCount();
}

void Wonder::MaterialDocument::CommandUpdateMaterial(const MaterialAsset& aMaterial)
{
    myReferenceMaterial.CopyFromOther(aMaterial);
    AccessEditMaterial().CopyFromOther(aMaterial);
    Engine::GetInstance().WaitForIdle();
    MaterialBuildQueue::AddMaterialToQueue(myEditMaterialId);
}

void Wonder::MaterialDocument::GenerateSubDocks()
{
    const ImGuiID imGuiID     = ImGui::GetID(myImGuiName.GetString());
    const ImVec2 docSpaceSize = ImGui::GetContentRegionAvail();
    ImGui::DockSpace(imGuiID, docSpaceSize, ImGuiDockNodeFlags_None, &globalDocumentLevelWindowClass);

    const ImGuiID dockSpaceId = imGuiID;

    ImGui::DockBuilderRemoveNode(dockSpaceId);
    ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockSpaceId, docSpaceSize);

    ImGuiID center = dockSpaceId,right = 0;
    
    ImGui::DockBuilderSplitNode(center,ImGuiDir_Right,0.3f, &right,&center);

    ImGui::DockBuilderDockWindow(myPanelNames[static_cast<uint32_t>(PanelNames::Viewport)].c_str(), center);
    ImGui::DockBuilderDockWindow(myPanelNames[static_cast<uint32_t>(PanelNames::Properties)].c_str(), right);

    ImGui::DockBuilderFinish(dockSpaceId);
}

void Wonder::MaterialDocument::DisplayProperties()
{
    uint8_t info = 0;
    bool needsRebuild = false;
    
    ImGui::TextUnformatted("Type");
    uint32_t currentSelection = static_cast<uint32_t>(AccessEditMaterial().GetMaterialType());
    if (ImGuiUtil::DisplayCombo("##Type",globalMaterialTypeNames,std::size(globalMaterialTypeNames),currentSelection))
    {
        MaterialType newMaterialType = static_cast<MaterialType>(currentSelection);
        if (newMaterialType != AccessEditMaterial().GetMaterialType())
        {
            info |= InfoFlag_EditEnded;
            needsRebuild = true;
        }
    }
    
    {
        ImGui::TextUnformatted("Vertex Shader");
        ImGui::SameLine();
        
        auto& vertexShaderNames = Vulkan::GetAllVertexNamesAndPaths();
        static Goose::String32 vertexNameFilter;
        if (ImGuiUtil::DisplayComboFiltered("##VertexShader",vertexShaderNames.dataKeys(),static_cast<uint32_t>(vertexShaderNames.size()),
            AccessEditMaterial().AccessVertexShaderName(),vertexNameFilter))
        {
            info |= InfoFlag_EditEnded;
            needsRebuild = true;
        }
    }
    
    {
        ImGui::TextUnformatted("Fragment Shader");
        ImGui::SameLine();
        
        auto& fragmentShaderNames = Vulkan::GetAllFragmentNamesAndPaths();
        static Goose::String32 fragmentNameFilter;
        if (ImGuiUtil::DisplayComboFiltered("##FragmentShader",fragmentShaderNames.dataKeys(),static_cast<uint32_t>(fragmentShaderNames.size()),
            AccessEditMaterial().AccessFragmentShaderName(),fragmentNameFilter))
        {
            info |= InfoFlag_EditEnded;
            needsRebuild = true;
        }
    }

    for (auto& pushConstant : AccessEditMaterial().AccessPushConstants())
    {
        uint8_t pushConstantEditInfo = 0;
        
        switch (pushConstant.info.type)
        {
            case ShaderReflection::PushConstantType::Padding:
            {
                uint32_t paddingAmount = *reinterpret_cast<uint32_t*>(pushConstant.info.data.data());
                ImGuiUtil::ThickSeparator(2.f);
                ImGui::Text("[PADDING: %d bytes]",static_cast<int>(paddingAmount));
                ImGuiUtil::ThickSeparator(2.f);
            }
                break;
            case ShaderReflection::PushConstantType::RenderImageIndex:
                break;
            case ShaderReflection::PushConstantType::TextureIndex:
            {
                TextureId textureId = *reinterpret_cast<TextureId*>(pushConstant.info.data.data());
                DISPLAY_WITH_COPY(pushConstantEditInfo,pushConstant.name.c_str(),textureId,ImGuiUtil::DisplayTextureReferenceEdit(textureId));
                
                if ((pushConstantEditInfo & InfoFlag_ChangedValue) != 0u &&
                    GetTextureRegistry().AccessAssetFromID(textureId).IsCubemap())
                {
                    LOG_WARNING("%s","You assigned a cubemap texture to a regular texture slot, epic fail!");
                }
                
                std::copy_n(reinterpret_cast<char*>(&textureId),sizeof(textureId),pushConstant.info.data.data());
            }
                break;
            case ShaderReflection::PushConstantType::CubemapIndex:
            {
                TextureId cubemapTextureId = *reinterpret_cast<TextureId*>(pushConstant.info.data.data());
                DISPLAY_WITH_COPY(pushConstantEditInfo,pushConstant.name.c_str(),cubemapTextureId,ImGuiUtil::DisplayTextureReferenceEdit(cubemapTextureId));
                
                if ((pushConstantEditInfo & InfoFlag_ChangedValue) != 0u &&
                    GetTextureRegistry().AccessAssetFromID(cubemapTextureId).IsCubemap() == false)
                {
                    LOG_WARNING("%s","You assigned a regular texture to a cubemap texture slot, epic fail!");
                }
                
                std::copy_n(reinterpret_cast<char*>(&cubemapTextureId),sizeof(cubemapTextureId),pushConstant.info.data.data());
            }
            break;
            case ShaderReflection::PushConstantType::Float:
            {
                float floatValue = *reinterpret_cast<float*>(pushConstant.info.data.data());
                DISPLAY_WITH_COPY(pushConstantEditInfo,pushConstant.name.c_str(),floatValue,ImGuiUtil::DisplayDragFloat("##floatValue",&floatValue));
                std::copy_n(reinterpret_cast<char*>(&floatValue),sizeof(floatValue),pushConstant.info.data.data());
            }
                break;
            case ShaderReflection::PushConstantType::Vector2:
            {
                glm::vec2 vector2Value = *reinterpret_cast<glm::vec2*>(pushConstant.info.data.data());
                DISPLAY_WITH_COPY(pushConstantEditInfo,pushConstant.name.c_str(),vector2Value,ImGuiUtil::DisplayDragFloat2("##vector2Value",&vector2Value.x));
                std::copy_n(reinterpret_cast<char*>(&vector2Value),sizeof(vector2Value),pushConstant.info.data.data());
            }
                break;
            case ShaderReflection::PushConstantType::Vector3:
            {
                glm::vec3 vector3Value = *reinterpret_cast<glm::vec3*>(pushConstant.info.data.data());
                DISPLAY_WITH_COPY(pushConstantEditInfo,pushConstant.name.c_str(),vector3Value,ImGuiUtil::DisplayDragFloat3("##vector3Value",&vector3Value.x));
                std::copy_n(reinterpret_cast<char*>(&vector3Value),sizeof(vector3Value),pushConstant.info.data.data());
            }
                break;
            case ShaderReflection::PushConstantType::Color:
            {
                glm::vec4 colorValue = *reinterpret_cast<glm::vec4*>(pushConstant.info.data.data());
                DISPLAY_WITH_COPY(pushConstantEditInfo,pushConstant.name.c_str(),colorValue,
                    ImGuiUtil::DisplayColor4("#colorValue",&colorValue.r,ImGuiColorEditFlags_NoInputs));
                std::copy_n(reinterpret_cast<char*>(&colorValue),sizeof(colorValue),pushConstant.info.data.data());
            }
                break;
        }
        
        info |= pushConstantEditInfo;
    }
    
    if (needsRebuild)
    {
        Engine::GetInstance().WaitForIdle();
        MaterialBuildQueue::AddMaterialToQueue(myEditMaterialId);
    }
    
    if ((info & InfoFlag_EditEnded) != 0u)
    {
        auto updateFunction = [](void* aMaterialDocument, IdType , const MaterialAsset& aMaterial)
        {
            static_cast<MaterialDocument*>(aMaterialDocument)->CommandUpdateMaterial(aMaterial);
        };

        const std::shared_ptr<UpdateCommand<MaterialAsset>> updateCommand = std::make_shared<
            UpdateCommand<MaterialAsset>>(this, 0, myReferenceMaterial, AccessEditMaterial(), updateFunction);
        myCommandManager.Add(updateCommand);
	
        myReferenceMaterial.CopyFromOther(AccessEditMaterial());
    }
}

void Wonder::MaterialDocument::RenderMaterial()
{
    myEditorViewport.ResizeCheck();
    myCameraController.SetProjection(myEditorViewport.GetViewportSize().x,myEditorViewport.GetViewportSize().y);
    
    myCameraController.SetSceneCameraInfo(mySceneObjectInfo);
    mySceneObjectInfo.linePoints = myLineGrid.GetLinePoints();
	
    mySceneObjectInfo.instances.ClearInstances();
    
    VkInit::ModelInstance modelInstance;
    modelInstance.transform = glm::mat4x4(1.f);
    mySceneObjectInfo.instances.AddInstance(myEditMaterialId,
        GetModelRegistry().AccessAssetFromID(DEFAULT_SPHERE_MODEL).GetMeshes()[0].id,modelInstance);
    
    myEditorViewport.RenderViewport(mySceneObjectInfo);
}

Wonder::MaterialAsset& Wonder::MaterialDocument::AccessEditMaterial() const
{
    return GetMaterialRegistry().AccessAssetFromID(myEditMaterialId);
}
