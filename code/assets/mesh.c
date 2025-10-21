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

  for (GLTFNodeListNode* gltf_node_iter = gltf_data.scene.nodes.first;
       gltf_node_iter;
       gltf_node_iter = gltf_node_iter->next)
  {
    AST_Geometry geometry = {0};
    GLTFNode gltf_node = gltf_node_iter->data;

    GLTFMesh gltf_mesh = GLTFMeshListGetItem(&gltf_data.meshes, gltf_node.mesh_id);
    GLTFPrimitive gltf_primitive = gltf_mesh.primitives.first->data;

    GLTFAccessor index_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, gltf_primitive.indecies_accessor_id);
    GLTFAccessor position_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, gltf_primitive.position_accessor_id);
    GLTFAccessor normal_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, gltf_primitive.normal_accessor_id);
    GLTFAccessor texcoord_accessor = GLTFAccessorListGetItem(&gltf_data.accessors, gltf_primitive.texcoord_accessor_id);

    GLTFBufferView index_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_views, index_accessor.buffer_view_id);
    GLTFBufferView position_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_views, position_accessor.buffer_view_id);
    GLTFBufferView normal_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_views, normal_accessor.buffer_view_id);
    GLTFBufferView texcoord_buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_views, texcoord_accessor.buffer_view_id);

    GLTFBuffer index_buffer = GLTFBufferListGetItem(&gltf_data.buffers, index_buffer_view.buffer_id);
    geometry.index_data = index_buffer.data + index_accessor.byte_offset + index_buffer_view.byte_offset;
    geometry.index_size = index_buffer_view.byte_length / index_accessor.count;
    geometry.index_count = index_accessor.count;

    GLTFBuffer position_buffer = GLTFBufferListGetItem(&gltf_data.buffers, position_buffer_view.buffer_id);
    GLTFBuffer normal_buffer = GLTFBufferListGetItem(&gltf_data.buffers, normal_buffer_view.buffer_id);
    GLTFBuffer texcoord_buffer = GLTFBufferListGetItem(&gltf_data.buffers, texcoord_buffer_view.buffer_id);

    geometry.vertecies = (AST_Vertex*)PushArena(arena, position_accessor.count*sizeof(AST_Vertex));
    geometry.vertecies_count = position_accessor.count;
    for (U32 i = 0; i < position_accessor.count; i += 1)
    {
      AST_Vertex* vertex = geometry.vertecies + i;
      vertex->position = *((Vec3F32*)(position_buffer.data + position_accessor.byte_offset + position_buffer_view.byte_offset) + i);
      vertex->normal = *((Vec3F32*)(normal_buffer.data + normal_accessor.byte_offset + normal_buffer_view.byte_offset) + i);
      vertex->uv = *((Vec2F32*)(texcoord_buffer.data + texcoord_accessor.byte_offset + texcoord_buffer_view.byte_offset) + i);
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

  return result;
}
