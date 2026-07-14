#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

const vec4 vertecies[] = {
  vec4(-0.5f, 0.5f, 0.5f, 1.0f),
  vec4(-0.5f, 0.5f,-0.5f, 1.0f),
  vec4( 0.5f, 0.5f,-0.5f, 1.0f),
  vec4( 0.5f, 0.5f, 0.5f, 1.0f),

  vec4(-0.5f,-0.5f, 0.5f, 1.0f),
  vec4(-0.5f,-0.5f,-0.5f, 1.0f),
  vec4( 0.5f,-0.5f,-0.5f, 1.0f),
  vec4( 0.5f,-0.5f, 0.5f, 1.0f),
};

const uint indecies[] = {
  0, 1, 2,
  0, 2, 3,
  
  4, 5, 6,
  4, 6, 7,

  0, 1, 5,
  0, 5, 4,

  3, 2, 6,
  3, 6, 7,

  0, 3, 4,
  0, 7, 4,

  1, 2, 6,
  1, 6, 5,
};

layout(buffer_reference, std430) readonly buffer BoundingBoxBuffer {
  mat4x4 transform;
  mat4x4 camera_transform;
  vec4   rgba;
};

layout(push_constant, std430) uniform args {
  BoundingBoxBuffer box_data;
};

void main() {
  vec4 vertex = vertecies[indecies[gl_VertexIndex]];
  gl_Position = box_data.camera_transform*box_data.transform*vertex;
};

