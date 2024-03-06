#include "../base/base_include.h"
#include "../os/os_include.h"
#include "../render/vulkan/r_init_vk.h"

#include "../base/base_include.cpp"
#include "../os/os_include.cpp"
#include "../render/vulkan/r_init_vk.cpp"


// ------------------------------------------------------------
// --AlNov: Data

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

struct Player
{
    Rect2f rectSize;
    Vec2f position;
    f32 speed;
};

// ------------------------------------------------------------
// --AlNov: Functions Prototypes

void DrawPlayer(Arena* arena, Player player);


int main()
{
    OS_Window window = OS_CreateWindow("Game", MakeVec2u(1280, 720));

    R_Init(window);

    OS_ShowWindow(&window);

    LARGE_INTEGER win32Freq;
    QueryPerformanceFrequency(&win32Freq);
    u64 frequency = win32Freq.QuadPart;

    LARGE_INTEGER win32Cycles;
    QueryPerformanceCounter(&win32Cycles);
    u64 startCycles = win32Cycles.QuadPart;

// ------------------------------------------------------------
// --AlNov: Game Staff
    Player player = {};
    player.rectSize.min = MakeVec2f(0.0f, 0.0f);
    player.rectSize.max = MakeVec2f(50.0f, 20.0f);
    player.position = MakeVec2f(300.0f, 300.0f);
    player.speed = 100.0f;

    f32 timeSec = 0.0f;
// END Game Staff

    Arena* tmpArena = AllocateArena(Megabytes(64));
    
    bool bFinished = false;
    while (!bFinished)
    {
        QueryPerformanceCounter(&win32Cycles);
        u64 endCycles = win32Cycles.QuadPart;
        u64 cyclesDelta = endCycles - startCycles;
        startCycles = endCycles;
        timeSec = (f32)cyclesDelta / (f32)frequency;
        printf("Time: %f\n", timeSec * 1000.0f);

        OS_EventList eventList = OS_GetEventList(tmpArena);

        OS_Event* currentEvent = eventList.firstEvent;
        while (currentEvent)
        {
            switch (currentEvent->type)
            {
                case OS_EVENT_TYPE_EXIT:
                {
                    bFinished = true;
                } break;

                case OS_EVENT_TYPE_KEYBOARD:
                {
                    switch (currentEvent->key)
                    {
                        case OS_KEY_ARROW_LEFT:
                        {
                            player.speed *= player.speed > 0.0f ? 1 : -1;
                        } break;

                        case OS_KEY_ARROW_RIGHT:
                        {
                            player.speed *= player.speed < 0.0f ? 1 : -1;
                        } break;

                        default: break;
                    }

                    printf("Player Position: %f\n", player.position.x);
                } break;
                
                default: break;
            }

            currentEvent = currentEvent->next;
        }

        DrawPlayer(tmpArena, player);
        player.position.x += -player.speed * timeSec;

        R_DrawMesh();

        // --AlNov: Locked to 60 fps
        f32 sleepTime = (1000.0f / 60.0f) - (timeSec * 1000.0f);
        Sleep((sleepTime > 0) ? sleepTime : 0);

        R_EndFrame();
        ResetArena(tmpArena);
    }

    return 0;
}


// ------------------------------------------------------------
// --AlNov: Functions Prototypes

void DrawPlayer(Arena* arena, Player player)
{
    Rect2f box = player.rectSize;
    box.x0 += player.position.x;
    box.x1 += player.position.x;
    box.y0 += player.position.y;
    box.y1 += player.position.y;

    box.x0 = (box.x0 / 1280.0f) * 2 - 1;
    box.y0 = (box.y0 / 720.0f) * 2 - 1;
    box.x1 = (box.x1 / 1280.0f) * 2 - 1;
    box.y1 = (box.y1 / 720.0f) * 2 - 1;

    R_Mesh* mesh = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
    mesh->mvp.color = MakeRGB(1.0f, 1.0f, 1.0f);
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
