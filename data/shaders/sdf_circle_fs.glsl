#version 460

float SDF2D_Circle(vec2 in_position, float radius)
{
  return length(in_position) - radius;
}

layout(location = 0) in vec2 uv;

layout(location = 1) in struct DataTransfer
{
  vec3 color;
} data_transfer;

layout(location = 0) out vec4 out_color;

void main()
{
  float d = SDF2D_Circle(uv, 1.0f);

  float antialising = smoothstep(0.0f, -(fwidth(d)), d);

  out_color = vec4(data_transfer.color, antialising);
}
