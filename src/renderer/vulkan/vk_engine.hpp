#pragma once

#include "platform/window.hpp"
#include "renderer/vulkan/vk_vma.h"
#include "renderer/vulkan/vk_types.h"
#include "core/logsys.hpp"

#include <vector>
#include <array>
#include <vulkan/vulkan_core.h>


namespace VK
{
  class Engine
  {
    public:
      Engine(Plat::Window* window) : _window(window) {Init();}

    private:

      void InitDepthFormat();
      void CreateSwapchain();
      void Init();
  

      static constexpr uint8_t _max_frames_in_flight = 2;
      std::array<ShaderDataBuffer, _max_frames_in_flight> _shader_data_buffers;
      std::array<VkCommandBuffer, _max_frames_in_flight> _frame_command_buffers;
      std::array<VkFence, _max_frames_in_flight> _frame_fences;
      std::array<VkSemaphore, _max_frames_in_flight> _image_acquired_semaphores;
      VkCommandPool _command_pool = VK_NULL_HANDLE;
      std::vector<VkSemaphore> _render_complete_semaphores;

      MeshData LoadMesh_OBJ(const char* path);

      Plat::Window* _window = nullptr;

      VkInstance _instance = VK_NULL_HANDLE;
      std::vector<VkPhysicalDevice> _physdevices;
      uint32_t _physdevice_index = 0;
      VkDevice _device = VK_NULL_HANDLE;

      VmaAllocator _allocator = VK_NULL_HANDLE;

      std::vector<VkQueueFamilyProperties> _qfamilies;
      uint32_t _qfamily = 0;
      
      VkSurfaceKHR _surface = VK_NULL_HANDLE;
      VkSurfaceCapabilitiesKHR _surface_capabilities;

      VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
      std::vector<VkImage> _swapchain_images;
      VkFormat _depth_format = VK_FORMAT_UNDEFINED;
      VkImage _depth_image = VK_NULL_HANDLE;
      VkImageView _depth_image_view = VK_NULL_HANDLE;
      VmaAllocation _depth_image_allocation = VK_NULL_HANDLE;
  };
};




