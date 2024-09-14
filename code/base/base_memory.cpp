#include "base_memory.h"

#include "base_logger.h"
#include "os/os_memory.h"

#include <stdlib.h>
#include <stdio.h>

func Arena*
AllocateArena(U64 size)
{
  void* memoryBlock = OS_AllocateMemory(size);
  Arena* arena = (Arena*)memoryBlock;
  arena->position = sizeof(Arena);
  arena->size = size;

  return arena;
}

func void*
PushArena(Arena* arena, U64 size)
{
  void* result = nullptr;

  if ((arena->position + size) < arena->size)
  {
    result = arena + arena->position;
    arena->position += size;
  }
  else
  {
    Assert(1 && "Not enough space for allocation");
  }

  return result;
}

  func void
ResetArena(Arena* arena)
{
  arena->position = sizeof(Arena);
}

  func void
FreeArena(Arena* arena)
{
  OS_FreeMemory(arena);
}
