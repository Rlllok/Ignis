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