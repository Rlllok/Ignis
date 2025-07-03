#version 460

layout(location = 0) in vec3 a_position;

layout(set = 0, binding = 0) uniform UData
{
  mat4x4 projection;
  float rotation;
  vec2 translate;
  vec2 size;
  vec3 color;
} u_data;

layout(location = 0) out vec2 uv;

layout(location = 1) out struct DataTransfer
{
  vec3 color;
  vec2 position;
  vec2 size;
  float rotation;
} data_transfer;

void main()
{
  uv = a_position.xy;

  data_transfer.color = u_data.color;
  data_transfer.position = u_data.translate;
  data_transfer.size = u_data.size;
  data_transfer.rotation = u_data.rotation;

  gl_Position = vec4(a_position.xy, 0.00f, 1.0f);
}
