#pragma once

#include <vulkan/vulkan.h>

#include "device.h"

class Texture
{
public:
    Texture(const Device& device, const VkCommandPool& cmdPool, const char* texturePath);
    ~Texture();

public:
    const VkImageView&  getImageView()  const { return textureImageView; }
    const VkSampler&    getSampler()    const { return textureSampler; }

private:
    const Device&           device;
    const VkCommandPool&    cmdPool;

    int width;
    int height;

    VkBuffer        stagingBuffer;
    VkDeviceMemory  stagingBufferMemory;

    VkImage         textureImage;
    VkImageView     textureImageView;
    VkDeviceMemory  textureImageMemory;
    VkSampler       textureSampler;

    VkCommandBuffer cmdBuffer;

private:
    void readFile(const char* texturePath);
    void createTextureImage();
    void createCmdBuffer();
    void copyBufferToImage();
    void changeImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void createImageView();
    void createSampler();

    void clearVulkan();
};