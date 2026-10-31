#include "TextureAsset.h"

#include "AssetRegistry.h"
#include "Engine.h"
#include "imgui_impl_vulkan.h"
#include "Vulkan/Image/Sampler.h"
#include "Loaders/TextureLoader.h"

void Wonder::TextureAsset::Init(const nlohmann::json& aJson)
{
    myId = TextureId(aJson[ID_JSON_KEY]);
    
    TextureLoader::AddToLoadQueue(myId);
}

void Wonder::TextureAsset::SaveToJson(nlohmann::json& aJson)
{
    aJson[ID_JSON_KEY] = {myId.id};
    aJson[NAME_JSON_KEY] = GetTextureRegistry().GetNameFromID(myId);
}

void Wonder::TextureAsset::InitEngine(const TextureId aId)
{
    myId = aId;
    myIsEngineAsset = true;
    
    TextureLoader::AddToLoadQueue(myId);
}

void Wonder::TextureAsset::Load()
{
    if (myIsLoaded)
    {
        Unload();
    }
    
    Filepath texturePath = myIsEngineAsset ? Wonder::GetEngineAssetPath() : GetIntermediateAssetPath();
    texturePath /= "Textures";
    texturePath /= GetTextureRegistry().GetNameFromID(myId);
    
    if (exists(texturePath) && is_directory(texturePath))
    {
        std::vector<Filepath> filepaths;
        for (auto& item : std::filesystem::recursive_directory_iterator(texturePath))
        {
            filepaths.push_back(item.path());
        }
        
        myTexture.InitCubemap(filepaths);
    }
    else
    {
        texturePath.replace_extension(".png");
    
        myTexture.Init(texturePath);
    }
    
    Vulkan::TextureDescriptor::GetInstance().AddTexture(myId,myTexture.GetImageView());
    myIsLoaded = true;
    
#ifndef _RETAIL
    myImGuiDescriptorSet = ImGui_ImplVulkan_AddTexture(
        Vulkan::GetSampler(Vulkan::SamplerType::BilinearRepeat),
        myTexture.GetImageView(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
#endif
}

void Wonder::TextureAsset::Unload()
{
    if (myIsLoaded == false)
    {
        return;
    }
    
    myTexture.Destroy();
    
    Vulkan::TextureDescriptor::GetInstance().RemoveTexture(myId);
    
    myIsLoaded = false;
#ifndef _RETAIL
    ImGui_ImplVulkan_RemoveTexture(myImGuiDescriptorSet);
#endif
}

bool Wonder::TextureAsset::IsLoaded() const
{
    return myIsLoaded;
}

VkDescriptorSet& Wonder::TextureAsset::GetImGuiDescriptorSet()
{
    return myImGuiDescriptorSet;
}
