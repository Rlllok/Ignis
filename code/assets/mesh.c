#pragma once

#include "mesh.h"

#include "gltf.c"

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

    U64 vertex_size = 2 * sizeof(Vec3F32) + sizeof(Vec2F32);
    U8* vertex_data = (U8*)OS_AllocateMemory(vertex_size * position_accessor.count);
    U8* current_data = vertex_data;
    for (U64 i = 0; i < position_accessor.count; i += 1)
    {
      Vec3F32* position = (Vec3F32*)(data_buffer.data + position_buffer_view.byte_offset) + i;
      Vec3F32* normal = (Vec3F32*)(data_buffer.data + normal_buffer_view.byte_offset) + i;
      Vec2F32* texcoord = (Vec2F32*)(data_buffer.data + texcoord_buffer_view.byte_offset) + i;

      Vec3F32* data_position = (Vec3F32*)(current_data + 0);
      Vec3F32* data_normal = (Vec3F32*)(current_data + sizeof(Vec3F32));
      Vec2F32* data_texcoord = (Vec2F32*)(current_data + sizeof(Vec3F32) + sizeof(Vec3F32));
      
      *data_position = *position;
      *data_normal = *normal;
      *data_texcoord = *texcoord;
      
      current_data += vertex_size;
    }
  
    geometry.vertex_data = vertex_data;
    geometry.vertex_size = vertex_size;
    geometry.vertex_count = position_accessor.count;

    AST_GeometryListPush(&result.geometry_list, geometry);
  }

  return result;
}
