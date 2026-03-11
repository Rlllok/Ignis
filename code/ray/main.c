#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.c"
#include "os/os_include.c"

#include <stdlib.h>
#include <time.h>

#include "ray.h"

#define RandomF32() ((F32)rand()/((F32)RAND_MAX + 1))
#define RandomF32Range(min, max) (min + (max - min)*RandomF32())

func Vec3F32
RandomUnitVec3F32() {
  while (1) {
    Vec3F32 v = MakeVec3F32(RandomF32Range(-1.0f, 1.0f), RandomF32Range(-1.0f, 1.0f), RandomF32Range(-1.0f, 1.0f));
    F32 length_square = MagnitudeSquareVec3F32(v);
    if (1e-160 < length_square && length_square <= 1.0f) {
      return ScaleVec3F32(v, 1.0f/length_square);
    }
  }
}

func Image32
CreateImage32(Arena* arena, I32 width, I32 height) {
  Image32 result = ZeroStruct();
  result.width = width;
  result.height = height;
  result.pixels = (U32*)PushArena(arena, sizeof(U32)*result.width*result.height);

  return result;
}

func void
WriteImage32(Image32 image, char* file_name) {
  U32 output_size = sizeof(U32)*image.width*image.height;

  BitmapHeader header = {
    .file_type = 0x4d42,
    .file_size = sizeof(BitmapHeader) + output_size,
    .bitmap_offset = sizeof(BitmapHeader),
    .size = sizeof(BitmapHeader) - 14,
    .width = image.width,
    .height = -image.height,
    .planes = 1,
    .bits_per_pixel = 32,
    .compression = 0,
    .size_of_bitmap = output_size,
    .horizontal_resolution = 0,
    .vertical_resolution = 0,
    .colors_used = 0,
    .colors_important = 0,
  };

  FILE* output_file = fopen(file_name, "wb");
  if (output_file) {
    fwrite(&header, sizeof(header), 1, output_file);
    fwrite(image.pixels, output_size, 1, output_file);
    fclose(output_file);
  }
  else {
    LogError("Cannot open %s\n", file_name);
  }
}

func F32
SRGBFromLinear(F32 L) {
  L = Clamp(L, 0.0f, 1.0f);

  F32 S = L*12.92;
  if (L > 0.0031308f) {
    S = 1.055f*powf(L, 1.0f/2.4f) - 0.055f;
  }

  return S;
}

func U32
Pixel32FromVec3F32(Vec3F32 v) {
  U32 result = 0xff000000;
  F32 r = Max(0.0f, Min(1.0f, v.r));
  r = r*255.0;
  F32 g = Max(0.0f, Min(1.0f, v.g));
  g = g*255.0;
  F32 b = Max(0.0f, Min(1.0f, v.b));
  b = b*255.0;

  result = result + ((U32)(r) << 16);
  result = result + ((U32)(g) << 8);
  result = result + (U32)(b);
  return result;
}

func Vec3F32
RayCast(World* world, Vec3F32 origin, Vec3F32 direction) {
  Vec3F32 result = ZeroStruct();
  Vec3F32 attenuation = MakeVec3F32(1.0f, 1.0f, 1.0f);

  F32 tolerance = 0.0001f;
  F32 min_hit_distance = 0.001f;

  for (I32 bounce_index = 0; bounce_index < 8; bounce_index += 1) {
    F32 hit_distance = F32_MAX;

    I32 hit_material = 0;
    Vec3F32 next_origin = MakeVec3F32(0.0f, 0.0f, 0.0f);
    Vec3F32 next_normal = MakeVec3F32(0.0f, 0.0f, 0.0f);

    world->bounces_computed += 1;

    for (I32 plane_index = 0; plane_index < world->planes.length; plane_index += 1) {
      Plane plane = PlaneArrayGet(&world->planes, plane_index);

      F32 denominator = DotVec3F32(plane.normal, direction);
      if ((denominator < -tolerance) || (denominator > tolerance)) {
        F32 t = (-plane.distance - DotVec3F32(plane.normal, origin))/denominator;
        if ((t > min_hit_distance) && (t < hit_distance)) {
          hit_distance = t;
          hit_material = plane.material_id;

          next_origin = ScaleVec3F32(direction, t);
          next_normal = plane.normal;
        }
      }
    }

    for (I32 sphere_index = 0; sphere_index < world->spheres.length; sphere_index += 1) {
      Sphere sphere = SphereArrayGet(&world->spheres, sphere_index);

      Vec3F32 local_origin = SubVec3F32(origin, sphere.position);

      F32 a = DotVec3F32(direction, direction);
      F32 b = 2.0f*DotVec3F32(direction, local_origin);
      F32 c = DotVec3F32(local_origin, local_origin) - sphere.radius*sphere.radius;
       
      F32 denominator = 2.0f*a;
      F32 root = sqrt(b*b - 4.0f*a*c);
      if (root > tolerance) {
        F32 tp = (-b + root)/denominator;
        F32 tn = (-b - root)/denominator;

        F32 t = tp;
        if ((tn > min_hit_distance) && (tn < tp)) {
          t = tn;
        }
        if ((t > min_hit_distance) && (t < hit_distance)) {
          hit_distance = t;
          hit_material = sphere.material_id;

          next_origin = AddVec3F32(origin, ScaleVec3F32(direction, t));
          next_normal = NormalizeVec3F32(SubVec3F32(next_origin, sphere.position));
        }
      }
    }

    if (hit_material != 0) {
      Material material = MaterialArrayGet(&world->materials, hit_material);
      result = AddVec3F32(result, MulVec3F32(attenuation, material.emit_color));
      // --AlNov: @TODO Turned of cos attunuation by settin 1.0f to max
      attenuation = ScaleVec3F32(MulVec3F32(attenuation, material.reflect_color), Max(1.0f, DotVec3F32(ScaleVec3F32(direction, -1.0f), next_normal)));

      origin = next_origin;
      Vec3F32 bounce_direction = SubVec3F32(direction, ScaleVec3F32(next_normal, 2.0f*DotVec3F32(direction, next_normal)));
      Vec3F32 random_bounce_direction = RandomUnitVec3F32();
      direction = NormalizeVec3F32(LerpVec3F32(random_bounce_direction, bounce_direction, material.scatter));
    }
    else {
      Material material = MaterialArrayGet(&world->materials, hit_material);
      result = AddVec3F32(result, MulVec3F32(attenuation, material.emit_color));
      
      break;
    }
  }

  return result;
}

I32 main() {
  Arena* arena = AllocateArena(Gigabytes(1), Kilobytes(64));
  Image32 image = CreateImage32(arena, 1280, 720);

  World world = ZeroStruct();
  world.viewport = (Viewport) {
    .height = 2.0f,
    .width = 2.0f*((F32)(image.width)/(F32)(image.height)),
  };
  world.materials = MaterialArrayAllocate(arena, 16);
  world.spheres = SphereArrayAllocate(arena, 16);
  world.planes = PlaneArrayAllocate(arena, 16);

  MaterialArrayAdd(&world.materials, (Material) {
    .emit_color = MakeVec3F32(0.7f, 0.6f, 0.62f),
  });
  MaterialArrayAdd(&world.materials, (Material) {
    .reflect_color = MakeVec3F32(0.2f, 0.88f, 0.2f),
  });
  MaterialArrayAdd(&world.materials, (Material) {
    .reflect_color = MakeVec3F32(0.9f, 0.35f, 0.9f),
  });
  MaterialArrayAdd(&world.materials, (Material) {
    .emit_color = MakeVec3F32(4.8f, 0.2f, 0.4f),
  });
  MaterialArrayAdd(&world.materials, (Material) {
    .reflect_color = MakeVec3F32(0.4f, 0.8f, 0.67f),
    .scatter = 0.9f,
  });

  SphereArrayAdd(&world.spheres, (Sphere) {
    .material_id = 2,
    .position = MakeVec3F32(0.5f, -0.8f, -3.5f),
    .radius = 1.0f,
  });
  SphereArrayAdd(&world.spheres, (Sphere) {
    .material_id = 3,
    .position = MakeVec3F32(-1.0f, -0.8f, -2.5f),
    .radius = 0.5f,
  });
  SphereArrayAdd(&world.spheres, (Sphere) {
    .material_id = 4,
    .position = MakeVec3F32(0.8f, 1.2f, -3.5f),
    .radius = 1.0f,
  });

  PlaneArrayAdd(&world.planes, (Plane) {
    .material_id = 1,
    .normal = MakeVec3F32(0.0f, 1.0f, 0.0f),
    .distance = 1.0f,
  });

  F32 focal_length = 1.0f;
  I32 samples_per_pixel = 16;
  Vec3F32 camera_center = MakeVec3F32(0.0f, 0.0f, 0.0f);

  Vec3F32 viewport_u = MakeVec3F32(world.viewport.width, 0.0f, 0.0f);
  Vec3F32 viewport_v = MakeVec3F32(0.0f, -world.viewport.height, 0.0f);

  Vec3F32 pixel_delta_u = ScaleVec3F32(viewport_u, 1.0f/(F32)image.width);
  Vec3F32 pixel_delta_v = ScaleVec3F32(viewport_v, 1.0f/(F32)image.height);

  Vec3F32 viewport_upper_left = SubVec3F32(camera_center, MakeVec3F32(0.0f, 0.0f, focal_length));
  viewport_upper_left = SubVec3F32(viewport_upper_left, ScaleVec3F32(viewport_u, 0.5f));
  viewport_upper_left = SubVec3F32(viewport_upper_left, ScaleVec3F32(viewport_v, 0.5f));

  Vec3F32 pixel_00 = AddVec3F32(viewport_upper_left, ScaleVec3F32(AddVec3F32(pixel_delta_u, pixel_delta_v), 0.5f));

  U32* out = image.pixels;
  clock_t start_time = clock();
  for (I32 y = 0; y < image.height; y += 1) {
    for (I32 x = 0; x < image.width; x += 1) {
      Vec3F32 color = ZeroStruct();
      for (I32 sample_index = 0; sample_index < samples_per_pixel; sample_index += 1) {
        Vec3F32 offset = MakeVec3F32(RandomF32() - 0.5f, RandomF32() - 0.5f, 0.0f);
        Vec3F32 pixel_position = AddVec3F32(pixel_00, ScaleVec3F32(pixel_delta_u, (F32)x + offset.x));
        pixel_position = AddVec3F32(pixel_position, ScaleVec3F32(pixel_delta_v, (F32)y + offset.y));
        Vec3F32 ray_origin = camera_center;
        Vec3F32 ray_direction = NormalizeVec3F32(SubVec3F32(pixel_position, ray_origin));

        color = AddVec3F32(color, RayCast(&world, ray_origin, ray_direction));
      }
      color = ScaleVec3F32(color, 1.0f/(F32)samples_per_pixel);
      color = MakeVec3F32(
        SRGBFromLinear(color.r),
        SRGBFromLinear(color.g),
        SRGBFromLinear(color.b)
      );
      *out = Pixel32FromVec3F32(color);
      out += 1;
    }
    if ((y%64) == 0) {
      LogText("\rY is %f%%", (F32)y*100.0f/(F32)image.height);
      fflush(stdout);
    }
  }
  clock_t end_time = clock();
  LogText("\nImage Generated.\n");
  LogText("Time: %dms\n", end_time - start_time);
  LogText("Bounces: %llu\n", world.bounces_computed);
  LogText("ms/bounce: %f\n", (F64)(end_time - start_time)/(F64)world.bounces_computed);

  WriteImage32(image, "ray_result.bmp");

  return 0;
}
