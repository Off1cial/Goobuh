#include "renderer/vulkan/vk_renderer.hpp"
#include "renderer/vulkan//vk_vma.h"

using namespace VK;

void Renderer::CreateVmaAllocator()
{
  VmaAllocatorCreateInfo info{};
  info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
  info.device = m_device;
  info.physicalDevice = m_physdevice;
  info.instance = m_instance;
  info.pVulkanFunctions = nullptr;

  vmaCreateAllocator(&info, &m_allocator);
}

