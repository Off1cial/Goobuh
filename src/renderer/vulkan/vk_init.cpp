#include "volk/volk.h"

#include "renderer/vulkan/vk_vma.h"
#include "renderer/vulkan/vk_engine.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>
#include <iostream>

static inline void vkcheck(int result)
{
  if (result != VK_SUCCESS)
  {
    LOG_FATAL("Vulkan check failed, result %d\n", result);
    exit(1);
  }
}

using namespace VK;

void VKRenderer::Init()
{
  std::cout << "Initialising Vulkan renderer\n";
  vkcheck(volkInitialize());
  // Create Application/Info
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Goobuh";
  app_info.apiVersion = VK_API_VERSION_1_3;

  uint32_t instance_ext_count = 0;
  char const *const *extension_names = SDL_Vulkan_GetInstanceExtensions(&instance_ext_count);

  VkInstanceCreateInfo instance_info{};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  instance_info.enabledExtensionCount = instance_ext_count;
  instance_info.ppEnabledExtensionNames = extension_names;

  vkcheck(vkCreateInstance(&instance_info, nullptr, &_instance));
  LOG_DEFAULT("Vulkan instance created");
  volkLoadInstance(_instance);
  if(!SDL_Vulkan_CreateSurface(_window->GetSDLWindow(), _instance, nullptr, &_surface)){
    LOG_FATAL("Failed to create vulkan surface");
  }

  // Find physical devices
  uint32_t device_count = 0;
  vkcheck(vkEnumeratePhysicalDevices(_instance, &device_count, nullptr));
  _physdevices.resize(device_count);
  vkcheck(vkEnumeratePhysicalDevices(_instance, &device_count, _physdevices.data()));
  // Default to device 0
  _physdevice_index = 0;

  VkPhysicalDeviceProperties2 phys_properties{};
  phys_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
  vkGetPhysicalDeviceProperties2(_physdevices[_physdevice_index], &phys_properties);
  printf("Selected device: %s\n", phys_properties.properties.deviceName);

  // Get the needed queue
  uint32_t qfamily_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(
      _physdevices[_physdevice_index],
      &qfamily_count,
      nullptr);
  _qfamilies.resize(qfamily_count);
  vkGetPhysicalDeviceQueueFamilyProperties(
      _physdevices[_physdevice_index],
      &qfamily_count,
      _qfamilies.data());
  for (uint32_t i = 0; i < qfamily_count; i++)
  {
    if (_qfamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
      _qfamily = i;
      break;
    }
  }
  // Check if presentation is supported
  if(SDL_Vulkan_GetPresentationSupport(
      _instance,
      _physdevices[_physdevice_index],
      qfamily_count))
  {
    LOG_FATAL("Failed to obtain SDL presentation support");
  }

    
  LOG_DEFAULT("Vulkan: Fetched presentation support");

  const float qpriorities = {1.0f};
  VkDeviceQueueCreateInfo queue_info{};
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.queueCount = 1;
  queue_info.queueFamilyIndex = _qfamily;
  queue_info.pQueuePriorities = &qpriorities;

  const std::vector<const char *> device_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkPhysicalDeviceVulkan12Features enabledVk12Features{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
      .descriptorIndexing = true,
      .shaderSampledImageArrayNonUniformIndexing = true,
      .descriptorBindingVariableDescriptorCount = true,
      .runtimeDescriptorArray = true,
      .bufferDeviceAddress = true};
  VkPhysicalDeviceVulkan13Features enabledVk13Features{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .pNext = &enabledVk12Features,
      .synchronization2 = true,
      .dynamicRendering = true,
  };
  VkPhysicalDeviceFeatures enabledVk10Features{
      .samplerAnisotropy = VK_TRUE,
  };

  // Get driver
  //
  VkDeviceCreateInfo device_info{};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.enabledExtensionCount = (unsigned int)device_extensions.size();
  device_info.ppEnabledExtensionNames = device_extensions.data();
  device_info.pEnabledFeatures = &enabledVk10Features;
  device_info.pNext = &enabledVk13Features;
  vkcheck(vkCreateDevice(_physdevices[_physdevice_index], &device_info, nullptr, &_device));
  LOG_DEFAULT("Vulkan: created _device");
  volkLoadDevice(_device);


  // Initialise VMA
  VmaVulkanFunctions vkFunctions{};
  VmaAllocatorCreateInfo allocatorCI{
      .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
      .physicalDevice = _physdevices[_physdevice_index],
      .device = _device,
      .pVulkanFunctions = &vkFunctions,
      .instance = _instance};
  vmaImportVulkanFunctionsFromVolk(&allocatorCI, &vkFunctions);

  vkcheck(vmaCreateAllocator(&allocatorCI, &_allocator));
  LOG_DEFAULT("Vulkan: allocator created");
  
  CreateSwapchain();
  _render_complete_semaphores.resize(_swapchain_images.size());
  

  VkCommandPoolCreateInfo cmd_pool_info{};
  cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  cmd_pool_info.queueFamilyIndex = _qfamily;
  vkcheck(vkCreateCommandPool(_device, &cmd_pool_info, nullptr, &_command_pool));

  // Create data for each frame in flight
  VkSemaphoreCreateInfo semaphore_info{};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkFenceCreateInfo fence_info{};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  VkCommandBufferAllocateInfo cmd_buff_alloc_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext = nullptr,
    .commandPool = _command_pool,
    .commandBufferCount = _max_frames_in_flight
  };
  vkcheck(vkAllocateCommandBuffers(_device, &cmd_buff_alloc_info, _frame_command_buffers.data()));
  for (uint32_t i = 0; i < _max_frames_in_flight; i++){
    VkBufferCreateInfo buff_create_info{};
    buff_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buff_create_info.size = sizeof(ShaderDataBuffer);
    buff_create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VmaAllocationCreateInfo alloc_create_info{};
    alloc_create_info.flags = 
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
      VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
      VMA_ALLOCATION_CREATE_MAPPED_BIT;
    alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO;

    vkcheck(
        vmaCreateBuffer(
          _allocator, 
          &buff_create_info, 
          &alloc_create_info, 
          &_shader_data_buffers[i].buffer,
          &_shader_data_buffers[i].allocation,
          &_shader_data_buffers[i].allocation_info)
      );

    VkBufferDeviceAddressInfo addr_info{};
    addr_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    addr_info.buffer = _shader_data_buffers[i].buffer;
    _shader_data_buffers[i].address = vkGetBufferDeviceAddress(_device, &addr_info);
  
    vkcheck(vkCreateFence(_device, &fence_info, nullptr, &_frame_fences[i]));
    vkcheck(vkCreateSemaphore(_device, &semaphore_info, nullptr, &_image_acquired_semaphores[i]));
  }
  for (VkSemaphore& semaphore : _render_complete_semaphores){
    vkcheck(vkCreateSemaphore(_device, &semaphore_info, nullptr, &semaphore));
  }

}

void VKRenderer::CreateSwapchain()
{
  vkcheck(
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
          _physdevices[_physdevice_index],
          _surface,
          &_surface_capabilities));
  VkExtent2D swapchain_extent = _surface_capabilities.currentExtent;

  // dimension correction
  int window_width, window_height;
  _window->GetDimensions(window_width, window_height);
  if (swapchain_extent.width == 0xFFFFFFFF)
  {
    swapchain_extent.width = (uint32_t)(window_width);
    swapchain_extent.height = (uint32_t)(window_height);
  }

  const VkFormat image_format = VK_FORMAT_B8G8R8A8_SRGB;
  // Prepare swapchain information
  VkSwapchainCreateInfoKHR swapchain_info{};
  swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.surface = _surface;
  swapchain_info.minImageCount = _surface_capabilities.minImageCount;
  swapchain_info.imageFormat = image_format;
  swapchain_info.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  swapchain_info.imageExtent.width = swapchain_extent.width;
  swapchain_info.imageExtent.height = swapchain_extent.height;
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

  vkcheck(vkCreateSwapchainKHR(_device, &swapchain_info, nullptr, &_swapchain));

  uint32_t image_count = 0;
  vkcheck(vkGetSwapchainImagesKHR(_device, _swapchain, &image_count, nullptr));
  _swapchain_images.resize(image_count);
  vkcheck(vkGetSwapchainImagesKHR(_device, _swapchain, &image_count, _swapchain_images.data()));
  _swapchain_images.resize(image_count); // just in case it magically changed

  std::vector<VkFormat> depth_format_list{VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT};
  VkFormat depth_format = VK_FORMAT_UNDEFINED;
  for (VkFormat &format : depth_format_list)
  {
    VkFormatProperties2 format_properties;
    format_properties.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    vkGetPhysicalDeviceFormatProperties2(
        _physdevices[_physdevice_index],
        format,
        &format_properties);

    if (format_properties.formatProperties.optimalTilingFeatures &
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
      depth_format = format;
      break;
    }
  }
  _depth_format = depth_format;

  VkImageCreateInfo depth_image_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = depth_format,
      .extent{
          .width = (uint32_t)window_width,
          .height = (uint32_t)window_height,
          .depth = 1},

      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;

  vkcheck(
      vmaCreateImage(
          _allocator,
          &depth_image_info,
          &alloc_info,
          &_depth_image,
          &_depth_image_allocation,
          nullptr));

  VkImageViewCreateInfo depth_view_info{};
  depth_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  depth_view_info.image = _depth_image;
  depth_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  depth_view_info.format = _depth_format;
  depth_view_info.subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
      .levelCount = 1,
      .layerCount = 1};

  vkcheck(vkCreateImageView(_device, &depth_view_info, nullptr, &_depth_image_view));
}
