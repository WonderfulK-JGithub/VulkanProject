#include "Vulkan/VkMesh.h"
#include "Util/FileManagement/FileTypes.h"

namespace Vulkan
{
    struct MeshData;
}

namespace Wonder::ModelLoader
{
    void LoadFromObj(const Filepath& aFilepath,std::vector<Vulkan::MeshData>& aMeshes);
    
    Vulkan::MeshId GenerateMeshId();
}
