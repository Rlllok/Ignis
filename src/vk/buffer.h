#pragma once

#include <vulkan/vulkan.h>

#include "device.h"

class Buffer
{
public:
    Buffer(const Device& device, const VkDeviceSize size, const VkBufferUsageFlagBits usage);
    
    ~Buffer();

public:
    void map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();
    void flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

public:
    const VkBuffer& getHandle() const { return buffer; }
    void*           getMapped() const { return mapped; }

private:
    const Device& device;

    VkBuffer        buffer;
    VkDeviceMemory  memory;
    VkDeviceSize    size;
    
    void* mapped = nullptr;

private:
    void init();
    void createBuffer(const VkBufferUsageFlagBits usage);
    void allocateMemory();
};