#pragma once

#include "base_core.h"

// --AlNov: STD ------------------------------------------------------
#include "stdio.h"
#include "math.h"

// --AlNov: Constants
#define PI 3.141592654f

// -------------------------------------------------------------------
// --AlNov: Math Defines (Min, Max ...)
#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Clamp(v, low, high) Max(Min(v, high), low)

// -------------------------------------------------------------------
// Vectors and Matrices
typedef union Vec2I32 Vec2I32;
union Vec2I32
{
  struct
  {
    I32 x;
    I32 y;
  };
  I32 values[2];
};

#define ZeroVec2I() {0,0}
#define OneVec2I() 	{1,1}

func Vec2I32 MakeVec2I32(I32 a, I32 b);
func Vec2I32 AddVec2I32(Vec2I32 a, Vec2I32 b);
func Vec2I32 SubVec2I32(Vec2I32 a, Vec2I32 b);

typedef union Vec2U32 Vec2U32;
union Vec2U32
{
  struct
  {
    U32 x;
    U32 y;
  };

	struct
	{
		U32 w;
		U32 h;
	};
  U32 values[2];
};

func Vec2U32 MakeVec2U32(U32 a, U32 b);
func Vec2U32 AddVec2U32(Vec2U32 a, Vec2U32 b);
func Vec2U32 SubVec2U32(Vec2U32 a, Vec2U32 b);

typedef union Vec2F32 Vec2F32;
union Vec2F32
{
  struct
  {
    F32 x;
    F32 y;
  };

  struct
  {
    F32 u;
    F32 v;
  };

  F32 values[2];
};

func Vec2F32 MakeVec2F32(F32 x, F32 y);
func Vec2F32 AddVec2F32(Vec2F32 a, Vec2F32 b);
func Vec2F32 SubVec2F32(Vec2F32 a, Vec2F32 b);
func Vec2F32 MulVec2F32(Vec2F32 a, Vec2F32 b);
func Vec2F32 DivVec2F32(Vec2F32 a, Vec2F32 b);
func Vec2F32 ScaleVec2F32(Vec2F32 v, F32 n);
func F32 DotVec2F32(Vec2F32 a, Vec2F32 b);
func F32 CrossVec2F32(Vec2F32 a, Vec2F32 b);
func F32 MagnitudeSquareVec2F32(Vec2F32 v);
func F32 MagnitudeVec2F32(Vec2F32 v);
func Vec2F32 NormalizeVec2F32(Vec2F32 v);
func Vec2F32 GetNormalToVec2F32(Vec2F32 v);

typedef struct Mat3F32 Mat3F32;
struct Mat3F32
{
  F32 values[3][3];
};

func Mat3F32 MakeMat3F32(F32 diagonal_value);
func Mat3F32 MulMat3F32(Mat3F32 a, Mat3F32 b);
func Mat3F32 MakeTransposeMat3F32(Mat3F32 m);

typedef union Vec3F32 Vec3F32;
union Vec3F32
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

func Vec3F32 MakeVec3F32(F32 x, F32 y, F32 z);
func Vec3F32 AddVec3F32(Vec3F32 a, Vec3F32 b);
func Vec3F32 SubVec3F32(Vec3F32 a, Vec3F32 b);
func Vec3F32 MulVec3F32(Vec3F32 a, Vec3F32 b);
func Vec3F32 DivVec3F32(Vec3F32 a, Vec3F32 b);
func Vec3F32 ScaleVec3F32(Vec3F32 v, F32 n);
func F32 DotVec3F32(Vec3F32 a, Vec3F32 b);
func Vec3F32 CrossVec3F32(Vec3F32 a, Vec3F32 b);
func F32 MagnitudeSquareVec3F32(Vec3F32 v);
func F32 MagnitudeVec3F32(Vec3F32 v);
func Vec3F32 NormalizeVec3F32(Vec3F32 v);
func Vec3F32 TransformVec3F32(Vec3F32 v, Mat3F32 m);

typedef struct Mat4F32 Mat4F32;
struct Mat4F32
{
  F32 values[4][4];
};

func Mat4F32 MakeMat4F32(F32 diagonal_value);
func Mat4F32 MulMat4F32(Mat4F32 a, Mat4F32 b);
func Mat4F32 MakeLookAtMat4F32(Vec3F32 position, Vec3F32 target, Vec3F32 up);
func Mat4F32 MakeOrthographicMat4F32(F32 left, F32 right, F32 bottom, F32 top, F32 near_z, F32 far_z);
func Mat4F32 MakePerspectiveMat4F32(F32 fov, F32 aspect, F32 near_z, F32 far_z);
func Mat4F32 MakeTransposeMat4F32(Vec3F32 v);
func Mat4F32 MakeRotationMat4F32(Vec3F32 axis, F32 angle);

typedef union Vec4F32 Vec4F32;
union Vec4F32
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

func Vec4F32 MakeVec4F32(F32 x, F32 y, F32 z, F32 w);
func Vec4F32 AddVec4F32(Vec4F32 a, Vec4F32 b);
func Vec4F32 SubVec4F32(Vec4F32 a, Vec4F32 b);
func Vec4F32 MulVec4F32(Vec4F32 a, Vec4F32 b);
func Vec4F32 DivVec4F32(Vec4F32 a, Vec4F32 b);
func Vec4F32 ScaleVec4F32(Vec4F32 v, F32 n);
func F32 DotVec4F32(Vec4F32 a, Vec4F32 b);
func F32 MagnitudeSquareVec4F32(Vec4F32 v);
func F32 MagnitudeVec4F32(Vec4F32 v);
func Vec4F32 NormalizeVec4F32(Vec4F32 v);

#define Vec2IFromVec(v)  MakeVec2I((I32)(v).x, (I32)(v).y)
#define Vec2uFromVec(v)  MakeVec2f((U32)(v).x, (U32)(v).y)
#define Vec2fFromVec(v)  MakeVec2f((F32)(v).x, (F32)(v).y)
#define Vec2FFromVec(v)  MakeVec2f((F32)(v).x, (F32)(v).y)
#define Vec3F32FromVec2(v) MakeVec3F32((F32)(v).x, (F32)(v).y, 0.0f)

// -------------------------------------------------------------------
// Rectangle
typedef union RectI32 RectI32;
union RectI32
{
  struct
  {
    Vec2I32 position;
    Vec2I32 size;
  };

  struct 
  {
    I32 x;
    I32 y;
    I32 w;
    I32 h;
  };

  Vec2I32 value[2];
};

typedef union RectF32 RectF32;
union RectF32
{
  struct
  {
    Vec2F32 position;
    Vec2F32 size;
  };

  struct
  {
    F32 x;
    F32 y;
    F32 w;
    F32 h;
  };

  Vec2F32 value[2];
};

func B32 InsideRectF32(RectF32 rect, Vec2F32 v);

// -------------------------------------------------------------------
// Default Math functions (Use F32)
typedef Vec2F32 Vec2;
typedef Vec3F32 Vec3;
typedef Vec4F32 Vec4;
typedef Mat3F32 Mat3;
typedef Mat4F32 Mat4;

#define RadiansFromDegrees(d) ((PI/180.0f)*d)

// Constructors
#define MakeVec2(x, y) MakeVec2F32(x, y)
#define MakeVec3(x, y, z) MakeVec3F32(x, y, z)
#define MakeVec4(x, y, z, w) MakeVec4F32(x, y, z, w)
#define ZeroVec4() MakeVec4(0.0f, 0.0f, 0.0f, 0.0f)
#define MakeMat3(diagonal_value) MakeMat3F32(diagonal_value)
#define ScaleVec2(v, n) ScaleVec2F32(v, n)
#define AddVec2(a, b) AddVec2F32(a, b)
#define SubVec2(a, b) SubVec2F32(a, b)
#define MulVec2(a, b) MulVec2F32(a, b)
#define DivVec2(a, b) DivVec2F32(a, b)
#define ScaleVec3(v, n) ScaleVec3F32(v, n)
#define AddVec3(a, b) AddVec3F32(a, b)
#define SubVec3(a, b) SubVec3F32(a, b)
#define MulVec3(a, b) MulVec3F32(a, b)
#define DivVec3(a, b) DivVec3F32(a, b)
#define CrossVec3(a, b) CrossVec3F32(a, b)
#define NormalizeVec3(v) NormalizeVec3F32(v)
#define ScaleVec4(v, n) ScaleVec4F32(v, n)
#define AddVec4(a, b) AddVec4F32(a, b)
#define SubVec4(a, b) SubVec4F32(a, b)
#define MulVec4(a, b) MulVec4F32(a, b)
#define DivVec4(a, b) DivVec4F32(a, b)
#define MakeTransposeMat3(v) MakeTransposeMat3F32(v)
#define MakeMat4(diagonal_value) MakeMat4F32(diagonal_value)
#define MulMat4(a, b) MulMat4F32(a, b)
#define MakeOrthographicMat4(left, right, bottom, top, near_z, far_z) MakeOrthographicMat4F32(left, right, bottom, top, near_z, far_z)
#define MakePerspectiveMat4(fov, aspect, near_z, far_z) MakePerspectiveMat4F32(fov, aspect, near_z, far_z)
#define MakeLookAtMat4(position, target, up) MakeLookAtMat4F32(position, target, up);
#define MakeTransposeMat4(v) MakeTransposeMat4F32(v)
#define MakeRotationMat4(axis, angle) MakeRotationMat4F32(axis, angle)

// -------------------------------------------------------------------
// Color
#define RGBFromHex(hex) ScaleVec3F32(MakeVec3F32(((hex>>16)&0xFF), ((hex>>8)&0xFF), ((hex)&0xFF)), 1.0f/255.0f)
#define RGBAFromHex(hex) ScaleVec4F32(MakeVec4F32(((hex>>24)&0xFF), ((hex>>16)&0xFF), ((hex>>8)&0xFF), ((hex)&0xFF)), 1.0f/255.0f)
