#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class VertexInputState
{
public:
    VertexInputState();

    VkPipelineVertexInputStateCreateInfo stateInfo;

protected:
    friend class GraphicsPipeline;

private:
};

class InputAssemblyState
{
public:
    InputAssemblyState();
    
    VkPipelineInputAssemblyStateCreateInfo stateInfo;

protected:
    friend class GraphicsPipeline;

private:
};

class ViewportState
{
public:
    ViewportState(uint32_t height, uint32_t width);

    VkPipelineViewportStateCreateInfo stateInfo;

protected:
    friend class GraphicsPipeline;

private:
    VkViewport  viewport;
    VkRect2D    scissor;
};

class RasterizationState
{
public:
    RasterizationState();

    VkPipelineRasterizationStateCreateInfo stateInfo;

protected:
    friend class GraphicsPipeline;

private:
};

class MultisampleState
{
public:
    MultisampleState();

    VkPipelineMultisampleStateCreateInfo stateInfo;

protected:
    friend class GraphicsPipeline;

private:
};

class DepthStencilState
{
public:
    DepthStencilState();

    VkPipelineDepthStencilStateCreateInfo stateInfo;

protected:
    friend class GraphicsPipeline;

private:
};

class ColorBlendState
{
public:
    ColorBlendState();

    VkPipelineColorBlendStateCreateInfo stateInfo;

protected:
    friend class GraphicsPipeline;

private:

    VkPipelineColorBlendAttachmentState colorBlendAttachment;
};

class DynamicState
{
public:
    DynamicState();

    VkPipelineDynamicStateCreateInfo stateInfo;

protected:
    friend class GraphicsPipeline;

private:

    std::vector<VkDynamicState> dynamicStates;
};