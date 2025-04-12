#include "r_gltf.h"

#include "sys/stat.h"

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

func U64
FindPosition(Buffer buffer, U8 value)
{
  U64 result = 0;

  while (buffer.data[result] != value) result += 1;

  return result;
}


func void
GLTFError(GLTFReader* reader, GLTFToken token, const char* message)
{
  reader->has_error = true;

  LOG_ERROR("- LINE %d - \"%.*s\" - %s\n", reader->line_number + 1, (U32)token.value.size, (char*)token.value.data, message);
}

func GLTFToken
GetGLTFToken(GLTFReader* reader)
{
  GLTFToken result = {};

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
    result.type = GLTF_TOKEN_TYPE_ERROR;
    result.value.size = 1;
    result.value.data = source.data + position;

    U8 value = source.data[position];
    position += 1;
    
    switch (value)
    {
      case ':': { result.type = GLTF_TOKEN_TYPE_COLON; } break;
      case ',': { result.type = GLTF_TOKEN_TYPE_COMMA; } break;
      case '{': { result.type = GLTF_TOKEN_TYPE_OPEN_BRACE; } break;
      case '}': { result.type = GLTF_TOKEN_TYPE_CLOSE_BRACE; } break;
      case '[': { result.type = GLTF_TOKEN_TYPE_OPEN_BRACKET; } break;
      case ']': { result.type = GLTF_TOKEN_TYPE_CLOSE_BRACKET; } break;

      case '"':
      {
        result.type = GLTF_TOKEN_TYPE_STRING;

        U64 string_start = position;

        while (IsInBound(source, position) && (source.data[position] != '"'))
        {
          position += 1;
        }

        result.value.data = source.data + string_start;
        result.value.size = position - string_start;
        
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

        result.type = GLTF_TOKEN_TYPE_NUMBER;
        
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

        result.value.data = source.data + start;
        result.value.size = position - start;
      } break;
    }
  }

  reader->position = position;

  return result;
}

func GLTFElement*
ParseList(GLTFReader* reader, GLTFToken start_token, enum GLTFTokenType end_type, B32 has_labels)
{
  GLTFElement* first_element = 0;
  GLTFElement* last_element = 0;
  
  while (!reader->has_error && IsInBound(reader->file_buffer, reader->position))
  {
    Buffer label = {};
    GLTFToken current_token = GetGLTFToken(reader);

    if (has_labels)
    {
      if (current_token.type == GLTF_TOKEN_TYPE_STRING)
      {
        label = current_token.value;

        GLTFToken colon = GetGLTFToken(reader);
        if (colon.type != GLTF_TOKEN_TYPE_COLON)
        {
          LOG_ERROR("Expect colon after field name");
        }
        else
        {
          current_token = GetGLTFToken(reader);
        }
      }
    }

    GLTFElement* element = ParseElement(reader, label, current_token);
    if (element)
    {
      last_element = (last_element ? last_element->next_sibling : first_element) = element;
    }
    else if (current_token.type == end_type)
    {
      break;
    }

    GLTFToken comma = GetGLTFToken(reader);
    if (comma.type == end_type)
    {
      break;
    }
    else if (comma.type != GLTF_TOKEN_TYPE_COMMA)
    {
      GLTFError(reader, current_token, "Unexpected token");
    }
  }

  return first_element;
}

func GLTFElement*
ParseElement(GLTFReader* reader, Buffer label, GLTFToken token)
{
  GLTFElement* result = 0;
  GLTFElement* sub_element = 0;

  if (token.type == GLTF_TOKEN_TYPE_OPEN_BRACE)
  {
    sub_element = ParseList(reader, token, GLTF_TOKEN_TYPE_CLOSE_BRACE, true);
  }
  else if (token.type == GLTF_TOKEN_TYPE_OPEN_BRACKET)
  {
    sub_element = ParseList(reader, token, GLTF_TOKEN_TYPE_CLOSE_BRACKET, false);
  }
  
  // @TODO @ERROR: Free Memory
  result = (GLTFElement*)OS_AllocateMemory(sizeof(GLTFElement));
  result->label = label;
  result->value = token.value;
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

func GLTFElement*
LookUpElement(GLTFElement* object, Buffer label)
{
  GLTFElement* result = 0;

  if (object)
  {
    for (GLTFElement* search = object->first_sub_element; search; search = search->next_sibling)
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
