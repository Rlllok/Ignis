#pragma once

#include "mesh.h"

#include "gltf.c"

func AST_StaticMesh
AST_LoadStaticMeshFromGLTF(Arena* arena, Str8 gltf_name)
{
  AST_StaticMesh result = {0};
  result.geometry_list = AST_GeometryListCreate(arena);
  
  GLTFReader gltf_reader = {0};
  gltf_reader.file_path = gltf_name;
  gltf_reader.file_buffer = GLTFReadFile(gltf_name);

  GLTFData gltf_data = GetGLTFData(&gltf_reader);

  for (I32 i = 0; i < gltf_data.nodes.length; i += 1)
  {
    AST_Geometry geometry = {0};
    GLTFNode gltf_node = GLTFNodeArrayGet(&gltf_data.nodes, i);
    if (gltf_node.mesh_id == GLTF_ID_NIL) continue;

    GLTFMesh gltf_mesh = GLTFMeshListGetItem(&gltf_data.meshes, gltf_node.mesh_id);
    GLTFPrimitive gltf_primitive = gltf_mesh.primitives.first->data;

    GLTFAccessor index_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, gltf_primitive.indecies_accessor_id);
    GLTFAccessor position_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, gltf_primitive.position_accessor_id);
    GLTFAccessor normal_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, gltf_primitive.normal_accessor_id);
    GLTFAccessor texcoord_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, gltf_primitive.texcoord_accessor_id);
    GLTFAccessor joints_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, gltf_primitive.joints_accessor_id);
    GLTFAccessor weights_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, gltf_primitive.weights_accessor_id);

    GLTFBufferView index_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_views, index_accessor.buffer_view_id);
    GLTFBufferView position_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_views, position_accessor.buffer_view_id);
    GLTFBufferView normal_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_views, normal_accessor.buffer_view_id);
    GLTFBufferView texcoord_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_views, texcoord_accessor.buffer_view_id);
    GLTFBufferView joints_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_views, joints_accessor.buffer_view_id);
    GLTFBufferView weights_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_views, weights_accessor.buffer_view_id);

    GLTFBuffer index_buffer = GLTFBufferListGetItem(&gltf_data.buffers, index_buffer_view.buffer_id);
    geometry.index_data = index_buffer.data + index_accessor.byte_offset + index_buffer_view.byte_offset;
    geometry.index_size = index_buffer_view.byte_length / index_accessor.count;
    geometry.index_count = index_accessor.count;

    GLTFBuffer position_buffer = GLTFBufferListGetItem(&gltf_data.buffers, position_buffer_view.buffer_id);
    GLTFBuffer normal_buffer = GLTFBufferListGetItem(&gltf_data.buffers, normal_buffer_view.buffer_id);
    GLTFBuffer texcoord_buffer = GLTFBufferListGetItem(&gltf_data.buffers, texcoord_buffer_view.buffer_id);
    GLTFBuffer joints_buffer = GLTFBufferListGetItem(&gltf_data.buffers, joints_buffer_view.buffer_id);
    GLTFBuffer weights_buffer = GLTFBufferListGetItem(&gltf_data.buffers, weights_buffer_view.buffer_id);

    geometry.vertecies = (AST_Vertex*)PushArena(arena, position_accessor.count*sizeof(AST_Vertex));
    geometry.vertecies_count = position_accessor.count;
    for (U32 i = 0; i < position_accessor.count; i += 1)
    {
      AST_Vertex* vertex = geometry.vertecies + i;
      vertex->position = *((Vec3F32*)(position_buffer.data + position_accessor.byte_offset + position_buffer_view.byte_offset) + i);
      vertex->position = AddVec3F32(vertex->position, gltf_node.translation); // --AlNov: @NOTE Not sure what problems such translation can cause
      vertex->normal = *((Vec3F32*)(normal_buffer.data + normal_accessor.byte_offset + normal_buffer_view.byte_offset) + i);
      vertex->uv = *((Vec2F32*)(texcoord_buffer.data + texcoord_accessor.byte_offset + texcoord_buffer_view.byte_offset) + i);
      Vec4U8 joint_ids = *((Vec4U8*)(joints_buffer.data + joints_accessor.byte_offset + joints_buffer_view.byte_offset) + i);
      vertex->joint_ids.x = (I32)joint_ids.x;
      vertex->joint_ids.y = (I32)joint_ids.y;
      vertex->joint_ids.z = (I32)joint_ids.z;
      vertex->joint_ids.w = (I32)joint_ids.w;
      vertex->joint_weights = *((Vec4F32*)(weights_buffer.data + weights_accessor.byte_offset + weights_buffer_view.byte_offset) + i);
    }

    for (U32 i = 0; i < geometry.index_count; i += 3)
    {
      AST_Vertex* vertex_1 = geometry.vertecies + ((U16*)geometry.index_data)[i];
      AST_Vertex* vertex_2 = geometry.vertecies + ((U16*)geometry.index_data)[i+1];
      AST_Vertex* vertex_3 = geometry.vertecies + ((U16*)geometry.index_data)[i+2];

      Vec3F32 edge_1 = SubVec3F32(vertex_2->position, vertex_1->position);
      Vec3F32 edge_2 = SubVec3F32(vertex_3->position, vertex_1->position);
      Vec2F32 delta_uv_1 = SubVec2F32(vertex_2->uv, vertex_1->uv);
      Vec2F32 delta_uv_2 = SubVec2F32(vertex_3->uv, vertex_1->uv);

      F32 fractional_part = 1.0f/(delta_uv_1.x*delta_uv_2.y - delta_uv_2.x*delta_uv_1.y);
      Vec3F32 tangent = {
        .x = fractional_part*(delta_uv_2.y*edge_1.x - delta_uv_1.y*edge_2.x),
        .y = fractional_part*(delta_uv_2.y*edge_1.y - delta_uv_1.y*edge_2.y),
        .z = fractional_part*(delta_uv_2.y*edge_1.z - delta_uv_1.y*edge_2.z),
      };

      vertex_1->tangent = tangent;
      vertex_2->tangent = tangent;
      vertex_3->tangent = tangent;
    }

    AST_GeometryListPush(&result.geometry_list, geometry);
  }

  result.skeleton.joints = JointArrayAllocate(arena, gltf_data.skin.joint_ids.length);
  for (I32 i = 0; i < gltf_data.skin.joint_ids.length; i += 1)
  {
    GLTF_ID joint_id = GLTFJointIDArrayGet(&gltf_data.skin.joint_ids, i);
    GLTFNode node = GLTFNodeArrayGet(&gltf_data.nodes, joint_id);

    Joint joint = _joint_nil;
    joint.name = node.name;
    joint.local_transform.translation = node.translation;
    joint.local_transform.rotation = node.rotation;
    joint.local_transform.scale = node.scale;

    GLTFAccessor inverse_bind_matrices_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, gltf_data.skin.inverse_bind_matrices_accessor);
    GLTFBufferView inverse_bind_matrices_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_views, inverse_bind_matrices_accessor.buffer_view_id);
    GLTFBuffer inverse_bind_matrices_buffer = GLTFBufferListGetItem(&gltf_data.buffers, inverse_bind_matrices_buffer_view.buffer_id);
    joint.inv_bind_pose = *((Mat4F32*)(inverse_bind_matrices_buffer.data + inverse_bind_matrices_accessor.byte_offset + inverse_bind_matrices_buffer_view.byte_offset) + i);

    if (node.parent_id != GLTF_ID_NIL)
    {
      for (I32 j = 0; j < gltf_data.skin.joint_ids.length; j += 1)
      {
        if (node.parent_id == GLTFJointIDArrayGet(&gltf_data.skin.joint_ids, j))
        {
          joint.parent_id = j;
        }
      }
    }

    JointArrayAdd(&result.skeleton.joints, joint);
  }

  result.skeletal_animations = SkeletalAnimationArrayAllocate(arena, gltf_data.animations.count);
  for (I32 animation_index = 0; animation_index < result.skeletal_animations.capacity; animation_index += 1)
  {
    SkeletalAnimation skeletal_animation = {0};

    GLTFAnimation gltf_animation = GLTFAnimationListGetItem(&gltf_data.animations, animation_index);
    skeletal_animation.name = gltf_animation.name;

    {
      // --AlNov 7 January 2026: @TODO Dead code
      // GLTFSampler sampler = GLTFSamplerListGetItem(&gltf_animation.samplers, 0);
      // GLTFAccessor accessor = GLTFAccessorListGetItem(&gltf_data.accessors, sampler.input_accessor_id);
      skeletal_animation.bone_animations = AnimationArrayAllocate(arena, result.skeleton.joints.length);
      // --AlNov. 12  December 2025: @TODO
      // What is going on there. Should .length = .capacity? Can AnimationArrayAdd(...) be used? 
    }

    // --AlNov 7 January 2026: @TODO Use Scratch Arena
    ScratchArena scratch = BeginScratchArena(_r_vk_state.arena);
    {
      GLTFChannelArray translation_channels = GLTFChannelArrayAllocate(scratch.arena, result.skeleton.joints.length);
      GLTFChannelArray rotation_channels = GLTFChannelArrayAllocate(scratch.arena, result.skeleton.joints.length);
      GLTFChannelArray scale_channels = GLTFChannelArrayAllocate(scratch.arena, result.skeleton.joints.length);

      // @TODO List should be changed to Array
      for (I32 i = 0; i < gltf_animation.channels.count; i += 1)
      {
        GLTFChannel channel = GLTFChannelListGetItem(&gltf_animation.channels, i);

        JointID joint_id = JointID_Nil;
        for (I32 j = 0; j < gltf_data.skin.joint_ids.length; j += 1)
        {
          if (GLTFJointIDArrayGet(&gltf_data.skin.joint_ids, j) == channel.target.node_id)
          {
            joint_id = j;
            break;
          }
        }

        if (joint_id == JointID_Nil)
        {
          continue;
        }

        switch (channel.target.type)
        {
          default:
          {
            AssertMessage(0, "Wrong Channel Target\n");
          }

          case GLTFTargetType_Translation:
          {
            GLTFChannelArrayAdd(&translation_channels, channel);
          } break;

          case GLTFTargetType_Rotation:
          {
            GLTFChannelArrayAdd(&rotation_channels, channel);
          } break;

          case GLTFTargetType_Scale:
          {
            GLTFChannelArrayAdd(&scale_channels, channel);
          } break;
        }
      }

      for (I32 i = 0; i < skeletal_animation.bone_animations.capacity; i += 1)
      {
        Animation bone_animation = {0};
        GLTFChannel tmp_channel = GLTFChannelArrayGet(&translation_channels, i);
        GLTFSampler tmp_sampler = GLTFSamplerListGetItem(&gltf_animation.samplers, tmp_channel.sampler_id);
        GLTFAccessor tmp_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, tmp_sampler.input_accessor_id);
        bone_animation.points = AnimationPointArrayAllocate(arena, tmp_accessor.count);

        for (I32 j = 0; j < bone_animation.points.capacity; j += 1)
        {
          AnimationPoint bone_animation_point = {0};
          bone_animation_point.type = AnimationPointType_Linear;

          // Translation
          {
            GLTFChannel channel = GLTFChannelArrayGet(&translation_channels, i);
            GLTFSampler sampler = GLTFSamplerListGetItem(&gltf_animation.samplers, channel.sampler_id);
            GLTFAccessor input_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, sampler.input_accessor_id);
            GLTFAccessor output_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, sampler.output_accessor_id);

            if (input_accessor.count != bone_animation.points.capacity)
            {
              LOG_DEBUG("Translation count (%i) != sample count (%i)\n", input_accessor.count, bone_animation.points.capacity);
            }

            bone_animation_point.linear.transform.translation = GetVec3F32FromGLTFAccessor(gltf_data, output_accessor, j);
            bone_animation_point.timestamp = (U64)(GetF32FromGLTFAccessor(gltf_data, input_accessor, j)*1000.0f);
            bone_animation.duration = Max(bone_animation.duration, bone_animation_point.timestamp);
          }
          // Rotation
          {
            GLTFChannel channel = GLTFChannelArrayGet(&rotation_channels, i);
            GLTFSampler sampler = GLTFSamplerListGetItem(&gltf_animation.samplers, channel.sampler_id);
            GLTFAccessor output_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, sampler.output_accessor_id);

            // --AlNov 10 December 2025: @TODO
            // There is a bug whene Blender optimize size of animation for gltf count.
            // For example, SimpleBoneAnimation.gltf has 60 samples for Translation,
            // 2 samples for Rotation and 2 samples for Scale (for root bone).
            // Blender removes samples for chennels that was not changed.
            // But we choose sample count for our animation based on the first chennel that we see in gltf_data.
            if (output_accessor.count != bone_animation.points.capacity)
            {
              LOG_DEBUG("Rotation count (%i) != sample count (%i)\n", output_accessor.count, bone_animation.points.capacity);
            }

            bone_animation_point.linear.transform.rotation = GetQuaternionFromGLTFAccessor(gltf_data, output_accessor, j);
          }
          // Scale
          {
            GLTFChannel channel = GLTFChannelArrayGet(&scale_channels, i);
            GLTFSampler sampler = GLTFSamplerListGetItem(&gltf_animation.samplers, channel.sampler_id);
            GLTFAccessor output_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, sampler.output_accessor_id);

            if (output_accessor.count != bone_animation.points.capacity)
            {
              LOG_DEBUG("Scale count (%i) != sample count (%i)\n", output_accessor.count, bone_animation.points.capacity);
            }

            bone_animation_point.linear.transform.scale = GetVec3F32FromGLTFAccessor(gltf_data, output_accessor, j);
          }

          AnimationPointArrayAdd(&bone_animation.points, bone_animation_point);
        }

        AnimationArrayAdd(&skeletal_animation.bone_animations, bone_animation);
      }
    }
    EndScratchArena(scratch);

    SkeletalAnimationArrayAdd(&result.skeletal_animations, skeletal_animation);
  }

  return result;
}
