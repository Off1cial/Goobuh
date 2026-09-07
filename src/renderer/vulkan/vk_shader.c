#include "volk/volk.h"
#include "renderer/vulkan/vk_types.h"
#include "common/logsys.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint32_t *pull_contents(
    const char *filename,
    size_t *size)
{
    FILE *fptr = fopen(filename, "rb");

    if (!fptr)
    {
        LOG_ERROR("Failed to open shader: %s\n", filename);
        return NULL;
    }

    if (fseek(fptr, 0, SEEK_END) != 0)
    {
        fclose(fptr);
        return NULL;
    }

    long file_size = ftell(fptr);

    if (file_size < 0)
    {
        fclose(fptr);
        return NULL;
    }

    rewind(fptr);

    if (file_size == 0 || file_size % sizeof(uint32_t) != 0)
    {
        LOG_ERROR("Invalid SPIR-V file: %s\n", filename);
        fclose(fptr);
        return NULL;
    }

    uint32_t *data = malloc((size_t)file_size);

    if (!data)
    {
        LOG_ERROR("Failed to allocate shader: %s\n", filename);
        fclose(fptr);
        return NULL;
    }

    size_t bytes_read = fread(
        data,
        1,
        (size_t)file_size,
        fptr);

    fclose(fptr);

    if (bytes_read != (size_t)file_size)
    {
        LOG_ERROR("Failed to read shader: %s\n", filename);
        free(data);
        return NULL;
    }

    *size = (size_t)file_size;
    return data;
}

static VkShaderModule CreateShaderModule(
    VkDevice device,
    const uint32_t *code,
    size_t code_size)
{
    VkShaderModuleCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code_size,
        .pCode = code,
    };

    VkShaderModule module = VK_NULL_HANDLE;

    if (vkCreateShaderModule(
            device,
            &info,
            NULL,
            &module) != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create shader module");
        return VK_NULL_HANDLE;
    }

    return module;
}

VKShader VKShader_create(
    VkDevice device,
    const char *vertexpath,
    const char *fragmentpath)
{
    VKShader shader = {0};

    size_t vertex_size = 0;
    size_t fragment_size = 0;

    uint32_t *vertex_code =
        pull_contents(vertexpath, &vertex_size);

    uint32_t *fragment_code =
        pull_contents(fragmentpath, &fragment_size);

    if (!vertex_code || !fragment_code)
    {
        free(vertex_code);
        free(fragment_code);
        return shader;
    }

    shader.vertex_module =
        CreateShaderModule(
            device,
            vertex_code,
            vertex_size);

    shader.fragment_module =
        CreateShaderModule(
            device,
            fragment_code,
            fragment_size);

    shader.device = device;

    free(vertex_code);
    free(fragment_code);

    return shader;
}
