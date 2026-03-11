#pragma once

typedef struct BitmapHeader BitmapHeader;
#pragma pack(push, 1)
struct BitmapHeader {
  U16 file_type;
  U32 file_size;
  U16 reserved_1;
  U16 reserved_2;
  U32 bitmap_offset;
  U32 size;
  I32 width;
  I32 height;
  U16 planes;
  U16 bits_per_pixel;
  U32 compression;
  U32 size_of_bitmap;
  I32 horizontal_resolution;
  I32 vertical_resolution;
  U32 colors_used;
  U32 colors_important;
};
#pragma pack(pop)

typedef struct Image32 Image32;
struct Image32 {
  I32  width;
  I32  height;
  U32* pixels;
};

typedef struct Viewport Viewport;
struct Viewport {
  F32 width;
  F32 height;
};

typedef struct Material Material;
struct Material {
  F32     scatter;
  Vec3F32 emit_color;
  Vec3F32 reflect_color;
};
Material _material_nil = ZeroStruct();
DefineArray(Material, MaterialArray, _material_nil);

typedef struct Plane Plane;
struct Plane {
  U32     material_id;
  Vec3F32 normal;
  F32     distance;
};
Plane _plane_nil = ZeroStruct();
DefineArray(Plane, PlaneArray, _plane_nil);

typedef struct Sphere Sphere;
struct Sphere {
  U32     material_id;
  Vec3F32 position;
  F32     radius;
};
Sphere _sphere_nil = ZeroStruct();
DefineArray(Sphere, SphereArray, _sphere_nil);

typedef struct World World;
struct World {
  Viewport      viewport;
  MaterialArray materials;
  SphereArray   spheres;
  PlaneArray    planes;

  U64 bounces_computed;
};

typedef struct RayHitResult RayHitResult;
struct RayHitResult {
  I32     hitted;
  Vec3F32 origin;
  Vec3F32 direction;
  Vec3F32 hit_position;
  Vec3F32 normal;
  Vec3F32 color;
};

func RayHitResult RayHit(World* world, Vec3F32 origin, Vec3F32 direction, I32 depth);
