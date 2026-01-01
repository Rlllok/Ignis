#version 460

layout(location = 0) in vec3  position;
layout(location = 1) in vec3  normal;
layout(location = 2) in vec3  tangent;
layout(location = 3) in vec2  uv;
layout(location = 4) in ivec4 joints_ids;
layout(location = 5) in vec4  joint_weights;

const int MAX_BONES = 64;

layout(set = 1, binding = 0) uniform InstanceData
{
  mat4 mvp;
};

void main()
{
  gl_Position = mvp*vec4(position, 1.0f);
}
