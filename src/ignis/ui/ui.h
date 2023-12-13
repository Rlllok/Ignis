#pragma once

#include <memory>

#include "ignis/vk/device.h"
#include "ignis/vk/commandPool.h"
#include "ignis/vk/buffer.h"

#include "glm/glm.hpp"

#include "elements/uiElement.h"

class UI
{
public:
    UI(const Device& device, const CommandPool& cmdPool, const VkRenderPass& renderPass);
    ~UI();

public:
    void prepareUIToDraw();
    void drawUI(const VkCommandBuffer& cmdBuffer);
    void addElement(IUIElement* element);

private:
    const Device&       device;
    const CommandPool&  cmdPool;
    const VkRenderPass  renderPass;

    VkImage         fontImage;
    VkDeviceMemory  fontImageMemory;
    VkImageView     fontImageView;

    VkBuffer        stagingBuffer;
    VkDeviceMemory  stagingBufferMemory;

    VkSampler sampler;

    VkDescriptorPool        descriptorPool;
    VkDescriptorSetLayout   descriptorSetLayout;
    VkDescriptorSet         descriptorSet;

    VkPipelineLayout    pipelineLayout;
    VkPipeline          pipeline;

    struct PushConstant
    {
        glm::vec2 scale;
        glm::vec2 translate;
    } pushConstant;

    std::unique_ptr<Buffer> vertexBuffer = nullptr;
    std::unique_ptr<Buffer> indexBuffer = nullptr;

    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;

    std::vector<IUIElement*> elements;

private:
    void init();
    void createStagingBuffer(VkDeviceSize bufferSize);
    void createSampler();
    void createDescriptorPool();
    void createDescriptorSetLayout();
    void allocateDescriptorSet();
    void createPipelineLayout();
    void createPipeline();
    void createBuffers();
    void updateBuffers();

    void newFrame();
};