#include "renderer/vulkan/vk_renderer.hpp"
#include "renderer/vulkan/vk_types.hpp"
#include "core/logsys.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

using namespace VK;

void Renderer::CreateSyncStructures()
{
  VkFenceCreateInfo fence_info = CreateInfo_Fence(VK_FENCE_CREATE_SIGNALED_BIT);
  VkSemaphoreCreateInfo semaphore_info = CreateInfo_Semaphore();

  for (int i = 0; i < FRAME_OVERLAP; i++){
    VK_CHECK(vkCreateFence(m_device, &fence_info, nullptr, &m_frames[i].render_fence));

    VK_CHECK(vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_frames[i].swapchain_semaphore));
    //VK_CHECK(vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_frames[i].render_semaphore));

  }
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

  VK_CHECK(vkCreateInstance(&vkinfo, nullptr, &m_instance));
  //RESULTCHECK("Failed to create vulkan instance");
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

  VkPhysicalDeviceSynchronization2Features sync2_features{};
  sync2_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
  sync2_features.synchronization2 = VK_TRUE;
  sync2_features.pNext = &dynamic_rendering; // chain: sync2 -> dynamic_rendering


  VkDeviceCreateInfo create_info =
      {
          .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
          //.pNext = &dynamic_rendering,
          .pNext = &sync2_features,
          .flags = 0,
          .queueCreateInfoCount = 1,
          .pQueueCreateInfos = &q_info,
          .enabledLayerCount = 0,
          .ppEnabledLayerNames = nullptr,
          .enabledExtensionCount = 1,
          .ppEnabledExtensionNames = device_extensions,
          .pEnabledFeatures = &device_features};

  VK_CHECK(vkCreateDevice(m_physdevice, &create_info, nullptr, &m_device));
  //RESULTCHECK("Failed to create vulkan device");
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
  CreateSyncStructures();
  /*
  if (!CreateSyncObjects())
  {
    LOG_FATAL("Failed to create vulkan sync objects");
    return false;
  }*/
  CreateVmaAllocator();  // Add error checking

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
