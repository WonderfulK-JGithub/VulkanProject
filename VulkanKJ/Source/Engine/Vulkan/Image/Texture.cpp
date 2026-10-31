#include "Texture.h"
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Engine.h"
#include "stb_image.h"
#include "../VkMemory.h"
#include "Util/FileManagement/FileTypes.h"
#include "Util/Logging/Logging.h"

void Vulkan::Texture::Init(const TextureInput& aInput)
{
	myLogicalDevice = aInput.logicalDevice;
	myPhysicalDevice = aInput.physicalDevice;
	myFilepaths = aInput.filepaths;
	myCommandBuffer = aInput.commandBuffer;
	myQueue = aInput.queue;

	
	Init();
}

void Vulkan::Texture::Init(const Filepath& aFilepath)
{
	Engine& engine = Engine::GetInstance();
	
	myLogicalDevice = engine.GetLogicalDevice();
	myPhysicalDevice = engine.GetPhysicalDevice();
	myFilepaths = {aFilepath};
	myCommandBuffer = engine.GetMainCommandBuffer();
	myQueue = engine.GetGraphicsQueue();
	
	Init();
}

void Vulkan::Texture::InitCubemap(const std::vector<Filepath>& aFilepaths)
{
	Engine& engine = Engine::GetInstance();
	
	myLogicalDevice = engine.GetLogicalDevice();
	myPhysicalDevice = engine.GetPhysicalDevice();
	myFilepaths = aFilepaths;
	myCommandBuffer = engine.GetMainCommandBuffer();
	myQueue = engine.GetGraphicsQueue();
	
	Init();
}

void Vulkan::Texture::Destroy() const
{
	myLogicalDevice.freeMemory(myImageMemory);

	myLogicalDevice.destroyImageView(myImageView);
	myLogicalDevice.destroyImage(myImage);
}

vk::ImageView Vulkan::Texture::GetImageView() const
{
	return myImageView;
}

void Vulkan::Texture::Init()
{
	Load();

	myIsCubemap = myPixels.size() > 1;

	ImageInput imageInput;
	imageInput.logicalDevice    = myLogicalDevice;
	imageInput.physicalDevice   = myPhysicalDevice;
	imageInput.width            = myWidth;
	imageInput.height           = myHeight;
	imageInput.format           = vk::Format::eR8G8B8A8Unorm;
	imageInput.arrayCount       = static_cast<uint32_t>(myPixels.size());
	imageInput.tiling           = vk::ImageTiling::eOptimal;
	imageInput.usage            = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	imageInput.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	imageInput.flags            = vk::ImageCreateFlags();

	if (myIsCubemap)
	{
		imageInput.flags |= vk::ImageCreateFlagBits::eCubeCompatible;
	}

	myImage = MakeImage(imageInput);
	myImageMemory = MakeImageMemory(imageInput, myImage);

	Populate();

	for (auto pixels : myPixels)
	{
		free(pixels);
	}

	MakeView();
}

void Vulkan::Texture::Load()
{
	for (uint32_t i = 0; i < myFilepaths.size(); i++)
	{
		Filepath path = myFilepaths[i];
		
		unsigned char* pixels = stbi_load(path.string().c_str(), &myWidth, &myHeight, &myChannels, STBI_rgb_alpha);
		if (pixels == nullptr)
		{
			LOG_ERROR("stbi Failed to load image %s", myFilepaths[i].string().c_str());
		}
		else
		{
			myPixels.push_back(pixels);
		}
	}

	
}

void Vulkan::Texture::Populate() const
{
	Vulkan::BufferCreationInput bufferInput;
	bufferInput.logicalDevice = myLogicalDevice;
	bufferInput.physicalDevice = myPhysicalDevice;
	bufferInput.memortyProperties = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;
	bufferInput.usage = vk::BufferUsageFlagBits::eTransferSrc;
	
	size_t imageSize = myWidth * myHeight * 4;
	bufferInput.size = imageSize * myPixels.size();

	Vulkan::Buffer intermediateBuffer = Vulkan::CreateBuffer(bufferInput);

	for (size_t i = 0; i < myPixels.size(); i++)
	{
		void* writeLocation = myLogicalDevice.mapMemory(intermediateBuffer.bufferMemory, i * imageSize, imageSize);
		memcpy(writeLocation, myPixels[i], imageSize);
		myLogicalDevice.unmapMemory(intermediateBuffer.bufferMemory);
	}

	

	ImageLayoutTransitionInput transitionInput;
	transitionInput.commandBuffer = myCommandBuffer;
	transitionInput.queue = myQueue;
	transitionInput.image = myImage;
	transitionInput.oldLayout = vk::ImageLayout::eUndefined;
	transitionInput.newLayout = vk::ImageLayout::eTransferDstOptimal;
	transitionInput.arrayCount = static_cast<uint32_t>(myPixels.size());
	TransitionImageLayoutSingleTime(transitionInput);

	BufferImageCopyInput copyInput;
	copyInput.commandBuffer = myCommandBuffer;
	copyInput.queue = myQueue;
	copyInput.srcBuffer = intermediateBuffer.buffer;
	copyInput.dstImage = myImage;
	copyInput.width = myWidth;
	copyInput.height = myHeight;
	copyInput.arrayCount = static_cast<uint32_t>(myPixels.size());
	CopyBufferToImage(copyInput);

	transitionInput.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	transitionInput.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	TransitionImageLayoutSingleTime(transitionInput);

	myLogicalDevice.freeMemory(intermediateBuffer.bufferMemory);
	myLogicalDevice.destroyBuffer(intermediateBuffer.buffer);
}

void Vulkan::Texture::MakeView()
{
	vk::ImageViewType viewType = myIsCubemap ? vk::ImageViewType::eCube : vk::ImageViewType::e2D;
	myImageView = MakeImageView(myLogicalDevice, myImage, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, viewType, static_cast<uint32_t>(myPixels.size()));
}