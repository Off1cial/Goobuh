#include <vulkan/vulkan.h>
#include "platform/window.hpp"
#include <vector>
#include <memory>
#include <vulkan/vulkan_core.h>
#include "core/common.h"

namespace VK
{

  struct Vertex
  {
    float pos[3];
    float normal[3];
    float col[4];
  };

  struct Mesh
  {
    // Where we start in the global vertex buffer
    VkDeviceSize vertex_offset;
    VkDeviceSize index_offset;
    // How we go in the buffer
    uint32_t vertex_count;
    uint32_t index_count;
  };

  enum class ShaderType
  {
    Vertex,
    Fragment,
    Geometry
  };

  class Shader
  {
  public:
    Shader(VkDevice vkdevice, const std::string &vertsrc, const std::string &fragsrc);
    ~Shader();

    FORCEINLINE VkShaderModule GetModule(const ShaderType type) const;
    FORCEINLINE VkShaderModule GetModule_Vertex() const { return m_vertmodule; }
    FORCEINLINE VkShaderModule GetModule_Fragment() const { return m_fragmodule; }

  private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkShaderModule m_vertmodule = VK_NULL_HANDLE;
    VkShaderModule m_fragmodule = VK_NULL_HANDLE;
  };

  class Pipeline
  {
  public:
    Pipeline(VkDevice device, const Shader &shader, VkFormat format);
    ~Pipeline();

    VkPipeline GetPipeline() const { return m_pipeline; }
    VkPipelineLayout GetPipelineLayout() const { return m_layout; }

  private:
    VkDevice m_device;

    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
  };

  enum class PresentMode
  {
    Immediate,
    Mailbox,
    VSyncFifo
  };

  class Renderer
  {
  public:
    Renderer(Plat::Window &window);
    ~Renderer();
    void Shutdown();

    void FrameStart();
    void FrameEnd();

  private:
    bool Init(Plat::Window &window);
    bool CreateSwapChain(Plat::Window &window);
    bool CreateCommandPool();
    bool CreateCommandBuffers();
    bool CreateSyncObjects();
    bool CreateVertexBuffer();

    // Create shaders -> create pipeline

    void TransitionImage(VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldlayout, VkImageLayout newlayout);

    uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);

    /*
    std::vector<char> PullShaderSource(const std::string& filename);
    VkShaderModule CreateShaderModule(const std::vector<char>& spv);
    VkShaderModule CreateShaderFromSource(const std::string& sourcefile);
    */
    // IMMEDIATE -> MAILBOX -> FIFO
    VkPresentModeKHR DeterminePresentMode(const std::vector<VkPresentModeKHR> &available, PresentMode preferred);

    VkPresentModeKHR GetVkPresentMode(PresentMode mode) const;

    VkInstance m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

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

    VkBuffer m_vertexbuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexmemory = VK_NULL_HANDLE;
    VkDeviceSize m_vertexbuffer_size = 1024 * 1024; // 1 MiB

    uint32_t m_current_image = 0;
    VkSemaphore m_image_available = VK_NULL_HANDLE;
    std::vector<VkSemaphore> m_render_finished{};
    VkFence m_in_flight = VK_NULL_HANDLE;

    // Currently bound shaders, make a shader wrapper to be handled by an asset manager?
    std::unique_ptr<Shader> m_shader;
    // HELLO RENDERER BRANCH
    std::vector<std::unique_ptr<Pipeline>> m_pipelines;
  };

};
