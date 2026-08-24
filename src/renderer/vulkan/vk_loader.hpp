#pragma once
#include "core/common.h"
#include "renderer/vulkan/vk_types.hpp"
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <string>
#include <filesystem>

namespace VK
{
  struct Surface
  {
    u32 start_index;
    u32 count;
  };

  struct MeshAsset
  {
    std::string name;

    std::vector<Surface> surfaces;
    MeshBuffers mesh_buffers;
  };
  class Renderer;

  std::optional<std::vector<std::shared_ptr<MeshAsset>>>
  loadGltfMeshes(
      Renderer *renderer,
      std::filesystem::path filepath);

};
