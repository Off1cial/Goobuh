#include <vulkan/vulkan.h>
#include "platform/window.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>
#include "core/common.h"


namespace VK
{
  enum class PresentMode
  {
    Immediate,
    Mailbox,
    VSyncFifo
  };

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
      bool CreateCommandPool();
      bool CreateCommandBuffers();
      bool CreateSyncObjects();


      void TransitionImage(VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldlayout, VkImageLayout newlayout);

      std::vector<char> PullShaderSource(const std::string& filename);
      VkShaderModule CreateShaderModule(const std::vector<char>& spv);
      VkShaderModule CreateShaderFromSource(const std::string& sourcefile);
      // IMMEDIATE -> MAILBOX -> FIFO
      VkPresentModeKHR DeterminePresentMode(const std::vector<VkPresentModeKHR>& available, PresentMode preferred);

      VkPresentModeKHR GetVkPresentMode(PresentMode mode) const;

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

      VkFormat m_swapchain_format = VK_FORMAT_UNDEFINED;
      VkExtent2D m_swapchain_extent{};

      VkCommandPool m_cmdpool = VK_NULL_HANDLE;
      std::vector<VkCommandBuffer> m_cmdbuffers{};

      uint32_t m_current_image = 0;
      VkSemaphore m_image_available = VK_NULL_HANDLE;
      std::vector<VkSemaphore> m_render_finished{};
      VkFence m_in_flight = VK_NULL_HANDLE;

      // Currently bound shaders, make a shader wrapper to be handled by an asset manager?
      VkShaderModule m_shader_vertex = VK_NULL_HANDLE;
      VkShaderModule m_shader_fragment = VK_NULL_HANDLE;
      // HELLO RENDERER BRANCH
  };

  enum class ShaderType
  {
    Vertex, Fragment, Geometry
  };

  class Shader
  {
    public:
      Shader(std::string& vertsrc, std::string& fragsrc);
      ~Shader();

      FORCEINLINE VkShaderModule GetModule(const ShaderType type) const;
      FORCEINLINE VkShaderModule GetModule_Vertex() const {return m_vertmodule;}
      FORCEINLINE VkShaderModule GetModule_Fragment() const {return m_fragmodule;}

    private:
      VkShaderModule m_vertmodule;
      VkShaderModule m_fragmodule;


      VkShaderModule CreateShaderModule(const std::vector<char>& spv);
      VkShaderModule CreateShaderFromSource(const std::string& sourcefile);

  };
};



