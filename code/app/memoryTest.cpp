#include <windows.h>
#include <stdio.h>
#include <stdint.h>

#pragma comment(lib, "user32.lib")

typedef int32_t i32;
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

#define Kilobytes(n) (n << 10)
#define Gigabytes(n) (((u64)n) << 30)

void fillMallocMemory(u8* array, u32 size)
{
    for (u32 i = 0; i < size; ++i)
    {
        array[i] = 1;
    }
}

struct Arena
{
    Arena* pointer;
    u64 size;
    u64 position;
    u64 commitPosition;
};

Arena* allocateArena(u64 size)
{
    // AlNov: @NOTE malloc doesn't allocate memory if size is more than phisical memory's size
    // Tried to allocate with more than 16 GB. Looks like it doesn's allocate memory at all.
    // Could not access arena-size in such situation

    // AlNov: @NOTE VirtualAlloc with MEM_COMMIT Flag has the same behavior as described for malloc.
    
    // AlNov: @NOTE To be precise, 22 Gigabytes is error's line. 23 Gigabytes and there is no allocation.
    // And malloc has the same property.

    // void* memoryBlock = malloc(size);
    void* memoryBlock = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
    VirtualAlloc(memoryBlock, size, MEM_COMMIT, PAGE_READWRITE);
    Arena* arena = (Arena*)memoryBlock;
    arena->size = size;
    arena->position = sizeof(Arena);
    arena->commitPosition = Kilobytes(4);

    return arena;
}

void releaseArena(Arena* arena)
{
    VirtualFree(arena, 0, MEM_RELEASE);
}

void* pushArena(Arena* arena, u64 size)
{
    void* result = nullptr;
    if (arena->position + size <= arena->size)
    {
        result = arena + arena->position;
        arena->position += size;

        return result; 
    }
    else
    {
        // AlNov: Out of arena size; Error
        return result;
    }
}

#define AllocateArray(arena, type, size) (type*)pushArena(arena, size * sizeof(type))

int main()
{
    // u8* mallocMemory = (u8*)malloc(sizeof(u8) * 1e9);

    // fillMallocMemory(mallocMemory, 1e9 / 4);

    // AlNov: @NOTE Physical memory will be used when allocated memory is used.
    // Even if allocated array's size is 4 GB, there will not be 4 GB of RAM.
    // There will be as much physical memory used as the size of initialized elements of array
    i32* intArenaArray;
    Arena* arena = allocateArena(Gigabytes(4));
    {
        u32 size = 1000000000;
        intArenaArray = AllocateArray(arena, i32, size);

        for (i32 i = 0; i < size / 1; ++i)
        {
            intArenaArray[i] = i;
        }

        // printf("Int Arena Array:\n");
        // printf("\t");
        // for (i32 i = 0; i < 10; ++i)
        // {
        //     printf("%i ", intArenaArray[i]);
        // }
        // printf("\n");

        int a = 10;
        int b = a; 
    }
    releaseArena(arena);

    return 0;
};