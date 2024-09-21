#pragma once

#include "../base/base_include.h"

struct R_Texture
{
  U64 handle;

  Str8  name;
  U32   size_bytes;
  I32   width;
  I32   height;
  I32   channels;
};
