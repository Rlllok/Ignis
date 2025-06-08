#pragma once

#include "base/base_include.h"

#define FRAMES_IN_FLIGHT 3
struct FrameResources
{
  VkFence submit_fence;
  VkCommandPool cmd_pool;
  VkCommandBuffer cmd_buffer;
  VkSemaphore acquire_semaphore;
  VkSemaphore release_semaphore;
};

func void R_VK_CreateFrameResources(R_VK_State* state, FrameResources* resources);
func void R_VK_DestroyFrameResources(R_VK_State* state, FrameResources* resources);

struct R_VK_Swapchain
{
  VkSwapchainKHR handle;
  VkSurfaceFormatKHR surface_format;
  Vec2u size;
  Arena* image_arena;
  U32 image_count;
  VkImage* images;
  VkImageView* image_views;
  VkImage depth_image;
  VkDeviceMemory depth_image_memory;
  VkImageView depth_image_view;
  FrameResources* frame_resources; // Per Image
  // U32 current_index;
};

func void R_VK_CreateSwapchain(R_VK_State* state, OS_Window* window);
func void R_VK_RecreateSwapchain(R_VK_State* state, OS_Window* window);
func void R_VK_DestroySwapchain(R_VK_State* state);
func B32 R_VK_AcquireNextImage(R_VK_State* state, U32 *image_index);
