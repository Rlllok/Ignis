#pragma once

#include "mesh.h"

#include "gltf.cpp"

func AST_Geometry
AST_LoadGeometryFromGLTF(const char* gltf_name)
{
  AST_Geometry result = {};
  
  GLTFReader gltf_reader = {};
  gltf_reader.file_buffer = ReadFile(gltf_name);
  //ParseGLTF(&gltf_reader);

  GLTFElement* buffers_list_element = LookUpElement(gltf_reader.element, ConstString("buffers"));
  GLTFElement* buffer_element = buffers_list_element->first_sub_element->first_sub_element;
  Buffer mesh_buffer = buffer_element->value;
  
  U64 comma_position = FindPosition(mesh_buffer, ',');
  mesh_buffer.data = mesh_buffer.data + comma_position + 1;
  mesh_buffer.size = mesh_buffer.size - comma_position - 1;
  Buffer decoded = Base64Decode(mesh_buffer);  
  
  result.index_data = decoded.data;
  result.index_size = sizeof(U16);
  result.index_count = 3;
  result.vertex_data = decoded.data + 8;
  result.vertex_size = sizeof(Vec3f);
  result.vertex_count = 3;

  return result;
}
