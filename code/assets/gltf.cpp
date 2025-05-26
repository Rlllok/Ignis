#include "gltf.h"

#include "base/base_core.h"
#include "base/base_string.h"
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
ReadFile(Buffer file_name)
{
  Buffer result = {};

  // @TODO Get CString from Buffer(String)
  const char* file_name_c = CFromStr8(file_name);
  
  FILE* file = fopen(file_name_c, "rb");
  if (file)
  {
    struct stat64 stat;
    stat64(file_name_c, &stat);

    result = AllocateBuffer(stat.st_size);
    if (result.data)
    {
      if (fread(result.data, result.size, 1, file) != 1)
      {
        LOG_ERROR("ERROR: Unable to read \"%s\".\n", file_name_c);
        FreeBuffer(&result);
      }
    }

    fclose(file);
  }
  else
  {
    LOG_ERROR("ERROR: Unable to open \"%s\"", file_name_c);
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

  while ((buffer.data[result] != value) && (buffer.size > result)) result += 1;

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

      case 't':
      {
        Buffer true_buffer = {};
        true_buffer.data = source.data + position - 1;
        true_buffer.size = 4;

        if (AreBuffersEqual(true_buffer, Str8FromC("true")))
        {
          result.type = GLTF_TOKEN_TYPE_TRUE;
          result.value.data = true_buffer.data;
          result.value.size = 4;
          
          position += 3;
        }
        
      } break;

      case 'f':
      {
        result.type = GLTF_TOKEN_TYPE_FALSE;

        
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
  B32 valid = true;
  
  GLTFElement* sub_element = 0;

  if (token.type == GLTF_TOKEN_TYPE_OPEN_BRACE)
  {
    sub_element = ParseList(reader, token, GLTF_TOKEN_TYPE_CLOSE_BRACE, true);
  }
  else if (token.type == GLTF_TOKEN_TYPE_OPEN_BRACKET)
  {
    sub_element = ParseList(reader, token, GLTF_TOKEN_TYPE_CLOSE_BRACKET, false);
  }
  else if ((token.type == GLTF_TOKEN_TYPE_TRUE) ||
           (token.type == GLTF_TOKEN_TYPE_FALSE) ||
           (token.type == GLTF_TOKEN_TYPE_STRING) ||
           (token.type == GLTF_TOKEN_TYPE_NUMBER))
  {
    
  }
  else
  {
    valid = false;
  }
  
  // @TODO @ERROR: Free Memory
  GLTFElement* result = 0;

  if (valid)
  {
    result = (GLTFElement*)OS_AllocateMemory(sizeof(GLTFElement));
    result->label = label;
    result->value = token.value;
    result->first_sub_element = sub_element;
    result->next_sibling = 0;
  }

  return result;
}

func GLTFData
GetGLTFData(GLTFReader* reader)
{
  // @TODO @NOTE NO FREE
  Arena* licky_arena = AllocateArena(Megabytes(16));
  
  GLTFData result = {};
  
  GLTFElement* head = ParseElement(reader, {}, GetGLTFToken(reader));

  result.default_scene_id = GetNumberElement(head, Str8FromC("scene"));
  
  // Meshes
  result.mesh_list = CreateListGLTFMesh(licky_arena);
  GLTFElement* meshes_list_element = LookUpElement(head, Str8FromC("meshes"));
  for (GLTFElement* mesh_element = meshes_list_element->first_sub_element;
       mesh_element;
       mesh_element = mesh_element->next_sibling)
  {
    GLTFMesh mesh = {};
    mesh.primitive_list = CreateListGLTFPrimitive(licky_arena);
    
    GLTFElement* primitives_list_element = LookUpElement(mesh_element, Str8FromC("primitives"));
    for (GLTFElement* primitive_element = primitives_list_element->first_sub_element;
         primitive_element;
         primitive_element = primitive_element->next_sibling)
    {
      GLTFPrimitive primitive = {};
      GLTFElement* attributes = LookUpElement(primitive_element, Str8FromC("attributes"));
      for (GLTFElement* attribute_element = attributes->first_sub_element;
           attribute_element;
           attribute_element = attribute_element->next_sibling)
      {
        GLTFAttribute attribute = {};
        if (AreBuffersEqual(attribute_element->label, Str8FromC("POSITION")))
        {
          primitive.position_accessor_id = GetNumberElement(attributes, Str8FromC("POSITION"));
        }
        else if (AreBuffersEqual(attribute_element->label, Str8FromC("NORMAL")))
        {
          primitive.normal_accessor_id = GetNumberElement(attributes, Str8FromC("NORMAL"));
        }
        else if (AreBuffersEqual(attribute_element->label, Str8FromC("TEXCOORD_0")))
        {
          primitive.texcoord_accessor_id = GetNumberElement(attributes, Str8FromC("TEXCOORD_0"));
        }
        else
        {
          LOG_ERROR("Wrong Attribute name\n");
        }
      }
            
      primitive.indices_accessor_id = GetNumberElement(primitive_element, Str8FromC("indices"));
      
      PushListGLTFPrimitive(&mesh.primitive_list, primitive);
    }
    PushListGLTFMesh(&result.mesh_list, mesh);
  }

  // BufferViews
  result.buffer_view_list = CreateListGLTFBufferView(licky_arena);
  GLTFElement* buffer_views_list = LookUpElement(head, Str8FromC("bufferViews"));
  for (GLTFElement* buffer_view_element = buffer_views_list->first_sub_element;
       buffer_view_element;
       buffer_view_element = buffer_view_element->next_sibling)
  {
    GLTFBufferView buffer_view = {};
    
    buffer_view.buffer_id = GetNumberElement(buffer_view_element, Str8FromC("buffer"));
    buffer_view.byte_offset = GetNumberElement(buffer_view_element, Str8FromC("byteOffset"));
    buffer_view.byte_length = GetNumberElement(buffer_view_element, Str8FromC("byteLength"));
    buffer_view.target = GetNumberElement(buffer_view_element, Str8FromC("target"));

    PushListGLTFBufferView(&result.buffer_view_list, buffer_view);
  }

  // Buffer
  result.buffer_list = CreateListGLTFBuffer(licky_arena);
  GLTFElement* buffers_element = LookUpElement(head, Str8FromC("buffers"));
  for (GLTFElement* buffer_element = buffers_element->first_sub_element;
       buffer_element;
       buffer_element = buffer_element->next_sibling)
  {
    GLTFBuffer buffer = {};

    buffer.uri = LookUpElement(buffer_element, Str8FromC("uri"))->value;
    if (FindPosition(buffer.uri, '.') != buffer.uri.size)
    {
      Arena* tmp_arena = AllocateArena(Megabytes(1));
      Str8 bin_file_path = SubStr8(tmp_arena, reader->file_path, 0, GetSymbolPositionLast(reader->file_path, '/'));
      bin_file_path = ConcatStr8(tmp_arena, bin_file_path, Str8FromC("/"));
      Str8 bin_file_name = SubStr8(tmp_arena, buffer.uri, 0, buffer.uri.size);
      buffer.buffer = ReadFile(ConcatStr8(tmp_arena, bin_file_path, bin_file_name));
      FreeArena(tmp_arena);
    }
    else
    {
      U64 comma_position = FindPosition(buffer.uri, ',');
      buffer.buffer.data = buffer.uri.data + comma_position + 1;
      buffer.buffer.size = buffer.uri.size - comma_position - 1;
      buffer.buffer = Base64Decode(buffer.buffer);  
    }

    PushListGLTFBuffer(&result.buffer_list, buffer);
  }

  // Accessors
  result.accessor_list = CreateListGLTFAccessor(licky_arena);
  GLTFElement* accessors_element = LookUpElement(head, Str8FromC("accessors"));
  for (GLTFElement* accessor_element = accessors_element->first_sub_element;
       accessor_element;
       accessor_element = accessor_element->next_sibling)
  {
    GLTFAccessor accessor = {};

    accessor.buffer_view_id = GetNumberElement(accessor_element, Str8FromC("bufferView"));
    accessor.byte_offset = GetNumberElement(accessor_element, Str8FromC("byteOffset"));
    accessor.count = GetNumberElement(accessor_element, Str8FromC("count"));

    PushListGLTFAccessor(&result.accessor_list, accessor);
  }
  
  return result;
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

func Buffer
GetGltfBufferData(GLTFBuffer gltf_buffer)
{
  Buffer result = {};


  return result;
}
