#include "r_vk_command_buffer.h"

func void
R_VK_CreateCommandPool(R_VK_State* vk_state)
{
  VkCommandPoolCreateInfo pool_info = {};
  pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = vk_state->device.queue_index;

  VK_CHECK(vkCreateCommandPool(vk_state->device.logical, &pool_info, 0, &vk_state->command_pool));
}

func void
R_VK_AllocateCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* out_command_buffer)
{
  VkCommandBufferAllocateInfo command_buffer_info = {};
  command_buffer_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_buffer_info.commandPool        = pool;
  command_buffer_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  command_buffer_info.commandBufferCount = 1;

  out_command_buffer->state = R_VK_COMMAND_BUFFER_STATE_NOT_ALLOCATED;
  VK_CHECK(vkAllocateCommandBuffers(vk_state->device.logical, &command_buffer_info, &out_command_buffer->handle));
  out_command_buffer->state = R_VK_COMMAND_BUFFER_STATE_READY;
}

func void
R_VK_FreeCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* command_buffer)
{
  vkFreeCommandBuffers(vk_state->device.logical, pool, 1, &command_buffer->handle);

  command_buffer->handle = 0;
  command_buffer->state  = R_VK_COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

func void
R_VK_BeginCommandBuffer(R_VK_CommandBuffer* command_buffer)
{
  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  VK_CHECK(vkBeginCommandBuffer(command_buffer->handle, &begin_info));
  command_buffer->state = R_VK_COMMAND_BUFFER_STATE_RECORDING;
}

func void
R_VK_EndCommandBuffer(R_VK_CommandBuffer* command_buffer)
{
  VK_CHECK(vkEndCommandBuffer(command_buffer->handle));
  command_buffer->state = R_VK_COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

func void
R_VK_SubmitComandBuffer(R_VK_CommandBuffer* command_buffer)
{
  command_buffer->state = R_VK_COMMAND_BUFFER_STATE_SUBMITTED;
}

func void
R_VK_ResetCommandBuffer(R_VK_CommandBuffer* command_buffer)
{
  command_buffer->state= R_VK_COMMAND_BUFFER_STATE_READY;
}

func void
R_VK_BeginSingleUseCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* out_command_buffer)
{
  R_VK_AllocateCommandBuffer(vk_state, pool, out_command_buffer);

  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VK_CHECK(vkBeginCommandBuffer(out_command_buffer->handle, &begin_info));
  out_command_buffer->state = R_VK_COMMAND_BUFFER_STATE_RECORDING;
}

func void
R_VK_EndSingleUseCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* command_buffer, VkQueue queue)
{
  R_VK_EndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info = {};
  submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers    = &command_buffer->handle;

  VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, 0));

  VK_CHECK(vkQueueWaitIdle(queue));

  R_VK_FreeCommandBuffer(vk_state, pool, command_buffer);
}

