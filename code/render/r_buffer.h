#pragma once

#include "base/base_include.h"

// --AlNov: @TODO Doing bit flags for the first time.
//          Maybe there is a better way to implement/name/etc.
enum BufferUsageFlagBits
{
  BUFFER_USAGE_FLAG_ZERO = 0,

  BUFFER_USAGE_FLAG_VERTEX  = 1 << 0,
  BUFFER_USAGE_FLAG_INDEX   = 1 << 1,
  BUFFER_USAGE_FLAG_UNIFORM = 1 << 2,

  BUFFER_USAGE_FLAG_COUNT
};
typedef U64 BufferUsageFlags;

enum BufferPropertyFlagBits
{
  BUFFER_PROPERTY_ZERO = 0,

  BUFFER_PROPERTY_HOST_COHERENT = 1 << 0,
  BUFFER_PROPERTY_HOST_VISIBLE = 1 << 1,
};
typedef U64 BufferPropertyFlags;

struct R_Buffer
{
  U64 handle;
  
  U64 size;
};

func R_Buffer R_CreateBuffer(U64 size, BufferUsageFlags usage_type, BufferPropertyFlags flags);
func void R_FillBuffer(R_Buffer* buffer, U8* data, U64 size, U64 offset);

struct R_VertexBuffer
{
  R_Buffer buffer;

  void* vertecies;
  U64   vertex_size;
  U64   vertex_count;
};

func R_VertexBuffer R_CreateVertexBuffer(void* vertecies, U64 vertex_size, U64 vertex_count);

struct R_IndexBuffer
{
  R_Buffer buffer;

  void* indecies;
  U64   index_size;
  U64   index_count;
};

func R_IndexBuffer R_CreateIndexBuffer(void* indecies, U64 vertex_size, U64 vertex_count);
