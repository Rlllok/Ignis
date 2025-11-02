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
  Vec4I32 joint_ids;
  Vec4F32 joint_weights;
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

typedef U32 AST_JointID;
#define AST_JointID_Nil U32_MAX
typedef struct AST_Joint AST_Joint;
struct AST_Joint
{
  Vec3F32 position;
  Vec3F32 scale;
  Quaternion rotation;

  Vec3F32 g_position;
  Vec3F32 g_scale;
  Quaternion g_rotation;

  Mat4F32 inverse_bind_transform;

  AST_JointID parent_id;
};
AST_Joint _ast_joint_nil = {.parent_id = AST_JointID_Nil};
DefineArray(AST_Joint, AST_JointArray, _ast_joint_nil)

typedef struct AST_StaticMesh AST_StaticMesh;
struct AST_StaticMesh
{
  Str8 name;

  AST_GeometryList geometry_list;
  AST_JointArray joints;
};

func AST_Geometry AST_LoadGeometryFromGLTF(Arena* arena, Str8 gltf_name);
func AST_StaticMesh AST_LoadStaticMeshFromGLTF(Arena* arena, Str8 gltf_name);
