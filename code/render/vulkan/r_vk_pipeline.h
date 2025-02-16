#pragma once

struct R_VK_Pipeline
{
  R_Pipeline* r_pipeline;

  VkPipeline            handle;
  VkPipelineLayout      layout;
  VkDescriptorSetLayout set_layout;
};

func B32  R_VK_CreatePipeline(R_Pipeline* pipeline);
func void R_VK_BindPipeline(R_Pipeline* pipeline);

enum R_VK_ShaderType
{
  R_VK_SHADER_TYPE_VERTEX,
  R_VK_SHADER_TYPE_FRAGMENT,

  R_VK_SHADER_TYPE_COUNT
};

struct R_VK_ShaderStage
{
  R_VK_ShaderType type;
  const char*     enter_point;
  U32             code_size;
  U8*             code;
  
  VkShaderModule                  vk_handle;
  VkPipelineShaderStageCreateInfo vk_info;
};

struct R_VK_ShaderProgram
{
  R_VK_ShaderStage      shader_stages[R_VK_SHADER_TYPE_COUNT];
  R_VK_Pipeline         pipeline;
  VkDescriptorSetLayout set_layout;
};

global VkDescriptorSetLayout scene_descriptor_layout;
global VkDescriptorSetLayout instance_descriptor_layout;

func void R_VK_BindShaderProgram(R_VK_CommandBuffer* command_buffer, R_VK_ShaderProgram* program);
