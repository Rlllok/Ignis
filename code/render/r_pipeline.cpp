#include "r_pipeline.h"

#pragma comment(lib, "third_party/glslang/lib/GenericCodeGen.lib")
#pragma comment(lib, "third_party/glslang/lib/glslang.lib")
#pragma comment(lib, "third_party/glslang/lib/glslang-default-resource-limits.lib")
#pragma comment(lib, "third_party/glslang/lib/MachineIndependent.lib")
#pragma comment(lib, "third_party/glslang/lib/OSDependent.lib")
#pragma comment(lib, "third_party/glslang/lib/SPIRV.lib")
#pragma comment(lib, "third_party/glslang/lib/SPIRV-Tools.lib")
#pragma comment(lib, "third_party/glslang/lib/SPIRV-Tools-opt.lib")
#pragma comment(lib, "third_party/glslang/lib/SPVRemapper.lib")

#include "third_party/glslang/include/Include/glslang_c_interface.h"
#include "third_party/glslang/include/Public/resource_limits_c.h"

#include <stdio.h>
#include <memory.h>

func void
R_H_LoadShader(Arena* arena, const char* path, const char* entry_point, R_ShaderType type, R_Shader* out_shader)
{
  FILE* file = fopen(path, "r");
  ASSERT(!file);

  fseek(file, 0L, SEEK_END);
  u32 shader_code_size = ftell(file);
  u8* shader_code = (u8*)PushArena(arena, shader_code_size * sizeof(u8));
  rewind(file);
  fread(shader_code, shader_code_size * sizeof(u8), 1, file);
  fclose(file);

  glslang_initialize_process();

  glslang_input_t input = {};
  input.language                          = GLSLANG_SOURCE_GLSL,
  input.stage                             = (glslang_stage_t)R_H_GlslangStageFromShaderType(type);
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

  LOG_INFO("Compiling shader \"%s\" ...\n", path);

  glslang_shader_t* shader = glslang_shader_create(&input);

  if (!glslang_shader_preprocess(shader, &input))
  {
    LOG_ERROR("GLSL preprocessing failed");
    LOG_ERROR("%s", glslang_shader_get_info_log(shader));
    LOG_ERROR("%s", glslang_shader_get_info_debug_log(shader));
    glslang_shader_delete(shader);
    ASSERT(true);
  }

  if (!glslang_shader_parse(shader, &input))
  {
    LOG_ERROR("GLSL parsing failed");
    LOG_ERROR("%s", glslang_shader_get_info_log(shader));
    LOG_ERROR("%s", glslang_shader_get_info_debug_log(shader));
    // LOG_ERROR("%s", glslang_shader_get_preprocessed_code(shader));
    glslang_shader_delete(shader);
    ASSERT(true);
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
    ASSERT(true);
  }

  glslang_program_SPIRV_generate(program, input.stage);

  *out_shader = {};
  
  out_shader->type        = type;
  out_shader->language    = R_SHADER_LANGUAGE_SPIRV;
  out_shader->entry_point = entry_point;
  out_shader->code_size   = 4 * glslang_program_SPIRV_get_size(program);
  out_shader->code        = (u8*)PushArena(arena, out_shader->code_size * sizeof(u8));
  glslang_program_SPIRV_get(program, (u32*)out_shader->code);

  const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
  if (spirv_messages)
  {
    LOG_ERROR("(%s) %s\b");
  }

  glslang_program_delete(program);
  glslang_shader_delete(shader);
  glslang_finalize_process();
}

func void
R_H_LoadShaderSPIRV(Arena* arena, const char* path, const char* entry_point, R_ShaderType type, R_Shader* out_shader)
{
  *out_shader = {};
  
  out_shader->type        = type;
  out_shader->language    = R_SHADER_LANGUAGE_SPIRV;
  out_shader->entry_point = entry_point;

  FILE* file = fopen(path, "rb");
  ASSERT(!file);

  fseek(file, 0L, SEEK_END);
  out_shader->code_size = ftell(file);
  out_shader->code = (u8*)PushArena(arena, out_shader->code_size * sizeof(u8));
  rewind(file);
  fread(out_shader->code, out_shader->code_size * sizeof(u8), 1, file);
  fclose(file);
}

func void
R_PipelineAssignAttributes(R_Pipeline* pipeline, R_VertexAttributeFormat* formats, u32 count)
{
  ASSERT(count > MAX_ATTRIBUTES);

  memcpy(pipeline->attributes, formats, count * sizeof(R_VertexAttributeFormat));
  pipeline->attributes_count = count;
}

func void
R_PipelineAssignBindingLayout(R_Pipeline* pipeline, R_BindingInfo* bindings, u32 count)
{
  ASSERT(count > MAX_BINDINGS);

  memcpy(pipeline->bindings, bindings, count * sizeof(R_BindingInfo));
  pipeline->bindings_count = count;
}

func u32
R_H_OffsetFromAttributeFormat(R_VertexAttributeFormat format)
{
  switch (format)
  {
    case R_VERTEX_ATTRIBUTE_FORMAT_VEC3F  : return sizeof(Vec3f);
    case R_VERTEX_ATTRIBUTE_FORMAT_VEC2F  : return sizeof(Vec2f);
    
    default: ASSERT(1); return 0;
  }
}

func u32
R_H_GlslangStageFromShaderType(R_ShaderType type)
{
  switch (type)
  {
    case R_SHADER_TYPE_VERTEX   : return GLSLANG_STAGE_VERTEX;
    case R_SHADER_TYPE_FRAGMENT : return GLSLANG_STAGE_FRAGMENT;

    default: ASSERT(1); return 0; // --AlNov: type cannot be used with glslang
  }
}