#include "renderer/vulkan/vk_types.h"
#include "renderer/vulkan/vk_engine.hpp"
#include "tinyobj/tiny_obj_loader.h"

#include <vector>


using namespace VK;


MeshData VKRenderer::LoadMesh_OBJ(const char* path)
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, nullptr, nullptr, path)){
    return {};
  }
  
  MeshData data{};
  
  for (tinyobj::index_t index : shapes[0].mesh.indices){
    Vertex v;
    int v_index = index.vertex_index; 
    int n_index = index.normal_index;
    int u_index = index.texcoord_index;
    v.pos.x = attrib.vertices[(size_t)(v_index * 3)];
    v.pos.y = -attrib.vertices[(size_t)(v_index * 3 + 1)];
    v.pos.z = attrib.vertices[(size_t)(v_index * 3 + 2)];

    v.normal.x = attrib.vertices[(size_t)(n_index * 3)];
    v.normal.y = -attrib.vertices[(size_t)(n_index * 3 + 1)];
    v.normal.z = attrib.vertices[(size_t)(n_index * 3 + 2)];

    v.pos.x = attrib.vertices[(size_t)(u_index * 2)];
    v.pos.y = 1.0f - attrib.vertices[(size_t)(u_index * 2 + 1)];

    v.col.x = v.col.y = v.col.z = v.col.w = 1;
    data.vertices.push_back(v);
    data.indices.push_back((uint32_t)data.indices.size());
  }

  VkDeviceSize v_buff_size = sizeof(Vertex) * data.vertices.size();
  VkDeviceSize i_buff_size = sizeof(uint32_t) * data.indices.size();

  VkBufferCreateInfo buff_info{};
  buff_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buff_info.size = v_buff_size + i_buff_size;
  buff_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

  VmaAllocationCreateInfo alloc_create_info{};
  alloc_create_info.flags = 
    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | 
    VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | 
    VMA_ALLOCATION_CREATE_MAPPED_BIT;
  alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO; 



  VkResult result = vmaCreateBuffer(_allocator, &buff_info, &alloc_create_info, &data.buffer, &data.allocation, &data.allocation_info);
  if (result != VK_SUCCESS){
    LOG_ERROR("Failed to load OBJ file %s", path);
    return {};
  }

  memcpy(data.allocation_info.pMappedData, data.vertices.data(), v_buff_size);
  memcpy((char*)(data.allocation_info.pMappedData) + v_buff_size, data.indices.data(), i_buff_size);
  
  return data;
}




