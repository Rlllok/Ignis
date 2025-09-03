#pragma once

#include "base/base_include.h"

typedef Str8 Buffer;

func Buffer AllocateBuffer(U64 size);
func void FreeBuffer(Buffer* buffer);
func void PrintBuffer(Buffer buffer);

func B32 BufferIsInBound(Buffer source, U64 position);
func B32 BufferIsDigit(Buffer source, U64 position);
func B32 BufferIsWhitespace(Buffer source, U64 position);
func B32 AreBuffersEqual(Buffer a, Buffer b);
func U64 FindPosition(Buffer buffer, U8 value);

typedef U16 GLTFTokenType;
enum GLTFTokenTypeEnum
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
  GLTF_TOKEN_TYPE_TRUE,
  GLTF_TOKEN_TYPE_FALSE,
  
  GLTF_TOKEN_TYPE_COUNT
} GLTFTokenTypeEnum;

typedef struct GLTFToken GLTFToken;
struct GLTFToken
{
  GLTFTokenType type;
  Buffer value;
};

typedef struct GLTFElement GLTFElement;
struct GLTFElement
{
  Buffer label;
  Buffer value;

  GLTFElement* first_sub_element;
  GLTFElement* next_sibling;
};

typedef struct GLTFReader GLTFReader;
struct GLTFReader
{
  Str8 file_path;
  B32 has_error;
  Buffer file_buffer;
  U64 position;
  U64 line_number;
  GLTFElement* element;
};

typedef U16 GLTFAttributeType;
enum GLTFAttributeTypeEnum
{
  GLTF_ATTRIBUTE_TYPE_NONE,
  
  GLTF_ATTRIBUTE_TYPE_POSITION,
  GLTF_ATTRIBUTE_TYPE_NORMAL,
  GLTF_ATTRIBUTE_TYPE_TEXCOORD_0,

  GLTF_ATTRIBUTE_TYPE_COUNT
} GLTFAttributeTypeEnum;

typedef struct GLTFAttribute GLTFAttribute;
struct GLTFAttribute
{
  U32 accessor_id;
  GLTFAttributeType type;

  GLTFAttribute* next;
};
DefineList(GLTFAttribute, GLTFAttributeList)

typedef U8 GLTFAccessorType;
enum GLTFAccessorTypeEnum
{
  GLTF_ACCESSOR_TYPE_NONE,

  GLTF_ACCESSOR_TYPE_SCALAR,
  GLTF_ACCESSOR_TYPE_VEC3,

  GLTF_ACCSSOR_TYPE_COUNT
} GLTFAccessorTypeEnum;

typedef struct GLTFAccessor GLTFAccessor;
struct GLTFAccessor
{
  U32 buffer_view_id;
  U64 byte_offset;
  U64 count;
  GLTFAccessorType type;
  
  GLTFAccessor* next;
};
DefineList(GLTFAccessor, GLTFAccessorList)

typedef struct GLTFPrimitive GLTFPrimitive;
struct GLTFPrimitive
{
  U32 indices_accessor_id;
  U32 position_accessor_id;
  U32 normal_accessor_id;
  U32 texcoord_accessor_id;

  GLTFPrimitive* next;
};
DefineList(GLTFPrimitive, GLTFPrimitiveList)

typedef struct GLTFMesh GLTFMesh;
struct GLTFMesh
{
  GLTFPrimitiveList primitive_list;
};
DefineList(GLTFMesh, GLTFMeshList)

typedef struct GLTFBuffer GLTFBuffer;
struct GLTFBuffer
{
  Str8 uri;
  Buffer buffer;

  GLTFBuffer* next;
};
DefineList(GLTFBuffer, GLTFBufferList)

typedef struct GLTFBufferView GLTFBufferView;
struct GLTFBufferView
{
  U32 buffer_id;
  U32 byte_offset;
  U32 byte_length;
  U32 target;

  GLTFBufferView* next;
};
DefineList(GLTFBufferView, GLTFBufferViewList)

typedef struct GLTFData GLTFData;
struct GLTFData
{
  U32 default_scene_id;
  GLTFBufferList buffer_list;
  GLTFMeshList mesh_list;
  GLTFBufferViewList buffer_view_list;
  GLTFAccessorList accessor_list;
};

func Buffer ReadGLTFFile(Buffer file_name);
func void GLTFError(GLTFReader* reader, GLTFToken token, const char* message);
func GLTFToken GetGLTFToken(GLTFReader* reader);
func GLTFElement* ParseList(GLTFReader* reader, GLTFToken start_token, GLTFTokenType end_type, B32 has_labels);
func GLTFElement* ParseElement(GLTFReader* reader, Buffer label, GLTFToken token);
func GLTFData GetGLTFData(GLTFReader* reader);
func GLTFElement* LookUpElement(GLTFElement* object, Buffer label);
func Buffer GetDataFromGLTFBuffer(GLTFBuffer gltf_buffer);

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
  Buffer result = AllocateBuffer(in.length * 3 / 4);
  U32 result_position = 0;

  for (I32 i = 0; i < in.length; i += 1)
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

    for (I32 i = 0; i < source.length; i += 1)
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
    LOG_ERROR("There is no element \"%.*s\"\n", (U32)label.length, label.data);
  }

  return result;
}
