#pragma once

#include "base/base_include.h"

#include "base/base_include.cpp"

// @TODO Remove Buffer from here to Base layer. The truth is that Buffer is String
struct Buffer
{
  U8* data;
  U64 size;
};

#define ConstString(String) {(U8*)(String), sizeof(String) - 1}

func Buffer AllocateBuffer(U64 size);
func void FreeBuffer(Buffer* buffer);
func void PrintBuffer(Buffer buffer);

func B32 BufferIsInBound(Buffer source, U64 position);
func B32 BufferIsDigit(Buffer source, U64 position);
func B32 BufferIsWhitespace(Buffer source, U64 position);
func B32 AreBuffersEqual(Buffer a, Buffer b);
func U64 FindPosition(Buffer buffer, U8 value);

enum GLTFTokenType
{
  GLTF_TOKEN_TYPE_ERROR,
  
  GLTF_TOKEN_TYPE_COLON,
  GLTF_TOKEN_TYPE_COMMA,
  GLTF_TOKEN_TYPE_OPEN_BRACE,
  GLTF_TOKEN_TYPE_CLOSE_BRACE,
  GLTF_TOKEN_TYPE_OPEN_BRACKET,
  GLTF_TOKEN_TYPE_CLOSE_BRACKET,
  GLTF_TOKEN_TYPE_STRING,
  GLTF_TOKEN_TYPE_NUMBER,
  
  GLTF_TOKEN_TYPE_COUNT
};

struct GLTFToken
{
  enum GLTFTokenType type;
  Buffer value;
};

struct GLTFElement
{
  Buffer label;
  Buffer value;

  GLTFElement* first_sub_element;
  GLTFElement* next_sibling;
};

struct GLTFReader
{
  B32 has_error;
  Buffer file_buffer;
  U64 position;
  U64 line_number;
  GLTFElement* element;
};

struct GLTFBufferView
{
  U32 buffer_id;
  U32 byte_offset;
  U32 byte_length;
  U32 target;

  GLTFBufferView* next;
};

enum GLTFAttributeType
{
  GLTF_ATTRIBUTE_TYPE_NONE,
  
  GLTF_ATTRIBUTE_TYPE_POSITION,

  GLTF_ATTRIBUTE_TYPE_COUNT
};

struct GLTFAttribute
{
  U32 accessor_id;
  GLTFAttributeType type;

  GLTFAttribute* next;
};

enum GLTFAccessorType
{
  GLTF_ACCESSOR_TYPE_NONE,

  GLTF_ACCESSOR_TYPE_SCALAR,
  GLTF_ACCESSOR_TYPE_VEC3,

  GLTF_ACCSSOR_TYPE_COUNT
};

struct GLTFAccessor
{
  U32 buffer_view_id;
  U64 byte_offset;
  U64 count;
  GLTFAccessorType type;
  
  GLTFAccessor* next;
};

struct GLTFPrimitive
{
  U32 indices_accessor_id;
  GLTFAttribute attributes;

  GLTFPrimitive* next;
};

struct GLTFMesh
{
  List primitive_list;
};

struct GLTFBuffer
{
  Buffer uri;
  U64 byte_length;

  GLTFBuffer* next;
};

struct GLTFData
{
  U32 default_scene_id;
  List buffer_list;
  List mesh_list;
  List buffer_view_list;
  List accessor_list;
};

func Buffer ReadGLTFFile(char const* file_name);
func void GLTFError(GLTFReader* reader, GLTFToken token, const char* message);
func GLTFToken GetGLTFToken(GLTFReader* reader);
func GLTFElement* ParseList(GLTFReader* reader, GLTFToken start_token, enum GLTFTokenType end_type);
func GLTFElement* ParseElement(GLTFReader* reader, Buffer label, GLTFToken token);
func GLTFData GetGLTFData(GLTFReader* reader);
func GLTFElement* LookUpElement(GLTFElement* object, Buffer label);

// Base64 Decoder
U8 base64_map[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                   'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                   'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                   'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

func Buffer
Base64Decode(Buffer in)
{
  U8 count = 0;
  U8 buffer[4];
  Buffer result = AllocateBuffer(in.size * 3 / 4);
  U32 result_position = 0;

  for (I32 i = 0; i < in.size; i += 1)
  {
    I32 map_index = 0;
    while ((map_index < 64) && (base64_map[map_index] != in.data[i])) map_index += 1;
    buffer[count] = map_index;
    count += 1;

    if (count == 4)
    {
      result.data[result_position] = (buffer[0] << 2) + (buffer[1] >> 4);
      result_position += 1;
      if (buffer[2] != 64)
      {
        result.data[result_position] = (buffer[1] << 4) + (buffer[2] >> 2);
        result_position += 1;
      }
      if (buffer[3] != 64)
      {
        result.data[result_position] = (buffer[2] << 6) + buffer[3];
        result_position += 1;
      }

      count = 0;
    }
  }

  return result;
}


// @TODO Move
func F64 GetNumberElement(GLTFElement* element, Buffer label)
{
  F64 result = 0;

  GLTFElement* number = LookUpElement(element, label);
  if (number)
  {
    Buffer source = number->value;

    for (I32 i = 0; i < source.size; i += 1)
    {
      U8 number_char = source.data[i] - (U8)'0';

      if (number_char < 10)
      {
        result = 10.0*result + (F64)number_char;
      }
      else
      {
        break;
      }
    }
  }
  else
  {
    LOG_ERROR("There is no element \"%.*s\"", (U32)label.size, label.data);
  }

  return result;
}
