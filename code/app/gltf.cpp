#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"
#include "assets/mesh.h"

#include "os/os_memory.h"
#include "sys/stat.h"
#include "assets/mesh.cpp"

I32 main()
{
  LOG_INFO("GLTF Test message!\n");

#if IGNIS_DEBUG
  LOG_INFO("DEBUG BUILD\n");
#endif // IGNIS_DEBUG

  GLTFReader gltf_reader = {};
  gltf_reader.file_buffer = ReadFile(ConstString("data/box_gltf/test.gltf"));

  GLTFData gltf_data = GetGLTFData(&gltf_reader);
  GLTFBufferView buffer_view = gltf_data.buffer_view_list.first->data;
  
  LOG_INFO("%d", buffer_view.byte_length);
  
  return 0;
}
