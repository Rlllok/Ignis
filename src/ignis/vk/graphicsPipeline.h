#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "swapchain.h"

class GraphicsPipeline
{
public:
    GraphicsPipeline(const Device& device, const Swapchain& swapchain);

    GraphicsPipeline(const GraphicsPipeline&) = delete;
    
    GraphicsPipeline(GraphicsPipeline&&) = delete;

    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    GraphicsPipeline& operator=(GraphicsPipeline&&) = delete;

    operator VkPipeline() const { return pipeline; }

    ~GraphicsPipeline();

public:
    VkRenderPass                getRenderPass()     const { return renderPass; }
    VkPipelineLayout            getPipelineLayout() const { return layout; }

    void bindPipeline(const VkCommandBuffer& cmdBuffer) const;

public:
    bool recreatePipeline();

private:
    const Device&       device;
    const Swapchain&    swapchain;
    VkPipeline          pipeline;
    VkPipelineCache     pipelineCache = VK_NULL_HANDLE;
    VkPipelineLayout    layout;
    VkRenderPass        renderPass;

private:
    bool createPipeline();
};