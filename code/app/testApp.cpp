#include "../base/base_include.h"
#include "../os/os_include.h"
#include "../render/vulkan/r_init_vk.h"

#include "../base/base_include.cpp"
#include "../os/os_include.cpp"
#include "../render/vulkan/r_init_vk.cpp"

#include <cmath>

R_Square* CreateSquare(Arena* arena, Vec3f color, Vec3f centerPosition);

int main()
{
    Vec2u windowExtent = MakeVec2u(1280, 720);

    OS_Window window = OS_CreateWindow("Test App", windowExtent);

    R_Init(window);

    OS_ShowWindow(&window);

    u16 bIsFinished = false;
    // f32 startTime = OS_CurrentTimeSeconds();
    LARGE_INTEGER win32Freq;
    QueryPerformanceFrequency(&win32Freq);

    LARGE_INTEGER win32Cycles;
    QueryPerformanceCounter(&win32Cycles);

    u64 frequency = win32Freq.QuadPart;
    u64 startCycles = win32Cycles.QuadPart;
    // --AlNov: @TODO @NOTE
    // If there is more information than 10 KB Arena drops error
    Arena* frameArena = AllocateArena(Kilobytes(10));
    while (!bIsFinished)
    {
        QueryPerformanceCounter(&win32Cycles);
        u64 endCycles = win32Cycles.QuadPart;
        u64 cyclesDelta = endCycles - startCycles;
        startCycles = endCycles;
        f32 ms = (1000.0f) * (f32)cyclesDelta / (f32)frequency;
        u32 fps = frequency / cyclesDelta;
        printf("MS: %fms    FPS: %i\n", ms, fps);

        // AlNov: @TODO Cannot understand why OS_EventList working.
        // Not OS_EventList*. I think this is because we are using arena that created there,
        // and pushed to function. Should study more about this case
        OS_EventList eventList = OS_GetEventList(frameArena);
        
        OS_Event* event = eventList.firstEvent;
        while (event)
        {
            if (event->type == OS_EVENT_TYPE_EXIT)
            {
                bIsFinished = true;
            }

            event = event->next;
        }

        static f32 t = 0.0f;
        t += 0.001f * ms;
        f32 colorValue = sinf(t);
        colorValue = (colorValue + 1.0f) / 2.0f;

        R_Square* square0 = CreateSquare(frameArena, MakeVec3f(1.0f, 0.0f, 0.0f), MakeVec3f(1.0f, 0.0f, 0.0f));
        R_AddSquareToDrawList(square0);
        R_Square* square1 = CreateSquare(frameArena, MakeVec3f(0.0f, 1.0f, 0.0f), MakeVec3f(0.0f, 1.0f, 0.0f));
        R_AddSquareToDrawList(square1);
        R_Square* square2 = CreateSquare(frameArena, MakeVec3f(0.0f, 0.0f, 1.0f), MakeVec3f(0.0f, -1.0f, 0.0f));
        R_AddSquareToDrawList(square2);
        R_Square* square3 = CreateSquare(frameArena, MakeVec3f(1.0f, 0.0f, 1.0f), MakeVec3f(-1.0f, 0.0f, 0.0f));
        R_AddSquareToDrawList(square3);

        R_DrawSquare();

        ResetArena(frameArena);
    }

    return 0;
}

R_Square* CreateSquare(Arena* arena, Vec3f color, Vec3f centerPosition)
{
    R_Square* square = (R_Square*)AllocateArena(sizeof(R_Square));
    square->mvp.color = color;
    square->mvp.centerPosition = centerPosition;
    square->vertecies[0].position = MakeVec3f(-0.5f, -0.5f, 0.0f);
    square->vertecies[1].position = MakeVec3f(0.5f, -0.5f, 0.0f);
    square->vertecies[2].position = MakeVec3f(0.5f, 0.5f, 0.0f);
    square->vertecies[3].position = MakeVec3f(-0.5f, 0.5f, 0.0f);
    square->indecies[0] = 0;
    square->indecies[1] = 1;
    square->indecies[2] = 2;
    square->indecies[3] = 2;
    square->indecies[4] = 3;
    square->indecies[5] = 0;

    return square;
}
