#include <vulkan/vulkan.h>
#include "platform/window.hpp"
#include <vector>

namespace VK
{
  class Renderer
  {
    public:
      Renderer(Plat::Window& window);
      ~Renderer();
      void Shutdown();

      void FrameStart();
      void FrameEnd();

    private:
      bool Init(Plat::Window& window);
      bool CreateSwapChain(Plat::Window& window);

      VkInstance m_instance = VK_NULL_HANDLE;
      VkSurfaceKHR m_surface =  VK_NULL_HANDLE;

      VkPhysicalDevice m_physdevice = VK_NULL_HANDLE;
      VkDevice m_device = VK_NULL_HANDLE;

      VkQueue m_graphqueue = VK_NULL_HANDLE;
      VkQueue m_presentqueue = VK_NULL_HANDLE;

      uint32_t m_graphqueue_index = UINT32_MAX;
      uint32_t m_presentqueue_index = UINT32_MAX;

      
      VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
      std::vector<VkImage> m_swapchain_images;
      std::vector<VkImageView> m_swapchain_imageviews;

      VkFormat m_swapchain_format;
      VkExtent2D m_swapchain_extent;
  };
};



