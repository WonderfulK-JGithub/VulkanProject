#pragma once
#include <vector>
#include <string>
#include <vulkan/vulkan.hpp>

#include "Util/StringRegistry.h"
#include "Util/Container/MappedVector.hpp"
#include "Util/FileManagement/FileTypes.h"

namespace Vulkan
{
	struct Shader
	{
		vk::ShaderModule module;
		vk::PipelineShaderStageCreateInfo shaderInfo;
		vk::ShaderStageFlags type;
	};

	Goose::MappedVector<Tga::StringId,Filepath>& GetAllVertexNamesAndPaths();
	Goose::MappedVector<Tga::StringId,Filepath>& GetAllFragmentNamesAndPaths();

	vk::PipelineShaderStageCreateInfo GetShaderInfo(const std::string& aFilename, vk::Device aDevice, vk::ShaderStageFlagBits aStage);

	void DestroyAllModules(vk::Device aDevice);
	
	void DisplayDebugTap();
	void ReloadModule(const Filepath& aFilename,vk::Device aDevice,vk::ShaderStageFlagBits aStage);
	
	namespace Internal
	{
		std::vector<char> ReadFile(const Filepath& aFilepath);

		vk::ShaderModule CreateShaderModule(const Filepath& aFilepath, vk::Device aDevice);
		
		bool TryCompileShader(const Filepath& aFilename);
		Filepath GetSpvFilename(const Filepath& aFilename);
		
		void LoadAllShaderNamesAndPaths();
	}
}
