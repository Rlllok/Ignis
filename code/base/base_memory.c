#include "base_memory.h"

#include "base_logger.h"
#include "os/os_memory.h"

func Arena*
AllocateArena(U64 reserve_size, U64 commit_size) {
  OS_ReserveResult reserve_result = OS_ReserveMemory(reserve_size);
  U64 commit_result_size = OS_CommitMemory(reserve_result.ptr, commit_size);
  Arena* arena = (Arena*)reserve_result.ptr;
  arena->position = sizeof(Arena);
  arena->reserved = reserve_result.size;
  arena->commited = commit_result_size;

  return arena;
}

func void*
PushArena(Arena* arena, U64 size) {
  void* result = 0;

  if ((arena->position + size) < arena->commited) {
    result = (void*)((U8*)arena + arena->position);
    arena->position += size;
  }
  else {
    // --AlNov 7 January 2026: @TODO No strategy for growing buffer
    U64 commit_result_size = OS_CommitMemory((void*)((U8*)arena + arena->commited), size);
    arena->commited += commit_result_size;
    
    if (arena->commited > arena->reserved) {
      Assert(!"Not enough space for allocation");
    }

    result = (void*)((U8*)arena + arena->position);
    arena->position += size;
  }

  return result;
}

func void* PushCopyArena(Arena* arena, U64 size, void* data) {
  void* result = PushArena(arena, size);
  memcpy(result, data, size);
  return result;
}

func void
ResetArena(Arena* arena) {
  arena->position = sizeof(Arena);
}

func void
FreeArena(Arena* arena) {
  OS_FreeMemory(arena, arena->reserved);
}

func ScratchArena
BeginScratchArena(Arena* arena) {
  ScratchArena result = {
    .arena = arena,
    .position = arena->position,
  };
  return result;
}

func void
EndScratchArena(ScratchArena scratch) {
  scratch.arena->position = scratch.position;
}
