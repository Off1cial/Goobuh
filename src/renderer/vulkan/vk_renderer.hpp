#include <vulkan/vulkan.h>
#include "platform/window.hpp"

namespace VK
{
  class Renderer
  {
    public:
      Renderer(Plat::Window& window);
      ~Renderer();
      bool Init(Plat::Window& window);
      void Shutdown();

      void FrameStart();
      void FrameEnd();

    private:
      VkInstance m_instance = VK_NULL_HANDLE;
      VkSurfaceKHR m_surface =  VK_NULL_HANDLE;

      VkPhysicalDevice m_physdevice = VK_NULL_HANDLE;
      VkDevice m_device = VK_NULL_HANDLE;

      VkQueue m_graphqueue = VK_NULL_HANDLE;
      VkQueue m_presentqueue = VK_NULL_HANDLE;

      VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
  };
};



