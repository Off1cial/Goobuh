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
#include <cfloat>

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

VKMesh VKMesh_load_obj(VK_Renderer* engine, const char* path, vec3_t halfs_out)
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

  //printf("Vertex loaded {%0.2f, %0.2f, %0.2f}\n", position[0], position[1], position[2]);

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
/*
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

*/

/*

VKMesh VKMesh_load_gltf( VK_Renderer* engine, const char* path, vec3_t halfs_out )
{
    VKMesh mesh = {};
    std::filesystem::path filepath( path );
    fastgltf::Parser parser;


    auto data = fastgltf::GltfDataBuffer::FromPath( filepath );

    if (!data)
    {
        printf( "Failed to read glTF: %s\n", path );
        return mesh;
    }

    auto asset = parser.loadGltf( data.get(), filepath.parent_path(), fastgltf::Options::LoadExternalBuffers );

    if (!asset)
    {
        printf( "Failed to parse glTF: %s\n", path );
        return mesh;
    }

    fastgltf::Asset& gltf = asset.get();

    std::vector<vertex_t> vertices;
    std::vector<uint32_t> indices;

    vec3_t bounds_min = {
        FLT_MAX,
        FLT_MAX,
        FLT_MAX
    };

    vec3_t bounds_max = {
        -FLT_MAX,
        -FLT_MAX,
        -FLT_MAX
    };

    for (fastgltf::Mesh& gltf_mesh : gltf.meshes)
    {
        for (fastgltf::Primitive& primitive : gltf_mesh.primitives)
        {
            if (primitive.type != fastgltf::PrimitiveType::Triangles)
            {
                printf( "Skipping non-triangle glTF primitive\n" );
                continue;
            }

            auto position_attribute = primitive.findAttribute( "POSITION" );

            if (position_attribute == primitive.attributes.end())
            {
                printf( "glTF primitive has no POSITION attribute\n" );
                continue;
            }

            fastgltf::Accessor& position_accessor = gltf.accessors[position_attribute->accessorIndex];
            size_t vertex_start = vertices.size();

            vertices.resize( vertex_start + position_accessor.count );

            fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>( gltf, position_accessor, [&]( fastgltf::math::fvec3 position, size_t index )
                                                                       {
                                                                           vertices[vertex_start + index] = make_gltf_vertex( position, fastgltf::math::fvec3(), fastgltf::math::fvec2() );

                                                                           for (int axis = 0; axis < 3; ++axis)
                                                                           {
                                                                               bounds_min[axis] = fminf( bounds_min[axis], position[axis] );
                                                                               bounds_max[axis] = fmaxf( bounds_max[axis], position[axis] );
                                                                           }
                                                                       } );

            auto normal_attribute = primitive.findAttribute( "NORMAL" );

            if (normal_attribute != primitive.attributes.end())
            {
                fastgltf::Accessor& normal_accessor = gltf.accessors[normal_attribute->accessorIndex];

                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>( gltf, normal_accessor, [&]( fastgltf::math::fvec3 normal, size_t index )
                                                                           {
                                                                               vertices[vertex_start + index].normal[0] = normal[0];
                                                                               vertices[vertex_start + index].normal[1] = normal[1];
                                                                               vertices[vertex_start + index].normal[2] = normal[2];
                                                                           } );
            }

            auto uv_attribute = primitive.findAttribute( "TEXCOORD_0" );

            if (uv_attribute != primitive.attributes.end())
            {
                fastgltf::Accessor& uv_accessor = gltf.accessors[uv_attribute->accessorIndex];

                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>( gltf, uv_accessor, [&]( fastgltf::math::fvec2 uv, size_t index )
                                                                           {
                                                                               vertices[vertex_start + index].uv[0] = uv[0];
                                                                               vertices[vertex_start + index].uv[1] = uv[1];
                                                                           } );
            }

            if (primitive.indicesAccessor.has_value())
            {
                fastgltf::Accessor& index_accessor = gltf.accessors[primitive.indicesAccessor.value()];

                fastgltf::iterateAccessor<uint32_t>( gltf, index_accessor, [&]( uint32_t index )
                                                     {
                                                         indices.push_back( (uint32_t)vertex_start + index );
                                                     } );
            }
            else
            {
                for (uint32_t i = 0; i < position_accessor.count; i++)
                    indices.push_back( (uint32_t)vertex_start + i );
            }
        }
    }

    if (vertices.empty() || indices.empty())
        return mesh;

    halfs_out[0] = (bounds_max[0] - bounds_min[0]) * 0.5f;
    halfs_out[1] = (bounds_max[1] - bounds_min[1]) * 0.5f;
    halfs_out[2] = (bounds_max[2] - bounds_min[2]) * 0.5f;

    return VKMesh_create( engine, vertices.data(), indices.data(), (uint32_t)vertices.size(), (uint32_t)indices.size() );
}
*/


VKMesh VKMesh_load_gltf( VK_Renderer* engine, const char* path, vec3_t halfs_out )
{
    VKMesh mesh = {};
    std::filesystem::path filepath( path );

    constexpr auto extensions =
        fastgltf::Extensions::KHR_mesh_quantization |
        fastgltf::Extensions::EXT_meshopt_compression |
        fastgltf::Extensions::KHR_texture_transform |
        fastgltf::Extensions::KHR_materials_emissive_strength;

    fastgltf::Parser parser( extensions );

    auto data = fastgltf::GltfDataBuffer::FromPath( filepath );
    if (data.error() != fastgltf::Error::None)
    {
        printf( "Failed to read glTF %s: %s\n", path,
                fastgltf::getErrorMessage( data.error() ).data() );
        return mesh;
    }

    constexpr auto options =
        fastgltf::Options::LoadExternalBuffers |
        fastgltf::Options::DecomposeNodeMatrices; // not required, harmless

    auto asset = parser.loadGltf( data.get(), filepath.parent_path(), options );
    if (asset.error() != fastgltf::Error::None)
    {
        printf( "Failed to parse glTF %s: %s\n", path,
                fastgltf::getErrorMessage( asset.error() ).data() );
        return mesh;
    }

    fastgltf::Asset& gltf = asset.get();

    std::vector<vertex_t> vertices;
    std::vector<uint32_t> indices;

    vec3_t bounds_min = { FLT_MAX, FLT_MAX, FLT_MAX };
    vec3_t bounds_max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    auto add_mesh = [&]( size_t mesh_index, const fastgltf::math::fmat4x4& m )
        {
            // Normal matrix = transpose(inverse(M)); winding flips if det < 0
            fastgltf::math::fmat4x4 inv = fastgltf::math::invert( m );

            float det =
                m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) -
                m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2]) +
                m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]);
            bool flip = det < 0.0f;

            for (fastgltf::Primitive& primitive : gltf.meshes[mesh_index].primitives)
            {
                if (primitive.type != fastgltf::PrimitiveType::Triangles)
                {
                    printf( "Skipping non-triangle glTF primitive\n" );
                    continue;
                }

                auto pos_attr = primitive.findAttribute( "POSITION" );
                if (pos_attr == primitive.attributes.end())
                    continue;

                fastgltf::Accessor& pos_acc = gltf.accessors[pos_attr->accessorIndex];
                size_t base = vertices.size();
                vertices.resize( base + pos_acc.count );

                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>( gltf, pos_acc,
                                                                           [&]( fastgltf::math::fvec3 p, size_t i )
                                                                           {
                                                                               float w[3];
                                                                               for (int r = 0; r < 3; ++r)
                                                                                   w[r] = m[0][r] * p[0] + m[1][r] * p[1] + m[2][r] * p[2] + m[3][r];

                                                                               vertices[base + i] = make_gltf_vertex(
                                                                                   fastgltf::math::fvec3( w[0], w[1], w[2] ),
                                                                                   fastgltf::math::fvec3( 0.f, 0.f, 1.f ),
                                                                                   fastgltf::math::fvec2() );

                                                                               for (int a = 0; a < 3; ++a)
                                                                               {
                                                                                   bounds_min[a] = fminf( bounds_min[a], w[a] );
                                                                                   bounds_max[a] = fmaxf( bounds_max[a], w[a] );
                                                                               }
                                                                           } );

                auto nrm_attr = primitive.findAttribute( "NORMAL" );
                if (nrm_attr != primitive.attributes.end())
                {
                    fastgltf::Accessor& acc = gltf.accessors[nrm_attr->accessorIndex];
                    fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>( gltf, acc,
                                                                               [&]( fastgltf::math::fvec3 n, size_t i )
                                                                               {
                                                                                   float o[3];
                                                                                   for (int r = 0; r < 3; ++r)
                                                                                       o[r] = inv[r][0] * n[0] + inv[r][1] * n[1] + inv[r][2] * n[2];

                                                                                   float len = sqrtf( o[0] * o[0] + o[1] * o[1] + o[2] * o[2] );
                                                                                   if (len > 0.0f) { o[0] /= len; o[1] /= len; o[2] /= len; }

                                                                                   vertices[base + i].normal[0] = o[0];
                                                                                   vertices[base + i].normal[1] = o[1];
                                                                                   vertices[base + i].normal[2] = o[2];
                                                                               } );
                }

                auto uv_attr = primitive.findAttribute( "TEXCOORD_0" );
                if (uv_attr != primitive.attributes.end())
                {
                    fastgltf::Accessor& acc = gltf.accessors[uv_attr->accessorIndex];
                    fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>( gltf, acc,
                                                                               [&]( fastgltf::math::fvec2 uv, size_t i )
                                                                               {
                                                                                   vertices[base + i].uv[0] = uv[0];
                                                                                   vertices[base + i].uv[1] = uv[1];
                                                                               } );
                }

                size_t idx_start = indices.size();

                if (primitive.indicesAccessor.has_value())
                {
                    fastgltf::Accessor& acc = gltf.accessors[primitive.indicesAccessor.value()];
                    fastgltf::iterateAccessor<uint32_t>( gltf, acc,
                                                         [&]( uint32_t idx ) { indices.push_back( (uint32_t)base + idx ); } );
                }
                else
                {
                    for (uint32_t i = 0; i < pos_acc.count; ++i)
                        indices.push_back( (uint32_t)base + i );
                }

                if (flip)
                    for (size_t i = idx_start; i + 2 < indices.size(); i += 3)
                        std::swap( indices[i + 1], indices[i + 2] );
            }
        };

        // Walk the scene graph so every node instance gets its world transform
    size_t scene_index = gltf.defaultScene.value_or( 0 );
    if (scene_index < gltf.scenes.size())
    {
        fastgltf::iterateSceneNodes( gltf, scene_index, fastgltf::math::fmat4x4(),
                                     [&]( fastgltf::Node& node, fastgltf::math::fmat4x4 matrix )
                                     {
                                         if (node.meshIndex.has_value())
                                             add_mesh( node.meshIndex.value(), matrix );
                                     } );
    }
    else
    {
        // No scenes: fall back to raw meshes
        for (size_t i = 0; i < gltf.meshes.size(); ++i)
            add_mesh( i, fastgltf::math::fmat4x4() );
    }

    if (vertices.empty() || indices.empty())
    {
        printf( "glTF %s produced no geometry\n", path );
        return mesh;
    }

    for (int a = 0; a < 3; ++a)
        halfs_out[a] = (bounds_max[a] - bounds_min[a]) * 0.5f;

    return VKMesh_create( engine, vertices.data(), indices.data(),
                          (uint32_t)vertices.size(), (uint32_t)indices.size() );
}