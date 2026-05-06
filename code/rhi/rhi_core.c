#include "rhi_core.h"

#include "base/base_logger.h"

// -------------------------------------------------------------------
// -- Command Buffer - -----------------------------------------------
func RHI_CommandBuffer
RHI_GetCommandBuffer(void) {
	return _r_state.device.GetCommandBuffer();
}

func void
RHI_BeginCommandBuffer(RHI_CommandBuffer command_buffer) {
	_r_state.device.BeginCommandBuffer(command_buffer);
}

func void
RHI_SubmitCommandBuffer(RHI_CommandBuffer command_buffer) {
	_r_state.device.SubmitCommandBuffer(command_buffer);
}

// -------------------------------------------------------------------
// -- Buffer ---------------------------------------------------------
func RHI_Buffer RHI_CreateBuffer(Str8 label, U32 capacity, RHI_BufferUsageFlags usage_flags, RHI_BufferPropertyFlags property_flags) {
	return _r_state.device.CreateBuffer(label, capacity, usage_flags, property_flags);
}

func U64
RHI_PushBuffer(RHI_Buffer buffer, U8* data, U64 size) {
	return _r_state.device.PushBuffer(buffer, data, size);
}

func void
RHI_ResetBuffer(RHI_Buffer buffer) {
	_r_state.device.ResetBuffer(buffer);
}

func RHI_DeviceAddress
RHI_BufferDeviceAddress(RHI_Buffer buffer) {
  return _r_state.device.BufferDeviceAddress(buffer);
}

func void
RHI_BindIndexBuffer(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset, RHI_IndexSize index_size) {
	_r_state.device.BindIndexBuffer(command_buffer, buffer, offset, index_size);
}

func void RHI_BindVertexBuffer(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset) {
	_r_state.device.BindVertexBuffer(command_buffer, buffer, offset);
}

// -------------------------------------------------------------------
// -- Texture --------------------------------------------------------
func RHI_Texture
RHI_CreateTexture(RHI_TextureCreateInfo* info) {
  return _r_state.device.CreateTexture(info);
}

func B32
RHI_DestroyTexture(RHI_Texture texture) {
  return _r_state.device.DestroyTexture(texture);
}

func void RHI_LoadImageToTexture(Str8 image_path, RHI_Texture texture) {
  _r_state.device.LoadDataToTexture(0, 0, texture);
}

func void
RHI_CopyTexture(RHI_CommandBuffer command_buffer, RHI_Texture source, RHI_Texture destination) {
  _r_state.device.CopyTexture(command_buffer, source, destination);
}

func U64
RHI_CopyTextureToBuffer(RHI_CommandBuffer command_buffer, RHI_Texture texture, RHI_Buffer buffer) {
  return _r_state.device.CopyTextureToBuffer(command_buffer, texture, buffer);
}

func void
RHI_CopyBufferToTexture(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset, U64 size, RHI_Texture texture) {
  _r_state.device.CopyBufferToTexture(command_buffer, buffer, offset, size, texture);
}

func RHI_TextureFormat
RHI_GetTextureFormat(RHI_Texture texture) {
  return _r_state.device.GetTextureFormat(texture);
}

func Vec2I32
RHI_GetTextureDimension(RHI_Texture texture) {
  return _r_state.device.GetTextureDimension(texture);
}

func RHI_TextureSampler
RHI_CreateTextureSampler(RHI_TextureSamplerCreateInfo* info) {
  return _r_state.device.CreateTextureSampler(info);
}

// -------------------------------------------------------------------
// -- Uniform Data  --------------------------------------------------
func void
RHI_BindGlobalVertexShaderData(RHI_CommandBuffer command_buffer, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_infos, I32 samplers_count, RHI_SamplerBindingInfo* sampler_infos) {
  Assert(uniform_buffers_count == 0 || uniform_infos != 0);
  Assert(samplers_count == 0 || sampler_infos != 0);

  _r_state.device.BindShaderData(command_buffer, RHI_ShaderKind_Vertex, 1, uniform_buffers_count, uniform_infos, samplers_count, sampler_infos);
}

func void
RHI_BindShaderArgument(RHI_CommandBuffer command_buffer, RHI_ShaderArgument argument) {
  _r_state.device.BindShaderArgument(command_buffer, argument);
}

func void
RHI_BindInstanceVertexShaderData(RHI_CommandBuffer command_buffer, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_infos, I32 samplers_count, RHI_SamplerBindingInfo* sampler_infos) {
  Assert(uniform_buffers_count == 0 || uniform_infos != 0);
  Assert(samplers_count == 0 || sampler_infos != 0);

	_r_state.device.BindShaderData(command_buffer, RHI_ShaderKind_Vertex, 0, uniform_buffers_count, uniform_infos, samplers_count, sampler_infos);
}

func void
RHI_BindGlobalFragmentShaderData(RHI_CommandBuffer command_buffer, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_infos, I32 samplers_count, RHI_SamplerBindingInfo* sampler_infos)
{
  Assert(uniform_buffers_count == 0 || uniform_infos != 0);
  Assert(samplers_count == 0 || sampler_infos != 0);

  _r_state.device.BindShaderData(command_buffer, RHI_ShaderKind_Fragment, 1, uniform_buffers_count, uniform_infos, samplers_count, sampler_infos);
}

func void
RHI_BindInstanceFragmentShaderData(RHI_CommandBuffer command_buffer, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_infos, I32 samplers_count, RHI_SamplerBindingInfo* sampler_infos) {
  Assert(uniform_buffers_count == 0 || uniform_infos != 0);
  Assert(samplers_count == 0 || sampler_infos != 0);

	_r_state.device.BindShaderData(command_buffer, RHI_ShaderKind_Fragment, 0, uniform_buffers_count, uniform_infos, samplers_count, sampler_infos);
}


// -------------------------------------------------------------------
// -- Swapchain ------------------------------------------------------
func RHI_TextureFormat
RHI_GetSwapchainTextureFormat() {
  return _r_state.device.GetSwapchainTextureFormat();
}

func RHI_Texture
RHI_AcquireSwapchainTexture(RHI_CommandBuffer command_buffer) {
	return _r_state.device.AcquireSwapchainTexture(command_buffer);
}

// -------------------------------------------------------------------
// -- Render Pass ----------------------------------------------------

func RHI_RenderPass*
RHI_BeginRenderPass(RHI_CommandBuffer command_buffer, U32 color_targets_count, RHI_ColorTarget* color_targets, RHI_DepthStencilTarget* depth_stencil_target, RHI_Resource* resources, I32 resources_count) {
	return _r_state.device.BeginRenderPass(command_buffer, color_targets_count, color_targets, depth_stencil_target, resources, resources_count);
}

func void
RHI_EndRenderPass(RHI_CommandBuffer command_buffer, RHI_RenderPass* render_pass) {
	_r_state.device.EndRenderPass(command_buffer, render_pass);
}

// -------------------------------------------------------------------
// -- Graphics Pipeline ----------------------------------------------
func U32
RHI_GetSizeOfVertexAttributeFormat(RHI_VertexAttributeFormat format) {
	U32 size_table[] = {
		sizeof(Vec2F32),
		sizeof(Vec3F32),
		sizeof(Vec4F32),
    sizeof(Vec4I32),
		0 // FORMAT_COUNT
	};

	return size_table[format];
}

func RHI_Shader
RHI_CreateShader(Arena* arena, RHI_ShaderCreateInfo* info) {
  return _r_state.device.CreateShader(arena, info);
}

func RHI_GraphicsPipeline
RHI_CreateGraphicsPipeline(RHI_GraphicsPipelineCreateInfo* info) {
	return _r_state.device.CreateGraphicsPipeline(info);
}

func void
RHI_BindGraphicsPipeline(RHI_CommandBuffer command_buffer, RHI_GraphicsPipeline pipeline) {
	_r_state.device.BindGraphicsPipeline(command_buffer, pipeline);
}

// -------------------------------------------------------------------
// -- Draw -----------------------------------------------------------
func void
RHI_SetViewport(RHI_CommandBuffer command_buffer, RectI32 viewport) {
	_r_state.device.SetViewport(command_buffer, viewport);
}

func void
RHI_SetScissor(RHI_CommandBuffer command_buffer, RectI32 scissor) {
	_r_state.device.SetScissor(command_buffer, scissor);
}

func void
RHI_DrawPrimitives(RHI_CommandBuffer command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance) {
	_r_state.device.DrawPrimitives(command_buffer, vertex_count, instance_count, first_vertex, first_instance);
}

func void
RHI_DrawIndexedPrimitives(RHI_CommandBuffer command_buffer, U32 index_count, U32 instance_count, U32 first_index, I32 vertex_offset, U32 first_instance) {
	_r_state.device.DrawIndexedPrimitives(command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
}

func void
RHI_PresentTexture(RHI_CommandBuffer command_buffer, RHI_Texture texture) {
  _r_state.device.PresentTexture(command_buffer, texture);
}

// -------------------------------------------------------------------
// -- State ----------------------------------------------------------
func B32
RHI_Init(OS_Window* window) {
#ifdef IGNIS_PLATFORM_MACOS
  AssignDeviceFunctions(Metal);
#else
		AssignDeviceFunctions(VK);
#endif // IGNIS_PLATFORM_MACOS

	_r_state.device.Init(window);

  return true;
}

func B32
RHI_Shutdown() {
  // --AlNov: @TODO Zero out _r_state.device struct
  B32 result = _r_state.device.Shutdown();
  _r_state.device = (RHI_Device)ZeroStruct();
	return result;
}

