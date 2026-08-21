#include "renderer/vulkan/vk_renderer.hpp"

using namespace VK;

void Renderer::CreateVmaAllocator()
{
  VmaAllocatorCreateInfo info{};
  info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
  info.device = m_device;
  info.instance = m_instance;
  info.pVulkanFunctions = nullptr;

  vmaCreateAllocator(&info, &m_vmaallocator);
}

