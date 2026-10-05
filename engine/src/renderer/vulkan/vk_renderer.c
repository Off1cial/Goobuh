#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "renderer/vulkan/vk_renderer.h"
#include "renderer/vulkan/vk_pipeline.h"
#include "renderer/vulkan/vk_vma.h"
#include "renderer/vulkan/vk_info.h"
#include "renderer/vulkan/vk_mesh.h"
#include "camera.h"
#include "common/logsys.h"
#include "renderer/vulkan/vk_types.h"

#include "math/matrix.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <stdlib.h>

static PushConstants camera_push_consts = {
    .model = _MatIdentity_,
    .projection = _MatIdentity_,
    .view = _MatIdentity_,
    .vertex_addr = 0};

static inline void vkcheck(VkResult result)
{
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("Vulkan check failed");
        exit(1);
    }
}

void destroy_swapchain(VK_Renderer *engine)
{
    for (uint32_t i = 0; i < engine->swapchain_data.image_count; i++)
    {
        vkDestroyImageView(engine->device, engine->swapchain_data.image_views[i], NULL);
    }
    free(engine->swapchain_data.image_views);
    free(engine->swapchain_data.images);
    vkDestroySwapchainKHR(engine->device, engine->swapchain, NULL);
}

void VK_Shutdown(VK_Renderer *engine)
{
    vkDeviceWaitIdle(engine->device);

    /* FIX: was destroying the same command pool MAX_FRAMES_IN_FLIGHT times */
    vkDestroyCommandPool(engine->device, engine->command_pool, NULL);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vmaDestroyBuffer(
            engine->allocator,
            engine->frames_in_flight.shader_data_buffers[i].buffer,
            engine->frames_in_flight.shader_data_buffers[i].allocation);
        vkDestroyFence(engine->device, engine->frames_in_flight.fence[i], NULL);
        vkDestroySemaphore(engine->device, engine->frames_in_flight.swapchain_semaphore[i], NULL);
    }

    for (uint32_t i = 0; i < engine->swapchain_data.image_count; i++)
        vkDestroySemaphore(engine->device, engine->frames_in_flight.render_semaphores[i], NULL);

    vkDestroyImageView(engine->device, engine->depth_image_view, NULL);
    vmaDestroyImage(engine->allocator, engine->depth_image, engine->depth_image_allocation);

    vkDestroyImageView(engine->device, engine->draw_image.view, NULL);
    vmaDestroyImage(engine->allocator, engine->draw_image.image, engine->draw_image.allocation);

    for (uint32_t i = 0; i < engine->swapchain_data.image_count; i++)
        vkDestroyImageView(engine->device, engine->swapchain_data.image_views[i], NULL);
    free(engine->swapchain_data.image_views);
    free(engine->swapchain_data.images);
    vkDestroySwapchainKHR(engine->device, engine->swapchain, NULL);

    VKTexture_destroy(engine, &engine->default_texture);
    vkDestroySampler(engine->device, engine->default_sampler, NULL);
    vkDestroyDescriptorPool(engine->device, engine->descriptor_pool, NULL);   // frees the set
    vkDestroyDescriptorSetLayout(engine->device, engine->texture_set_layout, NULL);

    vmaDestroyAllocator(engine->allocator);

    vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);
    vkDestroyDevice(engine->device, NULL);

    // vkDestroyDebugUtilsMessengerEXT(engine->instance, engine->debug_messenger, NULL);
    vkDestroyInstance(engine->instance, NULL);

    free(engine->physical_devices);
    free(engine->queue_families);
}

static inline VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspectMask)
{
    VkImageSubresourceRange subImage = {0};
    subImage.aspectMask = aspectMask;
    subImage.baseMipLevel = 0;
    subImage.levelCount = VK_REMAINING_MIP_LEVELS;
    subImage.baseArrayLayer = 0;
    subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;
    return subImage;
}

void copy_image_to_image(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize)
{
    VkImageBlit2 blitRegion = {.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = NULL};

    blitRegion.srcOffsets[1].x = (int)srcSize.width;
    blitRegion.srcOffsets[1].y = (int)srcSize.height;
    blitRegion.srcOffsets[1].z = 1;

    blitRegion.dstOffsets[1].x = (int)dstSize.width;
    blitRegion.dstOffsets[1].y = (int)dstSize.height;
    blitRegion.dstOffsets[1].z = 1;

    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcSubresource.mipLevel = 0;

    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstSubresource.mipLevel = 0;

    VkBlitImageInfo2 blitInfo = {.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = NULL};
    blitInfo.dstImage = destination;
    blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    blitInfo.srcImage = source;
    blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    blitInfo.filter = VK_FILTER_LINEAR;
    blitInfo.regionCount = 1;
    blitInfo.pRegions = &blitRegion;

    vkCmdBlitImage2(cmd, &blitInfo);
}

static void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier2 imageBarrier = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
    imageBarrier.pNext = NULL;

    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    imageBarrier.oldLayout = currentLayout;
    imageBarrier.newLayout = newLayout;

    VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange = image_subresource_range(aspectMask);
    imageBarrier.image = image;

    VkDependencyInfo depInfo = {0};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.pNext = NULL;

    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &imageBarrier;

    vkCmdPipelineBarrier2(cmd, &depInfo);
}

void VK_WindowResize(VK_Renderer *engine)
{
    vkDeviceWaitIdle(engine->device);

    vkDestroyImageView(engine->device, engine->depth_image_view, NULL);
    vmaDestroyImage(engine->allocator, engine->depth_image, engine->depth_image_allocation);

    vkDestroyImageView(engine->device, engine->draw_image.view, NULL);
    vmaDestroyImage(engine->allocator, engine->draw_image.image, engine->draw_image.allocation);

    destroy_swapchain(engine);
    create_swapchain(engine, engine->window); // updates draw_extent
    create_depth_image(engine);               // must come after create_swapchain

    engine->winresize_request = false;
}

static uint32_t frame_index, swapchain_image;
static VkCommandBuffer cmd;
static VkImage current_image;

bool VK_BeginRendering(VK_Renderer *engine, camera_t *camera)
{
    frame_index = engine->frame_index;
    VkFence *active_fence = &engine->frames_in_flight.fence[frame_index];
    vkWaitForFences(engine->device, 1, active_fence, VK_TRUE, UINT64_MAX);

    VkResult acquire_result = vkAcquireNextImageKHR(
        engine->device,
        engine->swapchain,
        UINT64_MAX,
        engine->frames_in_flight.swapchain_semaphore[frame_index],
        NULL,
        &swapchain_image);

    if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        engine->winresize_request = 1;
        return false;
    }
    else if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR)
    {
        vkcheck(acquire_result);
    }

    /* fence must be unsignaled before being submitted again */
    vkResetFences(engine->device, 1, active_fence);

    cmd = engine->frames_in_flight.command_buffers[frame_index];
    engine->cmd_active = cmd;
    vkResetCommandBuffer(cmd, 0);
    VkCommandBufferBeginInfo cmdbegin = begininfo_cmdbuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    MatrixCopy(camera->view, camera_push_consts.view);
    MatrixCopy(camera->proj, camera_push_consts.projection);

    vkBeginCommandBuffer(cmd, &cmdbegin);

    current_image = engine->swapchain_data.images[swapchain_image];

    transition_image(cmd, engine->draw_image.image,
                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    transition_image(cmd, engine->depth_image,
                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    VkClearValue clear_color = {{{0.02f, 0.1f, 0.1f, 1.0f}}};

    VkRenderingAttachmentInfo color_attachment = {0};
    color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    color_attachment.imageView = engine->draw_image.view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.clearValue = clear_color;

    VkRenderingAttachmentInfo depth_attachment = {0};
    depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depth_attachment.imageView = engine->depth_image_view;
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.clearValue.depthStencil.depth = 1.0f;

    VkRenderingInfo rendering_info = {0};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.renderArea.extent = engine->draw_extent;
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;
    rendering_info.pDepthAttachment = &depth_attachment;

    vkCmdBeginRendering(cmd, &rendering_info);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, engine->pipeline);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
            engine->pipeline_layout, 0, 1, &engine->texture_set, 0, NULL);

    VkViewport viewport = {0};
    viewport.width = (float)engine->draw_extent.width;
    viewport.height = (float)engine->draw_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {0};
    scissor.extent = engine->draw_extent;

    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    return true;
}

void VK_EndRendering(VK_Renderer *engine)
{
    vkCmdEndRendering(cmd);

    transition_image(
        cmd,
        engine->draw_image.image,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    transition_image(
        cmd,
        current_image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copy_image_to_image(
        cmd,
        engine->draw_image.image,
        current_image,
        engine->draw_extent,
        engine->swapchain_data.swapchain_extent);

    transition_image(
        cmd,
        current_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // transition_image(cmd, current_image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    vkEndCommandBuffer(cmd);

    VkCommandBufferSubmitInfo cmdsubmit = submitinfo_cmdbuffer(cmd);

    VkSemaphoreSubmitInfo wait_info = submit_info_semaphore(
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
        engine->frames_in_flight.swapchain_semaphore[frame_index]);
    VkSemaphoreSubmitInfo signal_info = submit_info_semaphore(
        VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
        engine->frames_in_flight.render_semaphores[swapchain_image]); // CHECK INDEX

    VkSubmitInfo2 submit = submit_info(&cmdsubmit, &signal_info, &wait_info);

    vkQueueSubmit2(engine->graphics_queue, 1, &submit, engine->frames_in_flight.fence[frame_index]);

    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &engine->frames_in_flight.render_semaphores[swapchain_image]; // CHECK INDEX
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &engine->swapchain;
    present_info.pImageIndices = &swapchain_image;

    VkResult present_result = vkQueuePresentKHR(engine->graphics_queue, &present_info);
    if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR)
    {
        engine->winresize_request = 1;
    }
    else
    {
        vkcheck(present_result);
    }

    engine->frame_index = (engine->frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
    engine->frame_count++;
}

/*

void VK_Draw(VK_Renderer *engine, camera_t* camera)
{
  uint32_t frame_index = engine->frame_index;
  VkFence *active_fence = &engine->frames_in_flight.fence[frame_index];
  vkWaitForFences(engine->device, 1, active_fence, VK_TRUE, UINT64_MAX);

  uint32_t swapchain_image;
  VkResult acquire_result = vkAcquireNextImageKHR(
      engine->device,
      engine->swapchain,
      UINT64_MAX,
      engine->frames_in_flight.swapchain_semaphore[frame_index],
      NULL,
      &swapchain_image);

  if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    // TODO: recreate_swapchain(engine);
    return;
  }
  else if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR)
  {
    vkcheck(acquire_result);
  }

  // FIX: fence must be unsignaled before being submitted again
  vkResetFences(engine->device, 1, active_fence);

  VkCommandBuffer cmd = engine->frames_in_flight.command_buffers[frame_index];
  vkResetCommandBuffer(cmd, 0);
  VkCommandBufferBeginInfo cmdbegin = begininfo_cmdbuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  MatrixCopy(camera->view, camera_push_consts.view);
  MatrixCopy(camera->proj, camera_push_consts.projection);
  //MatrixIdentity(camera_push_consts.view);
  //MatrixIdentity(camera_push_consts.projection);

  vkBeginCommandBuffer(cmd, &cmdbegin);

  VkImage current_image = engine->swapchain_data.images[swapchain_image];
  //VkImageView current_view = engine->swapchain_data.image_views[swapchain_image];

  transition_image(cmd, engine->draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  VkClearValue clear_color = {{{0.02f, 0.02f, 0.05f, 1.0f}}};

  VkRenderingAttachmentInfo color_attachment = {0};
  color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  color_attachment.imageView = engine->draw_image.view;
  color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.clearValue = clear_color;

  VkRenderingInfo rendering_info = {0};
  rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  rendering_info.renderArea.extent = engine->draw_extent;
  rendering_info.layerCount = 1;
  rendering_info.colorAttachmentCount = 1;
  rendering_info.pColorAttachments = &color_attachment;
  // Add a depth attachment here once a pipeline that uses depth-testing exists:
  // transition depth image to VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL once
  // (not every frame — it never leaves that layout), then set
  // rendering_info.pDepthAttachment.

  vkCmdBeginRendering(cmd, &rendering_info);

  vkCmdBindPipeline(
      cmd,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      engine->pipeline);


  VkViewport viewport = {0};
  viewport.width = (float)engine->draw_extent.width;
  viewport.height = (float)engine->draw_extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor = {0};
  scissor.extent = engine->draw_extent;

  vkCmdSetViewport(cmd, 0, 1, &viewport);
  vkCmdSetScissor(cmd, 0, 1, &scissor);


printf("PUSH VIEW:\n");
for (int i = 0; i < 4; i++)
    printf("%f %f %f %f\n",
        camera_push_consts.view[i],
        camera_push_consts.view[i + 4],
        camera_push_consts.view[i + 8],
        camera_push_consts.view[i + 12]);

printf("PUSH PROJ:\n");
for (int i = 0; i < 4; i++)
    printf("%f %f %f %f\n",
        camera_push_consts.projection[i],
        camera_push_consts.projection[i + 4],
        camera_push_consts.projection[i + 8],
            camera_push_consts.projection[i + 12] );
  VKMesh_draw(
      engine->device,
      engine->pipeline_layout,
      cmd,
      engine->pipeline,
      engine->mesh_data,
      &camera_push_consts
      );

  vkCmdEndRendering(cmd);

  transition_image(
      cmd,
      engine->draw_image.image,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
      );
  transition_image(
      cmd,
      current_image,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
      );
  copy_image_to_image(
      cmd,
      engine->draw_image.image,
      current_image,
      engine->draw_extent,
      engine->swapchain_data.swapchain_extent
      );

  transition_image(
      cmd,
      current_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
      );

  //transition_image(cmd, current_image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  vkEndCommandBuffer(cmd);

  VkCommandBufferSubmitInfo cmdsubmit = submitinfo_cmdbuffer(cmd);

  VkSemaphoreSubmitInfo wait_info = submit_info_semaphore(
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
      engine->frames_in_flight.swapchain_semaphore[frame_index]);
  VkSemaphoreSubmitInfo signal_info = submit_info_semaphore(
      VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
      engine->frames_in_flight.render_semaphores[swapchain_image]); // CHECK INDEX

  VkSubmitInfo2 submit = submit_info(&cmdsubmit, &signal_info, &wait_info);

  vkQueueSubmit2(engine->graphics_queue, 1, &submit, engine->frames_in_flight.fence[frame_index]);

  VkPresentInfoKHR present_info = {0};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &engine->frames_in_flight.render_semaphores[swapchain_image]; // CHECK INDEX
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &engine->swapchain;
  present_info.pImageIndices = &swapchain_image;

  VkResult present_result = vkQueuePresentKHR(engine->graphics_queue, &present_info);
  if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR){
    engine->winresize_request = 1;
  }
  else{
    vkcheck(present_result);
  }

  engine->frame_index = (engine->frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
  engine->frame_count++;
}
*/

void VK_Draw(VK_Renderer *engine, camera_t *camera)
{
    VK_BeginRendering(engine, camera);

    VKMesh_draw(
        engine->device,
        engine->pipeline_layout,
        cmd,
        engine->pipeline,
        engine->mesh_data,
        &camera_push_consts);

    VK_EndRendering(engine);
}

VkCommandBuffer begin_one_time_commands(VK_Renderer *engine)
{
    VkCommandBufferAllocateInfo alloc = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = engine->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer cmd;
    vkcheck(vkAllocateCommandBuffers(engine->device, &alloc, &cmd));

    VkCommandBufferBeginInfo begin = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkcheck(vkBeginCommandBuffer(cmd, &begin));
    return cmd;
}

void end_one_time_commands(VK_Renderer *engine, VkCommandBuffer cmd)
{
    vkcheck(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmd_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmd,
    };
    VkSubmitInfo2 submit = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmd_info,
    };
    vkcheck(vkQueueSubmit2(engine->graphics_queue, 1, &submit, VK_NULL_HANDLE));
    vkcheck(vkQueueWaitIdle(engine->graphics_queue));

    vkFreeCommandBuffers(engine->device, engine->command_pool, 1, &cmd);
}

VKTexture VKTexture_create(VK_Renderer *engine, const void *pixels,
                         uint32_t w, uint32_t h, VkFormat format)
{
    VKTexture tex = {0};
    tex.extent = (VkExtent3D){w, h, 1};
    tex.format = format;
    tex.mip_levels = 1; // see mipmaps below

    /* 1. staging buffer */
    VkDeviceSize size = (VkDeviceSize)w * h * 4;
    VkBufferCreateInfo buf_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    };
    VmaAllocationCreateInfo buf_alloc = {
        .usage = VMA_MEMORY_USAGE_AUTO,
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                 VMA_ALLOCATION_CREATE_MAPPED_BIT,
    };
    VkBuffer staging;
    VmaAllocation staging_alloc;
    VmaAllocationInfo staging_info;
    vkcheck(vmaCreateBuffer(engine->allocator, &buf_info, &buf_alloc,
                            &staging, &staging_alloc, &staging_info));
    memcpy(staging_info.pMappedData, pixels, size);

    /* 2. the image itself */
    VkImageCreateInfo img = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = tex.extent,
        .mipLevels = tex.mip_levels,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    VmaAllocationCreateInfo img_alloc = {.usage = VMA_MEMORY_USAGE_AUTO};
    vkcheck(vmaCreateImage(engine->allocator, &img, &img_alloc,
                           &tex.image, &tex.allocation, NULL));

    /* 3. record and run the upload */
    VkCommandBuffer cmd = begin_one_time_commands(engine);

    transition_image(cmd, tex.image,
                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy region = {
        .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        .imageExtent = tex.extent,
    };
    vkCmdCopyBufferToImage(cmd, staging, tex.image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    transition_image(cmd, tex.image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    end_one_time_commands(engine, cmd); // submit + wait

    vmaDestroyBuffer(engine->allocator, staging, staging_alloc);

    /* 4. view */
    VkImageViewCreateInfo view = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = tex.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, tex.mip_levels, 0, 1},
    };
    vkcheck(vkCreateImageView(engine->device, &view, NULL, &tex.view));
    return tex;
}


void VKTexture_destroy(VK_Renderer* engine, VKTexture* tex)
{
  if (!tex->view) return;
  vkDestroyImageView(engine->device, tex->view, NULL);
  vmaDestroyImage(engine->allocator, tex->image, tex->allocation);
  tex->view = VK_NULL_HANDLE;
}

void VKTexture_register(VK_Renderer* engine, VKTexture* tex, uint32_t index)
{
  VkDescriptorImageInfo info = {
    .sampler = engine->default_sampler,
    .imageView = tex->view,
    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  VkWriteDescriptorSet write = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = engine->texture_set,
    .dstBinding = 0,
    .dstArrayElement = index,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    .pImageInfo = &info,
  };
  vkUpdateDescriptorSets(engine->device, 1, &write, 0, NULL);
}

typedef struct { uint8_t* pixels; int w, h; } DecodedImage;

static DecodedImage decode_memory(const void* data, size_t size)
{
  DecodedImage out = {0};
  int channels;
  out.pixels = stbi_load_from_memory(data, (int)size, &out.w, &out.h, &channels, 4); // force RGBA
  return out;
}

static DecodedImage decode_file(const char* path)
{
  DecodedImage out = {0};
  int channels;
  out.pixels = stbi_load(path, &out.w, &out.h, &channels, 4);
  return out;
}