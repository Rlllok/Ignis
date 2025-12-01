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
func Vec3F32
LerpVec3F32(Vec3F32 a, Vec3F32 b, F32 t)
{
  Vec3F32 result = {0};
  result = AddVec3F32(ScaleVec3F32(a, 1.0f - t), ScaleVec3F32(b, t));
  return result;
}


func Vec4F32 MakeVec4F32(F32 x, F32 y, F32 z, F32 w) {Vec4F32 result = {x,y,z,w}; return result;}
func Vec4F32 Vec4F32FromVec3(Vec3F32 v, F32 w) {return MakeVec4F32(v.x, v.y, v.z, w);}
func Vec4F32 AddVec4F32(Vec4F32 a, Vec4F32 b) {return MakeVec4F32(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);}
func Vec4F32 SubVec4F32(Vec4F32 a, Vec4F32 b) {return MakeVec4F32(a.x-b.x, a.y-b.y, a.z-b.z, a.w+b.w);}
func Vec4F32 MulVec4F32(Vec4F32 a, Vec4F32 b) {return MakeVec4F32(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);}
func Vec4F32 DivVec4F32(Vec4F32 a, Vec4F32 b) {return MakeVec4F32(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);}
func Vec4F32 ScaleVec4F32(Vec4F32 v, F32 n) {return MakeVec4F32(v.x*n, v.y*n, v.z*n, v.w*n);}
func F32 DotVec4F32(Vec4F32 a, Vec4F32 b) {return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;}
func F32 MagnitudeSquareVec4F32(Vec4F32 v) {return v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w;}
func F32 MagnitudeVec4F32(Vec4F32 v) {return sqrt(MagnitudeSquareVec4F32(v));}
func Vec4F32 NormalizeVec4F32(Vec4F32 v) {return ScaleVec4F32(v, 1.0f/MagnitudeVec4F32(v));}
func Vec4F32
TransformVec4F32(Vec4F32 v, Mat4F32 m)
{
  Vec4F32 result = {0};
  for (I32 i = 0; i < 4; i += 1)
  {
    result.values[i] += v.values[0] * m.values[0][i];
    result.values[i] += v.values[1] * m.values[1][i];
    result.values[i] += v.values[2] * m.values[2][i];
    result.values[i] += v.values[3] * m.values[3][i];
  }
  return result;
}

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
ScaleMat4F32(Mat4F32 m, F32 n)
{
  for (I32 i = 0; i < 4; i += 1)
  {
    for (I32 j = 0; j < 4; j += 1)
    {
      m.values[i][j] *= n;
    }
  }

  return m;
}

func Mat4F32
InverseMat4F32(Mat4F32 m)
{
  F32 coef00 = m.values[2][2] * m.values[3][3] - m.values[3][2] * m.values[2][3];
  F32 coef02 = m.values[1][2] * m.values[3][3] - m.values[3][2] * m.values[1][3];
  F32 coef03 = m.values[1][2] * m.values[2][3] - m.values[2][2] * m.values[1][3];
  F32 coef04 = m.values[2][1] * m.values[3][3] - m.values[3][1] * m.values[2][3];
  F32 coef06 = m.values[1][1] * m.values[3][3] - m.values[3][1] * m.values[1][3];
  F32 coef07 = m.values[1][1] * m.values[2][3] - m.values[2][1] * m.values[1][3];
  F32 coef08 = m.values[2][1] * m.values[3][2] - m.values[3][1] * m.values[2][2];
  F32 coef10 = m.values[1][1] * m.values[3][2] - m.values[3][1] * m.values[1][2];
  F32 coef11 = m.values[1][1] * m.values[2][2] - m.values[2][1] * m.values[1][2];
  F32 coef12 = m.values[2][0] * m.values[3][3] - m.values[3][0] * m.values[2][3];
  F32 coef14 = m.values[1][0] * m.values[3][3] - m.values[3][0] * m.values[1][3];
  F32 coef15 = m.values[1][0] * m.values[2][3] - m.values[2][0] * m.values[1][3];
  F32 coef16 = m.values[2][0] * m.values[3][2] - m.values[3][0] * m.values[2][2];
  F32 coef18 = m.values[1][0] * m.values[3][2] - m.values[3][0] * m.values[1][2];
  F32 coef19 = m.values[1][0] * m.values[2][2] - m.values[2][0] * m.values[1][2];
  F32 coef20 = m.values[2][0] * m.values[3][1] - m.values[3][0] * m.values[2][1];
  F32 coef22 = m.values[1][0] * m.values[3][1] - m.values[3][0] * m.values[1][1];
  F32 coef23 = m.values[1][0] * m.values[2][1] - m.values[2][0] * m.values[1][1];

  Vec4F32 fac0 = { coef00, coef00, coef02, coef03 };
  Vec4F32 fac1 = { coef04, coef04, coef06, coef07 };
  Vec4F32 fac2 = { coef08, coef08, coef10, coef11 };
  Vec4F32 fac3 = { coef12, coef12, coef14, coef15 };
  Vec4F32 fac4 = { coef16, coef16, coef18, coef19 };
  Vec4F32 fac5 = { coef20, coef20, coef22, coef23 };

  Vec4F32 vec0 = { m.values[1][0], m.values[0][0], m.values[0][0], m.values[0][0] };
  Vec4F32 vec1 = { m.values[1][1], m.values[0][1], m.values[0][1], m.values[0][1] };
  Vec4F32 vec2 = { m.values[1][2], m.values[0][2], m.values[0][2], m.values[0][2] };
  Vec4F32 vec3 = { m.values[1][3], m.values[0][3], m.values[0][3], m.values[0][3] };

  Vec4F32 inv0 = AddVec4F32(SubVec4F32(MulVec4F32(vec1, fac0), MulVec4F32(vec2, fac1)), MulVec4F32(vec3, fac2));
  Vec4F32 inv1 = AddVec4F32(SubVec4F32(MulVec4F32(vec0, fac0), MulVec4F32(vec2, fac3)), MulVec4F32(vec3, fac4));
  Vec4F32 inv2 = AddVec4F32(SubVec4F32(MulVec4F32(vec0, fac1), MulVec4F32(vec1, fac3)), MulVec4F32(vec3, fac5));
  Vec4F32 inv3 = AddVec4F32(SubVec4F32(MulVec4F32(vec0, fac2), MulVec4F32(vec1, fac4)), MulVec4F32(vec2, fac5));

  Vec4F32 sign_a = { +1.0f, -1.0f, +1.0f, -1.0f };
  Vec4F32 sign_b = { -1.0f, +1.0f, -1.0f, +1.0f };

  Mat4F32 inverse = {0};
  for(U32 i = 0; i < 4; i += 1)
  {
  inverse.values[0][i] = inv0.values[i] * sign_a.values[i];
  inverse.values[1][i] = inv1.values[i] * sign_b.values[i];
  inverse.values[2][i] = inv2.values[i] * sign_a.values[i];
  inverse.values[3][i] = inv3.values[i] * sign_b.values[i];
  }

  Vec4F32 row0 = { inverse.values[0][0], inverse.values[1][0], inverse.values[2][0], inverse.values[3][0] };
  Vec4F32 m0 = { m.values[0][0], m.values[0][1], m.values[0][2], m.values[0][3] };
  Vec4F32 dot0 = MulVec4F32(m0, row0);
  F32 dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

  F32 one_over_det = 1 / dot1;

  return ScaleMat4F32(inverse, one_over_det);
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
 axis = NormalizeVec3F32(axis);
 
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

func Mat4F32
MakeScaleMat4F32(Vec3F32 v)
{
  Mat4F32 result = MakeMat4F32(1.0f);
  result.values[0][0] = v.x;
  result.values[1][1] = v.y;
  result.values[2][2] = v.z;
  result.values[3][3] = 1.0f;

  return result;
}

// -------------------------------------------------------------------
// Quaternions
func Quaternion MakeQuaternion(F32 x, F32 y, F32 z, F32 w) {return (Quaternion){x, y, z, w};}
func Vec4F32 Vec4F32FromQuaternion(Quaternion q) {return (Vec4F32){q.x, q.y, q.z, q.z};}

func Quaternion
AddQuaternion(Quaternion a, Quaternion b)
{
  Quaternion result = IdentityQuaternion();
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  result.z = a.z + b.z;
  result.w = a.w + b.w;
  return result;
}

func Quaternion
MulQuaternion(Quaternion l, Quaternion r)
{
  Quaternion result = {.w = 1};
  result.x = l.x*r.w + l.y*r.z - l.z*r.y + l.w*r.x;
  result.y = -l.x*r.z + l.y*r.w + l.z*r.x + l.w*r.y;
  result.z = l.x*r.y - l.y * r.x + l.z * r.w + l.w*r.z;
  result.w = -l.x*r.x - l.y*r.y - l.z*r.z + l.w*r.w;

  return result;
}

func Quaternion
ScaleQuaternion(Quaternion q, F32 s)
{
  Quaternion result = IdentityQuaternion();
  result.x = q.x*s;
  result.y = q.y*s;
  result.z = q.z*s;
  result.w = q.w*s;
  return result;
}

func Quaternion
MulQuaternionTest(Quaternion a, Quaternion b)
{
 Quaternion c;
 {
  c.x =  b.values[3] * +a.values[0];
  c.y =  b.values[2] * -a.values[0];
  c.z =  b.values[1] * +a.values[0];
  c.w =  b.values[0] * -a.values[0];
  c.x += b.values[2] * +a.values[1];
  c.y += b.values[3] * +a.values[1];
  c.z += b.values[0] * -a.values[1];
  c.w += b.values[1] * -a.values[1];
  c.x += b.values[1] * -a.values[2];
  c.y += b.values[0] * +a.values[2];
  c.z += b.values[3] * +a.values[2];
  c.w += b.values[2] * -a.values[2];
  c.x += b.values[0] * +a.values[3];
  c.y += b.values[1] * +a.values[3];
  c.z += b.values[2] * +a.values[3];
  c.w += b.values[3] * +a.values[3];
 }
 return c;
}

func Quaternion
NormalizeQuaternion(Quaternion q)
{
  Quaternion result = {0};
  F32 normal = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);

  result.x = q.x/normal;
  result.y = q.y/normal;
  result.z = q.z/normal;
  result.w = q.w/normal;

  return result;
}

func Quaternion ConjugateQuaternion(Quaternion q) {return MakeQuaternion(-q.x, -q.y, -q.z, q.w);}

func Vec3F32
RotateVec3F32(Vec3F32 v, Quaternion q)
{
  Vec3F32 result = {0};

  Quaternion v_rotated = MulQuaternion(MulQuaternion(q, MakeQuaternion(v.x, v.y, v.z, 0.0f)), ConjugateQuaternion(q));

  result.x = v_rotated.x;
  result.y = v_rotated.y;
  result.z = v_rotated.z;

  return result;
}

func Quaternion
SlerpQuaternion(Quaternion a, Quaternion b, F32 w)
{
  Quaternion result = IdentityQuaternion();

  F32 dot = DotVec4F32(Vec4F32FromQuaternion(a), Vec4F32FromQuaternion(b));
  F32 angle = acosf(dot);

  // --AlNov: @TODO Do we need to normalize?
  result = NormalizeQuaternion(AddQuaternion(
    ScaleQuaternion(a, sinf((1 - w)*angle)/sinf(angle)),
    ScaleQuaternion(b, sinf(w*angle)/sinf(angle))
  ));
  
  return result;
}


func Quaternion
QuaternionFromEuler(F32 roll, F32 pitch, F32 yaw)
{
  Quaternion result = {0};

  F32 cos_roll = cosf(roll*0.5f);
  F32 sin_roll = sinf(roll*0.5f);
  F32 cos_pitch = cosf(pitch*0.5f);
  F32 sin_pitch = sinf(pitch*0.5f);
  F32 cos_yaw = cosf(yaw*0.5f);
  F32 sin_yaw = sinf(yaw*0.5f);

  // --AlNov: ZYX
  result.x = sin_roll*cos_pitch*cos_yaw - cos_roll*sin_pitch*sin_yaw;
  result.y = cos_roll*sin_pitch*cos_yaw + sin_roll*cos_pitch*sin_yaw;
  result.z = cos_roll*cos_pitch*sin_yaw - sin_roll*sin_pitch*cos_yaw;
  result.w = cos_roll*cos_pitch*cos_yaw + sin_roll*sin_pitch*sin_yaw;

  return result;
}

func Vec3F32
EulerFromQuaternion(Quaternion q)
{
  Vec3F32 result = {0};

  result.x = atan2(2.0f*(q.w*q.x + q.y*q.z), 1.0f - 2.0f*(q.x*q.x + q.y*q.y));
  result.y = 2.0f*atan2(sqrtf(1.0f + 2.0f*(q.w*q.y - q.x*q.z)), sqrtf(1.0f - 2.0f*(q.w*q.y - q.x*q.z))) - PI/2.0f;
  result.z = atan2(2.0f*(q.w*q.z + q.x*q.y), 1.0f - 2.0f*(q.y*q.y + q.z*q.z));

  return result;
}

func Mat4F32
Mat4F32FromQuaternion(Quaternion q)
{
  Mat4F32 result = {0};
  Quaternion q_norm = NormalizeQuaternion(q);

  result.values[0][0] = 1 - 2*(q_norm.y*q_norm.y + q_norm.z*q_norm.z);
  result.values[0][1] = 2*(q_norm.x*q_norm.y - q_norm.w*q_norm.z);
  result.values[0][2] = 2*(q_norm.w*q_norm.y + q_norm.x*q_norm.z);
  result.values[0][3] = 0;

  result.values[1][0] = 2*(q_norm.x*q_norm.y + q_norm.w*q_norm.z);
  result.values[1][1] = 1 - 2*(q_norm.x*q_norm.x + q_norm.z*q_norm.z);
  result.values[1][2] = 2*(q_norm.y*q_norm.z - q_norm.w*q_norm.x);
  result.values[1][3] = 0;

  result.values[2][0] = 2*(q_norm.x*q_norm.z - q_norm.w*q_norm.y);
  result.values[2][1] = 2*(q_norm.w*q_norm.x + q_norm.y*q_norm.z);
  result.values[2][2] = 1 - 2*(q_norm.x*q_norm.x + q_norm.y*q_norm.y);
  result.values[2][3] = 0;

  result.values[3][0] = 0;
  result.values[3][1] = 0;
  result.values[3][2] = 0;
  result.values[3][3] = 1;

  return result;
}

// -------------------------------------------------------------------
// Rectangle
func B32
InsideRectF32(RectF32 rect, Vec2F32 v)
{
  B32 result = 0;

  result =
    (v.x > rect.position.x)&&
    (v.y > rect.position.y)&&
    (v.x < rect.position.x + rect.size.x)&&
    (v.y < rect.position.y + rect.size.y);

  return result;
}

// -------------------------------------------------------------------
// Transform
func Mat4F32
Mat4F32FromTransform(Transform t)
{
  Mat4F32 result = MakeMat4F32(1.0f);
  result = MulMat4F32(MakeScaleMat4F32(t.scale), result);
  result = MulMat4F32(Mat4F32FromQuaternion(t.rotation), result);
  result = MulMat4F32(MakeTransposeMat4F32(t.translation), result);
  return result;
}
