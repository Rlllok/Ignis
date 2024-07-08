#pragma once

#include "r_vk_core.h"
#include "r_vk_types.h"

func void R_VK_CreateRenderPass(R_VK_State* vk_state, R_VK_RenderPass* out_render_pass, Rect2f viewport_size, Vec4f clear_value)
func void R_VK_DestroyRenderPass(R_VK_RenderPass* render_pass);
func void R_VK_BegindRenderPass(VkCommandBuffer* command_buffer, R_VK_RenderPass* render_pass);
func void R_VK_EndRenderPass(VkCommandBuffer* command_buffer, R_VK_RenderPass* render_pass);
