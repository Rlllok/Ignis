#pragma comment(lib, "Winmm.lib")

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

struct Ball
{
    Vec2f size;
    Vec2f position;
    Vec2f velocity;
};

// ------------------------------------------------------------
// --AlNov: Functions Prototypes

void DrawPlayer(Arena* arena, Player player);
void DrawBall(Arena* arena, Ball ball);


int main()
{
    OS_Window window = OS_CreateWindow("Game", MakeVec2u(1280, 720));
    
    // --AlNov: change Windows schedular granuality
    // Should be added to win32 layer
    u32 desiredSchedularMS = 1;
    timeBeginPeriod(desiredSchedularMS);

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
// @TODO Coordinates is in pixel, so it can be better to change value to i32.
// And even better that that - change to world coordinates.
    Player player = {};
    player.rectSize.min = MakeVec2f(0.0f, 0.0f);
    player.rectSize.max = MakeVec2f(50.0f, 20.0f);
    player.position = MakeVec2f(300.0f, 300.0f);
    player.speed = 100.0f;

    Ball ball = {};
    ball.size.x = 30.0f;
    ball.size.y = 30.0f;
    ball.position.x = 100.0f;
    ball.position.y = 100.0f;
    ball.velocity.x = 0.0f;
    ball.velocity.y = 300.0;

    f32 timeSec = 0.0f;

    bool bLeftDown = false;
    bool bRightDown = false;
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
                            bLeftDown = currentEvent->isDown;
                        } break;

                        case OS_KEY_ARROW_RIGHT:
                        {
                            bRightDown = currentEvent->isDown;
                        } break;

                        default: break;
                    }

                    printf("Player Position: %f\n", player.position.x);
                } break;
                
                default: break;
            }

            currentEvent = currentEvent->next;
        }

        if (bLeftDown)
        {
            player.position.x += -player.speed * timeSec;
        }
        if (bRightDown)
        {
            player.position.x += player.speed * timeSec;
        }

        // --AlNov: @TODO There is bag when ball goes outside border and begins
        // straifing right-left
        if (ball.position.x + ball.size.x / 2.0f > 1280.0f
            || ball.position.x - ball.size.x / 2.0f < 0.0f)
        {
            ball.velocity.x *= -1;
        }
        if (ball.position.y + ball.size.y / 2.0f > 720.0f
            || ball.position.x - ball.size.y / 2.0f < 0.0f)
        {
            ball.velocity.y *= -1;
        }
        ball.position.x += ball.velocity.x * timeSec;
        ball.position.y += ball.velocity.y * timeSec;

        DrawPlayer(tmpArena, player);
        DrawBall(tmpArena, ball);

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

void DrawBall(Arena* arena, Ball ball)
{
    Rect2f box = {};
    box.x0 = ball.position.x - (ball.size.x / 2.0f);
    box.y0 = ball.position.y - (ball.size.y / 2.0f);
    box.x1 = ball.position.x + (ball.size.x / 2.0f);
    box.y1 = ball.position.y + (ball.size.y / 2.0f);

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