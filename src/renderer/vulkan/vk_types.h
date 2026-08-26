#pragma once

#include "math/vector.hpp"
#include <vector>
#include "renderer/vulkan/vk_vma.h"

namespace VK
{
  struct Vertex
  {
    Vector pos;
    Vector normal;
    Vector4 colour;
    float uv[2];
  };

  struct MeshData
  {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    VmaAllocation allocation;
    VmaAllocationInfo allocation_info;
    VkBuffer buffer;
  };

};
