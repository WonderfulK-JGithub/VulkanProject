#pragma once
#include <vulkan/vulkan.hpp>

#include "Util/FileManagement/FileTypes.h"

namespace Vulkan
{
	struct TextureInput
	{
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;
		std::vector<Filepath> filepaths;

		vk::CommandBuffer commandBuffer;
		vk::Queue queue;
	};

	class Texture
	{
	public:
		Texture() = default;
		~Texture() = default;
		
		void Init(const TextureInput& aInput);
		void Init(const Filepath& aFilepath);
		void InitCubemap(const std::vector<Filepath>& aFilepaths);
		
		void Destroy() const;
		
		vk::ImageView GetImageView() const;
	private:
		int myWidth{};
		int myHeight{};
		int myChannels{};
		vk::Device myLogicalDevice;
		vk::PhysicalDevice myPhysicalDevice;

		std::vector<Filepath> myFilepaths;
		std::vector<unsigned char*> myPixels;
		bool myIsCubemap{};

		vk::Image myImage;
		vk::DeviceMemory myImageMemory;
		vk::ImageView myImageView;

		vk::CommandBuffer myCommandBuffer;
		vk::Queue myQueue;

		void Init();
		void Load();
		void Populate() const;
		void MakeView();
	};
}
