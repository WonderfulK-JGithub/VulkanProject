#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include "VkMesh.h"

class VkScene
{
public:
	VkScene();

	/*std::vector<glm::vec3> myTrianglePositions;
	std::vector<glm::vec3> mySquarePositions;*/

	enum class MeshType
	{
		Triangle,
		Square,
		Obj
	};
	
	
	std::unordered_map<MeshType, std::vector<glm::vec3>> myMeshPositions;
};

