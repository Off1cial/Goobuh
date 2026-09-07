#pragma once

#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif

#include <volk/volk.h>

#include "renderer/vulkan/vk_types.h"
#include "renderer/vulkan/vk_vma.h"


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