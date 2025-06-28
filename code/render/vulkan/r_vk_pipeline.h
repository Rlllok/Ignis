#pragma once

struct R_VK_GraphicsPipeline
{
  VkPipeline handle;
  VkPipelineLayout layout;

  VkDescriptorSetLayout set_layout;
};

func void R_VK_CreateGraphicsPipeline(R_Pipeline* pipeline);
func void R_VK_DestroyPipeline(R_VK_State* state);
