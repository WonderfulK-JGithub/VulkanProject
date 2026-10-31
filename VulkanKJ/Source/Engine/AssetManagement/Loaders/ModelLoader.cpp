#include "ModelLoader.h"

#include <fstream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Util/StringUtils.h"
#include "Vulkan/VkMesh.h"

namespace 
{
    Vulkan::MeshId localMeshIdCounter;
}

void Wonder::ModelLoader::LoadFromObj(const Filepath& aFilepath, std::vector<Vulkan::MeshData>& aMeshes)
{
    std::vector<glm::vec3> v;
    std::vector<glm::vec3> vn;
    std::vector<glm::vec2> vt;

    std::unordered_map<std::string, uint32_t> history;
    
    Vulkan::MeshData& mesh = aMeshes.emplace_back();
    mesh.id = ++localMeshIdCounter;

    auto ReadVertexData = [&](const std::vector<std::string>& aWords)
    {
        glm::vec3 newPosition = glm::vec3(std::stof(aWords[1]), std::stof(aWords[2]), std::stof(aWords[3]));
        v.push_back(newPosition);
    };

    auto ReadTexcoordData = [&](const std::vector<std::string>& aWords)
    {
        glm::vec2 newUV = glm::vec2(std::stof(aWords[1]), std::stof(aWords[2]));
        vt.push_back(newUV);
    };

    auto ReadNormalData = [&](const std::vector<std::string>& aWords)
    {
        glm::vec3 newNormal = glm::vec3(std::stof(aWords[1]), std::stof(aWords[2]), std::stof(aWords[3]));
        vn.push_back(newNormal);
    };

    auto ReadCorner = [&](const std::string& aVertexDescription)
    {
        if (history.contains(aVertexDescription))
        {
            mesh.indices.push_back(history[aVertexDescription]);
            return;
        }

        uint32_t index = static_cast<uint32_t>(history.size());
        history.insert({ aVertexDescription,index });
        mesh.indices.push_back(index);

        Vulkan::PosColorVertex newVertex;

        std::vector<std::string> v_vt_vn = Goose::StringUtils::SplitString(aVertexDescription, "/");
        glm::vec3 position               = v[std::stol(v_vt_vn[0]) - 1];
        newVertex.position               = position;

        newVertex.color = glm::vec3(1.f);

        glm::vec2 texcoord{};
        if (v_vt_vn.size() == 3 && v_vt_vn[1].empty() == false)
        {
            texcoord = vt[std::stol(v_vt_vn[1]) - 1];
        }

        newVertex.uv = texcoord;

        glm::vec3 normal = vn[std::stol(v_vt_vn[2]) - 1];
        newVertex.normal = normal;

        mesh.vertices.push_back(newVertex);

    };

    auto ReadFaceData = [&](const std::vector<std::string>& aWords)
    {
        size_t triangleCount = aWords.size() - 3;

        for (size_t i = 0; i < triangleCount; i++)
        {
            ReadCorner(aWords[1]);
            ReadCorner(aWords[2 + i]);
            ReadCorner(aWords[3 + i]);
        }
    };

    


    std::ifstream file;
    file.open(aFilepath);

    std::string line;
    while (std::getline(file, line))
    {
        std::vector<std::string> words = Goose::StringUtils::SplitString(line, " ");

        if (!words[0].compare("v"))
        {
            ReadVertexData(words);
        }

        if (!words[0].compare("vn"))
        {
            ReadNormalData(words);
        }

        if (!words[0].compare("vt"))
        {
            ReadTexcoordData(words);
        }

        if (!words[0].compare("f"))
        {
            ReadFaceData(words);
        }
    }

    

    file.close();
}

Vulkan::MeshId Wonder::ModelLoader::GenerateMeshId()
{
    return ++localMeshIdCounter;
}
