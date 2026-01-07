#include "base_memory.h"

#include "base_logger.h"
#include "os/os_memory.h"

#include <stdlib.h>
#include <stdio.h>

func Arena*
AllocateArena(U64 size)
{
  void* ptr = OS_ReserveMemory(Gigabytes(64));
  OS_CommitMemory(ptr, size);
  Arena* arena = (Arena*)ptr;
  arena->position = sizeof(Arena);
  arena->size = size;

  return arena;
}

func void*
PushArena(Arena* arena, U64 size)
{
  void* result = 0;

  if ((arena->position + size) < arena->size)
  {
    result = (void*)((U8*)arena + arena->position);
    arena->position += size;
  }
  else
  {
    Assert(0 && "Not enough space for allocation");
  }

  return result;
}

func void* PushCopyArena(Arena* arena, U64 size, void* data)
{
  void* result = PushArena(arena, size);
  memcpy(result, data, size);
  return result;
}

func void
ResetArena(Arena* arena)
{
  arena->position = sizeof(Arena);
	OS_ZeroMemory((U8*)arena + arena->position, arena->size - arena->position);
}

func void
FreeArena(Arena* arena)
{
  OS_FreeMemory(arena, Gigabytes(64));
}
