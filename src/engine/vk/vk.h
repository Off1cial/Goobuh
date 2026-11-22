#ifndef __VK_H__
#define __VK_H__

#include <array>

#include <SDL3/SDL.h>
#include <vulkan/vulkan.hpp>
#include <VkBootstrap.h>

#include "engine/vk/vk_buff.h"


class CVulkanSystem
{
public:
    CVulkanSystem() {};
    CVulkanSystem( const char* title, int w, int h) { Init( title, w, h );}
    ~CVulkanSystem() { Shutdown(); }

    // Prevent copying
    CVulkanSystem( CVulkanSystem& ) = delete;
    CVulkanSystem& operator=( CVulkanSystem& ) = delete;
    

    bool Init( const char *title, int w, int h );

    void Shutdown();

private:
    

    SDL_Window* m_window =  nullptr;

    struct 
    {
        vkb::Instance instance{};
        vkb::Swapchain swapchain{};
        vkb::Device device{};
    } m_vkbData;


    struct DescriptorHeapResources
    {
        struct DescriptorHeapData
        {
            Rc<CVKBuff> buff;
            VkDeviceSize bindOffset = 0; // Device alignment offset
            VkDeviceSize rangeSize = 0; // Buffer size
            VkDeviceSize alignment = 0; // Alignment requirement of the device
            VkDeviceSize minReservationSize = 0;
        };

        // Host visible
        DescriptorHeapData resourceHeap{};
        DescriptorHeapData samplerHeap{};

        // Descriptor sizes, queried from device
        VkDeviceSize sampledImageSize = 0;
        VkDeviceSize samplerSize = 0;
        VkDeviceSize uniformBufferSize = 0;
        VkDeviceSize storageBufferSize = 0;
        VkDeviceSize imageDescriptorAlignment = 0;
        VkDeviceSize bufferDescriptorAlignment = 0;
        VkDeviceSize samplerDescriptorAlignment = 0;
        VkDeviceSize maxPushSize = 0;

        uint32 uniformBufferStride = 0;
        uint32 sampledImageStride = 0;

        // Byte offsets for the descriptors we may use
        uint32 cameraOffset = 0;
        // ...
    };
    DescriptorHeapResources m_descriptorHeaps{};

    vk::SurfaceKHR m_surface;
    vk::Device m_logicalDevice;

    vk::Queue m_graphicsQueue{};
    vk::Queue m_presentQueue{};

    std::vector<vk::MemoryPropertyFlags> m_memoryTypeFlags{};

    struct ShaderGroup
    {
        vk::UniqueShaderEXT vertexShader{};
        vk::UniqueShaderEXT fragmentShader{};
    };

    struct ShaderObjects
    {
        ShaderGroup shDefaultUnlit; // Albedo, colours
        ShaderGroup shDefaultLit; // Albedo, normals, lighting, colours 
    };

    struct ShaderBinaryCacheProperties
    {
        std::array<uint8_t, VK_UUID_SIZE> shaderBinaryUUID{};
        uint32 shaderBinaryVersion;
    };

    ShaderObjects m_shaderObjects;
    ShaderBinaryCacheProperties m_shaderBinaryCacheProperties{};

    // Constant data
    static constexpr vk::Format c_textureFormat = vk::Format::eR8G8B8Unorm;
    static constexpr vk::Format c_depthFormat = vk::Format::eD32Sfloat;
    

};


template <typename T>
[[nodiscard]] static inline constexpr T AlignUp( T value, T alignment )
{
    if (!alignment)
        return value;

    return (value + alignment -1 ) / alignment * alignment;
}

#endif
