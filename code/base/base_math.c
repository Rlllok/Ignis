#include "base_math.h"

// -------------------------------------------------------------------
// Vectors and Matrices
func Vec2I32 MakeVec2I32(I32 x, I32 y) {Vec2I32 result = {x,y}; return result;}
func Vec2I32 AddVec2I32(Vec2I32 a, Vec2I32 b) {return MakeVec2I32(a.x+b.x, a.y+b.y);}
func Vec2I32 SubVec2I32(Vec2I32 a, Vec2I32 b) {return MakeVec2I32(a.x-b.x, a.y-b.y);}

func Vec2U32 MakeVec2U32(U32 x, U32 y) {Vec2U32 result = {x,y}; return result;}
func Vec2U32 AddVec2U32(Vec2U32 a, Vec2U32 b) {return MakeVec2U32(a.x+b.x, a.y+b.y);}
func Vec2U32 SubVec2U32(Vec2U32 a, Vec2U32 b) {return MakeVec2U32(a.x-b.x, a.y-b.y);}

func Vec2F32 MakeVec2F32(F32 x, F32 y) {Vec2F32 result = {x,y}; return result;}
func Vec2F32 AddVec2F32(Vec2F32 a, Vec2F32 b) {return MakeVec2F32(a.x+b.x, a.y+b.y);}
func Vec2F32 SubVec2F32(Vec2F32 a, Vec2F32 b) {return MakeVec2F32(a.x-b.x, a.y-b.y);}
func Vec2F32 MulVec2F32(Vec2F32 a, Vec2F32 b) {return MakeVec2F32(a.x*b.x, a.y*b.y);}
func Vec2F32 DivVec2F32(Vec2F32 a, Vec2F32 b) {return MakeVec2F32(a.x/b.x, a.y/b.y);}
func Vec2F32 ScaleVec2F32(Vec2F32 v, F32 n) {return MakeVec2F32(v.x*n, v.y*n);}
func F32 DotVec2F32(Vec2F32 a, Vec2F32 b) {return a.x*b.x + a.y*b.y;}
func F32 CrossVec2F32(Vec2F32 a, Vec2F32 b) {return a.x*b.y - a.y*b.x;}
func F32 MagnitudeSquareVec2F32(Vec2F32 v) {return v.x*v.x + v.y*v.y;}
func F32 MagnitudeVec2F32(Vec2F32 v) {return sqrt(MagnitudeSquareVec2F32(v));}
func Vec2F32 NormalizeVec2F32(Vec2F32 v) {F32 magnitude = MagnitudeVec2F32(v); return ScaleVec2F32(v, (1.0f/(magnitude + !magnitude)));}
func Vec2F32 GetNormalToVec2F32(Vec2F32 v) {return NormalizeVec2F32(MakeVec2F32(v.y, -v.x));}

func Vec3F32 MakeVec3F32(F32 x, F32 y, F32 z) {Vec3F32 result = {x,y,z}; return result;}
func Vec3F32 AddVec3F32(Vec3F32 a, Vec3F32 b) {return MakeVec3F32(a.x+b.x, a.y+b.y, a.z+b.z);}
func Vec3F32 SubVec3F32(Vec3F32 a, Vec3F32 b) {return MakeVec3F32(a.x-b.x, a.y-b.y, a.z-b.z);}
func Vec3F32 MulVec3F32(Vec3F32 a, Vec3F32 b) {return MakeVec3F32(a.x*b.x, a.y*b.y, a.z*b.z);}
func Vec3F32 DivVec3F32(Vec3F32 a, Vec3F32 b) {return MakeVec3F32(a.x/b.x, a.y/b.y, a.z/b.z);}
func Vec3F32 ScaleVec3F32(Vec3F32 v, F32 n) {return MakeVec3F32(v.x*n, v.y*n, v.z*n);}
func F32 DotVec3F32(Vec3F32 a, Vec3F32 b) {return a.x*b.x + a.y*b.y + a.z*b.z;}
func Vec3F32 CrossVec3F32(Vec3F32 a, Vec3F32 b) {return MakeVec3F32(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);}
func F32 MagnitudeSquareVec3F32(Vec3F32 v) {return v.x*v.x + v.y*v.y + v.z*v.z;}
func F32 MagnitudeVec3F32(Vec3F32 v) {return sqrt(MagnitudeSquareVec3F32(v));}
func Vec3F32 NormalizeVec3F32(Vec3F32 v) {F32 magnitude = MagnitudeVec3F32(v); return ScaleVec3F32(v, (1.0f/(magnitude + !magnitude)));}
func Vec3F32
TransformVec3F32(Vec3F32 v, Mat3F32 m)
{
  Vec3F32 result = {0};
  for (I32 i = 0; i < 3; i += 1)
  {
    result.values[i] += v.values[0] * m.values[0][i];
    result.values[i] += v.values[1] * m.values[1][i];
    result.values[i] += v.values[2] * m.values[2][i];
  }
  return result;
}

func Vec4F32 MakeVec4F32(F32 x, F32 y, F32 z, F32 w) {Vec4F32 result = {x,y,z,w}; return result;}
func Vec4F32 AddVec4F32(Vec4F32 a, Vec4F32 b) {return MakeVec4F32(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);}
func Vec4F32 SubVec4F32(Vec4F32 a, Vec4F32 b) {return MakeVec4F32(a.x-b.x, a.y-b.y, a.z-b.z, a.w+b.w);}
func Vec4F32 MulVec4F32(Vec4F32 a, Vec4F32 b) {return MakeVec4F32(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);}
func Vec4F32 DivVec4F32(Vec4F32 a, Vec4F32 b) {return MakeVec4F32(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);}
func Vec4F32 ScaleVec4F32(Vec4F32 v, F32 n) {return MakeVec4F32(v.x*n, v.y*n, v.z*n, v.w*n);}
func F32 DotVec4F32(Vec4F32 a, Vec4F32 b) {return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;}
func F32 MagnitudeSquareVec4F32(Vec4F32 v) {return v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w;}
func F32 MagnitudeVec4F32(Vec4F32 v) {return sqrt(MagnitudeSquareVec4F32(v));}
func Vec4F32 NormalizeVec4F32(Vec4F32 v) {return ScaleVec4F32(v, 1.0f/MagnitudeVec4F32(v));}

func Mat3F32
MakeMat3F32(F32 diagonal_value)
{
  Mat3F32 result = {0};
  result.values[0][0] = diagonal_value;
  result.values[1][1] = diagonal_value;
  result.values[2][2] = diagonal_value;
  return result;
}

func Mat3F32
MulMat3F32(Mat3F32 a, Mat3F32 b)
{
  Mat3F32 c = {0};
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

func Mat3F32
MakeTransposeMat3F32(Mat3F32 m)
{
  Mat3F32 result = {0};
  for (I32 i = 0; i < 3; i += 1)
  {
    for (I32 j = 0; j < 3; j += 1)
    {
      result.values[i][j] = m.values[j][i];
    }
  }
  return result;
}

func Mat4F32
MakeMat4F32(F32 diagonal_value)
{
  Mat4F32 result = {0};
  result.values[0][0] = diagonal_value;
  result.values[1][1] = diagonal_value;
  result.values[2][2] = diagonal_value;
  result.values[3][3] = diagonal_value;
  return result;
}

func Mat4F32
MulMat4F32(Mat4F32 a, Mat4F32 b)
{
  Mat4F32 result = {0};
  for (I32 i = 0; i < 4; i += 1)
  {
    for (I32 j = 0; j < 4; j += 1)
    {
      result.values[i][j] += a.values[0][j] * b.values[i][0];
      result.values[i][j] += a.values[1][j] * b.values[i][1];
      result.values[i][j] += a.values[2][j] * b.values[i][2];
      result.values[i][j] += a.values[3][j] * b.values[i][3];
    }
  }
  return result;
}

func Mat4F32
MakeLookAtMat4F32(Vec3F32 position, Vec3F32 target, Vec3F32 up)
{
  Mat4F32 result = MakeMat4F32(1.0f);
  
  Vec3F32 f = NormalizeVec3F32(SubVec3F32(target, position));
  Vec3F32 r = NormalizeVec3F32(CrossVec3F32(f, up));
  Vec3F32 u = CrossVec3F32(r, f);
  
    result.values[0][0] = r.x;
		result.values[1][0] = r.y;
		result.values[2][0] = r.z;
		result.values[0][1] = u.x;
		result.values[1][1] = u.y;
		result.values[2][1] = u.z;
		result.values[0][2] =-f.x;
		result.values[1][2] =-f.y;
		result.values[2][2] =-f.z;
		result.values[3][0] =-DotVec3F32(r, position);
		result.values[3][1] =-DotVec3F32(u, position);
		result.values[3][2] = DotVec3F32(f, position);

  return result;
}

func Mat4F32
MakeOrthographicMat4F32(F32 left, F32 right, F32 bottom, F32 top, F32 near_z, F32 far_z)
{
  Mat4F32 result = MakeMat4F32(1.0f);
  
   result.values[0][0] = 2.0f / (right - left);
   result.values[1][1] = 2.0f / (top - bottom);
   result.values[2][2] = 2.0f / (far_z - near_z);
   result.values[3][3] = 1.0f;
   
   result.values[3][0] = (left + right) / (left - right);
   result.values[3][1] = (bottom + top) / (bottom - top);
   result.values[3][2] = (near_z + far_z) / (near_z - far_z);

   return result;
}

func Mat4F32
MakePerspectiveMat4F32(F32 fov, F32 aspect, F32 near_z, F32 far_z)
{
  Mat4F32 result = MakeMat4F32(0.0f);

  F32 fov_rad = fov * 2.0f * PI / 360.0f;
  F32 focal_length = 1.0 / tanf(fov_rad * 0.5f);

  result.values[0][0] = focal_length / aspect;
  result.values[1][1] = -focal_length;
  result.values[2][2] = near_z / (far_z - near_z);
  result.values[3][2] = (far_z * near_z) / (far_z - near_z);
  result.values[2][3] = -1.0f;

  return result;
}

func Mat4F32
MakeTransposeMat4F32(Vec3F32 v)
{
  Mat4F32 result = MakeMat4F32(1.0f);

  result.values[3][0] = v.x;
  result.values[3][1] = v.y;
  result.values[3][2] = v.z;

  return result;
}

func Mat4F32
MakeRotationMat4F32(Vec3F32 axis, F32 angle)
{
 Mat4F32 result = MakeMat4F32(1.f);
 
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
 
 return result;
}
