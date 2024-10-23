#version 460

float SDF2D_Box(vec2 in_position, vec2 size)
{
  vec2 d = abs(in_position) - size;

  return length(max(d, 0.0f)) + min(max(d.x, d.y), 0.0f);
}

layout(location = 0) in vec2 in_uv;

layout(location = 1) in struct DataTransfer
{
  vec3 color;
} data_transfer;

layout(location = 0) out vec4 out_color;

void main()
{
  float d = SDF2D_Box(in_uv, vec2(1.0f));
  float antialising = smoothstep(0.0f, -(fwidth(d)), d);

  out_color = vec4(data_transfer.color, antialising);
}
