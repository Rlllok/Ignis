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

enum R_VertexAttributeFormat
{
  R_VERTEX_ATTRIBUTE_FORMAT_R32G32B32_SFLOAT,
  R_VERTEX_ATTRIBUTE_FORMAT_R32G32_SFLOAT,

  R_VERTEX_ATTRIBUTE_FORMAT_COUNT
};

#define MAX_ATTRIBUTES 10
struct R_Pipeline
{
  u32                     backend_handle;
  R_Shader                shaders[R_SHADER_TYPE_COUNT];
  R_VertexAttributeFormat attributes[MAX_ATTRIBUTES];
  u32                     attributes_count;
};

func void R_H_LoadShader(Arena* arena, const char* path, const char* entry_point, R_ShaderType type, R_Shader* out_shader);

func void R_PipelineAddAttribute(R_Pipeline* pipeline, R_VertexAttributeFormat);

// --AlNov: @TODO Is it really needed or there is another way to get offset between attributes
func u32 R_H_OffsetFromAttributeFormat(R_VertexAttributeFormat format);