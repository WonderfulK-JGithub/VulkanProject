#include "TextureLoader.h"

#include <vector>

#include "AssetManagement/AssetRegistry.h"
#include "AssetManagement/TextureAsset.h"

namespace Wonder
{
    static std::vector<Wonder::TextureId> localTexturesToLoad;
    
    void TextureLoader::AddToLoadQueue(const TextureId aId)
    {
        localTexturesToLoad.push_back(aId);
    }

    bool TextureLoader::TexturesNeedsUpdate()
    {
        return localTexturesToLoad.empty() == false;
    }

    void TextureLoader::UpdateTextures()
    {
        auto& textureRegistry = GetTextureRegistry();

        for (auto& toLoad : localTexturesToLoad)
        {
            if (textureRegistry.AssetExists(toLoad))
            {
                textureRegistry.AccessAssetFromID(toLoad).Load();
            }
            else
            {
                LOG_ERROR("Tried to load nonexisting texture, id: %d",toLoad.id);
            }
        }
        
        localTexturesToLoad.clear();
    }
}
