#include "base_arena.h"

#include <stdlib.h>
#include <stdio.h>

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

    if ((arena->position + size) < arena->size)
    {
        result = arena + arena->position;
        arena->position += size;
    }
    else
    {
        printf("Not enough space for size %llu. Current position: %llu\n", size, arena->position);
        // --AlNov: Error for now
    }

    return result;
}

void ResetArena(Arena* arena)
{
    arena->position = sizeof(Arena);
}

void FreeArena(Arena* arena)
{
    free(arena);
}
