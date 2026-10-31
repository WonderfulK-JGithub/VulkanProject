#include "VkScene.h"

VkScene::VkScene()
{
	for (float z = -1.0f; z < 1.0f; z += 0.2f) 
	{
		for (float y = -1.0f; y < 1.0f; y += 0.2f) 
		{
			myMeshPositions[MeshType::Triangle].push_back(glm::vec3(0.5f, y, z));
		}
	}
	
	for (float z = -1.0f; z < 1.0f; z += 0.2f)
	{
		for (float y = -1.0f; y < 1.0f; y += 0.2f)
		{
			myMeshPositions[MeshType::Square].push_back(glm::vec3(-0.5f, y, z));
		}
	}

	for (float z = -1.0f; z < 1.0f; z += 0.2f)
	{
		for (float y = -1.0f; y < 1.0f; y += 0.2f)
		{
			myMeshPositions[MeshType::Obj].push_back(glm::vec3(-0.f, y, z));
		}
	}

	
	//myMeshPositions[VkMesh::MeshType::Obj].push_back(glm::vec3(0.f,0.f,0.f));
}
