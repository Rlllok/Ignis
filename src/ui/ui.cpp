#include "ui.h"

#include <vector>
#include <iostream>

#include <imgui.h>

#include "vk/error.h"
#include "vk/vulkanUtils.hpp"
#include "vk/realTimeShader.h"

// TO REMOVE AFTER TEST
#include "input/win_mouse.h"

UI::UI(const Device &device, const CommandPool& cmdPool, const VkRenderPass& renderPass)
    : device(device)
    , cmdPool(cmdPool)
    , renderPass(renderPass)
{
    init();
}

UI::~UI()
{
    ImGui::DestroyContext();
}

void UI::prepareUIToDraw()
{
    ImGuiIO& io = ImGui::GetIO();

    io.MousePos = ImVec2(Mouse::getX(), Mouse::getY());
    io.MouseDown[0] = Mouse::isLeftDown();
    io.MouseDown[1] = Mouse::isRightDown();

    newFrame();
    updateBuffers();
}

void UI::drawUI(const VkCommandBuffer& cmdBuffer)
{
    ImGuiIO& io = ImGui::GetIO();

    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkViewport viewport = {};
    viewport.width = ImGui::GetIO().DisplaySize.x;
    viewport.height = ImGui::GetIO().DisplaySize.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

    pushConstant.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
    pushConstant.translate = glm::vec2(-1.0f);

    vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), &pushConstant);

    ImDrawData* imDrawData = ImGui::GetDrawData();
    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;

    if (imDrawData->CmdListsCount > 0)
    {
        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &vertexBuffer->getHandle(), offsets);
        vkCmdBindIndexBuffer(cmdBuffer, indexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT16);

        for (int i = 0; i < imDrawData->CmdListsCount; i++)
        {
            const ImDrawList* cmdList = imDrawData->CmdLists[i];

            for (int j = 0; j < cmdList->CmdBuffer.Size; j++)
            {
                const ImDrawCmd* command = &cmdList->CmdBuffer[j];

                VkRect2D scissorRect = {};
                scissorRect.offset.x = std::max((int32_t)(command->ClipRect.x), 0);
                scissorRect.offset.y = std::max((int32_t)(command->ClipRect.y), 0);
                scissorRect.extent.width = (uint32_t)(command->ClipRect.z - command->ClipRect.x);
                scissorRect.extent.height = (uint32_t)(command->ClipRect.w - command->ClipRect.y);

                vkCmdSetScissor(cmdBuffer, 0, 1, &scissorRect);

                vkCmdDrawIndexed(cmdBuffer, command->ElemCount, 1, indexOffset, vertexOffset, 0);

                indexOffset += command->ElemCount;
            }

            vertexOffset += cmdList->VtxBuffer.Size;
        }
    }
}

void UI::addElement(IUIElement *element)
{
    if (element == nullptr) return;

    elements.push_back(element);
}

void UI::init()
{
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = 1280;
    io.DisplaySize.y = 720;

    unsigned char* fontData;
    int textWidth;
    int textHeight;

    io.Fonts->GetTexDataAsRGBA32(&fontData, &textWidth, &textHeight);
    
    VkDeviceSize fontSize = textWidth * textHeight * 4 * sizeof(char);

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.extent.width = textWidth;
    imageInfo.extent.height = textHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VK_CHECK_ERROR(vkCreateImage(device.getHandle(), &imageInfo, nullptr, &fontImage), "Cannot create Font Image for ImGui");

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device.getHandle(), fontImage, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = vku::findMemoryType(device.getGPU(), memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK_ERROR(vkAllocateMemory(device.getHandle(), &allocateInfo, nullptr, &fontImageMemory), "Cannot allocate Font Image Memory for ImGui");

    VK_CHECK_ERROR(vkBindImageMemory(device.getHandle(), fontImage, fontImageMemory, 0), "Cannot bind Font Image Memory for ImGui");

    VkImageViewCreateInfo imageViewInfo = {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = fontImage;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.layerCount = 1;

    VK_CHECK_ERROR(vkCreateImageView(device.getHandle(), &imageViewInfo, nullptr, &fontImageView), "Cannot create Font Image View for ImGui");

    createStagingBuffer(fontSize);

    void* data;
    vkMapMemory(device.getHandle(), stagingBufferMemory, 0, fontSize, 0, &data);
    {
        memcpy(data, fontData, static_cast<size_t>(fontSize));
    }
    vkUnmapMemory(device.getHandle(), stagingBufferMemory);

    VkCommandBuffer copyBuffer;

    VkCommandBufferAllocateInfo bufferAllocateInfo = {};
    bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    bufferAllocateInfo.commandPool = cmdPool.getHandle();
    bufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    bufferAllocateInfo.commandBufferCount = 1;

    VK_CHECK_ERROR(vkAllocateCommandBuffers(device.getHandle(), &bufferAllocateInfo, &copyBuffer), "Cannot create Command Buffer to copy Font Image");

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(copyBuffer, &beginInfo);
    {
        vku::changeImageLayout(
            copyBuffer,
            fontImage,
            imageViewInfo.subresourceRange,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_HOST_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT
        );

        VkExtent3D extent = {};
        extent.width = textWidth;
        extent.height = textHeight;
        extent.depth = 1;

        VkBufferImageCopy copyInfo = {};
        copyInfo.bufferOffset = 0;
        copyInfo.bufferRowLength = 0;
        copyInfo.bufferImageHeight = 0;
        copyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyInfo.imageSubresource.layerCount = 1;
        copyInfo.imageSubresource.baseArrayLayer = 0;
        copyInfo.imageSubresource.mipLevel = 0;
        copyInfo.imageOffset = {0, 0, 0};
        copyInfo.imageExtent = extent;

        vkCmdCopyBufferToImage(copyBuffer, stagingBuffer, fontImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

        vku::changeImageLayout(
            copyBuffer,
            fontImage,
            imageViewInfo.subresourceRange,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
        );
    }
    vkEndCommandBuffer(copyBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &copyBuffer;

    VkFence flushFence;
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    vkCreateFence(device.getHandle(), &fenceInfo, nullptr, &flushFence);

    VkQueue graphicsQueue;
    vkGetDeviceQueue(device.getHandle(), device.getGraphicsQueueIndex(), 0, &graphicsQueue);

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, flushFence);

    vkWaitForFences(device.getHandle(), 1, &flushFence, VK_TRUE, UINT32_MAX);

    vkDestroyFence(device.getHandle(), flushFence, nullptr);

    createSampler();
    createDescriptorPool();
    createDescriptorSetLayout();
    allocateDescriptorSet();
    createPipelineLayout();
    createPipeline();
    // createBuffers();
}

void UI::createStagingBuffer(VkDeviceSize bufferSize)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK_ERROR(vkCreateBuffer(device.getHandle(), &bufferInfo, nullptr, &stagingBuffer), "Cannot create staging buffer for Font Image");

    VkMemoryRequirements memoryRequirements = {};
    vkGetBufferMemoryRequirements(device.getHandle(), stagingBuffer, &memoryRequirements);

    VkMemoryPropertyFlags memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = vku::findMemoryType(device.getGPU(), memoryRequirements.memoryTypeBits, memProperties);

    VK_CHECK_ERROR(vkAllocateMemory(device.getHandle(), &allocateInfo, nullptr, &stagingBufferMemory), "Cannot allocate stagin buffer's memory for Font Image");

    vkBindBufferMemory(device.getHandle(), stagingBuffer, stagingBufferMemory, 0);
}

void UI::createSampler()
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;

    VK_CHECK_ERROR(vkCreateSampler(device.getHandle(), &samplerInfo, nullptr, &sampler), "Cannot create Sampler for ui");
}

void UI::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = 1;
    descriptorPoolInfo.pPoolSizes = &poolSize;
    descriptorPoolInfo.maxSets = 2;

    VK_CHECK_ERROR(vkCreateDescriptorPool(device.getHandle(), &descriptorPoolInfo, nullptr, &descriptorPool), "Cannot create Descriptor Pool for UI");
}

void UI::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding bindingInfo = {};
    bindingInfo.binding = 0;
    bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindingInfo.descriptorCount = 1;
    bindingInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &bindingInfo;

    VK_CHECK_ERROR(vkCreateDescriptorSetLayout(device.getHandle(), &layoutInfo, nullptr, &descriptorSetLayout), "Cannot create Descriptor Set Layout for UI");
}

void UI::allocateDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &descriptorSetLayout;

    VK_CHECK_ERROR(vkAllocateDescriptorSets(device.getHandle(), &allocateInfo, &descriptorSet), "Cannot create Descriptro Set for UI");

    VkDescriptorImageInfo imageDescriptorInfo = {};
    imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageDescriptorInfo.imageView = fontImageView;
    imageDescriptorInfo.sampler = sampler;

    VkWriteDescriptorSet writeSet = {};
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstSet = descriptorSet;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeSet.dstBinding = 0;
    writeSet.descriptorCount = 1;
    writeSet.pImageInfo = &imageDescriptorInfo;

    vkUpdateDescriptorSets(device.getHandle(), 1, &writeSet, 0, nullptr);   
}

void UI::createPipelineLayout()
{
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(pushConstant);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descriptorSetLayout;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;

    VK_CHECK_ERROR(vkCreatePipelineLayout(device.getHandle(), &layoutInfo, nullptr, &pipelineLayout), "Cannot create Pipeline Layout for UI");
}

void UI::createPipeline()
{
    // Shaders
    RealTimeShader vertexShader = RealTimeShader(device, "D:/ComputerScience/Projects/Graphics/ShaderGround/shaders/ui.vert", RealTimeShader::Stage::S_VERTEX);
    RealTimeShader fragmentShader = RealTimeShader(device, "D:/ComputerScience/Projects/Graphics/ShaderGround/shaders/ui.frag", RealTimeShader::Stage::S_FRAGMENT);

    if (!vertexShader.compile() || !fragmentShader.compile()) return;

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

    std::vector<VkPipelineShaderStageCreateInfo> stages = 
    {
        vertexStageInfo,
        fragmentStageInfo
    };

    // Vertex Input State
    VkVertexInputBindingDescription vertexInputBinding = {};
    vertexInputBinding.binding = 0;
    vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertexInputBinding.stride = sizeof(ImDrawVert);

    VkVertexInputAttributeDescription posVertexInputAttribute = {};
    posVertexInputAttribute.binding = 0;
    posVertexInputAttribute.location = 0;
    posVertexInputAttribute.format = VK_FORMAT_R32G32_SFLOAT;
    posVertexInputAttribute.offset = offsetof(ImDrawVert, pos);

    VkVertexInputAttributeDescription uvVertexInputAttribute = {};
    uvVertexInputAttribute.binding = 0;
    uvVertexInputAttribute.location = 1;
    uvVertexInputAttribute.format = VK_FORMAT_R32G32_SFLOAT;
    uvVertexInputAttribute.offset = offsetof(ImDrawVert, uv);

    VkVertexInputAttributeDescription colVertexInputAttribute = {};
    colVertexInputAttribute.binding = 0;
    colVertexInputAttribute.location = 2;
    colVertexInputAttribute.format = VK_FORMAT_R8G8B8A8_UNORM;
    colVertexInputAttribute.offset = offsetof(ImDrawVert, col);

    std::vector<VkVertexInputAttributeDescription> vertexInputAttributes =
    {
        posVertexInputAttribute,
        uvVertexInputAttribute,
        colVertexInputAttribute
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
    vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateInfo.vertexBindingDescriptionCount = 1;
    vertexInputStateInfo.pVertexBindingDescriptions = &vertexInputBinding;
    vertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
    vertexInputStateInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();

    // Input Assembly State
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo = {};
    inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStateInfo.primitiveRestartEnable = VK_FALSE;

    // Viewport State
    VkPipelineViewportStateCreateInfo viewportStateInfo = {};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.scissorCount = 1;

    // Rasterization State
    VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = {};
    rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateInfo.cullMode = VK_CULL_MODE_NONE;
    rasterizationStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateInfo.depthBiasEnable = VK_FALSE;
    rasterizationStateInfo.lineWidth = 1.0f;

    // Multisample State
    VkPipelineMultisampleStateCreateInfo multisampleStateInfo = {};
    multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleStateInfo.sampleShadingEnable = VK_FALSE;
    multisampleStateInfo.minSampleShading = 0.0f;
    multisampleStateInfo.pSampleMask = nullptr;
    multisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleStateInfo.alphaToOneEnable = VK_FALSE;

    // Depth-Stencil State
    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
    depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateInfo.depthTestEnable = VK_FALSE;
    depthStencilStateInfo.depthWriteEnable = VK_FALSE;
    depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateInfo.stencilTestEnable = VK_FALSE;

    // Color Blend State
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT| VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {};
    colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateInfo.attachmentCount = 1;
    colorBlendStateInfo.pAttachments = &colorBlendAttachment;

    // Dynamic State
    std::vector<VkDynamicState> dynamicStates = 
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates = dynamicStates.data();

    // Pipeline creation
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(stages.size());
    pipelineInfo.pStages = stages.data();
    pipelineInfo.pVertexInputState = &vertexInputStateInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyStateInfo;
    pipelineInfo.pTessellationState = nullptr;
    pipelineInfo.pViewportState = &viewportStateInfo;
    pipelineInfo.pRasterizationState = &rasterizationStateInfo;
    pipelineInfo.pMultisampleState = &multisampleStateInfo;
    pipelineInfo.pDepthStencilState = &depthStencilStateInfo;
    pipelineInfo.pColorBlendState = &colorBlendStateInfo;
    pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = nullptr;

    VK_CHECK_ERROR(vkCreateGraphicsPipelines(device.getHandle(), nullptr, 1,  &pipelineInfo, nullptr, &pipeline), "Cannot create GraphicsPipeline");
}

void UI::createBuffers()
{
    ImDrawData* imDrawData = ImGui::GetDrawData();

    VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
    vertexBuffer = std::make_unique<Buffer>(device, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vertexCount = imDrawData->TotalVtxCount;
    vertexBuffer->map();

    VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
    indexBuffer = std::make_unique<Buffer>(device, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    indexCount = imDrawData->TotalIdxCount;
    indexBuffer->map();

    ImDrawVert* vertexMapped = (ImDrawVert*)vertexBuffer->getMapped();
    ImDrawIdx* indexMapped = (ImDrawIdx*)indexBuffer->getMapped();

    for (int i = 0; i < imDrawData->CmdListsCount; i++)
    {
        const ImDrawList* cmdList = imDrawData->CmdLists[i];
        
        memcpy(vertexMapped, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(indexMapped, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

        vertexMapped += cmdList->VtxBuffer.Size;
        indexMapped += cmdList->IdxBuffer.Size;
    }

    vertexBuffer->flush();
    indexBuffer->flush();
}

void UI::updateBuffers()
{
    ImDrawData* imDrawData = ImGui::GetDrawData();

    VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
    VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

    if ((vertexBufferSize == 0) || (indexBufferSize == 0)) return;

    if (vertexBuffer == nullptr)
    {
        vertexBuffer = std::make_unique<Buffer>(device, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        vertexCount = imDrawData->TotalVtxCount;
        vertexBuffer->map();
    }

    if (indexBuffer == nullptr)
    {
        indexBuffer = std::make_unique<Buffer>(device, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        indexCount = imDrawData->TotalIdxCount;
        indexBuffer->map();
    }

    if (vertexCount != imDrawData->TotalVtxCount)
    {
        vertexBuffer->unmap();
        vertexBuffer.reset();
        vertexBuffer = std::make_unique<Buffer>(device, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        vertexCount = imDrawData->TotalVtxCount;
        vertexBuffer->map();
    }

    if (indexCount != imDrawData->TotalIdxCount)
    {
        indexBuffer->unmap();
        indexBuffer.reset();
        indexBuffer = std::make_unique<Buffer>(device, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        indexCount = imDrawData->TotalIdxCount;
        indexBuffer->map();    
    }

    ImDrawVert* vertexMapped = (ImDrawVert*)vertexBuffer->getMapped();
    ImDrawIdx* indexMapped = (ImDrawIdx*)indexBuffer->getMapped();

    for (int i = 0; i < imDrawData->CmdListsCount; i++)
    {
        const ImDrawList* cmdList = imDrawData->CmdLists[i];
        
        memcpy(vertexMapped, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(indexMapped, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

        vertexMapped += cmdList->VtxBuffer.Size;
        indexMapped += cmdList->IdxBuffer.Size;
    }

    vertexBuffer->flush();
    indexBuffer->flush();
}

void UI::newFrame()
{
    ImGui::NewFrame();

    // ImGui::ShowDemoWindow();

    static bool test = false;

    ImGui::Begin("Test window");
    ImGui::Checkbox("Rotate", &test);
    ImGui::End();

    ImGui::Begin("Second");
    ImGui::Text("Second window");
    ImGui::End();

    ImGui::Begin("UIElement test");
    for (auto& element : elements)
    {
        element->execute();
    }
    ImGui::End();
    
    ImGui::Render();
}
