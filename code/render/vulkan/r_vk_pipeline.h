#pragma once

struct R_VK_Pipeline
{
  R_Pipeline* r_pipeline;

  VkPipeline            handle;
  VkPipelineLayout      layout;
  VkDescriptorSetLayout set_layout;
};

struct R_VK_RenderPass
{
  VkRenderPass handle;
  Rect2f render_area;
};

func B32 R_VK_CreatePipeline(R_Pipeline* pipeline);

func void R_VK_CreateRenderPass(struct R_VK_State* vk_state, R_VK_RenderPass* out_render_pass, Rect2f render_area);
