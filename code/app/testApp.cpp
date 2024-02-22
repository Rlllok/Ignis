#include "../base/base_include.h"
#include "../os/os_include.h"
#include "../render/vulkan/r_init_vk.h"

#include "../base/base_include.cpp"
#include "../os/os_include.cpp"
#include "../render/vulkan/r_init_vk.cpp"

#include <cmath>
#include <stdlib.h>
#include <strsafe.h>

#define UI_ID (__LINE__)

union Rect2f
{
    struct
    {
        Vec2f min;
        Vec2f max;
    };

    struct
    {
        f32 x0;
        f32 y0;
        f32 x1;
        f32 y1;
    };

    Vec2f value[2];
};

struct UI_Widget
{
    u32 id;

    UI_Widget* first;
    UI_Widget* next;
    u32 childCount;

    Rect2f rectangle;
    RGB color;
};

struct UI_State
{
    Vec2f mousePosition;
    bool bMouseRelesed;
    bool bMousePressed;

    u32 hotId;
    u32 activeId;
}
ui_state = {};

void UI_Prepare();
void UI_Reset();

bool UI_CheckBoxHit(Vec2f topLeft, Vec2f botRight);

UI_Widget   UI_MakeLayout(Arena* arena, u32 id, Rect2f box, RGB color);
bool        UI_Button(Arena* arena, u32 id, Vec2f topLeft, Vec2f botRight, RGB color);
bool        UI_Button(Arena* arena, u32 id, UI_Widget* parent, RGB color);
void        UI_Slider(Arena* arena, u32 id, Vec2f topLeft, Vec2f botRight, f32& value);
void        UI_Draw(Arena* arena, UI_Widget* parent);

void R_DrawSquare(Arena* arena, Vec2f topLeft, Vec2f botRight, RGB color);
void R_DrawSquare(Arena* arena, Rect2f box, RGB color);

int main()
{
    Vec2u windowExtent = MakeVec2u(1280, 720);

    OS_Window window = OS_CreateWindow("Test App", windowExtent);

    R_Init(window);
    
    f32 monitorHZ = OS_GetMonitorHZ();
    f32 targetMiliseconds = 1000.0f / monitorHZ;
    printf("ms: %f\n", targetMiliseconds);
    // END

    LARGE_INTEGER win32Freq;
    QueryPerformanceFrequency(&win32Freq);
    u64 frequency = win32Freq.QuadPart;

    LARGE_INTEGER win32Cycles;
    QueryPerformanceCounter(&win32Cycles);
    u64 startCycles = win32Cycles.QuadPart;

    Arena* frameArena = AllocateArena(Megabytes(50));

    OS_ShowWindow(&window);

    u16 bIsFinished = false;
    while (!bIsFinished)
    {
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
                case OS_EVENT_TYPE_RESIZE:
                {
                    window.width = event->windowSize.width;
                    window.height = event->windowSize.height;
                    R_ResizeWindow();
                } break;
                case OS_EVENT_TYPE_MOUSE_PRESS:
                {
                    ui_state.bMousePressed = true;
                    ui_state.bMouseRelesed = false;
                } break;
                case OS_EVENT_TYPE_MOUSE_RELEASE:
                {
                    ui_state.bMouseRelesed = true;
                    ui_state.bMousePressed = false;
                } break;

                default:
                {
                } break;
            }

            event = event->next;
        }

        ui_state.mousePosition = OS_MousePosition(window);
        UI_Prepare();

        // --AlNov: @NOTE @TODO Maximum number of meshes is 10. This is the number of Vulkan DescriptorSets
        localPersist Vec2f topLeft = MakeVec2f(0, 0);
        localPersist Vec2f botRight = MakeVec2f(300, 350);
        Rect2f box = {};
        box.min = topLeft;
        box.max = botRight;
        Vec3f buttonColor = MakeRGB(0.3f, 0.3f, 0.3f);
        UI_Widget layout = UI_MakeLayout(frameArena, UI_ID, box, buttonColor);
        {
            if (UI_Button(frameArena, UI_ID, &layout, MakeRGB(1.0f, 0.0f, 0.0f)))
            {
                printf("Hello, Red Button!\n");
            }

            if (UI_Button(frameArena, UI_ID, &layout, MakeRGB(0.0f, 0.0f, 1.0f)))
            {
                printf("Hello, Blue Button!\n");
            }
        }
        UI_Draw(frameArena, &layout);

        // Slider
        // {
        //     Vec2f topLeft = MakeVec2f(100, 50);
        //     Vec2f botRight = MakeVec2f(300, 70);
        //     localPersist f32 sliderValue = 0;
        //     UI_Slider(frameArena, UI_ID, topLeft, botRight, sliderValue);
        // }

        R_DrawMesh();

        // --AlNov: Using sleep to take less CPU Time
        QueryPerformanceCounter(&win32Cycles);
        u64 endCycles = win32Cycles.QuadPart;
        u64 cyclesDelta = endCycles - startCycles;
        startCycles = endCycles;
        f32 ms = (1000.0f) * (f32)cyclesDelta / (f32)frequency;
        f32 sleepTime = targetMiliseconds - ms;
        Sleep((sleepTime > 0) ? sleepTime : 0);

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

void R_DrawSquare(Arena* arena, Rect2f box, RGB color)
{
    box.x0 = (box.x0 / 1280.0f) * 2 - 1;
    box.y0 = (box.y0 / 720.0f) * 2 - 1;
    box.x1 = (box.x1 / 1280.0f) * 2 - 1;
    box.y1 = (box.y1 / 720.0f) * 2 - 1;

    R_Mesh* mesh = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
    mesh->mvp.color = color;
    mesh->mvp.centerPosition = MakeVec3f(0.0f, 0.0f, 0.0f);
    mesh->vertecies[0].position = MakeVec3f(box.x0, box.y0, 0.0f);
    mesh->vertecies[1].position = MakeVec3f(box.x1, box.y0, 0.0f);
    mesh->vertecies[2].position = MakeVec3f(box.x1, box.y1, 0.0f);
    mesh->vertecies[3].position = MakeVec3f(box.x0, box.y1, 0.0f);
    mesh->indecies[0] = 0;
    mesh->indecies[1] = 1;
    mesh->indecies[2] = 2;
    mesh->indecies[3] = 2;
    mesh->indecies[4] = 3;
    mesh->indecies[5] = 0;

    R_AddMeshToDrawList(mesh);
}

UI_Widget UI_MakeLayout(Arena* arena, u32 id, Rect2f box, RGB color)
{
    UI_Widget widget = {};
    widget.id = id;
    widget.rectangle = box;
    widget.color = color;
    widget.childCount = 0;
    widget.first = 0;
    widget.next = 0;

    return widget;
}

bool UI_Button(Arena* arena, u32 id, Vec2f topLeft, Vec2f botRight, RGB color)
{
    R_Mesh* mesh = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
    if (UI_CheckBoxHit(topLeft, botRight))
    {
        ui_state.hotId = id;
        if (ui_state.bMouseRelesed)
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

bool UI_Button(Arena* arena, u32 id, UI_Widget* parent, RGB color)
{
    Rect2f box = {};
    box.x0 = parent->rectangle.x0 + 10;
    box.y0 = parent->rectangle.y0 + 10 + (50 * parent->childCount);
    box.x1 = parent->rectangle.x1 - 10;
    box.y1 = box.y0 + 30;

    if (UI_CheckBoxHit(box.min, box.max))
    {
        ui_state.hotId = id;
        if (ui_state.bMouseRelesed)
        {
            ui_state.activeId = id;
        }
    }

    UI_Widget* button = (UI_Widget*)PushArena(arena, sizeof(UI_Widget));
    button->id = id;
    button->first = 0;
    button->next = 0;
    button->childCount = 0;
    button->rectangle = box;
    if (ui_state.hotId == id)
    {
        button->color = MakeRGB(1.0f - color.r, 1.0f - color.g, 1.0f - color.b);
    }
    else
    {
        button->color = color;
    }

    if (parent->first)
    {
        UI_Widget* head = parent->first;
        while (head->next)
        {
            head = head->next;
        }

        head->next = button;
        ++parent->childCount;
    }
    else
    {
        parent->first = button;
        parent->childCount = 1;
    }

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
        if (ui_state.bMousePressed)
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

void UI_Draw(Arena* arena, UI_Widget* parent)
{
    R_DrawSquare(arena, parent->rectangle, parent->color);

    if (parent->first)
    {
        UI_Widget* head = parent->first;
        while (head)
        {
            R_DrawSquare(arena, head->rectangle, head->color);

            head = head->next;
        }
    }
}

void UI_Prepare()
{
    ui_state.hotId = -1;
    ui_state.activeId = -1;
}

void UI_Reset()
{
    // ui_state = {};
    ui_state.bMouseRelesed = false;
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
