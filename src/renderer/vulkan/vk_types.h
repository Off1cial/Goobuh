#pragma once

#include "renderer/interface/r_types.hpp"
#include "math/vector.hpp"
#include "math/matrix.hpp"
#include <vector>
#include "renderer/vulkan/vk_vma.h"

#define MAT_TRANSLATE 0
#define MAT_ROTATE 1
#define MAT_SCALE  2

namespace VK
{
  /*
  struct Vertex
  {
    Vector pos;
    Vector normal;
    Vector4 col;
    float uv[2];
  };*/

  struct MeshData
  {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    VmaAllocation allocation;
    VmaAllocationInfo allocation_info;
    VkBuffer buffer;
  };

  struct ShaderData
  {
    Mat4 projection;
    Mat4 view;
    Mat4 model[3];
  };

  struct ShaderDataBuffer
  {
    VmaAllocation allocation;
    VmaAllocationInfo allocation_info;
    VkBuffer buffer;
    VkDeviceAddress address;
  };

};
