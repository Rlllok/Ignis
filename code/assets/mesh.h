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

typedef U32 JointID; // --AlNov: @NOTE Index in array of joints in skeleton struct;
#define JointID_Nil U32_MAX
typedef struct Joint Joint;
struct Joint
{
  Str8 name;
  JointID parent_id;
  Transform local_transform;
  Transform global_transform;
  Mat4F32 inv_bind_pose;
};
Joint _joint_nil = {
  .parent_id = JointID_Nil,
  .local_transform = IdentityTransform(),
  .global_transform = IdentityTransform(),
};
DefineArray(Joint, JointArray, _joint_nil)

typedef struct JointPose JointPose;
struct JointPose
{
  Transform local_transform;
  Transform global_transform;
};
JointPose _joint_pose_nil = {.local_transform = IdentityTransform(), .global_transform = IdentityTransform()};
DefineArray(JointPose, JointPoseArray, _joint_pose_nil)

typedef U32 SkeletonID;
#define SkeletonID_Nil U32_MAX;
typedef struct Skeleton Skeleton;
struct Skeleton
{
  JointArray joints;
};
Skeleton _skeleton_nil = {0};
DefineArray(Skeleton, SkeletonArray, _skeleton_nil)

func SkeletonID CreateSkeleton();

typedef struct SkeletonKeySample SkeletonKeySample;
struct SkeletonKeySample
{
  TransformArray local_joint_transforms;
  U64 timestamp;
};
SkeletonKeySample _skeleton_key_sample_nil = {0};
DefineArray(SkeletonKeySample, SkeletonKeySampleArray, _skeleton_key_sample_nil)

typedef U32 SkeletonAnimationID;
#define SkeletonAnimationID_Nil U32_MAX
SkeletonAnimationID _skeleton_animation_id_nil = SkeletonAnimationID_Nil;
DefineArray(SkeletonAnimationID, SkeletonAnimationIDArray, _skeleton_animation_id_nil)

typedef struct SkeletonAnimation SkeletonAnimation;
struct SkeletonAnimation
{
  SkeletonAnimationID id;
  SkeletonID skeleton_id;

  U64 duration;
  U64 start_time;
  U64 end_time;
  SkeletonKeySampleArray key_samples; // --AlNov: @TODO This is the same for every skeleton (start_time and end_time what is different)
};
SkeletonAnimation _skeletal_animation_nil = {0};
DefineArray(SkeletonAnimation, SkeletonAnimationArray, _skeletal_animation_nil)

typedef struct AST_StaticMesh AST_StaticMesh;
struct AST_StaticMesh
{
  Str8 name;

  AST_GeometryList geometry_list;
  Skeleton skeleton;
  SkeletonAnimation animation;
};

func AST_Geometry AST_LoadGeometryFromGLTF(Arena* arena, Str8 gltf_name);
func AST_StaticMesh AST_LoadStaticMeshFromGLTF(Arena* arena, Str8 gltf_name);
