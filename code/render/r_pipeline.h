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
  U32               code_size;
  U8*               code;
};

enum R_VertexAttributeFormat
{
  R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
  R_VERTEX_ATTRIBUTE_FORMAT_VEC2F,

  R_VERTEX_ATTRIBUTE_FORMAT_COUNT
};

enum R_BindingType
{
  R_BINDING_TYPE_UNIFORM_BUFFER,
  R_BINDING_TYPE_TEXTURE_2D,

  R_BINDING_TYPE_COUNT
};

#define MAX_BINDINGS 10
struct R_BindingInfo
{
  R_BindingType type;
  R_ShaderType  shader_type;
};

#define MAX_ATTRIBUTES 10
struct R_Pipeline
{
  U32                     backend_handle;
  R_Shader                shaders[R_SHADER_TYPE_COUNT];
  R_VertexAttributeFormat attributes[MAX_ATTRIBUTES];
  U32                     attributes_count;

  R_BindingInfo scene_bindings[MAX_BINDINGS];
  U32           scene_bindings_count;
  R_BindingInfo instance_bindings[MAX_BINDINGS];
  U32           instance_bindings_count;

  B32 is_depth_test_enabled;
};

func void R_H_LoadShader(Arena* arena, const char* path, const char* entry_point, R_ShaderType type, R_Shader* out_shader);
func void R_H_LoadShaderSPIRV(Arena* arena, const char* path, const char* entry_point, R_ShaderType type, R_Shader* out_shader);

func void R_PipelineAssignAttributes(R_Pipeline* pipeline, R_VertexAttributeFormat* formats, U32 count);
func void R_PipelineAssignSceneBindingLayout(R_Pipeline* pipeline, R_BindingInfo* bindings, U32 count);
func void R_PipelineAssignInstanceBindingLayout(R_Pipeline* pipeline, R_BindingInfo* bindings, U32 count);

// --AlNov: @TODO Is it really needed or there is another way to get offset between attributes
func U32 R_H_OffsetFromAttributeFormat(R_VertexAttributeFormat format);
func U32 R_H_GlslangStageFromShaderType(R_ShaderType type);
