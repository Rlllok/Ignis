#version 460

layout(set = 3, binding = 0) uniform InstanceData
{
  vec4 color;
};

layout(location = 0) out vec4 out_color;

void main()
{
    out_color = color;
}
