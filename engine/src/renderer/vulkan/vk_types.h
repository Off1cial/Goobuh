#ifndef VK_TYPES_H
#define VK_TYPES_H
#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif

#include <volk/volk.h>

#include "renderer/vulkan/vk_vma.h"

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
} PushConstants;



VKShader VKShader_create(VkDevice device, const char*  vertexpath, const char* fragmentpath);


#endif
