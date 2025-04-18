#pragma once

#include "mesh.h"

#include "base/base_math.h"
#include "gltf.cpp"

func AST_Geometry
AST_LoadGeometryFromGLTF(const char* gltf_name)
{
  AST_Geometry result = {};
  
  GLTFReader gltf_reader = {};
  gltf_reader.file_buffer = ReadFile(ConstString("data/monkey_gltf/monkey.gltf"));

  GLTFData gltf_data = GetGLTFData(&gltf_reader);
  
  Buffer decoded = GetDataFromGLTFBuffer(gltf_data.buffer_list.first->data);  
  
  U64 index_accessor_id = gltf_data.mesh_list.first->data.primitive_list.first->data.indices_accessor_id;
  U64 position_accessor_id = 0; //gltf_data.mesh_list.first->data.primitive_list.first->data.attributes.accessor_id;
  U64 normal_accessor_id = 1;

  GLTFAccessor index_accessor = GetListGLTFAccessorItem(&gltf_data.accessor_list, index_accessor_id);
  GLTFAccessor position_accessor = GetListGLTFAccessorItem(&gltf_data.accessor_list, position_accessor_id);
  GLTFAccessor normal_accessor = GetListGLTFAccessorItem(&gltf_data.accessor_list, normal_accessor_id);

  GLTFBufferView index_buffer_view = GetListGLTFBufferViewItem(&gltf_data.buffer_view_list, index_accessor.buffer_view_id);
  GLTFBufferView position_buffer_view = GetListGLTFBufferViewItem(&gltf_data.buffer_view_list, position_accessor.buffer_view_id);
  GLTFBufferView normal_buffer_view = GetListGLTFBufferViewItem(&gltf_data.buffer_view_list, normal_accessor.buffer_view_id);
  
  result.index_data = decoded.data + index_buffer_view.byte_offset;
  result.index_size = index_buffer_view.byte_length / index_accessor.count;
  result.index_count = index_accessor.count;

  U64 vertex_size = 2 * sizeof(Vec3f);
  U8* vertex_data = (U8*)OS_AllocateMemory(vertex_size * position_accessor.count);
  for (U64 i = 0; i < position_accessor.count; i += 1)
  {
    Vec3f* position = (Vec3f*)(decoded.data + position_buffer_view.byte_offset) + i;
    Vec3f* normal = (Vec3f*)(decoded.data + normal_buffer_view.byte_offset) + i;

    Vec3f* data_position = (Vec3f*)(vertex_data) + i*2 + 0;
    Vec3f* data_normal = (Vec3f*)(vertex_data) + i*2 + 1;

    *data_position = *position;
    *data_normal = *normal;
  }
  
  result.vertex_data = vertex_data;
  result.vertex_size = vertex_size;
  result.vertex_count = position_accessor.count;

  return result;
}
