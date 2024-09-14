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
        U32 x;
        U32 y;
    };

    struct
    {
        U32 width;
        U32 height;
    };

    U32 values[2];
};

union Vec2f
{
    struct
    {
        F32 x;
        F32 y;
    };

    F32 values[2];
};

// --AlNov: Vec3
union Vec3f
{
    struct
    {
        F32 x;
        F32 y;
        F32 z;
    };

    struct
    {
        F32 r;
        F32 g;
        F32 b;
    };

    F32 values[3];
};

union Vec4f
{
    struct
    {
        F32 x;
        F32 y;
        F32 z;
        F32 w;
    };

    struct
    {
        F32 r;
        F32 g;
        F32 b;
        F32 a;
    };

    F32 values[4];
};

// --AlNov: Vector type convertion -----------------------------------
#define Vec2uFromVec(v)  MakeVec2f((U32)v.x, (U32)v.y)
#define Vec2fFromVec(v)  MakeVec2f((F32)v.x, (F32)v.y)
#define Vec3fFromVec2(v) MakeVec3f((F32)v.x, (F32)v.y, 0.0f)

// -------------------------------------------------------------------
// --AlNov: Matrix Types ---------------------------------------------

// --AlNov: Mat3x3f
struct Mat3x3f
{
  F32 values[3][3];
};

// --AlNov: Mat4x4f
struct Mat4x4f
{
  F32 values[4][4];
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
    F32 x0;
    F32 y0;
    F32 x1;
    F32 y1;
  };

  Vec2f value[2];
};

// -------------------------------------------------------------------
// --AlNov: Vector Operations ----------------------------------------

// --AlNov: Vec2 
func Vec2u MakeVec2u(U32 x, U32 y);

func Vec2f MakeVec2f(F32 x, F32 y);
func Vec2f AddVec2f(Vec2f a, Vec2f b);
func Vec2f SubVec2f(Vec2f a, Vec2f b);
func Vec2f MulVec2f(Vec2f a, F32 num);
func F32   DotVec2f(Vec2f a, Vec2f b);
func F32   CrossVec2f(Vec2f a, Vec2f b);
func Vec2f RotateVec2f(Vec2f v, F32 radians);
func F32   MagnitudeSquareVec2f(Vec2f v);
func F32   MagnitudeVec2f(Vec2f v);
func Vec2f NormalizeVec2f(Vec2f v);
func Vec2f NormalToVec2f(Vec2f v);

// --AlNov: Vec3
func Vec3f MakeVec3f(F32 x, F32 y, F32 z);
func Vec3f MulVec3f(Vec3f a, F32 num);
func Vec3f TransformVec3f(Vec3f v, Mat3x3f m);
func F32   DotVec3f(Vec3f a, Vec3f b);
func F32   MagnitudeVec3f(Vec3f v);
func Vec3f NormalizeVec3f(Vec3f v);

// --AlNov: Vec4
func Vec4f MakeVec4f(F32 x, F32 y, F32 z, F32 w);

// -------------------------------------------------------------------
// --AlNov: Matrix Operations ----------------------------------------

// --AlNov: Mat3x3
func Mat3x3f Make3x3f(F32 diagonal_value);
func Mat3x3f Mul3x3f(Mat3x3f A, Mat3x3f B);
func Mat3x3f Transpose3x3f(Mat3x3f m);

// --AlNov: Mat4x4
func Mat4x4f Make4x4f(F32 diagonal_value);
func Mat4x4f MakeOrthographic4x4f(F32 left, F32 right, F32 bottom, F32 top, F32 near_z, F32 far_z);
func Mat4x4f MakePerspective4x4f(F32 fov, F32 aspect_ration, F32 near_z, F32 far_z);
func Mat4x4f Transpose4x4f(Vec3f v);
