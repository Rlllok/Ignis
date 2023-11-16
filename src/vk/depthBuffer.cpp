#include "depthBuffer.h"

#include "error.h"
#include "vulkanUtils.hpp"

DepthBuffer::DepthBuffer(const Device &device, const VkExtent2D extent)
    : device(device)
    , extent(extent)
{
    createImage();
    allocateMemory();
    createImageView();
}

DepthBuffer::~DepthBuffer()
{
    vkDestroyImageView(device.getHandle(), depthImageView, nullptr);
    vkFreeMemory(device.getHandle(), depthImageMemory, nullptr);
    vkDestroyImage(device.getHandle(), depthImage, nullptr);
}

void DepthBuffer::createImage()
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_D32_SFLOAT;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.width = extent.width;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VK_CHECK_ERROR(vkCreateImage(device.getHandle(), &imageInfo, nullptr, &depthImage), "Cannot create Depth Image");
}

void DepthBuffer::allocateMemory()
{
    VkMemoryRequirements memoryRequirements = {};
    vkGetImageMemoryRequirements(device.getHandle(), depthImage, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = vku::findMemoryType(device.getGPU(), memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK_ERROR(vkAllocateMemory(device.getHandle(), &allocateInfo, nullptr, &depthImageMemory), "Cannot Allocate memory for Depth Image");

    vkBindImageMemory(device.getHandle(), depthImage, depthImageMemory, 0);
}

void DepthBuffer::createImageView()
{
    VkImageViewCreateInfo imageViewInfo = {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = depthImage;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = VK_FORMAT_D32_SFLOAT;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.layerCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;

    VK_CHECK_ERROR(vkCreateImageView(device.getHandle(), &imageViewInfo, nullptr, &depthImageView), "Cannot create Depth Image View");
}
