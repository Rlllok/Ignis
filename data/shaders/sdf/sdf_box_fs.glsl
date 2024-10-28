#version 460

float SDF2D_Box(vec2 in_position, vec2 size, float corner_radius)
{
  vec2 d = abs(in_position) - size + corner_radius;

  return length(max(d, 0.0f)) + min(max(d.x, d.y), 0.0f) - corner_radius;
}

layout(location = 0) in vec2 in_uv;

layout(location = 1) in struct DataTransfer
{
  vec3 color;
  vec2 position;
  vec2 size;
} DT;

layout(location = 0) out vec4 out_color;

void main()
{
  float corner_radius = 10.0f;
  float d = SDF2D_Box(gl_FragCoord.xy - DT.position - DT.size*0.5f, DT.size*0.5, corner_radius);
  float smooth_alpha = 1.0f - smoothstep(0.0f, 2.0f, d);

  out_color = vec4(DT.color, smooth_alpha);
}
