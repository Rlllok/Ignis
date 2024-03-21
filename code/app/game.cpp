#pragma comment(lib, "Winmm.lib")

#include "../base/base_include.h"
#include "../os/os_include.h"
#include "../render/vulkan/r_init_vk.h"

#include "../base/base_include.cpp"
#include "../os/os_include.cpp"
#include "../render/vulkan/r_init_vk.cpp"

#define function static

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
    Vec2f   size;
    Vec2f   position;
    f32     speed;
};

struct Ball
{
    Vec2f size;
    Vec2f position;
    Vec2f velocity;
};

struct Brick
{
    Brick* previous;
    Brick* next;

    Vec2f   size;
    Vec2f   position;
    RGB     color;
};

struct BrickList
{
    Brick* first;
    Brick* last;
    
    u32 brick_count;
};

// ------------------------------------------------------------
// --AlNov: Functions Prototypes

function void BrickListPush(BrickList* brick_list, Brick* brick);

function void DrawPlayer(Arena* arena, Player player);
function void DrawBall(Arena* arena, Ball ball);
function void DrawBricks(Arena* arena, BrickList brick_list);

function Rect2f Rect2fFromPlayer(const Player* player);
function Rect2f Rect2fFromBall(const Ball* ball);
function Rect2f Rect2fFromBrick(const Brick* brick);

function bool CheckBoxBoxCollision(Rect2f box1, Rect2f box2);
function bool CheckBallPlayerCollision(const Ball* ball, const Player* player);
function Brick* CheckBallBricksCollision(const Ball* ball, const BrickList brick_list);

int main()
{
    OS_Window window = OS_CreateWindow("Game", MakeVec2u(1280, 720));
    
    // --AlNov: change Windows schedular granuality
    // Should be added to win32 layer
    u32 desired_schedular_ms = 1;
    timeBeginPeriod(desired_schedular_ms);
    
    R_Init(window);
    
    OS_ShowWindow(&window);
    
    LARGE_INTEGER win32_freq;
    QueryPerformanceFrequency(&win32_freq);
    u64 frequency = win32_freq.QuadPart;
    
    LARGE_INTEGER win32_cycles;
    QueryPerformanceCounter(&win32_cycles);
    u64 start_cycles = win32_cycles.QuadPart;
    
    // ------------------------------------------------------------
    // --AlNov: Game Staff
    // @TODO Coordinates is in pixel, so it can be better to change value to i32.
    // And even better that that - change to world coordinates.
    Player player = {};
    player.position = MakeVec2f(600.0f, 650.0f);
    player.size = MakeVec2f(100.0f, 20.0f);
    player.speed = 250.0f;
    
    Ball ball = {};
    ball.size = MakeVec2f(30.0f, 30.0f);
    ball.position = MakeVec2f(600.0f, 600.0f);
    ball.velocity = MakeVec2f(0.0f, 200.0f);
    
    BrickList brick_list;
    const u32 bricks_count = 5;
    RGB default_brick_color = MakeRGB(0.4f, 0.8f, 1.0f);
    RGB hit_brick_color = MakeRGB(1.0f, 0.0f, 0.0f);
    Vec2f brick_size = MakeVec2f(60.0f, 20.0f);
    Vec2f first_brick_position = MakeVec2f(400.0f, 100.0f);
    f32 brick_padding_x = 5.0f;
    
    f32 time_sec = 0.0f;
    
    bool b_left_down = false;
    bool b_right_down = false;
    // END Game Staff
    
    Arena* tmp_arena = AllocateArena(Megabytes(64));

    // --AlNov: Init Bricks
    Arena* brick_arena = AllocateArena(Megabytes(4));
    for (u32 i = 0; i < bricks_count; i += 1)
    {
        Brick* brick = (Brick*)PushArena(brick_arena, sizeof(Brick));
        brick->size = brick_size;
        brick->position = first_brick_position;
        brick->position.x += i * (brick_size.x + brick_padding_x);
        brick->color = default_brick_color;
        BrickListPush(&brick_list, brick);
    }
    
    bool b_finished = false;
    while (!b_finished)
    {
        QueryPerformanceCounter(&win32_cycles);
        u64 end_cycles = win32_cycles.QuadPart;
        u64 cycles_delta = end_cycles - start_cycles;
        start_cycles = end_cycles;
        time_sec = (f32)cycles_delta / (f32)frequency;
        // printf("Time: %f\n", timeSec * 1000.0f);
        
        OS_EventList event_list = OS_GetEventList(tmp_arena);
        
        OS_Event* current_event = event_list.firstEvent;
        while (current_event)
        {
            switch (current_event->type)
            {
                case OS_EVENT_TYPE_EXIT:
                {
                    b_finished = true;
                } break;
                
                case OS_EVENT_TYPE_KEYBOARD:
                {
                    switch (current_event->key)
                    {
                        case OS_KEY_ARROW_LEFT:
                        {
                            b_left_down = current_event->isDown;
                        } break;
                        
                        case OS_KEY_ARROW_RIGHT:
                        {
                            b_right_down = current_event->isDown;
                        } break;
                        
                        default: break;
                    }
                } break;
                
                default: break;
            }
            
            current_event = current_event->next;
        }
        
        Vec2f new_player_position = player.position;
        if (b_left_down)
        {
            new_player_position.x += -player.speed * time_sec;
        }
        if (b_right_down)
        {
            new_player_position.x += player.speed * time_sec;
        }
        
        if (new_player_position.x + player.size.x / 2.0f < 1280.0f
            && new_player_position.x - player.size.x / 2.0f > 0.0f)
        {
            player.position = new_player_position;
        }
        
        // --AlNov: @TODO It can be to much to copy ball structure to check
        // collision with regards to the new ball position.
        // Should be better solution
        Ball ball_with_new_position = ball;
        Vec2f new_ball_position = AddVec2f(ball.position, MulVec2f(ball.velocity, time_sec));
        ball_with_new_position.position = new_ball_position;
        
        bool b_ball_player_collision = CheckBallPlayerCollision( &ball_with_new_position, &player);
        
        Brick* collided_brick = CheckBallBricksCollision(&ball_with_new_position, brick_list);
        
        if (new_ball_position.x + ball.size.x / 2.0f < 1280.0f
            && new_ball_position.x - ball.size.x / 2.0f > 0.0f
            && new_ball_position.y + ball.size.y / 2.0f < 720.0f
            && new_ball_position.y - ball.size.y / 2.0f > 0.0f
            && !b_ball_player_collision
            && collided_brick == 0)
        {
            ball.position = new_ball_position;
        }
        else
        {
            ball.velocity = MulVec2f(ball.velocity, -1);
            
            if (collided_brick != 0)
            {
                collided_brick->color = hit_brick_color;
            }
        }
        
        DrawPlayer(tmp_arena, player);
        DrawBall(tmp_arena, ball);
        DrawBricks(tmp_arena, brick_list);
        
        R_DrawMesh();
        
        // --AlNov: Locked to 60 fps
        f32 sleep_time = (1000.0f / 60.0f) - (time_sec * 1000.0f);
        Sleep((sleep_time > 0) ? sleep_time : 0);
        
        R_EndFrame();
        ResetArena(tmp_arena);
    }

    // --AlNov: There is no sense to reset and free brick arena for now.
    // The reason is that it is done at the end of the program.
    // But the logic is that bricks is bound to the level.
    // We cannot use frame arena for them (As I tried to do)
    // So we should have another arena with another life scope.
    ResetArena(brick_arena);
    FreeArena(brick_arena);
    
    return 0;
}


// ------------------------------------------------------------
// --AlNov: Functions' Implementation

function void BrickListPush(BrickList* brick_list, Brick* brick)
{
    if (brick_list->brick_count == 0)
    {
        brick_list->first = brick;
        brick_list->last = brick;
        brick_list->brick_count = 1;

        brick->previous = 0;
        brick->next = 0;
    }
    else
    {
        brick->previous = brick_list->last;
        brick->next = 0;
        brick_list->last->next = brick;
        brick_list->last = brick;
        brick_list->brick_count += 1;
    }
}

function void DrawPlayer(Arena* arena, Player player)
{
    Rect2f box = Rect2fFromPlayer(&player);
    
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

function void DrawBall(Arena* arena, Ball ball)
{
    Rect2f box = Rect2fFromBall(&ball);
    
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

function void DrawBricks(Arena* arena, BrickList brick_list)
{
    for (Brick* brick = brick_list.first; brick != 0; brick = brick->next)
    {
        Rect2f box = Rect2fFromBrick(brick);
        
        box.x0 = (box.x0 / 1280.0f) * 2 - 1;
        box.y0 = (box.y0 / 720.0f) * 2 - 1;
        box.x1 = (box.x1 / 1280.0f) * 2 - 1;
        box.y1 = (box.y1 / 720.0f) * 2 - 1;
        
        R_Mesh* mesh = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
        mesh->mvp.color = brick->color;
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
}

function Rect2f Rect2fFromPlayer(const Player* player)
{
    Rect2f box = {};
    box.x0 = player->position.x - player->size.x / 2.0f;
    box.y0 = player->position.y - player->size.y / 2.0f;
    box.x1 = player->position.x + player->size.x / 2.0f;
    box.y1 = player->position.y + player->size.y / 2.0f;
    
    return box;
}

function Rect2f Rect2fFromBall(const Ball* ball)
{
    Rect2f box = {};
    box.x0 = ball->position.x - ball->size.x / 2.0f;
    box.y0 = ball->position.y - ball->size.y / 2.0f;
    box.x1 = ball->position.x + ball->size.x / 2.0f;
    box.y1 = ball->position.y + ball->size.y / 2.0f;
    
    return box;
}

function Rect2f Rect2fFromBrick(const Brick* brick)
{
    Rect2f box = {};
    box.x0 = brick->position.x - brick->size.x / 2.0f;
    box.y0 = brick->position.y - brick->size.y / 2.0f;
    box.x1 = brick->position.x + brick->size.x / 2.0f;
    box.y1 = brick->position.y + brick->size.y / 2.0f;
    
    return box;
}

function bool CheckBoxBoxCollision(Rect2f box1, Rect2f box2)
{
    // AlNov: @TODO
    bool result = (box1.y0 > box2.y1) || (box1.y1 < box2.y0)
        || (box1.x0 > box2.x1) || (box1.x1 < box2.x0);
    
    return !result;
}

function bool CheckBallPlayerCollision(const Ball* ball, const Player* player)
{
    Rect2f ball_box   = Rect2fFromBall(ball);
    Rect2f player_box = Rect2fFromPlayer(player);
    
    return CheckBoxBoxCollision(ball_box, player_box);
}

function Brick* CheckBallBricksCollision(const Ball* ball, const BrickList brick_list)
{
    Brick* collided_brick = 0;
    
    for (Brick* brick = brick_list.first; brick != 0; brick = brick->next)
    {
        Rect2f ball_box = Rect2fFromBall(ball);
        Rect2f brick_box = Rect2fFromBrick(brick);
        
        bool b_is_colided = CheckBoxBoxCollision(ball_box, brick_box);
        if (b_is_colided)
        {
            collided_brick = brick;
            break;
        }
    }
    
    return collided_brick;
}
