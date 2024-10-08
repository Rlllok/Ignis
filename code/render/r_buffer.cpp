#include "r_buffer.h"
#include "vulkan/r_vk_buffer.h"

func void
R_FillBuffer(R_Buffer* buffer, U8* data, U64 size, U64 offset)
{
  _VK_FillBuffer(buffer, data, size, offset);
}

func R_VertexBuffer
R_CreateVertexBuffer(void* vertecies, U64 vertex_size, U64 vertex_count)
{
  R_VertexBuffer result = {};

  result.buffer = Renderer.CreateBuffer(
    vertex_size*vertex_count,
    BUFFER_USAGE_FLAG_VERTEX,
    BUFFER_PROPERTY_HOST_COHERENT | BUFFER_PROPERTY_HOST_VISIBLE);
  result.vertecies = vertecies;
  result.vertex_size = vertex_size;
  result.vertex_count = vertex_count;

  R_FillBuffer(&result.buffer, (U8*)vertecies, vertex_size*vertex_count, 0);

  return result;
}

func R_IndexBuffer
R_CreateIndexBuffer(void* indecies, U64 index_size, U64 index_count)
{
  R_IndexBuffer result = {};
  result.buffer = Renderer.CreateBuffer(
    index_size*index_count,
    BUFFER_USAGE_FLAG_INDEX,
    BUFFER_PROPERTY_HOST_COHERENT | BUFFER_PROPERTY_HOST_VISIBLE);
  result.indecies = indecies;
  result.index_size = index_size;
  result.index_count = index_count;

  R_FillBuffer(&result.buffer, (U8*)indecies, index_size*index_count, 0);

  return result;
}
