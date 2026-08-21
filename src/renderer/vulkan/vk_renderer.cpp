#include <algorithm>
#include "core/common.h"
#include "core/logsys.hpp"
#include "vk_renderer.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <ranges>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <iostream>
#include "renderer/vulkan/vk_vma.h"


VkResult RESULT;

static inline void RESULTCHECK(const char *failmsg)
{
  if (RESULT != VK_SUCCESS)
  {
    LOG_FATAL(failmsg);
  }
}



using namespace VK;

Renderer::Renderer(Plat::Window &window)
{
  Init(window);
}

Renderer::~Renderer()
{
  Shutdown();
}

bool Renderer::Init(Plat::Window &window)
{

  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Goobuh";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "Goobuh Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_4;

  u32 extension_count = 0;
  const char *const *extension_names = 0; // A pointer which is const, to a pointer of const char*??
  extension_names = SDL_Vulkan_GetInstanceExtensions(&extension_count);
  const char *validation_layer =
      "VK_LAYER_KHRONOS_validation";
  const VkInstanceCreateInfo vkinfo =
      {
          .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .pApplicationInfo = &app_info,
          .enabledLayerCount = 1,
          .ppEnabledLayerNames = &validation_layer,
          .enabledExtensionCount = extension_count,
          .ppEnabledExtensionNames = extension_names,
      };

  RESULT = vkCreateInstance(&vkinfo, nullptr, &m_instance);
  RESULTCHECK("Failed to create vulkan instance");
  LOG_DEFAULT("Instanced created");
  // Set up physical device
  u32 physdevice_count = 0;
  vkEnumeratePhysicalDevices(m_instance, &physdevice_count, nullptr);
  std::vector<VkPhysicalDevice> physical_devices(physdevice_count);
  vkEnumeratePhysicalDevices(m_instance, &physdevice_count, physical_devices.data());
  m_physdevice = physical_devices[0];

  // Physical device properties
  VkPhysicalDeviceProperties physdevice_properties{};
  vkGetPhysicalDeviceProperties(m_physdevice, &physdevice_properties);
  // Log the api version
  LOG_DEFAULT(
    "Vulkan API: %u.%u.%u",
    VK_API_VERSION_MAJOR(physdevice_properties.apiVersion),
    VK_API_VERSION_MINOR(physdevice_properties.apiVersion),
    VK_API_VERSION_PATCH(physdevice_properties.apiVersion));

  uint32_t q_familycount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(m_physdevice, &q_familycount, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(q_familycount);
  vkGetPhysicalDeviceQueueFamilyProperties(m_physdevice, &q_familycount, queue_families.data());
  // Create Vulkan surface
  bool res = SDL_Vulkan_CreateSurface(window.GetSDLWindow(), m_instance, nullptr, &m_surface);
  if (!res)
  {
    LOG_FATAL("Failed to create vulkan surface");
  }

  VkBool32 support;
  u32 i = 0;
  for (VkQueueFamilyProperties q_family : queue_families)
  {
    if (m_graphqueue_index == UINT32_MAX && q_family.queueCount > 0 && q_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
      m_graphqueue_index = i;
    }
    if (m_presentqueue_index == UINT32_MAX)
    {
      vkGetPhysicalDeviceSurfaceSupportKHR(m_physdevice, i, m_surface, &support);
      if (support)
        m_presentqueue_index = i;
    }
    ++i;
  }
  float q_priority = 1.0f;
  VkDeviceQueueCreateInfo q_info =
      {
          .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .queueFamilyIndex = m_graphqueue_index,
          .queueCount = 1,
          .pQueuePriorities = &q_priority};

  VkPhysicalDeviceFeatures device_features = {};
  const char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering{};
  dynamic_rendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
  dynamic_rendering.dynamicRendering = VK_TRUE;

  VkDeviceCreateInfo create_info =
      {
          .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
          .pNext = &dynamic_rendering,
          .flags = 0,
          .queueCreateInfoCount = 1,
          .pQueueCreateInfos = &q_info,
          .enabledLayerCount = 0,
          .ppEnabledLayerNames = nullptr,
          .enabledExtensionCount = 1,
          .ppEnabledExtensionNames = device_extensions,
          .pEnabledFeatures = &device_features};

  RESULT = vkCreateDevice(m_physdevice, &create_info, nullptr, &m_device);
  RESULTCHECK("Failed to create vulkan device");
  /*
  auto pfnBeginRendering =
    reinterpret_cast<PFN_vkCmdBeginRendering>(
        vkGetDeviceProcAddr(m_device, "vkCmdBeginRendering"));
  auto pfnEndRendering =
    reinterpret_cast<PFN_vkCmdEndRendering>(
        vkGetDeviceProcAddr(m_device, "vkCmdEndRendering"));


  LOG_DEFAULT("vkCmdBeginRendering: %p", (void*)pfnBeginRendering);
  LOG_DEFAULT("vkCmdEndRendering:   %p", (void*)pfnEndRendering);
  */
  vkGetDeviceQueue(m_device, m_graphqueue_index, 0, &m_graphqueue);
  vkGetDeviceQueue(m_device, m_presentqueue_index, 0, &m_presentqueue);

  if (!CreateSwapChain(window))
  {
    LOG_FATAL("Failed to create vulkan swapchain");
    return false;
  };
  if (!CreateCommandPool())
  {
    LOG_FATAL("Failed to create vulkan command pool");
    return false;
  }
  if (!CreateCommandBuffers())
  {
    LOG_FATAL("Failed to create vulkan command buffers");
    return false;
  }
  if (!CreateSyncObjects())
  {
    LOG_FATAL("Failed to create vulkan sync objects");
    return false;
  }
  CreateVmaAllocator();  // Add error checking
  if (!CreateVertexBuffer())
  {
    LOG_FATAL("Failed to create vertex buffer");
    return false;
  }

  std::string vertsrc = "resource/shaders/triangle.vert.spv";
  std::string fragsrc = "resource/shaders/triangle.frag.spv";
  m_shader = std::make_unique<Shader>(m_device, vertsrc, fragsrc);

  m_pipelinebuilder = std::make_unique<PipelineBuilder>(m_device);
  m_pipelines.push_back(
      std::make_unique<Pipeline>(CreatePipeline(
          *m_shader,
          VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
          VK_POLYGON_MODE_FILL,
          VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE,
          MSAASampleCount::MSAA1,
          m_swapchain_format,
          VK_FORMAT_UNDEFINED
          )
  ));
  //m_pipelines.push_back(std::make_unique<Pipeline>(m_device, *m_shader, m_swapchain_format));
  return true;
}

void Renderer::Shutdown()
{
  if (m_device != VK_NULL_HANDLE)
  {
    // Make sure the GPU is finished before destroying resources.
    vkDeviceWaitIdle(m_device);

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

  /*  NOT DYNAMIC

  // CLEAR
  VkClearColorValue clear_color{};

  clear_color.float32[0] = 0.02f;
  clear_color.float32[1] = 0.02f;
  clear_color.float32[2] = 0.04f;
  clear_color.float32[3] = 1.0f;

  VkImageSubresourceRange range{};

  range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  range.baseMipLevel = 0;
  range.levelCount = 1;
  range.baseArrayLayer = 0;
  range.layerCount = 1;

  vkCmdClearColorImage(
      command_buffer,
      m_swapchain_images[m_current_image],
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      &clear_color,
      1,
      &range);

  */

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
}

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

bool Renderer::CreateVertexBuffer()
{
  VkBufferCreateInfo buffer_info{};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = m_vertexbuffer_size;
  buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkResult result = vkCreateBuffer(m_device, &buffer_info, nullptr, &m_vertexbuffer);

  if (result != VK_SUCCESS){
    LOG_ERROR("Failed to create vertex buffer");
    return false;
  }

  VkMemoryRequirements mem_requirements{};
  vkGetBufferMemoryRequirements(m_device, m_vertexbuffer, &mem_requirements);

  uint32_t memory_type = FindMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VkMemoryAllocateInfo allocate_info{};
  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize = mem_requirements.size;
  allocate_info.memoryTypeIndex = memory_type;

  result =  vkAllocateMemory(m_device, &allocate_info, nullptr, &m_vertexmemory);
  if (result != VK_SUCCESS){
    LOG_ERROR("Failed to allocate memory to the vertex buffer");
    return false;
  }

  vkBindBufferMemory(
    m_device,
    m_vertexbuffer,
    m_vertexmemory,
    0);

  return true;
}

AllocatedBuffer Renderer::CreateBuffer(const VmaMemoryUsage mem_usage, const VkBufferUsageFlags buff_usage, const size_t size)
{
  VkBufferCreateInfo buff_info{};
  VmaAllocationCreateInfo alloc_info{};
  AllocatedBuffer new_buff{};

  // Prepare buffer information
  buff_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buff_info.pNext = nullptr;
  buff_info.usage = buff_usage;
  buff_info.size  = size;
  // Prepare allocation information
  alloc_info.usage = mem_usage; 
  alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

  VK_CHECK(vmaCreateBuffer(m_allocator, &buff_info, &alloc_info, &new_buff.buffer, &new_buff.allocation, &new_buff.info));
  return new_buff;
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



Pipeline Renderer::CreatePipeline(
    const Shader& shader,
    const VkPrimitiveTopology topology,
    const VkPolygonMode polygonmode,
    const VkCullModeFlags flags, 
    const VkFrontFace front,
    const MSAASampleCount msaa_samples,
    const VkFormat color_attachment_format,
    const VkFormat depth_format
    )
{
  m_pipelinebuilder->Clear();
  m_pipelinebuilder->SetShaderModules(shader.GetModule_Vertex(), shader.GetModule_Fragment());
  m_pipelinebuilder->SetInputTopology(topology);
  m_pipelinebuilder->SetPolygonMode(polygonmode);
  m_pipelinebuilder->SetCullMode(flags, front);
  m_pipelinebuilder->SetMSAA(m_physdevice, (VkSampleCountFlagBits)msaa_samples);
  m_pipelinebuilder->SetColorAttachmentFormat(color_attachment_format);
  m_pipelinebuilder->SetDepthFormat(depth_format);
  m_pipelinebuilder->DisableBlending();
  return m_pipelinebuilder->BuildPipeline(m_device);
}
