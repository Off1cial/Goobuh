#ifndef VK_TYPES_H
#define VK_TYPES_H
#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif

#include <volk/volk.h>

#include "renderer/vulkan/vk_vma.h"

typedef struct VK_Renderer VK_Renderer;

typedef struct
{
  float pos[3];
  float _pad0;

  float normal[3];
  float _pad1;

  float col[4];

  float uv[2];
  float _pad2[2];
} vertex_t;

typedef struct 
{
  float projection[16];
  float view[16];
  float model[16];
} ShaderData;

typedef struct 
{
  VkShaderModule vertex_module;
  VkShaderModule fragment_module;
  VkDevice device;
} VKShader;

typedef struct 
{
  VkImage image;
  VkImageView view;
  VmaAllocation allocation;
  VkExtent3D extent;
  VkFormat format;
  uint32_t mip_levels;
} VKTexture;


typedef struct VertexBuffer
{
  VkBuffer buffer;
  VmaAllocation allocation;
} VertexBuffer;

typedef struct PushConstants
{
  float projection[16];
  float view[16];
  float model[16];
  VkDeviceAddress vertex_addr;
  uint32_t tex_id;
} PushConstants;

#ifdef __cplusplus
extern "C" {
#endif

VKShader VKShader_create(VkDevice device, const char*  vertexpath, const char* fragmentpath);
VKTexture VKTexture_create(VK_Renderer* engine, const void* pixels, uint32_t w, uint32_t h, VkFormat format );

VKTexture* VKTexture_CreateFromFile( VK_Renderer* engine, const char* path, int* index_out );

void VKTexture_destroy(VK_Renderer* engine, VKTexture* tex);
void VKTexture_register(VK_Renderer* engine, VKTexture* tex, uint32_t index);

#ifdef __cplusplus
}
#endif
#endif
