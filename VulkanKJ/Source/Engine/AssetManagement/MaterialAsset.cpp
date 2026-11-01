#include "MaterialAsset.h"

#include "AssetRegistry.h"
#include "Engine.h"
#include "ID/IdFunctions.h"
#include "Property/PropertySaveAndLoadFunctions.h"
#include "Shaders/Shaders.h"
#include "Util/FileManagement/Settings.h"
#include "Vulkan/VkInit/PipelineLayout.h"

void Wonder::MaterialAsset::Init(const nlohmann::json& aJson)
{
    myId.id = aJson[ID_JSON_KEY];
    
    myVertexShaderName = Tga::StringRegistry::RegisterOrGetString(aJson["myVertexShaderName"]);
    myFragmentShaderName = Tga::StringRegistry::RegisterOrGetString(aJson["myFragmentShaderName"]);

    if (aJson.contains("myPushConstants"))
    {
        for (auto& pushConstantJson : aJson["myPushConstants"])
        {
            PushConstantProperty pushConstant;
            pushConstant.name = pushConstantJson["name"];
            pushConstant.info.type = pushConstantJson["type"].get<ShaderReflection::PushConstantType>();

            switch (pushConstant.info.type)
            {
                case ShaderReflection::PushConstantType::TextureIndex:
                {
                    TextureId textureId{pushConstantJson["data"]};
                    std::copy_n(reinterpret_cast<char*>(&textureId),sizeof(textureId),pushConstant.info.data.data());
                }
                    break;
                case ShaderReflection::PushConstantType::RenderImageIndex:
                {
                    RenderImageId renderImageId{pushConstantJson["data"]};
                    std::copy_n(reinterpret_cast<char*>(&renderImageId),sizeof(renderImageId),pushConstant.info.data.data());
                }
                    break;
                case ShaderReflection::PushConstantType::Float:
                {
                    float value{pushConstantJson["data"]};
                    std::copy_n(reinterpret_cast<char*>(&value),sizeof(float),pushConstant.info.data.data());
                }
                break;
                case ShaderReflection::PushConstantType::Vector2:
                {
                    glm::vec2 value{};
                    LoadProperty(value,pushConstantJson,"data");
                    std::copy_n(reinterpret_cast<char*>(&value),sizeof(glm::vec2),pushConstant.info.data.data());
                }
                break;
                case ShaderReflection::PushConstantType::Vector3:
                {
                    glm::vec3 value{};
                    LoadProperty(value,pushConstantJson,"data");
                    std::copy_n(reinterpret_cast<char*>(&value),sizeof(glm::vec3),pushConstant.info.data.data());
                }
                break;
                case ShaderReflection::PushConstantType::Color:
                {
                    glm::vec4 value{};
                    LoadProperty(value,pushConstantJson,"data");
                    std::copy_n(reinterpret_cast<char*>(&value),sizeof(glm::vec4),pushConstant.info.data.data());
                }
                break;
            }
            
            myPushConstants.emplace_back(pushConstant);
        }
    }
    
    AdjustPushConstantsToReflection();
    
    myRenderStage = aJson["myRenderStage"].get<Vulkan::RenderStageType>();
    myMaterialType = aJson["myMaterialType"].get<MaterialType>();
    
    myGraphicsPipelineLayout = Vulkan::GetPipelineLayout(myRenderStage);
    
    Build();
}

void Wonder::MaterialAsset::SaveToJson(nlohmann::json& aJson)
{
    aJson[ID_JSON_KEY] = myId.id;
    aJson[NAME_JSON_KEY] = GetMaterialRegistry().GetNameFromID(myId);
    
    aJson["myVertexShaderName"] = myVertexShaderName.GetString();
    aJson["myFragmentShaderName"] = myFragmentShaderName.GetString();

    aJson["myPushConstants"] = nlohmann::json::array();
    
    if (myPushConstants.empty() == false)
    {
        aJson["myPushConstants"] = nlohmann::json::array();
        for (auto& pushConstant : myPushConstants)
        {
            nlohmann::json pushConstantJson;
            pushConstantJson["name"] = pushConstant.name.c_str();
            pushConstantJson["type"] = pushConstant.info.type;

            switch (pushConstant.info.type)
            {
                case ShaderReflection::PushConstantType::TextureIndex:
                {
                    pushConstantJson["data"] = 
                        reinterpret_cast<TextureId*>(pushConstant.info.data.data())->id;
                }
                break;
                case ShaderReflection::PushConstantType::RenderImageIndex:
                {
                    pushConstantJson["data"] = 
                        *reinterpret_cast<RenderImageId*>(pushConstant.info.data.data());
                }
                break;
                case ShaderReflection::PushConstantType::Float:
                {
                    pushConstantJson["data"] = 
                        *reinterpret_cast<float*>(pushConstant.info.data.data());
                }
                break;
                case ShaderReflection::PushConstantType::Vector2:
                {
                    SaveProperty(*reinterpret_cast<glm::vec2*>(pushConstant.info.data.data()),pushConstantJson,"data");
                }
                break;
                case ShaderReflection::PushConstantType::Vector3:
                {
                    SaveProperty(*reinterpret_cast<glm::vec3*>(pushConstant.info.data.data()),pushConstantJson,"data");
                }
                break;
                case ShaderReflection::PushConstantType::Color:
                {
                    SaveProperty(*reinterpret_cast<glm::vec4*>(pushConstant.info.data.data()),pushConstantJson,"data");
                }
                break;
            }
            
            aJson["myPushConstants"].emplace_back(pushConstantJson);
        }
    }
    
    aJson["myRenderStage"] = myRenderStage;
    aJson["myMaterialType"] = myMaterialType;
}

void Wonder::MaterialAsset::CopyFromOther(const MaterialAsset& aOther)
{
    myVertexShaderName = aOther.myVertexShaderName;
    myFragmentShaderName = aOther.myFragmentShaderName;
    myPushConstants = aOther.myPushConstants;
    myMaterialType = aOther.myMaterialType;
    SetRenderStage(aOther.myRenderStage);
}

void Wonder::MaterialAsset::AdjustPushConstantsToReflection()
{
    Goose::MappedVector<std::string,ShaderReflection::PushConstantInfo> actualPushConstants = 
        ShaderReflection::GetShaderPushConstantTypes(Vulkan::GetAllFragmentNamesAndPaths().at(myFragmentShaderName));

    for (auto& pushConstant : myPushConstants)
    {
        if (actualPushConstants.contains(pushConstant.name.c_str()))
        {
            ShaderReflection::PushConstantInfo& info = actualPushConstants.at(pushConstant.name.c_str());
            if (info.type == pushConstant.info.type)
            {
                info = pushConstant.info;
            }
        }
    }
    
    myPushConstants.clear();

    for (uint32_t i = 0; i < actualPushConstants.size(); ++i)
    {
        PushConstantProperty pushConstant;
        pushConstant.name = actualPushConstants.dataKeys()[i];
        pushConstant.info = actualPushConstants.data()[i];
        myPushConstants.emplace_back(pushConstant);
    }
}

void Wonder::MaterialAsset::InitEngine(const MaterialId aId)
{
    myId = aId;
}

void Wonder::MaterialAsset::AddTexture(const Wonder::TextureId aTextureId)
{
    PushConstantProperty pushConstant;
    pushConstant.info.type = ShaderReflection::PushConstantType::TextureIndex;
    std::copy_n(reinterpret_cast<const char*>(&aTextureId),sizeof(aTextureId),pushConstant.info.data.data());
    myPushConstants.emplace_back(pushConstant);
}

void Wonder::MaterialAsset::AddRenderImage(const RenderImageId aImageId)
{
    PushConstantProperty pushConstant;
    pushConstant.info.type = ShaderReflection::PushConstantType::RenderImageIndex;
    std::copy_n(reinterpret_cast<const char*>(&aImageId),sizeof(aImageId),pushConstant.info.data.data());
    myPushConstants.emplace_back(pushConstant);
}

void Wonder::MaterialAsset::RemoveRenderImages()
{
    for (uint32_t i = 0; i < myPushConstants.size(); ++i)
    {
        if (myPushConstants[i].info.type == ShaderReflection::PushConstantType::RenderImageIndex)
        {
            myPushConstants.erase(myPushConstants.begin() + i);
            i--;
        }
    }
}

Tga::StringId& Wonder::MaterialAsset::AccessVertexShaderName()
{
    return myVertexShaderName;
}

Tga::StringId& Wonder::MaterialAsset::AccessFragmentShaderName()
{
    return myFragmentShaderName;
}

std::vector<Wonder::PushConstantProperty>& Wonder::MaterialAsset::AccessPushConstants()
{
    return myPushConstants;
}

void Wonder::MaterialAsset::SetRenderStage(const Vulkan::RenderStageType aRenderStage)
{
    myRenderStage = aRenderStage;
    myGraphicsPipelineLayout = Vulkan::GetPipelineLayout(aRenderStage);
}

void Wonder::MaterialAsset::SetMaterialType(const MaterialType& aMaterialType)
{
    myMaterialType = aMaterialType;
}

void Wonder::MaterialAsset::Build()
{
    auto& allResolutions = Vulkan::GetAllResolutions();
    for (uint32_t i = 0; i < allResolutions.size(); ++i)
    {
        BuildOrRebuildGraphicsPipeline(allResolutions.dataKeys()[i],allResolutions.data()[i]);
    }
}

void Wonder::MaterialAsset::Bind(const vk::CommandBuffer aCommandBuffer,Vulkan::ResolutionId aResolutionId) const
{
    aCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,myGraphicsPipelines.at(aResolutionId));

    std::vector<char> uploadPushConstantData;

    for (auto& pushConstant : myPushConstants)
    {
        size_t currentSize = uploadPushConstantData.size();
        switch (pushConstant.info.type)
        {
            case ShaderReflection::PushConstantType::Padding:
            {
                uint32_t paddingAmount = *reinterpret_cast<const uint32_t*>(pushConstant.info.data.data());
                uploadPushConstantData.resize(currentSize + paddingAmount);
            }
            break;
            case ShaderReflection::PushConstantType::TextureIndex:
            {
                uint32_t textureIndex = Vulkan::TextureDescriptor::GetInstance().GetTextureIndex(*
                    reinterpret_cast<const TextureId*>(pushConstant.info.data.data()));
                uploadPushConstantData.resize(currentSize + sizeof(textureIndex));
                std::copy_n(reinterpret_cast<char*>(&textureIndex),sizeof(textureIndex),uploadPushConstantData.data() + currentSize);
            }
                break;
            case ShaderReflection::PushConstantType::RenderImageIndex:
            {
                uint32_t renderImageIndex = Vulkan::TextureDescriptor::GetInstance().GetRenderImageTextureIndex(*
                   reinterpret_cast<const RenderImageId*>(pushConstant.info.data.data()));
                uploadPushConstantData.resize(currentSize + sizeof(renderImageIndex));
                std::copy_n(reinterpret_cast<char*>(&renderImageIndex),sizeof(renderImageIndex),uploadPushConstantData.data() + currentSize);
            }
                break;
            case ShaderReflection::PushConstantType::Float:
            {
                uploadPushConstantData.resize(currentSize + sizeof(float));
                std::copy_n(pushConstant.info.data.data(),sizeof(float),uploadPushConstantData.data() + currentSize);
            }
            break;
            case ShaderReflection::PushConstantType::Vector2:
            {
                uploadPushConstantData.resize(currentSize + sizeof(glm::vec2));
                std::copy_n(pushConstant.info.data.data(),sizeof(glm::vec2),uploadPushConstantData.data() + currentSize);
            }
            break;
            case ShaderReflection::PushConstantType::Vector3:
            {
                uploadPushConstantData.resize(currentSize + sizeof(glm::vec3));
                std::copy_n(pushConstant.info.data.data(),sizeof(glm::vec3),uploadPushConstantData.data() + currentSize);
            }
            break;
            case ShaderReflection::PushConstantType::Color:
            {
                uploadPushConstantData.resize(currentSize + sizeof(glm::vec4));
                std::copy_n(pushConstant.info.data.data(),sizeof(glm::vec4),uploadPushConstantData.data() + currentSize);
            }
            break;
        }
    }
    
    if (uploadPushConstantData.empty() == false)
    {
        aCommandBuffer.pushConstants(myGraphicsPipelineLayout,vk::ShaderStageFlagBits::eFragment,0,
        static_cast<uint32_t>(uploadPushConstantData.size()),uploadPushConstantData.data());//
    }
}

void Wonder::MaterialAsset::BuildOrRebuildGraphicsPipeline(const Vulkan::ResolutionId aResolutionId, const vk::Extent2D aResolution)
{
    if (myGraphicsPipelines.contains(aResolutionId))
    {
        vk::Device logicalDevice = Engine::GetInstance().GetLogicalDevice();
        logicalDevice.destroyPipeline(myGraphicsPipelines.at(aResolutionId));
    }
    
    VkInit::GraphicsPipelineInBundle specification{};
    specification.device = Engine::GetInstance().GetLogicalDevice();
    specification.vertexShaderfilePath = myVertexShaderName.GetString();
    specification.fragmentShaderfilePath = myFragmentShaderName.GetString();

    specification.renderSize = aResolution;
    specification.colorFormat = Engine::GetInstance().GetSwapchainColorFormat();
    specification.depthFormat = Engine::GetInstance().GetSwapchainDepthFormat();
    specification.stage = myRenderStage;

    switch (myMaterialType)
    {
        case MaterialType::Skybox:
        case MaterialType::Fullscreen:
            specification.vertexType = Vulkan::VertexLayoutType::Empty;
            break;
        case MaterialType::Mesh:
            specification.vertexType = Vulkan::VertexLayoutType::PosColor;
            break;
        case MaterialType::Line:
            specification.vertexType = Vulkan::VertexLayoutType::Line;
            break;
    }
    
    specification.layout = myGraphicsPipelineLayout;
    
    myGraphicsPipelines[aResolutionId] = VkInit::MakeGraphicsPipeline(specification);
}

void Wonder::MaterialAsset::DestroyGraphicsPipeline(const Vulkan::ResolutionId aResolutionId)
{
    if (myGraphicsPipelines.contains(aResolutionId))
    {
        vk::Device logicalDevice = Engine::GetInstance().GetLogicalDevice();
        logicalDevice.destroyPipeline(myGraphicsPipelines.at(aResolutionId));
        myGraphicsPipelines.erase(aResolutionId);
    }
}

void Wonder::MaterialAsset::Destroy()
{
    vk::Device logicalDevice = Engine::GetInstance().GetLogicalDevice(); 

    for (auto& pipeline : myGraphicsPipelines | std::views::values)
    {
        logicalDevice.destroyPipeline(pipeline);
    }
    
    myPushConstants.clear();
}

vk::PipelineLayout Wonder::MaterialAsset::GetPipelineLayout() const
{
    return myGraphicsPipelineLayout;
}
