#include "../base/base_include.h"
#include "../os/os_include.h"
#include "../render/vulkan/r_init_vk.h"

#include "../base/base_include.cpp"
#include "../os/os_include.cpp"
#include "../render/vulkan/r_init_vk.cpp"

#include <cmath>
#include <stdlib.h>

#define UI_ID (__LINE__)

void R_DrawSquare(Arena* arena, Vec2f topLeft, Vec2f botRight, RGB color);

struct UI_State
{
    Vec2f mousePosition;
    bool bRelesed;

    u32 hotId;
    u32 activeId;
}
ui_state = {};

void UI_Prepare();
void UI_Reset();

bool UI_CheckBoxHit(Vec2f topLeft, Vec2f botRight);

bool UI_Button(Arena* arena, u32 id, Vec2f topLeft, Vec2f botRight, RGB color);
void UI_Slider(Arena* arena, u32 id, Vec2f topLeft, Vec2f botRight, f32& value);

int main()
{
    Vec2u windowExtent = MakeVec2u(1280, 720);

    OS_Window window = OS_CreateWindow("Test App", windowExtent);

    R_Init(window);

    OS_ShowWindow(&window);

    LARGE_INTEGER win32Freq;
    QueryPerformanceFrequency(&win32Freq);
    u64 frequency = win32Freq.QuadPart;

    LARGE_INTEGER win32Cycles;
    QueryPerformanceCounter(&win32Cycles);
    u64 startCycles = win32Cycles.QuadPart;

    // --AlNov: @TODO @NOTE
    // If there is more information than 10 KB Arena drops error
    Arena* frameArena = AllocateArena(Kilobytes(10));
    u16 bIsFinished = false;
    while (!bIsFinished)
    {
        QueryPerformanceCounter(&win32Cycles);
        u64 endCycles = win32Cycles.QuadPart;
        u64 cyclesDelta = endCycles - startCycles;
        startCycles = endCycles;
        f32 ms = (1000.0f) * (f32)cyclesDelta / (f32)frequency;
        u32 fps = frequency / cyclesDelta;
        // printf("MS: %fms    FPS: %i\n", ms, fps);

        // AlNov: @TODO Cannot understand why OS_EventList working.
        // Not OS_EventList*. I think this is because we are using arena that created there,
        // and pushed to function. Should study more about this case
        OS_EventList eventList = OS_GetEventList(frameArena);
        
        OS_Event* event = eventList.firstEvent;
        while (event)
        {
            switch (event->type)
            {
                case OS_EVENT_TYPE_EXIT:
                {
                    bIsFinished = true;
                } break;
                case OS_EVENT_TYPE_MOUSE_RELEASE:
                {
                    ui_state.bRelesed = true;
                } break;

                default:
                {
                } break;
            }

            event = event->next;
        }

        static f32 t = 0.0f;
        t += 0.001f * ms;
        f32 sinValue = sinf(t);
        sinValue = (sinValue + 1.0f) / 2.0f;

        ui_state.mousePosition = OS_MousePosition(window);
        UI_Prepare();

        // --AlNov: @NOTE @TODO Maximum number of meshes is 10. This is the number of Vulkan DescriptorSets

        // Button 1
        {
            localPersist Vec2f topLeft = MakeVec2f(200, 250);
            localPersist Vec2f botRight = MakeVec2f(300, 350);
            Vec3f buttonColor = MakeRGB(0.0f, 1.0f, 0.0f);
            if (UI_Button(frameArena, UI_ID, topLeft, botRight, buttonColor))
            {
                u32 maxX = window.width - 200;
                u32 minX = 200;
                f32 randomX = (f32)(rand() % (maxX - minX + 1) + minX);
                u32 maxY = window.height - 100;
                u32 minY = 100;
                f32 randomY = (f32)(rand() % (maxY - minY + 1) + minY);

                topLeft.x = randomX;
                botRight.x = topLeft.x + 100;
                topLeft.y = randomY;
                botRight.y = topLeft.y + 100;
            }
        }
        // Button2
        {
            localPersist Vec2f topLeft = MakeVec2f(400, 150);
            localPersist Vec2f botRight = MakeVec2f(500, 250);
            Vec3f buttonColor = MakeRGB(1.0f, 0.0f, 0.0f);
            if (UI_Button(frameArena, UI_ID, topLeft, botRight, buttonColor))
            {
                u32 maxX = window.width - 200;
                u32 minX = 200;
                f32 randomX = (f32)(rand() % (maxX - minX + 1) + minX);
                u32 maxY = window.height - 100;
                u32 minY = 100;
                f32 randomY = (f32)(rand() % (maxY - minY + 1) + minY);

                topLeft.x = randomX;
                botRight.x = topLeft.x + 100;
                topLeft.y = randomY;
                botRight.y = topLeft.y + 100;
            }

        }
        {
            Vec2f topLeft = MakeVec2f(100, 50);
            Vec2f botRight = MakeVec2f(300, 70);
            localPersist f32 sliderValue = 0;
            UI_Slider(frameArena, UI_ID, topLeft, botRight, sliderValue);
        }

        R_DrawMesh();

        // --AlNov: Using sleep to take less CPU Time
        Sleep(3);

        UI_Reset();
        R_EndFrame();
        ResetArena(frameArena);
    }

    return 0;
}

void R_DrawSquare(Arena* arena, Vec2f topLeft, Vec2f botRight, RGB color)
{
    topLeft.x = (topLeft.x / 1280.0f) * 2 - 1;
    topLeft.y = (topLeft.y / 720.0f) * 2 - 1;
    botRight.x = (botRight.x / 1280.0f) * 2 - 1;
    botRight.y = (botRight.y / 720.0f) * 2 - 1;

    R_Mesh* mesh = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
    mesh->mvp.color = color;
    mesh->mvp.centerPosition = MakeVec3f(0.0f, 0.0f, 0.0f);
    mesh->vertecies[0].position = MakeVec3f(topLeft.x, topLeft.y, 0.0f);
    mesh->vertecies[1].position = MakeVec3f(botRight.x, topLeft.y, 0.0f);
    mesh->vertecies[2].position = MakeVec3f(botRight.x, botRight.y, 0.0f);
    mesh->vertecies[3].position = MakeVec3f(topLeft.x, botRight.y, 0.0f);
    mesh->indecies[0] = 0;
    mesh->indecies[1] = 1;
    mesh->indecies[2] = 2;
    mesh->indecies[3] = 2;
    mesh->indecies[4] = 3;
    mesh->indecies[5] = 0;

    R_AddMeshToDrawList(mesh);
}

bool UI_Button(Arena* arena, u32 id, Vec2f topLeft, Vec2f botRight, RGB color)
{
    R_Mesh* mesh = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
    if (UI_CheckBoxHit(topLeft, botRight))
    {
        ui_state.hotId = id;
        if (ui_state.bRelesed)
        {
            ui_state.activeId = id;
        }
    }
    topLeft.x = (topLeft.x / 1280.0f) * 2 - 1;
    topLeft.y = (topLeft.y / 720.0f) * 2 - 1;
    botRight.x = (botRight.x / 1280.0f) * 2 - 1;
    botRight.y = (botRight.y / 720.0f) * 2 - 1;
    mesh->mvp.centerPosition = MakeVec3f(0.0f, 0.0f, 0.0f);
    if (ui_state.hotId == id)
    {
        // --AlNov: Inver color when hover the button
        mesh->mvp.color = MakeVec3f(1.0f - color.r, 1.0f - color.g, 1.0f - color.b);
    }
    else
    {
        mesh->mvp.color = color;
    }
    mesh->vertecies[0].position = MakeVec3f(topLeft.x, topLeft.y, 0.0f);
    mesh->vertecies[1].position = MakeVec3f(botRight.x, topLeft.y, 0.0f);
    mesh->vertecies[2].position = MakeVec3f(botRight.x, botRight.y, 0.0f);
    mesh->vertecies[3].position = MakeVec3f(topLeft.x, botRight.y, 0.0f);
    mesh->indecies[0] = 0;
    mesh->indecies[1] = 1;
    mesh->indecies[2] = 2;
    mesh->indecies[3] = 2;
    mesh->indecies[4] = 3;
    mesh->indecies[5] = 0;

    R_AddMeshToDrawList(mesh);

    if (ui_state.activeId == id)
    {
        return true;
    }

    return false;
}

void UI_Slider(Arena* arena, u32 id, Vec2f topLeft, Vec2f botRight, f32& value)
{
    RGB backgroundColor = MakeRGB(0.4f, 0.4f, 0.4f);
    if (UI_CheckBoxHit(topLeft, botRight))
    {
        ui_state.hotId = id;
        // --AlNov: @TODO Maybe change to Hold
        if (ui_state.bRelesed)
        {
            ui_state.activeId = id;
        }
    }
    if (ui_state.hotId == id)
    {
        backgroundColor = MakeRGB(0.6f, 0.6f, 0.6f);
    }
    if (ui_state.activeId == id)
    {
        if (ui_state.mousePosition.x < topLeft.x)
        {
            value = 0.0f;
        }
        else if (ui_state.mousePosition.x > botRight.x)
        {
            value = 1.0f;
        }
        else
        {
            value = (ui_state.mousePosition.x - topLeft.x) / (botRight.x - topLeft.x);
            printf("value: %f\n", value);
        }
    }

    R_DrawSquare(arena, topLeft, botRight, backgroundColor);
    Vec2f sliderTopLeft = topLeft;
    sliderTopLeft.x += (botRight.x - topLeft.x) * value;
    Vec2f sliderBotRight = sliderTopLeft;
    sliderBotRight.x += 10;
    sliderBotRight.y =  botRight.y;
    R_DrawSquare(arena, sliderTopLeft, sliderBotRight, MakeRGB(0.2f, 0.4f, 0.7f));
}

void UI_Prepare()
{
    ui_state.hotId = -1;
    ui_state.activeId = -1;
}

void UI_Reset()
{
    ui_state = {};
}

bool UI_CheckBoxHit(Vec2f topLeft, Vec2f botRight)
{
    if (ui_state.mousePosition.x < topLeft.x
        || ui_state.mousePosition.y < topLeft.y
        || ui_state.mousePosition.x >= (botRight.x)
        || ui_state.mousePosition.y >= (botRight.y)
    )
    {
        return 0;
    }

    return 1;
}
