#version 460

// --AlNov: @TODO https://medium.com/@lumi_/render-any-2d-shape-using-a-single-shader-263be93879d9

float SDF2D_Circle(vec2 in_position, float radius)
{
  return length(in_position) - radius;
}

layout(location = 0) in vec2 uv;

layout(location = 1) in struct DataTransfer
{
  vec3 color;
  vec2 position;
  vec2 radius;
} DT;

layout(location = 0) out vec4 out_color;

void main()
{
  float d = SDF2D_Circle(gl_FragCoord.xy - DT.position, DT.radius.x);

  float antialising = smoothstep(0.0f, -(fwidth(d)), d);

  out_color = vec4(DT.color, antialising);
}
