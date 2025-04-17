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
  gltf_reader.file_buffer = ReadFile("data/box_gltf/test.gltf");

  Arena* list_arena = AllocateArena(Megabytes(4));
  List list = CreateList(list_arena);
  {
    I32 item0 = 110;
    PushList(&list, I32, &item0);
    I32 item1 = 10;
    PushList(&list, I32, &item1);
  }

  I32 item = *(I32*)list.first->data;

  GLTFData gltf_data = GetGLTFData(&gltf_reader);
  GLTFBufferView buffer_view = *(GLTFBufferView*)gltf_data.buffer_view_list.first->next->data;
  
  return 0;
}
