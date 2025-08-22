#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

layout(set = 3, binding = 0) uniform InstanceData
{
  float entity_id;
};

layout(location = 0) out vec4 out_color;
layout(location = 1) out uint out_two;

void main(void)
{
  out_color = vec4(step(0.9f, abs(uv*2.0f - 1.0f)), 0.0f, 1.0f);
  out_two = uint(entity_id);
}
