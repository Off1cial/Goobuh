#include "renderer/vulkan/vk_renderer.hpp"
#include "core/logsys.hpp"

using namespace VK;

bool Renderer::CreateSyncObjects()
{
  m_render_finished.resize(m_swapchain_images.size());

  VkSemaphoreCreateInfo semaphore_info{};
  semaphore_info.sType =
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_info{};
  fence_info.sType =
      VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

  fence_info.flags =
      VK_FENCE_CREATE_SIGNALED_BIT;

  if (vkCreateSemaphore(
          m_device,
          &semaphore_info,
          nullptr,
          &m_image_available) != VK_SUCCESS)
  {
    LOG_FATAL("Failed to create image available semaphore");
    return false;
  }

  if (vkCreateFence(
          m_device,
          &fence_info,
          nullptr,
          &m_in_flight) != VK_SUCCESS)
  {
    LOG_FATAL("Failed to create in-flight fence");
    return false;
  }

  for (VkSemaphore &semaphore : m_render_finished)
  {
    if (vkCreateSemaphore(
            m_device,
            &semaphore_info,
            nullptr,
            &semaphore) != VK_SUCCESS)
    {
      LOG_FATAL("Failed to create render finished semaphore");
      return false;
    }
  }

  return true;
}
