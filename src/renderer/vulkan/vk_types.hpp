#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "renderer/vulkan/vk_vma.h"
#include <functional>
#include <queue>
#include "math/vector.hpp"

namespace VK
{
  struct FrameData
  {
    VkCommandPool commandpool = VK_NULL_HANDLE;
    VkCommandBuffer commandbuffer = VK_NULL_HANDLE;
    VkSemaphore swapchain_semaphore = VK_NULL_HANDLE; // Render commands wait on swapchain request
    //VkSemaphore render_semaphore = VK_NULL_HANDLE; // Present once drawing has finished
    VkFence render_fence = VK_NULL_HANDLE;
  };

  struct AllocatedBuffer
  {
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
  };

  struct MeshBuffers
  {
    AllocatedBuffer vertex_buffer;
    AllocatedBuffer index_buffer;
    VkDeviceAddress vertex_address;
  };

  struct MeshPushConstants
  {
    float model[16];
    VkDeviceAddress vertex_address;
  };

  struct Vertex
  {

    float pos[3];
    float normal[3];
    float col[4];
    float uv[2];
  };

  enum class ShaderType
  {
    Vertex,
    Fragment,
    Geometry
  };

  enum class MSAASampleCount
  {
    MSAA1  = 0x01,
    MSAA2  = 0x02,
    MSAA4  = 0x04,
    MSAA8  = 0x08,
    MSAA16 = 0x10,
    MSAA32 = 0x20,
    MSAA64 = 0x40
  };

  enum class PresentMode
  {
    Immediate,
    Mailbox,
    VSyncFifo
  };


  struct AllocatedImage
  {
    VkImage image; 
    VkImageView image_view;
    VmaAllocation allocation;
    VkExtent3D image_extent;
    VkFormat image_format;
  };

  
  struct DeletionQueue
  {
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()>&& function)
    {
      deletors.push_back(function);
    }

    void flush()
    {
      for (auto func = deletors.rbegin(); func != deletors.rend(); func++)  
      {
        (*func)();
      }
      deletors.clear();
    }
  };
};



  VkFenceCreateInfo CreateInfo_Fence(VkFenceCreateFlags flags);
  VkSemaphoreCreateInfo CreateInfo_Semaphore(VkSemaphoreCreateFlags flags);
  VkSemaphoreCreateInfo CreateInfo_Semaphore(void);

  VkCommandBufferBeginInfo CreateInfo_CommandBufferBegin(VkCommandBufferUsageFlags flags);

  VkImageSubresourceRange CreateImageSubresourceRange(VkImageAspectFlags aspectmask);  
  
  VkSemaphoreSubmitInfo CreateInfo_SemaphoreSubmit(VkPipelineStageFlags2 stagemask, VkSemaphore semaphore);

  VkCommandBufferSubmitInfo CreateInfo_CommandBufferSubmit(VkCommandBuffer cmd);
  VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* semaphore_signal_info, VkSemaphoreSubmitInfo* semaphore_wait_info);

  VkImageCreateInfo CreateInfo_Image(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent);
  VkImageViewCreateInfo CreateInfo_ImageView(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags);
