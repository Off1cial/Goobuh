#pragma once

#include "renderer/vulkan/vk_mesh.h"

#ifdef __cplusplus
extern "C" {
#endif

VKMesh VKMesh_load_obj(VK_Renderer* engine, const char* path);
VKMesh VKMesh_load_gltf(VK_Renderer* engine, const char* path);

#ifdef __cplusplus
}
#endif
