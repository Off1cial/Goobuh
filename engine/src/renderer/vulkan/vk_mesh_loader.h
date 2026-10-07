#pragma once

#include "renderer/vulkan/vk_mesh.h"
#include "math/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

VKMesh VKMesh_load_obj(VK_Renderer* engine, const char* path, vec3_t halfs_out );
VKMesh VKMesh_load_gltf(VK_Renderer* engine, const char* path, vec3_t halfs_out );

#ifdef __cplusplus
}
#endif
