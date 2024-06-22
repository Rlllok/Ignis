#pragma once

#include "base_core.h"

// --AlNov: STD ------------------------------------------------------
#include "stdio.h"
#include "math.h"

// --AlNov: Constants
#define PI 3.141592654f

// -------------------------------------------------------------------
// --AlNov: Vector Types ---------------------------------------------

// --AlNov: Vec2
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

// --AlNov: Vec3
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

// --AlNov: Vector type convertion -----------------------------------
#define Vec2uFromVec(v)  MakeVec2f((u32)v.x, (u32)v.y)
#define Vec2fFromVec(v)  MakeVec2f((f32)v.x, (f32)v.y)
#define Vec3fFromVec2(v) MakeVec3f((f32)v.x, (f32)v.y, 0.0f)

// -------------------------------------------------------------------
// --AlNov: Matrix Types ---------------------------------------------

// --AlNov: Mat3x3f
struct Mat3x3f
{
  f32 values[3][3];
};

// --AlNov: Mat4x4f
struct Mat4x4f
{
  f32 values[4][4];
};


// -------------------------------------------------------------------
// --AlNov: Rectangle Type -------------------------------------------
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

// -------------------------------------------------------------------
// --AlNov: Vector Operations ----------------------------------------

// --AlNov: Vec2 
func Vec2u MakeVec2u(u32 x, u32 y);

func Vec2f MakeVec2f(f32 x, f32 y);
func Vec2f AddVec2f(Vec2f a, Vec2f b);
func Vec2f SubVec2f(Vec2f a, Vec2f b);
func Vec2f MulVec2f(Vec2f a, f32 num);
func f32   DotVec2f(Vec2f a, Vec2f b);
func f32   CrossVec2f(Vec2f a, Vec2f b);
func Vec2f RotateVec2f(Vec2f v, f32 radians);
func f32   MagnitudeSquareVec2f(Vec2f v);
func f32   MagnitudeVec2f(Vec2f v);
func Vec2f NormalizeVec2f(Vec2f v);
func Vec2f NormalToVec2f(Vec2f v);

// --AlNov: Vec3
func Vec3f MakeVec3f(f32 x, f32 y, f32 z);
func Vec3f MulVec3f(Vec3f a, f32 num);
func Vec3f TransformVec3f(Vec3f v, Mat3x3f m);
func f32   DotVec3f(Vec3f a, Vec3f b);
func f32   MagnitudeVec3f(Vec3f v);
func Vec3f NormalizeVec3f(Vec3f v);

// --AlNov: Vec4
func Vec4f MakeVec4f(f32 x, f32 y, f32 z, f32 w);

// -------------------------------------------------------------------
// --AlNov: Matrix Operations ----------------------------------------

// --AlNov: Mat3x3
func Mat3x3f Make3x3f(f32 diagonal_value);
func Mat3x3f Mul3x3f(Mat3x3f A, Mat3x3f B);
func Mat3x3f Transpose3x3f(Mat3x3f m);

// --AlNov: Mat4x4
func Mat4x4f Make4x4f(f32 diagonal_value);
func Mat4x4f MakeOrthographic4x4f(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z);
func Mat4x4f MakePerspective4x4f(f32 fov, f32 aspect_ration, f32 near_z, f32 far_z);
func Mat4x4f Transpose4x4f(Vec3f v);
