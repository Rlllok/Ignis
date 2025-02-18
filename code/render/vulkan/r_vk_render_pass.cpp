#include "r_vk_render_pass.h"

func void
R_VK_CreateRenderPass(R_VK_State* vk_state, R_VK_RenderPass* out_render_pass, Rect2f render_area)
{
  VkAttachmentDescription color_attachment = {};
  color_attachment.format = vk_state->swapchain.surface_format.format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_reference = {};
  color_attachment_reference.attachment = 0;
  color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depth_attachment = {};
  depth_attachment.format = VK_FORMAT_D32_SFLOAT;
  depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_attachment_reference = {};
  depth_attachment_reference.attachment = 1;
  depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = 0;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_reference;
  subpass.pResolveAttachments = 0;
  subpass.pDepthStencilAttachment = &depth_attachment_reference;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = 0;

  VkSubpassDependency subpass_dependency = {};
  subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpass_dependency.dstSubpass = 0;
  subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  subpass_dependency.srcAccessMask = 0;
  subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  VkAttachmentDescription attachments[2] = { color_attachment, depth_attachment };

  VkRenderPassCreateInfo render_pass_info = {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 2;
  render_pass_info.pAttachments = attachments;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &subpass_dependency;

  VK_CHECK(vkCreateRenderPass(vk_state->device.logical, &render_pass_info, 0, &out_render_pass->handle));

  out_render_pass->render_area = render_area;
}

func void
R_VK_DestroyRenderPass(R_VK_State* vk_state, R_VK_RenderPass* render_pass)
{
  if (!render_pass || render_pass->handle) { return; }

  vkDestroyRenderPass(vk_state->device.logical, render_pass->handle, 0);
  
  render_pass->handle = 0;
}

func void
R_VK_BeginRenderPass(Vec4f clear_color, F32 clear_depth, F32 clear_stencil)
{
  R_VK_CommandBuffer* command_buffer  = r_vk_state.current_command_buffer;
  R_VK_Framebuffer* framebuffer = r_vk_state.current_framebuffer;
  R_VK_RenderPass* render_pass = &r_vk_state.render_pass;

  VkRenderPassBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  begin_info.renderPass = render_pass->handle;
  begin_info.framebuffer = framebuffer->handle;
  begin_info.renderArea.offset.x = render_pass->render_area.x;
  begin_info.renderArea.offset.y = render_pass->render_area.y;
  begin_info.renderArea.extent.width = render_pass->render_area.w;
  begin_info.renderArea.extent.height = render_pass->render_area.h;

  VkClearValue color_clear_value = {};
  color_clear_value.color.float32[0] = clear_color.r;
  color_clear_value.color.float32[1] = clear_color.g;
  color_clear_value.color.float32[2] = clear_color.b;
  color_clear_value.color.float32[3] = clear_color.a;

  VkClearValue depth_stencil_clear_value = {};
  depth_stencil_clear_value.depthStencil.depth = clear_depth;
  depth_stencil_clear_value.depthStencil.stencil = clear_stencil;

  VkClearValue clear_values[2] = { color_clear_value, depth_stencil_clear_value };

  begin_info.clearValueCount = CountArrayElements(clear_values);
  begin_info.pClearValues = clear_values;

  vkCmdBeginRenderPass(command_buffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

func void
R_VK_EndRenderPass()
{
  vkCmdEndRenderPass(r_vk_state.current_command_buffer->handle);
}

