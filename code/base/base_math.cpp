#include "base_math.h"

// -------------------------------------------------------------------
// --AlNov: Vector Operations ----------------------------------------

// --AlNov: Vec2
func Vec2u
MakeVec2u(u32 x, u32 y)
{
  return { {x, y} };
}

func Vec2f
MakeVec2f(f32 x, f32 y)
{
  return { {x, y} };
}

func Vec2f
AddVec2f(Vec2f a, Vec2f b)
{
    Vec2f result = {};
    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

func Vec2f
SubVec2f(Vec2f a, Vec2f b)
{
  Vec2f result = {};
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  return result;
}

func Vec2f
MulVec2f(Vec2f a, f32 num)
{
    Vec2f result = a;
    result.x *= num;
    result.y *= num;

    return result;
}

func f32
DotVec2f(Vec2f a, Vec2f b)
{
  return a.x*b.x + a.y*b.y;
}

func f32
CrossVec2f(Vec2f a, Vec2f b)
{
  return a.x*b.y - a.y*b.x;
}

func Vec2f
RotateVec2f(Vec2f v, f32 radians)
{
  Vec2f result  = {};
  f32 cos_value = cos(radians);
  f32 sin_value = sin(radians);

  result.x = v.x * cos_value - v.y * sin_value;
  result.y = v.x * sin_value + v.y * cos_value;
  return result;
}

func f32
MagnitudeSquareVec2f(Vec2f v)
{
  return v.x * v.x + v.y * v.y;
}

func f32
MagnitudeVec2f(Vec2f v)
{
  return sqrt(v.x * v.x + v.y * v.y);
}

func Vec2f
NormalizeVec2f(Vec2f v)
{
  f32 magnitude = MagnitudeVec2f(v);
  if (magnitude == 0)
  {
    return { {0.0f, 0.0f} };
  }

  return { {v.x / magnitude, v.y / magnitude} };
}

func Vec2f
NormalToVec2f(Vec2f v)
{
  return NormalizeVec2f(MakeVec2f(v.y, -v.x));
}

// --AlNov: Vec3
func Vec3f
MakeVec3f(f32 x, f32 y, f32 z)
{
    Vec3f result = { {x, y, z} };
    return result;
}

func Vec3f
MulVec3f(Vec3f a, f32 num)
{
  a.x *= num;
  a.y *= num;
  a.z *= num;
  return a;
}

func Vec3f
TransformVec3f(Vec3f v, Mat3x3f m)
{
  Vec3f result = {};
  for (i32 i = 0; i < 3; i += 1)
  {
    result.values[i] += v.values[0] * m.values[0][i];
    result.values[i] += v.values[1] * m.values[1][i];
    result.values[i] += v.values[2] * m.values[2][i];
  }
  return result;
}

func f32
DotVec3f(Vec3f a, Vec3f b)
{
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

func f32
MagnitudeVec3f(Vec3f v)
{
  return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

func Vec3f
NormalizeVec3f(Vec3f v)
{
  f32 magnitude = MagnitudeVec3f(v);
  if (magnitude == 0)
  {
    return { {0.0f, 0.0f} };
  }

  return MulVec3f(v, 1.0f / magnitude);
}

func Vec4f
MakeVec4f(f32 x, f32 y, f32 z, f32 w)
{
    Vec4f result = { {x, y, z, w} };
    return result;
}

// -------------------------------------------------------------------
// --AlNov: Matrix Operations ----------------------------------------

// --AlNov: Mat3x3
func Mat3x3f
Make3x3f(f32 diagonal_value)
{
  Mat3x3f result = {};
  result.values[0][0] = diagonal_value;
  result.values[1][1] = diagonal_value;
  result.values[2][2] = diagonal_value;
  return result;
}

func Mat3x3f
Mul3x3f(Mat3x3f a, Mat3x3f b)
{
  Mat3x3f c = {};
  for (i32 i = 0; i < 3; i += 1)
  {
    for (i32 j = 0; j < 3; j += 1)
    {
      c.values[i][j] += a.values[0][j] * b.values[i][0];
      c.values[i][j] += a.values[1][j] * b.values[i][1];
      c.values[i][j] += a.values[2][j] * b.values[i][2];
    }
  }
  return c;
}

func Mat3x3f
Transpose3x3f(Mat3x3f m)
{
  Mat3x3f result = {};
  for (i32 i = 0; i < 3; i += 1)
  {
    for (i32 j = 0; j < 3; j += 1)
    {
      result.values[i][j] = m.values[j][i];
    }
  }
  return result;
}

// --AlNov: Mat4x4
func Mat4x4f
Make4x4f(f32 diagonal_value)
{
  Mat4x4f result = {};
  result.values[0][0] = diagonal_value;
  result.values[1][1] = diagonal_value;
  result.values[2][2] = diagonal_value;
  result.values[3][3] = diagonal_value;
  return result;
}

func Mat4x4f
MakeOrthographic4x4f(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z)
{
  Mat4x4f result = Make4x4f(1.0f);
  
   result.values[0][0] = 2.0f / (right - left);
   result.values[1][1] = 2.0f / (top - bottom);
   result.values[2][2] = 2.0f / (far_z - near_z);
   result.values[3][3] = 1.0f;
   
   result.values[3][0] = (left + right) / (left - right);
   result.values[3][1] = (bottom + top) / (bottom - top);
   result.values[3][2] = (near_z + far_z) / (near_z - far_z);

   return result;
}

func Mat4x4f
MakePerspective4x4f(f32 fov, f32 aspect_ration, f32 near_z, f32 far_z)
{
  Mat4x4f result = Make4x4f(0.0f);

  f32 fov_radians  = PI * fov / 180.0f;
  f32 focal_length = 1.0f / tanf(fov_radians / 2.0f);

  result.values[0][0] = focal_length / aspect_ration;
  result.values[1][1] = focal_length;
  result.values[2][2] = far_z / (far_z - near_z);
  result.values[3][3] = 0.0f;

  result.values[3][2] = -(near_z * far_z) / (far_z - near_z);
  result.values[2][3] = 1.0f;

  return result;
}

func Mat4x4f
Transpose4x4f(Vec3f v)
{
  Mat4x4f result = Make4x4f(1.0f);

  result.values[3][0] = v.x;
  result.values[3][1] = v.y;
  result.values[3][2] = v.z;

  return result;
}
