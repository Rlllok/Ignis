#pragma once

#include <vulkan/vulkan.h>

class Instance;

class Device
{
public:
    Device(const Instance& isntance);

    Device(const Device&) = delete;

    Device(Device&&) = delete;

    Device& operator=(const Device&) = delete;

    Device& operator=(Device&&) = delete;

    ~Device();

public:
    const VkDevice&         getHandle() const { return device; }
    const VkPhysicalDevice& getGPU() const { return gpu; }
    uint32_t                getGraphicsQueueIndex() const { return graphicsQueueIndex; }

private:
    VkPhysicalDevice    gpu;
    VkDevice            device;
    uint32_t            graphicsQueueIndex;
};