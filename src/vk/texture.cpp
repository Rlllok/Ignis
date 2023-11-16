#include "texture.h"

#include <stdexcept>
#include <string>

#include <stb_image.h>

#include "error.h"
#include "vulkanUtils.hpp"

Texture::Texture(const Device& device, const VkCommandPool& cmdPool, const char *texturePath)
    : device(device)
    , cmdPool(cmdPool)
{
    createCmdBuffer();
    readFile(texturePath);
    createImageView();
    createSampler();
}

Texture::~Texture()
{
    clearVulkan();
}

void Texture::readFile(const char *texturePath)
{
    int textureWidth;
    int textureHeight;
    int textureChannels;

    stbi_uc* pixels = stbi_load(texturePath, &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);

    VkDeviceSize imageSize = textureWidth * textureHeight * 4;

    if (!pixels)
    {
        throw std::runtime_error("Failed to load texture " + std::string(texturePath));
    }

    width = textureWidth;
    height = textureHeight;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = imageSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK_ERROR(vkCreateBuffer(device.getHandle(), &bufferInfo, nullptr, &stagingBuffer), "Cannot create staging buffer for texture " + std::string(texturePath));

    VkMemoryRequirements memoryRequirements = {};
    vkGetBufferMemoryRequirements(device.getHandle(), stagingBuffer, &memoryRequirements);

    VkMemoryPropertyFlags memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = vku::findMemoryType(device.getGPU(), memoryRequirements.memoryTypeBits, memProperties);

    VK_CHECK_ERROR(vkAllocateMemory(device.getHandle(), &allocateInfo, nullptr, &stagingBufferMemory), "Cannot allocate stagin buffer's memory for texture" + std::string(texturePath));

    vkBindBufferMemory(device.getHandle(), stagingBuffer, stagingBufferMemory, 0);

    void* data;
    vkMapMemory(device.getHandle(), stagingBufferMemory, 0, imageSize, 0, &data);
    {
        memcpy(data, pixels, static_cast<size_t>(imageSize));
    }
    vkUnmapMemory(device.getHandle(), stagingBufferMemory);

    stbi_image_free(pixels);

    createTextureImage();

    changeImageLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage();
    changeImageLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkFreeMemory(device.getHandle(), stagingBufferMemory, nullptr);
    vkDestroyBuffer(device.getHandle(), stagingBuffer, nullptr);
}

void Texture::createTextureImage()
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VK_CHECK_ERROR(vkCreateImage(device.getHandle(), &imageInfo, nullptr, &textureImage), "Cannot create Image for texture.");

    VkMemoryRequirements memoryRequirements = {};
    vkGetImageMemoryRequirements(device.getHandle(), textureImage, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = vku::findMemoryType(device.getGPU(), memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    VK_CHECK_ERROR(vkAllocateMemory(device.getHandle(), &allocateInfo, nullptr, &textureImageMemory), "Cannot allocate Image Memory for texture.");

    vkBindImageMemory(device.getHandle(), textureImage, textureImageMemory, 0);
}   

void Texture::clearVulkan()
{
    vkDestroyImage(device.getHandle(), textureImage, nullptr);
    vkFreeMemory(device.getHandle(), textureImageMemory, nullptr);
    vkDestroyImageView(device.getHandle(), textureImageView, nullptr);
    vkDestroySampler(device.getHandle(), textureSampler, nullptr);
}

void Texture::createCmdBuffer()
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = cmdPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    VK_CHECK_ERROR(vkAllocateCommandBuffers(device.getHandle(), &allocateInfo, &cmdBuffer), "Cannot create Command Buffer to copy Texture");
}

void Texture::copyBufferToImage()
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmdBuffer, &beginInfo);
    {
        VkExtent3D textureExtent = {};
        textureExtent.width = width;
        textureExtent.height = height;
        textureExtent.depth = 1;

        VkBufferImageCopy copyInfo = {};
        copyInfo.bufferOffset = 0;
        copyInfo.bufferRowLength = 0;
        copyInfo.bufferImageHeight = 0;
        copyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyInfo.imageSubresource.layerCount = 1;
        copyInfo.imageSubresource.baseArrayLayer = 0;
        copyInfo.imageSubresource.mipLevel = 0;
        copyInfo.imageOffset = {0, 0, 0};
        copyInfo.imageExtent = textureExtent;

        vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);
    }
    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    VkFence flushFence;
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    vkCreateFence(device.getHandle(), &fenceInfo, nullptr, &flushFence);

    VkQueue graphicsQueue;
    vkGetDeviceQueue(device.getHandle(), device.getGraphicsQueueIndex(), 0, &graphicsQueue);

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, flushFence);

    vkWaitForFences(device.getHandle(), 1, &flushFence, VK_TRUE, UINT32_MAX);

    vkDestroyFence(device.getHandle(), flushFence, nullptr);
}

void Texture::changeImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmdBuffer, &beginInfo);
    {
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = textureImage;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseMipLevel = 0;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("ERROR: unsupported image layout transition.");
		}

		vkCmdPipelineBarrier(
			cmdBuffer,
			srcStage,
			dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
    }
	vkEndCommandBuffer(cmdBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	VkFence flushFence;
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(device.getHandle(), &fenceInfo, nullptr, &flushFence);

    VkQueue graphicsQueue;
    vkGetDeviceQueue(device.getHandle(), device.getGraphicsQueueIndex(), 0, &graphicsQueue);

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, flushFence);

	vkWaitForFences(device.getHandle(), 1, &flushFence, VK_TRUE, UINT32_MAX);
	vkDestroyFence(device.getHandle(), flushFence, nullptr);
}

void Texture::createImageView()
{
    VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = textureImage;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.layerCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseMipLevel = 0;

    VK_CHECK_ERROR(vkCreateImageView(device.getHandle(), &imageViewInfo, nullptr, &textureImageView), "Cannot create Image View for texture");
}

void Texture::createSampler()
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 0;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias  = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VK_CHECK_ERROR(vkCreateSampler(device.getHandle(), &samplerInfo, nullptr, &textureSampler), "Cannot create Sampler for texture");
}
