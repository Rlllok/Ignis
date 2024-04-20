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
