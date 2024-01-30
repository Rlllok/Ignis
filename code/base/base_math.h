#pragma once

union Vec2u
{
    struct
    {
        u32 x;
        u32 y;
    };

    struct
    {
        u32 width;
        u32 height;
    };

    u32 values[2];
};

union Vec3f
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    };

    struct
    {
        f32 r;
        f32 g;
        f32 b;
    };

    f32 values[3];
};

typedef Vec3f RGB;
#define MakeRGB(r, g, b) MakeVec3f(r, g, b)

union Vec4f
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };

    struct
    {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };

    f32 values[4];
};

Vec2u MakeVec2u(u32 x, u32 y);
Vec3f MakeVec3f(f32 x, f32 y, f32 z);
Vec4f MakeVec4f(f32 x, f32 y, f32 z, f32 w);