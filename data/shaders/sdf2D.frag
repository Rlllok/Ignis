#version 460

#define TWO_PI 6.28318530718

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

layout(location = 0) in vec2   uv;
layout(location = 1) in vec2   size;
layout(location = 2) in vec3   color;
layout(location = 3) in float  is_box;

layout(location = 0) out vec4 out_color;

void main()
{
  vec3 white_color  = vec3(0.95f);
  vec3 red_color    = vec3(1.0f, 0.03f, 0.01f);
  vec3 blue_color   = vec3(0.02f, 0.03f, 0.98f);

  float d = 0.0f;

  if (is_box > 0.0f)
  {
    d = IG_SDF_Box(uv, vec2(1.0f));
  }
  else
  {
    d = IG_SDF_Circle(uv, 1.0f);
  }

  float antialising = smoothstep(0.0f, -(fwidth(d)), d);

  out_color = vec4(color, antialising);
}