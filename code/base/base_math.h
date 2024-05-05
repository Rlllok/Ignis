#pragma once

#include "base_core.h"

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

// -------------------------------------------------------------------
// --AlNov: Matrices
// --AlNov: @TODO Move all type to top of the file. And functions after
struct Mat3x3f32
{
  f32 values[3][3];
};

struct Mat4x4f32
{
  f32 values[4][4];
};

struct Mat6x6f
{
  f32 values[6][6];
};

Vec2u MakeVec2u(u32 x, u32 y);
Vec2f MakeVec2f(f32 x, f32 y);
Vec3f MakeVec3f(f32 x, f32 y, f32 z);
Vec4f MakeVec4f(f32 x, f32 y, f32 z, f32 w);

func Vec2f Vec2fFromVec2u(Vec2u v) { return MakeVec2f(v.x, v.y); }
 
Vec3f Vec3fFromVec2f(Vec2f v);

// ------------------------------------------------------------
// --AlNov: Vector Math
Vec2f AddVec2f(Vec2f a, Vec2f b);
func Vec2f SubVec2f(Vec2f a, Vec2f b);
Vec2f MulVec2f(Vec2f a, f32 num);
Vec3f MulVec3f(Vec3f a, f32 num);
func Vec3f TransformVec3f(Vec3f v, Mat3x3f32 m);
func f32 DotVec2f(Vec2f a, Vec2f b);
func f32 DotVec3f(Vec3f a, Vec3f b);
func f32 CrossVec2f(Vec2f a, Vec2f b);
Vec2f RotateVec2f(Vec2f v, f32 radians);

f32   MagnitudeSquareVec2f(Vec2f v);
f32   MagnitudeVec2f(Vec2f v);
Vec2f NormalizeVec2f(Vec2f v);

func Vec2f NormalToVec2f(Vec2f v);


func Mat3x3f32 Make3x3f32(f32 diagonal_value);
func Mat4x4f32 Make4x4f32(f32 diagonal_value);
func Mat4x4f32 MakeOrthographic4x4f32(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z);

func Mat3x3f32 Mul3x3f32(Mat3x3f32 A, Mat3x3f32 B);
func Mat3x3f32 Transpose3x3f32(Mat3x3f32 m);

func Vec3f Solve3x3f32(Mat3x3f32 m, Vec3f v);

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

