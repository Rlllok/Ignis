#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"

#include "os/os_memory.h"
#include "sys/stat.h"

#include "render/r_gltf.h"
#include "render/r_gltf.cpp"

I32 main()
{
  LOG_INFO("GLTF Test message!\n");

  GLTFReader gltf_reader = {};
  gltf_reader.file_buffer = ReadFile("data/box_gltf/test.gltf");

  ParseGLTF(&gltf_reader);

  GLTFElement* gltf_buffer_views = LookUpElement(gltf_reader.element, ConstString("bufferViews"));

  GLTFData gltf_data = {};
  
  for (GLTFElement* buffer_view = gltf_buffer_views->first_sub_element;
       buffer_view;
       buffer_view = buffer_view->next_sibling)
  {
    GLTFBufferView current_view = {};
    current_view.buffer_id = GetNumberElement(buffer_view, ConstString("buffer"));
    current_view.offset = GetNumberElement(buffer_view, ConstString("byteOffset"));
    current_view.byte_length = GetNumberElement(buffer_view, ConstString("byteLength"));
    current_view.target = GetNumberElement(buffer_view, ConstString("target"));

    GLTFAddBufferView(&gltf_data, current_view);
  }

  GLTFElement* gltf_meshes = LookUpElement(gltf_reader.element, ConstString("meshes"));
  GLTFElement* gltf_primitives = LookUpElement(gltf_meshes->first_sub_element, ConstString("primitives"));

  GLTFMesh mesh = {};
  mesh.indices_accessor_id = GetNumberElement(gltf_primitives->first_sub_element, ConstString("indices"));
  GLTFElement* gltf_attributes = LookUpElement(gltf_primitives->first_sub_element, ConstString("attributes"));
  mesh.atribute.acessor_id = GetNumberElement(gltf_attributes, ConstString("POSITION"));

  Buffer test = Base64Decode(ConstString("cGFzc3dvcmQ="));
  PrintBuffer(test);

  U64 comma_position = FindPosition(ConstString("Hello, World"), ',');
  
  return 0;
}
