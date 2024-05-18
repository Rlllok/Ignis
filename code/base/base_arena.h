#pragma once

#include "base_core.h"

// --AlNov: @TODO It is naive implementation of allocator to test is it works at all.
// There is no implementation of alignment ( I should to read about alignment more).
// There is no resize for arena.
// I should play more with arena to understand how I can use for it.

struct Arena
{
    u64 position;
    u64 size;
};

func Arena* AllocateArena(u64 size);

// --AlNov: @TODO Initialize with zero
func void* PushArena(Arena* arena, u64 size);

func void ResetArena(Arena* arena);
func void FreeArena(Arena* arena);
