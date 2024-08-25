#version 460

float IG_SDF_Circle(vec2 in_position, float radius)
{
  return length(in_position) - radius;
}

layout(location = 0) in vec2   uv;
layout(location = 1) in vec3   color;

layout(location = 0) out vec4 out_color;

void main()
{
  float d = IG_SDF_Circle(uv, 1.0f);

  float antialising = smoothstep(0.0f, -(fwidth(d)), d);

  out_color = vec4(color, antialising);
}