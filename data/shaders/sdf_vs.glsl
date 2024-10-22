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
} draw;

layout(location = 0) out vec2 uv;

void main()
{
  vec3 vertex_position = a_position * vec3(draw.size, 0.0f) + vec3(draw.translate, 0.0f);
  vertex_position += vec3(draw.size, 0.0f); // --AlNov: Anchor in upper-left corner

  uv = a_position.xy;
  
  gl_Position = scene.projection * vec4(vertex_position, 1.0f);
  gl_Position.z = 0.0f;
}
