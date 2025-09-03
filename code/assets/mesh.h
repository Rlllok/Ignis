#pragma once

#include "base/base_include.h"
#include "gltf.h"

typedef struct AST_Geometry AST_Geometry;
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

  AST_Geometry* next;
};
DefineList(AST_Geometry, AST_GeometryList)

typedef struct AST_StaticMesh AST_StaticMesh;
struct AST_StaticMesh
{
  Str8 name;

  AST_GeometryList geometry_list;
};

func AST_Geometry AST_LoadGeometryFromGLTF(Arena* arena, Str8 gltf_name);
func AST_StaticMesh AST_LoadStaticMeshFromGLTF(Arena* arena, Str8 gltf_name);
