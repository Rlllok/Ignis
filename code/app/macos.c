#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.c"
#include "os/os_include.c"

I32 main() {
  LogInfo("Hello MacOS\n");

  Arena* arena = AllocateArena(Gigabytes(4), Kilobytes(16));
  Arena* frame_arena = AllocateArena(Gigabytes(4), Kilobytes(16));
  B32 finished = 0;

  OS_Init(Megabytes(16));

  Vec2U32 window_size = MakeVec2U32(1280, 720);
  OS_Window* window = OS_CreateWindow(Str8C("Simple Triangle Test (MacOS)"), window_size);

  while (!finished) {
    OS_EventList event_list = OS_GetEventList(frame_arena, window);

    ResetArena(frame_arena);
  }

  return 0;
}
