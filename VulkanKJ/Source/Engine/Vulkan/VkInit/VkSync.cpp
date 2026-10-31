#include "VkSync.h"
#include "Util/Logging/Logging.h"

vk::Semaphore VkInit::CreateSemaphore(vk::Device aDevice)
{
    vk::SemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.flags = vk::SemaphoreCreateFlags();
    
    try
    {
        return aDevice.createSemaphore(semaphoreInfo);
    }
    catch (vk::SystemError err)
    {
        LOG_ERROR("Failed to create Semaphore, %s", err.what());
        
    }

    return{};
}

vk::Fence VkInit::CreateFence(vk::Device aDevice)
{
    vk::FenceCreateInfo fenceInfo{};
    fenceInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;

    try
    {
        return aDevice.createFence(fenceInfo);
    }
    catch (vk::SystemError err)
    {
        LOG_ERROR("Failed to create Fence, %s", err.what());
    }

    return {};
}
