#include "base/base_include.h"
#include "os/os_memory.h"

#include "base/base_include.c"
#include "os/win32/os_win32_memory.c"

DefineList(Vec2)

I32 main(void)
{
	Arena* arena = AllocateArena(Kilobytes(4));
	ListVec2 list = CreateListVec2(arena);
	PushListVec2(&list, MakeVec2(1.0f, 5.0f));

	return 0;
}

