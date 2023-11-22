#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "device.h"

class CommandPool
{
public:
    CommandPool(const Device& device);

    CommandPool(const CommandPool&) = delete;

    CommandPool(CommandPool&&) = delete;

    CommandPool& operator=(const CommandPool&) = delete;

    CommandPool& operator=(CommandPool&&) = delete;

    ~CommandPool();

public:
    const VkCommandPool& getHandle() const { return cmdPool; }

    VkCommandBuffer getCmdBuffer(uint32_t index) const { return cmdBuffers[index]; }

    void allocateCmdBuffers(uint32_t count);

private:
    const Device&   device;
    VkCommandPool   cmdPool;
    std::vector<VkCommandBuffer> cmdBuffers;
};