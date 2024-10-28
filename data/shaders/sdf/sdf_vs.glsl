#version 460

layout(location = 0) in vec3 a_position;

layout(set = 0, binding = 0) uniform SceneData
{
  mat4x4 projection;
} scene;

// --AlNov: @TODO DrawData name describe nothing for me
layout(set = 1, binding = 0) uniform DrawData
{
  vec2 translate;
  vec2 size;
  vec3 color;
} draw;

layout(location = 0) out vec2 uv;

layout(location = 1) out struct DataTransfer
{
  vec3 color;
  vec2 position;
  vec2 size;
} data_transfer;

void main()
{
  uv = a_position.xy;

  data_transfer.color = draw.color;
  data_transfer.position = draw.translate;
  data_transfer.size = draw.size;

  gl_Position = vec4(a_position.xy, 0.0f, 1.0f);
}
