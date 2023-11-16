#include "buffer.h"

#include <iostream>

#include "error.h"
#include "vulkanUtils.hpp"

Buffer::Buffer(const Device &device, const VkDeviceSize size, const VkBufferUsageFlagBits usage)
    : device(device)
    , size(size)
{
    createBuffer(usage);
    allocateMemory();
}

Buffer::~Buffer()
{
    vkDeviceWaitIdle(device.getHandle());

    vkDestroyBuffer(device.getHandle(), buffer, nullptr);
    vkFreeMemory(device.getHandle(), memory, nullptr);
}

void Buffer::map(VkDeviceSize size, VkDeviceSize offset)
{
    vkMapMemory(device.getHandle(), memory, offset, size, 0, &mapped);
}

void Buffer::unmap()
{
    if (!mapped) return;

    vkUnmapMemory(device.getHandle(), memory);
    mapped = nullptr;
}

void Buffer::flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange memoryRange = {};
    memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    memoryRange.memory = memory;
    memoryRange.offset = offset;
    memoryRange.size = size;

    vkFlushMappedMemoryRanges(device.getHandle(), 1, &memoryRange);
}

void Buffer::init()
{
}

void Buffer::createBuffer(const VkBufferUsageFlagBits usage)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK_ERROR(vkCreateBuffer(device.getHandle(), &bufferInfo, nullptr, &buffer), "Cannot create Buffer");
}

void Buffer::allocateMemory()
{
    VkMemoryRequirements memoryRequirements = {};
    vkGetBufferMemoryRequirements(device.getHandle(), buffer, &memoryRequirements);

    VkMemoryPropertyFlags memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = vku::findMemoryType(device.getGPU(), memoryRequirements.memoryTypeBits, memProperties);
    
    VK_CHECK_ERROR(vkAllocateMemory(device.getHandle(), &memoryAllocateInfo, nullptr, &memory), "Cannot allocate Buffer Memory");

    vkBindBufferMemory(device.getHandle(), buffer, memory, 0);
}
