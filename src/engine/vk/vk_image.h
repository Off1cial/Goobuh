#ifndef VKIMAGE_H
#define VKIMAGE_H

#include "core/util/rc.h"
#include <vulkan/vulkan.hpp>

struct VKImageState
{
    vk::ImageLayout layout = vk::ImageLayout::eUndefined;
    vk::PipelineStageFlags2 stageMask = vk::PipelineStageFlagBits2::eNone;
    vk::AccessFlags2 accessMask = vk::AccessFlagBits2::eNone;
    vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor;

    bool operator==(const VKImageState&) const = default;
};

class CVKImage : public RcObject
{
public:
    void Transition(vk::CommandBuffer cmdBuffer, const VKImageState& state );
private:
};


#endif