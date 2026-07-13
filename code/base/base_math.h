#pragma once

#include "base_core.h"
#include "base_container.h"

#include "stdio.h"
#include "math.h"

// -------------------------------------------------------------------
// -- Constants ------------------------------------------------------
#define PI 3.141592654f

// -------------------------------------------------------------------
// -- Simple Math ----------------------------------------------------
#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Clamp(v, low, high) Max(Min(v, high), low)
#define FloorF32(v) floorf(v)
#define CeilF32(v) ceilf(v)
#define RoundF32(v) roundf(v)
#define RadiansFromDegrees(d) (d*PI/180.0f)

// -------------------------------------------------------------------
// -- Vectors and Matrices -------------------------------------------
typedef union Vec2I32 Vec2I32;
union Vec2I32 {
  struct {
    I32 x;
    I32 y;
  };

  I32 values[2];
};

#define ZeroVec2I() {0,0}
#define OneVec2I() 	{1,1}

func Vec2I32 MakeVec2I32(I32 x, I32 y);
func Vec2I32 AddVec2I32(Vec2I32 a, Vec2I32 b);
func Vec2I32 SubVec2I32(Vec2I32 a, Vec2I32 b);

typedef union Vec3I32 Vec3I32;
union Vec3I32 {
  struct {
    I32 x;
    I32 y;
    I32 z;
  };

  I32 values[2];
};

#define ZeroVec3I() {0,0}
#define OneVec3I() 	{1,1}

func Vec3I32 MakeVec3I32(I32 x, I32 y, I32 z);
func Vec3I32 AddVec3I32(Vec3I32 a, Vec3I32 b);
func Vec3I32 SubVec3I32(Vec3I32 a, Vec3I32 b);

typedef union Vec4U8 Vec4U8;
union Vec4U8 {
  struct {
    U8 x;
    U8 y;
    U8 z;
    U8 w;
  };

  U8 values[4];
};

typedef union Vec4I32 Vec4I32;
union Vec4I32 {
  struct {
    I32 x;
    I32 y;
    I32 z;
    I32 w;
  };

  I32 values[4];
};

typedef union Vec2U32 Vec2U32;
union Vec2U32 {
  struct {
    U32 x;
    U32 y;
  };

	struct {
		U32 w;
		U32 h;
	};
  U32 values[2];
};

func Vec2U32 MakeVec2U32(U32 a, U32 b);
func Vec2U32 AddVec2U32(Vec2U32 a, Vec2U32 b);
func Vec2U32 SubVec2U32(Vec2U32 a, Vec2U32 b);

typedef union Vec2F32 Vec2F32;
union Vec2F32 {
  struct {
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
struct Mat3F32 {
  F32 values[3][3];
};

func Mat3F32 MakeMat3F32(F32 diagonal_value);
func Mat3F32 MulMat3F32(Mat3F32 a, Mat3F32 b);
func Mat3F32 MakeTransposeMat3F32(Mat3F32 m);

typedef union Vec3F32 Vec3F32;
union Vec3F32 {
  struct {
    F32 x;
    F32 y;
    F32 z;
  };

  struct {
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
func Vec3F32 LerpVec3F32(Vec3F32 a, Vec3F32 b, F32 t);

typedef struct Mat4F32 Mat4F32;
struct Mat4F32 {
  F32 values[4][4];
};

func Mat4F32 MakeMat4F32(F32 diagonal_value);
func Mat4F32 MulMat4F32(Mat4F32 a, Mat4F32 b);
func Mat4F32 ScaleMat4F32(Mat4F32 m, F32 n);
func Mat4F32 InverseMat4F32(Mat4F32 m);
func Mat4F32 MakeLookAtMat4F32(Vec3F32 position, Vec3F32 target, Vec3F32 up);
func Mat4F32 MakeOrthographicMat4F32(F32 left, F32 right, F32 bottom, F32 top, F32 near_z, F32 far_z);
func Mat4F32 MakePerspectiveMat4F32(F32 fov, F32 aspect, F32 near_z, F32 far_z);
func Mat4F32 MakeTransposeMat4F32(Vec3F32 v);
func Mat4F32 MakeRotationMat4F32(Vec3F32 axis, F32 angle);
func Mat4F32 MakeScaleMat4F32(Vec3F32 v);

typedef union Vec4F32 Vec4F32;
union Vec4F32 {
  struct {
    F32 x;
    F32 y;
    F32 z;
    F32 w;
  };

  struct {
    F32 r;
    F32 g;
    F32 b;
    F32 a;
  };

  F32 values[4];
};

func Vec4F32 MakeVec4F32(F32 x, F32 y, F32 z, F32 w);
func Vec4F32 Vec4F32FromVec3(Vec3F32 v, F32 w);
func Vec4F32 AddVec4F32(Vec4F32 a, Vec4F32 b);
func Vec4F32 SubVec4F32(Vec4F32 a, Vec4F32 b);
func Vec4F32 MulVec4F32(Vec4F32 a, Vec4F32 b);
func Vec4F32 DivVec4F32(Vec4F32 a, Vec4F32 b);
func Vec4F32 ScaleVec4F32(Vec4F32 v, F32 n);
func F32 DotVec4F32(Vec4F32 a, Vec4F32 b);
func F32 MagnitudeSquareVec4F32(Vec4F32 v);
func F32 MagnitudeVec4F32(Vec4F32 v);
func Vec4F32 NormalizeVec4F32(Vec4F32 v);
func Vec4F32 TransformVec4F32(Vec4F32 v, Mat4F32 m);
func B32 EqualVec4F32(Vec4F32 a, Vec4F32 b);
func Vec4F32 LerpVec4F32(Vec4F32 a, Vec4F32 b, F32 t);

#define Vec2IFromVec2F32(v)  MakeVec2I32((I32)(v).x, (I32)(v).y)

#define Vec2IFromVec(v)  MakeVec2I((I32)(v).x, (I32)(v).y)
#define Vec2uFromVec(v)  MakeVec2f((U32)(v).x, (U32)(v).y)
#define Vec2fFromVec(v)  MakeVec2f((F32)(v).x, (F32)(v).y)
#define Vec2FFromVec(v)  MakeVec2f((F32)(v).x, (F32)(v).y)
#define Vec3F32FromVec2(v) MakeVec3F32((F32)(v).x, (F32)(v).y, 0.0f)

// -------------------------------------------------------------------
// -- Quaternions ----------------------------------------------------
typedef union Quaternion Quaternion;
union Quaternion {
  struct {
    F32 x;
    F32 y;
    F32 z;
    F32 w;
  };
  F32 values[4];
};
#define IdentityQuaternion() (Quaternion){0.0f, 0.0f, 0.0f, 1.0f}

func Quaternion MakeQuaternion(F32 x, F32 y, F32 z, F32 w);
func Vec4F32 Vec4F32FromQuaternion(Quaternion q);
func Quaternion QuaternionFromVec4F32(Vec4F32 v);
func Quaternion AddQuaternion(Quaternion a, Quaternion b);
func Quaternion MulQuaternion(Quaternion l, Quaternion r);
func Quaternion ScaleQuaternion(Quaternion q, F32 s);
func Quaternion NormalizeQuaternion(Quaternion q);
func Quaternion ConjugateQuaternion(Quaternion q);

func Vec3F32 RotateVec3F32(Vec3F32 v, Quaternion q);
func Quaternion SlerpQuaternion(Quaternion a, Quaternion b, F32 w);

func Quaternion QuaternionFromEuler(F32 roll, F32 yaw, F32 pitch);
func Vec3F32 EulerFromQuaternion(Quaternion q);
func Mat4F32 Mat4F32FromQuaternion(Quaternion q);

func Quaternion QuaternionLookAt(Vec3F32 source, Vec3F32 target);

// -------------------------------------------------------------------
// -- Rectangle ------------------------------------------------------
typedef union RectI32 RectI32;
union RectI32 {
  struct {
    Vec2I32 position;
    Vec2I32 size;
  };

  struct {
    I32 x;
    I32 y;
    I32 w;
    I32 h;
  };

  Vec2I32 value[2];
};

typedef union RectF32 RectF32;
union RectF32 {
  struct {
    Vec2F32 position;
    Vec2F32 size;
  };

  struct {
    F32 x;
    F32 y;
    F32 w;
    F32 h;
  };

  Vec2F32 value[2];
};

func RectF32 MakeRectF32(F32 x, F32 y, F32 w, F32 h);
func B32 InsideRectF32(RectF32 rect, Vec2F32 v);

// -------------------------------------------------------------------
// -- Transform ------------------------------------------------------
typedef struct Transform Transform;
struct Transform {
  Vec3F32 translation;
  Quaternion rotation;
  Vec3F32 scale;
};
#define IdentityTransform() (Transform){.scale = {1.0f, 1.0f, 1.0f}, .rotation.w = 1.0f}
Transform _transform_nil = IdentityTransform();
DefineArray(Transform, TransformArray, _transform_nil)

func Mat4F32 Mat4F32FromTransform(Transform t);

// -------------------------------------------------------------------
// -- Color ----------------------------------------------------------
#define RGBFromHex(hex) ScaleVec3F32(MakeVec3F32(((hex>>16)&0xFF), ((hex>>8)&0xFF), ((hex)&0xFF)), 1.0f/255.0f)
#define RGBAFromHex(hex) ScaleVec4F32(MakeVec4F32(((hex>>24)&0xFF), ((hex>>16)&0xFF), ((hex>>8)&0xFF), ((hex)&0xFF)), 1.0f/255.0f)

func Vec3F32 RGBFromHSV(Vec3F32 hsv);
func Vec3F32 HSVFromRGB(Vec3F32 rgb);
