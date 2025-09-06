#pragma once

#include "base/base_include.h"
#include "gltf.h"

typedef struct AST_Vertex AST_Vertex;
struct AST_Vertex
{
  Vec3 position;
  Vec3 normal;
  Vec3 tangent;
  Vec2 uv;
};

typedef struct AST_Geometry AST_Geometry;
struct AST_Geometry
{
  U8* index_data;
  U32 index_size;
  U32 index_count;
  
  AST_Vertex* vertecies;
  U64 vertecies_count;

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
