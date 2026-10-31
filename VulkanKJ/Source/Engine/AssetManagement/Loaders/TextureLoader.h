#pragma once
#include "ID/IdTypes.h"

namespace Wonder::TextureLoader
{
    void AddToLoadQueue(TextureId aId);
    
    bool TexturesNeedsUpdate();
    void UpdateTextures();
}
