#ifndef VK_RENDERER_H
#define VK_RENDERER_H

//#include <vulkan/vulkan.h>
#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif

#include <volk/volk.h>

#include "volk/volk.h"
#include "renderer/vulkan/vk_vma.h"
#include "renderer/vulkan/vk_mesh.h"

#define MAX_FRAMES_IN_FLIGHT 2 

typedef struct SDL_Window SDL_Window;

typedef struct {
  VkBuffer buffer;
  VmaAllocation allocation;
  VmaAllocationInfo allocation_info;
  VkDeviceAddress address;
} ShaderDataBuffer;


typedef struct 
{
  VkFence fence[MAX_FRAMES_IN_FLIGHT];
  VkSemaphore swapchain_semaphore[MAX_FRAMES_IN_FLIGHT];

  VkSemaphore *render_semaphores;

  VkCommandBuffer command_buffers[MAX_FRAMES_IN_FLIGHT];
  ShaderDataBuffer shader_data_buffers[MAX_FRAMES_IN_FLIGHT];

} Frames;

typedef struct SwapchainData
{
  VkImageView* image_views;
  VkImage* images;
  VkFormat format;
  uint32_t image_count;
  VkExtent2D swapchain_extent;
} SwapchainData;  


typedef struct VK_Renderer
{
  VkInstance instance;
  VkPhysicalDevice* physical_devices;
  uint32_t physdevice_count;
  uint32_t physdevice_index;


  VkQueue graphics_queue;
  VkQueueFamilyProperties* queue_families;
  uint32_t queue_family_index;

  VkDevice device;
  VkSurfaceKHR surface;
  VkSurfaceCapabilitiesKHR surface_capabilities;

  VkSwapchainKHR swapchain;
  SwapchainData swapchain_data;

  VkImage depth_image;
  VkImageView depth_image_view;
  VkFormat depth_format;
  VmaAllocation depth_image_allocation;

  VkCommandPool command_pool;
  VmaAllocator allocator;


  VkPipeline pipeline;
  VkPipelineLayout pipeline_layout;

  Frames frames_in_flight; 
  uint32_t frame_count; // Number of total frames
  uint32_t frame_index; // Index of the frame to write to
  
  VKMesh* mesh_data;

} VK_Renderer;

uint8_t VK_Initialise(VK_Renderer* engine, SDL_Window* window);
void VK_Shutdown(VK_Renderer* engine);


typedef struct camera_t camera_t;
void VK_Draw(VK_Renderer* engine, camera_t* camera);


#endif
