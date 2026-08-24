#include "core/common.h"
#include "core/logsys.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "renderer/vulkan/vk_types.hpp"
#include "renderer/vulkan/vk_vma.h"
#include "renderer/vulkan/vk_renderer.hpp"

#include <span>

VkResult RESULT;

using namespace VK;

Renderer::Renderer(Plat::Window &window)
{
  Init(window);
}

Renderer::~Renderer()
{
  Shutdown();
}

void Renderer::Shutdown()
{
  if (m_device != VK_NULL_HANDLE)
  {
    // Make sure the GPU is finished before destroying resources.
    vkDeviceWaitIdle(m_device);

    for (int i = 0; i < FRAME_OVERLAP; i++)
    {
      vkDestroyCommandPool(m_device, m_frames[i].commandpool, nullptr);

      vkDestroyFence(m_device, m_frames[i].render_fence, nullptr);
      // vkDestroySemaphore(m_device, m_frames[i].render_semaphore, nullptr);
      vkDestroySemaphore(m_device, m_frames[i].swapchain_semaphore, nullptr);
    }
    for (VkSemaphore sem : m_render_finished_semaphores)
    {
      vkDestroySemaphore(m_device, sem, nullptr);
    }
    m_render_finished_semaphores.clear();
    m_deletions.flush();

    // Destroy pipeline
    vkDestroyShaderModule(m_device, m_shader->GetModule_Fragment(), nullptr);
    vkDestroyShaderModule(m_device, m_shader->GetModule_Vertex(), nullptr);

    // Destroy swapchain image views.
    for (VkImageView image_view : m_swapchain_imageviews)
    {
      if (image_view != VK_NULL_HANDLE)
      {
        vkDestroyImageView(
            m_device,
            image_view,
            nullptr);
      }
    }

    m_swapchain_imageviews.clear();

    // Destroy swapchain.
    if (m_swapchain != VK_NULL_HANDLE)
    {
      vkDestroySwapchainKHR(
          m_device,
          m_swapchain,
          nullptr);

      m_swapchain = VK_NULL_HANDLE;
    }

    if (m_allocator != VK_NULL_HANDLE)
    {
      vmaDestroyAllocator(m_allocator);
    }

    // Now the device has no remaining child objects.
    vkDestroyDevice(m_device, nullptr);
    m_device = VK_NULL_HANDLE;
  }

  // Surface belongs to the instance, so destroy it
  // after the device.
  if (m_surface != VK_NULL_HANDLE)
  {
    vkDestroySurfaceKHR(
        m_instance,
        m_surface,
        nullptr);

    m_surface = VK_NULL_HANDLE;
  }

  if (m_instance != VK_NULL_HANDLE)
  {
    vkDestroyInstance(
        m_instance,
        nullptr);

    m_instance = VK_NULL_HANDLE;
  }
}

VkPresentModeKHR Renderer::DeterminePresentMode(
    const std::vector<VkPresentModeKHR> &available,
    PresentMode preferred)
{
  VkPresentModeKHR preferred_vk;

  switch (preferred)
  {
  case PresentMode::Immediate:
    preferred_vk = VK_PRESENT_MODE_IMMEDIATE_KHR;
    break;

  case PresentMode::Mailbox:
    preferred_vk = VK_PRESENT_MODE_MAILBOX_KHR;
    break;

  case PresentMode::VSyncFifo:
    preferred_vk = VK_PRESENT_MODE_FIFO_KHR;
    break;
  }

  for (VkPresentModeKHR mode : available)
  {
    if (mode == preferred_vk)
      return mode;
  }

  // Fallback order for a low-latency renderer.
  for (VkPresentModeKHR mode : available)
  {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
      return VK_PRESENT_MODE_MAILBOX_KHR;
  }

  // FIFO is guaranteed.
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkPresentModeKHR Renderer::GetVkPresentMode(
    PresentMode mode) const
{
  switch (mode)
  {
  case PresentMode::Immediate:
    return VK_PRESENT_MODE_IMMEDIATE_KHR;

  case PresentMode::Mailbox:
    return VK_PRESENT_MODE_MAILBOX_KHR;

  case PresentMode::VSyncFifo:
    return VK_PRESENT_MODE_FIFO_KHR;
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

// NOT AS PERFORMANT AS THE PREVIOUS TRANSITION WHEN WE HAVE MORE PASSES
void Renderer::TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
{
  VkImageMemoryBarrier2 imageBarrier = {};
  imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  imageBarrier.pNext = nullptr;

  imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
  imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

  imageBarrier.oldLayout = currentLayout;
  imageBarrier.newLayout = newLayout;

  VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
  imageBarrier.subresourceRange = CreateImageSubresourceRange(aspectMask);
  imageBarrier.image = image;

  VkDependencyInfo depInfo{};
  depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  depInfo.pNext = nullptr;

  depInfo.imageMemoryBarrierCount = 1;
  depInfo.pImageMemoryBarriers = &imageBarrier;

  vkCmdPipelineBarrier2(cmd, &depInfo);
}

void Renderer::CopyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D src_size, VkExtent2D dest_size)
{
  VkImageBlit2 blitRegion{.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr};

  blitRegion.srcOffsets[1].x = (int32_t)src_size.width;
  blitRegion.srcOffsets[1].y = (int32_t)src_size.height;
  blitRegion.srcOffsets[1].z = 1;

  blitRegion.dstOffsets[1].x = (int32_t)dest_size.width;
  blitRegion.dstOffsets[1].y = (int32_t)dest_size.height;
  blitRegion.dstOffsets[1].z = 1;

  blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  blitRegion.srcSubresource.baseArrayLayer = 0;
  blitRegion.srcSubresource.layerCount = 1;
  blitRegion.srcSubresource.mipLevel = 0;

  blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  blitRegion.dstSubresource.baseArrayLayer = 0;
  blitRegion.dstSubresource.layerCount = 1;
  blitRegion.dstSubresource.mipLevel = 0;

  VkBlitImageInfo2 blitInfo{.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr};
  blitInfo.dstImage = destination;
  blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  blitInfo.srcImage = source;
  blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  blitInfo.filter = VK_FILTER_LINEAR;
  blitInfo.regionCount = 1;
  blitInfo.pRegions = &blitRegion;

  vkCmdBlitImage2(cmd, &blitInfo);
}

void Renderer::DrawBackground(VkCommandBuffer cmd)
{
  VkClearColorValue clear_value;
  clear_value = {{0.0f, 0.2f, 0.7f, 1.0f}};
  VkImageSubresourceRange clear_range = CreateImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
  vkCmdClearColorImage(cmd, m_draw_image.image, VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &clear_range);
}

uint32_t Renderer::FindMemoryType(
    uint32_t type_filter,
    VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties memory_properties{};

  vkGetPhysicalDeviceMemoryProperties(m_physdevice,
                                      &memory_properties);

  for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
  {
    if ((type_filter & (1 << i)) &&
        (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
    {
      return i;
    }
  }

  LOG_FATAL("Failed to find suitable Vulkan memory type");
  return UINT32_MAX;
}

void Renderer::DestroyBuffer(const AllocatedBuffer &buff)
{
  vmaDestroyBuffer(m_allocator, buff.buffer, buff.allocation);
}

MeshBuffers Renderer::MeshUpload(std::span<uint32_t> indices, std::span<Vertex> vertices)
{
  const size_t vertbuff_size = vertices.size() * sizeof(Vertex);
  const size_t indbuff_size = indices.size() * sizeof(uint32_t);

  MeshBuffers meshbuffs{};
  VkBufferUsageFlags vertusage =
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
      VK_BUFFER_USAGE_TRANSFER_DST_BIT |
      VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
  VkBufferUsageFlags indusage =
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
      VK_BUFFER_USAGE_TRANSFER_DST_BIT;

  meshbuffs.vertex_buffer = CreateBuffer(VMA_MEMORY_USAGE_GPU_ONLY, vertusage, vertbuff_size);
  meshbuffs.index_buffer = CreateBuffer(VMA_MEMORY_USAGE_GPU_ONLY, indusage, indbuff_size);

  // Locate vertex buffer
  VkBufferDeviceAddressInfo deviceAdressInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
      .pNext = nullptr,
      .buffer = meshbuffs.vertex_buffer.buffer,
  };
  meshbuffs.vertex_address = vkGetBufferDeviceAddress(m_device, &deviceAdressInfo);

  AllocatedBuffer staging = CreateBuffer(
      VMA_MEMORY_USAGE_CPU_ONLY,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      vertbuff_size + indbuff_size);

  VmaAllocationInfo allocationInfo{};
  vmaGetAllocationInfo(m_allocator, staging.allocation, &allocationInfo);
  void *data = allocationInfo.pMappedData;

  memcpy(data, vertices.data(), vertbuff_size);
  memcpy((char *)data + vertbuff_size, indices.data(), indbuff_size);

  ImmediateSubmit([&](VkCommandBuffer cmd)
                  {
    VkBufferCopy vertex_copy{};
    vertex_copy.dstOffset = 0;
    vertex_copy.srcOffset = 0;
    vertex_copy.size = vertbuff_size;

    vkCmdCopyBuffer(cmd, staging.buffer, meshbuffs.vertex_buffer.buffer, 1, &vertex_copy);

    VkBufferCopy index_copy{};
    index_copy.dstOffset = 0;
    index_copy.srcOffset = vertbuff_size;
    index_copy.size = indbuff_size;

    vkCmdCopyBuffer(cmd, staging.buffer, meshbuffs.index_buffer.buffer, 1, &index_copy); });

  DestroyBuffer(staging);
  return meshbuffs;
}

void Renderer::Draw()
{
  const int wait_timeout_ns = 1e9;
  u32 fence_count = 1;
  // Wait for the GPU to finish rendering the last frame with a 1s timeout
  VK_CHECK(
      vkWaitForFences(
          m_device,
          fence_count,
          &GetCurrentFrame().render_fence,
          VK_TRUE,
          wait_timeout_ns));
  VK_CHECK(
      vkResetFences(
          m_device, fence_count, &GetCurrentFrame().render_fence));

  // Request image to draw to from the swapchain
  u32 swapchain_image;
  VK_CHECK(
      vkAcquireNextImageKHR(
          m_device,
          m_swapchain,
          wait_timeout_ns,
          GetCurrentFrame().swapchain_semaphore,
          nullptr,
          &swapchain_image));

  VkCommandBuffer cmd = GetCurrentFrame().commandbuffer;

  VK_CHECK(vkResetCommandBuffer(cmd, 0));
  VkCommandBufferBeginInfo cmd_begininfo = CreateInfo_CommandBufferBegin(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  // Start recording
  VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begininfo));
  // Draw image: UNDEFINED -> GENERAL
  TransitionImage(
      cmd,
      m_draw_image.image,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_GENERAL);

  DrawBackground(cmd);

  // Draw image: GENERAL -> TRANSFER_SRC
  TransitionImage(
      cmd,
      m_draw_image.image,
      VK_IMAGE_LAYOUT_GENERAL,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

  // Swapchain: UNDEFINED -> TRANSFER_DST
  TransitionImage(
      cmd,
      m_swapchain_images[swapchain_image],
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // Copy draw image -> swapchain
  CopyImageToImage(
      cmd,
      m_draw_image.image,
      m_swapchain_images[swapchain_image],
      m_draw_extent,
      m_swapchain_extent);

  // Swapchain: TRANSFER_DST -> PRESENT
  TransitionImage(
      cmd,
      m_swapchain_images[swapchain_image],
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  VK_CHECK(vkEndCommandBuffer(cmd));

  VkCommandBufferSubmitInfo cmdinfo = CreateInfo_CommandBufferSubmit(cmd);

  VkSemaphoreSubmitInfo wait_info =
      CreateInfo_SemaphoreSubmit(
          VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
          GetCurrentFrame().swapchain_semaphore);
  VkSemaphoreSubmitInfo signal_info =
      CreateInfo_SemaphoreSubmit(
          VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
          m_render_finished_semaphores[swapchain_image]
          // GetCurrentFrame().render_semaphore
      );

  VkSubmitInfo2 submit = SubmitInfo(&cmdinfo, &signal_info, &wait_info);

  // Submit command buffer to queue
  VK_CHECK(vkQueueSubmit2(m_graphqueue, 1, &submit, GetCurrentFrame().render_fence));

  // Present
  VkPresentInfoKHR present_info{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.pNext = nullptr;
  present_info.pSwapchains = &m_swapchain;
  present_info.swapchainCount = 1;

  // present_info.pWaitSemaphores = &GetCurrentFrame().render_semaphore;
  present_info.pWaitSemaphores = &m_render_finished_semaphores[swapchain_image];

  present_info.waitSemaphoreCount = 1;

  present_info.pImageIndices = &swapchain_image;

  VK_CHECK(vkQueuePresentKHR(m_graphqueue, &present_info));
  m_framenumber++;
}

void Renderer::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)> &&function)
{
  printf("Submit\n");
  VK_CHECK(vkResetFences(m_device, 1, &m_imm_fence));
  VK_CHECK(vkResetCommandBuffer(m_imm_commandbuffer, 0));
  printf("Post Submit\n");

  VkCommandBuffer cmd = m_imm_commandbuffer;

  VkCommandBufferBeginInfo cmdBeginInfo = CreateInfo_CommandBufferBegin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

  function(cmd);

  VK_CHECK(vkEndCommandBuffer(cmd));

  VkCommandBufferSubmitInfo cmdinfo = CreateInfo_CommandBufferSubmit(cmd);
  VkSubmitInfo2 submit = SubmitInfo(&cmdinfo, nullptr, nullptr);

  // submit command buffer to the queue and execute it.
  //  _renderFence will now block until the graphic commands finish execution
  VK_CHECK(vkQueueSubmit2(m_graphqueue, 1, &submit, m_imm_fence));

  VK_CHECK(vkWaitForFences(m_device, 1, &m_imm_fence, true, 9999999999));
}
