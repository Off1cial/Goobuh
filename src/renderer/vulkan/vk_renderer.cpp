#include "core/common.h"
#include "vk_renderer.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>


using namespace VK;

Renderer::Renderer(Plat::Window& window)
{
  Init(window);
}

Renderer::~Renderer()
{
  Shutdown();
}

bool Renderer::Init(Plat::Window& window)
{
  u32 extension_count = 0; 
  const char* const* extension_names = 0; // dont ask
  
  extension_names = SDL_Vulkan_GetInstanceExtensions(&extension_count);
  const VkInstanceCreateInfo vkinfo = 
  {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .pApplicationInfo = nullptr,
    .enabledLayerCount = 0,
    .ppEnabledLayerNames = nullptr,
    .enabledExtensionCount = extension_count,
    .ppEnabledExtensionNames = extension_names,
  };

  vkCreateInstance(&vkinfo, nullptr, &m_instance);

  // Set up physical device
  u32 physdevice_count = 0;
  vkEnumeratePhysicalDevices(m_instance, &physdevice_count, nullptr);
  std::vector<VkPhysicalDevice> physical_devices(physdevice_count);
  vkEnumeratePhysicalDevices(m_instance, &physdevice_count, physical_devices.data());
  m_physdevice = physical_devices[0];


  uint32_t q_familycount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(m_physdevice, &q_familycount, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(q_familycount);
  vkGetPhysicalDeviceQueueFamilyProperties(m_physdevice, &q_familycount, queue_families.data());
  // Create Vulkan surface
  SDL_Vulkan_CreateSurface(window.GetSDLWindow(), m_instance,  nullptr, &m_surface);

  u32 graphqueue_index = UINT32_MAX;
  u32 presentqueue_index = UINT32_MAX;
  VkBool32 support;
  u32 i = 0;
  for (VkQueueFamilyProperties q_family : queue_families){
    if (graphqueue_index == UINT32_MAX && q_family.queueCount > 0 && q_family.queueFlags & VK_QUEUE_GRAPHICS_BIT){
      graphqueue_index = i;
    }
    if (presentqueue_index == UINT32_MAX){
      vkGetPhysicalDeviceSurfaceSupportKHR(m_physdevice, i, m_surface, &support);
      if (support) presentqueue_index = i;
    }
    ++i;
  }
  float q_priority = 1.0f;
  VkDeviceQueueCreateInfo q_info = 
  {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .queueFamilyIndex = graphqueue_index,
    .queueCount = 1,
    .pQueuePriorities = &q_priority
  };

  VkPhysicalDeviceFeatures device_features = {};
  const char* device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  VkDeviceCreateInfo create_info = 
  {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .queueCreateInfoCount = 1,
    .pQueueCreateInfos = &q_info,
    .enabledLayerCount = 0,
    .ppEnabledLayerNames = nullptr,
    .enabledExtensionCount = 1,
    .ppEnabledExtensionNames = device_extensions,
    .pEnabledFeatures = &device_features
  };
  
  vkCreateDevice(m_physdevice, &create_info, nullptr, &m_device);
  vkGetDeviceQueue(m_device, graphqueue_index, 0, &m_graphqueue);
  vkGetDeviceQueue(m_device, presentqueue_index, 0, &m_presentqueue);
  
  return true;
}


void Renderer::Shutdown()
{
  vkDestroyDevice(m_device, nullptr);
  vkDestroyInstance(m_instance, nullptr);
}

