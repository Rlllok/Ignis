#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "swapchain.h"

class ModelPipeline
{
public:
    ModelPipeline(const Device& device, const Swapchain& swapchain, const VkDescriptorSetLayout& descriptorLayout);

    ModelPipeline(const ModelPipeline&) = delete;
    
    ModelPipeline(ModelPipeline&&) = delete;

    ModelPipeline& operator=(const ModelPipeline&) = delete;

    ModelPipeline& operator=(ModelPipeline&&) = delete;

    operator VkPipeline() const { return pipeline; }

    ~ModelPipeline();

public:
    VkRenderPass                getRenderPass()     const { return renderPass; }
    VkPipelineLayout            getPipelineLayout() const { return layout; }

private:
    const Device&           device;
    const Swapchain&        swapchain;
    VkPipeline              pipeline;
    VkPipelineCache         pipelineCache = VK_NULL_HANDLE;
    VkDescriptorSetLayout   descriptorLayout;
    VkPipelineLayout        layout;
    VkRenderPass            renderPass;

private:
    bool createRenderPass();
    bool createPipeline();
};