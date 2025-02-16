#include "r_vk_swapchain.h"

func void
R_VK_CreateFramebuffer(
  R_VK_State* vk_state, R_VK_RenderPass* render_pass, Vec2u size,
  U32 attachment_count, VkImageView* attachments, R_VK_Framebuffer* out_framebuffer
)
{
  out_framebuffer->attachments = (VkImageView*)PushArena(vk_state->arena, sizeof(VkImageView) * attachment_count);
  for (U32 i = 0; i < attachment_count; i += 1)
  {
    out_framebuffer->attachments[i] = attachments[i];
  }
  out_framebuffer->attachment_count = attachment_count;

  VkFramebufferCreateInfo framebuffer_info = {};
  framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebuffer_info.renderPass      = r_vk_state.render_pass.handle;
  framebuffer_info.attachmentCount = attachment_count;
  framebuffer_info.pAttachments    = out_framebuffer->attachments;
  framebuffer_info.width           = size.width;
  framebuffer_info.height          = size.height;
  framebuffer_info.layers          = 1;

  VK_CHECK(vkCreateFramebuffer(vk_state->device.logical, &framebuffer_info, 0, &out_framebuffer->handle));
}

func void
R_VK_DestroyFramebuffer(R_VK_State* vk_state, R_VK_Framebuffer* framebuffer)
{
  vkDestroyFramebuffer(vk_state->device.logical, framebuffer->handle, 0);
  
  // --AlNov: @TODO @EROR There is memory leak. Don't free attachments
  *framebuffer = {};
}

func void
R_VK_CreateSwapchain(U32 width, U32 height)
{
  VkSwapchainCreateInfoKHR swapchain_info = {};
  swapchain_info.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.surface               = r_vk_state.swapchain.surface;
  swapchain_info.minImageCount         = r_vk_state.swapchain.image_count;
  swapchain_info.imageFormat           = r_vk_state.swapchain.surface_format.format;
  swapchain_info.imageColorSpace       = r_vk_state.swapchain.surface_format.colorSpace;
  swapchain_info.imageExtent.width     = width;
  swapchain_info.imageExtent.height    = height;
  swapchain_info.imageArrayLayers      = 1;
  swapchain_info.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_info.queueFamilyIndexCount = 0;
  swapchain_info.pQueueFamilyIndices   = 0;
  swapchain_info.preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swapchain_info.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.presentMode           = VK_PRESENT_MODE_IMMEDIATE_KHR;
  swapchain_info.clipped               = VK_TRUE;
  swapchain_info.oldSwapchain          = VK_NULL_HANDLE;

  VK_CHECK(vkCreateSwapchainKHR(r_vk_state.device.logical, &swapchain_info, 0, &r_vk_state.swapchain.handle));

  vkGetSwapchainImagesKHR(r_vk_state.device.logical, r_vk_state.swapchain.handle, &r_vk_state.swapchain.image_count, 0);
  // --AlNov: @TODO Images doesnt deleted on swapchain recreation
  r_vk_state.swapchain.images = (VkImage*)PushArena(r_vk_state.arena, r_vk_state.swapchain.image_count * sizeof(VkImage));
  vkGetSwapchainImagesKHR(r_vk_state.device.logical, r_vk_state.swapchain.handle, &r_vk_state.swapchain.image_count, r_vk_state.swapchain.images);

  r_vk_state.swapchain.image_views = (VkImageView*)PushArena(r_vk_state.arena, r_vk_state.swapchain.image_count * sizeof(VkImageView));
  for (U32 i = 0; i < r_vk_state.swapchain.image_count; i += 1)
  {
    VkImageViewCreateInfo image_view_info = {};
    image_view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_info.image                           = r_vk_state.swapchain.images[i];
    image_view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format                          = r_vk_state.swapchain.surface_format.format;
    image_view_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_info.subresourceRange.baseMipLevel   = 0;
    image_view_info.subresourceRange.levelCount     = 1;
    image_view_info.subresourceRange.baseArrayLayer = 0;
    image_view_info.subresourceRange.layerCount     = 1;

    VK_CHECK(vkCreateImageView(r_vk_state.device.logical, &image_view_info, 0, &r_vk_state.swapchain.image_views[i]));
  }
}

func void
R_VK_RecreateSwapchain()
{
  vkDeviceWaitIdle(r_vk_state.device.logical);

  R_VK_DestroySwapchain();
  R_VK_CreateSwapchain(50, 50);

  // --AlNov: Create Framebuffers
  {
    U32 image_count = r_vk_state.swapchain.image_count;
    r_vk_state.swapchain.framebuffers = (R_VK_Framebuffer*)PushArena(r_vk_state.arena, image_count * sizeof(R_VK_Framebuffer));

    for (U32 i = 0; i < image_count; i += 1)
    {
      R_VK_DestroyFramebuffer(&r_vk_state, &r_vk_state.swapchain.framebuffers[i]);

      VkImageView attachments[2] = { r_vk_state.swapchain.image_views[i], r_vk_state.depth_view };

      R_VK_CreateFramebuffer(
        &r_vk_state, &r_vk_state.render_pass, r_vk_state.swapchain.size,
        CountArrayElements(attachments), attachments, &r_vk_state.swapchain.framebuffers[i]
      );
    }
  }
}

func void
R_VK_DestroySwapchain()
{
  for (U32 i = 0; i < r_vk_state.swapchain.image_count; i += 1)
  {
    vkDestroyImageView(r_vk_state.device.logical,
                       r_vk_state.swapchain.image_views[i],
                       0);
  }

  vkDestroySwapchainKHR(r_vk_state.device.logical,
                        r_vk_state.swapchain.handle,
                        0);
}

