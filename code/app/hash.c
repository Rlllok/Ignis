#include "base/base_core.h"
#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.c"
#include "os/os_include.c"

DefineHashMap(I32)
DefineHashMap(F32)

I32 main()
{
  Arena* arena = AllocateArena(Megabytes(4));
  HashMapI32 map_i32 = HashMapI32Create(arena, 16);
  HashMapF32 map_f32 = HashMapF32Create(arena, 16);
  
  HashMapI32Put(&map_i32, Str8FromC("Bob"), 15);
  HashMapI32Put(&map_i32, Str8FromC("Bib"), 20);
  LOG_INFO("%d", HashMapI32Get(map_i32, Str8FromC("Bob")));
  LOG_INFO("%d", HashMapI32Get(map_i32, Str8FromC("Bib")));
  HashMapF32Put(&map_f32, Str8FromC("Bob"), 15.0f);
  HashMapF32Put(&map_f32, Str8FromC("Bib"), 20.0f);
  LOG_INFO("%f", HashMapF32Get(map_f32, Str8FromC("Bob")));
  LOG_INFO("%f", HashMapF32Get(map_f32, Str8FromC("Bib")));

  return 0;
}
