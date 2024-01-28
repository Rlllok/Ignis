#include "base_arena.h"

#include <stdlib.h>

Arena* AllocateArena(u64 size)
{
    void* memoryBlock = malloc(size);
    Arena* arena = (Arena*)memoryBlock;
    arena->position = sizeof(Arena);
    arena->size = size;

    return arena;
}

void* PushArena(Arena* arena, u64 size)
{
    void* result = nullptr;

    if (arena->position + size < arena->size)
    {
        result = arena + arena->position;
        arena->position += size;
    }
    else
    {
        // --AlNov: Error for now
    }

    return result;
}

void ResetArena(Arena* arena)
{
    arena->position = 0;
}

void FreeArena(Arena* arena)
{
    free(arena);
}