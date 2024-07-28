#pragma once

#include "../base/base_include.h"

enum R_ShaderType
{
  R_SHADER_TYPE_VERTEX,
  R_SHADER_TYPE_FRAGMENT,
  
  R_SHADER_TYPE_COUNT
};

enum R_ShaderLanguage
{
  R_SHADER_LANGUAGE_SPIRV,

  R_SHADER_LANGUAGE_COUNT
};

struct R_Shader
{
  R_ShaderType      type;
  R_ShaderLanguage  language; // --AlNov: Only SPIRV for now
  const char*       entry_point;
  u32               code_size;
  u8*               code;
};

struct R_Pipeline
{
  u32      backend_handle;
  R_Shader shaders[R_SHADER_TYPE_COUNT];
};

func void R_H_LoadShader(Arena* arena, const char* path, const char* entry_point, R_ShaderType type, R_Shader* out_shader);