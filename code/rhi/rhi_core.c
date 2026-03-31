#include "rhi_core.h"

#include "base/base_logger.h"

#include "third_party/glslang/include/Include/glslang_c_interface.h"
#include "third_party/glslang/include/Public/resource_limits_c.h"

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
func RHI_Buffer RHI_CreateBuffer(U32 capacity, RHI_BufferUsageFlags usage_flags, RHI_BufferPropertyFlags property_flags) {
	return _r_state.device.CreateBuffer(capacity, usage_flags, property_flags);
}

func U64
RHI_PushBuffer(RHI_Buffer buffer, U8* data, U64 size) {
	return _r_state.device.PushBuffer(buffer, data, size);
}

func void
RHI_ResetBuffer(RHI_Buffer buffer) {
	_r_state.device.ResetBuffer(buffer);
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
RHI_BindGlobalVertexShaderData(RHI_CommandBuffer command_buffer, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_info, I32 samplers_count, RHI_SamplerBindingInfo* sampler_info) {
  _r_state.device.BindShaderData(command_buffer, RHI_ShaderKind_Vertex, 1, uniform_buffers_count, uniform_info, samplers_count, sampler_info);
}

func void
RHI_BindInstanceVertexShaderData(RHI_CommandBuffer command_buffer, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_info, I32 samplers_count, RHI_SamplerBindingInfo* sampler_info) {
	_r_state.device.BindShaderData(command_buffer, RHI_ShaderKind_Vertex, 0, uniform_buffers_count, uniform_info, samplers_count, sampler_info);
}

func void
RHI_BindGlobalFragmentShaderData(RHI_CommandBuffer command_buffer, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_info, I32 samplers_count, RHI_SamplerBindingInfo* sampler_info)
{
  _r_state.device.BindShaderData(command_buffer, RHI_ShaderKind_Fragment, 1, uniform_buffers_count, uniform_info, samplers_count, sampler_info);
}

func void
RHI_BindInstanceFragmentShaderData(RHI_CommandBuffer command_buffer, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_info, I32 samplers_count, RHI_SamplerBindingInfo* sampler_info) {
	_r_state.device.BindShaderData(command_buffer, RHI_ShaderKind_Fragment, 0, uniform_buffers_count, uniform_info, samplers_count, sampler_info);
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
RHI_BeginRenderPass(RHI_CommandBuffer command_buffer, U32 color_targets_count, RHI_ColorTarget* color_targets, RHI_DepthStencilTarget* depth_stencil_target) {
	return _r_state.device.BeginRenderPass(command_buffer, color_targets_count, color_targets, depth_stencil_target);
}

func void
RHI_EndRenderPass(RHI_CommandBuffer command_buffer, RHI_RenderPass* render_pass) {
	_r_state.device.EndRenderPass(command_buffer, render_pass);
}

// -------------------------------------------------------------------
// -- Graphics Pipeline ----------------------------------------------
func U32
GetSizeOfVertexAttributeFormat(RHI_VertexAttributeFormat format) {
	U32 size_table[] = {
		sizeof(Vec2F32),
		sizeof(Vec3F32),
		sizeof(Vec4F32),
    sizeof(Vec4I32),
		0 // FORMAT_COUNT
	};

	return size_table[format];
}

func U32
_R_GlslangStageFromShaderType(RHI_ShaderKind kind) {
  switch (kind) {
    case RHI_ShaderKind_Vertex:   return GLSLANG_STAGE_VERTEX;
    case RHI_ShaderKind_Fragment: return GLSLANG_STAGE_FRAGMENT;

    default: Assert(0 && "Frong shader kind (Vertex and Fragment are available)"); return 0;
  }
}

func RHI_Shader
RHI_CreateShader(Arena* arena, RHI_ShaderCreateInfo* info) {
	RHI_Shader out_shader = {0};

  FILE* file = fopen(CFromStr8(info->file_name), "r");
  Assert(file);

  fseek(file, 0L, SEEK_END);
  U32 shader_code_size = ftell(file);
  U8* shader_code = (U8*)PushArena(arena, shader_code_size * sizeof(U8));
  rewind(file);
  fread(shader_code, shader_code_size * sizeof(U8), 1, file);
  fclose(file);

  glslang_initialize_process();

  glslang_input_t input = {0};
  input.language = GLSLANG_SOURCE_GLSL,
  input.stage = (glslang_stage_t)_R_GlslangStageFromShaderType(info->kind);
  input.client = GLSLANG_CLIENT_VULKAN;
  input.client_version = GLSLANG_TARGET_VULKAN_1_3;
  input.target_language = GLSLANG_TARGET_SPV;
  input.target_language_version = GLSLANG_TARGET_SPV_1_6;
  input.code = (const char*)shader_code;
  input.default_version = 100;
  input.default_profile = GLSLANG_NO_PROFILE;
  input.force_default_version_and_profile = 0;
  input.forward_compatible = 0;
  input.messages = GLSLANG_MSG_DEFAULT_BIT;
  input.resource = glslang_default_resource();

  LogInfo("Compiling shader \"%s\" ...\n", CFromStr8(info->file_name));

  glslang_shader_t* shader = glslang_shader_create(&input);

  if (!glslang_shader_preprocess(shader, &input)) {
    LogError("GLSL preprocessing failed");
    LogError("%s", glslang_shader_get_info_log(shader));
    LogError("%s", glslang_shader_get_info_debug_log(shader));
    glslang_shader_delete(shader);
    Assert(0);
  }

  if (!glslang_shader_parse(shader, &input)) {
    LogError("GLSL parsing failed");
    LogError("%s", glslang_shader_get_info_log(shader));
    LogError("%s", glslang_shader_get_info_debug_log(shader));
    // LogError("%s", glslang_shader_get_preprocessed_code(shader));
    glslang_shader_delete(shader);
    Assert(0);
  }

  glslang_program_t* program = glslang_program_create();
  glslang_program_add_shader(program, shader);

  if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
    LogError("GLSL linking failed");
    LogError("%s", glslang_program_get_info_log(program));
    LogError("%s", glslang_program_get_info_debug_log(program));
    glslang_program_delete(program);
    glslang_shader_delete(shader);
    Assert(0);
  }

  glslang_program_SPIRV_generate(program, input.stage);
  
  out_shader.kind = info->kind;
  out_shader.language = RHI_ShaderLanguage_SPIRV;
  out_shader.code_size = 4 * glslang_program_SPIRV_get_size(program);
  out_shader.code = (U8*)PushArena(arena, out_shader.code_size * sizeof(U8));
	out_shader.global_uniforms_count = info->global_uniforms_count;
  out_shader.global_samplers_count = info->global_samplers_count;
	out_shader.instance_uniforms_count = info->instance_uniforms_count;
	out_shader.instance_samplers_count = info->instance_samplers_count;

  glslang_program_SPIRV_get(program, (U32*)out_shader.code);

  const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
  if (spirv_messages) {
    LogError("(%s) %s\b");
  }

  glslang_program_delete(program);
  glslang_shader_delete(shader);
  glslang_finalize_process();

	return out_shader;
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
RHI_Init(RHI_RendererKind kind, OS_Window* window) {
#ifdef IGNIS_PLATFORM_MACOS
  if (kind == RHI_RendererKind_Metal) {
    // --AlNov: @TODO Should be available only on MacOS
    // AssignDeviceFunctions(Metal);
  }
	else {
		AssertMessage(0, "Wrong type of rendering backend\n");
	}
#else
	if (kind == RHI_RendererKind_Vulkan) {
		AssignDeviceFunctions(VK);
	}
	else {
		AssertMessage(0, "Wrong type of rendering backend\n");
	}
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

