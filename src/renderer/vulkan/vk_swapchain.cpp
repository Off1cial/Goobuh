#include "renderer/vulkan/vk_renderer.hpp"
#include "core/common.h"
#include "core/logsys.hpp"
#include <vulkan/vulkan_core.h>

using namespace VK;

bool Renderer::CreateSwapChain(Plat::Window &window)
{
  VkSurfaceCapabilitiesKHR capabilities{};

  VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      m_physdevice,
      m_surface,
      &capabilities);

  if (result != VK_SUCCESS)
  {
    LOG_FATAL("Failed to get Vulkan surface capabilities");
    return false;
  }

  // Surface format
  u32 format_count = 0;

  result = vkGetPhysicalDeviceSurfaceFormatsKHR(
      m_physdevice,
      m_surface,
      &format_count,
      nullptr);

  if (result != VK_SUCCESS || format_count == 0)
  {
    LOG_FATAL("Failed to get Vulkan surface formats");
    return false;
  }

  std::vector<VkSurfaceFormatKHR> formats(format_count);

  result = vkGetPhysicalDeviceSurfaceFormatsKHR(
      m_physdevice,
      m_surface,
      &format_count,
      formats.data());

  if (result != VK_SUCCESS)
  {
    LOG_FATAL("Failed to enumerate Vulkan surface formats");
    return false;
  }

  VkSurfaceFormatKHR surface_format = formats[0];

  // Prefer a standard SRGB format if available.
  for (const VkSurfaceFormatKHR &format : formats)
  {
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    {
      surface_format = format;
      break;
    }
  }

  // Present mode
  u32 present_mode_count = 0;

  result = vkGetPhysicalDeviceSurfacePresentModesKHR(
      m_physdevice,
      m_surface,
      &present_mode_count,
      nullptr);

  if (result != VK_SUCCESS || present_mode_count == 0)
  {
    LOG_FATAL("Failed to get Vulkan present modes");
    return false;
  }

  std::vector<VkPresentModeKHR> present_modes(
      present_mode_count);

  result = vkGetPhysicalDeviceSurfacePresentModesKHR(
      m_physdevice,
      m_surface,
      &present_mode_count,
      present_modes.data());

  if (result != VK_SUCCESS)
  {
    LOG_FATAL("Failed to enumerate Vulkan present modes");
    return false;
  }

  VkPresentModeKHR present_mode = DeterminePresentMode(present_modes, PresentMode::Immediate);

  // Swapchain extent
  VkExtent2D extent{};

  if (capabilities.currentExtent.width != UINT32_MAX)
  {
    extent = capabilities.currentExtent;
  }
  else
  {
    int width;
    int height;

    window.GetDimensions(width, height);

    extent.width = static_cast<u32>(width);
    extent.height = static_cast<u32>(height);

    extent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(
            capabilities.maxImageExtent.width,
            extent.width));

    extent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(
            capabilities.maxImageExtent.height,
            extent.height));
  }

  // Image count
  u32 image_count = capabilities.minImageCount + 1;

  if (capabilities.maxImageCount > 0 &&
      image_count > capabilities.maxImageCount)
  {
    image_count = capabilities.maxImageCount;
  }

  // Swapchain creation start
  VkSwapchainCreateInfoKHR create_info{};

  create_info.sType =
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

  create_info.surface = m_surface;

  create_info.minImageCount = image_count;

  create_info.imageFormat =
      surface_format.format;

  create_info.imageColorSpace =
      surface_format.colorSpace;

  create_info.imageExtent =
      extent;

  create_info.imageArrayLayers = 1;

  create_info.imageUsage =
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

  // Queue family sharing
  u32 queue_family_indices[] =
      {
          m_graphqueue_index,
          m_presentqueue_index};

  if (m_graphqueue_index != m_presentqueue_index)
  {
    create_info.imageSharingMode =
        VK_SHARING_MODE_CONCURRENT;

    create_info.queueFamilyIndexCount = 2;

    create_info.pQueueFamilyIndices =
        queue_family_indices;
  }
  else
  {
    create_info.imageSharingMode =
        VK_SHARING_MODE_EXCLUSIVE;
  }

  // Remaining swapchain configuration
  create_info.preTransform =
      capabilities.currentTransform;

  create_info.compositeAlpha =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  create_info.presentMode =
      present_mode;

  create_info.clipped = VK_TRUE;

  create_info.oldSwapchain =
      VK_NULL_HANDLE;

#ifdef _DEBUG
  printf("---- SWAPCHAIN ----\n");
  printf("surface: %p\n", (void *)create_info.surface);
  printf("minImageCount: %u\n", create_info.minImageCount);
  printf("imageFormat: %d\n", create_info.imageFormat);
  printf("imageColorSpace: %d\n", create_info.imageColorSpace);
  printf("extent: %u x %u\n",
         create_info.imageExtent.width,
         create_info.imageExtent.height);
  printf("imageArrayLayers: %u\n", create_info.imageArrayLayers);
  printf("imageUsage: 0x%x\n", create_info.imageUsage);
  printf("imageSharingMode: %d\n", create_info.imageSharingMode);
  printf("preTransform: 0x%x\n", create_info.preTransform);
  printf("compositeAlpha: 0x%x\n", create_info.compositeAlpha);
  printf("presentMode: %d\n", create_info.presentMode);
  printf("clipped: %d\n", create_info.clipped);
  printf("-------------------\n");

  printf(
      "current extent: %u x %u\n",
      capabilities.currentExtent.width,
      capabilities.currentExtent.height);

  printf(
      "min extent: %u x %u\n",
      capabilities.minImageExtent.width,
      capabilities.minImageExtent.height);

  printf(
      "max extent: %u x %u\n",
      capabilities.maxImageExtent.width,
      capabilities.maxImageExtent.height);

#endif
  // Create Swapchain
  result = vkCreateSwapchainKHR(
      m_device,
      &create_info,
      nullptr,
      &m_swapchain);

  if (result != VK_SUCCESS)
  {
    LOG_FATAL("Failed to create Vulkan swapchain");
    return false;
  }

  // Store swapchain properties
  m_swapchain_format =
      surface_format.format;

  m_swapchain_extent =
      extent;

  // Get swapchain images
  u32 swapchain_image_count = 0;

  result = vkGetSwapchainImagesKHR(
      m_device,
      m_swapchain,
      &swapchain_image_count,
      nullptr);

  if (result != VK_SUCCESS || swapchain_image_count == 0)
  {
    LOG_FATAL("Failed to get Vulkan swapchain images");
    return false;
  }

  m_swapchain_images.resize(
      swapchain_image_count);

  result = vkGetSwapchainImagesKHR(
      m_device,
      m_swapchain,
      &swapchain_image_count,
      m_swapchain_images.data());

  if (result != VK_SUCCESS)
  {
    LOG_FATAL("Failed to enumerate Vulkan swapchain images");
    return false;
  }

  // Create image views
  m_swapchain_imageviews.resize(
      m_swapchain_images.size());

  for (size_t i = 0;
       i < m_swapchain_images.size();
       ++i)
  {
    VkImageViewCreateInfo view_info{};

    view_info.sType =
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    view_info.image =
        m_swapchain_images[i];

    view_info.viewType =
        VK_IMAGE_VIEW_TYPE_2D;

    view_info.format =
        m_swapchain_format;

    view_info.components.r =
        VK_COMPONENT_SWIZZLE_IDENTITY;

    view_info.components.g =
        VK_COMPONENT_SWIZZLE_IDENTITY;

    view_info.components.b =
        VK_COMPONENT_SWIZZLE_IDENTITY;

    view_info.components.a =
        VK_COMPONENT_SWIZZLE_IDENTITY;

    view_info.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_COLOR_BIT;

    view_info.subresourceRange.baseMipLevel = 0;

    view_info.subresourceRange.levelCount = 1;

    view_info.subresourceRange.baseArrayLayer = 0;

    view_info.subresourceRange.layerCount = 1;

    result = vkCreateImageView(
        m_device,
        &view_info,
        nullptr,
        &m_swapchain_imageviews[i]);

    if (result != VK_SUCCESS)
    {
      LOG_FATAL("Failed to create Vulkan swapchain image view");
      return false;
    }
  }

  return true;
}
