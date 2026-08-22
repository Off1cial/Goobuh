#include "renderer/vulkan/vk_renderer.hpp"
#include "core/logsys.hpp"

using namespace VK;

bool Renderer::CreateCommandPool()
{
  VkCommandPoolCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  info.queueFamilyIndex = m_graphqueue_index;

  for (int i = 0; i < FRAME_OVERLAP; i++)
  {
    VkResult result = vkCreateCommandPool(
        m_device, &info, nullptr, &m_frames[i].commandpool);

    if (result != VK_SUCCESS)
    {
      LOG_FATAL("Failed to create command pool");
      return false;
    }

    VkCommandBufferAllocateInfo cmdAllocInfo{};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.commandPool = m_frames[i].commandpool;
    cmdAllocInfo.commandBufferCount = 1;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    result = vkAllocateCommandBuffers(
        m_device, &cmdAllocInfo, &m_frames[i].commandbuffer);

    if (result != VK_SUCCESS)
    {
      LOG_FATAL("Failed to allocate command buffer");
      return false;
    }
  }
  return true;
}