#pragma once

#include "volk/volk.h"

namespace VK
{
  typedef uint32_t TextureHandle;

  struct GPUTexture
  {
    VkImage image = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkExtent2D extent{};
    VkFormat format = VK_FORMAT_UNDEFINED;
    uint32_t mipLevels = 1;
  };

  // Filtering/wrap state, deduplicated — many textures share one sampler.
  class Sampler
  {
  public:
    VkSampler Get(VkDevice device, const VkSamplerCreateInfo &desc);
    void Destroy(VkDevice device);

  private:
    struct Key
    {
      VkFilter mag_filter, min_filter;
      VkSamplerMipmapMode mipmap_mode;
      VkSamplerAddressMode address_mode;
      float max_lod;
      bool operator==(const Key &) const = default;
    };
    struct KeyHash
    {
      size_t operator()(const Key &k) const;
    };
    std::unordered_map<Key, VkSampler, KeyHash> m_cache;
  };

};