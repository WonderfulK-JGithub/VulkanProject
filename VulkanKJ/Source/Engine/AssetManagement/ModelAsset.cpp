#include "ModelAsset.h"

#include "AssetRegistry.h"
#include "Loaders/ModelLoader.h"

void Wonder::ModelAsset::Init(const nlohmann::json& aJson)
{
    myId.id = aJson[ID_JSON_KEY];
    
    Load();
}

void Wonder::ModelAsset::SaveToJson(nlohmann::json& aJson)
{
    aJson[ID_JSON_KEY] = myId.id;
    aJson[NAME_JSON_KEY] = GetModelRegistry().GetNameFromID(myId);
}

void Wonder::ModelAsset::InitEngine(Wonder::ModelId aId)
{
    myId = aId;
    myIsEngineAsset = true;
}

void Wonder::ModelAsset::Load()
{
    Filepath modelFilepath = myIsEngineAsset ? Wonder::GetEngineAssetPath() : GetIntermediateAssetPath();
    modelFilepath /= "Models";
    modelFilepath /= GetModelRegistry().GetNameFromID(myId);
    
    if (exists(modelFilepath.replace_extension(".obj")))
    {
        ModelLoader::LoadFromObj(modelFilepath,myMeshes);
    }

    for (auto& mesh : myMeshes)
    {
        Vulkan::MeshPool::GetInstance().AddMesh(mesh);
    }
}

void Wonder::ModelAsset::LoadCustom(std::vector<Vulkan::PosColorVertex>& aVertices, std::vector<uint32_t>& aIndices)
{
    auto& meshData = myMeshes.emplace_back();
    meshData.vertices = aVertices;
    meshData.indices = aIndices;
    meshData.id = ModelLoader::GenerateMeshId();

    for (auto& mesh : myMeshes)
    {
        Vulkan::MeshPool::GetInstance().AddMesh(mesh);
    }
}

void Wonder::ModelAsset::Unload()
{
    for (auto& mesh : myMeshes)
    {
        Vulkan::MeshPool::GetInstance().RemoveMesh(mesh.id);
    }
    
    myMeshes.clear();
}

bool Wonder::ModelAsset::IsLoaded() const
{
    return myIsLoaded;
}

const std::vector<Vulkan::MeshData>& Wonder::ModelAsset::GetMeshes() const
{
    return myMeshes;
}
