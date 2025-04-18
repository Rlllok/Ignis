#pragma once

#include "mesh.h"

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

  GLTFAccessor index_accessor = GetListGLTFAccessorItem(&gltf_data.accessor_list, index_accessor_id);
  GLTFAccessor position_accessor = GetListGLTFAccessorItem(&gltf_data.accessor_list, position_accessor_id);

  GLTFBufferView index_buffer_view = GetListGLTFBufferViewItem(&gltf_data.buffer_view_list, index_accessor.buffer_view_id);
  GLTFBufferView position_buffer_view = GetListGLTFBufferViewItem(&gltf_data.buffer_view_list, position_accessor.buffer_view_id);
  
  result.index_data = decoded.data + index_buffer_view.byte_offset;
  result.index_size = index_buffer_view.byte_length / index_accessor.count;
  result.index_count = index_accessor.count;
  result.vertex_data = decoded.data + position_buffer_view.byte_offset;
  result.vertex_size = position_buffer_view.byte_length / position_accessor.count;
  result.vertex_count = position_accessor.count;

  return result;
}
