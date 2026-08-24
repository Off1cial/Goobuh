#include "renderer/vulkan/vk_types.hpp"
#include "renderer/vulkan/vk_renderer.hpp"
#include <vulkan/vulkan_core.h>

using namespace VK;

VkFenceCreateInfo CreateInfo_Fence(VkFenceCreateFlags flags)
{
  VkFenceCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags;
  return info;
}

VkSemaphoreCreateInfo CreateInfo_Semaphore(VkSemaphoreCreateFlags flags)
{
  VkSemaphoreCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags;
  return info;
}

VkSemaphoreCreateInfo CreateInfo_Semaphore(void)
{
  VkSemaphoreCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = 0;
  return info;
}


VkCommandBufferBeginInfo CreateInfo_CommandBufferBegin(VkCommandBufferUsageFlags flags)
{
  VkCommandBufferBeginInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  info.pNext = nullptr;

  info.pInheritanceInfo = nullptr;
  info.flags = flags;
  return info;
}


VkImageSubresourceRange CreateImageSubresourceRange(VkImageAspectFlags aspectmask)
{
  VkImageSubresourceRange subimage{};
  subimage.aspectMask = aspectmask;
  subimage.baseMipLevel = 0;
  subimage.levelCount = VK_REMAINING_MIP_LEVELS;
  subimage.baseArrayLayer = 0;
  subimage.layerCount = VK_REMAINING_ARRAY_LAYERS;

  return subimage;
}


VkSemaphoreSubmitInfo CreateInfo_SemaphoreSubmit(
    VkPipelineStageFlags2 stagemask,
    VkSemaphore semaphore)
{
  VkSemaphoreSubmitInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  info.stageMask = stagemask;
  info.semaphore = semaphore;
  info.deviceIndex = 0;
  info.value = 1;
  info.pNext = nullptr;
  
  return info;
}

VkCommandBufferSubmitInfo CreateInfo_CommandBufferSubmit(VkCommandBuffer cmd)
{
  VkCommandBufferSubmitInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  info.pNext = nullptr;
  info.commandBuffer = cmd;
  info.deviceMask = 0;

  return info;
}



VkImageCreateInfo CreateInfo_Image(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent)
{
  VkImageCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.pNext = nullptr;

  info.imageType = VK_IMAGE_TYPE_2D;
  info.format = format;
  info.extent = extent;

  info.mipLevels = 1;
  info.arrayLayers = 1;

  info.samples = VK_SAMPLE_COUNT_1_BIT;
  info.tiling = VK_IMAGE_TILING_OPTIMAL;
  info.usage = usage_flags;

  return info;
}


VkImageViewCreateInfo CreateInfo_ImageView(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags)
{
  VkImageViewCreateInfo info{};

  info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  info.pNext = nullptr;
  info.viewType =  VK_IMAGE_VIEW_TYPE_2D;
  info.image = image;
  info.format = format;

  info.subresourceRange.baseMipLevel = 0;
  info.subresourceRange.levelCount = 1;
  info.subresourceRange.baseArrayLayer = 0;
  info.subresourceRange.layerCount = 1;
  info.subresourceRange.aspectMask = aspect_flags;

  return info;
}



VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* semaphore_signal_info, VkSemaphoreSubmitInfo* semaphore_wait_info)
{
  VkSubmitInfo2 info{};
  info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  info.pNext = nullptr;

  info.waitSemaphoreInfoCount = semaphore_wait_info == nullptr ? 0 : 1;
  info.pWaitSemaphoreInfos = semaphore_wait_info;

  info.signalSemaphoreInfoCount = semaphore_signal_info == nullptr ? 0 : 1;
  info.pSignalSemaphoreInfos = semaphore_signal_info;

  info.commandBufferInfoCount = 1;
  info.pCommandBufferInfos = cmd;

  return info;
}


