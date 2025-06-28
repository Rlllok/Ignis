#include "r_vk_pipeline.h"
#include "render/r_pipeline.h"
#include "render/vulkan/r_vk_utils.h"
#include "third_party/vulkan/include/vulkan_core.h"

func void
R_VK_CreateGraphicsPipeline(R_Pipeline* pipeline)
{
  R_VK_State* state = &r_vk_state;

  pipeline->backend_handle = state->pipelines_count;
  R_VK_GraphicsPipeline* vk_pipeline = &state->graphics_pipelines[pipeline->backend_handle];

  VkDescriptorSetLayoutBinding bindings[4] = {};
  U32 binding_count = 0;
  for (U32 i = 0; i < pipeline->scene_bindings_count; i += 1)
  {
    R_BindingInfo* binding_info = &pipeline->scene_bindings[i];

    bindings[i] = {
      .binding = i,
      .descriptorType = R_VK_GetVkDescriptorType(binding_info->type),
      .descriptorCount = 1,
      .stageFlags = R_VK_GetVkShaderStage(binding_info->shader_type)
    };

    binding_count += 1;
  }

  VkDescriptorSetLayoutCreateInfo layout = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .bindingCount = binding_count,
    .pBindings = bindings
  };
  VK_CHECK(vkCreateDescriptorSetLayout(state->device.logical, &layout, 0, &vk_pipeline->set_layout));
    
  VkPipelineLayoutCreateInfo layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &vk_pipeline->set_layout,
  };
  VK_CHECK(vkCreatePipelineLayout(state->device.logical, &layout_info, 0, &state->graphics_pipelines[state->pipelines_count].layout));
  
  U32 stride = 0;
  VkVertexInputAttributeDescription attribute_descriptions[MAX_ATTRIBUTES];
  for (U32 i = 0; i < pipeline->attributes_count; i += 1)
  {
    attribute_descriptions[i] = {
      .location = i,
      .binding = 0,
      .format = R_VK_GetVkFormatAttribute(pipeline->attributes[i]),
      .offset = stride
    };

    stride += R_H_OffsetFromAttributeFormat(pipeline->attributes[i]);
  }
  
  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = stride,
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };
  
  VkPipelineVertexInputStateCreateInfo vertex_input = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &binding_description,
    .vertexAttributeDescriptionCount = pipeline->attributes_count,
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
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
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
    .depthTestEnable = VK_TRUE,
    .depthWriteEnable = VK_TRUE,
    .depthCompareOp = VK_COMPARE_OP_GREATER
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

  VkShaderModule vertex_module;
  {
    VkShaderModuleCreateInfo module_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = pipeline->shaders[R_SHADER_TYPE_VERTEX].code_size,
      .pCode = (U32*)pipeline->shaders[R_SHADER_TYPE_VERTEX].code
    };
    VK_CHECK(vkCreateShaderModule(state->device.logical, &module_info, 0, &vertex_module));
  }
  VkPipelineShaderStageCreateInfo vertex_shader = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_VERTEX_BIT,
    .module = vertex_module,
    .pName = pipeline->shaders[R_SHADER_TYPE_VERTEX].entry_point
  };
  
  VkShaderModule fragment_module;
  {
    VkShaderModuleCreateInfo module_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = pipeline->shaders[R_SHADER_TYPE_FRAGMENT].code_size,
      .pCode = (U32*)pipeline->shaders[R_SHADER_TYPE_FRAGMENT].code
    };
    VK_CHECK(vkCreateShaderModule(state->device.logical, &module_info, 0, &fragment_module));
  }
  VkPipelineShaderStageCreateInfo fragment_shader = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    .module = fragment_module,
    .pName = pipeline->shaders[R_SHADER_TYPE_FRAGMENT].entry_point
  };

  VkPipelineShaderStageCreateInfo shaders[] = {
    vertex_shader,
    fragment_shader
  };

  VkPipelineRenderingCreateInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &state->swapchain.surface_format.format,
    .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT
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
    .layout = state->graphics_pipelines[state->pipelines_count].layout,
    .renderPass = 0,
    .subpass = 0,
  };
  VK_CHECK(vkCreateGraphicsPipelines(state->device.logical,
                                     0, 1, &pipeline_info, 0,
                                     &state->graphics_pipelines[state->pipelines_count].handle));
  state->pipelines_count += 1;

  vkDestroyShaderModule(state->device.logical, vertex_module, 0);
  vkDestroyShaderModule(state->device.logical, fragment_module, 0);
}

func void
R_VK_DestroyPipeline(R_VK_State* state)
{
  for (U32 i = 0; i < state->pipelines_count; i += 1)
  {
    vkDestroyPipelineLayout(state->device.logical, state->graphics_pipelines[i].layout, 0);
    vkDestroyPipeline(state->device.logical, state->graphics_pipelines[i].handle, 0);
  }
}
