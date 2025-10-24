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
  Str8 label;
  Str8 value;

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

// -- NEW ------------------------------------------------------------
typedef U32 GLTF_ID;
#define GLTF_ID_NIL U32_MAX

typedef struct GLTFPrimitive GLTFPrimitive;
struct GLTFPrimitive
{
  GLTF_ID indecies_accessor_id;
  GLTF_ID material_accessor_id;

  // GLTF Attributes
  GLTF_ID position_accessor_id;
  GLTF_ID tangent_accessor_id;
  GLTF_ID normal_accessor_id;
  GLTF_ID texcoord_accessor_id;
};
DefineList(GLTFPrimitive, GLTFPrimitiveList)

typedef struct GLTFMesh GLTFMesh;
struct GLTFMesh
{
  GLTFPrimitiveList primitives;
};
DefineList(GLTFMesh, GLTFMeshList)

typedef struct GLTFNode GLTFNode;
struct GLTFNode
{
  GLTF_ID mesh_id;
  Vec3F32 translation;
};
DefineList(GLTFNode, GLTFNodeList)

typedef struct GLTFScene GLTFScene;
struct GLTFScene
{
  GLTFNodeList nodes;
  U32 nodes_count;
};

typedef struct GLTFBuffer GLTFBuffer;
struct GLTFBuffer
{
  Str8 uri;
  U64 byte_length;
  U8* data;
};
DefineList(GLTFBuffer, GLTFBufferList)

typedef struct GLTFBufferView GLTFBufferView;
struct GLTFBufferView
{
  GLTF_ID buffer_id;
  U32 byte_offset;
  U32 byte_length;
  U32 byte_stride;
  U32 target; // @NOTE: Not used
};
DefineList(GLTFBufferView, GLTFBufferViewList)

typedef U16 GLTFComponentType;
typedef enum GLTFComponentTypeEnum
{
  GLTFComponentType_None = 0,
  GLTFComponentType_Byte = 5120,
  GLTFComponentType_UnsignedByte = 5121,
  GLTFComponentType_Short = 5122,
  GLTFComponentType_UnsignedShort = 5123,
  GLTFComponentType_UnsignedInt = 5125,
  GLTFComponentType_Float = 5126,
} GLTFComponentTypeEnum;

typedef U8 GLTFAccessorType;
typedef enum GLTFAccessorTypeEnum
{
  GLTFAccessorType_None,
  GLTFAccessorType_Scalar,
  GLTFAccessorType_Vec2,
  GLTFAccessorType_Vec3,
  GLTFAccessorType_Vec4,
  GLTFAccessorType_Mat2,
  GLTFAccessorType_Mat3,
  GLTFAccessorType_Mat4,
} GLTFAccessorTypeEnum;

typedef struct GLTFAccessor GLTFAccessor;
struct GLTFAccessor
{
  GLTF_ID buffer_view_id;
  U32 byte_offset;
  GLTFAccessorType type;
  GLTFComponentType component_type;
  B32 normalized;
  U32 count;
};
DefineList(GLTFAccessor, GLTFAccessorList)

typedef struct GLTFAsset GLTFAsset;
struct GLTFAsset
{
  U32 version;
};

typedef struct GLTFData GLTFData;
struct GLTFData
{
  GLTFAsset asset;
  GLTFScene scene;
  GLTFMeshList meshes;
  GLTFBufferList buffers;
  GLTFBufferViewList buffer_views;
  GLTFAccessorList accessors;
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

func Str8
Base64Decode(Arena* arena, Str8 in)
{
  U8 count = 0;
  U8 buffer[4];
  Str8 result = AllocateStr8(arena, in.length * 3 / 4);
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
func F64 _GetNumberElementFunc(GLTFElement* element, Buffer label, F64 default_value)
{
  F64 result = default_value;

  GLTFElement* number = LookUpElement(element, label);
  if (number)
  {
    result = F64FromStr8(number->value);
  }
  else
  {
    LOG_ERROR("There is no element \"%.*s\"\n", (U32)label.length, label.data);
  }

  return result;
}
#define GetNumberElement(element, label) _GetNumberElementFunc(element, label, 0.0)
#define GetNumberElementWithDefault(element, label, default_value) _GetNumberElementFunc(element, label, default_value)

func GLTF_ID
GetIDElement(GLTFElement* element, Buffer label)
{
  GLTF_ID result = GLTF_ID_NIL;

  GLTFElement* id = LookUpElement(element, label);
  if (id)
  {
    result = (GLTF_ID)F64FromStr8(id->value);
  }
  else
  {
    LOG_ERROR("There is no element \"%.*s\"\n", (U32)label.length, label.data);
  }

  return result;
}

func Vec3F32
GetVec3F32Element(GLTFElement* element, Buffer label)
{
  Vec3F32 result = {0};

  GLTFElement* vector = LookUpElement(element, label);
  if (vector)
  {
    GLTFElement* x_element = vector->first_sub_element;
    result.x = (F32)F64FromStr8(x_element->value);
    GLTFElement* y_element = x_element->next_sibling;
    result.y = (F32)F64FromStr8(y_element->value);
    GLTFElement* z_element = y_element->next_sibling;
    result.z = (F32)F64FromStr8(z_element->value);
  }
  else
  {
    LOG_ERROR("There is no element \"%.*s\"\n", (U32)label.length, label.data);
  }

  return result;
}
