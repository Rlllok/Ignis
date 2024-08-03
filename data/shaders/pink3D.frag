#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;

void main()
{
  vec3  light_position  = vec3(1.0f, 1.0f, 1.0f);
  vec3  light_color     = vec3(1.0f, 1.0f, 1.0f);
  vec3  normal          = normalize(fragNormal);
  vec3  light_direction = normalize(light_position - fragPosition);
  float diffuse         = max(dot(normal, light_direction), 0.0f);
  vec3  diffuse_color   = diffuse * light_color;
  vec3  ambient_color   = 0.1f * light_color;
  vec3  final_color     = (diffuse_color + ambient_color) * fragColor;

  outColor = vec4(final_color, 1.0f);
}
