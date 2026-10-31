
#include "AssetRegistry.h"

#include "PrefabAsset.h"
#include "TextureAsset.h"

namespace Wonder
{
    static AssetRegistry<TextureAsset, TextureId> localTextureRegistry;
	static AssetRegistry<ModelAsset, ModelId> localModelRegistry;
	static AssetRegistry<MaterialAsset, MaterialId> localMaterialRegistry;
	static AssetRegistry<PrefabAsset, PrefabId> localPrefabRegistry;
}

void Wonder::RegisterDefaultTextures()
{
    {
        TextureAsset test1;
        test1.InitEngine(TEST_TEXTURE_1);
        GetTextureRegistry().RegisterAssetCopy(test1,TEST_TEXTURE_1,"xd");
    }
    
    {
        TextureAsset test2;
        test2.InitEngine(TEST_TEXTURE_2);
        GetTextureRegistry().RegisterAssetCopy(test2,TEST_TEXTURE_2,"rck_3");
    }
    
    {
        TextureAsset test3;
        test3.InitEngine(TEST_TEXTURE_3);
        GetTextureRegistry().RegisterAssetCopy(test3,TEST_TEXTURE_3,"bowling2bg");
    }
    
    {
        TextureAsset testCubemap;
        testCubemap.InitEngine(TEST_CUBEMAP);
        GetTextureRegistry().RegisterAssetCopy(testCubemap,TEST_CUBEMAP,"sky3");
    }
}

void Wonder::RegisterDefaultModels()
{
	{
		ModelAsset triangleModel;
		triangleModel.InitEngine(TRIANGLE_MODEL);
		
		constexpr glm::vec3 color = glm::vec3{ 1.f,1.f,1.f };
		constexpr glm::vec3 normal = glm::vec3{ 0.f,0.f,1.f };
		
		std::vector<Vulkan::PosColorVertex> vertices{
			Vulkan::PosColorVertex{glm::vec3{0.00f,-0.5f,0.f},color,glm::vec2{0.5f,1.f},normal},
			Vulkan::PosColorVertex{glm::vec3{0.5f,0.5f,0.f},color,glm::vec2{1.f,0.f},normal},
			Vulkan::PosColorVertex{glm::vec3{-0.5f,0.5f,0.f},color,glm::vec2{0.f,0.f},normal}
		};

		std::vector<uint32_t> indices
		{
			0,1,2
		};
		
		GetModelRegistry().RegisterAssetMove(std::move(triangleModel),TRIANGLE_MODEL,"triangle");
		GetModelRegistry().AccessAssetFromID(TRIANGLE_MODEL).LoadCustom(vertices,indices);
	}
	
	{
		ModelAsset squareModel;
		squareModel.InitEngine(SQUARE_MODEL);
		
		constexpr glm::vec3 color = glm::vec3{ 1.f,1.f,1.f };
		constexpr glm::vec3 normal = glm::vec3{ 0.f,0.f,1.f };
		
		std::vector<Vulkan::PosColorVertex> squareVertices{
			Vulkan::PosColorVertex{glm::vec3{0.5f,-0.5f,0.f},color,glm::vec2{1.f,1.f},normal},
			Vulkan::PosColorVertex{glm::vec3{0.5f,0.5f,0.f},color,glm::vec2{1.f,0.f},normal},
			Vulkan::PosColorVertex{glm::vec3{-0.5f,0.5f,0.f},color,glm::vec2{0.f,0.f},normal},
			Vulkan::PosColorVertex{glm::vec3{-0.5f,-0.5f,0.f},color,glm::vec2{0.f,1.f},normal},
		
		};
		std::vector<uint32_t> squareIndices
		{
			0,1,2,
			0,2,3
		};
		
		
		
		GetModelRegistry().RegisterAssetMove(std::move(squareModel),SQUARE_MODEL,"square");
		GetModelRegistry().AccessAssetFromID(SQUARE_MODEL).LoadCustom(squareVertices,squareIndices);
	}
	
	{
		ModelAsset testObjModel;
		testObjModel.InitEngine(TEST_OBJ_MODEL);
		GetModelRegistry().RegisterAssetMove(std::move(testObjModel),TEST_OBJ_MODEL,"vulkanModel");
		
		GetModelRegistry().AccessAssetFromID(TEST_OBJ_MODEL).Load();
	}
	
	{
		ModelAsset defaultSphere;
		defaultSphere.InitEngine(DEFAULT_SPHERE_MODEL);
		GetModelRegistry().RegisterAssetMove(std::move(defaultSphere),DEFAULT_SPHERE_MODEL,"DefaultSphere");
		
		GetModelRegistry().AccessAssetFromID(DEFAULT_SPHERE_MODEL).Load();
	}
}

void Wonder::RegisterDefaultMaterials()
{
	{
		Wonder::MaterialAsset triangleMaterial;
		triangleMaterial.InitEngine(TEST_MATERIAL_1);
		triangleMaterial.AccessVertexShaderName() = "shader.vert"_tgaid;
		triangleMaterial.AccessFragmentShaderName() = "shader.frag"_tgaid;
		triangleMaterial.SetRenderStage(Vulkan::RenderStageType::Forward);
		triangleMaterial.SetMaterialType(MaterialType::Mesh);
		triangleMaterial.AddTexture(Wonder::TEST_TEXTURE_1);
	
		triangleMaterial.Build();
		
		localMaterialRegistry.RegisterAssetCopy(triangleMaterial,TEST_MATERIAL_1,"Triangle_Material");
	}
	
	{
		Wonder::MaterialAsset squareMaterial;
		squareMaterial.InitEngine(TEST_MATERIAL_2);
		squareMaterial.AccessVertexShaderName() = "shader.vert"_tgaid;
		squareMaterial.AccessFragmentShaderName() = "shader.frag"_tgaid;
		squareMaterial.SetRenderStage(Vulkan::RenderStageType::Forward);
		squareMaterial.SetMaterialType(MaterialType::Mesh);
		squareMaterial.AddTexture(Wonder::TEST_TEXTURE_2);
	
		squareMaterial.Build();
		
		localMaterialRegistry.RegisterAssetCopy(squareMaterial,TEST_MATERIAL_2,"Square_Material");
	}
	
	{
		Wonder::MaterialAsset objMaterial;
		objMaterial.InitEngine(TEST_MATERIAL_3);
		objMaterial.AccessVertexShaderName() = "shader.vert"_tgaid;
		objMaterial.AccessFragmentShaderName() = "shader.frag"_tgaid;
		objMaterial.SetRenderStage(Vulkan::RenderStageType::Forward);
		objMaterial.SetMaterialType(MaterialType::Mesh);
		objMaterial.AddTexture(Wonder::TEST_TEXTURE_3);
	
		objMaterial.Build();
		
		localMaterialRegistry.RegisterAssetCopy(objMaterial,TEST_MATERIAL_3,"Default_Obj_Material");
	}
	
	{
		Wonder::MaterialAsset cubemapMaterial;
		cubemapMaterial.InitEngine(TEST_CUBEMAP_MATERIAL);
		cubemapMaterial.AccessVertexShaderName() = "skyShader.vert"_tgaid;
		cubemapMaterial.AccessFragmentShaderName() = "skyShader.frag"_tgaid;
		cubemapMaterial.SetRenderStage(Vulkan::RenderStageType::Cubemap);
		cubemapMaterial.SetMaterialType(MaterialType::Skybox);
		cubemapMaterial.AddTexture(Wonder::TEST_CUBEMAP);
	
		cubemapMaterial.Build();
		
		localMaterialRegistry.RegisterAssetCopy(cubemapMaterial,TEST_CUBEMAP_MATERIAL,"Default_Cubemap_Material");
	}
	
	{
		Wonder::MaterialAsset fullscreenCopyMaterial;
		fullscreenCopyMaterial.InitEngine(FULLSCREEN_COPY_SWAPCHAIN);
		fullscreenCopyMaterial.AccessVertexShaderName() = "fullscreenShader.vert"_tgaid;
		fullscreenCopyMaterial.AccessFragmentShaderName() = "fullscreenShader.frag"_tgaid;
		fullscreenCopyMaterial.SetRenderStage(Vulkan::RenderStageType::FullscreenSwapchain);
		fullscreenCopyMaterial.SetMaterialType(MaterialType::Fullscreen);
	
		fullscreenCopyMaterial.Build();
		
		localMaterialRegistry.RegisterAssetCopy(fullscreenCopyMaterial,FULLSCREEN_COPY_SWAPCHAIN,"Fullscreen_Copy_Swapchain_Material");
	}
	
	{
		Wonder::MaterialAsset fullscreenCopyMaterial;
		fullscreenCopyMaterial.InitEngine(FULLSCREEN_COPY_OFFSCREEN);
		fullscreenCopyMaterial.AccessVertexShaderName() = "fullscreenShader.vert"_tgaid;
		fullscreenCopyMaterial.AccessFragmentShaderName() = "fullscreenShader.frag"_tgaid;
		fullscreenCopyMaterial.SetRenderStage(Vulkan::RenderStageType::FullscreenOffscreen);
		fullscreenCopyMaterial.SetMaterialType(MaterialType::Fullscreen);
	
		fullscreenCopyMaterial.Build();
		
		localMaterialRegistry.RegisterAssetCopy(fullscreenCopyMaterial,FULLSCREEN_COPY_OFFSCREEN,"Fullscreen_Copy_Offscreen_Material");
	}
	
	{
		Wonder::MaterialAsset lineMaterial;
		lineMaterial.InitEngine(LINE_MATERIAL);
		lineMaterial.AccessVertexShaderName() = "lineShader.vert"_tgaid;
		lineMaterial.AccessFragmentShaderName() = "lineShader.frag"_tgaid;
		lineMaterial.SetRenderStage(Vulkan::RenderStageType::Forward);
		lineMaterial.SetMaterialType(MaterialType::Line);
	
		lineMaterial.Build();
		
		localMaterialRegistry.RegisterAssetCopy(lineMaterial,LINE_MATERIAL,"Line_Material");
	}
}

void Wonder::SearchAndRegisterAllAssets()
{
	LOG_INFO("%s","Started registering assets...");
	
	const Filepath& dataPath = Wonder::GetDataPath();

	std::vector<Filepath> allPaths;

	for (const auto& entry : std::filesystem::recursive_directory_iterator(dataPath))
	{
		if (entry.is_regular_file())
		{
			allPaths.push_back(entry.path());
		}
	}

	for (auto& path : allPaths)
	{
		const std::string extension = path.extension().string();

		if (extension == TEXTURE_FILE_EXTENSION)
		{
			localTextureRegistry.RegisterAsset(path);
		}
		else if (extension == MODEL_FILE_EXTENSION)
		{
			localModelRegistry.RegisterAsset(path);
		}
		else if (extension == PREFAB_FILE_EXTENSION)
		{
			localPrefabRegistry.RegisterAsset(path);
		}
		else if (extension == MATERIAL_FILE_EXTENSION)
		{
			localMaterialRegistry.RegisterAsset(path);
		}
	}
	
	LOG_INFO("%s","Registering assets finished!");
}
//
// void Goose::ResolveAllIdConflicts()
// {
// 	const FilePath& dataPath = Tga::Settings::GameAssetRoot();
//
// 	for (const auto& entry : fs::recursive_directory_iterator(dataPath))
// 	{
// 		if (entry.is_regular_file())
// 		{
// 			const std::string extension = entry.path().extension().string();
// 			const std::string name = entry.path().filename().string();
// 			if (extension == PREFAB_FILE_EXTENSION)
// 			{
// 				if (localPrefabRegistry.HasRegisteredAsset(name.c_str()) == false)
// 				{
// 					nlohmann::json newJson;
// 					FileFunctions::LoadFileToJson(entry.path(), newJson);
// 					PrefabInstanceID newId = CreatePrefabInstanceID();
// 					LOG_INFO("New id %i given to %s", newId, name.c_str());
// 					newJson["ID"] = newId;
// 					FileFunctions::SaveFileFromJson(entry.path(), newJson);
// 					localPrefabRegistry.RegisterAsset(entry.path());
// 				}
// 				
// 			}
// 			else if (extension == MATERIAL_FILE_EXTENSION)
// 			{
// 				if (localMaterialRegistry.HasRegisteredAsset(name.c_str()) == false)
// 				{
// 					nlohmann::json newJson;
// 					FileFunctions::LoadFileToJson(entry.path(), newJson);
// 					MaterialInstanceID newId = CreateMaterialInstanceID();
// 					LOG_INFO("New id %i given to %s", newId, name.c_str());
// 					newJson["ID"] = newId;
// 					FileFunctions::SaveFileFromJson(entry.path(), newJson);
// 					localMaterialRegistry.RegisterAsset(entry.path());
// 				}
// 			}
// 			else if (extension == MODEL_FILE_EXTENSION)
// 			{
// 				if (localModelRegistry.HasRegisteredAsset(name.c_str()) == false)
// 				{
// 					nlohmann::json newJson;
// 					FileFunctions::LoadFileToJson(entry.path(), newJson);
// 					ModelID newId = CreateModelID();
// 					LOG_INFO("New id %i given to %s", newId, name.c_str());
// 					newJson["ID"] = newId;
// 					FileFunctions::SaveFileFromJson(entry.path(), newJson);
// 					localModelRegistry.RegisterAsset(entry.path());
// 				}
// 			}
// 			else if (extension == TEXTURE_FILE_EXTENSION)
// 			{
// 				if (localTextureRegistry.HasRegisteredAsset(name.c_str()) == false)
// 				{
// 					nlohmann::json newJson;
// 					FileFunctions::LoadFileToJson(entry.path(), newJson);
// 					TextureID newId = CreateTextureID();
// 					LOG_INFO("New id %i given to %s", newId, name.c_str());
// 					newJson["ID"] = newId;
// 					FileFunctions::SaveFileFromJson(entry.path(), newJson);
// 					localTextureRegistry.RegisterAsset(entry.path());
// 				}
// 			}
// 			else if (extension == ANIMATION_FILE_EXTENSION)
// 			{
// 				if (localAnimationRegistry.HasRegisteredAsset(name.c_str()) == false)
// 				{
// 					nlohmann::json newJson;
// 					FileFunctions::LoadFileToJson(entry.path(), newJson);
// 					AnimationID newId = CreateAnimationID();
// 					LOG_INFO("New id %i given to %s", newId, name.c_str());
// 					newJson["ID"] = newId;
// 					FileFunctions::SaveFileFromJson(entry.path(), newJson);
// 					localAnimationRegistry.RegisterAsset(entry.path());
// 				}
// 			}
// 			else if (extension == LIGHT_FILE_EXTENSION)
// 			{
// 				if (localLightSettingsRegistry.HasRegisteredAsset(name.c_str()) == false)
// 				{
// 					nlohmann::json newJson;
// 					FileFunctions::LoadFileToJson(entry.path(), newJson);
// 					LightSettingsID newId = CreateLightSettingsID();
// 					LOG_INFO("New id %i given to %s", newId, name.c_str());
// 					newJson["ID"] = newId;
// 					FileFunctions::SaveFileFromJson(entry.path(), newJson);
// 					localLightSettingsRegistry.RegisterAsset(entry.path());
// 				}
// 			}
// 			else if (extension == PARTICLE_FILE_EXTENSION)
// 			{
// 				if (localParticleSystemRegistry.HasRegisteredAsset(name.c_str()) == false)
// 				{
// 					nlohmann::json newJson;
// 					FileFunctions::LoadFileToJson(entry.path(), newJson);
// 					LightSettingsID newId = CreateParticleSystemID();
// 					LOG_INFO("New id %i given to %s", newId, name.c_str());
// 					newJson["ID"] = newId;
// 					FileFunctions::SaveFileFromJson(entry.path(), newJson);
// 					localLightSettingsRegistry.RegisterAsset(entry.path());
// 				}
// 			}
// 		}
// 	}
//
// 	UserInfo::Save();
// }
Wonder::AssetRegistry<Wonder::TextureAsset, Wonder::TextureId>& Wonder::GetTextureRegistry()
{
    return localTextureRegistry;
}

Wonder::AssetRegistry<Wonder::ModelAsset, Wonder::ModelId>& Wonder::GetModelRegistry()
{
	return localModelRegistry;
}

Wonder::AssetRegistry<Wonder::MaterialAsset, Wonder::MaterialId>& Wonder::GetMaterialRegistry()
{
	return localMaterialRegistry;
}

Wonder::AssetRegistry<Wonder::PrefabAsset, Wonder::PrefabId>& Wonder::GetPrefabRegistry()
{
	return localPrefabRegistry;
}
