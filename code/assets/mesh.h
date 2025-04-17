#pragma once

#include "base/base_include.h"
#include "gltf.h"

struct AST_Geometry
{
  U8* index_data;
  U64 index_size;
  U64 index_count;
  U64 index_r_backend_offset;
  
  U8* vertex_data;
  U64 vertex_size;
  U64 vertex_count;
  U64 vertex_r_backend_offset;
};

func AST_Geometry AST_LoadGeometryFromGLTF(const char* gltf_name);
