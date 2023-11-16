#pragma once

#include <vulkan/vulkan.h>

#include "device.h"

class DepthBuffer
{
public:
    DepthBuffer(const Device &device, const VkExtent2D extent);
    ~DepthBuffer();

public:
    const VkImageView& getImageView() const { return depthImageView; }

private:
    const Device&       device;
    const VkExtent2D    extent;

    VkImage         depthImage;
    VkDeviceMemory  depthImageMemory;
    VkImageView     depthImageView;

private:
    void createImage();
    void allocateMemory();
    void createImageView();
};