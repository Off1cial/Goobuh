#ifndef VK_INFO_H
#define VK_INFO_H

#include "volk/volk.h"

static inline VkCommandBufferBeginInfo begininfo_cmdbuffer(VkCommandBufferUsageFlags flags)
{
  return (VkCommandBufferBeginInfo){
      .flags = flags,
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = NULL,
      .pInheritanceInfo = NULL};
}

static inline VkCommandBufferSubmitInfo submitinfo_cmdbuffer(VkCommandBuffer cmd)
{
  return (VkCommandBufferSubmitInfo){
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
      .pNext = NULL,
      .commandBuffer = cmd,
      .deviceMask = 0};
}

static inline VkSubmitInfo2 submit_info(VkCommandBufferSubmitInfo *cmd, VkSemaphoreSubmitInfo *signalSemaphoreInfo,
                                        VkSemaphoreSubmitInfo *waitSemaphoreInfo)
{
  return (VkSubmitInfo2){
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
      .pNext = NULL,

      .waitSemaphoreInfoCount = waitSemaphoreInfo == NULL ? 0 : 1,
      .pWaitSemaphoreInfos = waitSemaphoreInfo,

      .signalSemaphoreInfoCount = signalSemaphoreInfo == NULL ? 0 : 1,
      .pSignalSemaphoreInfos = signalSemaphoreInfo,

      .commandBufferInfoCount = 1,
      .pCommandBufferInfos = cmd};
}

static inline VkSemaphoreSubmitInfo submit_info_semaphore(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
{
  return (VkSemaphoreSubmitInfo){
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
      .pNext = NULL,
      .semaphore = semaphore,
      .stageMask = stageMask,
      .deviceIndex = 0,
      .value = 1};
}


static inline VkPipelineShaderStageCreateInfo createinfo_pipeline_shader_stage(VkShaderModule module, VkShaderStageFlagBits stage){
  return (VkPipelineShaderStageCreateInfo){
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .pName = "main",
    .stage = stage,
    .module = module
  };
}

static inline VkImageCreateInfo createinfo_image(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
{
     return (VkImageCreateInfo){
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .pNext = NULL,

    .imageType = VK_IMAGE_TYPE_2D,

    .format = format,
    .extent = extent,

    .mipLevels = 1,
    .arrayLayers = 1,

    //for MSAA. we will not be using it by default, so default it to 1 sample per pixel.
    .samples = VK_SAMPLE_COUNT_1_BIT,

    //optimal tiling, which means the image is stored on the best gpu format
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .usage = usageFlags};
}

static inline VkImageViewCreateInfo createinfo_imageview(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
{
    // build a image-view for the depth image to use for rendering
    return (VkImageViewCreateInfo){
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext = NULL,

    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .image = image,
    .format = format,
    .subresourceRange.baseMipLevel = 0,
    .subresourceRange.levelCount = 1,
    .subresourceRange.baseArrayLayer = 0,
    .subresourceRange.layerCount = 1,
    .subresourceRange.aspectMask = aspectFlags};
}


#endif
