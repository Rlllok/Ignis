#pragma once

#include "mesh.h"

#include "gltf.c"

#if 0
func AST_Geometry
AST_LoadGeometryFromGLTF(Arena* arena, Str8 gltf_name)
{
  AST_Geometry result = {0};
  
  GLTFReader gltf_reader = {0};
  gltf_reader.file_path = gltf_name;
  gltf_reader.file_buffer = GLTFReadFile(gltf_name);

  GLTFData gltf_data = GetGLTFData(&gltf_reader);
    
  U64 index_accessor_id = gltf_data.mesh_list.first->data.primitive_list.first->data.indices_accessor_id;
  U64 position_accessor_id = gltf_data.mesh_list.first->data.primitive_list.first->data.position_accessor_id;
  U64 normal_accessor_id = gltf_data.mesh_list.first->data.primitive_list.first->data.normal_accessor_id;

  GLTFAccessor index_accessor = GLTFAccessorListGetItem(&gltf_data.accessor_list, index_accessor_id);
  GLTFAccessor position_accessor = GLTFAccessorListGetItem(&gltf_data.accessor_list, position_accessor_id);
  GLTFAccessor normal_accessor = GLTFAccessorListGetItem(&gltf_data.accessor_list, normal_accessor_id);

  GLTFBufferView index_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_view_list, index_accessor.buffer_view_id);
  GLTFBufferView position_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_view_list, position_accessor.buffer_view_id);
  GLTFBufferView normal_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_view_list, normal_accessor.buffer_view_id);
  
  Buffer data_buffer = gltf_data.buffer_list.first->data.buffer;
  result.index_data = data_buffer.data + index_buffer_view.byte_offset;
  result.index_size = index_buffer_view.byte_length / index_accessor.count;
  result.index_count = index_accessor.count;

  U64 vertex_size = 2 * sizeof(Vec3F32);
  U8* vertex_data = (U8*)OS_AllocateMemory(vertex_size * position_accessor.count);
  for (U64 i = 0; i < position_accessor.count; i += 1)
  {
    Vec3F32* position = (Vec3F32*)(data_buffer.data + position_buffer_view.byte_offset) + i;
    Vec3F32* normal = (Vec3F32*)(data_buffer.data + normal_buffer_view.byte_offset) + i;

    Vec3F32* data_position = (Vec3F32*)(vertex_data) + i*2 + 0;
    Vec3F32* data_normal = (Vec3F32*)(vertex_data) + i*2 + 1;

    *data_position = *position;
    *data_normal = *normal;
  }
  
  result.vertex_data = vertex_data;
  result.vertex_size = vertex_size;
  result.vertex_count = position_accessor.count;

  return result;
}
#endif

func AST_StaticMesh
AST_LoadStaticMeshFromGLTF(Arena* arena, Str8 gltf_name)
{
  AST_StaticMesh result = {0};
  result.geometry_list = AST_GeometryListCreate(arena);
  
  GLTFReader gltf_reader = {0};
  gltf_reader.file_path = gltf_name;
  gltf_reader.file_buffer = GLTFReadFile(gltf_name);

  GLTFData gltf_data = GetGLTFData(&gltf_reader);
    
  for (GLTFMeshListNode* mesh_node = gltf_data.mesh_list.first;
       mesh_node;
       mesh_node = mesh_node->next)
  {
    AST_Geometry geometry = {0};
    
    U64 index_accessor_id = mesh_node->data.primitive_list.first->data.indices_accessor_id;
    U64 position_accessor_id = mesh_node->data.primitive_list.first->data.position_accessor_id;
    U64 normal_accessor_id = mesh_node->data.primitive_list.first->data.normal_accessor_id;
    U64 texcoord_accessor_id = mesh_node->data.primitive_list.first->data.texcoord_accessor_id;

    GLTFAccessor index_accessor = GLTFAccessorListGetItem(&gltf_data.accessor_list, index_accessor_id);
    GLTFAccessor position_accessor = GLTFAccessorListGetItem(&gltf_data.accessor_list, position_accessor_id);
    GLTFAccessor normal_accessor = GLTFAccessorListGetItem(&gltf_data.accessor_list, normal_accessor_id);
    GLTFAccessor texcoord_accessor = GLTFAccessorListGetItem(&gltf_data.accessor_list, texcoord_accessor_id);

    GLTFBufferView index_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_view_list, index_accessor.buffer_view_id);
    GLTFBufferView position_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_view_list, position_accessor.buffer_view_id);
    GLTFBufferView normal_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_view_list, normal_accessor.buffer_view_id);
    GLTFBufferView texcoord_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_view_list, texcoord_accessor.buffer_view_id);
  
    Buffer data_buffer = gltf_data.buffer_list.first->data.buffer;
    geometry.index_data = data_buffer.data + index_buffer_view.byte_offset;
    geometry.index_size = index_buffer_view.byte_length / index_accessor.count;
    geometry.index_count = index_accessor.count;

    geometry.vertecies = (AST_Vertex*)PushArena(arena, position_accessor.count*sizeof(AST_Vertex));
    for (U32 i = 0; i < position_accessor.count; i += 1)
    {
      AST_Vertex* vertex = geometry.vertecies + i;
      vertex->position = *((Vec3F32*)(data_buffer.data + position_buffer_view.byte_offset) + i);
      vertex->normal = *((Vec3F32*)(data_buffer.data + normal_buffer_view.byte_offset) + i);
      vertex->uv = *((Vec2F32*)(data_buffer.data + texcoord_buffer_view.byte_offset) + i);
    }
  
    geometry.vertecies_count = position_accessor.count;

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

  return result;
}
