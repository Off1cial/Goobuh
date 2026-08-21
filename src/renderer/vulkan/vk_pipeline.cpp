#include "renderer/vulkan/vk_renderer.hpp"
#include "core/logsys.hpp"
#include <random>
#include <regex>
#include <vulkan/vulkan_core.h>

using namespace VK;

/*
Pipeline::~Pipeline()
{
  if (m_pipeline != VK_NULL_HANDLE)
    vkDestroyPipeline(m_device, m_pipeline, nullptr);

  if (m_layout != VK_NULL_HANDLE)
    vkDestroyPipelineLayout(m_device, m_layout, nullptr);
}

Pipeline::Pipeline(VkDevice device, const Shader &shader, VkFormat format)
    : m_device(device)
{
  if (device == VK_NULL_HANDLE)
  {
    LOG_ERROR("Failed to create pipeline (null device)");
    return;
  }
  if (shader.GetModule_Vertex() == VK_NULL_HANDLE ||
      shader.GetModule_Fragment() == VK_NULL_HANDLE)
  {
    LOG_ERROR("Failed to create pipeline, invalid shader parameters");
    return;
  }

  VkPipelineLayoutCreateInfo layout_info{};
  layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  VkResult result = vkCreatePipelineLayout(device, &layout_info, nullptr, &m_layout);
  if (result != VK_SUCCESS)
  {
    LOG_ERROR("Failed to create pipeline layout");
    return;
  }

  // Create shader stages
  VkPipelineShaderStageCreateInfo shader_stages[2]{};
  // Stage 0 - Vertex
  shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shader_stages[0].module = shader.GetModule_Vertex();
  shader_stages[0].pName = "main";
  // Stage 1 - Fragment
  shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shader_stages[1].module = shader.GetModule_Fragment();
  shader_stages[1].pName = "main";

  // TODO: SETUP VERTEX BUFFERS
  VkPipelineVertexInputStateCreateInfo vertex_input{};
  vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  // hard coded triangle for now
  VkPipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // like GLenum GL_TRIANGLES
  input_assembly.primitiveRestartEnable = VK_FALSE;

  // Viewport provided in the commands
  // Make vulkan aware
  VkPipelineViewportStateCreateInfo viewport_state{};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.scissorCount = 1;
  VkPipelineRasterizationStateCreateInfo raster_info{};
  // TODO: Add a way to input these -> a pipeline creation system
  raster_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  raster_info.depthClampEnable = VK_FALSE;
  raster_info.rasterizerDiscardEnable = VK_FALSE;
  raster_info.polygonMode = VK_POLYGON_MODE_FILL;
  raster_info.cullMode = VK_CULL_MODE_BACK_BIT;
  raster_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
  raster_info.depthBiasEnable = VK_FALSE;
  raster_info.lineWidth = 1.0f;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.rasterizationSamples =
      VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState colour_blend_attachment{};
  colour_blend_attachment.blendEnable = VK_FALSE;
  colour_blend_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT |
      VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colour_blending{};
  colour_blending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colour_blending.logicOpEnable = VK_FALSE;
  colour_blending.attachmentCount = 1;
  colour_blending.pAttachments =
      &colour_blend_attachment;

  VkDynamicState dynamic_states[] =
      {
          VK_DYNAMIC_STATE_VIEWPORT,
          VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamic_state{};
  dynamic_state.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount = 2;
  dynamic_state.pDynamicStates = dynamic_states;

  // Dynamic rendering: tell Vulkan what we're rendering into.
  VkPipelineRenderingCreateInfo rendering_info{};
  rendering_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  rendering_info.colorAttachmentCount = 1;
  rendering_info.pColorAttachmentFormats =
      &format;

  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType =
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

  pipeline_info.pNext = &rendering_info;

  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stages;

  pipeline_info.pVertexInputState = &vertex_input;
  pipeline_info.pInputAssemblyState = &input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &raster_info;
  pipeline_info.pMultisampleState = &multisampling;
  pipeline_info.pColorBlendState = &colour_blending;
  pipeline_info.pDynamicState = &dynamic_state;

  pipeline_info.layout = m_layout;

  result = vkCreateGraphicsPipelines(
      m_device,
      VK_NULL_HANDLE,
      1,
      &pipeline_info,
      nullptr,
      &m_pipeline);

  if (result != VK_SUCCESS)
    LOG_FATAL("Failed to create graphics pipeline");
}

*/
// ---------------------------------------------------------------------------------------------
//
//
//
//
//  IMPLEMENTING THE NEW CLASS
//
//
//
//
//
//
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

  VkPipelineLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

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
  VkPipelineVertexInputStateCreateInfo _vertexInputInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

  // build the actual pipeline
  // we now use all of the info structs we have been writing into into this one
  // to create the pipeline
  VkGraphicsPipelineCreateInfo pipelineInfo = {.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
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


