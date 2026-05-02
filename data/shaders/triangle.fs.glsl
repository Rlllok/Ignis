#version 460

#extension GL_EXT_buffer_reference  : require
#extension GL_EXT_buffer_reference2 : require

layout(location = 0) in vec3 color;

layout(buffer_reference) readonly buffer TriangleDataBuffer {
  vec3 tint;
};

layout(set = 2, binding = 0) uniform GlobalData {
  TriangleDataBuffer data_buffer;
};

layout(location = 0) out vec4 frag_color;

void main()
{
	frag_color = vec4(color+data_buffer[1].tint, 1.0f);
}
