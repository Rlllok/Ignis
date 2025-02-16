#include "r_vk_pipeline.h"

func B32
R_VK_CreatePipeline(R_Pipeline* pipeline)
{
  // --AlNov: Vertex Shader
  VkShaderModule vertex_shader_module;
  R_Shader* vertex_shader = &pipeline->shaders[R_SHADER_TYPE_VERTEX];
  {
    VkShaderModuleCreateInfo module_info = {};
    module_info.sType     = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_info.codeSize  = vertex_shader->code_size;
    module_info.pCode     = (U32*)vertex_shader->code;
    VK_CHECK(vkCreateShaderModule(r_vk_state.device.logical, &module_info, 0, &vertex_shader_module));
  }
  VkPipelineShaderStageCreateInfo vertex_shader_stage = {};
  vertex_shader_stage.sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertex_shader_stage.stage   = R_VK_ShaderStageFromShaderType(vertex_shader->type);
  vertex_shader_stage.module  = vertex_shader_module;
  vertex_shader_stage.pName   = vertex_shader->entry_point;

  // --AlNov: Fragment Shader
  VkShaderModule fragment_shader_module;
  R_Shader* fragment_shader = &pipeline->shaders[R_SHADER_TYPE_FRAGMENT];
  {
    VkShaderModuleCreateInfo module_info = {};
    module_info.sType     = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_info.codeSize  = fragment_shader->code_size;
    module_info.pCode     = (U32*)fragment_shader->code;
    VK_CHECK(vkCreateShaderModule(r_vk_state.device.logical, &module_info, 0, &fragment_shader_module));
  }
  VkPipelineShaderStageCreateInfo fragment_shader_stage = {};
  fragment_shader_stage.sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragment_shader_stage.stage   = R_VK_ShaderStageFromShaderType(fragment_shader->type);
  fragment_shader_stage.module  = fragment_shader_module;
  fragment_shader_stage.pName   = fragment_shader->entry_point;

  VkPipelineShaderStageCreateInfo stages[] = {
    vertex_shader_stage,
    fragment_shader_stage
  };

  VkDescriptorSetLayoutBinding scene_bindings[MAX_BINDINGS] = {};
  for (U32 i = 0; i < pipeline->scene_bindings_count; i += 1)
  {
    scene_bindings[i].binding = 0;
    scene_bindings[i].descriptorType = R_VK_DescriptorTypeFromBindingType(
      pipeline->scene_bindings[i].type
    );
    scene_bindings[i].descriptorCount = 1;
    scene_bindings[i].stageFlags = R_VK_ShaderStageFromShaderType(
      pipeline->scene_bindings[i].shader_type
    );
  }

  VkDescriptorSetLayoutCreateInfo scene_layout_info = {};
  scene_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  scene_layout_info.bindingCount = pipeline->scene_bindings_count;
  scene_layout_info.pBindings = scene_bindings;
  VK_CHECK(vkCreateDescriptorSetLayout(
    r_vk_state.device.logical,
    &scene_layout_info,
    0,
    &scene_descriptor_layout
  ));

  VkDescriptorSetLayoutBinding instance_bindings[MAX_BINDINGS] = {};
  for (U32 i = 0; i < pipeline->instance_bindings_count; i += 1)
  {
    instance_bindings[i].binding = 0;
    instance_bindings[i].descriptorType = R_VK_DescriptorTypeFromBindingType(
      pipeline->instance_bindings[i].type
    );
    instance_bindings[i].descriptorCount = 1;
    instance_bindings[i].stageFlags = R_VK_ShaderStageFromShaderType(
      pipeline->instance_bindings[i].shader_type
    );
  }

  VkDescriptorSetLayoutCreateInfo instance_layout_info = {};
  instance_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  instance_layout_info.bindingCount = pipeline->instance_bindings_count;
  instance_layout_info.pBindings = instance_bindings;
  VK_CHECK(vkCreateDescriptorSetLayout(
    r_vk_state.device.logical,
    &instance_layout_info,
    0,
    &instance_descriptor_layout
  ));

  // --AlNov: @TODO Get rid of hardcoded size of array
  VkVertexInputAttributeDescription vertex_attributes[10] = {};

  U32 offset = 0;
  for (U32 i = 0; i < pipeline->attributes_count; i++)
  {
    vertex_attributes[i].location = i;
    vertex_attributes[i].binding  = 0;
    vertex_attributes[i].format   = R_VK_VkFormatFromAttributeFormat(pipeline->attributes[i]);
    vertex_attributes[i].offset   = offset;

    offset += R_H_OffsetFromAttributeFormat(pipeline->attributes[i]);
  }

  VkVertexInputBindingDescription vertex_description = {};
  vertex_description.binding   = 0;
  vertex_description.stride    = offset;
  vertex_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkPipelineVertexInputStateCreateInfo vertex_input_state_info = {};
  vertex_input_state_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_state_info.vertexBindingDescriptionCount   = 1;
  vertex_input_state_info.pVertexBindingDescriptions      = &vertex_description;
  vertex_input_state_info.vertexAttributeDescriptionCount = pipeline->attributes_count;
  vertex_input_state_info.pVertexAttributeDescriptions    = vertex_attributes;

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};
  input_assembly_state_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly_state_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly_state_info.primitiveRestartEnable = VK_FALSE;

  // --AlNov: Viewport State
  VkViewport viewport = {};
  viewport.x        = 0;
  viewport.y        = 0;
  viewport.height   = r_vk_state.swapchain.size.height;
  viewport.width    = r_vk_state.swapchain.size.width;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset        = {0, 0};
  scissor.extent.height = r_vk_state.swapchain.size.height;
  scissor.extent.width  = r_vk_state.swapchain.size.width;

  VkPipelineViewportStateCreateInfo viewport_state_info = {};
  viewport_state_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state_info.viewportCount = 1;
  viewport_state_info.pViewports    = &viewport;
  viewport_state_info.scissorCount  = 1;
  viewport_state_info.pScissors     = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterization_state_info = {};
  rasterization_state_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterization_state_info.depthClampEnable        = VK_FALSE;
  rasterization_state_info.rasterizerDiscardEnable = VK_FALSE;
  rasterization_state_info.polygonMode             = VK_POLYGON_MODE_FILL;
  rasterization_state_info.cullMode                = VK_CULL_MODE_NONE;
  rasterization_state_info.frontFace               = VK_FRONT_FACE_CLOCKWISE;
  rasterization_state_info.depthBiasEnable         = VK_FALSE;
  rasterization_state_info.lineWidth               = 1.0f;

  VkPipelineMultisampleStateCreateInfo multisample_state_info = {};
  multisample_state_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample_state_info.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
  multisample_state_info.sampleShadingEnable   = VK_FALSE;
  multisample_state_info.minSampleShading      = 0.0f;
  multisample_state_info.pSampleMask           = nullptr;
  multisample_state_info.alphaToCoverageEnable = VK_FALSE;
  multisample_state_info.alphaToOneEnable      = VK_FALSE;

  VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {};
  depth_stencil_state_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_stencil_state_info.depthTestEnable       = pipeline->is_depth_test_enabled ? VK_TRUE : VK_FALSE;
  depth_stencil_state_info.depthWriteEnable      = pipeline->is_depth_test_enabled ? VK_TRUE : VK_FALSE;
  depth_stencil_state_info.depthCompareOp        = VK_COMPARE_OP_LESS;
  depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
  depth_stencil_state_info.stencilTestEnable     = VK_FALSE;

  VkPipelineColorBlendAttachmentState color_blend_attachment = {};
  color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT| VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
    | VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment.blendEnable    = VK_TRUE;
  color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo color_blend_state_info = {};
  color_blend_state_info.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blend_state_info.logicOpEnable   = VK_FALSE;
  color_blend_state_info.attachmentCount = 1;
  color_blend_state_info.pAttachments    = &color_blend_attachment;

  VkDynamicState dynamic_states[] = {
    VK_DYNAMIC_STATE_SCISSOR
  };

  VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
  dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state_info.dynamicStateCount = CountArrayElements(dynamic_states);
  dynamic_state_info.pDynamicStates = dynamic_states;

  {
    VkDescriptorSetLayout descriptors[2] = {
      scene_descriptor_layout,
      instance_descriptor_layout
    };
    VkPipelineLayoutCreateInfo layout_info = {};
    layout_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount         = 2;
    layout_info.pSetLayouts            = descriptors;
    layout_info.pushConstantRangeCount = 0;
    layout_info.pPushConstantRanges    = 0;

    VK_CHECK(vkCreatePipelineLayout(r_vk_state.device.logical, &layout_info, 0, &r_vk_state.pipelines[r_vk_state.pipelines_count].layout));
  }

  VkGraphicsPipelineCreateInfo pipeline_info = {};
  pipeline_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount          = 2;
  pipeline_info.pStages             = stages;
  pipeline_info.pVertexInputState   = &vertex_input_state_info;
  pipeline_info.pInputAssemblyState = &input_assembly_state_info;
  pipeline_info.pViewportState      = &viewport_state_info;
  pipeline_info.pRasterizationState = &rasterization_state_info;
  pipeline_info.pMultisampleState   = &multisample_state_info;
  pipeline_info.pDepthStencilState  = &depth_stencil_state_info;
  pipeline_info.pColorBlendState    = &color_blend_state_info;
  pipeline_info.pDynamicState       = &dynamic_state_info;
  pipeline_info.layout              = r_vk_state.pipelines[r_vk_state.pipelines_count].layout;
  pipeline_info.renderPass          = r_vk_state.render_pass.handle;
  pipeline_info.subpass             = 0;

  VK_CHECK(vkCreateGraphicsPipelines(r_vk_state.device.logical, 0, 1, &pipeline_info, nullptr, &r_vk_state.pipelines[r_vk_state.pipelines_count].handle));

  pipeline->backend_handle = r_vk_state.pipelines_count;
  r_vk_state.pipelines[r_vk_state.pipelines_count].r_pipeline = pipeline;

  r_vk_state.pipelines_count += 1;

  return true;
}

func void
R_VK_BindPipeline(R_Pipeline* pipeline)
{
  r_vk_state.active_pipeline_index = pipeline->backend_handle;
  vkCmdBindPipeline(r_vk_state.current_command_buffer->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, r_vk_state.pipelines[r_vk_state.active_pipeline_index].handle);
}

func void
R_VK_BindShaderProgram(R_VK_CommandBuffer* command_buffer, R_VK_ShaderProgram* program)
{
  vkCmdBindPipeline(command_buffer->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, program->pipeline.handle);
}
