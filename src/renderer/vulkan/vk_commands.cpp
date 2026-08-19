#include "renderer/vulkan/vk_renderer.hpp"
#include "core/logsys.hpp"

using namespace VK;

bool Renderer::CreateCommandPool()
{
  VkCommandPoolCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  info.queueFamilyIndex = m_graphqueue_index;

  VkResult result = vkCreateCommandPool(
      m_device,
      &info,
      nullptr,
      &m_cmdpool);

  if (result != VK_SUCCESS)
  {
    LOG_FATAL("Failed to create command pool");
    return false;
  }

  return true;
}

bool Renderer::CreateCommandBuffers()
{
  m_cmdbuffers.resize(m_swapchain_images.size());

  VkCommandBufferAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.commandPool = m_cmdpool;
  info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  info.commandBufferCount =
      static_cast<uint32_t>(m_cmdbuffers.size());

  VkResult result = vkAllocateCommandBuffers(
      m_device,
      &info,
      m_cmdbuffers.data());

  if (result != VK_SUCCESS)
  {
    LOG_FATAL("Failed to allocate command buffers");
    return false;
  }

  return true;
}