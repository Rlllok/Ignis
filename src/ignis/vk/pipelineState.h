#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class VertexInputState
{
public:
    VertexInputState();

protected:
    friend class GraphicsPipeline;

private:
    VkPipelineVertexInputStateCreateInfo stateInfo;
};

class InputAssemblyState
{
public:
    InputAssemblyState();

protected:
    friend class GraphicsPipeline;

private:
    VkPipelineInputAssemblyStateCreateInfo stateInfo;
};

class ViewportState
{
public:
    ViewportState(uint32_t height, uint32_t width);

protected:
    friend class GraphicsPipeline;

private:
    VkPipelineViewportStateCreateInfo stateInfo;

    VkViewport  viewport;
    VkRect2D    scissor;
};

class RasterizationState
{
public:
    RasterizationState();

protected:
    friend class GraphicsPipeline;

private:
    VkPipelineRasterizationStateCreateInfo stateInfo;
};

class MultisampleState
{
public:
    MultisampleState();

protected:
    friend class GraphicsPipeline;

private:
    VkPipelineMultisampleStateCreateInfo stateInfo;
};

class DepthStencilState
{
public:
    DepthStencilState();

protected:
    friend class GraphicsPipeline;

private:
    VkPipelineDepthStencilStateCreateInfo stateInfo;
};

class ColorBlendState
{
public:
    ColorBlendState();

protected:
    friend class GraphicsPipeline;

private:
    VkPipelineColorBlendStateCreateInfo stateInfo;

    VkPipelineColorBlendAttachmentState colorBlendAttachment;
};

class DynamicState
{
public:
    DynamicState();

protected:
    friend class GraphicsPipeline;

private:
    VkPipelineDynamicStateCreateInfo stateInfo;

    std::vector<VkDynamicState> dynamicStates;
};