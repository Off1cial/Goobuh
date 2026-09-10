#include "renderer/vulkan/vk_mesh_loader.h"
#include "renderer/vulkan/vk_mesh.h"
#include "renderer/vulkan/vk_types.h"

#include "tinyobj/tiny_obj_loader.h"

#include "fastgltf/core.hpp"
#include "fastgltf/base64.hpp"
#include "fastgltf/tools.hpp"
#include "fastgltf/types.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/util.hpp"





#include <vector>
#include <unordered_map>
#include <filesystem>
#include <cstring>
#include <cstdio>

struct VertexKey
{
  int position;
  int normal;
  int uv;

  bool operator==(const VertexKey& other) const
  {
    return position == other.position &&
           normal == other.normal &&
           uv == other.uv;
  }
};

struct VertexKeyHash
{
  size_t operator()(const VertexKey& key) const
  {
    size_t h = std::hash<int>{}(key.position);
    h ^= std::hash<int>{}(key.normal) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<int>{}(key.uv) + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
  }
};

static vertex_t make_obj_vertex(const tinyobj::attrib_t& attrib, const tinyobj::index_t& index)
{
  vertex_t vertex = {};

  if (index.vertex_index >= 0)
  {
    vertex.pos[0] = attrib.vertices[(size_t)index.vertex_index * 3 + 0];
    vertex.pos[1] = attrib.vertices[(size_t)index.vertex_index * 3 + 1];
    vertex.pos[2] = attrib.vertices[(size_t)index.vertex_index * 3 + 2];
  }

  if (index.normal_index >= 0)
  {
    vertex.normal[0] = attrib.normals[(size_t)index.normal_index * 3 + 0];
    vertex.normal[1] = attrib.normals[(size_t)index.normal_index * 3 + 1];
    vertex.normal[2] = attrib.normals[(size_t)index.normal_index * 3 + 2];
  }

  vertex.col[0] = 1.0f;
  vertex.col[1] = 1.0f;
  vertex.col[2] = 1.0f;
  vertex.col[3] = 1.0f;

  if (index.texcoord_index >= 0)
  {
    vertex.uv[0] = attrib.texcoords[(size_t)index.texcoord_index * 2 + 0];
    vertex.uv[1] = 1.0f - attrib.texcoords[(size_t)index.texcoord_index * 2 + 1];
  }

  return vertex;
}

VKMesh VKMesh_load_obj(VK_Renderer* engine, const char* path)
{
  VKMesh mesh = {};

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn;
  std::string err;

  bool loaded = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path, nullptr, true, true);

  if (!warn.empty())
    printf("OBJ warning: %s\n", warn.c_str());

  if (!err.empty())
    printf("OBJ error: %s\n", err.c_str());

  if (!loaded)
    return mesh;

  std::vector<vertex_t> vertices;
  std::vector<uint32_t> indices;
  std::unordered_map<VertexKey, uint32_t, VertexKeyHash> vertex_map;

  for (const tinyobj::shape_t& shape : shapes)
  {
    for (const tinyobj::index_t& index : shape.mesh.indices)
    {
      VertexKey key = {index.vertex_index, index.normal_index, index.texcoord_index};
      auto found = vertex_map.find(key);

      if (found != vertex_map.end())
      {
        indices.push_back(found->second);
        continue;
      }

      auto vertex_index = (uint32_t)vertices.size();

      vertices.push_back(make_obj_vertex(attrib, index));
      indices.push_back(vertex_index);
      vertex_map[key] = vertex_index;
    }
  }

  if (vertices.empty() || indices.empty())
    return mesh;

  return VKMesh_create(engine, vertices.data(), indices.data(), (uint32_t)vertices.size(), (uint32_t)indices.size());
}

static vertex_t make_gltf_vertex(const fastgltf::math::fvec3& position, const fastgltf::math::fvec3& normal, const fastgltf::math::fvec2& uv)
{
  vertex_t vertex = {};

  vertex.pos[0] = position[0];
  vertex.pos[1] = position[1];
  vertex.pos[2] = position[2];

  printf("Vertex loaded {%0.2f, %0.2f, %0.2f}\n", position[0], position[1], position[2]);

  vertex.normal[0] = normal[0];
  vertex.normal[1] = normal[1];
  vertex.normal[2] = normal[2];

  
  vertex.col[0] = 1.0f;
  vertex.col[1] = 1.0f;
  vertex.col[2] = 1.0f;
  vertex.col[3] = 1.0f;

  vertex.uv[0] = uv[0];
  vertex.uv[1] = uv[1];

  return vertex;
}

VKMesh VKMesh_load_gltf(VK_Renderer* engine, const char* path)
{
  VKMesh mesh = {};
  std::filesystem::path filepath(path);
  fastgltf::Parser parser;

  auto data = fastgltf::GltfDataBuffer::FromPath(filepath);

  if (!data)
  {
    printf("Failed to read glTF: %s\n", path);
    return mesh;
  }

  auto asset = parser.loadGltf(data.get(), filepath.parent_path(), fastgltf::Options::LoadExternalBuffers);

  if (!asset)
  {
    printf("Failed to parse glTF: %s\n", path);
    return mesh;
  }

  fastgltf::Asset& gltf = asset.get();

  std::vector<vertex_t> vertices;
  std::vector<uint32_t> indices;

  for (fastgltf::Mesh& gltf_mesh : gltf.meshes)
  {
    for (fastgltf::Primitive& primitive : gltf_mesh.primitives)
    {
      if (primitive.type != fastgltf::PrimitiveType::Triangles)
      {
        printf("Skipping non-triangle glTF primitive\n");
        continue;
      }

      auto position_attribute = primitive.findAttribute("POSITION");

      if (position_attribute == primitive.attributes.end())
      {
        printf("glTF primitive has no POSITION attribute\n");
        continue;
      }

      fastgltf::Accessor& position_accessor = gltf.accessors[position_attribute->accessorIndex];
      size_t vertex_start = vertices.size();

      vertices.resize(vertex_start + position_accessor.count);

      fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(gltf, position_accessor, [&](fastgltf::math::fvec3 position, size_t index)
      {
        vertices[vertex_start + index] = make_gltf_vertex(position, fastgltf::math::fvec3(), fastgltf::math::fvec2());
      });

      auto normal_attribute = primitive.findAttribute("NORMAL");

      if (normal_attribute != primitive.attributes.end())
      {
        fastgltf::Accessor& normal_accessor = gltf.accessors[normal_attribute->accessorIndex];

        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(gltf, normal_accessor, [&](fastgltf::math::fvec3 normal, size_t index)
        {
          vertices[vertex_start + index].normal[0] = normal[0];
          vertices[vertex_start + index].normal[1] = normal[1];
          vertices[vertex_start + index].normal[2] = normal[2];
        });
      }

      auto uv_attribute = primitive.findAttribute("TEXCOORD_0");

      if (uv_attribute != primitive.attributes.end())
      {
        fastgltf::Accessor& uv_accessor = gltf.accessors[uv_attribute->accessorIndex];

        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(gltf, uv_accessor, [&](fastgltf::math::fvec2 uv, size_t index)
        {
          vertices[vertex_start + index].uv[0] = uv[0];
          vertices[vertex_start + index].uv[1] = uv[1];
        });
      }

      if (primitive.indicesAccessor.has_value())
      {
        fastgltf::Accessor& index_accessor = gltf.accessors[primitive.indicesAccessor.value()];

        fastgltf::iterateAccessor<uint32_t>(gltf, index_accessor, [&](uint32_t index)
        {
          indices.push_back((uint32_t)vertex_start + index);
        });
      }
      else
      {
        for (uint32_t i = 0; i < position_accessor.count; i++)
          indices.push_back((uint32_t)vertex_start + i);
      }
    }
  }

  if (vertices.empty() || indices.empty())
    return mesh;

  return VKMesh_create(engine, vertices.data(), indices.data(), (uint32_t)vertices.size(), (uint32_t)indices.size());
}
