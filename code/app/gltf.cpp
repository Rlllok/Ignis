#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"

#include "os/os_memory.h"
#include "sys/stat.h"

struct Buffer
{
  U64 size;
  U8* data;
};

#define ConstString(String) {sizeof(String) - 1, (U8*)(String)}

func Buffer AllocateBuffer(U64 size);
func void FreeBuffer(Buffer* buffer);
func void PrintBuffer(Buffer buffer);

func Buffer ReadFile(char const* file_name);

enum TokenType
{
  TOKEN_TYPE_ERROR,
  
  TOKEN_TYPE_COLON,
  TOKEN_TYPE_COMMA,
  TOKEN_TYPE_OPEN_BRACE,
  TOKEN_TYPE_CLOSE_BRACE,
  TOKEN_TYPE_OPEN_BRACKET,
  TOKEN_TYPE_CLOSE_BRACKET,
  TOKEN_TYPE_STRING,
  TOKEN_TYPE_NUMBER,
  
  TOKEN_TYPE_COUNT
};

struct Token
{
  enum TokenType type;
  Buffer data;
};

struct Element
{
  Buffer label;
  Buffer value;

  Element* first_sub_element;
  Element* next_sibling;
};

func B32 IsInBound(Buffer source, U64 position);
func B32 IsDigit(Buffer source, U64 position);
func B32 IsWhitespace(Buffer source, U64 position);
func B32 AreBuffersEqual(Buffer a, Buffer b);

struct GLTFNode
{
  U32 mesh_id;
};

struct GLTFScene
{
  U32 node_id;
};

struct GLTFMesh
{
  
};

struct GLTFBuffer
{
  U32 byte_length;
  U8* data;
};

struct GLTFBufferView
{
  U32 buffer_id;
  U32 offset;
  U32 byte_length;
  U32 target;
};

enum GLTFType
{
  GLTF_TYPE_NONE,
  
  GLTF_TYPE_SCALAR,
  GLTF_TYPE_VEC3,

  GLTF_TYPE_COUNT
};

struct GLTFAccessor
{
  U32 buffer_view_id;
  U32 byte_offset;
  GLTFType type;
  U32 count;
};

struct GLTFData
{
  
};

struct GLTFReader
{
  B32 has_error;
  Buffer file_buffer;
  U64 position;
  U64 line_number;
  Element* element;
};

func void GLTFError(GLTFReader* reader, Token token, const char* message);
func Token GetGLTFToken(GLTFReader* reader);
func Element* ParseList(GLTFReader* reader, Token start_token, enum TokenType end_type);
func Element* ParseElement(GLTFReader* reader, Buffer label, Token token);
func B32 ParseGLTF(GLTFReader* reader);
func Element* LookUpElement(Element* object, Buffer label);

func F64 GetNumberElement(Element* element, Buffer label)
{
  F64 result = 0;

  Element* number = LookUpElement(element, label);
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

I32 main()
{
  LOG_INFO("GLTF Test message!\n");

  GLTFReader gltf_reader = {};
  gltf_reader.file_buffer = ReadFile("data/box_gltf/test.gltf");

  ParseGLTF(&gltf_reader);

#if 0
  for (Element* element = gltf_reader.element.next_element; element; element = element->next_element)
  {
    PrintBuffer(element->label);
    printf(" : ");
    PrintBuffer(element->value);
    printf("\n");
  }
#endif 

  Element* gltf_buffer_views = LookUpElement(gltf_reader.element, ConstString("bufferViews"));
  Element* gltf_buffer_view = gltf_buffer_views->first_sub_element;
  GLTFBufferView buffer_view = {};
  buffer_view.buffer_id = GetNumberElement(gltf_buffer_view, ConstString("buffer"));
  buffer_view.offset = GetNumberElement(gltf_buffer_view, ConstString("byteOffset"));
  buffer_view.byte_length = GetNumberElement(gltf_buffer_view, ConstString("byteLength"));
  buffer_view.target = GetNumberElement(gltf_buffer_view, ConstString("target"));

  Element* first = LookUpElement(gltf_reader.element, ConstString("bufferViews"));
  if (first)
  {
    PrintBuffer(first->label);
    printf(" : ");
    PrintBuffer(first->value);

    for (Element* element = first->first_sub_element; element; element = element->next_sibling)
    {
      Element* second = LookUpElement(element, ConstString("buffer"));
      PrintBuffer(second->label);
      printf(" : ");
      PrintBuffer(second->value);
      printf("\n");
    }
  }
  
  return 0;
}

// Buffer
func Buffer
AllocateBuffer(U64 size)
{
  Buffer result = {};

  result.data = (U8*)OS_AllocateMemory(size * sizeof(U8));
  if (result.data)
  {
    result.size = size;
  }

  return result;
}

func void
FreeBuffer(Buffer* buffer)
{
  if (buffer->data)
  {
    OS_FreeMemory(buffer->data);
  }

  *buffer = {};
}

func void
PrintBuffer(Buffer buffer)
{
  if (buffer.data)
  {
    fwrite(buffer.data, sizeof(buffer.data[0]), buffer.size, stdout);
  }
}

func Buffer
ReadFile(char const* file_name)
{
  Buffer result = {};

  FILE* file = fopen(file_name, "rb");
  if (file)
  {
    struct __stat64 stat;
    __stat64(file_name, &stat);

    result = AllocateBuffer(stat.st_size);
    if (result.data)
    {
      if (fread(result.data, result.size, 1, file) != 1)
      {
        LOG_ERROR("ERROR: Unable to read \"%s\".\n", file_name);
        FreeBuffer(&result);
      }
    }

    fclose(file);
  }
  else
  {
    LOG_ERROR("ERROR: Unable to open \"%s\"", file_name);
  }

  return result;
}

// Check
func B32
IsInBound(Buffer source, U64 position)
{
  B32 result = position < source.size;

  return result;
}

func B32
IsDigit(Buffer source, U64 position)
{
  B32 result = false;

  if (IsInBound(source, position))
  {
    U8 value = source.data[position];

    result = ((value >= '0') && (value <= '9'));
  }

  return result;
}

func B32
IsWhitespace(Buffer source, U64 position)
{
  B32 result = false;

  if (IsInBound(source, position))
  {
    U8 value = source.data[position];

    result = ((value == ' ') || (value == '\t') || (value == '\n') || (value == '\r'));
  }

  return result;
}

func B32
AreBuffersEqual(Buffer a, Buffer b)
{
  if (a.size != b.size)
  {
    return false;
  }

  for (U64 i = 0; i < a.size; i += 1)
  {
    if (a.data[i] != b.data[i])
    {
      return false;
    }
  }

  return true;
}

func void
GLTFError(GLTFReader* reader, Token token, const char* message)
{
  reader->has_error = true;

  LOG_ERROR("- LINE %d - \"%.*s\" - %s\n", reader->line_number + 1, (U32)token.data.size, (char*)token.data.data, message);
}

func Token
GetGLTFToken(GLTFReader* reader)
{
  Token result = {};

  Buffer source = reader->file_buffer;
  U64 position = reader->position;
  
  while (IsWhitespace(source, position))
  {
    position += 1;
    if (source.data[position] == '\n')
    {
      reader->line_number += 1;
    }
  }

  if (IsInBound(source, position))
  {
    result.type = TOKEN_TYPE_ERROR;
    result.data.size = 1;
    result.data.data = source.data + position;

    U8 value = source.data[position];
    position += 1;
    
    switch (value)
    {
      case ':': { result.type = TOKEN_TYPE_COLON; } break;
      case ',': { result.type = TOKEN_TYPE_COMMA; } break;
      case '{': { result.type = TOKEN_TYPE_OPEN_BRACE; } break;
      case '}': { result.type = TOKEN_TYPE_CLOSE_BRACE; } break;
      case '[': { result.type = TOKEN_TYPE_OPEN_BRACKET; } break;
      case ']': { result.type = TOKEN_TYPE_CLOSE_BRACKET; } break;

      case '"':
      {
        result.type = TOKEN_TYPE_STRING;

        U64 string_start = position;

        while (IsInBound(source, position) && (source.data[position] != '"'))
        {
          position += 1;
        }

        result.data.data = source.data + string_start;
        result.data.size = position - string_start;
        
        position += 1;
      } break;

      case '-':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      {
        U64 start = position - 1;

        result.type = TOKEN_TYPE_NUMBER;
        
        while (IsDigit(source, position))
        {
          position += 1;
        }

        if (source.data[position] == '.')
        {
          position += 1;
          while (IsDigit(source, position))
          {
            position += 1;
          }
        }

        result.data.data = source.data + start;
        result.data.size = position - start;
      } break;
    }
  }

  reader->position = position;

  return result;
}

func Element*
ParseList(GLTFReader* reader, Token start_token, enum TokenType end_type, B32 has_labels)
{
  Element* first_element = 0;
  Element* last_element = 0;
  
  while (!reader->has_error && IsInBound(reader->file_buffer, reader->position))
  {
    Buffer label = {};
    Token value = GetGLTFToken(reader);

    if (has_labels)
    {
      if (value.type == TOKEN_TYPE_STRING)
      {
        label = value.data;

        Token colon = GetGLTFToken(reader);
        if (colon.type != TOKEN_TYPE_COLON)
        {
          LOG_ERROR("Expect colon after field name");
        }
        else
        {
          value = GetGLTFToken(reader);
        }
      }
    }

    Element* element = ParseElement(reader, label, value);
    if (element)
    {
      last_element = (last_element ? last_element->next_sibling : first_element) = element;
    }
    else if (value.type == end_type)
    {
      break;
    }

    Token comma = GetGLTFToken(reader);
    if (comma.type == end_type)
    {
      break;
    }
    else if (comma.type != TOKEN_TYPE_COMMA)
    {
      GLTFError(reader, value, "Unexpected token");
    }
  }

  return first_element;
}

func Element*
ParseElement(GLTFReader* reader, Buffer label, Token token)
{
  Element* result = 0;
  Element* sub_element = 0;

  if (token.type == TOKEN_TYPE_OPEN_BRACE)
  {
    sub_element = ParseList(reader, token, TOKEN_TYPE_CLOSE_BRACE, true);
  }
  else if (token.type == TOKEN_TYPE_OPEN_BRACKET)
  {
    sub_element = ParseList(reader, token, TOKEN_TYPE_CLOSE_BRACKET, false);
  }
  
  // @TODO @ERROR: Free Memory
  result = (Element*)OS_AllocateMemory(sizeof(Element));
  result->label = label;
  result->value = token.data;
  result->first_sub_element = sub_element;
  result->next_sibling = 0;

  return result;
}

func B32
ParseGLTF(GLTFReader* reader)
{
  reader->element = ParseElement(reader, {}, GetGLTFToken(reader));

  return true;
}

func Element* LookUpElement(Element* object, Buffer label)
{
  Element* result = 0;

  if (object)
  {
    for (Element* search = object->first_sub_element; search; search = search->next_sibling)
    {
      if (AreBuffersEqual(search->label, label))
      {
        result = search;
        break;
      }
    }

    return result;
  }

  return result;
}
