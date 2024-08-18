#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragPosition;
layout(location = 3) in float frag_time;

layout(location = 0) out vec4 outColor;

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

void main()
{
  vec3  light_position  = vec3(1.0f, 1.0f, 1.0f);
  vec3  light_color     = vec3(1.0f, 1.0f, 1.0f);
  vec3  normal          = normalize(fragNormal);
  vec3  light_direction = normalize(light_position - fragPosition);
  float diffuse         = max(dot(normal, light_direction), 0.0f);
  vec3  diffuse_color   = diffuse * light_color;
  vec3  ambient_color   = 0.1f * light_color;
  vec3  color           = fragColor * IG_Remap(sin(frag_time), -1.0f, 1.0f, 0.5f, 1.0f);
  vec3  final_color     = (diffuse_color + ambient_color) * color;

  outColor = vec4(final_color, 1.0f);
}
