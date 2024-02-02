#include "../base/base_include.h"
#include "../os/os_include.h"
#include "../render/vulkan/r_init_vk.h"

#include "../base/base_include.cpp"
#include "../os/os_include.cpp"
#include "../render/vulkan/r_init_vk.cpp"

#include <cmath>

R_Mesh* CreateMesh(Arena* arena, Vec3f color, Vec3f centerPosition);

void R_DrawSquare(Arena* arena, Vec2f topLeft, Vec2f botRight, RGB color);

struct UI_State
{
    u32 mouseX;
    u32 mouseY;
    bool wasDown;
    bool isDown;
    bool isUp;

    u32 hotId;
    u32 activeId;
}
ui_state = {};

void UI_Prepare();
void UI_Reset();

bool UI_CheckBoxHit(Vec2f topLeft, Vec2f botRight);

bool UI_Button(Arena* arena, u32 id, Vec2f topLeft, Vec2f botRight, RGB color);

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

                case OS_EVENT_TYPE_MOUSE_MOVE:
                {
                    ui_state.mouseX = event->mouseX;
                    ui_state.mouseY = event->mouseY;
                    ui_state.isDown = 0;
                }
                case OS_EVENT_TYPE_MOUSE_RELEASE:
                {
                    ui_state.mouseX = event->mouseX;
                    ui_state.mouseY = event->mouseY;
                    ui_state.isDown = 0;
                } break;
                case OS_EVENT_TYPE_MOUSE_PRESS:
                {
                    ui_state.mouseX = event->mouseX;
                    ui_state.mouseY = event->mouseY;
                    ui_state.isDown = 1;
                }

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

        UI_Prepare();
        // printf("isDown: %i      X: %i       Y: %i\n", ui_state.isDown, ui_state.mouseX, ui_state.mouseY);
        // printf("isDown: %i      wasDown: %i\n", ui_state.isDown, ui_state.wasDown);

        // --AlNov: @NOTE @TODO Maximum number of meshes is 10. This is the number of Vulkan DescriptorSets
        Vec2f topLeft = MakeVec2f(500, 250);
        Vec2f botRight = MakeVec2f(600, 350);
        Vec3f buttonColor = MakeRGB(1.0f, 1.0f, 1.0f);
        if (UI_Button(frameArena, 0, topLeft, botRight, buttonColor))
        {
            printf("Button is active!!!\n");
        }

        R_DrawMesh();

        // --AlNov: Using sleep to take less CPU Time
        Sleep(3);

        UI_Reset();
        ResetArena(frameArena);
    }

    return 0;
}

R_Mesh* CreateMesh(Arena* arena, Vec3f color, Vec3f centerPosition)
{
    R_Mesh* mesh = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
    mesh->mvp.color = color;
    mesh->mvp.centerPosition = centerPosition;
    mesh->vertecies[0].position = MakeVec3f(-0.5f, -0.5f, 0.0f);
    mesh->vertecies[1].position = MakeVec3f(0.5f, -0.5f, 0.0f);
    mesh->vertecies[2].position = MakeVec3f(0.5f, 0.5f, 0.0f);
    mesh->vertecies[3].position = MakeVec3f(-0.5f, 0.5f, 0.0f);
    mesh->indecies[0] = 0;
    mesh->indecies[1] = 1;
    mesh->indecies[2] = 2;
    mesh->indecies[3] = 2;
    mesh->indecies[4] = 3;
    mesh->indecies[5] = 0;

    return mesh;
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
        if (ui_state.isDown)
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
        mesh->mvp.color = MakeVec3f(0.8f, 0.2f, 0.6f);
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

void UI_Prepare()
{
    ui_state.hotId = -1;
    ui_state.activeId = -1;
}

void UI_Reset()
{
}

bool UI_CheckBoxHit(Vec2f topLeft, Vec2f botRight)
{
    if (ui_state.mouseX < topLeft.x
        || ui_state.mouseY < topLeft.y
        || ui_state.mouseX >= (botRight.x)
        || ui_state.mouseY >= (botRight.y)
    )
    {
        return 0;
    }

    return 1;
}
