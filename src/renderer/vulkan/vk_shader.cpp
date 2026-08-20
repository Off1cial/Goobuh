#include <fstream>
#include <vulkan/vulkan_core.h>
#include "renderer/vulkan/vk_renderer.hpp"
#include "core/logsys.hpp"




using namespace VK;

std::vector<char> Renderer::PullShaderSource(const std::string& filename)
{
  std::fstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open()){
    LOG_ERROR("Failed to open shader source: %s", filename);
    return {};
  }
  size_t filesize = (size_t)file.tellg();
  std::vector<char> buff(filesize);

  // Read
  file.seekg(0);
  file.read(buff.data(), filesize);
  file.close();
  return buff;
}

VkShaderModule Renderer::CreateShaderModule(const std::vector<uint8_t>& spv)
{
  VkShaderModuleCreateInfo create_info{};
  create_info.sType    = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
  create_info.codeSize = spv.size();
  create_info.pCode    = reinterpret_cast<const uint32_t*>(spv.data());

  VkShaderModule shadermodule{};
  VkResult res = vkCreateShaderModule(m_device, &create_info, nullptr, &shadermodule);
  if (res != VK_SUCCESS){
    LOG_FATAL("Failed to create vulkan shader");
    return {};
  }
  return shadermodule;
}

VkShaderModule Renderer::CreateShaderFromSource(const std::string& sourcefile)
{
  std::vector<char> fcontents = PullShaderSource(sourcefile);
  if (fcontents.size() <= 0){
    LOG_ERROR("Failed to read shader source: %s", sourcefile);
    return {};
  }
}
