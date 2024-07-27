#version 460

layout(location = 0) in vec3 frag_nolor;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_position;
layout(location = 3) in vec2 frag_uv;

layout(location = 0) out vec4 out_color;

float IF_Lerp(float a, float b, float t)
{
  return a * (1.0f - t) + b * t;
}

float IG_InverseLerp(float a, float b, float v)
{
  return (v - a) / (b - a);
}

float IG_Remap(float value, float in_min, float in_max, float out_min, float out_max)
{
  return value / (in_max - in_min) * (out_max - out_min);
}

vec3 BackgroundColor()
{
  float distance_from_center = length(abs(frag_uv.xy - 0.5f));

  float vignette = 1.0f - distance_from_center;
  vignette = smoothstep(0.0f, 2.0f, vignette);

  return vec3(vignette);
}

vec3 DrawGrid(vec3 color, uvec2 resolution, vec3 line_color, float cell_spacing, float line_width)
{
  vec2 center_uv = frag_uv - 0.5f;
  vec2 cells = abs(fract(center_uv * resolution / cell_spacing) - 0.5f);
  float distance_to_edge = (0.5 - max(cells.x, cells.y)) * cell_spacing;
  float lines = smoothstep(0.0f, line_width, distance_to_edge);

  color = mix(line_color, color, lines);

  return color;
}

void main()
{
  uvec2 resolution = uvec2(1280, 720);

  vec2 pixel_coords = (frag_uv - 0.5f) * resolution;

  vec3 color = BackgroundColor();
  color = DrawGrid(color, resolution, vec3(0.3f), 10.0f, 1.0f);
  color = DrawGrid(color, resolution, vec3(0.0f), 100.0f, 2.0f);

  out_color = vec4(color, 1.0f);
}