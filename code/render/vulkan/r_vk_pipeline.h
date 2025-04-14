#pragma once

struct R_VK_GraphicsPipeline
{
  VkPipeline handle;
  VkPipelineLayout layout;
};

func void R_VK_CreateGraphicsPipeline(R_VK_State* state);
func void R_VK_DestroyPipeline(R_VK_State* state);
