#include "base_math.h"

Vec2u MakeVec2u(u32 x, u32 y)
{
    return { {x, y} };
}

Vec2f MakeVec2f(f32 x, f32 y)
{
    return { {x, y} };
}

Vec3f MakeVec3f(f32 x, f32 y, f32 z)
{
    Vec3f result = { {x, y, z} };
    return result;
}

func Vec3f TransformVec3f(Vec3f v, Mat3x3f32 m)
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

func f32 DotVec2f(Vec2f a, Vec2f b)
{
  return a.x*b.x + a.y*b.y;
}

func f32 DotVec3f(Vec3f a, Vec3f b)
{
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

func f32 CrossVec2f(Vec2f a, Vec2f b)
{
  return a.x*b.y - a.y*b.x;
}

Vec4f MakeVec4f(f32 x, f32 y, f32 z, f32 w)
{
    Vec4f result = { {x, y, z, w} };
    return result;
}

Vec3f Vec3fFromVec2f(Vec2f v)
{
  return MakeVec3f(v.x, v.y, 0.0f);
}

Vec2f AddVec2f(Vec2f a, Vec2f b)
{
    Vec2f result = {};
    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

Vec2f SubVec2f(Vec2f a, Vec2f b)
{
  return { {a.x - b.x, a.y - b.y} };
}

Vec2f MulVec2f(Vec2f a, f32 num)
{
    Vec2f result = a;
    result.x *= num;
    result.y *= num;

    return result;
}

Vec3f MulVec3f(Vec3f a, f32 num)
{
  a.x *= num;
  a.y *= num;
  a.z *= num;
  return a;
}

Vec2f RotateVec2f(Vec2f v, f32 radians)
{
  Vec2f result  = {};
  f32 cos_value = cos(radians);
  f32 sin_value = sin(radians);

  result.x = v.x * cos_value - v.y * sin_value;
  result.y = v.x * sin_value + v.y * cos_value;
  return result;
}

f32 MagnitudeSquareVec2f(Vec2f v)
{
  return v.x * v.x + v.y * v.y;
}

f32 MagnitudeVec2f(Vec2f v)
{
  return sqrt(v.x * v.x + v.y * v.y);
}

Vec2f NormalizeVec2f(Vec2f v)
{
  f32 magnitude = MagnitudeVec2f(v);
  if (magnitude == 0)
  {
    return { {0.0f, 0.0f} };
  }

  return { {v.x / magnitude, v.y / magnitude} };
}

func Vec2f NormalToVec2f(Vec2f v)
{
  return NormalizeVec2f(MakeVec2f(v.y, -v.x));
}

func Mat3x3f32 Make3x3f32(f32 diagonal_value)
{
  Mat3x3f32 result = {};
  result.values[0][0] = diagonal_value;
  result.values[1][1] = diagonal_value;
  result.values[2][2] = diagonal_value;
  return result;
}

func Mat4x4f32 Make4x4f32(f32 diagonal_value)
{
  Mat4x4f32 result = {};
  result.values[0][0] = diagonal_value;
  result.values[1][1] = diagonal_value;
  result.values[2][2] = diagonal_value;
  result.values[3][3] = diagonal_value;
  return result;
}

func Mat4x4f32 MakeOrthographic4x4f32(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z)
{
  Mat4x4f32 result = Make4x4f32(1.0f);
  
   result.values[0][0] = 2.0f / (right - left);
   result.values[1][1] = 2.0f / (top - bottom);
   result.values[2][2] = 2.0f / (far_z - near_z);
   result.values[3][3] = 1.0f;
   
   result.values[3][0] = (left + right) / (left - right);
   result.values[3][1] = (bottom + top) / (bottom - top);
   result.values[3][2] = (near_z + far_z) / (near_z - far_z);

   return result;
}

func Mat3x3f32 Mul3x3f32(Mat3x3f32 a, Mat3x3f32 b)
{
  Mat3x3f32 c = {};
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

func Mat3x3f32 Transpose3x3f32(Mat3x3f32 m)
{
  Mat3x3f32 result = {};
  for (i32 i = 0; i < 3; i += 1)
  {
    for (i32 j = 0; j < 3; j += 1)
    {
      result.values[i][j] = m.values[j][i];
    }
  }
  return result;
}

func Vec3f Solve3x3f32(Mat3x3f32 m, Vec3f v)
{
  Vec3f x = {};
  i32   number_of_iteration = 5;
  for (i32 alg_iteration = 0; alg_iteration < number_of_iteration; alg_iteration += 1)
  {
    for (i32 i = 0; i < 3; i += 1)
    {
      // if (m.values[i][i] != 0.0f)
      if (fabsf(m.values[i][i]) > 0.000005)
      {
        Vec3f row = MakeVec3f(m.values[i][0], m.values[i][1], m.values[i][2]);
        x.values[i] += (v.values[i] / m.values[i][i]) - (DotVec3f(row, x) / m.values[i][i]);
      }
    }
  }
  return x;
}
