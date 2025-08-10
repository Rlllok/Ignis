#pragma once

#include "base/base_include.h"

#define R_MAX_ATTRIBUTES 10
struct R_Pipeline
{
  PipelineID backend_handle = INVALID_ID;
  R_Shader                shaders[R_SHADER_TYPE_COUNT];
  R_VertexAttributeFormat attributes[R_MAX_ATTRIBUTES];
  U32                     attributes_count;

  R_BindingInfo global_bindings[R_MAX_BINDINGS];
  U32           global_bindings_count;
  R_BindingInfo instance_bindings[R_MAX_BINDINGS];
  U32           instance_bindings_count;

  B32 is_back_culing_enabled;
  B32 is_depth_test_enabled;
};

func void R_H_LoadShader(Arena* arena, const char* path, const char* entry_point, R_ShaderType type, R_Shader* out_shader);
func void R_H_LoadShaderSPIRV(Arena* arena, const char* path, const char* entry_point, R_ShaderType type, R_Shader* out_shader);

func void R_PipelineAssignAttributes(R_Pipeline* pipeline, R_VertexAttributeFormat* formats, U32 count);
func void R_PipelineAssignGlobalBindingLayout(R_Pipeline* pipeline, R_BindingInfo* bindings, U32 count);
func void R_PipelineAssignInstanceBindingLayout(R_Pipeline* pipeline, R_BindingInfo* bindings, U32 count);

// --AlNov: @TODO Is it really needed or there is another way to get offset between attributes
func U32 R_H_OffsetFromAttributeFormat(R_VertexAttributeFormat format);
func U32 R_H_GlslangStageFromShaderType(R_ShaderType type);
