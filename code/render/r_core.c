#include "r_core.h"

#include "base/base_logger.h"

#include "third_party/glslang/include/Include/glslang_c_interface.h"
#include "third_party/glslang/include/Public/resource_limits_c.h"

// -------------------------------------------------------------------
// Command Buffer
func R_CommandBuffer
R_GetCommandBuffer(void)
{
	return _r_state.device.GetCommandBuffer();
}

func void
R_BeginCommandBuffer(R_CommandBuffer command_buffer)
{
	_r_state.device.BeginCommandBuffer(command_buffer);
}

func void
R_SubmitCommandBuffer(R_CommandBuffer command_buffer)
{
	_r_state.device.SubmitCommandBuffer(command_buffer);
}

// -------------------------------------------------------------------
// Buffer
func R_Buffer R_CreateBuffer(U32 capacity, R_BufferUsageFlags usage_flags, R_BufferPropertyFlags property_flags)
{
	return _r_state.device.CreateBuffer(capacity, usage_flags, property_flags);
}

func U64
R_PushBuffer(R_Buffer buffer, U8* data, U64 size)
{
	return _r_state.device.PushBuffer(buffer, data, size);
}

func void
R_ResetBuffer(R_Buffer buffer)
{
	_r_state.device.ResetBuffer(buffer);
}

func void
R_BindIndexBuffer(R_CommandBuffer command_buffer, R_Buffer buffer, U64 offset, R_IndexSize index_size)
{
	_r_state.device.BindIndexBuffer(command_buffer, buffer, offset, index_size);
}

func void R_BindVertexBuffer(R_CommandBuffer command_buffer, R_Buffer buffer, U64 offset)
{
	_r_state.device.BindVertexBuffer(command_buffer, buffer, offset);
}

// -------------------------------------------------------------------
// Texture
func R_Texture
R_CreateTexture(R_TextureCreateInfo* info)
{
  return _r_state.device.CreateTexture(info);
}

func B32
R_DestroyTexture(R_Texture texture)
{
  return _r_state.device.DestroyTexture(texture);
}

func void R_LoadImageToTexture(Str8 image_path, R_Texture texture)
{
  _r_state.device.LoadDataToTexture(0, 0, texture);
}

func void
R_CopyTexture(R_CommandBuffer command_buffer, R_Texture source, R_Texture destination)
{
  _r_state.device.CopyTexture(command_buffer, source, destination);
}

func U64
R_CopyTextureToBuffer(R_CommandBuffer command_buffer, R_Texture texture, R_Buffer buffer)
{
  return _r_state.device.CopyTextureToBuffer(command_buffer, texture, buffer);
}

func void
R_CopyBufferToTexture(R_CommandBuffer command_buffer, R_Buffer buffer, U64 offset, U64 size, R_Texture texture)
{
  _r_state.device.CopyBufferToTexture(command_buffer, buffer, offset, size, texture);
}

func R_TextureFormat
R_GetTextureFormat(R_Texture texture)
{
  return _r_state.device.GetTextureFormat(texture);
}

func Vec2I32
R_GetTextureDimension(R_Texture texture)
{
  return _r_state.device.GetTextureDimension(texture);
}

func R_TextureSampler
R_CreateTextureSampler(R_TextureSamplerCreateInfo* info)
{
  return _r_state.device.CreateTextureSampler(info);
}

// -------------------------------------------------------------------
// Uniform Data
func void
R_BindGlobalVertexShaderData(R_CommandBuffer command_buffer, I32 uniform_buffers_count, R_UniformBufferBindingInfo* uniform_info, I32 samplers_count, R_SamplerBindingInfo* sampler_info)
{
	_r_state.device.BindGlobalShaderData(command_buffer, R_SHADER_TYPE_VERTEX, uniform_buffers_count, uniform_info, samplers_count, sampler_info);
}

func void
R_BindInstanceVertexShaderData(R_CommandBuffer command_buffer, I32 uniform_buffers_count, R_UniformBufferBindingInfo* uniform_info, I32 samplers_count, R_SamplerBindingInfo* sampler_info)
{
	_r_state.device.BindInstanceShaderData(command_buffer, R_SHADER_TYPE_VERTEX, uniform_buffers_count, uniform_info, samplers_count, sampler_info);
}

func void
R_BindGlobalFragmentShaderData(R_CommandBuffer command_buffer, I32 uniform_buffers_count, R_UniformBufferBindingInfo* uniform_info, I32 samplers_count, R_SamplerBindingInfo* sampler_info)
{
	_r_state.device.BindGlobalShaderData(command_buffer, R_SHADER_TYPE_FRAGMENT, uniform_buffers_count, uniform_info, samplers_count, sampler_info);
}

func void
R_BindInstanceFragmentShaderData(R_CommandBuffer command_buffer, I32 uniform_buffers_count, R_UniformBufferBindingInfo* uniform_info, I32 samplers_count, R_SamplerBindingInfo* sampler_info)
{
	_r_state.device.BindInstanceShaderData(command_buffer, R_SHADER_TYPE_FRAGMENT, uniform_buffers_count, uniform_info, samplers_count, sampler_info);
}


// -------------------------------------------------------------------
// Swapchain
func R_TextureFormat
R_GetSwapchainTextureFormat()
{
  return _r_state.device.GetSwapchainTextureFormat();
}

func R_Texture
R_AcquireSwapchainTexture(R_CommandBuffer command_buffer)
{
	return _r_state.device.AcquireSwapchainTexture(command_buffer);
}

// -------------------------------------------------------------------
// Render Pass
func R_RenderPass*
R_BeginRenderPass(R_CommandBuffer command_buffer, U32 color_targets_count, R_ColorTarget* color_targets, R_DepthStencilTarget* depth_stencil_target)
{
	return _r_state.device.BeginRenderPass(command_buffer, color_targets_count, color_targets, depth_stencil_target);
}

func void
R_EndRenderPass(R_CommandBuffer command_buffer, R_RenderPass* render_pass)
{
	_r_state.device.EndRenderPass(command_buffer, render_pass);
}

// -------------------------------------------------------------------
// Graphics Pipeline
func U32
GetSizeOfVertexAttributeFormat(R_VertexAttributeFormat format)
{
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
_R_GlslangStageFromShaderType(R_ShaderType type)
{
  switch (type)
  {
    case R_SHADER_TYPE_VERTEX   : return GLSLANG_STAGE_VERTEX;
    case R_SHADER_TYPE_FRAGMENT : return GLSLANG_STAGE_FRAGMENT;

    default: Assert(0); return 0; // --AlNov: type cannot be used with glslang
  }
}

func R_Shader
R_CreateShader(Arena* arena, R_ShaderCreateInfo* info)
{
	R_Shader out_shader = {0};

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
  input.language                          = GLSLANG_SOURCE_GLSL,
  input.stage                             = (glslang_stage_t)_R_GlslangStageFromShaderType(info->type);
  input.client                            = GLSLANG_CLIENT_VULKAN;
  input.client_version                    = GLSLANG_TARGET_VULKAN_1_3;
  input.target_language                   = GLSLANG_TARGET_SPV;
  input.target_language_version           = GLSLANG_TARGET_SPV_1_6;
  input.code                              = (const char*)shader_code;
  input.default_version                   = 100;
  input.default_profile                   = GLSLANG_NO_PROFILE;
  input.force_default_version_and_profile = false;
  input.forward_compatible                = false;
  input.messages                          = GLSLANG_MSG_DEFAULT_BIT;
  input.resource                          = glslang_default_resource();

  LOG_INFO("Compiling shader \"%s\" ...\n", CFromStr8(info->file_name));

  glslang_shader_t* shader = glslang_shader_create(&input);

  if (!glslang_shader_preprocess(shader, &input))
  {
    LOG_ERROR("GLSL preprocessing failed");
    LOG_ERROR("%s", glslang_shader_get_info_log(shader));
    LOG_ERROR("%s", glslang_shader_get_info_debug_log(shader));
    glslang_shader_delete(shader);
    Assert(0);
  }

  if (!glslang_shader_parse(shader, &input))
  {
    LOG_ERROR("GLSL parsing failed");
    LOG_ERROR("%s", glslang_shader_get_info_log(shader));
    LOG_ERROR("%s", glslang_shader_get_info_debug_log(shader));
    // LOG_ERROR("%s", glslang_shader_get_preprocessed_code(shader));
    glslang_shader_delete(shader);
    Assert(0);
  }

  glslang_program_t* program = glslang_program_create();
  glslang_program_add_shader(program, shader);

  if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
  {
    LOG_ERROR("GLSL linking failed");
    LOG_ERROR("%s", glslang_program_get_info_log(program));
    LOG_ERROR("%s", glslang_program_get_info_debug_log(program));
    glslang_program_delete(program);
    glslang_shader_delete(shader);
    Assert(0);
  }

  glslang_program_SPIRV_generate(program, input.stage);
  
  out_shader.type        = info->type;
  out_shader.language    = R_SHADER_LANGUAGE_SPIRV;
  out_shader.code_size   = 4 * glslang_program_SPIRV_get_size(program);
  out_shader.code        = (U8*)PushArena(arena, out_shader.code_size * sizeof(U8));
	out_shader.global_uniforms_count = info->global_uniforms_count;
  out_shader.global_samplers_count = info->global_samplers_count;
	out_shader.instance_uniforms_count = info->instance_uniforms_count;
	out_shader.instance_samplers_count = info->instance_samplers_count;

  glslang_program_SPIRV_get(program, (U32*)out_shader.code);

  const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
  if (spirv_messages)
  {
    LOG_ERROR("(%s) %s\b");
  }

  glslang_program_delete(program);
  glslang_shader_delete(shader);
  glslang_finalize_process();

	return out_shader;
}

func R_GraphicsPipeline
R_CreateGraphicsPipeline(R_GraphicsPipelineCreateInfo* info)
{
	return _r_state.device.CreateGraphicsPipeline(info);
}

func void
R_BindGraphicsPipeline(R_CommandBuffer command_buffer, R_GraphicsPipeline pipeline)
{
	_r_state.device.BindGraphicsPipeline(command_buffer, pipeline);
}

// -------------------------------------------------------------------
// Draw
func void
R_SetViewport(R_CommandBuffer command_buffer, RectI32 viewport)
{
	_r_state.device.SetViewport(command_buffer, viewport);
}

func void
R_SetScissor(R_CommandBuffer command_buffer, RectI32 scissor)
{
	_r_state.device.SetScissor(command_buffer, scissor);
}

func void
R_DrawPrimitives(R_CommandBuffer command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance)
{
	_r_state.device.DrawPrimitives(command_buffer, vertex_count, instance_count, first_vertex, first_instance);
}

func void
R_DrawIndexedPrimitives(R_CommandBuffer command_buffer, U32 index_count, U32 instance_count, U32 first_index, I32 vertex_offset, U32 first_instance)
{
	_r_state.device.DrawIndexedPrimitives(command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
}

func void
R_PresentTexture(R_CommandBuffer command_buffer, R_Texture texture)
{
  _r_state.device.PresentTexture(command_buffer, texture);
}

// -------------------------------------------------------------------
// State
func B32 
R_Init(R_RendererType type, OS_Window* window)
{
	if ( type == R_RENDERER_TYPE_VK)
	{
		AssignDeviceFunctions(VK);
	}
	else
	{
		AssertMessage(0, "Wrong type of rendering backend\n");
	}

	_r_state.device.Init(window);

  return true;
}

func B32
R_Shutdown(void)
{
  // --AlNov: @TODO Zero out _r_state.device struct
	return _r_state.device.Shutdown();
}

