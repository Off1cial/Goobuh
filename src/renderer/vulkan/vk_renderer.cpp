#include "core/common.h"
#include "core/logsys.hpp"
#include "vk_renderer.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "renderer/vulkan/vk_types.hpp"
#include "renderer/vulkan/vk_vma.h"


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

		for (int i = 0; i < FRAME_OVERLAP; i++) {
			vkDestroyCommandPool(m_device, m_frames[i].commandpool, nullptr);

      vkDestroyFence(m_device, m_frames[i].render_fence, nullptr);
      //vkDestroySemaphore(m_device, m_frames[i].render_semaphore, nullptr);
      vkDestroySemaphore(m_device, m_frames[i].swapchain_semaphore, nullptr);
		}
    for (VkSemaphore sem : m_render_finished_semaphores){
      vkDestroySemaphore(m_device, sem, nullptr);
    }
    m_render_finished_semaphores.clear();

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

    if (m_allocator != VK_NULL_HANDLE){
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

/*

void Renderer::FrameStart()
{
  vkWaitForFences(
      m_device,
      1,
      &m_in_flight,
      VK_TRUE,
      UINT64_MAX);

  u32 image_index;

  VkResult result = vkAcquireNextImageKHR(
      m_device,
      m_swapchain,
      UINT64_MAX,
      m_image_available,
      VK_NULL_HANDLE,
      &image_index);

  if (result != VK_SUCCESS &&
      result != VK_SUBOPTIMAL_KHR)
  {
    LOG_FATAL("Failed to acquire swapchain image");
    return;
  }

  m_current_image = image_index;

  vkResetFences(
      m_device,
      1,
      &m_in_flight);

  vkResetCommandBuffer(
      m_cmdbuffers[m_current_image],
      0);

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  RESULT = vkBeginCommandBuffer(
      m_cmdbuffers[m_current_image],
      &begin_info);

  if (RESULT != VK_SUCCESS)
  {
    LOG_FATAL("Failed to begin command buffer");
    return;
  }

  VkCommandBuffer command_buffer =
      m_cmdbuffers[m_current_image];

  // PRESENT → TRANSFER
  TransitionImage(
      command_buffer,
      m_swapchain_images[m_current_image],
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

 
 

  VkRenderingAttachmentInfo color_attachment{};
  color_attachment.sType =
      VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;

  color_attachment.imageView =
      m_swapchain_imageviews[m_current_image];

  color_attachment.imageLayout =
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  color_attachment.loadOp =
      VK_ATTACHMENT_LOAD_OP_CLEAR;

  color_attachment.storeOp =
      VK_ATTACHMENT_STORE_OP_STORE;

  VkClearValue clear_value{};
  clear_value.color.float32[0] = 0.02f;
  clear_value.color.float32[1] = 0.04f;
  clear_value.color.float32[2] = 0.04f;
  clear_value.color.float32[3] = 1.0f;

  color_attachment.clearValue = clear_value;

  VkRenderingInfo rendering{};
  rendering.sType =
      VK_STRUCTURE_TYPE_RENDERING_INFO;

  rendering.renderArea.offset = {0, 0};
  rendering.renderArea.extent = m_swapchain_extent;

  rendering.layerCount = 1;

  rendering.colorAttachmentCount = 1;
  rendering.pColorAttachments = &color_attachment;

  vkCmdBeginRendering(
      m_cmdbuffers[m_current_image],
      &rendering);

  vkCmdBindPipeline(
      m_cmdbuffers[m_current_image],
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_pipelines[0]->pipeline);

  VkViewport viewport{};
  viewport.x = viewport.y = 0.0f;
  viewport.width = (float)m_swapchain_extent.width;
  viewport.height = (float)m_swapchain_extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(m_cmdbuffers[m_current_image], 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = m_swapchain_extent;

  vkCmdSetScissor(m_cmdbuffers[m_current_image], 0, 1, &scissor);

  vkCmdDraw(m_cmdbuffers[m_current_image], 3, 1, 0, 0);



  vkCmdEndRendering(
      m_cmdbuffers[m_current_image]);

  // TRANSFER → PRESENT
  TransitionImage(
      command_buffer,
      m_swapchain_images[m_current_image],
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

*/

/*
void Renderer::TransitionImage(
    VkCommandBuffer command_buffer,
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout)
{
  VkImageMemoryBarrier barrier{};

  barrier.sType =
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;

  barrier.srcQueueFamilyIndex =
      VK_QUEUE_FAMILY_IGNORED;

  barrier.dstQueueFamilyIndex =
      VK_QUEUE_FAMILY_IGNORED;

  barrier.image = image;

  barrier.subresourceRange.aspectMask =
      VK_IMAGE_ASPECT_COLOR_BIT;

  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;

  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags src_stage;
  VkPipelineStageFlags dst_stage;

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
    barrier.srcAccessMask = 0;

    barrier.dstAccessMask =
        VK_ACCESS_TRANSFER_WRITE_BIT;

    src_stage =
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    dst_stage =
        VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (
      old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
      new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
  {
    barrier.srcAccessMask =
        VK_ACCESS_TRANSFER_WRITE_BIT;

    barrier.dstAccessMask = 0;

    src_stage =
        VK_PIPELINE_STAGE_TRANSFER_BIT;

    dst_stage =
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  }
  else if (
      old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
  {

    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dst_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  }
  else if (
      old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
      new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
  {
    barrier.srcAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    barrier.dstAccessMask = 0;

    src_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    dst_stage =
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  }

  else
  {
    LOG_FATAL("Unsupported image layout transition");
    return;
  }

  vkCmdPipelineBarrier(
      command_buffer,

      src_stage,
      dst_stage,

      0,

      0,
      nullptr,

      0,
      nullptr,

      1,
      &barrier);
}*/


// NOT AS PERFORMANT AS THE PREVIOUS TRANSITION WHEN WE HAVE MORE PASSES
void Renderer::TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier2 imageBarrier {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
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

    VkDependencyInfo depInfo {};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.pNext = nullptr;

    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &imageBarrier;

    vkCmdPipelineBarrier2(cmd, &depInfo);
}

/*
void Renderer::FrameEnd()
{
  VkPipelineStageFlags wait_stage =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submit{};
  submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore render_finished =
      m_render_finished[m_current_image];

  submit.waitSemaphoreCount = 1;
  submit.pWaitSemaphores = &m_image_available;

  submit.pWaitDstStageMask = &wait_stage;

  submit.commandBufferCount = 1;
  submit.pCommandBuffers =
      &m_cmdbuffers[m_current_image];

  submit.signalSemaphoreCount = 1;
  submit.pSignalSemaphores = &render_finished;

  RESULT = vkEndCommandBuffer(
      m_cmdbuffers[m_current_image]);

  if (RESULT != VK_SUCCESS)
  {
    LOG_FATAL("Failed to end command buffer");
    return;
  }

  VkResult result = vkQueueSubmit(
      m_graphqueue,
      1,
      &submit,
      m_in_flight);

  if (result != VK_SUCCESS)
  {
    LOG_FATAL("Failed to submit command buffer");
    return;
  }

  VkPresentInfoKHR present{};
  present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  present.waitSemaphoreCount = 1;
  present.pWaitSemaphores = &render_finished;

  present.swapchainCount = 1;
  present.pSwapchains = &m_swapchain;

  present.pImageIndices = &m_current_image;

  result = vkQueuePresentKHR(
      m_presentqueue,
      &present);

  if (result != VK_SUCCESS &&
      result != VK_SUBOPTIMAL_KHR)
  {
    LOG_FATAL("Failed to present swapchain image");
  }
}*/


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


void Renderer::DestroyBuffer(const AllocatedBuffer& buff)
{
  vmaDestroyBuffer(m_allocator, buff.buffer, buff.allocation);
}


/*
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
  VkBufferDeviceAddressInfo deviceAdressInfo
  {
    .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
    .pNext = nullptr,
    .buffer = meshbuffs.vertex_buffer.buffer,
  };
	meshbuffs.vertex_address = vkGetBufferDeviceAddress(m_device, &deviceAdressInfo);



  AllocatedBuffer staging = CreateBuffer(
      VMA_MEMORY_USAGE_CPU_ONLY, 
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
      vertbuff_size + indbuff_size);
  
  void* data = staging.allocation->GetMappedData();
  
  memcpy(data, vertices.data(), vertbuff_size);
  memcpy((char*)data + vertbuff_size, indices.data(), indbuff_size);




// UNFINISHED





  return meshbuffs;
}
*/



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
      wait_timeout_ns )
    );
  VK_CHECK(
      vkResetFences(
        m_device, fence_count, &GetCurrentFrame().render_fence
        )
      );

  // Request image to draw to from the swapchain
  u32 swapchain_image;
  VK_CHECK(
      vkAcquireNextImageKHR(
        m_device, 
        m_swapchain, 
        wait_timeout_ns, 
        GetCurrentFrame().swapchain_semaphore,
        nullptr,
        &swapchain_image
      )
    );

  VkCommandBuffer cmd = GetCurrentFrame().commandbuffer;

  VK_CHECK(vkResetCommandBuffer(cmd, 0));
  VkCommandBufferBeginInfo cmd_begininfo = CreateInfo_CommandBufferBegin(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
      );
  
  // Start recording
  VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begininfo));

  TransitionImage(
      cmd, 
      m_swapchain_images[swapchain_image],
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

  VkClearColorValue clr_value;
  float flash = std::abs(sin((float)m_framenumber / 100000.0f));
  float flash1 = std::abs(sin((float)m_framenumber / 60000.0f));
  clr_value = { {flash1, 0.0f, flash, 1.0f} };

  VkImageSubresourceRange clr_range =
    CreateImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
  
  // Clear the image
  vkCmdClearColorImage(
      cmd, 
      m_swapchain_images[swapchain_image], 
      VK_IMAGE_LAYOUT_GENERAL, 
      &clr_value, 1, &clr_range);


  // Presenting - no more drawing
  TransitionImage(
      cmd, 
      m_swapchain_images[swapchain_image],
      VK_IMAGE_LAYOUT_GENERAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  VK_CHECK(vkEndCommandBuffer(cmd));


  VkCommandBufferSubmitInfo cmdinfo = CreateInfo_CommandBufferSubmit(cmd);

  VkSemaphoreSubmitInfo wait_info = 
    CreateInfo_SemaphoreSubmit(
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
      GetCurrentFrame().swapchain_semaphore
      );
  VkSemaphoreSubmitInfo signal_info = 
    CreateInfo_SemaphoreSubmit(
        VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
        m_render_finished_semaphores[swapchain_image]
        //GetCurrentFrame().render_semaphore
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

  //present_info.pWaitSemaphores = &GetCurrentFrame().render_semaphore;
  present_info.pWaitSemaphores = &m_render_finished_semaphores[swapchain_image];

  present_info.waitSemaphoreCount = 1;

  present_info.pImageIndices = &swapchain_image;

  VK_CHECK(vkQueuePresentKHR(m_graphqueue, &present_info));
  m_framenumber++;
}
