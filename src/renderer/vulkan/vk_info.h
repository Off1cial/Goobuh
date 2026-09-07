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

#endif
