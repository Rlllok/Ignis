#include "pipelineState.h"

#include <iostream>
#include <vector>

VertexInputState::VertexInputState()
    : stateInfo({})
{
    stateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    stateInfo.vertexBindingDescriptionCount = 0;
    stateInfo.pVertexBindingDescriptions = nullptr;
    stateInfo.vertexAttributeDescriptionCount = 0;
    stateInfo.pVertexAttributeDescriptions = nullptr;
}

InputAssemblyState::InputAssemblyState()
    : stateInfo({})
{
    stateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    stateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    stateInfo.primitiveRestartEnable = VK_FALSE;
}

ViewportState::ViewportState(uint32_t height, uint32_t width)
    : stateInfo({})
    , viewport({})
    , scissor({})
{
    viewport.x = 0;
    viewport.y = 0;
    viewport.height = height;
    viewport.width = width;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor.offset = {0, 0};
    scissor.extent.height = height;
    scissor.extent.width = width;

    stateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    stateInfo.viewportCount = 1;
    stateInfo.pViewports = &viewport;
    stateInfo.scissorCount = 1;
    stateInfo.pScissors = &scissor;
}

RasterizationState::RasterizationState()
    : stateInfo({})
{
    stateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    stateInfo.depthClampEnable = VK_FALSE;
    stateInfo.rasterizerDiscardEnable = VK_FALSE;
    stateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    stateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    stateInfo.depthBiasEnable = VK_FALSE;
    stateInfo.lineWidth = 1.0f;
}

MultisampleState::MultisampleState()
    : stateInfo({})
{
    stateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    stateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    stateInfo.sampleShadingEnable = VK_FALSE;
    stateInfo.minSampleShading = 0.0f;
    stateInfo.pSampleMask = nullptr;
    stateInfo.alphaToCoverageEnable = VK_FALSE;
    stateInfo.alphaToOneEnable = VK_FALSE;
}

DepthStencilState::DepthStencilState()
    : stateInfo({})
{
    stateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    stateInfo.depthTestEnable = VK_FALSE;
    stateInfo.depthWriteEnable = VK_FALSE;
    stateInfo.depthBoundsTestEnable = VK_FALSE;
    stateInfo.stencilTestEnable = VK_FALSE;
}

ColorBlendState::ColorBlendState()
    : stateInfo({})
    , colorBlendAttachment({})
{
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT| VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    stateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    stateInfo.logicOpEnable = VK_FALSE;
    stateInfo.attachmentCount = 1;
    stateInfo.pAttachments = &colorBlendAttachment;
}

DynamicState::DynamicState()
    : stateInfo({})
    , dynamicStates(0)
{
    dynamicStates = 
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    stateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    stateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    stateInfo.pDynamicStates = dynamicStates.data();
}