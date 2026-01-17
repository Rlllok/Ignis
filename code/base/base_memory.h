#pragma once

#include "memory.h"

#include "base_core.h"

// --AlNov: @TODO
// There is no implementation of alignment ( I should to read about alignment more).

typedef struct Arena Arena;
struct Arena
{
  U64 position;
  U64 reserved;
  U64 commited;
};

func Arena* AllocateArena(U64 reserve_size, U64 commit_size);

// --AlNov: @TODO Initialize with zero
func void* PushArena(Arena* arena, U64 size);
func void* PushCopyArena(Arena* arena, U64 size, void* data);

func void ResetArena(Arena* arena);
func void FreeArena(Arena* arena);

// -- Scratch Arena --------------------------------------------------
typedef struct ScratchArena ScratchArena;
struct ScratchArena
{
  Arena* arena;
  U64    position;
};

func ScratchArena BeginScratchArena(Arena* arena);
func void         EndScratchArena  (ScratchArena scratch);
