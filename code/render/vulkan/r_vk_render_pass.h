#pragma once

enum R_VK_RenderPassState
{
  R_VK_RENDER_PASS_STATE_NONE,
  R_VK_RENDER_PASS_STATE_READY,
  R_VK_RENDER_PASS_STATE_RECORDING,
  R_VK_RENDER_PASS_STATE_IN_RENDER_PASS,
  R_VK_RENDER_PASS_STATE_RECORDING_ENDED,
  R_VK_RENDER_PASS_STATE_SUBMITTED,
  R_VK_RENDER_PASS_STATE_NOT_ALLOCATED,

  R_VK_RENDER_PASS_STATE_COUNT
};

struct R_VK_RenderPass
{
  VkRenderPass         handle;
  Rect2f               render_area;
  R_VK_RenderPassState state;
};

func void R_VK_CreateRenderPass(struct R_VK_State* vk_state, R_VK_RenderPass* out_render_pass, Rect2f render_area);
func void R_VK_DestroyRenderPass(struct R_VK_State* vk_state, R_VK_RenderPass* render_pass);

func void R_VK_BeginRenderPass(Vec4f clear_color, F32 clear_depth, F32 clear_stencil);
func void R_VK_EndRenderPass();
