#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 norm;

layout(set = 2, binding = 0) uniform GlobalData
{
  vec3 light_direction;
  vec3 ambient_color;
};

layout(location = 0) out vec4 out_color;

void main()
{
  vec3 ambient_light = ambient_color;
  vec3 diffuse_light = vec3(max(0.0f, dot(-light_direction, norm)));

  out_color = vec4(ambient_light + diffuse_light, 1.0f);
}
