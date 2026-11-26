#ifndef __VK_H__
#define __VK_H__

#include <array>

#include <SDL3/SDL.h>
#include <vulkan/vulkan.hpp>
#include <VkBootstrap.h>
#include <vulkan/vulkan_core.h>

#include "engine/vk/vk_buff.h"
#include "core/util/utlvector.h"
#include "vulkan/vulkan.hpp"

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

    bool InitCommandPool();

    

    static constexpr vk::ImageSubresourceRange
    CreateImageSubresourceRange(
            const uint32 baseMipLevel = 0,
            const vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor,
            const uint32 levelCount = 1
            )
    {
        return vk::ImageSubresourceRange
        {
             aspectMask,
             baseMipLevel,
             levelCount,
             0,
             1,
        };
    }


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

    struct ImageState
    {
        vk::ImageLayout layout = vk::ImageLayout::eUndefined;
        vk::PipelineStageFlags2 stageMask = vk::PipelineStageFlagBits2::eNone;
        vk::AccessFlags2 accessMask = vk::AccessFlagBits2::eNone; 
        vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor;

        bool operator==(const ImageState&) const = default; 
    };

    struct SwapchainImage
    {
        vk::Image image;
        vk::UniqueImageView imageView;
        vk::UniqueSemaphore renderFinsihedSemaphore;
        ImageState state;

    };  

    DescriptorHeapResources m_descriptorHeaps{};


    CUtlVector<SwapchainImage> m_swapchainImages;
    vk::Format m_swapchainFormat;
    vk::Extent2D m_swapchainExtent;

    vk::SurfaceKHR m_surface;
    vk::Device m_logicalDevice;

    // Queue families are the same for now
    uint32 m_graphicsQueueFamily = 0;
    uint32 m_presentQueueFamily = 0;
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

    static constexpr vk::ComponentMapping c_identityComponentMapping{
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity
    };

    

};


template <typename T>
[[nodiscard]] static inline constexpr T AlignUp( T value, T alignment )
{
    if (!alignment)
        return value;

    return (value + alignment -1 ) / alignment * alignment;
}

#endif
