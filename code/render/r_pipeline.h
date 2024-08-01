#pragma once

#include "base/base_include.h"

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

enum R_UniformType
{
  R_UNIFORM_TYPE_VEC3F,
  R_UNIFORM_TYPE_MAT4x4F,
  
  R_UNIFORM_TYPE_COUNT
};

#define MAX_UNIFORMS 3
struct R_Uniforms
{
  R_UniformType types[MAX_UNIFORMS];
  // u32           offsets[MAX_UNIFORMS];
  u32           size;
  u32           count;
};

#define MAX_ATTRIBUTES 10
struct R_Pipeline
{
  u32                     backend_handle;
  R_Shader                shaders[R_SHADER_TYPE_COUNT];
  R_VertexAttributeFormat attributes[MAX_ATTRIBUTES];
  u32                     attributes_count;
  R_Uniforms              uniforms;
};

func void R_H_LoadShader(Arena* arena, const char* path, const char* entry_point, R_ShaderType type, R_Shader* out_shader);
func void R_H_LoadShaderSPIRV(Arena* arena, const char* path, const char* entry_point, R_ShaderType type, R_Shader* out_shader);

func void R_PipelineAddAttribute(R_Pipeline* pipeline, R_VertexAttributeFormat format);
func void R_PipelineAddUniform(R_Pipeline* pipeline, R_UniformType type);

// --AlNov: @TODO Is it really needed or there is another way to get offset between attributes
func u32 R_H_OffsetFromAttributeFormat(R_VertexAttributeFormat format);
func u32 R_H_GlslangStageFromShaderType(R_ShaderType type);