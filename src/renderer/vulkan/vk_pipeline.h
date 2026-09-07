#ifndef VULKAN_PIPELINE_H
#define VULKAN_PIPELINE_H

#include "volk/volk.h"

enum ShaderStages
{
  SHADER_STAGE_VERTEX = 0,
  SHADER_STAGE_FRAGMENT,
  SHADER_STAGE_COUNT // not an enum of use
};

typedef struct VKPipelineSet
{
  
  VkPipelineShaderStageCreateInfo shader_stages[SHADER_STAGE_COUNT];
  uint32_t shader_stage_count;
  VkPipelineInputAssemblyStateCreateInfo input_assembly;
  VkPipelineRasterizationStateCreateInfo rasterizer;
  VkPipelineColorBlendAttachmentState colblend_attachment_state;
  VkFormat colattachment_format;
  VkPipelineMultisampleStateCreateInfo multisampling;
  VkPipelineDepthStencilStateCreateInfo depth_stencil;
  VkPipelineRenderingCreateInfo rendering;
  
  VkVertexInputBindingDescription vertex_binding;
  VkVertexInputAttributeDescription vertex_attributes[4];

  VkPipelineLayout layout;
} VKPipelineSet;


typedef struct VK_Renderer VK_Renderer;

// Pipeline set fillers
void VKPipeline_set_shaders(VKPipelineSet* set, VkShaderModule vertex_module, VkShaderModule fragment_module);
void VKPipeline_set_topology(VKPipelineSet* set, VkPrimitiveTopology mode);
void VKPipeline_set_polygonmode(VKPipelineSet* set, VkPolygonMode mode);
void VKPipeline_set_cull_mode(VKPipelineSet* set, VkCullModeFlags mode, VkFrontFace frontface);
  

VkPipeline VKPipeline_build(VK_Renderer* engine, VKPipelineSet* set);
void VKPipeline_clear(VKPipelineSet* set);



#endif
