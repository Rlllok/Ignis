#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

const vec4 vertecies[] = {
  vec4(-1.0f, 0.0f, -1.0f, 1.0f),
  vec4( 1.0f, 0.0f, -1.0f, 1.0f),
  vec4( 1.0f, 0.0f,  1.0f, 1.0f),
  vec4(-1.0f, 0.0f,  1.0f, 1.0f),
};

const vec2 uv[] = {
  vec2(0.0f, 1.0f),
  vec2(1.0f, 1.0f),
  vec2(1.0f, 0.0f),
  vec2(0.0f, 0.0f),
};

const uint indecies[] = {
  0, 1, 3,
  1, 2, 3
};

layout(buffer_reference, std430) readonly buffer PillarsDataBuffer {
  vec3   player_position;
  vec3   camera_position;
  mat4x4 camera_inverse;
  vec4   color;
  vec2   resolution;
};

layout(push_constant, std430) uniform args {
  PillarsDataBuffer pillars_data;
};

void main() {
  gl_Position = vec4(uv[indecies[gl_VertexIndex]]*2.0f - 1.0f, 1.0f, 1.0f);
}
