#pragma once

enum R_VK_CommandBufferState
{
  R_VK_COMMAND_BUFFER_STATE_NONE,
  R_VK_COMMAND_BUFFER_STATE_READY,
  R_VK_COMMAND_BUFFER_STATE_RECORDING,
  R_VK_COMMAND_BUFFER_STATE_IN_RENDER_PASS,
  R_VK_COMMAND_BUFFER_STATE_RECORDING_ENDED,
  R_VK_COMMAND_BUFFER_STATE_SUBMITTED,
  R_VK_COMMAND_BUFFER_STATE_NOT_ALLOCATED,

  R_VK_COMMAND_BUFFER_STATE_COUNT
};

struct R_VK_CommandBuffer
{
  VkCommandBuffer handle;

  R_VK_CommandBufferState state;
};

struct R_VK_CommandPool
{
  VkCommandPool pool;
  VkCommandBuffer buffers[NUM_FRAMES_IN_FLIGHT];
};

func void R_VK_CreateCommandPool(R_VK_State* vk_state);
func void R_VK_AllocateCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* out_command_buffer);
func void R_VK_FreeCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* command_buffer);
func void R_VK_BeginCommandBuffer(R_VK_CommandBuffer* command_buffer);
func void R_VK_EndCommandBuffer(R_VK_CommandBuffer* command_buffer);
func void R_VK_SubmitComandBuffer(R_VK_CommandBuffer* command_buffer); // --AlNov: @TODO Only change state for now
func void R_VK_ResetCommandBuffer(R_VK_CommandBuffer* command_buffer);
func void R_VK_BeginSingleUseCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* out_command_buffer);
func void R_VK_EndSingleUseCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* command_buffer, VkQueue queue);

