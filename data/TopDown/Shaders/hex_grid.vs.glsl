#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

const vec4 vertecies[] = {
  vec4(-1.0f, 0.0f, -1.0f, 1.0f),
  vec4( 1.0f, 0.0f, -1.0f, 1.0f),
  vec4( 1.0f, 0.0f,  1.0f, 1.0f),
  vec4(-1.0f, 0.0f,  1.0f, 1.0f),
};

const uint indecies[] = {
  0, 1, 3,
  1, 2, 3
};

layout(buffer_reference, std430) readonly buffer GridDataBuffer {
  mat4x4 transform;
  mat4x4 camera_transform;
  vec3 background_color;
  vec3 grid_color;
};

layout(push_constant, std430) uniform args {
  GridDataBuffer grid_data;
};

layout(location = 0) out vec3 world_position;

void main() {
  vec4 vertex = vertecies[indecies[gl_VertexIndex]];
  world_position = vec3(grid_data.transform*vertex);
  gl_Position = grid_data.camera_transform*vec4(world_position, 1.0f);
}
