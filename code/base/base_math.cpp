#include "base_math.h"

// -------------------------------------------------------------------
// --AlNov: Vector Operations ----------------------------------------

// --AlNov: Vec2
func Vec2u
MakeVec2u(U32 x, U32 y)
{
  return { {x, y} };
}

inline Vec2f
MakeVec2f(F32 x, F32 y)
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
MulVec2f(Vec2f a, F32 num)
{
    Vec2f result = a;
    result.x *= num;
    result.y *= num;

    return result;
}

func F32
DotVec2f(Vec2f a, Vec2f b)
{
  return a.x*b.x + a.y*b.y;
}

func F32
CrossVec2f(Vec2f a, Vec2f b)
{
  return a.x*b.y - a.y*b.x;
}

func Vec2f
RotateVec2f(Vec2f v, F32 radians)
{
  Vec2f result  = {};
  F32 cos_value = cos(radians);
  F32 sin_value = sin(radians);

  result.x = v.x * cos_value - v.y * sin_value;
  result.y = v.x * sin_value + v.y * cos_value;
  return result;
}

func F32
MagnitudeSquareVec2f(Vec2f v)
{
  return v.x * v.x + v.y * v.y;
}

func F32
MagnitudeVec2f(Vec2f v)
{
  return sqrt(v.x * v.x + v.y * v.y);
}

func Vec2f
NormalizeVec2f(Vec2f v)
{
  F32 magnitude = MagnitudeVec2f(v);
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
MakeVec3f(F32 x, F32 y, F32 z)
{
    Vec3f result = { {x, y, z} };
    return result;
}

func Vec3f
MulVec3f(Vec3f a, F32 num)
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
  for (I32 i = 0; i < 3; i += 1)
  {
    result.values[i] += v.values[0] * m.values[0][i];
    result.values[i] += v.values[1] * m.values[1][i];
    result.values[i] += v.values[2] * m.values[2][i];
  }
  return result;
}

func F32
DotVec3f(Vec3f a, Vec3f b)
{
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

func F32
MagnitudeVec3f(Vec3f v)
{
  return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

func Vec3f
NormalizeVec3f(Vec3f v)
{
  F32 magnitude = MagnitudeVec3f(v);
  if (magnitude == 0)
  {
    return { {0.0f, 0.0f} };
  }

  return MulVec3f(v, 1.0f / magnitude);
}

func Vec3f
CrossVec3f(Vec3f a, Vec3f b)
{
  Vec3f result = {
    .x = a.y*b.z - a.z*b.y,
    .y = a.z*b.x - a.x*b.z,
    .z = a.x*b.y - a.y*b.x
  };
  return result;
}

func Vec4f
MakeVec4f(F32 x, F32 y, F32 z, F32 w)
{
    Vec4f result = { {x, y, z, w} };
    return result;
}

// -------------------------------------------------------------------
// --AlNov: Matrix Operations ----------------------------------------

// --AlNov: Mat3x3
func Mat3x3f
Make3x3f(F32 diagonal_value)
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
  for (I32 i = 0; i < 3; i += 1)
  {
    for (I32 j = 0; j < 3; j += 1)
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
  for (I32 i = 0; i < 3; i += 1)
  {
    for (I32 j = 0; j < 3; j += 1)
    {
      result.values[i][j] = m.values[j][i];
    }
  }
  return result;
}

// --AlNov: Mat4x4
func Mat4x4f
Make4x4f(F32 diagonal_value)
{
  Mat4x4f result = {};
  result.values[0][0] = diagonal_value;
  result.values[1][1] = diagonal_value;
  result.values[2][2] = diagonal_value;
  result.values[3][3] = diagonal_value;
  return result;
}

func Mat4x4f
MakeLookAt(Vec3f from, Vec3f to, Vec3f up)
{
  Mat4x4f result = Make4x4f(1.0f);
  
  Vec3f f = NormalizeVec3f(to - from);
  Vec3f r = NormalizeVec3f(CrossVec3f(f, up));
  Vec3f u = CrossVec3f(r, f);
  
    result.values[0][0] = r.x;
		result.values[1][0] = r.y;
		result.values[2][0] = r.z;
		result.values[0][1] = u.x;
		result.values[1][1] = u.y;
		result.values[2][1] = u.z;
		result.values[0][2] =-f.x;
		result.values[1][2] =-f.y;
		result.values[2][2] =-f.z;
		result.values[3][0] =-DotVec3f(r, from);
		result.values[3][1] =-DotVec3f(u, from);
		result.values[3][2] = DotVec3f(f, from);

  return result;
}

func Mat4x4f
MakeOrthographic4x4f(F32 left, F32 right, F32 bottom, F32 top, F32 near_z, F32 far_z)
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
MakePerspective4x4f(F32 fov, F32 aspect, F32 near_z, F32 far_z)
{
  Mat4x4f result = Make4x4f(0.0f);

  F32 fov_rad = fov * 2.0f * PI / 360.0f;
  F32 focal_length = 1.0 / tanf(fov_rad * 0.5f);

  result.values[0][0] = focal_length / aspect;
  result.values[1][1] = -focal_length;
  result.values[2][2] = near_z / (far_z - near_z);
  result.values[3][2] = (far_z * near_z) / (far_z - near_z);
  result.values[2][3] = -1.0f;

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

func Mat4x4f
Rotate4x4f(Vec3f axis, F32 angle)
{
 Mat4x4f result = Make4x4f(1.f);
 
 F32 sin_theta = sin(angle);
 F32 cos_theta = cos(angle);
 F32 cos_value = 1.f - cos_theta;
 result.values[0][0] = (axis.x * axis.x * cos_value) + cos_theta;
 result.values[0][1] = (axis.x * axis.y * cos_value) + (axis.z * sin_theta);
 result.values[0][2] = (axis.x * axis.z * cos_value) - (axis.y * sin_theta);
 result.values[1][0] = (axis.y * axis.x * cos_value) - (axis.z * sin_theta);
 result.values[1][1] = (axis.y * axis.y * cos_value) + cos_theta;
 result.values[1][2] = (axis.y * axis.z * cos_value) + (axis.x * sin_theta);
 result.values[2][0] = (axis.z * axis.x * cos_value) + (axis.y * sin_theta);
 result.values[2][1] = (axis.z * axis.y * cos_value) - (axis.x * sin_theta);
 result.values[2][2] = (axis.z * axis.z * cos_value) + cos_theta;
 
 return result;}
