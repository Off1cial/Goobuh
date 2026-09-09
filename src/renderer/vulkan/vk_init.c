#include "renderer/vulkan/vk_renderer.h"
#include "renderer/vulkan/vk_pipeline.h"
#include "renderer/vulkan/vk_vma.h"
#include "renderer/vulkan/vk_mesh.h"

#include "renderer/vulkan/vk_info.h"
#include "renderer/vulkan/vk_mesh_loader.h"
#include "common/logsys.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <stdlib.h>
#include <stdio.h>

static inline void vkcheck(VkResult result)
{
  if (result != VK_SUCCESS)
  {
    LOG_FATAL("Vulkan check failed");
    exit(1);
  }
}

/*
VkBool32 debug_call_back(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallback_data,
    void *pUserdata)
{
  if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
  {
    printf("Validation layer: %s\n", pCallback_data->pMessage);
  }
  return VK_FALSE;
}
*/

static void create_instance(VK_Renderer *engine)
{
  printf("Creating vulkan instance\n");
  volkInitialize();
  VkApplicationInfo app_info = {0};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Engine";
  app_info.apiVersion = VK_API_VERSION_1_4;

  uint32_t num_instance_extensions = 0;
  printf("Fetching instance extensions..\n");
  char const *const *instance_extension_names =
      SDL_Vulkan_GetInstanceExtensions(&num_instance_extensions);

  const char **requested_extensions =
      malloc((num_instance_extensions + 1) * sizeof(*requested_extensions));

  for (uint32_t i = 0; i < num_instance_extensions; i++)
  {
    requested_extensions[i] = instance_extension_names[i];
  }
  requested_extensions[num_instance_extensions] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  num_instance_extensions++;

  VkDebugUtilsMessengerCreateInfoEXT debug_info = {0};
  debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  //debug_info.pfnUserCallback = debug_call_back;

  const char *requested_layers[1];
  requested_layers[0] = "VK_LAYER_KHRONOS_validation";

  VkInstanceCreateInfo instance_info = {0};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  instance_info.enabledExtensionCount = num_instance_extensions;
  instance_info.ppEnabledExtensionNames = requested_extensions;
  instance_info.enabledLayerCount = 0;
  instance_info.ppEnabledLayerNames = requested_layers;
  instance_info.pNext = &debug_info;

#ifdef DEBUG
  uint32_t layer_count = 0;
  vkEnumerateInstanceLayerProperties(&layer_count, NULL);

  VkLayerProperties *layers =
      malloc(sizeof(VkLayerProperties) * layer_count);

  vkEnumerateInstanceLayerProperties(&layer_count, layers);

  for (uint32_t i = 0; i < layer_count; i++)
    printf("Layer: %s\n", layers[i].layerName);

  free(layers);
#endif

  vkcheck(vkCreateInstance(&instance_info, NULL, &engine->instance));
  volkLoadInstance(engine->instance);
  free(requested_extensions);

  /* FIX: the pNext debug_info above only covers instance create/destroy
   * messages. Create a real persistent messenger so validation errors
   * during normal rendering actually get printed. */
  // vkcheck(vkCreateDebugUtilsMessengerEXT(engine->instance, &debug_info, NULL, &engine->debug_messenger));
}

static void select_device(VK_Renderer *engine)
{
  printf("Selecting physical device\n");
  uint32_t device_count = 0;
  vkcheck(vkEnumeratePhysicalDevices(engine->instance, &device_count, NULL));
  engine->physical_devices = malloc(sizeof(VkPhysicalDevice) * device_count);
  engine->physdevice_count = device_count;
  engine->physdevice_index = 0;
  vkcheck(vkEnumeratePhysicalDevices(engine->instance, &device_count, engine->physical_devices));
}

static VkPhysicalDevice active_device(VK_Renderer *engine)
{
  return engine->physical_devices[engine->physdevice_index];
}

static void select_graphics_queue(VK_Renderer *engine)
{
  printf("Selecting graphics queue\n");
  uint32_t num_queue_families = 0;
  engine->queue_family_index = 0;

  vkGetPhysicalDeviceQueueFamilyProperties(
      active_device(engine),
      &num_queue_families,
      NULL);

  engine->queue_families = malloc(
      sizeof(VkQueueFamilyProperties) * num_queue_families);

  vkGetPhysicalDeviceQueueFamilyProperties(
      active_device(engine),
      &num_queue_families,
      engine->queue_families);

  for (uint32_t i = 0; i < num_queue_families; i++)
  {
    if (engine->queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
      engine->queue_family_index = i;
      break;
    }
  }

  if (!SDL_Vulkan_GetPresentationSupport(engine->instance, active_device(engine), engine->queue_family_index))
  {
    LOG_FATAL("Vulkan device does not support presentation?");
    exit(1);
  }
}

static void create_logical_device(VK_Renderer *engine)
{
  printf("Creating logical device\n");
  VkPhysicalDeviceVulkan13Features v13_features = {0};
  VkPhysicalDeviceVulkan12Features v12_features = {0};
  VkPhysicalDeviceFeatures v10_features = {0};

  const float priority = 1.0f;
  VkDeviceQueueCreateInfo queue_info = {0};
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.queueFamilyIndex = engine->queue_family_index;
  queue_info.pQueuePriorities = &priority;
  queue_info.queueCount = 1;

  v13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
  v13_features.dynamicRendering = true;
  v13_features.synchronization2 = true;
  v13_features.pNext = &v12_features;

  v12_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
  v12_features.descriptorIndexing = true;
  v12_features.shaderSampledImageArrayNonUniformIndexing = true;
  v12_features.descriptorBindingVariableDescriptorCount = true;
  v12_features.runtimeDescriptorArray = true;
  v12_features.bufferDeviceAddress = true;

  v10_features.samplerAnisotropy = VK_TRUE;

  const char *device_extensions = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

  VkDeviceCreateInfo device_info = {0};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = &v13_features;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.enabledExtensionCount = (uint32_t)1;
  device_info.ppEnabledExtensionNames = &device_extensions;
  device_info.pEnabledFeatures = &v10_features;

  vkcheck(vkCreateDevice(active_device(engine), &device_info, NULL, &engine->device));
  volkLoadDevice(engine->device);
  vkGetDeviceQueue(engine->device, engine->queue_family_index, 0, &engine->graphics_queue);
}

static void create_allocator(VK_Renderer *engine)
{
  printf("Creating vma allocator\n");
  VmaVulkanFunctions functions = {0};
  functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

  VmaAllocatorCreateInfo allocator_info = {0};

  allocator_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  allocator_info.physicalDevice = active_device(engine);
  allocator_info.device = engine->device;
  allocator_info.pVulkanFunctions = &functions;
  allocator_info.instance = engine->instance;

  vkcheck(vmaCreateAllocator(&allocator_info, &engine->allocator));
}

static void create_surface(VK_Renderer *engine, SDL_Window *window)
{
  printf("Creating vulkan surface\n");
  if (!SDL_Vulkan_CreateSurface(window, engine->instance, NULL, &engine->surface))
  {
    LOG_FATAL("Failed to create vulkan surface");
    exit(1);
  }


}

void create_swapchain(VK_Renderer *engine, SDL_Window *window)
{
  printf("Creating swapchain\n");
  VkExtent2D *swapchain_extent = &engine->swapchain_data.swapchain_extent;
  vkcheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      active_device(engine),
      engine->surface,
      &engine->surface_capabilities));


  *swapchain_extent = engine->surface_capabilities.currentExtent;
  if (swapchain_extent->width == 0xFFFFFFFF)
  {
    SDL_GetWindowSize(
        window,
        (int *)&swapchain_extent->width,
        (int *)&swapchain_extent->height);
  }

  const VkFormat image_format = VK_FORMAT_B8G8R8A8_SRGB;
  engine->swapchain_data.format = image_format; /* ADDED: stash for pipeline creation later */

  VkSwapchainCreateInfoKHR swapchain_info = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = engine->surface,
      .minImageCount = engine->surface_capabilities.minImageCount,
      .imageFormat = image_format,
      .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
      .imageExtent = {
          .width = swapchain_extent->width,
          .height = swapchain_extent->height},
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = VK_PRESENT_MODE_MAILBOX_KHR};

  vkcheck(vkCreateSwapchainKHR(engine->device, &swapchain_info, NULL, &engine->swapchain));

  SwapchainData *data = &engine->swapchain_data;
  data->image_count = 0;
  vkcheck(vkGetSwapchainImagesKHR(engine->device, engine->swapchain, &data->image_count, NULL));
  data->images = malloc(sizeof(VkImage) * data->image_count);
  vkcheck(vkGetSwapchainImagesKHR(engine->device, engine->swapchain, &data->image_count, data->images));
  data->images = realloc(data->images, sizeof(VkImage) * data->image_count);

  /* FIX/ADDED: create a view per swapchain image, required for
   * vkCmdBeginRendering color attachments since there's no render pass. */
  data->image_views = malloc(sizeof(VkImageView) * data->image_count);
  for (uint32_t i = 0; i < data->image_count; i++)
  {
    VkImageViewCreateInfo view_info = {0};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = data->images[i];
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = image_format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.layerCount = 1;
    vkcheck(vkCreateImageView(engine->device, &view_info, NULL, &data->image_views[i]));
  }



  // Create draw image
  engine->draw_image.extent.width = swapchain_extent->width;
  engine->draw_image.extent.height = swapchain_extent->height;
  engine->draw_image.extent.depth = 1;
  
  engine->draw_extent.width = engine->draw_image.extent.width * engine->draw_scale;
  engine->draw_extent.height = engine->draw_image.extent.height * engine->draw_scale;


  engine->draw_image.format = VK_FORMAT_R16G16B16A16_SFLOAT;
  VkImageUsageFlags usage_flags = 0;
  usage_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  usage_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  usage_flags |= VK_IMAGE_USAGE_STORAGE_BIT;
  usage_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  VkImageCreateInfo image_info = createinfo_image(
      engine->draw_image.format, 
      usage_flags, 
      engine->draw_image.extent);

  VmaAllocationCreateInfo alloc_info = {0};
  alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  vmaCreateImage(
      engine->allocator, 
      &image_info, 
      &alloc_info, 
      &engine->draw_image.image, 
      &engine->draw_image.allocation, 
      NULL);

  VkImageViewCreateInfo view_info = createinfo_imageview(
      engine->draw_image.format, 
      engine->draw_image.image, 
      VK_IMAGE_ASPECT_COLOR_BIT);

  vkcheck(vkCreateImageView(
        engine->device, 
        &view_info, 
        NULL, 
        &engine->draw_image.view));

}



static void create_depth_attachment(VK_Renderer *engine, SDL_Window *window)
{
  printf("Creating depth attachment\n");

  VkFormat depth_formats[2] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
  engine->depth_format = VK_FORMAT_UNDEFINED;

  for (int i = 0; i < 2; i++)
  {
    VkFormatProperties2 format_properties = {0};
    format_properties.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    vkGetPhysicalDeviceFormatProperties2(active_device(engine), depth_formats[i], &format_properties);
    if (format_properties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
      engine->depth_format = depth_formats[i];
      break;
    }
  }

  if (engine->depth_format == VK_FORMAT_UNDEFINED)
  {
    LOG_FATAL("No supported depth format found");
    exit(1);
  }

  int w_width, w_height;
  SDL_GetWindowSize(window, &w_width, &w_height);
  VkImageCreateInfo image_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = engine->depth_format,
      .extent = {
          .width = (uint32_t)w_width,
          .height = (uint32_t)w_height,
          .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationCreateInfo alloc_info = {0};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

  vkcheck(vmaCreateImage(engine->allocator, &image_info, &alloc_info, &engine->depth_image, &engine->depth_image_allocation, NULL));

  VkImageViewCreateInfo view_info = {0};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = engine->depth_image;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format = engine->depth_format;
  view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  view_info.subresourceRange.layerCount = 1;
  view_info.subresourceRange.levelCount = 1;

  vkcheck(vkCreateImageView(engine->device, &view_info, NULL, &engine->depth_image_view));
}

static void create_frame_data(VK_Renderer *engine)
{
  printf("Creating frames in flight (%d)\n", MAX_FRAMES_IN_FLIGHT);

  VkCommandPoolCreateInfo pool_info = {0};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = engine->queue_family_index;
  vkcheck(vkCreateCommandPool(engine->device, &pool_info, NULL, &engine->command_pool));

  VkCommandBufferAllocateInfo cmdbuffer_alloc_info = {0};
  cmdbuffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmdbuffer_alloc_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
  cmdbuffer_alloc_info.commandPool = engine->command_pool;
  vkcheck(vkAllocateCommandBuffers(engine->device, &cmdbuffer_alloc_info, engine->frames_in_flight.command_buffers));

  VkSemaphoreCreateInfo semaphore_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  VkFenceCreateInfo fence_info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT};

  engine->frames_in_flight.render_semaphores = malloc(sizeof(VkSemaphore) * engine->swapchain_data.image_count);
  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    VkBufferCreateInfo shbuffer_info = {0};
    shbuffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    shbuffer_info.size = sizeof(ShaderData);
    shbuffer_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VmaAllocationCreateInfo shbuffer_alloc_info = {0};
    shbuffer_alloc_info.flags =
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
        VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
        VMA_ALLOCATION_CREATE_MAPPED_BIT;
    shbuffer_alloc_info.usage = VMA_MEMORY_USAGE_AUTO;

    vkcheck(vmaCreateBuffer(
        engine->allocator,
        &shbuffer_info,
        &shbuffer_alloc_info,
        &engine->frames_in_flight.shader_data_buffers[i].buffer,
        &engine->frames_in_flight.shader_data_buffers[i].allocation,
        &engine->frames_in_flight.shader_data_buffers[i].allocation_info));

    VkBufferDeviceAddressInfo bda_info = {0};
    bda_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bda_info.buffer = engine->frames_in_flight.shader_data_buffers[i].buffer;
    engine->frames_in_flight.shader_data_buffers[i].address = vkGetBufferDeviceAddress(engine->device, &bda_info);

    vkcheck(vkCreateFence(engine->device, &fence_info, NULL, &engine->frames_in_flight.fence[i]));
    vkcheck(vkCreateSemaphore(engine->device, &semaphore_info, NULL, &engine->frames_in_flight.swapchain_semaphore[i]));
    // vkcheck(vkCreateSemaphore(engine->device, &semaphore_info, NULL, &engine->frames_in_flight.render_semaphore[i]));
  }
  for (uint32_t i = 0; i < engine->swapchain_data.image_count; i++)
  {
    vkcheck(vkCreateSemaphore(engine->device, &semaphore_info, NULL, &engine->frames_in_flight.render_semaphores[i]));
  }
}

void create_default_pipeline(VK_Renderer *engine)
{
  VKPipelineSet pipeline_set = {0};
  VKPipeline_clear(&pipeline_set);

  VKShader shader = VKShader_create(
      engine->device,
      "resource/shaders/spv/default.vert.spv",
      "resource/shaders/spv/default.frag.spv");

  VkPushConstantRange push_constant = {
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    .offset = 0,
    .size = sizeof(PushConstants)
  };

  VkPipelineLayoutCreateInfo layout_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &push_constant,
      .setLayoutCount = 0,
      .pSetLayouts = NULL

  };



  vkcheck(vkCreatePipelineLayout(
      engine->device,
      &layout_info,
      NULL,
      &engine->pipeline_layout));

  VKPipeline_set_shaders(
      &pipeline_set,
      shader.vertex_module,
      shader.fragment_module);

  VKPipeline_set_topology(
      &pipeline_set,
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

  VKPipeline_set_polygonmode(
      &pipeline_set,
      VK_POLYGON_MODE_FILL);

  VKPipeline_set_cull_mode(
      &pipeline_set,
      VK_CULL_MODE_BACK_BIT,
      VK_FRONT_FACE_COUNTER_CLOCKWISE);

  VKPipeline_enable_blending(&pipeline_set);

  pipeline_set.layout = engine->pipeline_layout;

  engine->pipeline = VKPipeline_build(engine, &pipeline_set);
}

uint8_t VK_Initialise(VK_Renderer *engine, SDL_Window *window)
{
  memset(engine, 0, sizeof(VK_Renderer));

  create_instance(engine);
  select_device(engine);
  select_graphics_queue(engine);
  create_logical_device(engine);
  create_allocator(engine);

  engine->window = window;
  engine->winresize_request = 0;
  engine->draw_scale =  1.0f;
  create_surface(engine, window);
  create_swapchain(engine, window);
  create_depth_attachment(engine, window);

  create_frame_data(engine);
  create_default_pipeline(engine);
  
  int w, h;
  SDL_GetWindowSize(window, &w, &h);

  engine->mesh_data = malloc(sizeof(VKMesh));
  VKMesh testmesh = VKMesh_load_gltf(engine, "resource/models/monkey.glb");
  *engine->mesh_data = testmesh;

  return 1;
}

