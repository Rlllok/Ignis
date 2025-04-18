#include "r_vk_pipeline.h"

func void
R_VK_CreateGraphicsPipeline(R_VK_State* state)
{
  VkPipelineLayoutCreateInfo layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
  };
  VK_CHECK(vkCreatePipelineLayout(state->device.logical, &layout_info, 0, &state->pipeline.layout));

  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = 2 * sizeof(Vec3f),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
  };

  VkVertexInputAttributeDescription attribute_descriptions[2];
  attribute_descriptions[0] = {
    .location = 0,
    .binding = 0,
    .format = VK_FORMAT_R32G32B32_SFLOAT,
    .offset = 0
  };
  attribute_descriptions[1] = {
    .location = 1,
    .binding = 0,
    .format = VK_FORMAT_R32G32B32_SFLOAT,
    .offset = 0
  };
  
  VkPipelineVertexInputStateCreateInfo vertex_input = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &binding_description,
    .vertexAttributeDescriptionCount = CountArrayElements(attribute_descriptions),
    .pVertexAttributeDescriptions = attribute_descriptions
  };

  VkPipelineInputAssemblyStateCreateInfo input_assembly = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = false
  };

  VkPipelineRasterizationStateCreateInfo rasterization = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = false,
    .rasterizerDiscardEnable = false,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_NONE,
    .frontFace = VK_FRONT_FACE_CLOCKWISE,
    .depthBiasEnable = false,
    .lineWidth = 1.0f
  };

  VkDynamicState dynamic_states[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
  };

  VkPipelineColorBlendAttachmentState blend_attachment = {
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
  };

  VkPipelineColorBlendStateCreateInfo blend = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments = &blend_attachment
  };

  VkPipelineViewportStateCreateInfo viewport = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .scissorCount = 1
  };

  VkPipelineDepthStencilStateCreateInfo depth_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthCompareOp = VK_COMPARE_OP_ALWAYS
  };

  VkPipelineMultisampleStateCreateInfo multisample = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
  };

  VkPipelineDynamicStateCreateInfo dynamic = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = CountArrayElements(dynamic_states),
    .pDynamicStates = dynamic_states
  };

  R_H_LoadShader(state->arena, "data/shaders/triangle.vs.glsl",
                 "main", R_SHADER_TYPE_VERTEX,
                 &state->vertex_shader);
  R_H_LoadShader(state->arena, "data/shaders/triangle.fs.glsl",
                 "main", R_SHADER_TYPE_FRAGMENT,
                 &state->fragment_shader);

  VkShaderModule vertex_module;
  {
    VkShaderModuleCreateInfo module_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = state->vertex_shader.code_size,
      .pCode = (U32*)state->vertex_shader.code
    };
    VK_CHECK(vkCreateShaderModule(state->device.logical, &module_info, 0, &vertex_module));
  }
  VkPipelineShaderStageCreateInfo vertex_shader = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_VERTEX_BIT,
    .module = vertex_module,
    .pName = state->vertex_shader.entry_point
  };
  
  VkShaderModule fragment_module;
  {
    VkShaderModuleCreateInfo module_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = state->fragment_shader.code_size,
      .pCode = (U32*)state->fragment_shader.code
    };
    VK_CHECK(vkCreateShaderModule(state->device.logical, &module_info, 0, &fragment_module));
  }
  VkPipelineShaderStageCreateInfo fragment_shader = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    .module = fragment_module,
    .pName = state->fragment_shader.entry_point
  };

  VkPipelineShaderStageCreateInfo shaders[] = {
    vertex_shader,
    fragment_shader
  };

  VkPipelineRenderingCreateInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &state->swapchain.surface_format.format
  };

  VkGraphicsPipelineCreateInfo pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &rendering_info,
    .stageCount = CountArrayElements(shaders),
    .pStages = shaders,
    .pVertexInputState = &vertex_input,
    .pInputAssemblyState = &input_assembly,
    .pViewportState = &viewport,
    .pRasterizationState = &rasterization,
    .pMultisampleState = &multisample,
    .pDepthStencilState = &depth_state,
    .pColorBlendState = &blend,
    .pDynamicState = &dynamic,
    .layout = state->pipeline.layout,
    .renderPass = 0,
    .subpass = 0
  };
  VK_CHECK(vkCreateGraphicsPipelines(state->device.logical,
                                     0, 1, &pipeline_info, 0,
                                     &state->pipeline.handle));

  vkDestroyShaderModule(state->device.logical, vertex_module, 0);
  vkDestroyShaderModule(state->device.logical, fragment_module, 0);
}

func void
R_VK_DestroyPipeline(R_VK_State* state)
{
  vkDestroyPipelineLayout(state->device.logical, state->pipeline.layout, 0);
  vkDestroyPipeline(state->device.logical, state->pipeline.handle, 0);
}
