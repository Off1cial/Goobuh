#include "renderer/vulkan/vk_mesh.h"
#include "renderer/vulkan/vk_renderer.h"
#include "common/logsys.h"

#include "math/matrix.h"

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
      .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
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

/*
void VKMesh_draw(VkDevice device, VkPipelineLayout pipeline_layout, VkCommandBuffer cmd, VkPipeline pipeline, VKMesh *mesh_data)
{
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  VkDeviceSize offset = 0;

  vkCmdBindVertexBuffers(cmd, 0, 1, &mesh_data->vertex_buffer, &offset);
  //vkCmdBindIndexBuffer(cmd, mesh_data->index_buffer, 0, VK_INDEX_TYPE_UINT32);

  if (mesh_data->index_count){
    vkCmdBindIndexBuffer(cmd, mesh_data->index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmd, mesh_data->index_count, 1, 0, 0, 0);
  }
  else{
    vkCmdDraw(cmd, mesh_data->vertex_count, 1, 0, 0);
  }
}
*/

void VKMesh_draw(VkDevice device, VkPipelineLayout pipeline_layout, VkCommandBuffer cmd, VkPipeline pipeline, VKMesh *mesh_data, PushConstants* push_constants)
{
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  VkBufferDeviceAddressInfo address_info = {0};
  address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
  address_info.buffer = mesh_data->vertex_buffer;

  push_constants->vertex_addr = vkGetBufferDeviceAddress(device, &address_info);

  vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), push_constants);

  if (mesh_data->index_count) {
    vkCmdBindIndexBuffer(cmd, mesh_data->index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmd, mesh_data->index_count, 1, 0, 0, 0);
  }
  else {
    vkCmdDraw(cmd, mesh_data->vertex_count, 1, 0, 0);
  }
}
