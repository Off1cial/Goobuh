/*
#define VMA_IMPLEMENTATION
#include "vulkan/vk_mem_alloc.h"
*/
#include <vulkan/vulkan.hpp>
#include "engine/vk/vk.h"
#include <SDL3/SDL_video.h>
#include <VkBootstrap.h>
#include <SDL3/SDL_vulkan.h>
#include <optional>
#include <vulkan/vulkan_core.h>

#include "core/error.h"
#include "vulkan/vulkan.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

bool CVulkanSystem::Init( const char *title, int w, int h)
{
    if (!SDL_Init( SDL_INIT_VIDEO ))
    {
        return false;
    }

    m_window = SDL_CreateWindow( title, w, h, SDL_WINDOW_VULKAN);

    if (!m_window)
    {
        return false;
    }

    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
             
    // Create Vulkan instance
    auto instanceBuilder = vkb::InstanceBuilder()
                            .require_api_version(1, 4)
                            .set_app_name( "Game - Vulkan ")
                            .set_engine_name( "Game" )
                            #ifdef _DEBUG
                            .request_validation_layers(true)
                            .use_default_debug_messenger();
                            #else
                            ;
                            #endif

    auto instanceResult = instanceBuilder.build();
    if (!instanceResult)
    {
        auto error = instanceResult.full_error();
        if (error.vk_result != VK_SUCCESS)
        {
            for ( std::string errstr : error.detailed_failure_reasons)
                printf("[VULKAN]: InstanceBuild error: %s\n", errstr.data());
            return false;
        }
    }

    m_vkbData.instance = instanceResult.value();
    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_vkbData.instance.instance, vkGetInstanceProcAddr);

    // Create Vulkan surface

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    
    if (!SDL_Vulkan_CreateSurface( m_window, m_vkbData.instance.instance, nullptr, &surface ))
    {

    }
    m_surface = vk::SurfaceKHR{surface};
        
    const std::array extensions
    {
        VK_EXT_SHADER_OBJECT_EXTENSION_NAME,     
        VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
        VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME,
    };

    vkb::PhysicalDeviceSelector physSelector{m_vkbData.instance};

    physSelector
        .add_required_extensions(extensions.size(), extensions.data())
        .set_surface(static_cast<VkSurfaceKHR>(m_surface))
        .set_minimum_version(1,4)
        .add_required_extension_features(
                VkPhysicalDeviceVulkan14Features
                {
                    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
                    .maintenance5 = VK_TRUE,
                })

        .add_required_extension_features(
                VkPhysicalDeviceVulkan13Features
                {
.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
                    .synchronization2 = VK_TRUE,
                    .dynamicRendering = VK_TRUE,
                })

        .add_required_extension_features(
                VkPhysicalDeviceVulkan12Features
                {
                    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
                    .bufferDeviceAddress = VK_TRUE,
                })

        .add_required_extension_features(
                VkPhysicalDeviceVulkan11Features
                {
                    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
                    .shaderDrawParameters = VK_TRUE,
                })

        .add_required_extension_features(
                VkPhysicalDeviceShaderObjectFeaturesEXT
                {
                    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
                    .shaderObject = VK_TRUE,
                })

        .add_required_extension_features(
                VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
                {
                    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
                    .extendedDynamicState3DepthClampEnable = VK_TRUE,
                    .extendedDynamicState3PolygonMode = VK_TRUE,
                    .extendedDynamicState3RasterizationSamples = VK_TRUE,
                    .extendedDynamicState3SampleMask = VK_TRUE,
                    .extendedDynamicState3AlphaToCoverageEnable = VK_TRUE,
                    .extendedDynamicState3AlphaToOneEnable = VK_TRUE,
                    .extendedDynamicState3LogicOpEnable = VK_TRUE,
                    .extendedDynamicState3ColorBlendEnable = VK_TRUE,
                    .extendedDynamicState3ColorWriteMask = VK_TRUE,
                })
        .add_required_extension_features(
                VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT
                {
                    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
                    .vertexInputDynamicState = VK_TRUE,
                })
        .add_required_extension_features(
                VkPhysicalDeviceDescriptorHeapFeaturesEXT
                {
                    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT,
                    .descriptorHeap = VK_TRUE,
                });

    
        std::optional<vkb::PhysicalDevice> selectedDevice;

    auto physDevicesResult = physSelector.select_devices();
    if (!physDevicesResult)
    {   
        Error( ERROR_VULKAN, "Failed to find appropriate physical device\n");
        return false;
    }
    const std::vector<vkb::PhysicalDevice> physDevices = 
        physSelector.select_devices().value();

    for (const vkb::PhysicalDevice &device : physDevices)
    {
        vk::PhysicalDevice candidate{device.physical_device};

        const vk::FormatProperties2 formatProps = 
            candidate.getFormatProperties2(c_textureFormat);

        const vk::FormatFeatureFlags requiredFeatures = 
            vk::FormatFeatureFlagBits::eBlitSrc |
            vk::FormatFeatureFlagBits::eBlitDst |
            vk::FormatFeatureFlagBits::eSampledImageFilterLinear;

        if ((formatProps.formatProperties.optimalTilingFeatures & requiredFeatures) != requiredFeatures)
        {
            continue;
        }
        else
        {
            selectedDevice = device;
            break;
        }
    }
    
    if (!selectedDevice.has_value())
    {
        // Exit error
        return false;
    }
    
    vkb::PhysicalDevice physicalDevice = *selectedDevice;
    vk::PhysicalDevice finalPhysicalDevice{physicalDevice.physical_device};


    // Load phyiscal device properties
    // Stored in a vector to avoid querying later 
    
    const vk::PhysicalDeviceMemoryProperties2 memoryProperties = 
        finalPhysicalDevice.getMemoryProperties2();
    m_memoryTypeFlags.clear();

    m_memoryTypeFlags.reserve(memoryProperties.memoryProperties.memoryTypeCount);

    for (uint32_t i = 0; i < memoryProperties.memoryProperties.memoryTypeCount; ++i)
    {
        m_memoryTypeFlags.push_back(memoryProperties.memoryProperties.memoryTypes[i].propertyFlags);
    }

    
    vk::PhysicalDeviceDescriptorHeapPropertiesEXT heapProperties{};
    vk::PhysicalDeviceShaderObjectPropertiesEXT shaderObjectProperties{};
    shaderObjectProperties.pNext = &heapProperties;

    vk::PhysicalDeviceProperties2 properties2{};
    properties2.pNext = &shaderObjectProperties;

    finalPhysicalDevice.getProperties2(&properties2);

    // Obtain descriptor sizes
    m_descriptorHeaps.sampledImageSize = finalPhysicalDevice.getDescriptorSizeEXT(vk::DescriptorType::eSampledImage);
    m_descriptorHeaps.samplerSize = finalPhysicalDevice.getDescriptorSizeEXT(vk::DescriptorType::eSampler);
    m_descriptorHeaps.uniformBufferSize = finalPhysicalDevice.getDescriptorSizeEXT(vk::DescriptorType::eUniformBuffer);
    m_descriptorHeaps.storageBufferSize = finalPhysicalDevice.getDescriptorSizeEXT(vk::DescriptorType::eStorageBuffer);
    // Obtain descriptor alignments
    m_descriptorHeaps.imageDescriptorAlignment = heapProperties.imageDescriptorAlignment;
    m_descriptorHeaps.bufferDescriptorAlignment = heapProperties.bufferDescriptorAlignment;
    m_descriptorHeaps.samplerDescriptorAlignment = heapProperties.samplerDescriptorAlignment;
    // Device heap alignment
    m_descriptorHeaps.resourceHeap.alignment = heapProperties.resourceHeapAlignment;
    m_descriptorHeaps.samplerHeap.alignment = heapProperties.samplerHeapAlignment;
    // Query the device's interal reserved range
    m_descriptorHeaps.resourceHeap.minReservationSize = heapProperties.minResourceHeapReservedRange;
    m_descriptorHeaps.samplerHeap.minReservationSize = heapProperties.minSamplerHeapReservedRange;
    // Push data size
    m_descriptorHeaps.maxPushSize = heapProperties.maxPushDataSize;
    // Fetching strides, easier indexing into the heap
    m_descriptorHeaps.uniformBufferStride = static_cast<uint32>( 
        AlignUp(m_descriptorHeaps.uniformBufferSize, m_descriptorHeaps.bufferDescriptorAlignment)); 
    
    m_descriptorHeaps.sampledImageStride = static_cast<uint32>(
        AlignUp(m_descriptorHeaps.sampledImageSize, m_descriptorHeaps.imageDescriptorAlignment));

    
    // Shader binary properties
    std::copy( 
            shaderObjectProperties.shaderBinaryUUID.begin(), 
            shaderObjectProperties.shaderBinaryUUID.end(), 
            m_shaderBinaryCacheProperties.shaderBinaryUUID.begin() );
    m_shaderBinaryCacheProperties.shaderBinaryVersion = shaderObjectProperties.shaderBinaryVersion;

    // Logical device
    vkb::Result<vkb::Device> logicalDeviceResult = vkb::DeviceBuilder( physicalDevice ).build(); 
    if (!logicalDeviceResult)
    {
        Error( ERROR_VULKAN, "Failed to build logical device\n");
        return false;
    }

    m_logicalDevice = vk::Device(m_vkbData.device.device);

    // Load device function pointers
    VULKAN_HPP_DEFAULT_DISPATCHER.init(
            m_vkbData.instance.instance, 
            vkGetInstanceProcAddr,
            m_vkbData.device.device, 
            vkGetDeviceProcAddr
            );

    
    // Fetch the graphics queue
    vkb::Result<VkQueue> graphQueueResult = m_vkbData.device.get_queue(vkb::QueueType::graphics);
    if (!graphQueueResult)
    {
        Error( ERROR_VULKAN, "Failed to fetch graphics queue\n");
        return false;
    }
    m_graphicsQueue = graphQueueResult.value();
    
    vkb::Result<uint32> graphQueueFamilyResult = m_vkbData.device.get_queue_index(vkb::QueueType::graphics);
    if (!graphQueueFamilyResult)
    {
        Error( ERROR_VULKAN, "Failed to fetch graphics queue family\n");
        return false;
    }
    m_graphicsQueueFamily = graphQueueFamilyResult.value();

    // Fetch the presentation quueue
    vkb::Result<VkQueue> presentQueueResult = m_vkbData.device.get_queue(vkb::QueueType::present);
    if (!presentQueueResult)
    {
        Error( ERROR_VULKAN, "Failed to fetch presentation queue\n");
        return false;
    }
    m_presentQueue = presentQueueResult.value();
    
    vkb::Result<uint32> presentQueueFamilyResult = m_vkbData.device.get_queue_index(vkb::QueueType::present);
    if (!presentQueueFamilyResult)
    {
        Error( ERROR_VULKAN, "Failed to fetch presentation queue family\n");
        return false;
    }
    

    // For now, use the single queue family to avoid managing ownership transfers
    if (m_graphicsQueueFamily != m_presentQueueFamily)
    {   
        Error( ERROR_VULKAN, "Graphics queue and present queue are not of the same family\n");
        return false;
    }
    
    // Find present modes
    // Default to FIFO for now
    const VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    
    vkb::SwapchainBuilder swapchainBuilder = vkb::SwapchainBuilder(m_vkbData.device)
                                                    .set_desired_extent(w, h)
                                                    .set_desired_present_mode(presentMode)
                                                    .add_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    
    vkb::Result<vkb::Swapchain> swapchainResult = swapchainBuilder.build();

    if (!swapchainResult)
    {
        Error( ERROR_VULKAN "Failed to create swapchain\n");
        return false;
    }

    m_vkbData.swapchain = swapchainResult.value();
    m_swapchainFormat = static_cast<vk::Format>(m_vkbData.swapchain.image_format);
    m_swapchainExtent = { 
        .width = m_vkbData.swapchain.extent.width,
        .height = m_vkbData.swapchain.extent.height
    };

    
    
    // Retrieve swapchain images
    auto getImagesResult = m_vkbData.swapchain.get_images();
    if (!getImagesResult)
    {
        Error( ERROR_VULKAN "Failed to fetch swapchain images\n");
        return false;
    }

    const std::vector<VkImage> images = getImagesResult.value();
    
    m_swapchainImages.EnsureCapacity(images.size());

    for ( VkImage image : images )
    {
        const vk::Image swapchainImage{image};


        vk::ImageViewCreateInfo imageViewInfo{};
        imageViewInfo.setImage(swapchainImage)
             .setViewType(vk::ImageViewType::e2D)
             .setFormat(m_swapchainFormat)
             .setComponents(c_identityComponentMapping)
             .setSubresourceRange(CreateImageSubresourceRange());

        SwapchainImage swImage = {
            .image = swapchainImage,
            .imageView = m_logicalDevice.createImageViewUnique(imageViewInfo),
            .state = ImageState{.aspectMask = vk::ImageAspectFlagBits::eColor}
        };

        m_swapchainImages.AddToHead(swImage);

        
    }
    return true;
}


bool CVulkanSystem::InitCommandPool()
{
//....
}



void CVulkanSystem::Shutdown()
{
    // Do shutdown!
}
