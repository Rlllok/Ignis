#pragma once

struct R_VK_Framebuffer
{
  VkFramebuffer    handle;
  U32              attachment_count;
  VkImageView*     attachments;
};

func void R_VK_CreateFramebuffer(R_VK_State* vk_state, R_VK_RenderPass* render_pass, Vec2u size, U32 attachment_count, VkImageView* attachments, R_VK_Framebuffer* out_framebuffer);
func void R_VK_DestroyFramebuffer(R_VK_State* vk_state, R_VK_Framebuffer* framebuffer);

struct R_VK_Swapchain
{
  VkSwapchainKHR     handle;
  VkSurfaceKHR       surface;
  VkSurfaceFormatKHR surface_format;
  Vec2u              size;
  U32                image_count;
  VkImage*           images;
  VkImageView*       image_views;
  R_VK_Framebuffer*  framebuffers;
};

func void R_VK_CreateSwapchain(U32 width, U32 height);
func void R_VK_RecreateSwapchain();
func void R_VK_DestroySwapchain();

