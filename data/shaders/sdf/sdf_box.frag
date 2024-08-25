#version 460

float IG_SDF_Box(vec2 in_position, vec2 size)
{
  vec2 d = abs(in_position) - size;

  return length(max(d, 0.0f)) + min(max(d.x, d.y), 0.0f);
}

layout(location = 0) in vec2   uv;
layout(location = 1) in vec3   color;

layout(location = 0) out vec4 out_color;

void main()
{
  float d           = IG_SDF_Box(uv, vec2(1.0f));
  float antialising = smoothstep(0.0f, -(fwidth(d)), d);

  out_color = vec4(color, antialising);
}