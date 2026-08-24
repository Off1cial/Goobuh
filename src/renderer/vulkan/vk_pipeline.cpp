#include "renderer/vulkan/vk_renderer.hpp"
#include "core/logsys.hpp"
#include "renderer/vulkan/vk_types.hpp"
#include <random>
#include <regex>
#include <vulkan/vulkan_core.h>

using namespace VK;

void PipelineBuilder::Clear()
{
  // clear all of the structs we need back to 0 with their correct stype
  m_inputAssembly = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};

  m_rasterizer = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};

  m_colorBlendAttachment = {};

  m_multisampling = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};

  m_pipelineLayout = {};

  m_depthStencil = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

  m_renderInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};

  m_shaderstages.clear();


  VkPushConstantRange push_constant{};
  push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  push_constant.offset = 0;
  push_constant.size = sizeof(MeshPushConstants);

  VkPipelineLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutInfo.pPushConstantRanges = &push_constant;
  layoutInfo.pushConstantRangeCount = 1;
  layoutInfo.pSetLayouts = nullptr;
  layoutInfo.setLayoutCount = 0;
  layoutInfo.pNext = nullptr;

  VkPipelineLayout pipelineLayout;
  VK_CHECK(vkCreatePipelineLayout(
    m_device,
    &layoutInfo,
    nullptr,
    &pipelineLayout
  ));



  m_pipelineLayout = pipelineLayout;
}

Pipeline PipelineBuilder::BuildPipeline(VkDevice device)
{
  // make viewport state from our stored viewport and scissor.
  // at the moment we wont support multiple viewports or scissors
  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.pNext = nullptr;

  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  // setup dummy color blending. We arent using transparent objects yet
  // the blending is just "no blend", but we do write to the color attachment
  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.pNext = nullptr;

  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &m_colorBlendAttachment;

  // completely clear VertexInputStateCreateInfo, as we have no need for it
  VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
  _vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  // build the actual pipeline
  // we now use all of the info structs we have been writing into into this one
  // to create the pipeline
  VkGraphicsPipelineCreateInfo pipelineInfo;
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  // connect the renderInfo to the pNext extension mechanism
  pipelineInfo.pNext = &m_renderInfo;

  pipelineInfo.stageCount = (uint32_t)m_shaderstages.size();
  pipelineInfo.pStages = m_shaderstages.data();
  pipelineInfo.pVertexInputState = &_vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &m_inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &m_rasterizer;
  pipelineInfo.pMultisampleState = &m_multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDepthStencilState = &m_depthStencil;
  pipelineInfo.layout = m_pipelineLayout;

  VkDynamicState state[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  dynamicInfo.pDynamicStates = &state[0];
  dynamicInfo.dynamicStateCount = 2;

  pipelineInfo.pDynamicState = &dynamicInfo;

  // its easy to error out on create graphics pipeline, so we handle it a bit
  // better than the common VK_CHECK case
  VkPipeline newPipeline;
  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                nullptr, &newPipeline) != VK_SUCCESS)
  {
    LOG_FATAL("Failed to create grapphics pipeline");
    //return VK_NULL_HANDLE; // failed to create graphics pipeline
    return {VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE};
  }
  else
  {
    //return newPipeline;
    return {.vk_device = device, .pipeline = newPipeline, .layout = m_pipelineLayout};
  }
}

VkPipelineShaderStageCreateInfo PipelineBuilder::ShaderStageCreateInfo(const VkShaderStageFlagBits stage, const VkShaderModule module)
{
  VkPipelineShaderStageCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  info.stage = stage;
  info.module = module;
  info.pName = "main";
  return info;
}

void PipelineBuilder::SetShaderModules(VkShaderModule vertex, VkShaderModule fragment)
{
  m_shaderstages.clear();

  m_shaderstages.push_back(
     ShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertex)
    );
  m_shaderstages.push_back(
    ShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
      );
}

void PipelineBuilder::SetInputTopology(const VkPrimitiveTopology topology)
{
  m_inputAssembly.topology = topology;
  m_inputAssembly.primitiveRestartEnable = VK_FALSE;
}


void PipelineBuilder::SetPolygonMode(const VkPolygonMode mode)
{
  m_rasterizer.polygonMode = mode;
  m_rasterizer.lineWidth = 1.0f;
}

void PipelineBuilder::SetCullMode(const VkCullModeFlags flags, const VkFrontFace front)
{
  m_rasterizer.cullMode = flags;
  m_rasterizer.frontFace = front;
}

void PipelineBuilder::SetMSAA(const VkPhysicalDevice physdevice, const VkSampleCountFlagBits samplecount)
{
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(physdevice, &properties);
    
  VkSampleCountFlagBits final = samplecount;

  // Fetch hardware MSAA capabilities
  VkSampleCountFlags counts = 
    properties.limits.framebufferColorSampleCounts &
    properties.limits.framebufferDepthSampleCounts;
  if (!counts){
    LOG_ERROR("Attemped to set MSAA for an unsupported device");
    return;
  }
  // Use the maximum of the requested and supported
  while (final > 1){
    if (counts & final){
      // This version is supported
      break;
    }else{
      final = (VkSampleCountFlagBits)(final >> 1);
    }
  }
   

  // These settings might disable it and make the above redundant so..
  // TODO: REVIEW MSAA SELECTION
  m_multisampling.sampleShadingEnable = VK_FALSE;
  m_multisampling.rasterizationSamples = final;
  m_multisampling.minSampleShading = 1.0f;
  m_multisampling.pSampleMask = nullptr;

  m_multisampling.alphaToCoverageEnable = VK_FALSE;
  m_multisampling.alphaToOneEnable = VK_FALSE;
}



void PipelineBuilder::SetDepthFormat(const VkFormat format)
{
  m_renderInfo.depthAttachmentFormat = format;
}

void PipelineBuilder::SetColorAttachmentFormat(const VkFormat format)
{
    m_colorAttachmentformat = format;
    // connect the format to the renderInfo  structure
    m_renderInfo.colorAttachmentCount = 1;
    m_renderInfo.pColorAttachmentFormats = &m_colorAttachmentformat;
}



void PipelineBuilder::DisableBlending()
{
  m_colorBlendAttachment.colorWriteMask = 
    VK_COLOR_COMPONENT_R_BIT | 
    VK_COLOR_COMPONENT_G_BIT | 
    VK_COLOR_COMPONENT_B_BIT | 
    VK_COLOR_COMPONENT_A_BIT;

  m_colorBlendAttachment.blendEnable = VK_FALSE;
}

void PipelineBuilder::EnableBlending_Additive()
{
  m_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  m_colorBlendAttachment.blendEnable = VK_TRUE;
  m_colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  m_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
  m_colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  m_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  m_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  m_colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

void PipelineBuilder::EnableBlending_AlphaBlend()
{
  m_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  m_colorBlendAttachment.blendEnable = VK_TRUE;
  m_colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  m_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  m_colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  m_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  m_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  m_colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

void PipelineBuilder::DisableDepthTest()
{
  m_depthStencil.depthTestEnable = VK_FALSE;
  m_depthStencil.depthWriteEnable = VK_FALSE;
  m_depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
  m_depthStencil.depthBoundsTestEnable = VK_FALSE;
  m_depthStencil.stencilTestEnable = VK_FALSE;
  m_depthStencil.front = {};
  m_depthStencil.back = {};
  m_depthStencil.minDepthBounds = 0.f;
  m_depthStencil.maxDepthBounds = 1.f;
}


