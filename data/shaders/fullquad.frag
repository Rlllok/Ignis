#version 460

#define TWO_PI 6.28318530718

layout(location = 0) in vec2   frag_coordinates;
layout(location = 1) in float  frag_time;
layout(location = 2) in vec2   resolution;
layout(location = 3) in vec2   mouse_position;

layout(location = 0) out vec4 out_color;

float IG_Remap(float value, float in_min, float in_max, float out_min, float out_max)
{
  return value / (in_max - in_min) * (out_max - out_min);
}

float IG_Plot(vec2 st, float value){
  return  smoothstep(value - 0.02, value, st.y) - smoothstep( value, value+0.02, st.y);
}

vec3 IG_RgbFromHsv(vec3 hsv)
{
  vec4 K = vec4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
  vec3 p = abs(fract(hsv.xxx + K.xyz) * 6.0f - K.www);

  return hsv.z * mix(K.xxx, clamp(p - K.xxx, 0.0f, 1.0f), hsv.y);
}

float IG_SDF_Circle(vec2 in_position, float radius)
{
  return length(in_position) - radius;
}

float IG_SDF_Box(vec2 in_position, vec2 size)
{
  vec2 d = abs(in_position) - size;

  return length(max(d, 0.0f)) + min(max(d.x, d.y), 0.0f);
}

void main()
{
  vec3 white_color  = vec3(0.95f);
  vec3 red_color    = vec3(1.0f, 0.03f, 0.01f);
  vec3 blue_color   = vec3(0.02f, 0.03f, 0.98f);

  vec2 uv = frag_coordinates / resolution;
  vec3 final_color = white_color;

  // final_color = IG_RgbFromHsv(vec3(300.0f / 360.0f, 0.93f, 0.41f));

  float border_width = abs(sin(frag_time)) * 0.1f + 0.05f;
  float f = step(border_width, uv.x) * step(border_width, uv.y) * step(border_width, 1.0f - uv.x) * step(border_width, 1.0f - uv.y);
  final_color = vec3(f);

  f = step(0.05f, uv.x) * step(0.05f, uv.y) * step(0.6f, 1.0f - uv.x) * step(0.6f, 1.0f - uv.y);
  final_color = mix(final_color, red_color, f);
  
  // float d = IG_SDF_Circle(frag_coordinates - mouse_position, 75.0f);
  float d = IG_SDF_Box(frag_coordinates - mouse_position, vec2(50.0f, 50.0f));
  final_color = (d > 0.0f) ? white_color : blue_color;
  final_color *= 1.0f + 0.5f * cos(d);
  final_color = mix(final_color, red_color, 1.0f - smoothstep(0.0f, 3.0f, abs(d)));

  out_color = vec4(final_color, 1.0f);
}