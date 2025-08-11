#pragma once

#include "../base/base_include.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"

typedef struct R_Texture R_Texture;
struct R_Texture
{
  U64 handle;

  Str8 name;
  U32 size_bytes;
  I32 width;
  I32 height;
  I32 channels;
  U32 size;
};
