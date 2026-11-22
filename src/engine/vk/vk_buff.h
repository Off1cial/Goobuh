#ifndef __VK_BUFF_H__
#define __VK_BUFF_H__

#include "core/util/rc.h"
#include <vulkan/vulkan.hpp>

class CVKBuff : public RcObject
{
public:
    CVKBuff()  {}
    ~CVKBuff() = default;

    //void Init();
protected:
    vk::UniqueDeviceMemory m_memory{};
    vk::UniqueBuffer m_buffer{};
    vk::DeviceAddress m_gpuAddress{};
    vk::DeviceSize m_size = 0;
    void *m_cpuAddress = nullptr;
};

#endif