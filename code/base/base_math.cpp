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
