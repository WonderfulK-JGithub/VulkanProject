#pragma once
#include "DocumentBase.h"

namespace Wonder
{
    class TextureDocument final : public DocumentBase
    {
        enum class PanelNames
        {
            Image,
            Count,
        };
        
    public:
        void Init(const Filepath& aFilePath) override;
        void Update(float aDeltaTime) override;
        void Close() override;
        void Save() override;

    protected:
        void GenerateSubDocks() override;

        ImGuiWindowFlags myWindowFlags{};
    
    };
}


