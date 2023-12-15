#include "graphicsPipeline.h"

#include <array>
#include <iostream>

#include "pipelineState.h"
#include "realTimeShader.h"
#include "error.h"

GraphicsPipeline::GraphicsPipeline(const Device& device, const Swapchain& swapchain)
    : device(device)
    , swapchain(swapchain)
{
   createPipeline();
}

GraphicsPipeline::~GraphicsPipeline()
{
    vkDestroyPipelineLayout(device.getHandle(), layout, nullptr);
    vkDestroyRenderPass(device.getHandle(), renderPass, nullptr);
    vkDestroyPipeline(device.getHandle(), pipeline, nullptr);
    vkDestroyPipelineCache(device.getHandle(), pipelineCache, nullptr);
}

bool GraphicsPipeline::recreatePipeline()
{
    vkDeviceWaitIdle(device.getHandle());

    RealTimeShader vertexShader = RealTimeShader(device, "shaders/FullQuad.vert", RealTimeShader::Stage::S_VERTEX);
    RealTimeShader fragmentShader = RealTimeShader(device, "shaders/RM.frag", RealTimeShader::Stage::S_FRAGMENT);

    if (!vertexShader.compile() || !fragmentShader.compile()) return false;
    
    vkDestroyPipelineLayout(device.getHandle(), layout, nullptr);
    vkDestroyRenderPass(device.getHandle(), renderPass, nullptr);
    vkDestroyPipeline(device.getHandle(), pipeline, nullptr);

    return createPipeline();
}

bool GraphicsPipeline::createPipeline()
{
     // Shaders
    RealTimeShader vertexShader = RealTimeShader(device, "shaders/FullQuad.vert", RealTimeShader::Stage::S_VERTEX);
    RealTimeShader fragmentShader = RealTimeShader(device, "shaders/RM.frag", RealTimeShader::Stage::S_FRAGMENT);

    if (!vertexShader.compile() || !fragmentShader.compile()) return false;

    VkPipelineShaderStageCreateInfo vertexStageInfo = {};
    vertexStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStageInfo.module = vertexShader.getVkModule();
    vertexStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentStageInfo = {};
    fragmentStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStageInfo.module = fragmentShader.getVkModule();
    fragmentStageInfo.pName = "main";

    std::array<VkPipelineShaderStageCreateInfo, 2> stages = 
    {
        vertexStageInfo,
        fragmentStageInfo
    };

    // Vertex Input State
    VertexInputState vertexInputState;

    // Input Assembly State
    InputAssemblyState InputAssemblyState;

    // Viewport State
    ViewportState viewportState(swapchain.getExtent().height, swapchain.getExtent().width);

    // Rasterization State
    RasterizationState rasterizationState;

    // Multisample State
    MultisampleState multisampleState;

    // Depth-Stencil State
    DepthStencilState depthStencilState;

    // Color Blend State
    ColorBlendState colorBlendState;

    // Pipeline Layout
    struct PushConstant
    {
        double colorValue;
    };

    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstant);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 0;
    layoutInfo.pSetLayouts = nullptr;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;

    VK_CHECK_ERROR(vkCreatePipelineLayout(device.getHandle(), &layoutInfo, nullptr, &layout), "Cannot create Graphics Pipeline Layout");

    // Render Pass
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapchain.getSurfaceFormat().format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0;
    renderPassInfo.pDependencies = nullptr;

    VK_CHECK_ERROR(vkCreateRenderPass(device.getHandle(), &renderPassInfo, nullptr, &renderPass), "Cannot create Redner Pass");

    // Dynamic State
    DynamicState dynamicState;

    // Pipeline creation
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(stages.size());
    pipelineInfo.pStages = stages.data();
    pipelineInfo.pVertexInputState = &vertexInputState.stateInfo;
    pipelineInfo.pInputAssemblyState = &InputAssemblyState.stateInfo;
    pipelineInfo.pTessellationState = nullptr;
    pipelineInfo.pViewportState = &viewportState.stateInfo;
    pipelineInfo.pRasterizationState = &rasterizationState.stateInfo;
    pipelineInfo.pMultisampleState = &multisampleState.stateInfo;
    pipelineInfo.pDepthStencilState = &depthStencilState.stateInfo;
    pipelineInfo.pColorBlendState = &colorBlendState.stateInfo;
    pipelineInfo.pDynamicState = &dynamicState.stateInfo;
    pipelineInfo.layout = layout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = nullptr;

    if (pipelineCache == VK_NULL_HANDLE)
    {
        VkPipelineCacheCreateInfo cacheInfo = {};
        cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

        VK_CHECK_ERROR(vkCreatePipelineCache(device.getHandle(), &cacheInfo, nullptr, &pipelineCache), "Cannot create Pipeline Cache");
    }

    VK_CHECK_ERROR(vkCreateGraphicsPipelines(device.getHandle(), pipelineCache, 1,  &pipelineInfo, nullptr, &pipeline), "Cannot create GraphicsPipeline");

    return true;
}
