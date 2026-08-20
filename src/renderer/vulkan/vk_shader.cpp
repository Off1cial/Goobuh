#include <fstream>
#include <vulkan/vulkan_core.h>
#include "renderer/vulkan/vk_renderer.hpp"
#include "core/logsys.hpp"
#include <filesystem>


using namespace VK;

std::vector<char> PullShaderSource(const std::string& filename)
{
  LOG_DEFAULT("Working dir: %s", std::filesystem::current_path().c_str());
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open()){
    LOG_ERROR("Failed to open shader source: %s", filename.data());
    return {};
  }
  size_t filesize = (size_t)file.tellg();
  std::vector<char> buff(filesize);

  // Read
  file.seekg(0);
  file.read(buff.data(), (std::streamsize)filesize);
  file.close();
  return buff;
}

VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& contents)
{
  if (device == VK_NULL_HANDLE){
    LOG_ERROR("Unable to create shader (null device)");
  }
  VkShaderModuleCreateInfo create_info{};
  create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = contents.size();
  create_info.pCode    = reinterpret_cast<const uint32_t*>(contents.data());

  VkShaderModule shadermodule{};
  VkResult res = vkCreateShaderModule(device, &create_info, nullptr, &shadermodule);
  if (res != VK_SUCCESS){
    LOG_FATAL("Failed to create vulkan shader");
    return {};
  }
  return shadermodule;
}

VkShaderModule CreateShaderFromSource(VkDevice device, const std::string& sourcefile)
{
  std::vector<char> fcontents = PullShaderSource(sourcefile);
  if (fcontents.size() <= 0){
    LOG_ERROR("Failed to read shader source: %s", sourcefile.data());
    return {};
  }
  return CreateShaderModule(device, fcontents);
}




Shader::Shader(const VkDevice vkdevice, const std::string& vertsrc, const std::string& fragsrc) : m_device(vkdevice)
{
  if (vkdevice == VK_NULL_HANDLE){
    LOG_ERROR("Failed to create shader, (null device)");
    return;
  }
  //m_device = vkdevice;  class Pipeline;
  class Shader;
  m_vertmodule = CreateShaderFromSource(vkdevice, vertsrc);
  m_fragmodule = CreateShaderFromSource(vkdevice, fragsrc);
}


Shader::~Shader()
{
  vkDestroyShaderModule(m_device, m_fragmodule, nullptr);
  vkDestroyShaderModule(m_device, m_vertmodule, nullptr);
}
