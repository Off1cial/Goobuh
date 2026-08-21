#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "renderer/vulkan/vk_vma.h"
#include "platform/window.hpp"
#include "core/common.h"
#include <vector>
#include <memory>
#include <span>
#include <iostream>



namespace VK
{
  static inline void VK_CHECK(VkResult result){
    if (result != VK_SUCCESS){
      std::cout << "Vulkan error: " << result << std::endl;
      exit(1);
    }
  }


  struct AllocatedBuffer
  {
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
  };

  struct MeshBuffers
  {
    AllocatedBuffer vertex_buffer;
    AllocatedBuffer index_buffer;
    VkDeviceAddress vertex_address;
  };

  struct MeshPushConstants
  {
    float model[16];
    VkDeviceAddress vertex_address;
  };

  struct Vertex
  {
    float pos[3];
    float normal[3];
    float col[4];
  };

  enum class ShaderType
  {
    Vertex,
    Fragment,
    Geometry
  };

  enum class MSAASampleCount
  {
    MSAA1  = 0x01,
    MSAA2  = 0x02,
    MSAA4  = 0x04,
    MSAA8  = 0x08,
    MSAA16 = 0x10,
    MSAA32 = 0x20,
    MSAA64 = 0x40
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


  struct Pipeline
  {
    VkDevice vk_device;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
  };

  class PipelineBuilder
  {
    public:
      std::vector<VkPipelineShaderStageCreateInfo> m_shaderstages;

      VkPipelineInputAssemblyStateCreateInfo m_inputAssembly;
      VkPipelineRasterizationStateCreateInfo m_rasterizer;
      VkPipelineColorBlendAttachmentState m_colorBlendAttachment;
      VkPipelineMultisampleStateCreateInfo m_multisampling;
      VkPipelineLayout m_pipelineLayout;
      VkPipelineDepthStencilStateCreateInfo m_depthStencil;
      VkPipelineRenderingCreateInfo m_renderInfo;
      VkFormat m_colorAttachmentformat;

      void Clear();
      PipelineBuilder(VkDevice device) : m_device(device) {Clear();}
      Pipeline BuildPipeline(VkDevice device);

      void SetShaderModules(const VkShaderModule vertex, const VkShaderModule fragment);
      void SetInputTopology(const VkPrimitiveTopology topology);
      void SetPolygonMode(const VkPolygonMode mode);
      void SetCullMode(const VkCullModeFlags flags, const VkFrontFace front);
      void SetMSAA(const VkPhysicalDevice physdevice, const VkSampleCountFlagBits samplecount); // Cap to hardware maximum
      void SetColorAttachmentFormat(const VkFormat format);
      void SetDepthFormat(const VkFormat format);
      void DisableBlending();
      void DisableDepthTest();
    
    private:
      VkPipelineShaderStageCreateInfo ShaderStageCreateInfo(const VkShaderStageFlagBits stage, const VkShaderModule module);
      VkDevice m_device;
  };

  /*
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
*/

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
    void CreateVmaAllocator();
    bool CreateVertexBuffer();

    MeshBuffers MeshUpload(std::span<uint32_t> indices, std::span<Vertex> verticess);
    Pipeline CreatePipeline(
      const Shader& shader,
      const VkPrimitiveTopology topology,
      const VkPolygonMode polygonmode,
      const VkCullModeFlags flags, 
      const VkFrontFace front,
      const MSAASampleCount msaa_samples,
      const VkFormat color_attachment_format,
      const VkFormat depth_format
    );
    // Create shaders -> create pipeline
    void TransitionImage(VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldlayout, VkImageLayout newlayout);
    uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);

    AllocatedBuffer CreateBuffer(const VmaMemoryUsage mem_usage, const VkBufferUsageFlags buff_usage, const size_t size);
    void DestroyBuffer(const AllocatedBuffer& buff);
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
    VkDeviceSize m_vertexbuffer_size = 1024 * 1024; // 1 MiB will change this, adding vma allocator

    
    VmaAllocator m_allocator = VK_NULL_HANDLE;
      
    uint32_t m_current_image = 0;
    VkSemaphore m_image_available = VK_NULL_HANDLE;
    std::vector<VkSemaphore> m_render_finished{};
    VkFence m_in_flight = VK_NULL_HANDLE;

    // Currently bound shaders, make a shader wrapper to be handled by an asset manager?
    std::unique_ptr<Shader> m_shader;
    // HELLO RENDERER BRANCH
    std::unique_ptr<PipelineBuilder> m_pipelinebuilder;
    std::vector<std::unique_ptr<Pipeline>> m_pipelines;
  };

};
