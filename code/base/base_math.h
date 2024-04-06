#pragma once

// --AlNov: STD ------------------------------------------------------
#include "stdio.h"
#include "math.h"

// -------------------------------------------------------------------
// --AlNov: Vectors
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

union Vec2f
{
    struct
    {
        f32 x;
        f32 y;
    };

    f32 values[2];
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
Vec2f MakeVec2f(f32 x, f32 y);
Vec3f MakeVec3f(f32 x, f32 y, f32 z);
Vec4f MakeVec4f(f32 x, f32 y, f32 z, f32 w);

// ------------------------------------------------------------
// --AlNov: Vector Math
Vec2f AddVec2f(Vec2f a, Vec2f b);
Vec2f MulVec2f(Vec2f a, f32 num);

f32   MagnitudeSquareVec2f(Vec2f v);
f32   MagnitudeVec2f(Vec2f v);
Vec2f NormalizeVec2f(Vec2f v);

// -------------------------------------------------------------------
// --AlNov: Rectangle
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

