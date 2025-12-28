#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"
#include "assets/animation.h"
#include "assets/mesh.h"
#include "ui/ui_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "render/r_include.c"
#include "assets/animation.c"
#include "assets/mesh.c"
#include "ui/ui_include.c"

#include "ignis.h"
#include "ignis_r.h"
#include "ignis_ui.h"

#include "ignis.c"
#include "ignis_r.c"
#include "ignis_ui.c"

typedef struct Ignis_State Ignis_State;
struct Ignis_State
{
  Arena* arena;

  OS_Window window;

  B32 finished;
} _ignis_state;

func void Init_Ignis();
func void Ignis_HandleEvents(Arena* arena);

I32 main()
{
  LOG_DEBUG("Hello Ignis\n");
  Init_Ignis();

  while (!_ignis_state.finished)
  {
    Ignis_HandleEvents(_ignis_state.arena);
  }

  return 0;
}

func void
Init_Ignis()
{
  _ignis_state = (Ignis_State){0};

  _ignis_state.arena = AllocateArena(Megabytes(32));

  OS_Init(Megabytes(32));
  OS_CreateWindow(Str8C("Ignis"), MakeVec2U32(1280, 720), &_ignis_state.window);
  OS_ShowWindow(&_ignis_state.window);
}

func void
Ignis_HandleEvents(Arena* arena)
{
  OS_EventList event_list = OS_GetEventList(arena, &_ignis_state.window);

  if (OS_IsKeyPressed(OS_KEY_ESC))
  {
    _ignis_state.finished = 1;
  }
}
