#include "renderer/vulkan/vk_renderer.hpp"
#include "core/logsys.hpp"
#include <vulkan/vulkan_core.h>


using namespace VK;

Pipeline::~Pipeline()
{
  if (m_pipeline != VK_NULL_HANDLE)
    vkDestroyPipeline(m_device, m_pipeline, nullptr);

  if (m_layout != VK_NULL_HANDLE)
    vkDestroyPipelineLayout(m_device, m_layout, nullptr);
}


Pipeline::Pipeline(VkDevice device, const Shader& shader, VkFormat format)
  : m_device(device)
{
  if (device == VK_NULL_HANDLE){
    LOG_ERROR("Failed to create pipeline (null device)");
    return;
  }
  if (shader.GetModule_Vertex() == VK_NULL_HANDLE ||
      shader.GetModule_Fragment() == VK_NULL_HANDLE){
    LOG_ERROR("Failed to create pipeline, invalid shader parameters");
    return;
  }
  
  VkPipelineLayoutCreateInfo layout_info{};
  layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  VkResult result = vkCreatePipelineLayout(device, &layout_info, nullptr, &m_layout);
  if (result != VK_SUCCESS){
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
      VK_DYNAMIC_STATE_SCISSOR
  };

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

