#include "r_pipeline.h"

#include <stdio.h>

func void R_H_LoadShader(Arena* arena, const char* path, const char* entry_point, R_ShaderType type, R_Shader* out_shader)
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

func void R_PipelineAddAttribute(R_Pipeline* pipeline, R_VertexAttributeFormat format)
{
  ASSERT(pipeline->attributes_count == MAX_ATTRIBUTES);

  pipeline->attributes[pipeline->attributes_count] = format;
  pipeline->attributes_count += 1;
}

func void R_PipelineAddUniform(R_Pipeline* pipeline, R_UniformType type)
{
  ASSERT(pipeline->uniforms.count == MAX_UNIFORMS);

  pipeline->uniforms.types[pipeline->uniforms.count] = type;

  u32 size = 0;
  switch(type)
  {
    case R_UNIFORM_TYPE_VEC3F:
    {
      size                  = sizeof(Vec3f);
      u32 allignment_amount = 16 - size % 16; // --AlNov: Allignment as VK specification suggest
      size                  += allignment_amount;
    } break;
    case R_UNIFORM_TYPE_MAT4x4F:
    {
      size                  = sizeof(Mat4x4f);
      u32 allignment_amount = 16 - size % 16;
      size                  += allignment_amount;
    } break;

    default: ASSERT(1);
  }

  pipeline->uniforms.size += size;
  pipeline->uniforms.count += 1;
}

func u32 R_H_OffsetFromAttributeFormat(R_VertexAttributeFormat format)
{
  switch (format)
  {
    case R_VERTEX_ATTRIBUTE_FORMAT_R32G32_SFLOAT    : return sizeof(Vec2f);
    case R_VERTEX_ATTRIBUTE_FORMAT_R32G32B32_SFLOAT : return sizeof(Vec3f);
    
    default: ASSERT(1); return 0;
  }
}