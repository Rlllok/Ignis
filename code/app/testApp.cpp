#include "../base/base_include.h"
#include "../os/os_include.h"
#include "../render/vulkan/r_init_vk.h"

#include "../base/base_include.cpp"
#include "../os/os_include.cpp"
#include "../render/vulkan/r_init_vk.cpp"

#include <cmath>

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

        R_DrawSquare(MakeVec3f(0.3f, 0.1f, 0.0f), MakeVec3f(colorValue, 0.3f, 0.7f));

        ResetArena(frameArena);
    }

    return 0;
}
