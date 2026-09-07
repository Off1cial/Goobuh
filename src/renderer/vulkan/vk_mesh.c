#include "renderer/vulkan/vk_mesh.h"
#include "renderer/vulkan/vk_renderer.h"
#include "common/logsys.h"


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static inline void vkcheck(VkResult result)
{
  if (result != VK_SUCCESS)
  {
    LOG_FATAL("Vulkan check failed");
    exit(1);
  }
}

VKMesh VKMesh_create(
    VK_Renderer *engine,
    const vertex_t *vertices,
    const uint32_t *indices,
    const uint32_t vertex_count,
    const uint32_t index_count)
{
  VKMesh mesh = {0};

  VkBufferCreateInfo buffer_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = sizeof(vertex_t) * vertex_count,
      .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };

  VmaAllocationCreateInfo allocation_info = {
      .usage = VMA_MEMORY_USAGE_AUTO,
      .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
               VMA_ALLOCATION_CREATE_MAPPED_BIT,
  };

  VmaAllocationInfo allocation_result;

  vkcheck(vmaCreateBuffer(
      engine->allocator,
      &buffer_info,
      &allocation_info,
      &mesh.vertex_buffer,
      &mesh.vertex_allocation,
      &allocation_result));

  memcpy(
      allocation_result.pMappedData,
      vertices,
      sizeof(vertex_t) * vertex_count);

  vmaFlushAllocation(
      engine->allocator,
      mesh.vertex_allocation,
      0,
      sizeof(vertex_t) * vertex_count);

  mesh.vertex_count = vertex_count;

  if (indices && index_count)
  {
    buffer_info.size = sizeof(uint32_t) * index_count;
    buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    allocation_result = (VmaAllocationInfo){0};

    vkcheck(vmaCreateBuffer(
        engine->allocator,
        &buffer_info,
        &allocation_info,
        &mesh.index_buffer,
        &mesh.index_allocation,
        &allocation_result));

    memcpy(
        allocation_result.pMappedData,
        indices,
        sizeof(uint32_t) * index_count);

    vmaFlushAllocation(
        engine->allocator,
        mesh.index_allocation,
        0,
        sizeof(uint32_t) * index_count);

    mesh.index_count = index_count;
  }

  return mesh;
}
