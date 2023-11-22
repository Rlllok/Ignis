#include "commandPool.h"

#include <iostream>

#include "error.h"

CommandPool::CommandPool(const Device &device)
    : device(device)
{
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolInfo.queueFamilyIndex = device.getGraphicsQueueIndex();

    VK_CHECK_ERROR(vkCreateCommandPool(device.getHandle(), &cmdPoolInfo, nullptr, &cmdPool), "Cannot create Command Pool");
}

CommandPool::~CommandPool()
{
    vkDestroyCommandPool(device.getHandle(), cmdPool, nullptr);
}

void CommandPool::allocateCmdBuffers(uint32_t count)
{
    cmdBuffers.resize(count);

    VkCommandBufferAllocateInfo cmdBufferInfo = {};
    cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferInfo.commandPool = cmdPool;
    cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferInfo.commandBufferCount = count;

    VK_CHECK_ERROR(vkAllocateCommandBuffers(device.getHandle(), &cmdBufferInfo, cmdBuffers.data()), "Cannot allocate Command Buffer");
}
