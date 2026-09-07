#pragma once

#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif

#include <volk/volk.h>
#include "renderer/vulkan/vk_types.h"
#include "renderer/vulkan/vk_vma.h"
#include "math/matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VKMesh
{
  VkBuffer vertex_buffer;
  VmaAllocation vertex_allocation;

  VkBuffer index_buffer;
  VmaAllocation index_allocation;

  uint32_t vertex_count;
  uint32_t index_count;
  
} VKMesh;


typedef struct VK_Renderer VK_Renderer;

VKMesh VKMesh_create(VK_Renderer* engine, const vertex_t* vertices, const uint32_t* indices, const uint32_t vertex_count, const uint32_t index_count);


void   VKMesh_draw(VkDevice device, VkPipelineLayout pipeline_layout, VkCommandBuffer buffer, VkPipeline pipeline, VKMesh* mesh_data, mat4 proj, mat4 view, mat4 model);


#ifdef __cplusplus
}
#endif
