#include "base_memory.h"

#include "base_logger.h"
#include "os/os_memory.h"

#include <stdlib.h>
#include <stdio.h>

func Arena*
AllocateArena(U64 reserve_size, U64 commit_size)
{
  void* ptr = OS_ReserveMemory(reserve_size);
  OS_CommitMemory(ptr, commit_size);
  Arena* arena = (Arena*)ptr;
  arena->position = sizeof(Arena);
  arena->reserved = reserve_size;
  arena->commited = commit_size;

  return arena;
}

func void*
PushArena(Arena* arena, U64 size)
{
  void* result = 0;

  if ((arena->position + size) < arena->commited)
  {
    result = (void*)((U8*)arena + arena->position);
    arena->position += size;
  }
  else
  {
    // --AlNov 7 January 2026: @TODO No strategy for growing buffer
    U64 commit_size = Kilobytes(4)*1024;
    OS_CommitMemory((void*)((U8*)arena + arena->commited), commit_size);
    arena->commited += commit_size;
    
    if (arena->commited > arena->reserved)
    {
      Assert(!"Not enough space for allocation");
    }

    LOG_DEBUG("Grow Arena\n");
    LOG_DEBUG("reserved: %llu commited: %llu position: %llu size: %llu\n", arena->reserved, arena->commited, arena->position, size);
    result = (void*)((U8*)arena + arena->position);
    arena->position += size;

    LOG_DEBUG("new_position: %llu\n", arena->position);
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
	// OS_ZeroMemory((U8*)arena + arena->position, arena->size - arena->position);
}

func void
FreeArena(Arena* arena)
{
  OS_FreeMemory(arena, arena->reserved);
}
