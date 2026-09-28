#include "volk/volk.h"
#include "renderer/vulkan/vk_renderer.h"
#include "renderer/vulkan/vk_types.h"
#include "renderer/vulkan/vk_info.h"
#include "renderer/vulkan/vk_pipeline.h"

#include "common/logsys.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

static inline void vkcheck(VkResult result)
{
  if (result != VK_SUCCESS)
  {
    LOG_FATAL("Vulkan check failed");
    exit(1);
  }
}



void VKPipeline_clear(VKPipelineSet* set)
{
  if (!set) return;

  memset(set, 0, sizeof(VKPipelineSet));

  set->input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  set->input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  set->input_assembly.primitiveRestartEnable = VK_FALSE;

  set->rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  set->rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  set->rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  set->rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  set->rasterizer.lineWidth = 1.0f;

  set->colattachment_format = VK_FORMAT_UNDEFINED;

  set->multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  set->multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  set->depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  set->depth_stencil.depthTestEnable = VK_FALSE;
  set->depth_stencil.depthWriteEnable = VK_FALSE;
  set->depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
  set->depth_stencil.stencilTestEnable = VK_FALSE;

  set->rendering.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
}

void VKPipeline_set_shaders(
    VKPipelineSet* set,
    VkShaderModule vertex_module,
    VkShaderModule fragment_module)
{
  if (!set) return;

  printf("vertex:   %p\n", (void *)vertex_module);
  printf("fragment: %p\n", (void *)fragment_module);

  set->shader_stages[SHADER_STAGE_VERTEX] =
    createinfo_pipeline_shader_stage(vertex_module, VK_SHADER_STAGE_VERTEX_BIT);

  set->shader_stages[SHADER_STAGE_FRAGMENT] =
    createinfo_pipeline_shader_stage(fragment_module, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void VKPipeline_set_topology(VKPipelineSet* set, VkPrimitiveTopology topology)
{
  if (!set) return;

  set->input_assembly.topology = topology;
  set->input_assembly.primitiveRestartEnable = VK_FALSE;
}

void VKPipeline_set_polygonmode(VKPipelineSet* set, VkPolygonMode mode)
{
  if (!set) return;

  set->rasterizer.polygonMode = mode;
  set->rasterizer.lineWidth = 1.0f;
}

void VKPipeline_set_cull_mode(
    VKPipelineSet* set,
    VkCullModeFlags mode,
    VkFrontFace frontface)
{
  if (!set) return;

  set->rasterizer.cullMode = mode;
  set->rasterizer.frontFace = frontface;
}


void VK_Pipeline_disable_blending(VKPipelineSet* set){
  set->colblend_attachment_state.colorWriteMask = 
    VK_COLOR_COMPONENT_R_BIT | 
    VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT | 
    VK_COLOR_COMPONENT_A_BIT;

  set->colblend_attachment_state.blendEnable = VK_FALSE;
}
void VKPipeline_enable_blending(VKPipelineSet* set){


    set->colblend_attachment_state.colorWriteMask = 
      VK_COLOR_COMPONENT_R_BIT | 
      VK_COLOR_COMPONENT_G_BIT | 
      VK_COLOR_COMPONENT_B_BIT | 
      VK_COLOR_COMPONENT_A_BIT;


    set->colblend_attachment_state.blendEnable = VK_TRUE;

    set->colblend_attachment_state.srcColorBlendFactor = 
      VK_BLEND_FACTOR_SRC_ALPHA;
    set->colblend_attachment_state.dstColorBlendFactor = 
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    set->colblend_attachment_state.colorBlendOp = 
      VK_BLEND_OP_ADD;
    set->colblend_attachment_state.srcAlphaBlendFactor = 
      VK_BLEND_FACTOR_ONE;
    set->colblend_attachment_state.dstAlphaBlendFactor = 
      VK_BLEND_FACTOR_ZERO;
    set->colblend_attachment_state.alphaBlendOp =  VK_BLEND_OP_ADD;
}


VkPipeline VKPipeline_build(
    VK_Renderer *engine,
    VKPipelineSet *set)
{
  if (!engine || !set)
    return VK_NULL_HANDLE;

  VkVertexInputAttributeDescription attributes[4] = {
    {
      .location = 0,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = offsetof(vertex_t, pos)
    },
    {
      .location = 1,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = offsetof(vertex_t, normal)
    },
    {
      .location = 2,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32A32_SFLOAT,
      .offset = offsetof(vertex_t, col)
    },
    {
      .location = 3,
      .binding = 0,
      .format = VK_FORMAT_R32G32_SFLOAT,
      .offset = offsetof(vertex_t, uv)
    }
  };

  VkVertexInputBindingDescription binding = {
    .binding = 0,
    .stride = sizeof(vertex_t),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
  };

  VkPipelineVertexInputStateCreateInfo vertex_input = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &binding,
    .vertexAttributeDescriptionCount = 4,
    .pVertexAttributeDescriptions = attributes
  };

  VkPipelineViewportStateCreateInfo viewport = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .scissorCount = 1
  };

  VkPipelineColorBlendAttachmentState color_blend_attachment = {
    .blendEnable = VK_FALSE,
    .colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT |
      VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT
  };

  VkPipelineColorBlendStateCreateInfo color_blend = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .attachmentCount = 1,
    .pAttachments = &color_blend_attachment
  };

  VkDynamicState dynamic_states[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
  };

  VkPipelineDynamicStateCreateInfo dynamic_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = sizeof(dynamic_states) / sizeof(dynamic_states[0]),
    .pDynamicStates = dynamic_states
  };

  VkPipelineRenderingCreateInfo rendering = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &engine->swapchain_data.format,
    .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
    .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
  };

  VkPipelineShaderStageCreateInfo shader_stages[SHADER_STAGE_COUNT];

  memcpy(shader_stages, set->shader_stages, sizeof(shader_stages));

  VkGraphicsPipelineCreateInfo pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &rendering,
    .stageCount = SHADER_STAGE_COUNT,
    .pStages = shader_stages,
    .pVertexInputState = &vertex_input,
    .pInputAssemblyState = &set->input_assembly,
    .pViewportState = &viewport,
    .pRasterizationState = &set->rasterizer,
    .pMultisampleState = &set->multisampling,
    .pDepthStencilState = &set->depth_stencil,
    .pColorBlendState = &color_blend,
    .pDynamicState = &dynamic_state,
    .layout = set->layout,
    .renderPass = VK_NULL_HANDLE,
    .subpass = 0
  };

  VkPipeline pipeline = VK_NULL_HANDLE;

  vkcheck(vkCreateGraphicsPipelines(
    engine->device,
    VK_NULL_HANDLE,
    1,
    &pipeline_info,
    NULL,
    &pipeline));

  return pipeline;
}
