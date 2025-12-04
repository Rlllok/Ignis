#include "gltf.h"

#include "base/base_core.h"
#include "base/base_string.h"
#include "sys/stat.h"

func Buffer
AllocateBuffer(U64 size)
{
  Buffer result = {0};

  result.data = (U8*)OS_AllocateMemory(size * sizeof(U8));
  if (result.data)
  {
    result.length = size;
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

  *buffer = (Buffer){0};
}

func void
PrintBuffer(Buffer buffer)
{
  if (buffer.data)
  {
    fwrite(buffer.data, sizeof(buffer.data[0]), buffer.length, stdout);
  }
}

func Buffer
GLTFReadFile(Buffer file_name)
{
  Buffer result = {0};

  // @TODO Get CString from Buffer(String)
  const char* file_name_c = CFromStr8(file_name);
  
  FILE* file = fopen(file_name_c, "rb");
  if (file)
  {
  #if IGNIS_PLATFORM_LINUX
    struct stat file_stat;
    stat(file_name_c, &file_stat);
  #endif
  #if IGNIS_PLATFORM_WIN32
    struct _stat64 file_stat;
    _stat64(file_name_c, &file_stat);
  #endif

    LOG_DEBUG("FILE SIZE GLTF: %i\n", file_stat.st_size);
    result = AllocateBuffer(file_stat.st_size);
    if (result.data)
    {
      if (fread(result.data, result.length, 1, file) != 1)
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
  B32 result = position < source.length;

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
  if (a.length != b.length)
  {
    return false;
  }

  for (U64 i = 0; i < a.length; i += 1)
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

  while ((buffer.data[result] != value) && (buffer.length > result)) result += 1;

  return result;
}

func void
GLTFError(GLTFReader* reader, GLTFToken token, const char* message)
{
  reader->has_error = true;

  LOG_ERROR("- LINE %d - \"%.*s\" - %s\n", reader->line_number + 1, (U32)token.value.length, (char*)token.value.data, message);
}

func GLTFToken
GetGLTFToken(GLTFReader* reader)
{
  GLTFToken result = {0};

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
    result.value.length = 1;
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
        result.value.length = position - string_start;
        
        position += 1;
      } break;

      case 't':
      {
        Buffer true_buffer = {0};
        true_buffer.data = source.data + position - 1;
        true_buffer.length = 4;

        if (AreBuffersEqual(true_buffer, Str8C("true")))
        {
          result.type = GLTF_TOKEN_TYPE_TRUE;
          result.value.data = true_buffer.data;
          result.value.length = 4;
          
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

        if (source.data[position] == 'E' || source.data[position] == 'e')
        {
          position += 1;
          if (source.data[position] == '-')
          {
            position += 1;
          }
          while (IsDigit(source, position))
          {
            position += 1;
          }
        }

        result.value.data = source.data + start;
        result.value.length = position - start;
      } break;
    }
  }

  reader->position = position;

  return result;
}

func GLTFElement*
ParseList(GLTFReader* reader, GLTFToken start_token, GLTFTokenType end_type, B32 has_labels)
{
  GLTFElement* first_element = 0;
  GLTFElement* last_element = 0;
  
  while (!reader->has_error && IsInBound(reader->file_buffer, reader->position))
  {
    Buffer label = {0};
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
      if (last_element)
      {
        last_element->next_sibling = element;
        last_element = last_element->next_sibling;
      }
      else
      {
        first_element = element;
        last_element = first_element;
      }
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
    else if (comma.type == GLTF_TOKEN_TYPE_ERROR)
    {
      GLTFError(reader, current_token, "ERROR");
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
  Arena* licky_arena = AllocateArena(Megabytes(64));
  
  GLTFData gltf_data = {0};
  
  GLTFElement* head = ParseElement(reader, (Buffer){0}, GetGLTFToken(reader));

  U32 node_count = 0;
  for (GLTFElement* node_element = LookUpElement(head, Str8C("nodes"))->first_sub_element;
       node_element;
       node_element = node_element->next_sibling)
  {
    node_count += 1;
  }

  gltf_data.nodes = GLTFNodeArrayAllocate(licky_arena, node_count);
  GLTF_ID current_node_id = 0;
  for (GLTFElement* node_element = LookUpElement(head, Str8C("nodes"))->first_sub_element;
       node_element;
       node_element = node_element->next_sibling)
  {
    GLTFNode node = _gltf_node_nil;
    GLTFElement* node_name = LookUpElement(node_element, Str8C("name"));
    if (node_name)
    {
      node.name = node_name->value;
    }
    node.mesh_id = GetIDElement(node_element, Str8C("mesh"));
    node.translation = GetVec3F32Element(node_element, Str8C("translation"));
    node.scale = GetVec3F32Element(node_element, Str8C("scale"));
    if (node.scale.x == 0 && node.scale.y == 0 && node.scale.z == 0)
    {
      node.scale = MakeVec3F32(1.0f, 1.0f, 1.0f);
    }
    node.rotation = GetQuaternionElement(node_element, Str8C("rotation"));
    if (node.rotation.x == 0 && node.rotation.y == 0 && node.rotation.z == 0 && node.rotation.w)
    {
      node.rotation.w = 1;
    }

    GLTFElement* node_children = LookUpElement(node_element, Str8C("children"));
    if (node_children)
    {
      GLTFElement* first_child = node_children->first_sub_element;
      node.first_child_id = (GLTF_ID)F64FromStr8(first_child->value);

      GLTFNode* sibling_node = GLTFNodeArrayGetPointer(&gltf_data.nodes, node.first_child_id);
      for (GLTFElement* child_element = first_child->next_sibling; child_element; child_element = child_element->next_sibling)
      {
        F64 sibling_id = F64FromStr8(child_element->value);
        sibling_node->next_sibling_id = sibling_id;

        sibling_node = GLTFNodeArrayGetPointer(&gltf_data.nodes, sibling_id);
      }
    }

    GLTFNodeArrayAdd(&gltf_data.nodes, node);

    current_node_id += 1;
  }

  for (I32 i = 0; i < gltf_data.nodes.length; i += 1)
  {
    GLTFNode* node = GLTFNodeArrayGetPointer(&gltf_data.nodes, i);

    if (node->first_child_id != GLTF_ID_NIL)
    {
      GLTF_ID child_id = node->first_child_id;
      while (child_id != GLTF_ID_NIL)
      {
        GLTFNode* child = GLTFNodeArrayGetPointer(&gltf_data.nodes, child_id);
        child->parent_id = i;

        child_id = child->next_sibling_id;
      }
    }
  }

  gltf_data.meshes = GLTFMeshListCreate(licky_arena);
  GLTFElement* gltf_mesh = LookUpElement(head, Str8C("meshes"));
  if (gltf_mesh)
  {
    for (GLTFElement* mesh_element = gltf_mesh->first_sub_element;
         mesh_element;
         mesh_element = mesh_element->next_sibling)
    {
      GLTFMesh mesh = {0};
      mesh.primitives = GLTFPrimitiveListCreate(licky_arena);
      for (GLTFElement* primitive_element = LookUpElement(mesh_element, Str8C("primitives"))->first_sub_element;
          primitive_element;
          primitive_element = primitive_element->next_sibling)
        {
          GLTFPrimitive primitive = {0};
          primitive.indecies_accessor_id = GetNumberElement(primitive_element, Str8C("indices"));
          primitive.material_accessor_id = GetNumberElement(primitive_element, Str8C("material"));

          GLTFElement* attributes_element = LookUpElement(primitive_element, Str8C("attributes"));
          primitive.position_accessor_id = GetNumberElement(attributes_element, Str8C("POSITION"));
          primitive.tangent_accessor_id = GetNumberElement(attributes_element, Str8C("TANGENT"));
          primitive.normal_accessor_id = GetNumberElement(attributes_element, Str8C("NORMAL"));
          primitive.texcoord_accessor_id = GetNumberElement(attributes_element, Str8C("TEXCOORD_0"));
          primitive.joints_accessor_id = GetNumberElement(attributes_element, Str8C("JOINTS_0"));
          primitive.weights_accessor_id = GetNumberElement(attributes_element, Str8C("WEIGHTS_0"));

          GLTFPrimitiveListPush(&mesh.primitives, primitive);
        }

        GLTFMeshListPush(&gltf_data.meshes, mesh);
    }
  }
  
  gltf_data.skin.joint_ids = GLTFJointIDArrayAllocate(licky_arena, 32);
  GLTFElement* gltf_skin = LookUpElement(head, Str8C("skins"));
  if (gltf_skin)
  {
    for (GLTFElement* skin_element = gltf_skin->first_sub_element;
         skin_element;
         skin_element = skin_element->next_sibling)
    {
      gltf_data.skin.inverse_bind_matrices_accessor = GetNumberElement(skin_element, Str8C("inverseBindMatrices"));
      LOG_INFO("Inverset bind: %d\n", gltf_data.skin.inverse_bind_matrices_accessor);

      for (GLTFElement* joint_id_element = LookUpElement(skin_element, Str8C("joints"))->first_sub_element;
           joint_id_element; joint_id_element = joint_id_element->next_sibling)
      {
        GLTFJointIDArrayAdd(&gltf_data.skin.joint_ids, (GLTF_ID)(F64FromStr8(joint_id_element->value)));
      }
    }
  }

  gltf_data.buffer_views = GLTFBufferViewListCreate(licky_arena);
  for (GLTFElement* buffer_view_element = LookUpElement(head, Str8C("bufferViews"))->first_sub_element;
       buffer_view_element;
       buffer_view_element = buffer_view_element->next_sibling)
  {
    GLTFBufferView buffer_view = {0};
    buffer_view.buffer_id = GetNumberElement(buffer_view_element, Str8C("buffer"));
    buffer_view.byte_offset = GetNumberElement(buffer_view_element, Str8C("byteOffset"));
    buffer_view.byte_length = GetNumberElement(buffer_view_element, Str8C("byteLength"));
    buffer_view.byte_stride = GetNumberElement(buffer_view_element, Str8C("byteStride"));

    GLTFBufferViewListPush(&gltf_data.buffer_views, buffer_view);
  }

  gltf_data.buffers = GLTFBufferListCreate(licky_arena);
  for (GLTFElement* buffer_element = LookUpElement(head, Str8C("buffers"))->first_sub_element;
       buffer_element;
       buffer_element = buffer_element->next_sibling)
  {
    GLTFBuffer buffer = {0};
    GLTFElement* uri_element = LookUpElement(buffer_element, Str8C("uri"));
    buffer.uri = uri_element ? uri_element->value : (Str8){0};
    if (FindPosition(buffer.uri, '.') != buffer.uri.length)
    {
      Arena* tmp_arena = AllocateArena(Megabytes(1));
      Str8 bin_file_path = SubStr8(tmp_arena, reader->file_path, 0, GetSymbolPositionLast(reader->file_path, '/'));
      bin_file_path = ConcatStr8(tmp_arena, bin_file_path, Str8C("/"));
      Str8 bin_file_name = SubStr8(tmp_arena, buffer.uri, 0, buffer.uri.length);
      Str8 bin_data = GLTFReadFile(ConcatStr8(tmp_arena, bin_file_path, bin_file_name));
      buffer.data = bin_data.data;
      buffer.byte_length = bin_data.length;
      FreeArena(tmp_arena);
    }
    else
    {
      U64 comma_position = FindPosition(buffer.uri, ',');
      Str8 decoded = Base64Decode(
        licky_arena,
        (Str8){
          .data = buffer.uri.data + comma_position + 1,
          .length= buffer.uri.length - comma_position - 1,
        }
      );
      buffer.data = decoded.data;
      buffer.byte_length = decoded.length;
    }

    GLTFBufferListPush(&gltf_data.buffers, buffer);
  }

  gltf_data.accessors = GLTFAccessorListCreate(licky_arena);
  for (GLTFElement* accessor_element = LookUpElement(head, Str8C("accessors"))->first_sub_element;
       accessor_element;
       accessor_element = accessor_element->next_sibling)
  {
    GLTFAccessor accessor = {0};
    accessor.buffer_view_id = GetNumberElement(accessor_element, Str8C("bufferView"));
    accessor.byte_offset = GetNumberElement(accessor_element, Str8C("byteOffset"));
    GLTFElement* type_element = LookUpElement(accessor_element, Str8C("type"));
    if (Str8Equal(Str8C("SCALAR"), type_element->value))
    {
      accessor.type = GLTFAccessorType_Scalar;
    }
    else if (Str8Equal(Str8C("VEC2"), type_element->value))
    {
      accessor.type = GLTFAccessorType_Vec2;
    }
    else if (Str8Equal(Str8C("VEC3"), type_element->value))
    {
      accessor.type = GLTFAccessorType_Vec3;
    }
    else if (Str8Equal(Str8C("VEC4"), type_element->value))
    {
      accessor.type = GLTFAccessorType_Vec4;
    }
    else if (Str8Equal(Str8C("MAT2"), type_element->value))
    {
      accessor.type = GLTFAccessorType_Mat2;
    }
    else if (Str8Equal(Str8C("MAT3"), type_element->value))
    {
      accessor.type = GLTFAccessorType_Mat3;
    }
    else if (Str8Equal(Str8C("MAT4"), type_element->value))
    {
      accessor.type = GLTFAccessorType_Mat4;
    }
    else
    {
      AssertMessage(0, "Unsupported AccessorType\n");
    }
    accessor.component_type = GetNumberElement(accessor_element, Str8C("componentType"));
    accessor.normalized = 0;
    accessor.count = GetNumberElement(accessor_element, Str8C("count"));

    GLTFAccessorListPush(&gltf_data.accessors, accessor);
  }

  GLTFElement* gltf_animation = LookUpElement(head, Str8C("animations"));
  if (gltf_animation)
  {
    gltf_data.animations = GLTFAnimationListCreate(licky_arena);
    for (GLTFElement* animation_element = gltf_animation->first_sub_element;
         animation_element;
         animation_element = animation_element->next_sibling)
    {
      GLTFAnimation animation = {0};
      GLTFElement* name = LookUpElement(animation_element, Str8C("name"));
      if (name)
      {
        animation.name = name->value;
      }
      animation.channels = GLTFChannelListCreate(licky_arena);
      for (
        GLTFElement* channel_element = LookUpElement(animation_element, Str8C("channels"))->first_sub_element;
        channel_element;
        channel_element = channel_element->next_sibling
      )
      {
        GLTFChannel channel = {0};
        channel.sampler_id = GetNumberElement(channel_element, Str8C("sampler"));
        GLTFElement* target_element = LookUpElement(channel_element, Str8C("target"));
        channel.target.node_id = GetNumberElement(target_element, Str8C("node"));
        GLTFElement* type_element = LookUpElement(target_element, Str8C("path"));
        if (Str8Equal(Str8C("translation"), type_element->value))
        {
          channel.target.type = GLTFTargetType_Translation;
        }
        else if (Str8Equal(Str8C("rotation"), type_element->value))
        {
          channel.target.type = GLTFTargetType_Rotation;
        }
        else if (Str8Equal(Str8C("scale"), type_element->value))
        {
          channel.target.type = GLTFTargetType_Scale;
        }
        else
        {
          AssertMessage(0, "Unsupported TargetPathType\n");
        }
        
        GLTFChannelListPush(&animation.channels, channel);
      }

      animation.samplers = GLTFSamplerListCreate(licky_arena);
      for (
        GLTFElement* sampler_element = LookUpElement(animation_element, Str8C("samplers"))->first_sub_element;
        sampler_element;
        sampler_element = sampler_element->next_sibling
      )
      {
        GLTFSampler sampler = {0};
        sampler.input_accessor_id = GetNumberElement(sampler_element, Str8C("input"));
        sampler.output_accessor_id = GetNumberElement(sampler_element, Str8C("output"));

        GLTFSamplerListPush(&animation.samplers, sampler);
      }

      GLTFAnimationListPush(&gltf_data.animations, animation);
    }
  }
  
  return gltf_data;
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
  }

  return result;
}

func void*
GetDataFromGLTFAccessor(GLTFData gltf_data, GLTFAccessor accessor, U64 byte_offset)
{
  GLTFBufferView buffer_view = GLTFBufferViewListGetItem(&gltf_data.buffer_views, accessor.buffer_view_id);
  GLTFBuffer buffer = GLTFBufferListGetItem(&gltf_data.buffers, buffer_view.buffer_id);

  return (void*)(buffer.data + accessor.byte_offset + buffer_view.byte_offset + byte_offset);
}

func F32
GetF32FromGLTFAccessor(GLTFData gltf_data, GLTFAccessor accessor, U32 index)
{
  F32 result = 0.0f;

  if (index >= accessor.count)
  {
    LOG_DEBUG("Out of accessor's data length.\n");
    return result;
  }
  if (accessor.type != GLTFAccessorType_Scalar && accessor.component_type != GLTFComponentType_Float)
  {
    LOG_DEBUG("Accessor's type is not F32.\n");
  }
  result = *(F32*)GetDataFromGLTFAccessor(gltf_data, accessor, sizeof(F32)*index);
  return result;
}

func Vec3F32
GetVec3F32FromGLTFAccessor(GLTFData gltf_data, GLTFAccessor accessor, U32 index)
{
  Vec3F32 result = MakeVec3F32(0.0f, 0.0f, 0.0f);

  if (index >= accessor.count)
  {
    LOG_DEBUG("Out of accessor's data length.");
    return result;
  }
  if (accessor.type != GLTFAccessorType_Vec3 && accessor.component_type != GLTFComponentType_Float)
  {
    LOG_DEBUG("Accessor's type is not Vec3F32.\n");
  }
  result = *(Vec3F32*)GetDataFromGLTFAccessor(gltf_data, accessor, sizeof(Vec3F32)*index);
  return result;
}

func Vec4F32
GetVec4F32FromGLTFAccessor(GLTFData gltf_data, GLTFAccessor accessor, U32 index)
{
  Vec4F32 result = MakeVec4F32(0.0f, 0.0f, 0.0f, 0.0f);

  if (index >= accessor.count)
  {
    LOG_DEBUG("Out of accessor's data length.");
    return result;
  }
  if (accessor.type != GLTFAccessorType_Vec4 && accessor.component_type != GLTFComponentType_Float)
  {
    LOG_DEBUG("Accessor's type is not Vec4F32.\n");
  }
  result = *(Vec4F32*)GetDataFromGLTFAccessor(gltf_data, accessor, sizeof(Vec4F32)*index);
  return result;
}

func Quaternion GetQuaternionFromGLTFAccessor(GLTFData gltf_data, GLTFAccessor accessor, U32 index)
{
  Quaternion result = IdentityQuaternion();

  if (index >= accessor.count)
  {
    LOG_DEBUG("Out of accessor's data length.");
    return result;
  }
  if (accessor.type != GLTFAccessorType_Vec4 && accessor.component_type != GLTFComponentType_Float)
  {
    LOG_DEBUG("Accessor's type is not Quaternion.\n");
  }
  result = *(Quaternion*)GetDataFromGLTFAccessor(gltf_data, accessor, sizeof(Quaternion)*index);
  return result;
}
